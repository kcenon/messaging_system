// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file error_codes.h
 * @brief Error codes for messaging_system
 *
 * messaging_system uses error code range -700 to -799.
 * This range is independent of common_system error codes as
 * messaging_system is an extension module built on top of
 * common infrastructure systems.
 *
 * Error Code Organization:
 * - -700 to -719: Message errors
 * - -720 to -739: Routing errors
 * - -740 to -759: Queue errors
 * - -760 to -779: Subscription errors
 * - -780 to -799: Publishing errors
 */

#pragma once

#include <string_view>

namespace kcenon::messaging {
namespace error {

/**
 * @brief Base value for all messaging_system error codes
 */
constexpr int base = -700;

// ============================================================================
// Message Errors (-700 to -719)
// ============================================================================

constexpr int invalid_message = base - 0;        // -700
constexpr int message_too_large = base - 1;      // -701
constexpr int message_expired = base - 2;        // -702
constexpr int invalid_payload = base - 3;        // -703
constexpr int message_serialization_failed = base - 4;  // -704
constexpr int message_deserialization_failed = base - 5; // -705

// ============================================================================
// Routing Errors (-720 to -739)
// ============================================================================

constexpr int routing_failed = base - 20;        // -720
constexpr int unknown_topic = base - 21;         // -721
constexpr int no_subscribers = base - 22;        // -722
constexpr int invalid_topic_pattern = base - 23; // -723
constexpr int route_not_found = base - 24;       // -724

// ============================================================================
// Queue Errors (-740 to -759)
// ============================================================================

constexpr int queue_full = base - 40;            // -740
constexpr int queue_empty = base - 41;           // -741
constexpr int queue_stopped = base - 42;         // -742
constexpr int enqueue_failed = base - 43;        // -743
constexpr int dequeue_failed = base - 44;        // -744
constexpr int queue_timeout = base - 45;         // -745

// ============================================================================
// Subscription Errors (-760 to -779)
// ============================================================================

constexpr int subscription_failed = base - 60;    // -760
constexpr int subscription_not_found = base - 61; // -761
constexpr int duplicate_subscription = base - 62; // -762
constexpr int unsubscribe_failed = base - 63;     // -763
constexpr int invalid_subscription = base - 64;   // -764

// ============================================================================
// Publishing Errors (-780 to -799)
// ============================================================================

constexpr int publication_failed = base - 80;     // -780
constexpr int no_route_found = base - 81;         // -781
constexpr int message_rejected = base - 82;       // -782
constexpr int broker_unavailable = base - 83;     // -783
constexpr int broker_not_started = base - 84;     // -784

// ============================================================================
// Error Message Lookup
// ============================================================================

/**
 * @brief Get human-readable error message for error code
 * @param code Error code
 * @return Error message string
 */
inline std::string_view get_error_message(int code) {
    switch (code) {
        // Message errors
        case invalid_message: return "Invalid message";
        case message_too_large: return "Message too large";
        case message_expired: return "Message expired";
        case invalid_payload: return "Invalid message payload";
        case message_serialization_failed: return "Message serialization failed";
        case message_deserialization_failed: return "Message deserialization failed";

        // Routing errors
        case routing_failed: return "Message routing failed";
        case unknown_topic: return "Unknown topic";
        case no_subscribers: return "No subscribers for topic";
        case invalid_topic_pattern: return "Invalid topic pattern";
        case route_not_found: return "Route not found";

        // Queue errors
        case queue_full: return "Message queue full";
        case queue_empty: return "Message queue empty";
        case queue_stopped: return "Message queue stopped";
        case enqueue_failed: return "Failed to enqueue message";
        case dequeue_failed: return "Failed to dequeue message";
        case queue_timeout: return "Queue operation timeout";

        // Subscription errors
        case subscription_failed: return "Subscription failed";
        case subscription_not_found: return "Subscription not found";
        case duplicate_subscription: return "Duplicate subscription";
        case unsubscribe_failed: return "Unsubscribe failed";
        case invalid_subscription: return "Invalid subscription";

        // Publishing errors
        case publication_failed: return "Publication failed";
        case no_route_found: return "No route found for message";
        case message_rejected: return "Message rejected";
        case broker_unavailable: return "Message broker unavailable";
        case broker_not_started: return "Message broker not started";

        default: return "Unknown messaging error";
    }
}

// ============================================================================
// Compile-time Range Validation
// ============================================================================

namespace validation {
    // Validate all error codes are within the allocated range
    static_assert(invalid_message >= -799 && invalid_message <= -700,
                  "messaging_system error codes must be in range [-799, -700]");
    static_assert(message_deserialization_failed >= -799 && message_deserialization_failed <= -700,
                  "messaging_system error codes must be in range [-799, -700]");
    static_assert(route_not_found >= -799 && route_not_found <= -700,
                  "messaging_system error codes must be in range [-799, -700]");
    static_assert(queue_timeout >= -799 && queue_timeout <= -700,
                  "messaging_system error codes must be in range [-799, -700]");
    static_assert(invalid_subscription >= -799 && invalid_subscription <= -700,
                  "messaging_system error codes must be in range [-799, -700]");
    static_assert(broker_not_started >= -799 && broker_not_started <= -700,
                  "messaging_system error codes must be in range [-799, -700]");
} // namespace validation

} // namespace error
} // namespace kcenon::messaging
