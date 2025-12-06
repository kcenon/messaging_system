// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task_context.h
 * @brief Task execution context for distributed task queue system
 *
 * Provides an execution context that handlers receive during task execution.
 * The context allows handlers to update progress, save checkpoints,
 * spawn subtasks, and check for cancellation.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/common/patterns/result.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @struct progress_info
 * @brief Progress update information
 */
struct progress_info {
	double progress{0.0};       // Progress value 0.0 to 1.0
	std::string message;        // Optional progress message
	std::chrono::system_clock::time_point timestamp;
};

/**
 * @struct task_log_entry
 * @brief Log entry created during task execution
 */
struct task_log_entry {
	enum class level { info, warning, error };

	level log_level;
	std::string message;
	std::chrono::system_clock::time_point timestamp;
};

/**
 * @class task_context
 * @brief Execution context provided to handlers during task execution
 *
 * The task context provides utilities for:
 * - Progress tracking and reporting
 * - Checkpoint save/restore for long-running tasks
 * - Subtask spawning for workflow patterns
 * - Cancellation checking
 * - Task-specific logging
 *
 * @example
 * common::Result<value_container> handler(const task& t, task_context& ctx) {
 *     ctx.update_progress(0.0, "Starting...");
 *
 *     // Check for previous checkpoint
 *     auto checkpoint = ctx.load_checkpoint();
 *     int start = checkpoint.get_int_or("step", 0);
 *
 *     for (int i = start; i < 100; ++i) {
 *         if (ctx.is_cancelled()) {
 *             return error("Task cancelled");
 *         }
 *
 *         // Do work...
 *
 *         ctx.update_progress(i / 100.0, "Processing step " + std::to_string(i));
 *         ctx.save_checkpoint({{"step", i}});
 *     }
 *
 *     ctx.log_info("Task completed successfully");
 *     return ok(result);
 * }
 */
class task_context {
public:
	/**
	 * @brief Callback type for subtask spawning
	 *
	 * Returns the task ID of the spawned subtask on success
	 */
	using subtask_spawner = std::function<common::Result<std::string>(task)>;

	/**
	 * @brief Construct a task context
	 * @param current_task Reference to the task being executed
	 * @param attempt Current attempt number (1-based)
	 */
	explicit task_context(task& current_task, size_t attempt = 1);

	~task_context() = default;

	// Non-copyable, non-movable (context is bound to specific execution)
	task_context(const task_context&) = delete;
	task_context& operator=(const task_context&) = delete;
	task_context(task_context&&) = delete;
	task_context& operator=(task_context&&) = delete;

	// ========================================================================
	// Progress tracking
	// ========================================================================

	/**
	 * @brief Update task progress
	 *
	 * Updates the progress value and optional message. Progress is clamped
	 * to the range [0.0, 1.0].
	 *
	 * @param progress Progress value between 0.0 and 1.0
	 * @param message Optional progress message
	 */
	void update_progress(double progress, const std::string& message = "");

	/**
	 * @brief Get current progress value
	 * @return Progress value between 0.0 and 1.0
	 */
	double progress() const;

	/**
	 * @brief Get progress history
	 * @return Vector of progress updates
	 */
	std::vector<progress_info> progress_history() const;

	// ========================================================================
	// Checkpoint management (for recovery on retry)
	// ========================================================================

	/**
	 * @brief Save a checkpoint for recovery
	 *
	 * Saves the current state so that if the task fails and is retried,
	 * it can resume from this checkpoint instead of starting over.
	 *
	 * @param state The state to save
	 */
	void save_checkpoint(const container_module::value_container& state);

	/**
	 * @brief Save a checkpoint using a shared pointer
	 * @param state Shared pointer to the state
	 */
	void save_checkpoint(std::shared_ptr<container_module::value_container> state);

	/**
	 * @brief Load the saved checkpoint
	 *
	 * Returns the previously saved checkpoint state, or an empty container
	 * if no checkpoint was saved.
	 *
	 * @return The checkpoint state
	 */
	container_module::value_container load_checkpoint() const;

	/**
	 * @brief Check if a checkpoint exists
	 * @return true if a checkpoint was saved
	 */
	bool has_checkpoint() const;

	/**
	 * @brief Clear the checkpoint
	 */
	void clear_checkpoint();

	// ========================================================================
	// Subtask management
	// ========================================================================

	/**
	 * @brief Set the subtask spawner callback
	 *
	 * This is called by the worker pool to provide the mechanism for
	 * spawning subtasks.
	 *
	 * @param spawner The callback function
	 */
	void set_subtask_spawner(subtask_spawner spawner);

	/**
	 * @brief Spawn a subtask
	 *
	 * Creates a new task that will be executed by the worker pool.
	 * The subtask is independent of the parent task.
	 *
	 * @param subtask The task to spawn
	 * @return Result containing the subtask ID or error
	 */
	common::Result<std::string> spawn_subtask(task subtask);

	/**
	 * @brief Get IDs of spawned subtasks
	 * @return Vector of subtask IDs
	 */
	std::vector<std::string> spawned_subtask_ids() const;

	// ========================================================================
	// Cancellation
	// ========================================================================

	/**
	 * @brief Check if the task has been cancelled
	 *
	 * Handlers should periodically check this and return early if true.
	 *
	 * @return true if the task was cancelled
	 */
	bool is_cancelled() const;

	/**
	 * @brief Request task cancellation
	 *
	 * Called by the worker pool to signal cancellation.
	 */
	void request_cancellation();

	// ========================================================================
	// Logging
	// ========================================================================

	/**
	 * @brief Log an info message
	 * @param message The message to log
	 */
	void log_info(const std::string& message);

	/**
	 * @brief Log a warning message
	 * @param message The message to log
	 */
	void log_warning(const std::string& message);

	/**
	 * @brief Log an error message
	 * @param message The message to log
	 */
	void log_error(const std::string& message);

	/**
	 * @brief Get all log entries
	 * @return Vector of log entries
	 */
	std::vector<task_log_entry> logs() const;

	// ========================================================================
	// Task information
	// ========================================================================

	/**
	 * @brief Get the current task being executed
	 * @return Reference to the current task
	 */
	const task& current_task() const;

	/**
	 * @brief Get the current attempt number
	 * @return Attempt number (1-based)
	 */
	size_t attempt_number() const;

	/**
	 * @brief Get the execution start time
	 * @return Time point when execution started
	 */
	std::chrono::system_clock::time_point started_at() const;

	/**
	 * @brief Get elapsed execution time
	 * @return Duration since execution started
	 */
	std::chrono::milliseconds elapsed() const;

private:
	void add_log(task_log_entry::level level, const std::string& message);

	task& task_;
	size_t attempt_;
	std::chrono::system_clock::time_point started_at_;

	// Thread-safe state
	std::atomic<bool> cancelled_{false};
	std::atomic<double> progress_{0.0};

	// Protected state
	mutable std::mutex mutex_;
	std::string progress_message_;
	std::vector<progress_info> progress_history_;
	std::shared_ptr<container_module::value_container> checkpoint_;
	std::vector<task_log_entry> logs_;
	std::vector<std::string> spawned_subtasks_;

	subtask_spawner subtask_spawner_;
};

}  // namespace kcenon::messaging::task
