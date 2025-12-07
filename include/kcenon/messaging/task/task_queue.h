// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task_queue.h
 * @brief Task queue for distributed task queue system
 *
 * Implements a task-specific queue that extends the existing message_queue
 * with support for multiple named queues, delayed execution, and tag-based
 * cancellation.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/core/message_queue.h>
#include <kcenon/common/patterns/result.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @struct task_queue_config
 * @brief Configuration for task queue
 */
struct task_queue_config {
	size_t max_size = 100000;
	bool enable_persistence = false;
	std::string persistence_path;
	bool enable_delayed_queue = true;
	std::chrono::milliseconds delayed_poll_interval{1000};
};

/**
 * @struct delayed_task
 * @brief Internal structure for delayed task scheduling
 */
struct delayed_task {
	task t;
	std::chrono::system_clock::time_point eta;

	bool operator<(const delayed_task& other) const {
		// Priority queue is max-heap, so invert comparison for earliest first
		return eta > other.eta;
	}
};

/**
 * @class task_queue
 * @brief Task queue with multiple named queues and delayed execution support
 *
 * The task_queue manages multiple independent message queues, each identified
 * by a queue name. It supports priority-based ordering, delayed task execution
 * using ETA scheduling, and tag-based task cancellation.
 *
 * @example
 * task_queue_config config;
 * config.max_size = 10000;
 * task_queue queue(config);
 *
 * // Start the queue
 * queue.start();
 *
 * // Enqueue a task
 * auto t = task_builder("email.send")
 *     .payload(payload)
 *     .queue("high-priority")
 *     .build()
 *     .unwrap();
 * queue.enqueue(std::move(t));
 *
 * // Dequeue from multiple queues
 * auto result = queue.dequeue({"high-priority", "default"}, std::chrono::seconds(5));
 */
class task_queue {
public:
	/**
	 * @brief Construct a task queue with configuration
	 * @param config Queue configuration
	 */
	explicit task_queue(task_queue_config config = {});

	/**
	 * @brief Destructor - stops the queue
	 */
	~task_queue();

	// Non-copyable
	task_queue(const task_queue&) = delete;
	task_queue& operator=(const task_queue&) = delete;

	// Movable
	task_queue(task_queue&&) noexcept;
	task_queue& operator=(task_queue&&) noexcept;

	// ========================================================================
	// Lifecycle
	// ========================================================================

	/**
	 * @brief Start the task queue
	 *
	 * Starts the delayed task processing thread if enabled.
	 *
	 * @return Success or error
	 */
	common::VoidResult start();

	/**
	 * @brief Stop the task queue
	 *
	 * Stops all queue operations and the delayed task thread.
	 */
	void stop();

	/**
	 * @brief Check if the queue is running
	 * @return true if started and not stopped
	 */
	bool is_running() const;

	// ========================================================================
	// Enqueue operations
	// ========================================================================

	/**
	 * @brief Enqueue a single task
	 *
	 * If the task has an ETA in the future, it will be added to the delayed
	 * queue and processed when the ETA is reached.
	 *
	 * @param t Task to enqueue
	 * @return Task ID on success, or error
	 */
	common::Result<std::string> enqueue(task t);

	/**
	 * @brief Enqueue multiple tasks
	 *
	 * Enqueues all tasks atomically. If any task fails to enqueue,
	 * the operation continues with remaining tasks.
	 *
	 * @param tasks Vector of tasks to enqueue
	 * @return Vector of task IDs (only successful ones)
	 */
	common::Result<std::vector<std::string>> enqueue_bulk(std::vector<task> tasks);

	// ========================================================================
	// Dequeue operations
	// ========================================================================

	/**
	 * @brief Dequeue a task from the specified queues
	 *
	 * Attempts to dequeue a task from the specified queues in order.
	 * The first queue with an available task will be used.
	 *
	 * @param queue_names List of queue names to check (in priority order)
	 * @param timeout Maximum time to wait for a task
	 * @return Task on success, or error (queue_empty on timeout)
	 */
	common::Result<task> dequeue(
		const std::vector<std::string>& queue_names,
		std::chrono::milliseconds timeout);

	/**
	 * @brief Try to dequeue without waiting
	 *
	 * @param queue_names List of queue names to check
	 * @return Task if available, or error
	 */
	common::Result<task> try_dequeue(const std::vector<std::string>& queue_names);

	// ========================================================================
	// Cancellation
	// ========================================================================

	/**
	 * @brief Cancel a task by ID
	 *
	 * Marks the task as cancelled. If the task is in a queue, it will
	 * be skipped when dequeued.
	 *
	 * @param task_id Task ID to cancel
	 * @return Success or error (task_not_found if not found)
	 */
	common::VoidResult cancel(const std::string& task_id);

	/**
	 * @brief Cancel all tasks with a specific tag
	 *
	 * @param tag Tag to match
	 * @return Success (always succeeds, even if no tasks matched)
	 */
	common::VoidResult cancel_by_tag(const std::string& tag);

	// ========================================================================
	// Query operations
	// ========================================================================

	/**
	 * @brief Get a task by ID
	 *
	 * Returns a copy of the task if found.
	 *
	 * @param task_id Task ID
	 * @return Task or error (task_not_found if not found)
	 */
	common::Result<task> get_task(const std::string& task_id) const;

	/**
	 * @brief Get the size of a specific queue
	 *
	 * @param queue_name Queue name
	 * @return Number of tasks in the queue (0 if queue doesn't exist)
	 */
	size_t queue_size(const std::string& queue_name) const;

	/**
	 * @brief Get total number of tasks across all queues
	 * @return Total task count
	 */
	size_t total_size() const;

	/**
	 * @brief Get number of delayed tasks
	 * @return Number of tasks waiting for ETA
	 */
	size_t delayed_size() const;

	/**
	 * @brief List all queue names
	 * @return Vector of queue names
	 */
	std::vector<std::string> list_queues() const;

	/**
	 * @brief Check if a queue exists
	 * @param queue_name Queue name
	 * @return true if queue exists
	 */
	bool has_queue(const std::string& queue_name) const;

private:
	// Internal helpers
	void ensure_queue_exists(const std::string& queue_name);
	void process_delayed_tasks();
	void delayed_task_worker();
	bool is_task_cancelled(const std::string& task_id) const;
	void register_task(const task& t);
	void unregister_task(const std::string& task_id);

	task_queue_config config_;

	// Per-queue message queues (queue_name -> message_queue)
	mutable std::mutex queues_mutex_;
	std::unordered_map<std::string, std::unique_ptr<message_queue>> queues_;

	// Delayed tasks
	mutable std::mutex delayed_mutex_;
	std::priority_queue<delayed_task> delayed_queue_;

	// Task registry (task_id -> task metadata)
	mutable std::mutex registry_mutex_;
	std::unordered_map<std::string, task> task_registry_;

	// Cancelled task IDs
	mutable std::mutex cancelled_mutex_;
	std::unordered_set<std::string> cancelled_tasks_;

	// Tag to task ID mapping for tag-based cancellation
	mutable std::mutex tags_mutex_;
	std::unordered_map<std::string, std::unordered_set<std::string>> tag_to_tasks_;

	// Lifecycle
	std::atomic<bool> running_{false};
	std::atomic<bool> stopped_{false};
	std::thread delayed_worker_thread_;
	std::condition_variable delayed_cv_;
};

}  // namespace kcenon::messaging::task
