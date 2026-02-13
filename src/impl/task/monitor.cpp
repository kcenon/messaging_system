// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/task/monitor.h>

#include <kcenon/messaging/error/messaging_error_category.h>

#include <algorithm>

namespace kcenon::messaging::task {

// ============================================================================
// Constructor / Destructor
// ============================================================================

task_monitor::task_monitor(
	std::shared_ptr<task_queue> queue,
	std::shared_ptr<result_backend_interface> results,
	std::shared_ptr<worker_pool> workers)
	: queue_(std::move(queue))
	, results_(std::move(results))
	, workers_(std::move(workers))
{
}

task_monitor::~task_monitor() = default;

// ============================================================================
// Queue statistics
// ============================================================================

std::vector<queue_stats> task_monitor::get_queue_stats() const {
	std::vector<queue_stats> stats;

	if (!queue_) {
		return stats;
	}

	auto queue_names = queue_->list_queues();
	size_t delayed_total = queue_->delayed_size();
	size_t active_count = 0;

	{
		std::lock_guard lock(active_mutex_);
		active_count = active_tasks_.size();
	}

	for (const auto& name : queue_names) {
		queue_stats qs;
		qs.name = name;
		qs.pending_count = queue_->queue_size(name);
		qs.running_count = 0;
		qs.delayed_count = 0;
		stats.push_back(std::move(qs));
	}

	// Distribute delayed count proportionally or add as separate entry
	if (delayed_total > 0 && !stats.empty()) {
		stats[0].delayed_count = delayed_total;
	}

	// Update running count from active tasks
	if (active_count > 0 && !stats.empty()) {
		std::lock_guard lock(active_mutex_);
		for (const auto& t : active_tasks_) {
			auto config = t.config();
			for (auto& qs : stats) {
				if (qs.name == config.queue_name) {
					qs.running_count++;
					break;
				}
			}
		}
	}

	return stats;
}

common::Result<queue_stats> task_monitor::get_queue_stats(
	const std::string& queue_name) const
{
	if (!queue_) {
		return common::Result<queue_stats>::err(
			make_typed_error_code(messaging_error_category::backend_not_ready));
	}

	if (!queue_->has_queue(queue_name)) {
		return common::Result<queue_stats>::err(
			make_typed_error_code(messaging_error_category::task_not_found));
	}

	queue_stats qs;
	qs.name = queue_name;
	qs.pending_count = queue_->queue_size(queue_name);
	qs.running_count = 0;
	qs.delayed_count = 0;

	// Count running tasks for this queue
	{
		std::lock_guard lock(active_mutex_);
		for (const auto& t : active_tasks_) {
			auto config = t.config();
			if (config.queue_name == queue_name) {
				qs.running_count++;
			}
		}
	}

	return common::ok(std::move(qs));
}

// ============================================================================
// Worker status
// ============================================================================

std::vector<worker_info> task_monitor::get_workers() const {
	std::vector<worker_info> workers;

	if (!workers_) {
		return workers;
	}

	// Create worker info based on available worker pool information
	size_t total = workers_->total_workers();
	size_t active = workers_->active_workers();

	// Generate synthetic worker info based on pool status
	for (size_t i = 0; i < total; ++i) {
		worker_info info;
		info.worker_id = "worker-" + std::to_string(i);
		info.active_tasks = (i < active) ? 1 : 0;
		info.last_heartbeat = std::chrono::system_clock::now();
		info.is_healthy = workers_->is_running();
		workers.push_back(std::move(info));
	}

	return workers;
}

std::optional<worker_statistics> task_monitor::get_worker_statistics() const {
	if (!workers_) {
		return std::nullopt;
	}

	return workers_->get_statistics();
}

// ============================================================================
// Task queries
// ============================================================================

std::vector<task> task_monitor::list_active_tasks() const {
	std::lock_guard lock(active_mutex_);
	return active_tasks_;
}

std::vector<task> task_monitor::list_pending_tasks(
	const std::string& queue_name) const
{
	(void)queue_name;  // Reserved for future implementation

	std::vector<task> pending;

	if (!queue_) {
		return pending;
	}

	// Note: task_queue doesn't expose direct iteration over pending tasks,
	// so we rely on tracking active tasks and providing queue statistics
	// For detailed pending task list, additional task_queue API would be needed

	return pending;
}

std::vector<task> task_monitor::list_failed_tasks(size_t limit) const {
	std::lock_guard lock(failed_mutex_);

	if (failed_tasks_.size() <= limit) {
		return failed_tasks_;
	}

	// Return the most recent failed tasks
	std::vector<task> result(
		failed_tasks_.end() - static_cast<ptrdiff_t>(limit),
		failed_tasks_.end()
	);
	return result;
}

// ============================================================================
// Task management
// ============================================================================

common::VoidResult task_monitor::cancel_task(const std::string& task_id) {
	if (!queue_) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::backend_not_ready));
	}

	return queue_->cancel(task_id);
}

