#include "messaging_system/core/messaging_container.h"
#include "messaging_system/error_codes.h"
#include <kcenon/common/patterns/result.h>
#include <container/values/string_value.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace messaging {

MessagingContainer::MessagingContainer()
    : container_(std::make_shared<value_container>()) {}

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
    try {
        if (topic.empty()) {
            return common::make_error<MessagingContainer>(
                common::error_info{
                    error::INVALID_MESSAGE,
                    "Topic cannot be empty",
                    "MessagingContainer::create",
                    ""
                }
            );
        }

        MessagingContainer container;

        auto serialize_entry = [](const std::string& key, const std::string& data) {
            container_module::string_value value(key, data);
            return value.serialize();
        };

        std::string serialized = "@data={{";
        serialized += serialize_entry("source", source);
        serialized += serialize_entry("target", target);
        serialized += serialize_entry("topic", topic);
        serialized += serialize_entry("trace_id", generate_uuid());
        serialized += "}};";

        container.container_->deserialize(serialized, false);

        return common::ok(std::move(container));
    } catch (const std::exception& e) {
        std::cerr << "MessagingContainer::create failed: " << e.what() << std::endl;
        throw;
    }
}

std::string MessagingContainer::source() const {
    auto val = container_->get_value("source");
    return val ? val->data() : "";
}

std::string MessagingContainer::target() const {
    auto val = container_->get_value("target");
    return val ? val->data() : "";
}

std::string MessagingContainer::topic() const {
    auto val = container_->get_value("topic");
    return val ? val->data() : "";
}

std::string MessagingContainer::trace_id() const {
    auto val = container_->get_value("trace_id");
    return val ? val->data() : "";
}

Result<std::vector<uint8_t>> MessagingContainer::serialize() const {
    try {
        auto serialized = container_->serialize_array();
        return common::ok(std::move(serialized));
    } catch (const std::exception& e) {
        return common::make_error<std::vector<uint8_t>>(
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
        return common::make_error<MessagingContainer>(
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
        // value_container constructor can deserialize from byte array
        std::string data_str(data.begin(), data.end());
        container.container_ = std::make_shared<value_container>(data_str, false);

        // Validate required fields
        auto topic_val = container.container_->get_value("topic");
        if (!topic_val || topic_val->data().empty()) {
            return common::make_error<MessagingContainer>(
                common::error_info{
                    error::INVALID_MESSAGE,
                    "Deserialized container missing required 'topic' field",
                    "MessagingContainer::deserialize",
                    ""
                }
            );
        }

        return common::ok(std::move(container));
    } catch (const std::exception& e) {
        return common::make_error<MessagingContainer>(
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

MessagingContainerBuilder& MessagingContainerBuilder::add_value(std::shared_ptr<value> val) {
    if (val) {
        values_.push_back(std::move(val));
    }
    return *this;
}

MessagingContainerBuilder& MessagingContainerBuilder::optimize_for_speed() {
    optimize_speed_ = true;
    return *this;
}

Result<MessagingContainer> MessagingContainerBuilder::build() {
    auto result = MessagingContainer::create(source_, target_, topic_);
    if (result.is_err()) {
        return result;
    }

    auto container = result.unwrap();

    if (!trace_id_.empty()) {
        container.container().add(
            std::make_shared<container_module::string_value>("trace_id", trace_id_), true);
    }

    for (const auto& val : values_) {
        if (val) {
            container.container().add(val, true);
        }
    }

    return common::ok(std::move(container));
}

} // namespace messaging
