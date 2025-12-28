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

task::task()
	: task_id_(generate_task_id()),
	  created_at_(std::chrono::system_clock::now()) {
}

task::task(const std::string& task_name)
	: task_id_(generate_task_id()),
	  task_name_(task_name),
	  created_at_(std::chrono::system_clock::now()) {
}

task::task(const task& other)
	: task_id_(other.task_id_),
	  task_name_(other.task_name_),
	  state_(other.state_),
	  config_(other.config_),
	  created_at_(other.created_at_),
	  started_at_(other.started_at_),
	  completed_at_(other.completed_at_),
	  payload_(other.payload_),
	  attempt_count_(other.attempt_count_),
	  progress_(other.progress_.load()),
	  progress_message_(other.progress_message_),
	  result_(other.result_),
	  error_message_(other.error_message_),
	  error_traceback_(other.error_traceback_) {
}

task::task(task&& other) noexcept
	: task_id_(std::move(other.task_id_)),
	  task_name_(std::move(other.task_name_)),
	  state_(other.state_),
	  config_(std::move(other.config_)),
	  created_at_(other.created_at_),
	  started_at_(other.started_at_),
	  completed_at_(other.completed_at_),
	  payload_(std::move(other.payload_)),
	  attempt_count_(other.attempt_count_),
	  progress_(other.progress_.load()),
	  progress_message_(std::move(other.progress_message_)),
	  result_(std::move(other.result_)),
	  error_message_(std::move(other.error_message_)),
	  error_traceback_(std::move(other.error_traceback_)) {
}

task& task::operator=(const task& other) {
	if (this != &other) {
		task_id_ = other.task_id_;
		task_name_ = other.task_name_;
		state_ = other.state_;
		config_ = other.config_;
		created_at_ = other.created_at_;
		started_at_ = other.started_at_;
		completed_at_ = other.completed_at_;
		payload_ = other.payload_;
		attempt_count_ = other.attempt_count_;
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
		task_id_ = std::move(other.task_id_);
		task_name_ = std::move(other.task_name_);
		state_ = other.state_;
		config_ = std::move(other.config_);
		created_at_ = other.created_at_;
		started_at_ = other.started_at_;
		completed_at_ = other.completed_at_;
		payload_ = std::move(other.payload_);
		attempt_count_ = other.attempt_count_;
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
		now - created_at_);

	return elapsed >= config_.expires.value();
}

const container_module::value_container& task::payload() const {
	if (!payload_) {
		throw std::runtime_error("Task has no payload");
	}
	return *payload_;
}

container_module::value_container& task::payload() {
	if (!payload_) {
		payload_ = std::make_shared<container_module::value_container>();
	}
	return *payload_;
}

