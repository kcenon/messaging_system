#pragma once

#include <core/container.h>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <kcenon/common/patterns/result.h>

namespace kcenon::messaging {

/**
 * @enum message_priority
 * @brief Message priority levels
 */
enum class message_priority : uint8_t {
	lowest = 0,
	low = 1,
	normal = 2,
	high = 3,
	highest = 4,
	critical = 5
};

/**
 * @enum message_type
 * @brief Message type classification
 */
enum class message_type {
	command,	  // Execute an action
	event,		  // Something happened
	query,		  // Request information
	reply,		  // Response to query/command
	notification  // Informational message
};

/**
 * @struct message_metadata
 * @brief Message metadata and headers
 */
struct message_metadata {
	std::string id;			   // Unique message ID
	std::string topic;		   // Topic/channel
	std::string source;		   // Source service/component
	std::string target;		   // Target service/component (optional)
	std::string correlation_id;  // For request/reply correlation
	std::string trace_id;		   // Distributed tracing ID

	message_type type;
	message_priority priority;

	std::chrono::system_clock::time_point timestamp;
	std::optional<std::chrono::milliseconds> ttl;  // Time-to-live

	// Additional headers (key-value pairs)
	std::unordered_map<std::string, std::string> headers;
};

/**
 * @class message
 * @brief Core message structure using container_system
 */
class message {
	friend class message_builder;

	message_metadata metadata_;
	std::shared_ptr<container_module::value_container> payload_;

public:
	message();

	// Constructors
	explicit message(const std::string& topic);
	message(const std::string& topic, message_type type);

	// Metadata access
	const message_metadata& metadata() const { return metadata_; }
	message_metadata& metadata() { return metadata_; }

	// Payload access
	const container_module::value_container& payload() const;
	container_module::value_container& payload();

	// Convenience methods
	bool is_expired() const;
	std::chrono::milliseconds age() const;

	// Serialization
	common::Result<std::vector<uint8_t>> serialize() const;
	static common::Result<message> deserialize(
		const std::vector<uint8_t>& data);
};

/**
 * @class message_builder
 * @brief Builder pattern for message construction
 */
class message_builder {
	message msg_;

public:
	message_builder();

	message_builder& topic(std::string topic);
	message_builder& source(std::string source);
	message_builder& target(std::string target);
	message_builder& type(message_type type);
	message_builder& priority(message_priority priority);
	message_builder& ttl(std::chrono::milliseconds ttl);
	message_builder& correlation_id(std::string id);
	message_builder& trace_id(std::string id);
	message_builder& header(std::string key, std::string value);

	message_builder& payload(
		std::shared_ptr<container_module::value_container> payload);

	common::Result<message> build();
};

}  // namespace kcenon::messaging
