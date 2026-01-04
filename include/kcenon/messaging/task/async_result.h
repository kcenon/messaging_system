// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file async_result.h
 * @brief Asynchronous result handle for distributed task queue system
 *
 * Provides a handle for asynchronously retrieving results after task submission.
 * Supports polling, blocking wait, and callback-based result retrieval.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/result_backend.h>
#include <kcenon/common/patterns/result.h>
#include <kcenon/common/interfaces/executor_interface.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @class async_result
 * @brief Handle for asynchronously retrieving task execution results
 *
 * An async_result is returned when a task is submitted to the queue.
 * It provides methods to:
 * - Check task status and progress
 * - Wait for results (blocking or with timeout)
 * - Register callbacks for completion
 * - Cancel task execution
 *
 * Thread Safety:
 * - All methods are thread-safe
 * - Multiple threads can safely check status and wait for results
 *
 * @example
 * // Submit task and get async result
 * auto result = client.send("email.send", payload);
 *
 * // Polling approach
 * while (!result.is_ready()) {
 *     std::cout << "Progress: " << result.progress() * 100 << "%\n";
 *     std::this_thread::sleep_for(std::chrono::seconds(1));
 * }
 *
 * // Blocking wait with timeout
 * auto outcome = result.get(std::chrono::seconds(30));
 * if (outcome.is_ok()) {
 *     std::cout << "Result: " << outcome.value().to_string() << "\n";
 * }
 *
 * // Callback approach
 * result.then(
 *     [](const auto& res) { std::cout << "Success!\n"; },
 *     [](const auto& err) { std::cout << "Failed: " << err << "\n"; }
 * );
 */
class async_result {
public:
	/**
	 * @brief Construct an async result handle
	 * @param task_id Unique identifier of the submitted task
	 * @param backend Shared pointer to the result backend
	 * @param executor Optional executor for callback monitoring (recommended)
	 *
	 * If executor is provided, callback monitoring uses it for background tasks.
	 * If not provided, falls back to std::thread.
	 */
	async_result(
		std::string task_id,
		std::shared_ptr<result_backend_interface> backend,
		std::shared_ptr<common::interfaces::IExecutor> executor = nullptr
	);

	/**
	 * @brief Default constructor (invalid result)
	 */
	async_result();

	// Copy and move operations
	async_result(const async_result& other);
	async_result(async_result&& other) noexcept;
	async_result& operator=(const async_result& other);
	async_result& operator=(async_result&& other) noexcept;

	~async_result() = default;

	// ========================================================================
	// Task identification
	// ========================================================================

	/**
	 * @brief Get the task ID
	 * @return Unique task identifier
	 */
	const std::string& task_id() const { return task_id_; }

	/**
	 * @brief Check if this is a valid result handle
	 * @return true if the handle is associated with a task
	 */
	bool is_valid() const { return !task_id_.empty() && backend_ != nullptr; }

	// ========================================================================
	// Status queries
	// ========================================================================

	/**
	 * @brief Get current task state
	 * @return Current task state
	 */
	task_state state() const;

	/**
	 * @brief Check if the task has completed (success or failure)
	 * @return true if task is in a terminal state
	 */
	bool is_ready() const;

	/**
	 * @brief Check if the task completed successfully
	 * @return true if task state is succeeded
	 */
	bool is_successful() const;

	/**
	 * @brief Check if the task failed
	 * @return true if task state is failed
	 */
	bool is_failed() const;

	/**
	 * @brief Check if the task was cancelled
	 * @return true if task state is cancelled
	 */
	bool is_cancelled() const;

	// ========================================================================
	// Progress queries
	// ========================================================================

	/**
	 * @brief Get current progress value
	 * @return Progress value between 0.0 and 1.0
	 */
	double progress() const;

	/**
	 * @brief Get progress message
	 * @return Latest progress message
	 */
	std::string progress_message() const;

	// ========================================================================
	// Result retrieval (blocking)
	// ========================================================================

	/**
	 * @brief Wait for and retrieve the result
	 *
	 * Blocks until the task completes or timeout expires.
	 * If the task fails, returns an error with the failure message.
	 *
	 * @param timeout Maximum time to wait (default: indefinite)
	 * @return Result containing value_container on success, error otherwise
	 */
	common::Result<container_module::value_container> get(
		std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
	);

	/**
	 * @brief Wait for task completion without retrieving result
	 *
	 * Blocks until the task reaches a terminal state or timeout expires.
	 *
	 * @param timeout Maximum time to wait
	 * @return true if task completed within timeout, false if timed out
	 */
	bool wait(std::chrono::milliseconds timeout = std::chrono::milliseconds::max());

	// ========================================================================
	// Result retrieval (callback-based)
	// ========================================================================

	/**
	 * @brief Register callbacks for task completion
	 *
	 * The appropriate callback will be invoked when the task completes.
	 * If the task is already complete, the callback is invoked immediately.
	 *
	 * @param on_success Callback for successful completion
	 * @param on_failure Callback for failure (optional)
	 */
	void then(
		std::function<void(const container_module::value_container&)> on_success,
		std::function<void(const std::string&)> on_failure = nullptr
	);

	// ========================================================================
	// Task control
	// ========================================================================

	/**
	 * @brief Request task cancellation
	 *
	 * Attempts to cancel the task. If the task is already running,
	 * cancellation may not take effect immediately.
	 *
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult revoke();

	// ========================================================================
	// Child task management
	// ========================================================================

	/**
	 * @brief Get results for child tasks
	 *
	 * Returns async_result handles for any subtasks spawned by this task.
	 *
	 * @return Vector of async_result handles for child tasks
	 */
	std::vector<async_result> children() const;

	/**
	 * @brief Add a child task ID
	 * @param child_task_id ID of the child task
	 */
	void add_child(const std::string& child_task_id);

	// ========================================================================
	// Error information
	// ========================================================================

	/**
	 * @brief Get error message if task failed
	 * @return Error message or empty string
	 */
	std::string error_message() const;

	/**
	 * @brief Get error traceback if task failed
	 * @return Error traceback or empty string
	 */
	std::string error_traceback() const;

private:
	void invoke_callbacks();
	void start_callback_monitor();

	std::string task_id_;
	std::shared_ptr<result_backend_interface> backend_;
	std::shared_ptr<common::interfaces::IExecutor> executor_;

	// Thread-safe callback management
	mutable std::mutex mutex_;
	std::function<void(const container_module::value_container&)> success_callback_;
	std::function<void(const std::string&)> failure_callback_;
	std::atomic<bool> callback_invoked_{false};
	std::atomic<bool> callback_monitor_started_{false};

	// Child tasks
	std::vector<std::string> child_task_ids_;
};

}  // namespace kcenon::messaging::task
