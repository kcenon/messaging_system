// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file event_bridge.h
 * @brief Bridge between messaging_system and common_system event bus
 *
 * This header provides integration between the messaging_system's message bus
 * and common_system's event bus, enabling cross-module event communication.
 */

#pragma once

#include <kcenon/common/patterns/event_bus.h>
#include <kcenon/messaging/error/messaging_error_category.h>
#include "../core/message_bus.h"
#include <chrono>
#include <memory>
#include <string>

namespace kcenon::messaging::integration {

// ============================================================================
// Messaging Events
// ============================================================================

/**
 * @struct message_received_event
 * @brief Event published when a message is received
 */
struct message_received_event {
    std::string topic;
    std::string message_id;
    message_type type;
    std::chrono::system_clock::time_point timestamp;

    message_received_event(const message& msg)
        : topic(msg.metadata().topic),
          message_id(msg.metadata().id),
          type(msg.metadata().type),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct message_published_event
 * @brief Event published when a message is published
 */
struct message_published_event {
    std::string topic;
    std::string message_id;
    size_t subscriber_count;
    std::chrono::system_clock::time_point timestamp;

    message_published_event(const std::string& t, const std::string& id, size_t count = 0)
        : topic(t),
          message_id(id),
          subscriber_count(count),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct message_error_event
 * @brief Event published when a message processing error occurs
 */
struct message_error_event {
    std::string topic;
    std::string message_id;
    std::string error_message;
    int error_code;
    std::chrono::system_clock::time_point timestamp;

    message_error_event(const std::string& t, const std::string& id,
                        const std::string& err_msg, int code = -1)
        : topic(t),
          message_id(id),
          error_message(err_msg),
          error_code(code),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct message_bus_started_event
 * @brief Event published when message bus starts
 */
struct message_bus_started_event {
    size_t worker_count;
    std::chrono::system_clock::time_point timestamp;

    explicit message_bus_started_event(size_t workers)
        : worker_count(workers),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct message_bus_stopped_event
 * @brief Event published when message bus stops
 */
struct message_bus_stopped_event {
    uint64_t total_messages_processed;
    std::chrono::system_clock::time_point timestamp;

    explicit message_bus_stopped_event(uint64_t processed = 0)
        : total_messages_processed(processed),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct topic_created_event
 * @brief Event published when a new topic is created
 *
 * This event is published when a subscription creates a new topic pattern
 * that didn't exist before in the topic router.
 */
struct topic_created_event {
    std::string topic_pattern;
    std::chrono::system_clock::time_point timestamp;

    explicit topic_created_event(const std::string& pattern)
        : topic_pattern(pattern),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct subscriber_added_event
 * @brief Event published when a subscriber is added to a topic
 */
struct subscriber_added_event {
    uint64_t subscription_id;
    std::string topic_pattern;
    int priority;
    std::chrono::system_clock::time_point timestamp;

    subscriber_added_event(uint64_t id, const std::string& pattern, int prio)
        : subscription_id(id),
          topic_pattern(pattern),
          priority(prio),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct subscriber_removed_event
 * @brief Event published when a subscriber is removed from a topic
 */
struct subscriber_removed_event {
    uint64_t subscription_id;
    std::string topic_pattern;
    std::chrono::system_clock::time_point timestamp;

    subscriber_removed_event(uint64_t id, const std::string& pattern)
        : subscription_id(id),
          topic_pattern(pattern),
          timestamp(std::chrono::system_clock::now()) {}
};

// ============================================================================
// Event Bridge
// ============================================================================

/**
 * @class messaging_event_bridge
 * @brief Bridge between messaging_system and common_system event bus
 *
 * This class subscribes to message bus events and publishes them to the
 * common_system event bus, enabling other modules to react to messaging events.
 *
 * Example usage:
 * @code
 * auto bus = std::make_shared<message_bus>(...);
 * messaging_event_bridge bridge(bus);
 * bridge.start();
 *
 * // Subscribe to messaging events via common event bus
 * auto& event_bus = common::get_event_bus();
 * event_bus.subscribe<message_received_event>([](const auto& evt) {
 *     std::cout << "Received message on topic: " << evt.topic << std::endl;
 * });
 * @endcode
 */
class messaging_event_bridge {
public:
    /**
     * @brief Virtual destructor for polymorphic type support
     */
    virtual ~messaging_event_bridge() = default;

