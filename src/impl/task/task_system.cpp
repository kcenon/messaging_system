// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/task/task_system.h>

#include <kcenon/messaging/error/messaging_error_category.h>

namespace kcenon::messaging::task {

// ============================================================================
// Constructor / Destructor
// ============================================================================

task_system::task_system(task_system_config config)
	: config_(std::move(config)) {
}

task_system::~task_system() {
	if (running_) {
		stop();
	}
}

// ============================================================================
// Lifecycle management
// ============================================================================

common::VoidResult task_system::start() {
	std::lock_guard<std::mutex> lock(init_mutex_);

	if (running_) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::already_running));
	}

	// Initialize components if not already done
	if (!initialized_) {
		auto init_result = initialize();
		if (!init_result.is_ok()) {
			return init_result;
		}
	}

	// Start task queue
	auto queue_result = queue_->start();
	if (!queue_result.is_ok()) {
		return queue_result;
	}

	// Start worker pool
	auto worker_result = workers_->start();
	if (!worker_result.is_ok()) {
		return worker_result;
	}

	// Start scheduler if enabled
	if (scheduler_) {
		auto scheduler_result = scheduler_->start();
		if (!scheduler_result.is_ok()) {
			workers_->stop();
			return scheduler_result;
		}
	}

	running_ = true;
	return common::ok();
}

common::VoidResult task_system::stop() {
	std::lock_guard<std::mutex> lock(init_mutex_);

	if (!running_) {
		return common::ok();
	}

	// Stop scheduler first
	if (scheduler_) {
		scheduler_->stop();
	}

	// Stop worker pool
	if (workers_) {
		workers_->stop();
	}

	// Stop task queue
	if (queue_) {
		queue_->stop();
	}

	running_ = false;
	return common::ok();
}

common::VoidResult task_system::shutdown_graceful(std::chrono::milliseconds timeout) {
	std::lock_guard<std::mutex> lock(init_mutex_);

	if (!running_) {
		return common::ok();
	}

	// Stop scheduler first
	if (scheduler_) {
		scheduler_->stop();
	}

	// Gracefully shutdown worker pool
	if (workers_) {
		auto result = workers_->shutdown_graceful(timeout);
		if (!result.is_ok()) {
			return result;
		}
	}

	// Stop task queue
	if (queue_) {
		queue_->stop();
	}

	running_ = false;
	return common::ok();
}

bool task_system::is_running() const {
	return running_;
}

// ============================================================================
// Component access
// ============================================================================

task_client& task_system::client() {
	if (!initialized_) {
		std::lock_guard<std::mutex> lock(init_mutex_);
		if (!initialized_) {
			initialize();
		}
	}
	return *client_;
}

const task_client& task_system::client() const {
	return const_cast<task_system*>(this)->client();
}

worker_pool& task_system::workers() {
	if (!initialized_) {
		std::lock_guard<std::mutex> lock(init_mutex_);
		if (!initialized_) {
			initialize();
		}
	}
	return *workers_;
}

const worker_pool& task_system::workers() const {
	return const_cast<task_system*>(this)->workers();
}

task_scheduler* task_system::scheduler() {
	if (!config_.enable_scheduler) {
		return nullptr;
	}
	if (!initialized_) {
		std::lock_guard<std::mutex> lock(init_mutex_);
		if (!initialized_) {
			initialize();
		}
	}
	return scheduler_.get();
}

const task_scheduler* task_system::scheduler() const {
	return const_cast<task_system*>(this)->scheduler();
}

task_monitor* task_system::monitor() {
	if (!config_.enable_monitoring) {
		return nullptr;
	}
	if (!initialized_) {
		std::lock_guard<std::mutex> lock(init_mutex_);
		if (!initialized_) {
			initialize();
		}
	}
	return monitor_.get();
}

const task_monitor* task_system::monitor() const {
	return const_cast<task_system*>(this)->monitor();
}

