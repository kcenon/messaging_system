// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/task/memory_result_backend.h>

#include <algorithm>

namespace kcenon::messaging::task {

// ============================================================================
// State storage
// ============================================================================

common::VoidResult memory_result_backend::store_state(
	const std::string& task_id,
	task_state state)
{
	std::unique_lock lock(mutex_);

	auto& entry = get_or_create(task_id);
	entry.state = state;
	entry.updated_at = std::chrono::system_clock::now();

	// Notify waiters when reaching terminal state
	if (is_terminal_state(state)) {
		cv_.notify_all();
	}

	return common::ok();
}

// ============================================================================
// Result storage
// ============================================================================

common::VoidResult memory_result_backend::store_result(
	const std::string& task_id,
	const container_module::value_container& result)
{
	std::unique_lock lock(mutex_);

	auto& entry = get_or_create(task_id);
	entry.result = result;
	entry.updated_at = std::chrono::system_clock::now();

	// Notify waiters
	cv_.notify_all();

	return common::ok();
}

// ============================================================================
// Error storage
// ============================================================================

common::VoidResult memory_result_backend::store_error(
	const std::string& task_id,
	const std::string& error,
	const std::string& traceback)
{
	std::unique_lock lock(mutex_);

	auto& entry = get_or_create(task_id);
	entry.error = error_data{
		error,
		traceback,
		std::chrono::system_clock::now()
	};
	entry.updated_at = std::chrono::system_clock::now();

	// Notify waiters
	cv_.notify_all();

	return common::ok();
}

// ============================================================================
// Progress storage
// ============================================================================

common::VoidResult memory_result_backend::store_progress(
	const std::string& task_id,
	double progress,
	const std::string& message)
{
	std::unique_lock lock(mutex_);

	auto& entry = get_or_create(task_id);
	entry.progress = std::clamp(progress, 0.0, 1.0);
	entry.progress_message = message;
	entry.updated_at = std::chrono::system_clock::now();

	return common::ok();
}

// ============================================================================
// Query methods
// ============================================================================

common::Result<task_state> memory_result_backend::get_state(
	const std::string& task_id)
{
	std::shared_lock lock(mutex_);

	auto it = results_.find(task_id);
	if (it == results_.end()) {
		return common::make_error<task_state>(
			-1,
			"Task not found: " + task_id,
			"memory_result_backend"
		);
	}

	return common::ok(it->second.state);
}

common::Result<container_module::value_container> memory_result_backend::get_result(
	const std::string& task_id)
{
	std::shared_lock lock(mutex_);

	auto it = results_.find(task_id);
	if (it == results_.end()) {
		return common::make_error<container_module::value_container>(
			-1,
			"Task not found: " + task_id,
			"memory_result_backend"
		);
	}

	if (!it->second.result.has_value()) {
		return common::make_error<container_module::value_container>(
			-2,
			"Result not available for task: " + task_id,
			"memory_result_backend"
		);
	}

	return common::ok(it->second.result.value());
}

common::Result<progress_data> memory_result_backend::get_progress(
	const std::string& task_id)
{
	std::shared_lock lock(mutex_);

	auto it = results_.find(task_id);
	if (it == results_.end()) {
		return common::make_error<progress_data>(
			-1,
			"Task not found: " + task_id,
			"memory_result_backend"
		);
	}

	return common::ok(progress_data{
		it->second.progress,
		it->second.progress_message,
		it->second.updated_at
	});
}

common::Result<error_data> memory_result_backend::get_error(
	const std::string& task_id)
{
	std::shared_lock lock(mutex_);

	auto it = results_.find(task_id);
	if (it == results_.end()) {
		return common::make_error<error_data>(
			-1,
			"Task not found: " + task_id,
			"memory_result_backend"
		);
	}

	if (!it->second.error.has_value()) {
		return common::make_error<error_data>(
			-2,
			"Error not available for task: " + task_id,
			"memory_result_backend"
		);
	}

	return common::ok(it->second.error.value());
}

// ============================================================================
// Blocking operations
// ============================================================================

common::Result<container_module::value_container> memory_result_backend::wait_for_result(
	const std::string& task_id,
	std::chrono::milliseconds timeout)
{
	std::shared_lock lock(mutex_);

	auto deadline = std::chrono::steady_clock::now() + timeout;

	// Wait for task to reach terminal state
	while (true) {
		auto it = results_.find(task_id);

		if (it == results_.end()) {
			// Task doesn't exist yet, wait for it
			if (cv_.wait_until(lock, deadline) == std::cv_status::timeout) {
				return common::make_error<container_module::value_container>(
					-3,
					"Timeout waiting for task: " + task_id,
					"memory_result_backend"
				);
			}
			continue;
		}

		const auto& entry = it->second;

		// Check if task completed successfully
		if (entry.state == task_state::succeeded) {
			if (entry.result.has_value()) {
				return common::ok(entry.result.value());
			}
			return common::make_error<container_module::value_container>(
				-2,
				"Task succeeded but no result available: " + task_id,
				"memory_result_backend"
			);
		}

		// Check if task failed
		if (entry.state == task_state::failed) {
			std::string error_msg = "Task failed";
			if (entry.error.has_value()) {
				error_msg = entry.error->message;
			}
			return common::make_error<container_module::value_container>(
				-4,
				error_msg,
				"memory_result_backend",
				entry.error.has_value() ? entry.error->traceback : ""
			);
		}

		// Check if task was cancelled
		if (entry.state == task_state::cancelled) {
			return common::make_error<container_module::value_container>(
				-5,
				"Task was cancelled: " + task_id,
				"memory_result_backend"
			);
		}

		// Check if task expired
		if (entry.state == task_state::expired) {
			return common::make_error<container_module::value_container>(
				-6,
				"Task expired: " + task_id,
				"memory_result_backend"
			);
		}

		// Task still running, wait for update
		if (cv_.wait_until(lock, deadline) == std::cv_status::timeout) {
			return common::make_error<container_module::value_container>(
				-3,
				"Timeout waiting for task: " + task_id,
				"memory_result_backend"
			);
		}
	}
}

// ============================================================================
// Cleanup
// ============================================================================

common::VoidResult memory_result_backend::cleanup_expired(
	std::chrono::milliseconds max_age)
{
	std::unique_lock lock(mutex_);

	auto now = std::chrono::system_clock::now();
	auto cutoff = now - max_age;

	for (auto it = results_.begin(); it != results_.end();) {
		// Only cleanup terminal tasks that are old enough
		if (is_terminal_state(it->second.state) &&
			it->second.updated_at < cutoff) {
			it = results_.erase(it);
		} else {
			++it;
		}
	}

	return common::ok();
}

// ============================================================================
// Optional methods
// ============================================================================

bool memory_result_backend::exists(const std::string& task_id) {
	std::shared_lock lock(mutex_);
	return results_.find(task_id) != results_.end();
}

common::VoidResult memory_result_backend::remove(const std::string& task_id) {
	std::unique_lock lock(mutex_);

	auto it = results_.find(task_id);
	if (it != results_.end()) {
		results_.erase(it);
	}

	return common::ok();
}

size_t memory_result_backend::size() const {
	std::shared_lock lock(mutex_);
	return results_.size();
}

void memory_result_backend::clear() {
	std::unique_lock lock(mutex_);
	results_.clear();
}

// ============================================================================
// Private methods
// ============================================================================

bool memory_result_backend::is_terminal_state(task_state state) {
	return state == task_state::succeeded ||
	       state == task_state::failed ||
	       state == task_state::cancelled ||
	       state == task_state::expired;
}

memory_result_backend::task_result& memory_result_backend::get_or_create(
	const std::string& task_id)
{
	auto it = results_.find(task_id);
	if (it == results_.end()) {
		auto now = std::chrono::system_clock::now();
		auto [new_it, _] = results_.emplace(
			task_id,
			task_result{
				task_state::pending,
				std::nullopt,
				std::nullopt,
				0.0,
				"",
				now,
				now
			}
		);
		return new_it->second;
	}
	return it->second;
}

}  // namespace kcenon::messaging::task
