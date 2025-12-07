// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file monitor.h
 * @brief Task monitoring for distributed task queue system
 *
 * Implements queue/worker/task status monitoring with event subscriptions.
 * Provides real-time insights into task queue health and worker status.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/result_backend.h>
#include <kcenon/messaging/task/worker_pool.h>
#include <kcenon/common/patterns/result.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @struct queue_stats
 * @brief Statistics for a single task queue
 */
struct queue_stats {
	std::string name;
	size_t pending_count = 0;
	size_t running_count = 0;
	size_t delayed_count = 0;
};

/**
 * @struct worker_info
 * @brief Information about a worker
 */
struct worker_info {
	std::string worker_id;
	std::vector<std::string> queues;
	size_t active_tasks = 0;
	std::chrono::system_clock::time_point last_heartbeat;
	bool is_healthy = true;
};

/**
 * @class task_monitor
 * @brief Monitor for task queue system status and events
 *
 * The task_monitor provides real-time monitoring of queue statistics,
 * worker status, and task states. It supports event subscriptions for
 * task lifecycle events.
 *
 * @example
 * auto queue = std::make_shared<task_queue>();
 * auto results = std::make_shared<memory_result_backend>();
 * auto workers = std::make_shared<worker_pool>(queue, results);
 *
 * task_monitor monitor(queue, results, workers);
 *
 * // Get queue statistics
 * auto stats = monitor.get_queue_stats();
 * for (const auto& stat : stats) {
 *     std::cout << stat.name << ": " << stat.pending_count << " pending\n";
 * }
 *
 * // Subscribe to task events
 * monitor.on_task_completed([](const task& t, bool success) {
 *     std::cout << "Task " << t.task_id() << " completed: "
 *               << (success ? "success" : "failed") << "\n";
 * });
 */
class task_monitor {
public:
	/**
	 * @brief Construct a task monitor
	 * @param queue Task queue to monitor
	 * @param results Result backend for task state queries
	 * @param workers Optional worker pool for worker status (can be nullptr)
	 */
	task_monitor(
		std::shared_ptr<task_queue> queue,
		std::shared_ptr<result_backend_interface> results,
		std::shared_ptr<worker_pool> workers = nullptr
	);

	/**
	 * @brief Destructor
	 */
	~task_monitor();

	// Non-copyable, non-movable
	task_monitor(const task_monitor&) = delete;
	task_monitor& operator=(const task_monitor&) = delete;
	task_monitor(task_monitor&&) = delete;
	task_monitor& operator=(task_monitor&&) = delete;

	// ========================================================================
	// Queue statistics
	// ========================================================================

	/**
	 * @brief Get statistics for all queues
	 * @return Vector of queue statistics
	 */
	std::vector<queue_stats> get_queue_stats() const;

	/**
	 * @brief Get statistics for a specific queue
	 * @param queue_name Name of the queue
	 * @return Queue statistics or error if queue not found
	 */
	common::Result<queue_stats> get_queue_stats(const std::string& queue_name) const;

	// ========================================================================
	// Worker status
	// ========================================================================

	/**
	 * @brief Get information about all workers
	 * @return Vector of worker information
	 */
	std::vector<worker_info> get_workers() const;

	/**
	 * @brief Get overall worker pool statistics
	 * @return Worker statistics from the worker pool
	 */
	std::optional<worker_statistics> get_worker_statistics() const;

	// ========================================================================
	// Task queries
	// ========================================================================

	/**
	 * @brief List currently running tasks
	 * @return Vector of active tasks
	 */
	std::vector<task> list_active_tasks() const;

	/**
	 * @brief List pending tasks in a queue
	 * @param queue_name Queue name (default: "default")
	 * @return Vector of pending tasks
	 */
	std::vector<task> list_pending_tasks(const std::string& queue_name = "default") const;

	/**
	 * @brief List failed tasks
	 * @param limit Maximum number of tasks to return
	 * @return Vector of failed tasks
	 */
	std::vector<task> list_failed_tasks(size_t limit = 100) const;

	// ========================================================================
	// Task management
	// ========================================================================

	/**
	 * @brief Cancel a task
	 * @param task_id ID of the task to cancel
	 * @return Success or error
	 */
	common::VoidResult cancel_task(const std::string& task_id);

	/**
	 * @brief Retry a failed task
	 * @param task_id ID of the task to retry
	 * @return Success or error
	 */
	common::VoidResult retry_task(const std::string& task_id);

	/**
	 * @brief Purge all tasks from a queue
	 * @param queue_name Name of the queue to purge
	 * @return Success or error
	 */
	common::VoidResult purge_queue(const std::string& queue_name);

	// ========================================================================
	// Event subscription
	// ========================================================================

	/// Callback for task started events
	using task_started_handler = std::function<void(const task&)>;
	/// Callback for task completed events
	using task_completed_handler = std::function<void(const task&, bool success)>;
	/// Callback for task failed events
	using task_failed_handler = std::function<void(const task&, const std::string& error)>;
	/// Callback for worker offline events
	using worker_offline_handler = std::function<void(const std::string& worker_id)>;

	/**
	 * @brief Subscribe to task started events
	 * @param handler Callback function
	 */
	void on_task_started(task_started_handler handler);

	/**
	 * @brief Subscribe to task completed events
	 * @param handler Callback function
	 */
	void on_task_completed(task_completed_handler handler);

	/**
	 * @brief Subscribe to task failed events
	 * @param handler Callback function
	 */
	void on_task_failed(task_failed_handler handler);

	/**
	 * @brief Subscribe to worker offline events
	 * @param handler Callback function
	 */
	void on_worker_offline(worker_offline_handler handler);

	// ========================================================================
	// Event notification (internal use)
	// ========================================================================

	/**
	 * @brief Notify that a task has started
	 * @param t The task that started
	 */
	void notify_task_started(const task& t);

	/**
	 * @brief Notify that a task has completed
	 * @param t The task that completed
	 * @param success Whether the task succeeded
	 */
	void notify_task_completed(const task& t, bool success);

	/**
	 * @brief Notify that a task has failed
	 * @param t The task that failed
	 * @param error Error message
	 */
	void notify_task_failed(const task& t, const std::string& error);

	/**
	 * @brief Notify that a worker went offline
	 * @param worker_id ID of the worker that went offline
	 */
	void notify_worker_offline(const std::string& worker_id);

private:
	// Dependencies
	std::shared_ptr<task_queue> queue_;
	std::shared_ptr<result_backend_interface> results_;
	std::shared_ptr<worker_pool> workers_;

	// Event handlers
	mutable std::mutex handlers_mutex_;
	std::vector<task_started_handler> started_handlers_;
	std::vector<task_completed_handler> completed_handlers_;
	std::vector<task_failed_handler> failed_handlers_;
	std::vector<worker_offline_handler> offline_handlers_;

	// Active task tracking
	mutable std::mutex active_mutex_;
	std::vector<task> active_tasks_;

	// Failed task tracking
	mutable std::mutex failed_mutex_;
	std::vector<task> failed_tasks_;
	size_t max_failed_tasks_ = 1000;
};

}  // namespace kcenon::messaging::task