    /**
     * @brief Construct event bridge with message bus
     * @param bus Shared pointer to message bus
     */
    explicit messaging_event_bridge(std::shared_ptr<message_bus> bus)
        : bus_(std::move(bus)),
          event_bus_(common::get_event_bus()) {}

    /**
     * @brief Start the event bridge
     * @return VoidResult indicating success or error
     */
    common::VoidResult start() {
        if (running_) {
            return common::VoidResult::err(
                make_typed_error_code(messaging_error_category::already_running));
        }

        running_ = true;
        event_bus_.publish(message_bus_started_event{bus_->worker_count()});
        return common::ok();
    }

    /**
     * @brief Stop the event bridge
     */
    void stop() {
        if (running_) {
            auto stats = bus_->get_statistics();
            event_bus_.publish(message_bus_stopped_event{stats.messages_processed});
            running_ = false;
        }
    }

    /**
     * @brief Notify that a message was received
     * @param msg The received message
     */
    void on_message_received(const message& msg) {
        if (running_) {
            event_bus_.publish(message_received_event{msg});
        }
    }

    /**
     * @brief Notify that a message was published
     * @param topic Topic the message was published to
     * @param message_id ID of the published message
     * @param subscriber_count Number of subscribers that received the message
     */
    void on_message_published(const std::string& topic,
                               const std::string& message_id,
                               size_t subscriber_count = 0) {
        if (running_) {
            event_bus_.publish(message_published_event{topic, message_id, subscriber_count});
        }
    }

    /**
     * @brief Notify that a message processing error occurred
     * @param topic Topic of the message
     * @param message_id ID of the message
     * @param error_message Error description
     * @param error_code Error code
     */
    void on_message_error(const std::string& topic,
                          const std::string& message_id,
                          const std::string& error_message,
                          int error_code = -1) {
        if (running_) {
            event_bus_.publish(message_error_event{topic, message_id, error_message, error_code});
        }
    }

    /**
     * @brief Notify that a new topic was created
     * @param topic_pattern The topic pattern that was created
     */
    void on_topic_created(const std::string& topic_pattern) {
        if (running_) {
            event_bus_.publish(topic_created_event{topic_pattern});
        }
    }

    /**
     * @brief Notify that a subscriber was added
     * @param subscription_id ID of the new subscription
     * @param topic_pattern Topic pattern subscribed to
     * @param priority Subscription priority
     */
    void on_subscriber_added(uint64_t subscription_id,
                              const std::string& topic_pattern,
                              int priority) {
        if (running_) {
            event_bus_.publish(subscriber_added_event{subscription_id, topic_pattern, priority});
        }
    }

    /**
     * @brief Notify that a subscriber was removed
     * @param subscription_id ID of the removed subscription
     * @param topic_pattern Topic pattern that was unsubscribed
     */
    void on_subscriber_removed(uint64_t subscription_id,
                                const std::string& topic_pattern) {
        if (running_) {
            event_bus_.publish(subscriber_removed_event{subscription_id, topic_pattern});
        }
    }

    /**
     * @brief Check if bridge is running
     * @return true if running
     */
    bool is_running() const { return running_; }

    /**
     * @brief Get the event bus reference
     * @return Reference to the common event bus
     */
    common::simple_event_bus& get_event_bus() { return event_bus_; }

private:
    std::shared_ptr<message_bus> bus_;
    common::simple_event_bus& event_bus_;
    std::atomic<bool> running_{false};
};

}  // namespace kcenon::messaging::integration
