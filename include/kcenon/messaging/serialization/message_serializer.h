#pragma once

#include <core/container.h>
#include <kcenon/common/patterns/result.h>
#include <kcenon/messaging/core/message.h>

#include <memory>
#include <string>
#include <vector>

namespace kcenon::messaging::serialization {

/**
 * @enum serialization_format
 * @brief Supported serialization formats
 */
enum class serialization_format {
	binary,    ///< Binary format (compact, fast)
	json,      ///< JSON format (human-readable)
	xml        ///< XML format (verbose, structured)
};

/**
 * @class message_serializer
 * @brief Unified serialization interface for messages and containers
 *
 * This class provides high-performance serialization capabilities using
 * container_system's optimized serialization features.
 *
 * @example Binary serialization
 * @code
 * message_serializer serializer;
 *
 * // Serialize message
 * auto binary = serializer.serialize(msg);
 * if (binary.is_ok()) {
 *     send_over_network(binary.value());
 * }
 *
 * // Deserialize message
 * auto restored = serializer.deserialize<message>(received_data);
 * @endcode
 *
 * @example JSON serialization
 * @code
 * message_serializer serializer(serialization_format::json);
 *
 * auto json = serializer.to_json(msg);
 * std::cout << json.value() << std::endl;
 * @endcode
 */
class message_serializer {
	serialization_format format_{serialization_format::binary};

public:
	/**
	 * @brief Default constructor (binary format)
	 */
	message_serializer() = default;

	/**
	 * @brief Construct with specific format
	 * @param format Serialization format to use
	 */
	explicit message_serializer(serialization_format format);

	~message_serializer() = default;

	/**
	 * @brief Set serialization format
	 * @param format New format to use
	 */
	void set_format(serialization_format format);

	/**
	 * @brief Get current serialization format
	 * @return Current format
	 */
	serialization_format get_format() const noexcept;

	// Container serialization

	/**
	 * @brief Serialize container to binary
	 * @param container Container to serialize
	 * @return Result containing binary data or error
	 */
	common::Result<std::vector<uint8_t>> serialize(
		const container_module::value_container& container) const;

	/**
	 * @brief Serialize container to binary
	 * @param container Container shared_ptr to serialize
	 * @return Result containing binary data or error
	 */
	common::Result<std::vector<uint8_t>> serialize(
		std::shared_ptr<container_module::value_container> container) const;

	/**
	 * @brief Deserialize binary to container
	 * @param data Binary data
	 * @return Result containing container or error
	 */
	common::Result<std::shared_ptr<container_module::value_container>>
	deserialize_container(const std::vector<uint8_t>& data) const;

	/**
	 * @brief Deserialize string to container
	 * @param data Serialized string data
	 * @return Result containing container or error
	 */
	common::Result<std::shared_ptr<container_module::value_container>>
	deserialize_container(const std::string& data) const;

	// Message serialization

	/**
	 * @brief Serialize message to binary
	 * @param msg Message to serialize
	 * @return Result containing binary data or error
	 */
	common::Result<std::vector<uint8_t>> serialize(const message& msg) const;

	/**
	 * @brief Deserialize binary to message
	 * @param data Binary data
	 * @return Result containing message or error
	 */
	common::Result<message> deserialize_message(
		const std::vector<uint8_t>& data) const;

	// JSON operations

	/**
	 * @brief Convert container to JSON string
	 * @param container Container to convert
	 * @return Result containing JSON string or error
	 */
	common::Result<std::string> to_json(
		const container_module::value_container& container) const;

	/**
	 * @brief Convert container shared_ptr to JSON string
	 * @param container Container to convert
	 * @return Result containing JSON string or error
	 */
	common::Result<std::string> to_json(
		std::shared_ptr<container_module::value_container> container) const;

	/**
	 * @brief Convert message to JSON string
	 * @param msg Message to convert
	 * @return Result containing JSON string or error
	 */
	common::Result<std::string> to_json(const message& msg) const;

