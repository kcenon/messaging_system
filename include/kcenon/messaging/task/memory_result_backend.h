// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file memory_result_backend.h
 * @brief In-memory implementation of result backend interface
 *
 * Provides a thread-safe, in-memory storage for task results.
 * Suitable for single-process environments where persistence is not required.
 */

#pragma once

#include <kcenon/messaging/task/result_backend.h>

#include <condition_variable>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

namespace kcenon::messaging::task {

/**
 * @class memory_result_backend
 * @brief In-memory implementation of result_backend_interface
 *
 * Thread-safe storage for task results using shared_mutex for
 * concurrent read access and exclusive write access.
 *
 * Features:
 * - Thread-safe with reader-writer locking
 * - Efficient waiting using condition_variable
 * - Automatic cleanup of expired results
 * - No external dependencies
 *
 * Limitations:
 * - Data is lost on process termination
 * - Not suitable for distributed environments
 * - Memory grows with number of stored tasks
 *
 * @example
 * auto backend = std::make_shared<memory_result_backend>();
 *
 * // Store result
 * backend->store_state("task-1", task_state::running);
 * backend->store_progress("task-1", 0.5, "Halfway done");
 * backend->store_state("task-1", task_state::succeeded);
 * backend->store_result("task-1", result_data);
 *
 * // Wait for result
 * auto result = backend->wait_for_result("task-1", std::chrono::seconds(30));
 */
class memory_result_backend : public result_backend_interface {
public:
	/**
	 * @brief Construct a memory result backend
	 */
	memory_result_backend() = default;

	~memory_result_backend() override = default;

	// Non-copyable, non-movable (contains std::shared_mutex)
	memory_result_backend(const memory_result_backend&) = delete;
	memory_result_backend& operator=(const memory_result_backend&) = delete;
	memory_result_backend(memory_result_backend&&) = delete;
	memory_result_backend& operator=(memory_result_backend&&) = delete;

	// ========================================================================
	// State storage
	// ========================================================================

	common::VoidResult store_state(
		const std::string& task_id,
		task_state state
	) override;

	// ========================================================================
	// Result storage
	// ========================================================================

	common::VoidResult store_result(
		const std::string& task_id,
		const container_module::value_container& result
	) override;

	// ========================================================================
	// Error storage
	// ========================================================================

	common::VoidResult store_error(
		const std::string& task_id,
		const std::string& error,
		const std::string& traceback = ""
	) override;

	// ========================================================================
	// Progress storage
	// ========================================================================

	common::VoidResult store_progress(
		const std::string& task_id,
		double progress,
		const std::string& message = ""
	) override;

	// ========================================================================
	// Query methods
	// ========================================================================

	common::Result<task_state> get_state(
		const std::string& task_id
	) override;

	common::Result<container_module::value_container> get_result(
		const std::string& task_id
	) override;

	common::Result<progress_data> get_progress(
		const std::string& task_id
	) override;

	common::Result<error_data> get_error(
		const std::string& task_id
	) override;

	// ========================================================================
	// Blocking operations
	// ========================================================================

	common::Result<container_module::value_container> wait_for_result(
		const std::string& task_id,
		std::chrono::milliseconds timeout
	) override;

	// ========================================================================
	// Cleanup
	// ========================================================================

	common::VoidResult cleanup_expired(
		std::chrono::milliseconds max_age
	) override;

	// ========================================================================
	// Optional methods
	// ========================================================================

	bool exists(const std::string& task_id) override;

	common::VoidResult remove(const std::string& task_id) override;

	size_t size() const override;

	/**
	 * @brief Clear all stored data
	 *
	 * Removes all task data from the backend.
	 */
	void clear();

private:
	/**
	 * @struct task_result
	 * @brief Internal storage structure for task data
	 */
	struct task_result {
		task_state state{task_state::pending};
		std::optional<container_module::value_container> result;
		std::optional<error_data> error;
		double progress{0.0};
		std::string progress_message;
		std::chrono::system_clock::time_point created_at;
		std::chrono::system_clock::time_point updated_at;
	};

	/**
	 * @brief Check if state is terminal (succeeded, failed, cancelled, expired)
	 */
	static bool is_terminal_state(task_state state);

	/**
	 * @brief Get or create task result entry
	 *
	 * Must be called with exclusive lock held
	 */
	task_result& get_or_create(const std::string& task_id);

	// Thread-safe storage
	mutable std::shared_mutex mutex_;
	std::condition_variable_any cv_;
	std::unordered_map<std::string, task_result> results_;
};

}  // namespace kcenon::messaging::task
