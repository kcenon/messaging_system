// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include "kcenon/messaging/task/task.h"

#include <kcenon/messaging/error/error_codes.h>

#include <iomanip>
#include <random>
#include <sstream>

namespace kcenon::messaging::task {

namespace {

std::string generate_unique_id() {
	static std::random_device rd;
	static std::mt19937_64 gen(rd());
	static std::uniform_int_distribution<uint64_t> dis;

	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
						 now.time_since_epoch())
						 .count();

	std::ostringstream oss;
	oss << "task-" << std::hex << std::setfill('0') << std::setw(16) << timestamp
		<< "-" << std::setw(8) << (dis(gen) & 0xFFFFFFFF);
	return oss.str();
}

}  // namespace

// ============================================================================
// task implementation
// ============================================================================

task::task() : message("task", message_type::command), task_id_(generate_task_id()) {
}

task::task(const std::string& task_name)
	: message("task", message_type::command),
	  task_id_(generate_task_id()),
	  task_name_(task_name) {
}

task::task(const task& other)
	: message(other),
	  task_id_(other.task_id_),
	  task_name_(other.task_name_),
	  state_(other.state_),
	  config_(other.config_),
	  attempt_count_(other.attempt_count_),
	  started_at_(other.started_at_),
	  completed_at_(other.completed_at_),
	  progress_(other.progress_.load()),
	  progress_message_(other.progress_message_),
	  result_(other.result_),
	  error_message_(other.error_message_),
	  error_traceback_(other.error_traceback_) {
}

task::task(task&& other) noexcept
	: message(std::move(other)),
	  task_id_(std::move(other.task_id_)),
	  task_name_(std::move(other.task_name_)),
	  state_(other.state_),
	  config_(std::move(other.config_)),
	  attempt_count_(other.attempt_count_),
	  started_at_(other.started_at_),
	  completed_at_(other.completed_at_),
	  progress_(other.progress_.load()),
	  progress_message_(std::move(other.progress_message_)),
	  result_(std::move(other.result_)),
	  error_message_(std::move(other.error_message_)),
	  error_traceback_(std::move(other.error_traceback_)) {
}

task& task::operator=(const task& other) {
	if (this != &other) {
		message::operator=(other);
		task_id_ = other.task_id_;
		task_name_ = other.task_name_;
		state_ = other.state_;
		config_ = other.config_;
		attempt_count_ = other.attempt_count_;
		started_at_ = other.started_at_;
		completed_at_ = other.completed_at_;
		progress_.store(other.progress_.load());
		progress_message_ = other.progress_message_;
		result_ = other.result_;
		error_message_ = other.error_message_;
		error_traceback_ = other.error_traceback_;
	}
	return *this;
}

task& task::operator=(task&& other) noexcept {
	if (this != &other) {
		message::operator=(std::move(other));
		task_id_ = std::move(other.task_id_);
		task_name_ = std::move(other.task_name_);
		state_ = other.state_;
		config_ = std::move(other.config_);
		attempt_count_ = other.attempt_count_;
		started_at_ = other.started_at_;
		completed_at_ = other.completed_at_;
		progress_.store(other.progress_.load());
		progress_message_ = std::move(other.progress_message_);
		result_ = std::move(other.result_);
		error_message_ = std::move(other.error_message_);
		error_traceback_ = std::move(other.error_traceback_);
	}
	return *this;
}

void task::set_state(task_state state) {
	state_ = state;
}

void task::increment_attempt() {
	++attempt_count_;
}

void task::set_started_at(std::chrono::system_clock::time_point time) {
	started_at_ = time;
}

void task::set_completed_at(std::chrono::system_clock::time_point time) {
	completed_at_ = time;
}

void task::set_progress(double progress) {
	// Clamp progress to [0.0, 1.0]
	if (progress < 0.0) {
		progress = 0.0;
	} else if (progress > 1.0) {
		progress = 1.0;
	}
	progress_.store(progress, std::memory_order_release);
}

const std::string& task::progress_message() const {
	std::lock_guard<std::mutex> lock(progress_mutex_);
	return progress_message_;
}

void task::set_progress_message(const std::string& message) {
	std::lock_guard<std::mutex> lock(progress_mutex_);
	progress_message_ = message;
}

bool task::has_result() const {
	return result_ != nullptr;
}

const container_module::value_container& task::result() const {
	if (!result_) {
		throw std::runtime_error("Task has no result");
	}
	return *result_;
}

void task::set_result(std::shared_ptr<container_module::value_container> result) {
	result_ = std::move(result);
}

const std::string& task::error_message() const {
	return error_message_;
}

const std::string& task::error_traceback() const {
	return error_traceback_;
}

void task::set_error(const std::string& message, const std::string& traceback) {
	error_message_ = message;
	error_traceback_ = traceback;
}

bool task::is_terminal_state() const {
	return state_ == task_state::succeeded || state_ == task_state::failed ||
		   state_ == task_state::cancelled || state_ == task_state::expired;
}

bool task::is_expired() const {
	if (!config_.expires.has_value()) {
		return false;
	}

	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
		now - metadata().timestamp);

	return elapsed >= config_.expires.value();
}

bool task::should_retry() const {
	if (state_ != task_state::failed) {
		return false;
	}
	return attempt_count_ < config_.max_retries;
}

