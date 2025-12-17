// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file worker_pool.h
 * @brief Worker pool for distributed task queue system
 *
 * Implements a thread pool that processes tasks from queues using registered
 * handlers. Uses thread_system for thread management instead of direct
 * std::thread usage. Supports multiple queues, handler matching, graceful
 * shutdown, and statistics collection.
 *
 * C++20 Concepts are used for type-safe handler registration, providing
 * clearer compile-time error messages when handler types don't match
 * the expected signature.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_handler.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/result_backend.h>
#include <kcenon/common/patterns/result.h>
#include <kcenon/thread/core/thread_base.h>

#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @struct worker_config
 * @brief Configuration for worker pool
 */
struct worker_config {
	size_t concurrency = std::thread::hardware_concurrency();
	std::vector<std::string> queues = {"default"};
	std::chrono::milliseconds poll_interval{100};
	bool prefetch = true;
	size_t prefetch_count = 10;
};

/**
 * @struct worker_statistics
 * @brief Statistics collected by the worker pool
 */
struct worker_statistics {
	size_t total_tasks_processed = 0;
	size_t total_tasks_succeeded = 0;
	size_t total_tasks_failed = 0;
	size_t total_tasks_retried = 0;
	size_t total_tasks_timed_out = 0;
	std::chrono::milliseconds total_execution_time{0};
	std::chrono::milliseconds avg_execution_time{0};
	std::chrono::system_clock::time_point started_at;
	std::chrono::system_clock::time_point last_task_at;
};

// Forward declaration
class worker_pool;

/**
 * @class task_pool_worker
 * @brief Worker thread that processes tasks using thread_system's thread_base
 *
 * This class inherits from kcenon::thread::thread_base to delegate thread
 * lifecycle management to thread_system. Each worker polls the task queue,
 * matches tasks to handlers, and executes them.
 */
class task_pool_worker : public kcenon::thread::thread_base {
public:
	/**
	 * @brief Construct a task pool worker
	 * @param worker_id Unique identifier for this worker
	 * @param pool Reference to the owning worker_pool
	 */
	task_pool_worker(size_t worker_id, worker_pool& pool);

	~task_pool_worker() override = default;

	// Non-copyable, non-movable
	task_pool_worker(const task_pool_worker&) = delete;
	task_pool_worker& operator=(const task_pool_worker&) = delete;
	task_pool_worker(task_pool_worker&&) = delete;
	task_pool_worker& operator=(task_pool_worker&&) = delete;

	/**
	 * @brief Get the worker ID
	 * @return Worker identifier
	 */
	[[nodiscard]] size_t worker_id() const { return worker_id_; }

protected:
	/**
	 * @brief Determines whether the worker should continue processing
	 * @return true if there may be more work to do
	 */
	[[nodiscard]] auto should_continue_work() const -> bool override;

	/**
	 * @brief Main work routine - processes one task per call
	 * @return common::VoidResult indicating success or failure
	 */
	auto do_work() -> common::VoidResult override;

private:
	size_t worker_id_;
	worker_pool& pool_;
};

/**
 * @class worker_pool
 * @brief Thread pool for executing distributed tasks
 *
 * The worker pool manages a set of worker threads that fetch tasks from
 * queues, match them to registered handlers, and execute them. It handles
 * retries, timeouts, and result storage automatically.
 *
 * Thread management is delegated to thread_system's thread_base class,
 * providing standardized lifecycle management, graceful shutdown, and
 * monitoring capabilities.
 *
 * @example
 * auto queue = std::make_shared<task_queue>();
 * auto results = std::make_shared<memory_result_backend>();
 *
 * worker_config config;
 * config.concurrency = 4;
 * config.queues = {"high-priority", "default"};
 *
 * worker_pool pool(queue, results, config);
 *
 * // Register handlers
 * pool.register_handler("email.send", [](const task& t, task_context& ctx) {
 *     // Send email...
 *     return common::ok(container_module::value_container{});
 * });
 *
 * // Start processing
 * pool.start();
 *
 * // ... later
 * pool.shutdown_graceful(std::chrono::seconds(30));
 */
class worker_pool {
	friend class task_pool_worker;
public:
	/**
	 * @brief Construct a worker pool
	 * @param queue Task queue to fetch tasks from
	 * @param results Result backend to store execution results
	 * @param config Worker configuration
	 */
	worker_pool(
		std::shared_ptr<task_queue> queue,
		std::shared_ptr<result_backend_interface> results,
		worker_config config = {}
	);

	/**
	 * @brief Destructor - stops the pool if running
	 */
	~worker_pool();

	// Non-copyable, non-movable
	worker_pool(const worker_pool&) = delete;
	worker_pool& operator=(const worker_pool&) = delete;
	worker_pool(worker_pool&&) = delete;
	worker_pool& operator=(worker_pool&&) = delete;

