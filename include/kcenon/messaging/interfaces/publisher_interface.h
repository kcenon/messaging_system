// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#pragma once

#include <kcenon/common/patterns/result.h>
#include <string>
#include <memory>

namespace kcenon::messaging {

class message;

/**
 * @interface publisher_interface
 * @brief Abstract interface for message publishers
 *
 * This interface defines the contract for publishing messages to the messaging system.
 * Implementations can vary from simple in-memory publishers to distributed message brokers.
 */
class publisher_interface {
public:
    virtual ~publisher_interface() = default;

    /**
     * @brief Publish a message to a topic
     * @param topic Topic to publish to
     * @param msg Message to publish
     * @return Result indicating success or error
     *
     * The topic can be a simple string or a pattern depending on the
     * routing implementation. Messages are delivered to all matching subscribers.
     */
    virtual common::VoidResult publish(
        const std::string& topic,
        const message& msg
    ) = 0;

    /**
     * @brief Check if publisher is ready
     * @return true if ready to publish
     *
     * Publishers may need initialization or connection establishment before
     * they can publish messages. This method allows clients to check readiness.
     */
    virtual bool is_ready() const = 0;
};

} // namespace kcenon::messaging
