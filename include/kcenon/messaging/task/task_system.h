// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task_system.h
 * @brief Facade integrating all distributed task queue system components
 *
 * Provides a unified interface for the distributed task queue system,
 * combining task queue, worker pool, scheduler, and monitor into a
 * single, easy-to-use API.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_client.h>
#include <kcenon/messaging/task/task_handler.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/worker_pool.h>
#include <kcenon/messaging/task/scheduler.h>
#include <kcenon/messaging/task/monitor.h>
#include <kcenon/messaging/task/result_backend.h>
#include <kcenon/messaging/task/memory_result_backend.h>
#include <kcenon/messaging/task/async_result.h>
#include <kcenon/common/patterns/result.h>

#include <memory>
#include <string>

namespace kcenon::messaging::task {

/**
 * @struct task_system_config
 * @brief Configuration for the task system facade
 *
 * Combines configuration options for all task system components.
 */
struct task_system_config {
	/// Task queue configuration
	task_queue_config queue;

	/// Worker pool configuration
	worker_config worker;

	/// Enable the task scheduler (default: true)
	bool enable_scheduler = true;

	/// Enable the task monitor (default: true)
	bool enable_monitoring = true;

	/// Result backend type: "memory" (default) or "redis"
	std::string result_backend_type = "memory";
};

/**
 * @class task_system
 * @brief Facade integrating all distributed task queue components
 *
 * The task_system provides a unified interface for:
 * - Task submission and tracking
 * - Handler registration
 * - Worker pool management
 * - Task scheduling
 * - System monitoring
 *
 * It manages the lifecycle of all components and provides convenience
 * methods for common operations.
 *
 * Thread Safety:
 * - All public methods are thread-safe
 * - Components are initialized lazily on first access
 *
 * @example
 * task_system_config config;
 * config.worker.concurrency = 4;
 * config.worker.queues = {"default", "high-priority"};
 *
 * task_system system(config);
 *
 * // Register handler
 * system.register_handler("email.send", [](const task& t, task_context& ctx) {
 *     // Process task
 *     ctx.update_progress(0.5, "Sending email...");
 *     container_module::value_container result;
 *     result.add("status", "sent");
 *     return common::ok(result);
 * });
 *
 * // Start system
 * system.start();
 *
 * // Submit task
 * container_module::value_container payload;
 * payload.add("to", "user@example.com");
 * auto result = system.submit("email.send", payload);
 *
 * // Wait for result
 * auto outcome = result.get(std::chrono::seconds(30));
 * if (outcome.is_ok()) {
 *     std::cout << "Email sent successfully!\n";
 * }
 *
 * // Stop system
 * system.stop();
 */
class task_system {
public:
	/**
	 * @brief Construct a task system with configuration
	 * @param config Task system configuration
	 */
	explicit task_system(task_system_config config = {});

	/**
	 * @brief Destructor - stops the system if running
	 */
	~task_system();

	// Non-copyable, non-movable
	task_system(const task_system&) = delete;
	task_system& operator=(const task_system&) = delete;
	task_system(task_system&&) = delete;
	task_system& operator=(task_system&&) = delete;

	// ========================================================================
	// Lifecycle management
	// ========================================================================

	/**
	 * @brief Start the task system
	 *
	 * Initializes and starts all enabled components:
	 * - Task queue
	 * - Worker pool
	 * - Scheduler (if enabled)
	 * - Monitor (if enabled)
	 *
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult start();

	/**
	 * @brief Stop the task system
	 *
	 * Stops all components in reverse order.
	 *
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult stop();

	/**
	 * @brief Gracefully shutdown the task system
	 *
	 * Waits for currently executing tasks to complete before stopping.
	 *
	 * @param timeout Maximum time to wait for tasks to complete
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult shutdown_graceful(
		std::chrono::milliseconds timeout = std::chrono::seconds(30)
	);

	/**
	 * @brief Check if the system is running
	 * @return true if started and not stopped
	 */
	bool is_running() const;

	// ========================================================================
	// Component access
	// ========================================================================

	/**
	 * @brief Get the task client
	 * @return Reference to the task client
	 */
	task_client& client();

	/**
	 * @brief Get the task client (const version)
	 * @return Const reference to the task client
	 */
	const task_client& client() const;

	/**
	 * @brief Get the worker pool
	 * @return Reference to the worker pool
	 */
	worker_pool& workers();

	/**
	 * @brief Get the worker pool (const version)
	 * @return Const reference to the worker pool
	 */
	const worker_pool& workers() const;

	/**
	 * @brief Get the task scheduler
	 * @return Pointer to the scheduler (nullptr if disabled)
	 */
	task_scheduler* scheduler();

	/**
	 * @brief Get the task scheduler (const version)
	 * @return Const pointer to the scheduler (nullptr if disabled)
	 */
	const task_scheduler* scheduler() const;

