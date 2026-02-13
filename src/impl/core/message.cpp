#include "kcenon/messaging/core/message.h"

#include <kcenon/messaging/error/messaging_error_category.h>

#include <random>
#include <sstream>
#include <iomanip>

namespace kcenon::messaging {

namespace {

// Generate a unique message ID
std::string generate_message_id() {
	static std::random_device rd;
	static std::mt19937_64 gen(rd());
	static std::uniform_int_distribution<uint64_t> dis;

	auto now = std::chrono::system_clock::now();
	auto timestamp =
		std::chrono::duration_cast<std::chrono::microseconds>(
			now.time_since_epoch())
			.count();

	std::ostringstream oss;
	oss << std::hex << std::setfill('0') << std::setw(16) << timestamp << "-"
		<< std::setw(16) << dis(gen);
	return oss.str();
}

}  // namespace

// ============================================================================
// message implementation
// ============================================================================

message::message() : metadata_{}, payload_(nullptr) {
	metadata_.id = generate_message_id();
	metadata_.type = message_type::event;
	metadata_.priority = message_priority::normal;
	metadata_.timestamp = std::chrono::system_clock::now();
	payload_ = std::make_shared<container_module::value_container>();
}

message::message(const std::string& topic)
	: message() {
	metadata_.topic = topic;
}

message::message(const std::string& topic, message_type type)
	: message(topic) {
	metadata_.type = type;
}

const container_module::value_container& message::payload() const {
	if (!payload_) {
		throw std::runtime_error("Message payload is null");
	}
	return *payload_;
}

container_module::value_container& message::payload() {
	if (!payload_) {
		payload_ = std::make_shared<container_module::value_container>();
	}
	return *payload_;
}

bool message::is_expired() const {
	if (!metadata_.ttl.has_value()) {
		return false;
	}

	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
		now - metadata_.timestamp);

	return elapsed >= metadata_.ttl.value();
}

std::chrono::milliseconds message::age() const {
	auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		now - metadata_.timestamp);
}

common::Result<std::vector<uint8_t>> message::serialize() const {
	// TODO: Implement proper serialization
	// For now, return a placeholder implementation with minimal data
	std::vector<uint8_t> result;

	// Add a placeholder byte to indicate serialization format version
	result.push_back(0x01);  // Format version 1

	// Add topic length and topic bytes
	const auto& topic = metadata_.topic;
	result.push_back(static_cast<uint8_t>(topic.size()));
	result.insert(result.end(), topic.begin(), topic.end());

	// Serialize metadata
	// This is a simplified implementation - in production,
	// use a proper serialization library like Protocol Buffers or MessagePack

	return common::ok(std::move(result));
}

common::Result<message> message::deserialize(const std::vector<uint8_t>& data) {
	// TODO: Implement proper deserialization
	// For now, return a default message
	if (data.empty()) {
		return common::Result<message>::err(
			make_typed_error_code(messaging_error_category::invalid_message));
	}

	message msg;
	return common::ok(std::move(msg));
}

// ============================================================================
// message_builder implementation
// ============================================================================

message_builder::message_builder() : msg_() {
}

message_builder& message_builder::topic(std::string topic) {
	msg_.metadata_.topic = std::move(topic);
	return *this;
}

message_builder& message_builder::source(std::string source) {
	msg_.metadata_.source = std::move(source);
	return *this;
}

message_builder& message_builder::target(std::string target) {
	msg_.metadata_.target = std::move(target);
	return *this;
}

message_builder& message_builder::type(message_type type) {
	msg_.metadata_.type = type;
	return *this;
}

message_builder& message_builder::priority(message_priority priority) {
	msg_.metadata_.priority = priority;
	return *this;
}

message_builder& message_builder::ttl(std::chrono::milliseconds ttl) {
	msg_.metadata_.ttl = ttl;
	return *this;
}

message_builder& message_builder::correlation_id(std::string id) {
	msg_.metadata_.correlation_id = std::move(id);
	return *this;
}

message_builder& message_builder::trace_id(std::string id) {
	msg_.metadata_.trace_id = std::move(id);
	return *this;
}

message_builder& message_builder::header(std::string key, std::string value) {
	msg_.metadata_.headers[std::move(key)] = std::move(value);
	return *this;
}

message_builder& message_builder::payload(
	std::shared_ptr<container_module::value_container> payload) {
	msg_.payload_ = std::move(payload);
	return *this;
}

common::Result<message> message_builder::build() {
	// Validate the message
	if (msg_.metadata_.topic.empty()) {
		return common::Result<message>::err(
			make_typed_error_code(messaging_error_category::invalid_message));
	}

	// Create a copy to return
	message result = std::move(msg_);

	// Reset builder state for reuse
	msg_ = message();

	return common::ok(std::move(result));
}

}  // namespace kcenon::messaging