std::chrono::milliseconds task::get_next_retry_delay() const {
	if (attempt_count_ == 0) {
		return config_.retry_delay;
	}

	// Exponential backoff: delay * (multiplier ^ attempt)
	double multiplier = std::pow(config_.retry_backoff_multiplier, attempt_count_);
	auto delay_ms =
		static_cast<int64_t>(config_.retry_delay.count() * multiplier);

	// Cap at 1 hour maximum
	constexpr int64_t max_delay_ms = 3600000;
	if (delay_ms > max_delay_ms) {
		delay_ms = max_delay_ms;
	}

	return std::chrono::milliseconds(delay_ms);
}

std::string task::generate_task_id() {
	return generate_unique_id();
}

common::Result<std::vector<uint8_t>> task::serialize() const {
	// TODO: Implement full serialization
	// For now, return a placeholder
	std::vector<uint8_t> result;
	result.push_back(0x02);  // Format version 2 for task

	// Serialize task_id length and content
	auto id_size = static_cast<uint8_t>(task_id_.size());
	result.push_back(id_size);
	result.insert(result.end(), task_id_.begin(), task_id_.end());

	// Serialize task_name length and content
	auto name_size = static_cast<uint8_t>(task_name_.size());
	result.push_back(name_size);
	result.insert(result.end(), task_name_.begin(), task_name_.end());

	// Serialize state
	result.push_back(static_cast<uint8_t>(state_));

	return common::ok(std::move(result));
}

common::Result<task> task::deserialize(const std::vector<uint8_t>& data) {
	if (data.empty()) {
		return common::Result<task>(
			common::error_info{error::invalid_message, "Empty data cannot be deserialized"});
	}

	if (data[0] != 0x02) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed,
							   "Invalid task format version"});
	}

	if (data.size() < 3) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed,
							   "Task data too short"});
	}

	size_t offset = 1;

	// Deserialize task_id
	auto id_size = data[offset++];
	if (offset + id_size > data.size()) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed,
							   "Invalid task_id length"});
	}
	std::string task_id(data.begin() + offset, data.begin() + offset + id_size);
	offset += id_size;

	// Deserialize task_name
	if (offset >= data.size()) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed,
							   "Missing task_name"});
	}
	auto name_size = data[offset++];
	if (offset + name_size > data.size()) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed,
							   "Invalid task_name length"});
	}
	std::string task_name(data.begin() + offset, data.begin() + offset + name_size);
	offset += name_size;

	// Deserialize state
	if (offset >= data.size()) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed, "Missing state"});
	}
	auto state = static_cast<task_state>(data[offset]);

	task t(task_name);
	t.task_id_ = task_id;
	t.state_ = state;

	return common::ok(std::move(t));
}

// ============================================================================
// task_builder implementation
// ============================================================================

task_builder::task_builder(const std::string& task_name) : task_(task_name) {
}

task_builder& task_builder::payload(
	std::shared_ptr<container_module::value_container> payload) {
	if (payload) {
		task_.set_task_payload(std::move(payload));
	}
	return *this;
}

task_builder& task_builder::payload(const container_module::value_container& payload) {
	// Create a copy of the value_container
	auto payload_copy = std::make_shared<container_module::value_container>(payload, false);
	task_.set_task_payload(std::move(payload_copy));
	return *this;
}

task_builder& task_builder::priority(message_priority priority) {
	task_.config_.priority = priority;
	task_.metadata().priority = priority;
	return *this;
}

task_builder& task_builder::timeout(std::chrono::milliseconds timeout) {
	task_.config_.timeout = timeout;
	return *this;
}

task_builder& task_builder::retries(size_t max_retries) {
	task_.config_.max_retries = max_retries;
	return *this;
}

task_builder& task_builder::retry_delay(std::chrono::milliseconds delay) {
	task_.config_.retry_delay = delay;
	return *this;
}

task_builder& task_builder::retry_backoff(double multiplier) {
	task_.config_.retry_backoff_multiplier = multiplier;
	return *this;
}

task_builder& task_builder::queue(const std::string& queue_name) {
	task_.config_.queue_name = queue_name;
	return *this;
}

task_builder& task_builder::eta(std::chrono::system_clock::time_point execute_at) {
	task_.config_.eta = execute_at;
	return *this;
}

task_builder& task_builder::countdown(std::chrono::milliseconds delay) {
	task_.config_.eta = std::chrono::system_clock::now() + delay;
	return *this;
}

task_builder& task_builder::expires(std::chrono::milliseconds expires_in) {
	task_.config_.expires = expires_in;
	return *this;
}

task_builder& task_builder::tag(const std::string& tag) {
	task_.config_.tags.push_back(tag);
	return *this;
}

task_builder& task_builder::tags(const std::vector<std::string>& tags) {
	task_.config_.tags.insert(task_.config_.tags.end(), tags.begin(), tags.end());
	return *this;
}

common::Result<task> task_builder::build() {
	if (task_.task_name_.empty()) {
		return common::Result<task>(
			common::error_info{error::invalid_message, "Task name cannot be empty"});
	}

	task result = std::move(task_);
	task_ = task("");
	return common::ok(std::move(result));
}

}  // namespace kcenon::messaging::task
