#include "messaging_system/core/messaging_container.h"
#include "messaging_system/error_codes.h"
#include <kcenon/common/patterns/error_info.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace messaging {

// Helper to generate trace ID
static std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

Result<MessagingContainer> MessagingContainer::create(
    const std::string& source,
    const std::string& target,
    const std::string& topic
) {
    if (topic.empty()) {
        return Result<MessagingContainer>::error(
            common::error_info{
                error::INVALID_MESSAGE,
                "Topic cannot be empty",
                "MessagingContainer::create",
                ""
            }
        );
    }

    MessagingContainer container;
    container.container_.set_value("source", source);
    container.container_.set_value("target", target);
    container.container_.set_value("topic", topic);
    container.container_.set_value("trace_id", generate_uuid());

    return Result<MessagingContainer>::ok(std::move(container));
}

std::string MessagingContainer::source() const {
    return container_.get_value("source").to_string();
}

std::string MessagingContainer::target() const {
    return container_.get_value("target").to_string();
}

std::string MessagingContainer::topic() const {
    return container_.get_value("topic").to_string();
}

std::string MessagingContainer::trace_id() const {
    return container_.get_value("trace_id").to_string();
}

Result<std::vector<uint8_t>> MessagingContainer::serialize() const {
    try {
        auto serialized = container_.serialize();
        return Result<std::vector<uint8_t>>::ok(std::move(serialized));
    } catch (const std::exception& e) {
        return Result<std::vector<uint8_t>>::error(
            common::error_info{
                error::SERIALIZATION_ERROR,
                std::string("Serialization failed: ") + e.what(),
                "MessagingContainer::serialize",
                ""
            }
        );
    }
}

Result<MessagingContainer> MessagingContainer::deserialize(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return Result<MessagingContainer>::error(
            common::error_info{
                error::SERIALIZATION_ERROR,
                "Cannot deserialize empty data",
                "MessagingContainer::deserialize",
                ""
            }
        );
    }

    try {
        MessagingContainer container;
        container.container_ = value_container::deserialize(data);

        // Validate required fields
        if (!container.container_.contains("topic") ||
            container.container_.get_value("topic").to_string().empty()) {
            return Result<MessagingContainer>::error(
                common::error_info{
                    error::INVALID_MESSAGE,
                    "Deserialized container missing required 'topic' field",
                    "MessagingContainer::deserialize",
                    ""
                }
            );
        }

        return Result<MessagingContainer>::ok(std::move(container));
    } catch (const std::exception& e) {
        return Result<MessagingContainer>::error(
            common::error_info{
                error::SERIALIZATION_ERROR,
                std::string("Deserialization failed: ") + e.what(),
                "MessagingContainer::deserialize",
                ""
            }
        );
    }
}

// Builder implementation
MessagingContainerBuilder& MessagingContainerBuilder::source(std::string s) {
    source_ = std::move(s);
    return *this;
}

MessagingContainerBuilder& MessagingContainerBuilder::target(std::string t) {
    target_ = std::move(t);
    return *this;
}

MessagingContainerBuilder& MessagingContainerBuilder::topic(std::string topic) {
    topic_ = std::move(topic);
    return *this;
}

MessagingContainerBuilder& MessagingContainerBuilder::trace_id(std::string id) {
    trace_id_ = std::move(id);
    return *this;
}

MessagingContainerBuilder& MessagingContainerBuilder::add_value(const std::string& key, value val) {
    values_[key] = std::move(val);
    return *this;
}

MessagingContainerBuilder& MessagingContainerBuilder::optimize_for_speed() {
    optimize_speed_ = true;
    return *this;
}

Result<MessagingContainer> MessagingContainerBuilder::build() {
    auto result = MessagingContainer::create(source_, target_, topic_);
    if (result.is_error()) {
        return result;
    }

    auto container = result.value();

    if (!trace_id_.empty()) {
        container.container().set_value("trace_id", trace_id_);
    }

    for (const auto& [key, val] : values_) {
        container.container().set_value(key, val);
    }

    return Result<MessagingContainer>::ok(std::move(container));
}

} // namespace messaging