	/**
	 * @brief Parse JSON string to container
	 * @param json JSON string
	 * @return Result containing container or error
	 */
	common::Result<std::shared_ptr<container_module::value_container>>
	from_json(const std::string& json) const;

	// XML operations

	/**
	 * @brief Convert container to XML string
	 * @param container Container to convert
	 * @return Result containing XML string or error
	 */
	common::Result<std::string> to_xml(
		const container_module::value_container& container) const;

	/**
	 * @brief Convert container shared_ptr to XML string
	 * @param container Container to convert
	 * @return Result containing XML string or error
	 */
	common::Result<std::string> to_xml(
		std::shared_ptr<container_module::value_container> container) const;

	// Batch operations (SIMD-optimized when available)

	/**
	 * @brief Batch serialize multiple containers
	 * @param containers Vector of containers to serialize
	 * @return Result containing concatenated binary data or error
	 */
	common::Result<std::vector<uint8_t>> batch_serialize(
		const std::vector<std::shared_ptr<container_module::value_container>>&
			containers) const;

	/**
	 * @brief Batch deserialize binary to multiple containers
	 * @param data Binary data containing multiple serialized containers
	 * @return Result containing vector of containers or error
	 */
	common::Result<std::vector<std::shared_ptr<container_module::value_container>>>
	batch_deserialize(const std::vector<uint8_t>& data) const;
};

// Inline implementations

inline message_serializer::message_serializer(serialization_format format)
	: format_(format) {}

inline void message_serializer::set_format(serialization_format format) {
	format_ = format;
}

inline serialization_format message_serializer::get_format() const noexcept {
	return format_;
}

inline common::Result<std::vector<uint8_t>> message_serializer::serialize(
	const container_module::value_container& container) const {
	using container_fmt = container_module::value_container::serialization_format;
	auto result = container.serialize(container_fmt::binary);
	if (result.is_err()) {
		return common::make_error<std::vector<uint8_t>>(
			common::error_codes::INTERNAL_ERROR,
			std::string("Serialization failed: ") + result.error().message,
			"message_serializer");
	}
	return common::ok(std::move(result.value()));
}

inline common::Result<std::vector<uint8_t>> message_serializer::serialize(
	std::shared_ptr<container_module::value_container> container) const {
	if (!container) {
		return common::make_error<std::vector<uint8_t>>(
			common::error_codes::INVALID_ARGUMENT,
			"Container is null",
			"message_serializer");
	}
	return serialize(*container);
}

inline common::Result<std::shared_ptr<container_module::value_container>>
message_serializer::deserialize_container(
	const std::vector<uint8_t>& data) const {
	try {
		auto container = std::make_shared<container_module::value_container>(
			data, false);
		return common::ok(std::move(container));
	} catch (const std::exception& e) {
		return common::make_error<std::shared_ptr<container_module::value_container>>(
			common::error_codes::INTERNAL_ERROR,
			std::string("Deserialization failed: ") + e.what(),
			"message_serializer");
	}
}

inline common::Result<std::shared_ptr<container_module::value_container>>
message_serializer::deserialize_container(const std::string& data) const {
	try {
		auto container = std::make_shared<container_module::value_container>(
			data, false);
		return common::ok(std::move(container));
	} catch (const std::exception& e) {
		return common::make_error<std::shared_ptr<container_module::value_container>>(
			common::error_codes::INTERNAL_ERROR,
			std::string("Deserialization failed: ") + e.what(),
			"message_serializer");
	}
}

inline common::Result<std::vector<uint8_t>> message_serializer::serialize(
	const message& msg) const {
	return msg.serialize();
}

inline common::Result<message> message_serializer::deserialize_message(
	const std::vector<uint8_t>& data) const {
	return message::deserialize(data);
}

inline common::Result<std::string> message_serializer::to_json(
	const container_module::value_container& container) const {
	using container_fmt = container_module::value_container::serialization_format;
	auto result = container.serialize_string(container_fmt::json);
	if (result.is_err()) {
		return common::make_error<std::string>(
			common::error_codes::INTERNAL_ERROR,
			std::string("JSON conversion failed: ") + result.error().message,
			"message_serializer");
	}
	return common::ok(std::move(result.value()));
}

inline common::Result<std::string> message_serializer::to_json(
	std::shared_ptr<container_module::value_container> container) const {
	if (!container) {
		return common::make_error<std::string>(
			common::error_codes::INVALID_ARGUMENT,
			"Container is null",
			"message_serializer");
	}
	return to_json(*container);
}

inline common::Result<std::string> message_serializer::to_json(
	const message& msg) const {
	return to_json(msg.payload());
}

inline common::Result<std::shared_ptr<container_module::value_container>>
message_serializer::from_json(const std::string& /*json*/) const {
	// Note: JSON parsing requires additional implementation
	// For now, we return an error indicating the feature needs implementation
	return common::make_error<std::shared_ptr<container_module::value_container>>(
		common::error_codes::INTERNAL_ERROR,
		"JSON parsing not yet implemented",
		"message_serializer");
}

inline common::Result<std::string> message_serializer::to_xml(
	const container_module::value_container& container) const {
	using container_fmt = container_module::value_container::serialization_format;
	auto result = container.serialize_string(container_fmt::xml);
	if (result.is_err()) {
		return common::make_error<std::string>(
			common::error_codes::INTERNAL_ERROR,
			std::string("XML conversion failed: ") + result.error().message,
			"message_serializer");
	}
	return common::ok(std::move(result.value()));
}

inline common::Result<std::string> message_serializer::to_xml(
	std::shared_ptr<container_module::value_container> container) const {
	if (!container) {
		return common::make_error<std::string>(
			common::error_codes::INVALID_ARGUMENT,
			"Container is null",
			"message_serializer");
	}
	return to_xml(*container);
}

inline common::Result<std::vector<uint8_t>> message_serializer::batch_serialize(
	const std::vector<std::shared_ptr<container_module::value_container>>&
		containers) const {
	std::vector<uint8_t> result;

	// Reserve estimated space
	result.reserve(containers.size() * 1024);

	for (const auto& container : containers) {
		if (!container) {
			continue;
		}

		auto serialized = serialize(*container);
		if (!serialized.is_ok()) {
			return common::make_error<std::vector<uint8_t>>(serialized.error());
		}

		const auto& data = serialized.value();
		// Store size as 4-byte header
		uint32_t size = static_cast<uint32_t>(data.size());
		result.push_back(static_cast<uint8_t>(size & 0xFF));
		result.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
		result.push_back(static_cast<uint8_t>((size >> 16) & 0xFF));
		result.push_back(static_cast<uint8_t>((size >> 24) & 0xFF));

		result.insert(result.end(), data.begin(), data.end());
	}

	return common::ok(std::move(result));
}

inline common::Result<std::vector<std::shared_ptr<container_module::value_container>>>
message_serializer::batch_deserialize(const std::vector<uint8_t>& data) const {
	std::vector<std::shared_ptr<container_module::value_container>> result;

	size_t offset = 0;
	while (offset + 4 <= data.size()) {
		// Read 4-byte size header
		uint32_t size = static_cast<uint32_t>(data[offset]) |
						(static_cast<uint32_t>(data[offset + 1]) << 8) |
						(static_cast<uint32_t>(data[offset + 2]) << 16) |
						(static_cast<uint32_t>(data[offset + 3]) << 24);
		offset += 4;

		if (offset + size > data.size()) {
			return common::make_error<std::vector<std::shared_ptr<container_module::value_container>>>(
				common::error_codes::INVALID_ARGUMENT,
				"Batch data is truncated",
				"message_serializer");
		}

		std::vector<uint8_t> chunk(data.begin() + offset,
								   data.begin() + offset + size);
		offset += size;

		auto container = deserialize_container(chunk);
		if (!container.is_ok()) {
			return common::make_error<std::vector<std::shared_ptr<container_module::value_container>>>(
				container.error());
		}

		result.push_back(std::move(container.value()));
	}

	return common::ok(std::move(result));
}

}  // namespace kcenon::messaging::serialization
