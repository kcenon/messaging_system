// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/task/worker_pool.h>
#include <kcenon/messaging/task/task_context.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <format>

namespace kcenon::messaging::task {

// ============================================================================
// task_pool_worker implementation
// ============================================================================

task_pool_worker::task_pool_worker(size_t worker_id, worker_pool& pool)
	: thread_base(std::format("task_worker_{}", worker_id))
	, worker_id_(worker_id)
	, pool_(pool) {
	// Set wake interval to match pool's poll interval
	set_wake_interval(pool_.config_.poll_interval);
}

auto task_pool_worker::should_continue_work() const -> bool {
	// Continue as long as the pool is running and shutdown hasn't been requested
	return pool_.running_.load() && !pool_.shutdown_requested_.load();
}

auto task_pool_worker::do_work() -> common::VoidResult {
	// Process one task per do_work call
	pool_.process_one_task();
	return common::ok();
}

worker_pool::worker_pool(
	std::shared_ptr<task_queue> queue,
	std::shared_ptr<result_backend_interface> results,
	worker_config config)
	: config_(std::move(config))
	, queue_(std::move(queue))
	, results_(std::move(results)) {
	stats_.started_at = std::chrono::system_clock::now();
}

worker_pool::~worker_pool() {
	if (running_) {
		stop();
	}
}

// ============================================================================
// Handler registration
// ============================================================================

void worker_pool::register_handler(std::shared_ptr<task_handler_interface> handler) {
	if (!handler) {
		return;
	}
	std::lock_guard<std::mutex> lock(handlers_mutex_);
	handlers_[handler->name()] = std::move(handler);
}

void worker_pool::register_handler(const std::string& name, simple_task_handler handler) {
	register_handler(make_handler(name, std::move(handler)));
}

bool worker_pool::unregister_handler(const std::string& name) {
	std::lock_guard<std::mutex> lock(handlers_mutex_);
	return handlers_.erase(name) > 0;
}

bool worker_pool::has_handler(const std::string& name) const {
	std::lock_guard<std::mutex> lock(handlers_mutex_);
	return handlers_.find(name) != handlers_.end();
}

std::vector<std::string> worker_pool::list_handlers() const {
	std::lock_guard<std::mutex> lock(handlers_mutex_);
	std::vector<std::string> names;
	names.reserve(handlers_.size());
	for (const auto& [name, _] : handlers_) {
		names.push_back(name);
	}
	return names;
}

// ============================================================================
// Lifecycle
// ============================================================================

common::VoidResult worker_pool::start() {
	if (running_) {
		return common::VoidResult(common::error_info{-1, "Worker pool is already running"});
	}

	if (!queue_) {
		return common::VoidResult(common::error_info{-1, "Task queue is not set"});
	}

	if (!results_) {
		return common::VoidResult(common::error_info{-1, "Result backend is not set"});
	}

	running_ = true;
	shutdown_requested_ = false;

	// Update statistics
	{
		std::lock_guard<std::mutex> lock(stats_mutex_);
		stats_.started_at = std::chrono::system_clock::now();
	}

	// Create and start worker threads using thread_system
	workers_.reserve(config_.concurrency);
	for (size_t i = 0; i < config_.concurrency; ++i) {
		auto worker = std::make_unique<task_pool_worker>(i, *this);
		auto result = worker->start();
		if (result.is_err()) {
			// Stop already started workers
			for (auto& w : workers_) {
				w->stop();
			}
			workers_.clear();
			running_ = false;
			return common::VoidResult(common::error_info{
				-1, "Failed to start worker " + std::to_string(i)});
		}
		workers_.push_back(std::move(worker));
	}

	return common::ok();
}

common::VoidResult worker_pool::stop() {
	if (!running_) {
		return common::VoidResult(common::error_info{-1, "Worker pool is not running"});
	}

	running_ = false;
	shutdown_requested_ = true;

	// Notify workers to stop
	shutdown_cv_.notify_all();

	// Stop all worker threads (thread_system handles joining)
	for (auto& worker : workers_) {
		worker->stop();
	}
	workers_.clear();

	return common::ok();
}

common::VoidResult worker_pool::shutdown_graceful(std::chrono::milliseconds timeout) {
	if (!running_) {
		return common::VoidResult(common::error_info{-1, "Worker pool is not running"});
	}

	shutdown_requested_ = true;

	// Wait for active workers to finish
	auto deadline = std::chrono::steady_clock::now() + timeout;
	{
		std::unique_lock<std::mutex> lock(shutdown_mutex_);
		while (active_count_ > 0 && std::chrono::steady_clock::now() < deadline) {
			shutdown_cv_.wait_until(lock, deadline);
		}
	}

	// Check if we timed out
	bool timed_out = active_count_ > 0;

	// Stop the pool
	running_ = false;
	shutdown_cv_.notify_all();

	// Stop all worker threads (thread_system handles joining)
	for (auto& worker : workers_) {
		worker->stop();
	}
	workers_.clear();

	if (timed_out) {
		return common::VoidResult(
			common::error_info{-1, "Graceful shutdown timed out with active tasks"});
	}

	return common::ok();
}

// ============================================================================
// Status
// ============================================================================

bool worker_pool::is_running() const {
	return running_;
}

size_t worker_pool::active_workers() const {
	return active_count_.load();
}

size_t worker_pool::idle_workers() const {
	size_t active = active_count_.load();
	size_t total = workers_.size();
	return total > active ? total - active : 0;
}

size_t worker_pool::total_workers() const {
	return workers_.size();
}

// ============================================================================
// Statistics
// ============================================================================

worker_statistics worker_pool::get_statistics() const {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}

void worker_pool::reset_statistics() {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	stats_ = worker_statistics{};
	stats_.started_at = std::chrono::system_clock::now();
}

// ============================================================================
// Private methods
// ============================================================================

bool worker_pool::process_one_task() {
	// Try to dequeue a task (non-blocking with short timeout)
	auto result = queue_->dequeue(config_.queues, config_.poll_interval);

	if (result.is_err()) {
		// No task available or error
		return false;
	}

	auto t = result.unwrap();

	// Check for shutdown request
	if (shutdown_requested_) {
		// Re-enqueue the task if shutting down
		queue_->enqueue(std::move(t));
		return false;
	}

	// Mark as active
	++active_count_;

	// Find handler
	auto handler = find_handler(t.task_name());
	if (!handler) {
		// No handler found, mark task as failed
		t.set_state(task_state::failed);
		t.set_error("No handler registered for task: " + t.task_name());
		results_->store_state(t.task_id(), task_state::failed);
		results_->store_error(t.task_id(), t.error_message());
		--active_count_;
		shutdown_cv_.notify_all();
		return true;  // Task was processed (even though it failed)
	}

	// Create task context
	task_context ctx(t, t.attempt_count() + 1);

	// Set subtask spawner
	ctx.set_subtask_spawner([this](task subtask) -> common::Result<std::string> {
		return queue_->enqueue(std::move(subtask));
	});

	// Execute the task
	auto start_time = std::chrono::steady_clock::now();
	t.set_state(task_state::running);
	t.set_started_at(std::chrono::system_clock::now());
	results_->store_state(t.task_id(), task_state::running);

	auto exec_result = execute_task(t, ctx);
	auto end_time = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		end_time - start_time);

