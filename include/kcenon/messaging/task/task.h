// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task.h
 * @brief Task class for distributed task queue system
 *
 * Provides a task abstraction that extends the message class with additional
 * features for task scheduling, retry handling, and progress tracking.
 */

#pragma once

#include <kcenon/messaging/core/message.h>
#include <kcenon/common/patterns/result.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @enum task_state
 * @brief Represents the lifecycle state of a task
 */
enum class task_state {
	pending,    // Waiting to be queued
	queued,     // Added to queue
	running,    // Currently executing
	succeeded,  // Completed successfully
	failed,     // Execution failed
	retrying,   // Retrying after failure
	cancelled,  // Cancelled by user
	expired     // Expired before execution
};

/**
 * @brief Convert task_state to string representation
 * @param state The task state
 * @return String representation of the state
 */
inline std::string to_string(task_state state) {
	switch (state) {
		case task_state::pending: return "pending";
		case task_state::queued: return "queued";
		case task_state::running: return "running";
		case task_state::succeeded: return "succeeded";
		case task_state::failed: return "failed";
		case task_state::retrying: return "retrying";
		case task_state::cancelled: return "cancelled";
		case task_state::expired: return "expired";
		default: return "unknown";
	}
}

/**
 * @brief Parse string to task_state
 * @param str String representation
 * @return Parsed task_state or pending as default
 */
inline task_state task_state_from_string(const std::string& str) {
	if (str == "pending") return task_state::pending;
	if (str == "queued") return task_state::queued;
	if (str == "running") return task_state::running;
	if (str == "succeeded") return task_state::succeeded;
	if (str == "failed") return task_state::failed;
	if (str == "retrying") return task_state::retrying;
	if (str == "cancelled") return task_state::cancelled;
	if (str == "expired") return task_state::expired;
	return task_state::pending;
}

/**
 * @struct task_config
 * @brief Configuration options for task execution
 */
struct task_config {
	std::chrono::milliseconds timeout{300000};  // Default 5 minutes
	size_t max_retries = 3;
	std::chrono::milliseconds retry_delay{1000};
	double retry_backoff_multiplier = 2.0;
	message_priority priority = message_priority::normal;
	std::optional<std::chrono::system_clock::time_point> eta;  // Scheduled execution time
	std::optional<std::chrono::milliseconds> expires;          // Expiration duration
	std::string queue_name = "default";
	std::vector<std::string> tags;
};

/**
 * @class task
 * @brief Distributed task queue task extending message
 *
 * A task represents a unit of work that can be queued, executed by workers,
 * and tracked for progress and results. It extends the message class to
 * leverage existing messaging infrastructure.
 */
class task : public message {
	friend class task_builder;

public:
	task();

	/**
	 * @brief Construct a task with name
	 * @param task_name Handler identifier (e.g., "email.send")
	 */
	explicit task(const std::string& task_name);

	// Copy and move constructors
	task(const task& other);
	task(task&& other) noexcept;
	task& operator=(const task& other);
	task& operator=(task&& other) noexcept;

	~task() = default;

	// Task identification
	const std::string& task_id() const { return task_id_; }
	const std::string& task_name() const { return task_name_; }

	// State management
	task_state state() const { return state_; }
	void set_state(task_state state);

	// Configuration
	const task_config& config() const { return config_; }
	task_config& config() { return config_; }

	// Execution tracking
	size_t attempt_count() const { return attempt_count_; }
	void increment_attempt();

	const std::chrono::system_clock::time_point& started_at() const {
		return started_at_;
	}
	void set_started_at(std::chrono::system_clock::time_point time);

	const std::chrono::system_clock::time_point& completed_at() const {
		return completed_at_;
	}
	void set_completed_at(std::chrono::system_clock::time_point time);

	// Progress tracking (thread-safe)
	double progress() const { return progress_.load(std::memory_order_acquire); }
	void set_progress(double progress);

	const std::string& progress_message() const;
	void set_progress_message(const std::string& message);

	// Result/Error
	bool has_result() const;
	const container_module::value_container& result() const;
	void set_result(std::shared_ptr<container_module::value_container> result);

	bool has_error() const { return !error_message_.empty(); }
	const std::string& error_message() const;
	const std::string& error_traceback() const;
	void set_error(const std::string& message, const std::string& traceback = "");

	/**
	 * @brief Set the task payload
	 * @param payload Shared pointer to value_container
	 */
	void set_task_payload(std::shared_ptr<container_module::value_container> payload) {
		set_payload(std::move(payload));
	}

	// Utility methods
	bool is_terminal_state() const;
	bool is_expired() const;
	bool should_retry() const;
	std::chrono::milliseconds get_next_retry_delay() const;

	// Serialization
	common::Result<std::vector<uint8_t>> serialize() const;
	static common::Result<task> deserialize(const std::vector<uint8_t>& data);

private:
	std::string task_id_;
	std::string task_name_;
	task_state state_ = task_state::pending;
	task_config config_;

	// Execution tracking
	size_t attempt_count_ = 0;
	std::chrono::system_clock::time_point started_at_;
	std::chrono::system_clock::time_point completed_at_;

	// Progress (atomic for thread-safe access)
	std::atomic<double> progress_{0.0};
	std::string progress_message_;
	mutable std::mutex progress_mutex_;

	// Result/error storage
	std::shared_ptr<container_module::value_container> result_;
	std::string error_message_;
	std::string error_traceback_;

	// Generate unique task ID
	static std::string generate_task_id();
};

/**
 * @class task_builder
 * @brief Builder pattern for task construction
 */
class task_builder {
public:
	/**
	 * @brief Construct a builder for a named task
	 * @param task_name Handler identifier (e.g., "email.send")
	 */
	explicit task_builder(const std::string& task_name);

	task_builder& payload(std::shared_ptr<container_module::value_container> payload);
	task_builder& payload(const container_module::value_container& payload);
	task_builder& priority(message_priority priority);
	task_builder& timeout(std::chrono::milliseconds timeout);
	task_builder& retries(size_t max_retries);
	task_builder& retry_delay(std::chrono::milliseconds delay);
	task_builder& retry_backoff(double multiplier);
	task_builder& queue(const std::string& queue_name);
	task_builder& eta(std::chrono::system_clock::time_point execute_at);
	task_builder& countdown(std::chrono::milliseconds delay);
	task_builder& expires(std::chrono::milliseconds expires_in);
	task_builder& tag(const std::string& tag);
	task_builder& tags(const std::vector<std::string>& tags);

	/**
	 * @brief Build the task
	 * @return Result containing the task or error
	 */
	common::Result<task> build();

private:
	task task_;
};

}  // namespace kcenon::messaging::task
