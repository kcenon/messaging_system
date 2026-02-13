// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include "kcenon/messaging/task/task_context.h"

#include <kcenon/messaging/error/messaging_error_category.h>

namespace kcenon::messaging::task {

task_context::task_context(task& current_task, size_t attempt)
	: task_(current_task),
	  attempt_(attempt),
	  started_at_(std::chrono::system_clock::now()) {}

// ============================================================================
// Progress tracking
// ============================================================================

void task_context::update_progress(double progress, const std::string& message) {
	// Clamp progress to [0.0, 1.0]
	if (progress < 0.0) {
		progress = 0.0;
	} else if (progress > 1.0) {
		progress = 1.0;
	}

	progress_.store(progress, std::memory_order_release);

	// Also update the task's progress
	task_.set_progress(progress);

	std::lock_guard<std::mutex> lock(mutex_);
	progress_message_ = message;
	if (!message.empty()) {
		task_.set_progress_message(message);
	}

	// Add to history
	progress_history_.push_back(
		{progress, message, std::chrono::system_clock::now()});
}

double task_context::progress() const {
	return progress_.load(std::memory_order_acquire);
}

std::vector<progress_info> task_context::progress_history() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return progress_history_;
}

// ============================================================================
// Checkpoint management
// ============================================================================

void task_context::save_checkpoint(const container_module::value_container& state) {
	auto state_copy = std::make_shared<container_module::value_container>(state, false);
	save_checkpoint(std::move(state_copy));
}

void task_context::save_checkpoint(
	std::shared_ptr<container_module::value_container> state) {
	std::lock_guard<std::mutex> lock(mutex_);
	checkpoint_ = std::move(state);
}

container_module::value_container task_context::load_checkpoint() const {
	std::lock_guard<std::mutex> lock(mutex_);
	if (checkpoint_) {
		return container_module::value_container(*checkpoint_, false);
	}
	return container_module::value_container();
}

bool task_context::has_checkpoint() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return checkpoint_ != nullptr;
}

void task_context::clear_checkpoint() {
	std::lock_guard<std::mutex> lock(mutex_);
	checkpoint_.reset();
}

// ============================================================================
// Subtask management
// ============================================================================

void task_context::set_subtask_spawner(subtask_spawner spawner) {
	std::lock_guard<std::mutex> lock(mutex_);
	subtask_spawner_ = std::move(spawner);
}

common::Result<std::string> task_context::spawn_subtask(task subtask) {
	subtask_spawner spawner;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		spawner = subtask_spawner_;
	}

	if (!spawner) {
		return common::Result<std::string>::err(
			make_typed_error_code(messaging_error_category::task_spawner_not_configured));
	}

	auto result = spawner(std::move(subtask));
	if (result.is_ok()) {
		std::lock_guard<std::mutex> lock(mutex_);
		spawned_subtasks_.push_back(result.unwrap());
	}

	return result;
}

std::vector<std::string> task_context::spawned_subtask_ids() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return spawned_subtasks_;
}

// ============================================================================
// Cancellation
// ============================================================================

bool task_context::is_cancelled() const {
	return cancelled_.load(std::memory_order_acquire);
}

void task_context::request_cancellation() {
	cancelled_.store(true, std::memory_order_release);
}

// ============================================================================
// Logging
// ============================================================================

void task_context::add_log(task_log_entry::level level,
						   const std::string& message) {
	std::lock_guard<std::mutex> lock(mutex_);
	logs_.push_back({level, message, std::chrono::system_clock::now()});
}

void task_context::log_info(const std::string& message) {
	add_log(task_log_entry::level::info, message);
}

void task_context::log_warning(const std::string& message) {
	add_log(task_log_entry::level::warning, message);
}

void task_context::log_error(const std::string& message) {
	add_log(task_log_entry::level::error, message);
}

std::vector<task_log_entry> task_context::logs() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return logs_;
}

// ============================================================================
// Task information
// ============================================================================

const task& task_context::current_task() const {
	return task_;
}

size_t task_context::attempt_number() const {
	return attempt_;
}

std::chrono::system_clock::time_point task_context::started_at() const {
	return started_at_;
}

std::chrono::milliseconds task_context::elapsed() const {
	auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now - started_at_);
}

}  // namespace kcenon::messaging::task
