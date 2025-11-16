// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#pragma once

#include <kcenon/common/patterns/result.h>
#include <string>
#include <functional>
#include <cstdint>

namespace kcenon::messaging {

class message;

/**
 * @brief Subscription callback function type
 */
using subscription_callback = std::function<common::VoidResult(const message&)>;

/**
 * @brief Message filter function type
 */
using message_filter = std::function<bool(const message&)>;

/**
 * @interface subscriber_interface
 * @brief Abstract interface for message subscribers
 *
 * This interface defines the contract for subscribing to messages in the messaging system.
 * Subscribers can register callbacks that will be invoked when matching messages are published.
 */
class subscriber_interface {
public:
    virtual ~subscriber_interface() = default;

    /**
     * @brief Subscribe to a topic pattern
     * @param topic_pattern Topic pattern (may support wildcards)
     * @param callback Callback to invoke for matching messages
     * @param filter Optional message filter for additional filtering
     * @param priority Subscription priority (higher = processed first)
     * @return Subscription ID for later unsubscription, or error
     *
     * Topic patterns may support wildcards depending on the implementation:
     * - "user.*" matches "user.created", "user.updated"
     * - "user.#" matches "user.created", "user.profile.updated"
     */
    virtual common::Result<uint64_t> subscribe(
        const std::string& topic_pattern,
        subscription_callback callback,
        message_filter filter = nullptr,
        int priority = 5
    ) = 0;

    /**
     * @brief Unsubscribe from a topic
     * @param subscription_id Subscription ID returned from subscribe()
     * @return Result indicating success or error
     */
    virtual common::VoidResult unsubscribe(uint64_t subscription_id) = 0;

    /**
     * @brief Check if subscriber is active
     * @return true if subscriber can receive messages
     */
    virtual bool is_active() const = 0;
};

} // namespace kcenon::messaging
