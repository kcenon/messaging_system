// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file result_backend.h
 * @brief Abstract interface for storing and retrieving task execution results
 *
 * Provides a backend interface for the distributed task queue system
 * to store task states, results, errors, and progress information.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/common/patterns/result.h>

#include <chrono>
#include <memory>
#include <string>

namespace kcenon::messaging::task {

/**
 * @struct progress_data
 * @brief Progress information stored in result backend
 */
struct progress_data {
	double progress{0.0};      // Progress value 0.0 to 1.0
	std::string message;       // Optional progress message
	std::chrono::system_clock::time_point updated_at;
};

/**
 * @struct error_data
 * @brief Error information stored in result backend
 */
struct error_data {
	std::string message;     // Error message
	std::string traceback;   // Stack trace or additional details
	std::chrono::system_clock::time_point occurred_at;
};

/**
 * @interface result_backend_interface
 * @brief Abstract interface for task result storage and retrieval
 *
 * The result backend is responsible for:
 * - Storing task states throughout the lifecycle
 * - Storing task execution results
 * - Storing error information on failure
 * - Tracking task progress
 * - Providing synchronous waiting for results
 *
 * Implementations may use different storage backends:
 * - Memory (default, for single-process environments)
 * - Redis (for distributed environments)
 * - Database (for persistence)
 *
 * Thread Safety:
 * - All methods must be thread-safe
 * - Implementations must handle concurrent access from multiple workers
 *
 * @example
 * // Store task progress
 * backend->store_progress("task-123", 0.5, "Processing 50%");
 *
 * // Store successful result
 * backend->store_state("task-123", task_state::succeeded);
 * backend->store_result("task-123", result_container);
 *
 * // Wait for result
 * auto result = backend->wait_for_result("task-123", std::chrono::seconds(30));
 */
class result_backend_interface {
public:
	virtual ~result_backend_interface() = default;

	// ========================================================================
	// State storage
	// ========================================================================

	/**
	 * @brief Store task state
	 *
	 * Updates the current state of a task. This should be called whenever
	 * the task transitions to a new state.
	 *
	 * @param task_id Unique task identifier
	 * @param state New task state
	 * @return VoidResult indicating success or error
	 */
	virtual common::VoidResult store_state(
		const std::string& task_id,
		task_state state
	) = 0;

	// ========================================================================
	// Result storage
	// ========================================================================

	/**
	 * @brief Store task execution result
	 *
	 * Stores the successful result of a task execution. This should be
	 * called after the task completes successfully.
	 *
	 * @param task_id Unique task identifier
	 * @param result Result data as value_container
	 * @return VoidResult indicating success or error
	 */
	virtual common::VoidResult store_result(
		const std::string& task_id,
		const container_module::value_container& result
	) = 0;

	// ========================================================================
	// Error storage
	// ========================================================================

	/**
	 * @brief Store task error information
	 *
	 * Stores error details when a task fails. This should be called
	 * when a task execution encounters an error.
	 *
	 * @param task_id Unique task identifier
	 * @param error Error message
	 * @param traceback Optional stack trace or additional details
	 * @return VoidResult indicating success or error
	 */
	virtual common::VoidResult store_error(
		const std::string& task_id,
		const std::string& error,
		const std::string& traceback = ""
	) = 0;

	// ========================================================================
	// Progress storage
	// ========================================================================

	/**
	 * @brief Store task progress
	 *
	 * Updates the progress of a running task. This allows tracking
	 * long-running tasks and displaying progress to users.
	 *
	 * @param task_id Unique task identifier
	 * @param progress Progress value between 0.0 and 1.0
	 * @param message Optional progress message
	 * @return VoidResult indicating success or error
	 */
	virtual common::VoidResult store_progress(
		const std::string& task_id,
		double progress,
		const std::string& message = ""
	) = 0;

	// ========================================================================
	// Query methods
	// ========================================================================

	/**
	 * @brief Get current task state
	 *
	 * @param task_id Unique task identifier
	 * @return Result containing task_state or error if task not found
	 */
	virtual common::Result<task_state> get_state(
		const std::string& task_id
	) = 0;

	/**
	 * @brief Get task execution result
	 *
	 * Returns the stored result for a completed task.
	 *
	 * @param task_id Unique task identifier
	 * @return Result containing value_container or error if not available
	 */
	virtual common::Result<container_module::value_container> get_result(
		const std::string& task_id
	) = 0;

	/**
	 * @brief Get task progress
	 *
	 * @param task_id Unique task identifier
	 * @return Result containing progress_data or error if task not found
	 */
	virtual common::Result<progress_data> get_progress(
		const std::string& task_id
	) = 0;

	/**
	 * @brief Get task error information
	 *
	 * @param task_id Unique task identifier
	 * @return Result containing error_data or error if no error stored
	 */
	virtual common::Result<error_data> get_error(
		const std::string& task_id
	) = 0;

	// ========================================================================
	// Blocking operations
	// ========================================================================

	/**
	 * @brief Wait for task result with timeout
	 *
	 * Blocks until the task completes (succeeded or failed) or timeout.
	 * This is the primary method for synchronously waiting for task results.
	 *
	 * @param task_id Unique task identifier
	 * @param timeout Maximum time to wait
	 * @return Result containing value_container on success,
	 *         error if task failed, timed out, or was cancelled
	 */
	virtual common::Result<container_module::value_container> wait_for_result(
		const std::string& task_id,
		std::chrono::milliseconds timeout
	) = 0;

	// ========================================================================
	// Cleanup
	// ========================================================================

	/**
	 * @brief Clean up expired task data
	 *
	 * Removes task data older than the specified age. This helps prevent
	 * unbounded memory/storage growth.
	 *
	 * @param max_age Maximum age of data to keep
	 * @return VoidResult indicating success or error
	 */
	virtual common::VoidResult cleanup_expired(
		std::chrono::milliseconds max_age
	) = 0;

	// ========================================================================
	// Optional methods with default implementations
	// ========================================================================

	/**
	 * @brief Check if a task exists in the backend
	 *
	 * @param task_id Unique task identifier
	 * @return true if task exists, false otherwise
	 */
	virtual bool exists(const std::string& task_id) {
		return get_state(task_id).is_ok();
	}

	/**
	 * @brief Delete task data
	 *
	 * Removes all stored data for a specific task.
	 *
	 * @param task_id Unique task identifier
	 * @return VoidResult indicating success or error
	 */
	virtual common::VoidResult remove(const std::string& task_id) {
		return common::ok();  // Default: no-op
	}

	/**
	 * @brief Get the number of stored tasks
	 *
	 * @return Number of tasks in the backend
	 */
	virtual size_t size() const {
		return 0;  // Default: unknown
	}
};

}  // namespace kcenon::messaging::task
