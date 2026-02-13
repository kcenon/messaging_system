// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file messaging_error_category.h
 * @brief Error category for messaging_system typed error codes
 *
 * Provides a messaging-specific error category that integrates with
 * common_system's typed_error_code infrastructure. This enables:
 * - Type-safe error codes that carry their origin category
 * - Direct integration with Result<T> via typed_error_code
 * - Clear identification of messaging errors vs other system errors
 *
 * Usage Example:
 * @code
 * #include <kcenon/messaging/error/messaging_error_category.h>
 *
 * // Preferred: use codes enum with make_typed_error_code
 * auto ec = make_typed_error_code(messaging_error_category::queue_full);
 *
 * // Use with Result<T>
 * return Result<int>::err(ec);
 *
 * // Alternative: use make_messaging_error_code with int constants
 * auto ec2 = make_messaging_error_code(error::queue_full);
 *
 * // Check category
 * if (ec.category() == messaging_error_category::instance()) {
 *     // Handle messaging-specific error
 * }
 * @endcode
 *
 * @see error_codes.h for error code constants
 * @see common_system's error_category.h for base class
 */

#pragma once

#include "error_codes.h"

#include <kcenon/common/error/error_category.h>

namespace kcenon::messaging {

/**
 * @class messaging_error_category
 * @brief Error category for messaging_system error codes
 *
 * Singleton error category that maps messaging error codes (-700 to -799)
 * to human-readable messages. Inherits from common::error_category and
 * follows the same pattern as common_error_category.
 *
 * Thread Safety:
 * - Stateless singleton, safe for concurrent access
 * - C++11 guarantees thread-safe static local initialization
 */
class messaging_error_category : public common::error_category {
public:
    /**
     * @brief Typed error code enumeration for messaging_system
     *
     * Mirrors the constexpr int values from error_codes.h, enabling
     * type-safe error code construction via make_typed_error_code().
     * Follows the same pattern as common_error_category::codes.
     *
     * @code
     * auto ec = make_typed_error_code(messaging_error_category::queue_full);
     * return Result<int>::err(ec);
     * @endcode
     */
    enum codes : int {
        // Message errors (-700 to -719)
        invalid_message = error::invalid_message,
        message_too_large = error::message_too_large,
        message_expired = error::message_expired,
        invalid_payload = error::invalid_payload,
        message_serialization_failed = error::message_serialization_failed,
        message_deserialization_failed = error::message_deserialization_failed,

        // Task errors (-706 to -715)
        task_not_found = error::task_not_found,
        task_already_running = error::task_already_running,
        task_cancelled = error::task_cancelled,
        task_timeout = error::task_timeout,
        task_failed = error::task_failed,
        task_handler_not_found = error::task_handler_not_found,
        task_spawner_not_configured = error::task_spawner_not_configured,
        task_invalid_argument = error::task_invalid_argument,
        task_operation_failed = error::task_operation_failed,
        schedule_already_exists = error::schedule_already_exists,

        // Routing errors (-720 to -739)
        routing_failed = error::routing_failed,
        unknown_topic = error::unknown_topic,
        no_subscribers = error::no_subscribers,
        invalid_topic_pattern = error::invalid_topic_pattern,
        route_not_found = error::route_not_found,

        // Queue errors (-740 to -759)
        queue_full = error::queue_full,
        queue_empty = error::queue_empty,
        queue_stopped = error::queue_stopped,
        enqueue_failed = error::enqueue_failed,
        dequeue_failed = error::dequeue_failed,
        queue_timeout = error::queue_timeout,
        dlq_full = error::dlq_full,
        dlq_empty = error::dlq_empty,
        dlq_message_not_found = error::dlq_message_not_found,
        dlq_replay_failed = error::dlq_replay_failed,
        dlq_not_configured = error::dlq_not_configured,

        // Subscription errors (-760 to -779)
        subscription_failed = error::subscription_failed,
        subscription_not_found = error::subscription_not_found,
        duplicate_subscription = error::duplicate_subscription,
        unsubscribe_failed = error::unsubscribe_failed,
        invalid_subscription = error::invalid_subscription,

        // Publishing errors (-780 to -799)
        publication_failed = error::publication_failed,
        no_route_found = error::no_route_found,
        message_rejected = error::message_rejected,
        broker_unavailable = error::broker_unavailable,
        broker_not_started = error::broker_not_started,
        already_running = error::already_running,
        not_running = error::not_running,
        backend_not_ready = error::backend_not_ready,
        request_timeout = error::request_timeout,
        not_supported = error::not_supported,

        // Transport errors (-790 to -794)
        connection_failed = error::connection_failed,
        send_timeout = error::send_timeout,
        receive_timeout = error::receive_timeout,
        authentication_failed = error::authentication_failed,
        not_connected = error::not_connected,
    };

    /**
     * @brief Returns the singleton instance
     *
     * Thread-safe due to C++11 static local variable initialization.
     *
     * @return Reference to the singleton instance
     */
    static const messaging_error_category& instance() noexcept {
        static messaging_error_category inst;
        return inst;
    }

    /**
     * @brief Returns the category name
     * @return "messaging"
     */
    std::string_view name() const noexcept override {
        return "messaging";
    }

    /**
     * @brief Returns a human-readable message for the error code
     *
     * Delegates to error::get_error_message() which contains the
     * canonical mapping for all messaging error codes.
     *
     * @param code Error code value (expected range: -700 to -799)
     * @return Human-readable error message
     */
    std::string message(int code) const override {
        return std::string(error::get_error_message(code));
    }

private:
    messaging_error_category() = default;
};

/**
 * @brief Create a typed_error_code for a messaging error code
 *
 * Convenience function that wraps an integer error code with the
 * messaging_error_category.
 *
 * @param code Messaging error code (from error:: namespace)
 * @return typed_error_code with messaging_error_category
 *
 * @code
 * auto ec = make_messaging_error_code(error::queue_full);
 * // ec.category().name() == "messaging"
 * // ec.message() == "Message queue full"
 * @endcode
 */
inline common::typed_error_code make_messaging_error_code(int code) noexcept {
    return common::typed_error_code(code, messaging_error_category::instance());
}

/**
 * @brief Create a typed_error_code from messaging_error_category::codes enum
 *
 * ADL-discoverable overload that follows the common_system pattern.
 * Enables type-safe error code construction without specifying the category.
 *
 * @param code Messaging error code enum value
 * @return typed_error_code with messaging_error_category
 *
 * @code
 * auto ec = make_typed_error_code(messaging_error_category::queue_full);
 * return Result<int>::err(ec);
 * @endcode
 */
inline common::typed_error_code make_typed_error_code(
    messaging_error_category::codes code) noexcept {
    return common::typed_error_code(
        static_cast<int>(code), messaging_error_category::instance());
}

} // namespace kcenon::messaging
