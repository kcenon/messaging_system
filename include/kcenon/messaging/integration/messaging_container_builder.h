#pragma once

#include <core/container.h>
#include <kcenon/common/patterns/result.h>

#include <chrono>
#include <memory>
#include <optional>
#include <string>

namespace kcenon::messaging::integration {

/**
 * @enum optimization_mode
 * @brief Optimization modes for container building
 */
enum class optimization_mode {
	balanced,         ///< Default balanced mode
	speed,            ///< Optimize for speed (less validation)
	memory,           ///< Optimize for memory (more compact)
	network           ///< Optimize for network transfer
};

/**
 * @class messaging_container_builder
 * @brief Type-safe builder pattern for constructing messaging containers
 *
 * This builder provides a fluent interface for creating value_container
 * instances optimized for messaging scenarios. It integrates with
 * container_system's SIMD optimizations and serialization features.
 *
 * @example
 * @code
 * auto container = messaging_container_builder()
 *     .source("trading_engine", "session_001")
 *     .target("risk_monitor", "main")
 *     .message_type("market_data")
 *     .add_value("symbol", "AAPL")
 *     .add_value("price", 175.50)
 *     .add_value("volume", 1000000)
 *     .add_value("timestamp", std::chrono::system_clock::now())
 *     .optimize_for_speed()
 *     .build();
 *
 * if (container) {
 *     message_bus.publish(container.value());
 * }
 * @endcode
 */
class messaging_container_builder {
	std::shared_ptr<container_module::value_container> container_;
	optimization_mode mode_{optimization_mode::balanced};
	bool validated_{false};
	std::string error_message_;

public:
	messaging_container_builder();
	~messaging_container_builder() = default;

	// Move operations
	messaging_container_builder(messaging_container_builder&&) noexcept = default;
	messaging_container_builder& operator=(messaging_container_builder&&) noexcept = default;

	// Copy operations
	messaging_container_builder(const messaging_container_builder&) = default;
	messaging_container_builder& operator=(const messaging_container_builder&) = default;

	/**
	 * @brief Set source identifier with optional sub-id
	 * @param source_id Primary source identifier
	 * @param sub_id Optional sub-identifier (default: empty)
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& source(const std::string& source_id,
										const std::string& sub_id = "");

	/**
	 * @brief Set target identifier with optional sub-id
	 * @param target_id Primary target identifier
	 * @param sub_id Optional sub-identifier (default: empty)
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& target(const std::string& target_id,
										const std::string& sub_id = "");

	/**
	 * @brief Set message type
	 * @param type Message type string
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& message_type(const std::string& type);

	/**
	 * @brief Add a string value
	 * @param key Value key/name
	 * @param value String value
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_value(const std::string& key,
										   const std::string& value);

	/**
	 * @brief Add a const char* value (converts to string)
	 * @param key Value key/name
	 * @param value C-string value
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_value(const std::string& key,
										   const char* value);

	/**
	 * @brief Add an integer value
	 * @param key Value key/name
	 * @param value Integer value
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_value(const std::string& key, int value);

	/**
	 * @brief Add a long long value
	 * @param key Value key/name
	 * @param value Long long value
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_value(const std::string& key, long long value);

	/**
	 * @brief Add a double value
	 * @param key Value key/name
	 * @param value Double value
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_value(const std::string& key, double value);

	/**
	 * @brief Add a boolean value
	 * @param key Value key/name
	 * @param value Boolean value
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_value(const std::string& key, bool value);

	/**
	 * @brief Add a timestamp value
	 * @param key Value key/name
	 * @param value Time point value
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_value(
		const std::string& key,
		std::chrono::system_clock::time_point value);

	/**
	 * @brief Add binary data
	 * @param key Value key/name
	 * @param data Binary data vector
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_binary(const std::string& key,
											const std::vector<uint8_t>& data);

	/**
	 * @brief Add a nested container
	 * @param key Value key/name
	 * @param container Nested container
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& add_container(
		const std::string& key,
		std::shared_ptr<container_module::value_container> container);

	/**
	 * @brief Optimize container for speed (less validation, faster serialization)
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& optimize_for_speed();

	/**
	 * @brief Optimize container for memory efficiency
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& optimize_for_memory();

	/**
	 * @brief Optimize container for network transfer
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& optimize_for_network();

	/**
	 * @brief Build the container
	 * @return Result containing the built container or error
	 */
	common::Result<std::shared_ptr<container_module::value_container>> build();