std::shared_ptr<task_queue> task_system::queue() {
	if (!initialized_) {
		std::lock_guard<std::mutex> lock(init_mutex_);
		if (!initialized_) {
			initialize();
		}
	}
	return queue_;
}

std::shared_ptr<result_backend_interface> task_system::results() {
	if (!initialized_) {
		std::lock_guard<std::mutex> lock(init_mutex_);
		if (!initialized_) {
			initialize();
		}
	}
	return results_;
}

// ============================================================================
// Convenience methods - Handler registration
// ============================================================================

void task_system::register_handler(std::shared_ptr<task_handler_interface> handler) {
	workers().register_handler(std::move(handler));
}

void task_system::register_handler(const std::string& name, simple_task_handler handler) {
	workers().register_handler(name, std::move(handler));
}

bool task_system::unregister_handler(const std::string& name) {
	return workers().unregister_handler(name);
}

// ============================================================================
// Convenience methods - Task submission
// ============================================================================

async_result task_system::submit(
	const std::string& task_name,
	const container_module::value_container& payload) {
	return client().send(task_name, payload);
}

async_result task_system::submit(task t) {
	return client().send(std::move(t));
}

async_result task_system::submit_later(task t, std::chrono::milliseconds delay) {
	return client().send_later(std::move(t), delay);
}

std::vector<async_result> task_system::submit_batch(std::vector<task> tasks) {
	return client().send_batch(std::move(tasks));
}

// ============================================================================
// Convenience methods - Scheduling
// ============================================================================

common::VoidResult task_system::schedule_periodic(
	const std::string& name,
	task task_template,
	std::chrono::seconds interval) {
	auto* sched = scheduler();
	if (!sched) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::not_running));
	}
	return sched->add_periodic(name, std::move(task_template), interval);
}

common::VoidResult task_system::schedule_cron(
	const std::string& name,
	task task_template,
	const std::string& cron_expression) {
	auto* sched = scheduler();
	if (!sched) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::not_running));
	}
	return sched->add_cron(name, std::move(task_template), cron_expression);
}

// ============================================================================
// Statistics and status
// ============================================================================

worker_statistics task_system::get_statistics() const {
	if (!initialized_) {
		return worker_statistics{};
	}
	return workers().get_statistics();
}

size_t task_system::pending_count(const std::string& queue_name) const {
	if (!initialized_) {
		return 0;
	}
	return client().pending_count(queue_name);
}

size_t task_system::active_workers() const {
	if (!initialized_) {
		return 0;
	}
	return workers().active_workers();
}

size_t task_system::total_workers() const {
	if (!initialized_) {
		return 0;
	}
	return workers().total_workers();
}

// ============================================================================
// Private methods
// ============================================================================

common::VoidResult task_system::initialize() {
	if (initialized_) {
		return common::ok();
	}

	// Create task queue
	queue_ = std::make_shared<task_queue>(config_.queue);

	// Create result backend
	if (config_.result_backend_type == "memory") {
		results_ = std::make_shared<memory_result_backend>();
	} else {
		// For now, default to memory backend
		// Future: Add Redis backend support
		results_ = std::make_shared<memory_result_backend>();
	}

	// Create worker pool
	workers_ = std::make_unique<worker_pool>(queue_, results_, config_.worker);

	// Create task client
	client_ = std::make_unique<task_client>(queue_, results_);

	// Create scheduler if enabled
	if (config_.enable_scheduler) {
		auto client_ptr = std::make_shared<task_client>(queue_, results_);
		scheduler_ = std::make_unique<task_scheduler>(client_ptr);
	}

	// Create monitor if enabled
	if (config_.enable_monitoring) {
		// Note: We pass workers_ as shared_ptr for monitoring
		// Since monitor doesn't own workers, we create a non-owning shared_ptr
		std::shared_ptr<worker_pool> workers_shared(workers_.get(), [](worker_pool*) {});
		monitor_ = std::make_unique<task_monitor>(queue_, results_, workers_shared);
	}

	initialized_ = true;
	return common::ok();
}

}  // namespace kcenon::messaging::task