	/**
	 * @brief Get the task monitor
	 * @return Pointer to the monitor (nullptr if disabled)
	 */
	task_monitor* monitor();

	/**
	 * @brief Get the task monitor (const version)
	 * @return Const pointer to the monitor (nullptr if disabled)
	 */
	const task_monitor* monitor() const;

	/**
	 * @brief Get the task queue
	 * @return Shared pointer to the task queue
	 */
	std::shared_ptr<task_queue> queue();

	/**
	 * @brief Get the result backend
	 * @return Shared pointer to the result backend
	 */
	std::shared_ptr<result_backend_interface> results();

	// ========================================================================
	// Convenience methods - Handler registration
	// ========================================================================

	/**
	 * @brief Register a task handler
	 *
	 * Convenience method that delegates to worker_pool::register_handler.
	 *
	 * @param handler Shared pointer to handler implementation
	 */
	void register_handler(std::shared_ptr<task_handler_interface> handler);

	/**
	 * @brief Register a lambda task handler
	 *
	 * Convenience method that delegates to worker_pool::register_handler.
	 *
	 * @param name Handler name (matches task_name)
	 * @param handler Lambda function
	 */
	void register_handler(const std::string& name, simple_task_handler handler);

	/**
	 * @brief Unregister a handler
	 * @param name Handler name to remove
	 * @return true if handler was found and removed
	 */
	bool unregister_handler(const std::string& name);

	// ========================================================================
	// Convenience methods - Task submission
	// ========================================================================

	/**
	 * @brief Submit a task for immediate execution
	 *
	 * Convenience method that delegates to task_client::send.
	 *
	 * @param task_name Handler name (e.g., "email.send")
	 * @param payload Task payload
	 * @return async_result handle for tracking the task
	 */
	async_result submit(
		const std::string& task_name,
		const container_module::value_container& payload
	);

	/**
	 * @brief Submit a task object for execution
	 *
	 * @param t Task to submit
	 * @return async_result handle for tracking the task
	 */
	async_result submit(task t);

	/**
	 * @brief Submit a task for delayed execution
	 *
	 * @param t Task to submit
	 * @param delay Time to wait before execution
	 * @return async_result handle
	 */
	async_result submit_later(task t, std::chrono::milliseconds delay);

	/**
	 * @brief Submit multiple tasks at once
	 *
	 * @param tasks Vector of tasks to submit
	 * @return Vector of async_result handles
	 */
	std::vector<async_result> submit_batch(std::vector<task> tasks);

	// ========================================================================
	// Convenience methods - Scheduling
	// ========================================================================

	/**
	 * @brief Add a periodic schedule
	 *
	 * Convenience method that delegates to task_scheduler::add_periodic.
	 * Only works if scheduler is enabled.
	 *
	 * @param name Unique schedule identifier
	 * @param task_template Task to execute
	 * @param interval Time between executions
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult schedule_periodic(
		const std::string& name,
		task task_template,
		std::chrono::seconds interval
	);

	/**
	 * @brief Add a cron-based schedule
	 *
	 * Convenience method that delegates to task_scheduler::add_cron.
	 * Only works if scheduler is enabled.
	 *
	 * @param name Unique schedule identifier
	 * @param task_template Task to execute
	 * @param cron_expression Cron expression (e.g., "0 3 * * *")
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult schedule_cron(
		const std::string& name,
		task task_template,
		const std::string& cron_expression
	);

	// ========================================================================
	// Statistics and status
	// ========================================================================

	/**
	 * @brief Get worker pool statistics
	 * @return Worker statistics
	 */
	worker_statistics get_statistics() const;

	/**
	 * @brief Get number of pending tasks
	 * @param queue_name Queue name (default: "default")
	 * @return Number of pending tasks
	 */
	size_t pending_count(const std::string& queue_name = "default") const;

	/**
	 * @brief Get number of active workers
	 * @return Number of workers currently executing tasks
	 */
	size_t active_workers() const;

	/**
	 * @brief Get total number of workers
	 * @return Total worker count
	 */
	size_t total_workers() const;

private:
	/**
	 * @brief Initialize all components
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult initialize();

	/// Configuration
	task_system_config config_;

	/// Running state
	std::atomic<bool> running_{false};
	std::atomic<bool> initialized_{false};

	/// Core components
	std::shared_ptr<task_queue> queue_;
	std::shared_ptr<result_backend_interface> results_;
	std::unique_ptr<worker_pool> workers_;
	std::unique_ptr<task_client> client_;

	/// Optional components
	std::unique_ptr<task_scheduler> scheduler_;
	std::unique_ptr<task_monitor> monitor_;

	/// Initialization mutex
	mutable std::mutex init_mutex_;
};

}  // namespace kcenon::messaging::task