	if (exec_result.is_ok()) {
		// Task succeeded
		t.set_state(task_state::succeeded);
		t.set_completed_at(std::chrono::system_clock::now());
		results_->store_state(t.task_id(), task_state::succeeded);

		// Store result if available
		if (t.has_result()) {
			results_->store_result(t.task_id(), t.result());
		}

		handler->on_success(t, t.has_result() ? t.result() : container_module::value_container{});
		record_task_completed(true, duration);
	} else {
		// Task failed - set state before checking retry eligibility
		auto error = exec_result.error();
		t.set_state(task_state::failed);

		if (t.should_retry()) {
			// Retry the task
			t.increment_attempt();
			t.set_state(task_state::retrying);
			handler->on_retry(t, t.attempt_count());
			results_->store_state(t.task_id(), task_state::retrying);

			// Calculate retry delay
			auto retry_delay = t.get_next_retry_delay();
			auto eta = std::chrono::system_clock::now() + retry_delay;
			t.config().eta = eta;

			queue_->enqueue(std::move(t));
			record_task_retried();
		} else {
			// Mark as failed permanently
			t.set_state(task_state::failed);
			t.set_error(error.message);
			t.set_completed_at(std::chrono::system_clock::now());
			results_->store_state(t.task_id(), task_state::failed);
			results_->store_error(t.task_id(), error.message);

			handler->on_failure(t, error.message);
			record_task_completed(false, duration);
		}
	}

