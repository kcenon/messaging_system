// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include "kcenon/messaging/task/task_queue.h"

#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/common/logging/log_functions.h>

#include <algorithm>

namespace kcenon::messaging::task {

// ============================================================================
// delayed_task_worker implementation
// ============================================================================

delayed_task_worker::delayed_task_worker(task_queue& parent,
										 std::chrono::milliseconds poll_interval)
	: thread_base("delayed_task_worker"), parent_(parent) {
	set_wake_interval(poll_interval);
}

void delayed_task_worker::notify_new_task() {
	{
		std::lock_guard<std::mutex> lock(cv_mutex_);
		notified_.store(true);
	}
	cv_.notify_one();
}

bool delayed_task_worker::should_continue_work() const {
	// Return false when stopped to allow graceful shutdown
	if (parent_.stopped_.load()) {
		return false;
	}
	// Always return true otherwise to ensure thread_base's wait_for returns
	// immediately and delegates waiting to do_work()'s cv_.wait_for, which
	// can be properly notified by notify_new_task()
	return true;
}

kcenon::thread::result_void delayed_task_worker::do_work() {
	// Process any ready delayed tasks
	parent_.process_delayed_tasks();

	// Calculate wait time
	auto wait_time = parent_.get_next_delayed_wait_time();

	// Wait for notification or timeout
	{
		std::unique_lock<std::mutex> lock(cv_mutex_);
		if (!parent_.stopped_.load()) {
			cv_.wait_for(lock, wait_time, [this]() {
				return notified_.load() || parent_.stopped_.load();
			});
			notified_.store(false);
		}
	}

	return {};
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

task_queue::task_queue(task_queue_config config) : config_(std::move(config)) {
}

task_queue::~task_queue() {
	stop();
}

task_queue::task_queue(task_queue&& other) noexcept
	: config_(std::move(other.config_)),
	  queues_(std::move(other.queues_)),
	  delayed_queue_(std::move(other.delayed_queue_)),
	  task_registry_(std::move(other.task_registry_)),
	  cancelled_tasks_(std::move(other.cancelled_tasks_)),
	  tag_to_tasks_(std::move(other.tag_to_tasks_)),
	  running_(other.running_.load()),
	  stopped_(other.stopped_.load()),
	  delayed_worker_(std::move(other.delayed_worker_)) {
	other.running_.store(false);
	other.stopped_.store(true);
}

task_queue& task_queue::operator=(task_queue&& other) noexcept {
	if (this != &other) {
		stop();

		config_ = std::move(other.config_);
		queues_ = std::move(other.queues_);
		delayed_queue_ = std::move(other.delayed_queue_);
		task_registry_ = std::move(other.task_registry_);
		cancelled_tasks_ = std::move(other.cancelled_tasks_);
		tag_to_tasks_ = std::move(other.tag_to_tasks_);
		running_.store(other.running_.load());
		stopped_.store(other.stopped_.load());
		delayed_worker_ = std::move(other.delayed_worker_);

		other.running_.store(false);
		other.stopped_.store(true);
	}
	return *this;
}

// ============================================================================
// Lifecycle
// ============================================================================

common::VoidResult task_queue::start() {
	if (running_.load()) {
		return common::VoidResult(
			common::error_info{error::already_running, "Task queue already running"});
	}

	stopped_.store(false);
	running_.store(true);

	if (config_.enable_delayed_queue) {
		delayed_worker_ =
			std::make_unique<delayed_task_worker>(*this, config_.delayed_poll_interval);
		auto result = delayed_worker_->start();
		if (result.has_error()) {
			running_.store(false);
			stopped_.store(true);
			delayed_worker_.reset();
			return common::VoidResult(
				common::error_info{error::not_running, "Failed to start delayed task worker"});
		}
	}

	common::logging::log_info("Task queue started");
	return common::ok();
}

void task_queue::stop() {
	if (!running_.load()) {
		return;
	}

	stopped_.store(true);
	running_.store(false);

	// Stop all queues
	{
		std::lock_guard<std::mutex> lock(queues_mutex_);
		for (auto& [name, queue] : queues_) {
			queue->stop();
		}
	}

	// Stop delayed worker using thread_system's lifecycle management
	if (delayed_worker_) {
		delayed_worker_->notify_new_task();  // Wake up to check stopped_ flag
		delayed_worker_->stop();
		delayed_worker_.reset();
	}

	common::logging::log_info("Task queue stopped");
}

bool task_queue::is_running() const {
	return running_.load() && !stopped_.load();
}

// ============================================================================
// Enqueue operations
// ============================================================================

common::Result<std::string> task_queue::enqueue(task t) {
	if (!is_running()) {
		return common::Result<std::string>(
			common::error_info{error::not_running, "Task queue not running"});
	}

	const std::string task_id = t.task_id();
	const std::string queue_name = t.config().queue_name;

	// Check if task should be delayed
	if (t.config().eta.has_value()) {
		auto now = std::chrono::system_clock::now();
		auto eta = t.config().eta.value();
		if (eta > now) {
			// Add to delayed queue
			// Note: Capture eta before std::move to avoid undefined behavior
			{
				std::lock_guard<std::mutex> lock(delayed_mutex_);
				delayed_queue_.push({std::move(t), eta});
			}
			// Notify delayed worker about new task
			if (delayed_worker_) {
				delayed_worker_->notify_new_task();
			}

			common::logging::log_trace("Task " + task_id + " added to delayed queue");
			return common::ok(task_id);
		}
	}

	// Register task
	register_task(t);

	// Set task state to queued
	t.set_state(task_state::queued);

	// Ensure queue exists
	ensure_queue_exists(queue_name);

	// Enqueue to the appropriate queue
	message msg = static_cast<message>(t);

	common::VoidResult result = common::ok();
	{
		std::lock_guard<std::mutex> lock(queues_mutex_);
		auto it = queues_.find(queue_name);
		if (it != queues_.end()) {
			result = it->second->enqueue(std::move(msg));
		} else {
			result = common::VoidResult(
				common::error_info{error::queue_empty, "Queue not found"});
		}
	}

	if (!result.is_ok()) {
		unregister_task(task_id);
		return common::Result<std::string>(result.error());
	}

	common::logging::log_trace("Task " + task_id + " enqueued to " + queue_name);
	return common::ok(task_id);
}

common::Result<std::vector<std::string>> task_queue::enqueue_bulk(
	std::vector<task> tasks) {
	std::vector<std::string> task_ids;
	task_ids.reserve(tasks.size());

	for (auto& t : tasks) {
		auto result = enqueue(std::move(t));
		if (result.is_ok()) {
			task_ids.push_back(result.unwrap());
		}
	}

	return common::ok(std::move(task_ids));
}

// ============================================================================
// Dequeue operations
// ============================================================================

common::Result<task> task_queue::dequeue(
	const std::vector<std::string>& queue_names,
	std::chrono::milliseconds timeout) {

	if (!is_running()) {
		return common::Result<task>(
			common::error_info{error::not_running, "Task queue not running"});
	}

	if (queue_names.empty()) {
		return common::Result<task>(
			common::error_info{error::queue_empty, "No queue names specified"});
	}

	auto start_time = std::chrono::steady_clock::now();
	auto remaining_timeout = timeout;

	while (remaining_timeout > std::chrono::milliseconds::zero() || timeout == std::chrono::milliseconds::max()) {
		// Try each queue in order
		for (const auto& queue_name : queue_names) {
			std::lock_guard<std::mutex> lock(queues_mutex_);
			auto it = queues_.find(queue_name);
			if (it != queues_.end()) {
				auto result = it->second->try_dequeue();
				if (result.is_ok()) {
					message msg = std::move(result.unwrap());

					// Reconstruct task from message
					// For now, we look up from registry
					auto msg_id = msg.metadata().id;
					{
						std::lock_guard<std::mutex> reg_lock(registry_mutex_);
						auto task_it = task_registry_.find(msg_id);
						if (task_it != task_registry_.end()) {
							// Check if cancelled
							if (is_task_cancelled(msg_id)) {
								unregister_task(msg_id);
								continue;  // Skip cancelled tasks
							}

							task t = std::move(task_it->second);
							t.set_state(task_state::running);
							t.set_started_at(std::chrono::system_clock::now());
							task_registry_.erase(task_it);

							common::logging::log_trace("Task " + msg_id + " dequeued from " + queue_name);
							return common::ok(std::move(t));
						}
					}
				}
			}
		}

		// Wait a bit before retrying
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (timeout != std::chrono::milliseconds::max()) {
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - start_time);
			remaining_timeout = timeout - elapsed;
		}

		if (!is_running()) {
			break;
		}
	}

	return common::Result<task>(
		common::error_info{error::queue_empty, "No tasks available (timeout)"});
}

common::Result<task> task_queue::try_dequeue(
	const std::vector<std::string>& queue_names) {
	return dequeue(queue_names, std::chrono::milliseconds::zero());
}

// ============================================================================
// Cancellation
// ============================================================================

common::VoidResult task_queue::cancel(const std::string& task_id) {
	{
		std::lock_guard<std::mutex> lock(cancelled_mutex_);
		cancelled_tasks_.insert(task_id);
	}

	// Update task state in registry
	{
		std::lock_guard<std::mutex> lock(registry_mutex_);
		auto it = task_registry_.find(task_id);
		if (it != task_registry_.end()) {
			it->second.set_state(task_state::cancelled);
		}
	}

	common::logging::log_trace("Task " + task_id + " cancelled");
	return common::ok();
}

common::VoidResult task_queue::cancel_by_tag(const std::string& tag) {
	std::vector<std::string> task_ids;

	{
		std::lock_guard<std::mutex> lock(tags_mutex_);
		auto it = tag_to_tasks_.find(tag);
		if (it != tag_to_tasks_.end()) {
			task_ids.assign(it->second.begin(), it->second.end());
		}
	}

	for (const auto& task_id : task_ids) {
		cancel(task_id);
	}

	common::logging::log_trace("Cancelled " + std::to_string(task_ids.size()) +
							   " tasks with tag: " + tag);
	return common::ok();
}

// ============================================================================
// Query operations
// ============================================================================

common::Result<task> task_queue::get_task(const std::string& task_id) const {
	std::lock_guard<std::mutex> lock(registry_mutex_);
	auto it = task_registry_.find(task_id);
	if (it == task_registry_.end()) {
		return common::Result<task>(
			common::error_info{error::task_not_found, "Task not found: " + task_id});
	}
	return common::ok(task(it->second));
}

size_t task_queue::queue_size(const std::string& queue_name) const {
	std::lock_guard<std::mutex> lock(queues_mutex_);
	auto it = queues_.find(queue_name);
	if (it == queues_.end()) {
		return 0;
	}
	return it->second->size();
}

size_t task_queue::total_size() const {
	std::lock_guard<std::mutex> lock(queues_mutex_);
	size_t total = 0;
	for (const auto& [name, queue] : queues_) {
		total += queue->size();
	}
	return total;
}

size_t task_queue::delayed_size() const {
	std::lock_guard<std::mutex> lock(delayed_mutex_);
	return delayed_queue_.size();
}

std::vector<std::string> task_queue::list_queues() const {
	std::lock_guard<std::mutex> lock(queues_mutex_);
	std::vector<std::string> names;
	names.reserve(queues_.size());
	for (const auto& [name, queue] : queues_) {
		names.push_back(name);
	}
	return names;
}

bool task_queue::has_queue(const std::string& queue_name) const {
	std::lock_guard<std::mutex> lock(queues_mutex_);
	return queues_.find(queue_name) != queues_.end();
}

// ============================================================================
// Internal helpers
// ============================================================================

void task_queue::ensure_queue_exists(const std::string& queue_name) {
	std::lock_guard<std::mutex> lock(queues_mutex_);
	if (queues_.find(queue_name) == queues_.end()) {
		queue_config qconfig;
		qconfig.max_size = config_.max_size;
		qconfig.enable_priority = true;  // Always enable priority for tasks
		queues_[queue_name] = std::make_unique<message_queue>(qconfig);

		common::logging::log_trace("Created queue: " + queue_name);
	}
}

void task_queue::process_delayed_tasks() {
	auto now = std::chrono::system_clock::now();
	std::vector<task> ready_tasks;

	{
		std::lock_guard<std::mutex> lock(delayed_mutex_);
		while (!delayed_queue_.empty()) {
			const auto& top = delayed_queue_.top();
			if (top.eta <= now) {
				ready_tasks.push_back(std::move(const_cast<delayed_task&>(top).t));
				delayed_queue_.pop();
			} else {
				break;
			}
		}
	}

	// Enqueue ready tasks
	for (auto& t : ready_tasks) {
		// Clear ETA to prevent re-adding to delayed queue
		t.config().eta = std::nullopt;
		auto result = enqueue(std::move(t));
		if (!result.is_ok()) {
			common::logging::log_warning("Failed to enqueue delayed task: " +
										 std::string(result.error().message));
		}
	}
}

std::chrono::milliseconds task_queue::get_next_delayed_wait_time() const {
	std::lock_guard<std::mutex> lock(delayed_mutex_);

	if (delayed_queue_.empty()) {
		return config_.delayed_poll_interval;
	}

	auto now = std::chrono::system_clock::now();
	auto next_eta = delayed_queue_.top().eta;

	if (next_eta <= now) {
		return std::chrono::milliseconds::zero();
	}

	auto until_eta =
		std::chrono::duration_cast<std::chrono::milliseconds>(next_eta - now);
	return std::min(config_.delayed_poll_interval, until_eta);
}

bool task_queue::is_task_cancelled(const std::string& task_id) const {
	std::lock_guard<std::mutex> lock(cancelled_mutex_);
	return cancelled_tasks_.find(task_id) != cancelled_tasks_.end();
}

void task_queue::register_task(const task& t) {
	const std::string& task_id = t.task_id();

	// Register in task registry
	{
		std::lock_guard<std::mutex> lock(registry_mutex_);
		task_registry_[task_id] = t;
	}

	// Register tags
	const auto& tags = t.config().tags;
	if (!tags.empty()) {
		std::lock_guard<std::mutex> lock(tags_mutex_);
		for (const auto& tag : tags) {
			tag_to_tasks_[tag].insert(task_id);
		}
	}
}

void task_queue::unregister_task(const std::string& task_id) {
	// Get tags before removing from registry
	std::vector<std::string> tags;
	{
		std::lock_guard<std::mutex> lock(registry_mutex_);
		auto it = task_registry_.find(task_id);
		if (it != task_registry_.end()) {
			tags = it->second.config().tags;
			task_registry_.erase(it);
		}
	}

	// Remove from tag mappings
	if (!tags.empty()) {
		std::lock_guard<std::mutex> lock(tags_mutex_);
		for (const auto& tag : tags) {
			auto it = tag_to_tasks_.find(tag);
			if (it != tag_to_tasks_.end()) {
				it->second.erase(task_id);
				if (it->second.empty()) {
					tag_to_tasks_.erase(it);
				}
			}
		}
	}

	// Remove from cancelled set
	{
		std::lock_guard<std::mutex> lock(cancelled_mutex_);
		cancelled_tasks_.erase(task_id);
	}
}

}  // namespace kcenon::messaging::task