void task::set_payload(std::shared_ptr<container_module::value_container> payload) {
	payload_ = std::move(payload);
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

namespace {

// Helper to write a string with 2-byte length prefix
void write_string(std::vector<uint8_t>& buffer, const std::string& str) {
	auto len = static_cast<uint16_t>(str.size());
	buffer.push_back(static_cast<uint8_t>(len & 0xFF));
	buffer.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
	buffer.insert(buffer.end(), str.begin(), str.end());
}

// Helper to read a string with 2-byte length prefix
bool read_string(const std::vector<uint8_t>& data, size_t& offset, std::string& str) {
	if (offset + 2 > data.size()) return false;
	uint16_t len = data[offset] | (static_cast<uint16_t>(data[offset + 1]) << 8);
	offset += 2;
	if (offset + len > data.size()) return false;
	str.assign(data.begin() + offset, data.begin() + offset + len);
	offset += len;
	return true;
}

// Helper to write a 64-bit integer
void write_int64(std::vector<uint8_t>& buffer, int64_t value) {
	for (int i = 0; i < 8; ++i) {
		buffer.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
	}
}

// Helper to read a 64-bit integer
bool read_int64(const std::vector<uint8_t>& data, size_t& offset, int64_t& value) {
	if (offset + 8 > data.size()) return false;
	value = 0;
	for (int i = 0; i < 8; ++i) {
		value |= static_cast<int64_t>(data[offset + i]) << (i * 8);
	}
	offset += 8;
	return true;
}

}  // namespace

common::Result<std::vector<uint8_t>> task::serialize() const {
	std::vector<uint8_t> result;

	// Format version 3 for composition-based task (Issue #192)
	result.push_back(0x03);

	// Serialize task_id and task_name
	write_string(result, task_id_);
	write_string(result, task_name_);

	// Serialize state
	result.push_back(static_cast<uint8_t>(state_));

	// Serialize priority
	result.push_back(static_cast<uint8_t>(config_.priority));

	// Serialize created_at timestamp
	auto created_ts = std::chrono::duration_cast<std::chrono::microseconds>(
		created_at_.time_since_epoch()).count();
	write_int64(result, created_ts);

	// Serialize attempt_count
	result.push_back(static_cast<uint8_t>(attempt_count_ & 0xFF));

	// Serialize queue_name
	write_string(result, config_.queue_name);

	// Serialize error info if present
	result.push_back(has_error() ? 1 : 0);
	if (has_error()) {
		write_string(result, error_message_);
		write_string(result, error_traceback_);
	}

	return common::ok(std::move(result));
}

common::Result<task> task::deserialize(const std::vector<uint8_t>& data) {
	if (data.empty()) {
		return common::Result<task>(
			common::error_info{error::invalid_message, "Empty data cannot be deserialized"});
	}

	uint8_t version = data[0];
	size_t offset = 1;

	// Support both v2 (legacy) and v3 (composition-based) formats
	if (version == 0x02) {
		// Legacy format (v2) - basic deserialization for backward compatibility
		if (data.size() < 3) {
			return common::Result<task>(
				common::error_info{error::message_deserialization_failed,
								   "Task data too short"});
		}

		// Deserialize task_id (1-byte length prefix in v2)
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

	if (version != 0x03) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed,
							   "Unsupported task format version"});
	}

	// Version 3 format (composition-based)
	std::string task_id, task_name;
	if (!read_string(data, offset, task_id) || !read_string(data, offset, task_name)) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed,
							   "Failed to read task identification"});
	}

	if (offset >= data.size()) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed, "Missing state"});
	}
	auto state = static_cast<task_state>(data[offset++]);

	if (offset >= data.size()) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed, "Missing priority"});
	}
	auto priority = static_cast<message_priority>(data[offset++]);

	int64_t created_ts;
	if (!read_int64(data, offset, created_ts)) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed, "Missing timestamp"});
	}

	if (offset >= data.size()) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed, "Missing attempt_count"});
	}
	auto attempt_count = static_cast<size_t>(data[offset++]);

	std::string queue_name;
	if (!read_string(data, offset, queue_name)) {
		return common::Result<task>(
			common::error_info{error::message_deserialization_failed, "Missing queue_name"});
	}

	// Create task and populate fields
	task t(task_name);
	t.task_id_ = task_id;
	t.state_ = state;
	t.config_.priority = priority;
	t.created_at_ = std::chrono::system_clock::time_point(
		std::chrono::microseconds(created_ts));
	t.attempt_count_ = attempt_count;
	t.config_.queue_name = queue_name;

	// Read error info if present
	if (offset < data.size() && data[offset++] == 1) {
		std::string error_msg, error_tb;
		if (read_string(data, offset, error_msg)) {
			t.error_message_ = error_msg;
		}
		if (read_string(data, offset, error_tb)) {
			t.error_traceback_ = error_tb;
		}
	}

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
		task_.set_payload(std::move(payload));
	}
	return *this;
}

task_builder& task_builder::payload(const container_module::value_container& payload) {
	// Create a copy of the value_container
	auto payload_copy = std::make_shared<container_module::value_container>(payload, false);
	task_.set_payload(std::move(payload_copy));
	return *this;
}

task_builder& task_builder::priority(message_priority priority) {
	task_.config_.priority = priority;
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
