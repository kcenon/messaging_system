// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task_client.h
 * @brief Client interface for task creation and submission
 *
 * Provides a high-level API for submitting tasks to the distributed task queue.
 * Supports immediate execution, delayed execution, batch operations, and
 * workflow patterns (chain, chord).
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/async_result.h>
#include <kcenon/messaging/task/result_backend.h>
#include <kcenon/common/patterns/result.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @class task_client
 * @brief Client for submitting tasks to the distributed task queue
 *
 * The task_client provides methods for:
 * - Immediate task execution
 * - Delayed/scheduled task execution
 * - Batch task submission
 * - Workflow patterns (chain, chord)
 *
 * Thread Safety:
 * - All methods are thread-safe
 * - Multiple threads can safely submit tasks concurrently
 *
 * @example
 * task_client client(queue, results);
 *
 * // Single task
 * auto result = client.send("process.data", {{"file", "data.csv"}});
 *
 * // Execute after 5 minutes
 * auto delayed = client.send_later(
 *     task_builder("cleanup.temp").build().value(),
 *     std::chrono::minutes(5)
 * );
 *
 * // Batch execution
 * auto batch_results = client.send_batch({task1, task2, task3});
 *
 * // Sequential execution (chain)
 * auto chain_result = client.chain({
 *     task_builder("step1").build().value(),
 *     task_builder("step2").build().value(),
 *     task_builder("step3").build().value()
 * });
 */
class task_client {
public:
	/**
	 * @brief Construct a task client
	 * @param queue Shared pointer to the task queue
	 * @param results Shared pointer to the result backend
	 */
	task_client(
		std::shared_ptr<task_queue> queue,
		std::shared_ptr<result_backend_interface> results
	);

	/**
	 * @brief Default destructor
	 */
	~task_client() = default;

	// Copy and move
	task_client(const task_client&) = default;
	task_client& operator=(const task_client&) = default;
	task_client(task_client&&) noexcept = default;
	task_client& operator=(task_client&&) noexcept = default;

	// ========================================================================
	// Immediate execution
	// ========================================================================

	/**
	 * @brief Submit a task for immediate execution
	 * @param t Task to submit
	 * @return async_result handle for tracking the task
	 */
	async_result send(task t);

	/**
	 * @brief Submit a task by name and payload
	 * @param task_name Handler name (e.g., "email.send")
	 * @param payload Task payload
	 * @return async_result handle for tracking the task
	 */
	async_result send(
		const std::string& task_name,
		const container_module::value_container& payload
	);

	/**
	 * @brief Submit a task by name with shared payload
	 * @param task_name Handler name
	 * @param payload Shared pointer to task payload
	 * @return async_result handle
	 */
	async_result send(
		const std::string& task_name,
		std::shared_ptr<container_module::value_container> payload
	);

	// ========================================================================
	// Delayed/Scheduled execution
	// ========================================================================

	/**
	 * @brief Submit a task for delayed execution
	 * @param t Task to submit
	 * @param delay Time to wait before execution
	 * @return async_result handle
	 */
	async_result send_later(task t, std::chrono::milliseconds delay);

	/**
	 * @brief Submit a task for execution at a specific time
	 * @param t Task to submit
	 * @param eta Execution time
	 * @return async_result handle
	 */
	async_result send_at(task t, std::chrono::system_clock::time_point eta);

	// ========================================================================
	// Batch execution
	// ========================================================================

	/**
	 * @brief Submit multiple tasks at once
	 * @param tasks Vector of tasks to submit
	 * @return Vector of async_result handles
	 */
	std::vector<async_result> send_batch(std::vector<task> tasks);

	// ========================================================================
	// Workflow patterns
	// ========================================================================

	/**
	 * @brief Execute tasks sequentially (chain pattern)
	 *
	 * Each task's result is passed as input to the next task.
	 * If any task fails, the chain stops and the error is propagated.
	 *
	 * @param tasks Vector of tasks to execute in sequence
	 * @return async_result for the final task in the chain
	 */
	async_result chain(std::vector<task> tasks);

	/**
	 * @brief Execute tasks in parallel with aggregation (chord pattern)
	 *
	 * All tasks execute in parallel. When all complete, the callback
	 * task is executed with all results aggregated.
	 *
	 * @param tasks Vector of tasks to execute in parallel
	 * @param callback Task to execute after all parallel tasks complete
	 * @return async_result for the callback task
	 */
	async_result chord(std::vector<task> tasks, task callback);

	// ========================================================================
	// Result retrieval
	// ========================================================================

	/**
	 * @brief Get result handle for an existing task
	 * @param task_id Task ID to look up
	 * @return async_result handle for the task
	 */
	async_result get_result(const std::string& task_id);

	// ========================================================================
	// Task cancellation
	// ========================================================================

	/**
	 * @brief Cancel a task by ID
	 * @param task_id Task ID to cancel
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult cancel(const std::string& task_id);

	/**
	 * @brief Cancel all tasks with a specific tag
	 * @param tag Tag to match
	 * @return VoidResult indicating success
	 */
	common::VoidResult cancel_by_tag(const std::string& tag);

	// ========================================================================
	// Queue information
	// ========================================================================

	/**
	 * @brief Get the number of pending tasks in a queue
	 * @param queue_name Queue name (default: "default")
	 * @return Number of pending tasks
	 */
	size_t pending_count(const std::string& queue_name = "default") const;

	/**
	 * @brief Check if the client is connected and operational
	 * @return true if queue and backend are valid
	 */
	bool is_connected() const;

private:
	// Internal helper for submitting a task
	async_result submit_task(task& t);

	// Helper to create chain/chord task IDs
	static std::string generate_workflow_id();

	std::shared_ptr<task_queue> queue_;
	std::shared_ptr<result_backend_interface> results_;
};

}  // namespace kcenon::messaging::task