	/**
	 * @brief Reset the builder to initial state
	 * @return Reference to this builder for chaining
	 */
	messaging_container_builder& reset();

	/**
	 * @brief Check if builder is in valid state
	 * @return true if valid, false otherwise
	 */
	bool is_valid() const noexcept;

	/**
	 * @brief Get current optimization mode
	 * @return Current optimization mode
	 */
	optimization_mode get_optimization_mode() const noexcept;

private:
	void apply_optimization();
};

// Inline implementations

inline messaging_container_builder::messaging_container_builder()
	: container_(std::make_shared<container_module::value_container>())
	, mode_(optimization_mode::balanced)
	, validated_(false) {}

inline messaging_container_builder& messaging_container_builder::source(
	const std::string& source_id, const std::string& sub_id) {
	container_->set_source(source_id, sub_id);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::target(
	const std::string& target_id, const std::string& sub_id) {
	container_->set_target(target_id, sub_id);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::message_type(
	const std::string& type) {
	container_->set_message_type(type);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_value(
	const std::string& key, const std::string& value) {
	container_->add_value(key, value);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_value(
	const std::string& key, const char* value) {
	container_->add_value(key, std::string(value));
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_value(
	const std::string& key, int value) {
	container_->add_value(key, value);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_value(
	const std::string& key, long long value) {
	container_->add_value(key, value);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_value(
	const std::string& key, double value) {
	container_->add_value(key, value);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_value(
	const std::string& key, bool value) {
	container_->add_value(key, value);
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_value(
	const std::string& key, std::chrono::system_clock::time_point value) {
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		value.time_since_epoch()).count();
	container_->add_value(key, static_cast<long long>(ms));
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_binary(
	const std::string& key, const std::vector<uint8_t>& data) {
	container_->add_value(key, container_module::value_types::bytes_value,
						  container_module::value_variant{data});
	return *this;
}

inline messaging_container_builder& messaging_container_builder::add_container(
	const std::string& key,
	std::shared_ptr<container_module::value_container> nested) {
	container_->add_value(key, container_module::value_types::container_value,
						  container_module::value_variant{nested});
	return *this;
}

inline messaging_container_builder& messaging_container_builder::optimize_for_speed() {
	mode_ = optimization_mode::speed;
	return *this;
}

inline messaging_container_builder& messaging_container_builder::optimize_for_memory() {
	mode_ = optimization_mode::memory;
	return *this;
}

inline messaging_container_builder& messaging_container_builder::optimize_for_network() {
	mode_ = optimization_mode::network;
	return *this;
}

inline common::Result<std::shared_ptr<container_module::value_container>>
messaging_container_builder::build() {
	if (!container_) {
		return common::error<std::shared_ptr<container_module::value_container>>(
			common::error_info{
				.code = "BUILDER_INVALID",
				.message = "Builder is in invalid state"
			});
	}

	apply_optimization();

	auto result = container_;
	container_ = std::make_shared<container_module::value_container>();
	validated_ = false;

	return common::ok(std::move(result));
}

inline messaging_container_builder& messaging_container_builder::reset() {
	container_ = std::make_shared<container_module::value_container>();
	mode_ = optimization_mode::balanced;
	validated_ = false;
	error_message_.clear();
	return *this;
}

inline bool messaging_container_builder::is_valid() const noexcept {
	return container_ != nullptr;
}

inline optimization_mode messaging_container_builder::get_optimization_mode() const noexcept {
	return mode_;
}

inline void messaging_container_builder::apply_optimization() {
	switch (mode_) {
	case optimization_mode::speed:
		container_->set_soo_enabled(true);
		break;
	case optimization_mode::memory:
		container_->set_soo_enabled(true);
		break;
	case optimization_mode::network:
		container_->set_soo_enabled(true);
		break;
	case optimization_mode::balanced:
	default:
		container_->set_soo_enabled(true);
		break;
	}
}

}  // namespace kcenon::messaging::integration