	// ========================================================================
	// Handler registration
	// ========================================================================

	/**
	 * @brief Register a task handler
	 * @param handler Shared pointer to handler implementation
	 */
	void register_handler(std::shared_ptr<task_handler_interface> handler);

	/**
	 * @brief Register a lambda task handler
	 * @param name Handler name (matches task_name)
	 * @param handler Lambda function
	 */
	void register_handler(const std::string& name, simple_task_handler handler);

	/**
	 * @brief Register a task handler using C++20 concept constraint
	 *
	 * This overload accepts any callable that satisfies the TaskHandlerCallable
	 * concept, providing better compile-time error messages than std::function.
	 *
	 * @tparam Handler A type satisfying TaskHandlerCallable concept
	 * @param name Handler name (matches task_name)
	 * @param handler Any callable matching the task handler signature
	 *
	 * @example
	 * pool.register_handler("process", [](const task& t, task_context& ctx)
	 *     -> common::Result<container_module::value_container> {
	 *     // Process task...
	 *     return common::ok(container_module::value_container{});
	 * });
	 */
	template<TaskHandlerCallable Handler>
	void register_handler(const std::string& name, Handler&& handler) {
		register_handler(name, simple_task_handler(std::forward<Handler>(handler)));
	}

	/**
	 * @brief Unregister a handler
	 * @param name Handler name to remove
	 * @return true if handler was found and removed
	 */
	bool unregister_handler(const std::string& name);

	/**
	 * @brief Check if a handler is registered
	 * @param name Handler name
	 * @return true if handler exists
	 */
	bool has_handler(const std::string& name) const;

	/**
	 * @brief List registered handler names
	 * @return Vector of handler names
	 */
	std::vector<std::string> list_handlers() const;

	// ========================================================================
	// Lifecycle
	// ========================================================================

	/**
	 * @brief Start the worker pool
	 *
	 * Spawns worker threads and begins processing tasks.
	 *
	 * @return Success or error if already running
	 */
	common::VoidResult start();

	/**
	 * @brief Stop the worker pool immediately
	 *
	 * Signals all workers to stop. Tasks currently being executed
	 * may be interrupted.
	 *
	 * @return Success or error
	 */
	common::VoidResult stop();

	/**
	 * @brief Gracefully shutdown the worker pool
	 *
	 * Waits for currently executing tasks to complete before
	 * stopping workers.
	 *
	 * @param timeout Maximum time to wait for tasks to complete
	 * @return Success or error (timeout if tasks don't complete in time)
	 */
	common::VoidResult shutdown_graceful(std::chrono::milliseconds timeout);

	// ========================================================================
	// Status
	// ========================================================================

	/**
	 * @brief Check if the pool is running
	 * @return true if started and not stopped
	 */
	bool is_running() const;

	/**
	 * @brief Get number of workers currently executing tasks
	 * @return Active worker count
	 */
	size_t active_workers() const;

	/**
	 * @brief Get number of idle workers
	 * @return Idle worker count
	 */
	size_t idle_workers() const;

	/**
	 * @brief Get total number of workers
	 * @return Total worker count (active + idle)
	 */
	size_t total_workers() const;

	// ========================================================================
	// Statistics
	// ========================================================================

	/**
	 * @brief Get worker pool statistics
	 * @return Current statistics
	 */
	worker_statistics get_statistics() const;

	/**
	 * @brief Reset statistics counters
	 */
	void reset_statistics();

private:
	// Process a single task (called by task_pool_worker::do_work)
	bool process_one_task();

	// Task execution
	common::VoidResult execute_task(task& t, task_context& ctx);

	// Find matching handler
	std::shared_ptr<task_handler_interface> find_handler(const std::string& task_name) const;

	// Update statistics
	void record_task_completed(bool success, std::chrono::milliseconds duration);
	void record_task_retried();
	void record_task_timed_out();

	// Configuration
	worker_config config_;

	// Dependencies
	std::shared_ptr<task_queue> queue_;
	std::shared_ptr<result_backend_interface> results_;

	// Handlers (task_name -> handler)
	mutable std::mutex handlers_mutex_;
	std::unordered_map<std::string, std::shared_ptr<task_handler_interface>> handlers_;

	// Worker threads (managed by thread_system)
	std::vector<std::unique_ptr<task_pool_worker>> workers_;

	// Worker state tracking
	mutable std::mutex state_mutex_;
	std::atomic<size_t> active_count_{0};

	// Lifecycle control
	std::atomic<bool> running_{false};
	std::atomic<bool> shutdown_requested_{false};
	std::condition_variable shutdown_cv_;
	mutable std::mutex shutdown_mutex_;

	// Statistics
	mutable std::mutex stats_mutex_;
	worker_statistics stats_;
};

}  // namespace kcenon::messaging::task