common::VoidResult task_monitor::retry_task(const std::string& task_id) {
	if (!queue_ || !results_) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::backend_not_ready));
	}

	// Get the task from queue
	auto task_result = queue_->get_task(task_id);
	if (task_result.is_err()) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::task_not_found));
	}

	auto t = task_result.unwrap();

	// Check if task is in failed state
	if (t.state() != task_state::failed) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::task_operation_failed));
	}

	// Reset task state and re-enqueue
	t.set_state(task_state::pending);

	auto enqueue_result = queue_->enqueue(std::move(t));
	if (enqueue_result.is_err()) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::enqueue_failed));
	}

	// Remove from failed tasks list
	{
		std::lock_guard lock(failed_mutex_);
		failed_tasks_.erase(
			std::remove_if(
				failed_tasks_.begin(),
				failed_tasks_.end(),
				[&task_id](const task& t) { return t.task_id() == task_id; }
			),
			failed_tasks_.end()
		);
	}

	return common::ok();
}

common::VoidResult task_monitor::purge_queue(const std::string& queue_name) {
	if (!queue_) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::backend_not_ready));
	}

	if (!queue_->has_queue(queue_name)) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::task_not_found));
	}

	// Cancel all tasks with tag matching queue name
	// Note: For full purge functionality, task_queue would need additional API
	return queue_->cancel_by_tag(queue_name);
}

// ============================================================================
// Event subscription
// ============================================================================

void task_monitor::on_task_started(task_started_handler handler) {
	std::lock_guard lock(handlers_mutex_);
	started_handlers_.push_back(std::move(handler));
}

void task_monitor::on_task_completed(task_completed_handler handler) {
	std::lock_guard lock(handlers_mutex_);
	completed_handlers_.push_back(std::move(handler));
}

void task_monitor::on_task_failed(task_failed_handler handler) {
	std::lock_guard lock(handlers_mutex_);
	failed_handlers_.push_back(std::move(handler));
}

void task_monitor::on_worker_offline(worker_offline_handler handler) {
	std::lock_guard lock(handlers_mutex_);
	offline_handlers_.push_back(std::move(handler));
}

// ============================================================================
// Event notification
// ============================================================================

void task_monitor::notify_task_started(const task& t) {
	// Track active task
	{
		std::lock_guard lock(active_mutex_);
		active_tasks_.push_back(t);
	}

	// Notify handlers
	std::vector<task_started_handler> handlers;
	{
		std::lock_guard lock(handlers_mutex_);
		handlers = started_handlers_;
	}

	for (const auto& handler : handlers) {
		if (handler) {
			handler(t);
		}
	}
}

void task_monitor::notify_task_completed(const task& t, bool success) {
	// Remove from active tasks
	{
		std::lock_guard lock(active_mutex_);
		active_tasks_.erase(
			std::remove_if(
				active_tasks_.begin(),
				active_tasks_.end(),
				[&t](const task& active) { return active.task_id() == t.task_id(); }
			),
			active_tasks_.end()
		);
	}

	// Notify handlers
	std::vector<task_completed_handler> handlers;
	{
		std::lock_guard lock(handlers_mutex_);
		handlers = completed_handlers_;
	}

	for (const auto& handler : handlers) {
		if (handler) {
			handler(t, success);
		}
	}
}

void task_monitor::notify_task_failed(const task& t, const std::string& error) {
	// Remove from active tasks
	{
		std::lock_guard lock(active_mutex_);
		active_tasks_.erase(
			std::remove_if(
				active_tasks_.begin(),
				active_tasks_.end(),
				[&t](const task& active) { return active.task_id() == t.task_id(); }
			),
			active_tasks_.end()
		);
	}

	// Add to failed tasks
	{
		std::lock_guard lock(failed_mutex_);
		failed_tasks_.push_back(t);

		// Trim if exceeds max
		if (failed_tasks_.size() > max_failed_tasks_) {
			failed_tasks_.erase(
				failed_tasks_.begin(),
				failed_tasks_.begin() +
					static_cast<ptrdiff_t>(failed_tasks_.size() - max_failed_tasks_)
			);
		}
	}

	// Notify handlers
	std::vector<task_failed_handler> handlers;
	{
		std::lock_guard lock(handlers_mutex_);
		handlers = failed_handlers_;
	}

	for (const auto& handler : handlers) {
		if (handler) {
			handler(t, error);
		}
	}
}

void task_monitor::notify_worker_offline(const std::string& worker_id) {
	std::vector<worker_offline_handler> handlers;
	{
		std::lock_guard lock(handlers_mutex_);
		handlers = offline_handlers_;
	}

	for (const auto& handler : handlers) {
		if (handler) {
			handler(worker_id);
		}
	}
}

}  // namespace kcenon::messaging::task