	// Mark as idle
	--active_count_;
	shutdown_cv_.notify_all();
	return true;
}

common::VoidResult worker_pool::execute_task(task& t, task_context& ctx) {
	auto handler = find_handler(t.task_name());
	if (!handler) {
		return common::VoidResult(
			common::error_info{-1, "No handler registered for task: " + t.task_name()});
	}

	try {
		// Check for cancellation before execution
		if (ctx.is_cancelled()) {
			t.set_state(task_state::cancelled);
			results_->store_state(t.task_id(), task_state::cancelled);
			return common::VoidResult(common::error_info{-1, "Task was cancelled"});
		}

		// Get timeout from task config
		auto timeout = t.config().timeout;

		// Execute the handler asynchronously with timeout support
		auto future = std::async(std::launch::async, [&handler, &t, &ctx]() {
			return handler->execute(t, ctx);
		});

		// Wait for result with timeout
		auto status = future.wait_for(timeout);

		if (status == std::future_status::timeout) {
			// Soft timeout: request cancellation so handler can check is_cancelled()
			ctx.request_cancellation();

			// Record timeout in statistics
			record_task_timed_out();

			// Return timeout error
			return common::VoidResult(common::error_info{
				-1, "Task execution timed out after " +
					std::to_string(timeout.count()) + "ms"});
		}

		// Get the result (may throw if handler threw)
		auto result = future.get();

		if (result.is_err()) {
			return common::VoidResult(result.error());
		}

		// Store the result in the task
		auto result_value = result.unwrap();
		t.set_result(std::make_shared<container_module::value_container>(std::move(result_value)));

		// Update progress in result backend
		results_->store_progress(t.task_id(), ctx.progress(), "");

		return common::ok();
	} catch (const std::exception& e) {
		return common::VoidResult(common::error_info{-1, std::string("Exception: ") + e.what()});
	} catch (...) {
		return common::VoidResult(common::error_info{-1, "Unknown exception occurred"});
	}
}

std::shared_ptr<task_handler_interface> worker_pool::find_handler(
	const std::string& task_name) const {
	std::lock_guard<std::mutex> lock(handlers_mutex_);
	auto it = handlers_.find(task_name);
	if (it != handlers_.end()) {
		return it->second;
	}
	return nullptr;
}

void worker_pool::record_task_completed(bool success, std::chrono::milliseconds duration) {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	++stats_.total_tasks_processed;
	if (success) {
		++stats_.total_tasks_succeeded;
	} else {
		++stats_.total_tasks_failed;
	}
	stats_.total_execution_time += duration;
	if (stats_.total_tasks_processed > 0) {
		stats_.avg_execution_time = std::chrono::milliseconds(
			stats_.total_execution_time.count() / static_cast<long long>(stats_.total_tasks_processed));
	}
	stats_.last_task_at = std::chrono::system_clock::now();
}

void worker_pool::record_task_retried() {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	++stats_.total_tasks_retried;
}

void worker_pool::record_task_timed_out() {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	++stats_.total_tasks_timed_out;
}

}  // namespace kcenon::messaging::task
