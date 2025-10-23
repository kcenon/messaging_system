#pragma once

#include <core/container.h>
#include <kcenon/common/patterns/result.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace messaging {

using common::Result;
using common::VoidResult;
using container_module::value;
using container_module::value_container;
using container_module::value_types;

class MessagingContainer {
    std::shared_ptr<value_container> container_;

public:
    MessagingContainer();

    // Factory method
    static Result<MessagingContainer> create(
        const std::string& source,
        const std::string& target,
        const std::string& topic
    );

    // Accessors
    std::string source() const;
    std::string target() const;
    std::string topic() const;
    std::string trace_id() const;

    // Serialization
    Result<std::vector<uint8_t>> serialize() const;
    static Result<MessagingContainer> deserialize(const std::vector<uint8_t>& data);

    // Underlying container access
    const value_container& container() const { return *container_; }
    value_container& container() { return *container_; }
};

class MessagingContainerBuilder {
    std::string source_;
    std::string target_;
    std::string topic_;
    std::string trace_id_;
    std::vector<std::shared_ptr<value>> values_;
    bool optimize_speed_{false};

public:
    MessagingContainerBuilder& source(std::string s);
    MessagingContainerBuilder& target(std::string t);
    MessagingContainerBuilder& topic(std::string topic);
    MessagingContainerBuilder& trace_id(std::string id);
    MessagingContainerBuilder& add_value(std::shared_ptr<value> val);
    MessagingContainerBuilder& optimize_for_speed();

    Result<MessagingContainer> build();
};

} // namespace messaging
