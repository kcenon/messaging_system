// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file core.cppm
 * @brief Core partition for messaging_system module.
 *
 * This partition provides core messaging infrastructure including
 * message types, message queue, message bus, and message broker.
 *
 * Contents:
 * - message: Core message structure with metadata and payload
 * - message_builder: Builder pattern for message construction
 * - message_queue: Thread-safe message queue
 * - message_bus: Global message routing and dispatch
 * - message_broker: Advanced message routing with subscriptions
 * - topic_router: Topic-based message routing with wildcards
 * - priority: Message priority levels
 *
 * @see kcenon.messaging for the primary module interface
 */

module;

// =============================================================================
// Global Module Fragment - Standard Library Headers
// =============================================================================
#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Third-party headers (container_system)
#include <core/container.h>

export module kcenon.messaging:core;

export import kcenon.common;
export import kcenon.thread;
export import kcenon.container;

// =============================================================================
// Forward Declarations
// =============================================================================

export namespace kcenon::messaging {

class message;
class message_builder;
class message_queue;
class message_bus;
class message_broker;
class topic_router;

} // namespace kcenon::messaging

// =============================================================================
// Priority Definitions
// =============================================================================

export namespace kcenon::messaging {

/**
 * @enum message_priority
 * @brief Message priority levels for queue ordering
 */
enum class message_priority {
    lowest = 0,     ///< Lowest priority - processed last
    low = 1,        ///< Low priority
    normal = 2,     ///< Normal priority (default)
    high = 3,       ///< High priority
    highest = 4,    ///< Highest priority - processed first
    critical = 5    ///< Critical - immediate processing
};

/**
 * @brief Convert priority to string representation
 * @param priority The priority level
 * @return String representation
 */
constexpr std::string_view to_string(message_priority priority) noexcept {
    switch (priority) {
        case message_priority::lowest: return "lowest";
        case message_priority::low: return "low";
        case message_priority::normal: return "normal";
        case message_priority::high: return "high";
        case message_priority::highest: return "highest";
        case message_priority::critical: return "critical";
        default: return "unknown";
    }
}

/**
 * @brief Parse string to message_priority
 * @param str String representation
 * @return Parsed priority or normal as default
 */
message_priority priority_from_string(std::string_view str) noexcept;

} // namespace kcenon::messaging

// =============================================================================
// Message Types and Metadata
// =============================================================================

export namespace kcenon::messaging {

/**
 * @enum message_type
 * @brief Message type classification
 */
enum class message_type {
    command,        ///< Execute an action
    event,          ///< Something happened (notification)
    query,          ///< Request information
    reply,          ///< Response to query/command
    notification    ///< Informational message
};

/**
 * @brief Convert message_type to string representation
 * @param type The message type
 * @return String representation
 */
constexpr std::string_view to_string(message_type type) noexcept {
    switch (type) {
        case message_type::command: return "command";
        case message_type::event: return "event";
        case message_type::query: return "query";
        case message_type::reply: return "reply";
        case message_type::notification: return "notification";
        default: return "unknown";
    }
}

/**
 * @struct message_metadata
 * @brief Message metadata and headers
 */
struct message_metadata {
    std::string id;                 ///< Unique message ID
    std::string topic;              ///< Topic/channel for routing
    std::string source;             ///< Source service/component
    std::string target;             ///< Target service/component (optional)
    std::string correlation_id;     ///< For request/reply correlation
    std::string trace_id;           ///< Distributed tracing ID

    message_type type = message_type::event;
    message_priority priority = message_priority::normal;

    std::chrono::system_clock::time_point timestamp;
    std::optional<std::chrono::milliseconds> ttl;  ///< Time-to-live

    /// Additional headers (key-value pairs)
    std::unordered_map<std::string, std::string> headers;
};

/**
 * @class message
 * @brief Core message structure using container_system
 *
 * Messages are the fundamental unit of communication in the messaging system.
 * Each message has metadata for routing and a payload for data.
 */
class message {
    friend class message_builder;

public:
    message();
    explicit message(const std::string& topic);
    message(const std::string& topic, message_type type);

    // Metadata access
    const message_metadata& metadata() const noexcept { return metadata_; }
    message_metadata& metadata() noexcept { return metadata_; }

    // Payload access
    const container_module::value_container& payload() const;
    container_module::value_container& payload();

    // Convenience methods
    bool is_expired() const noexcept;
    std::chrono::milliseconds age() const noexcept;

    // Serialization
    kcenon::common::Result<std::vector<uint8_t>> serialize() const;
    static kcenon::common::Result<message> deserialize(const std::vector<uint8_t>& data);

protected:
    void set_payload(std::shared_ptr<container_module::value_container> payload);

private:
    message_metadata metadata_;
    std::shared_ptr<container_module::value_container> payload_;
};

/**
 * @class message_builder
 * @brief Builder pattern for message construction
 *
 * Provides a fluent interface for constructing messages with proper validation.
 */
class message_builder {
public:
    message_builder();

    message_builder& topic(std::string topic);
    message_builder& source(std::string source);
    message_builder& target(std::string target);
    message_builder& type(message_type type);
    message_builder& priority(message_priority priority);
    message_builder& ttl(std::chrono::milliseconds ttl);
    message_builder& correlation_id(std::string id);
    message_builder& trace_id(std::string id);
    message_builder& header(std::string key, std::string value);
    message_builder& payload(std::shared_ptr<container_module::value_container> payload);

    kcenon::common::Result<message> build();

private:
    message msg_;
};

} // namespace kcenon::messaging

// =============================================================================
// Message Queue
// =============================================================================

export namespace kcenon::messaging {

/**
 * @class message_queue
 * @brief Thread-safe priority message queue
 *
 * Provides a thread-safe queue for messages with priority ordering.
 * Higher priority messages are dequeued first.
 */
class message_queue {
public:
    explicit message_queue(size_t max_size = 10000);
    ~message_queue();

    // Non-copyable
    message_queue(const message_queue&) = delete;
    message_queue& operator=(const message_queue&) = delete;

    // Movable
    message_queue(message_queue&&) noexcept;
    message_queue& operator=(message_queue&&) noexcept;

    /**
     * @brief Enqueue a message
     * @param msg Message to enqueue
     * @return true if enqueued, false if queue is full
     */
    bool enqueue(message msg);

    /**
     * @brief Try to dequeue a message
     * @return Message if available, nullopt otherwise
     */
    std::optional<message> try_dequeue();

    /**
     * @brief Wait for and dequeue a message
     * @param timeout Maximum time to wait
     * @return Message if available within timeout, nullopt otherwise
     */
    std::optional<message> wait_dequeue(std::chrono::milliseconds timeout);

    /**
     * @brief Get current queue size
     */
    size_t size() const noexcept;

    /**
     * @brief Check if queue is empty
     */
    bool empty() const noexcept;

    /**
     * @brief Clear all messages from the queue
     */
    void clear();

private:
    struct message_comparator {
        bool operator()(const message& a, const message& b) const;
    };

    std::priority_queue<message, std::vector<message>, message_comparator> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    size_t max_size_;
    std::atomic<bool> stopped_{false};
};

} // namespace kcenon::messaging

// =============================================================================
// Topic Router
// =============================================================================

export namespace kcenon::messaging {

/**
 * @class topic_router
 * @brief Topic-based message routing with wildcard support
 *
 * Supports hierarchical topics with wildcards:
 * - '*' matches a single level (e.g., "orders.*" matches "orders.created")
 * - '#' matches multiple levels (e.g., "orders.#" matches "orders.eu.created")
 */
class topic_router {
public:
    using handler_fn = std::function<void(const message&)>;

    topic_router();
    ~topic_router();

    /**
     * @brief Subscribe a handler to a topic pattern
     * @param pattern Topic pattern (supports * and # wildcards)
     * @param handler Handler function
     * @return Subscription ID for unsubscribing
     */
    std::string subscribe(const std::string& pattern, handler_fn handler);

    /**
     * @brief Unsubscribe a handler
     * @param subscription_id ID returned from subscribe
     * @return true if unsubscribed, false if not found
     */
    bool unsubscribe(const std::string& subscription_id);

    /**
     * @brief Route a message to matching handlers
     * @param msg Message to route
     * @return Number of handlers that received the message
     */
    size_t route(const message& msg);

    /**
     * @brief Check if a topic matches a pattern
     * @param topic The topic to check
     * @param pattern The pattern to match against
     * @return true if matches
     */
    static bool matches(const std::string& topic, const std::string& pattern);

private:
    struct subscription {
        std::string id;
        std::string pattern;
        handler_fn handler;
    };

    std::vector<subscription> subscriptions_;
    mutable std::shared_mutex mutex_;
    std::atomic<uint64_t> next_id_{0};
};

} // namespace kcenon::messaging

// =============================================================================
// Message Bus
// =============================================================================

export namespace kcenon::messaging {

/**
 * @class message_bus
 * @brief Global message routing and dispatch
 *
 * Provides a singleton message bus for decoupled communication
 * between components.
 */
class message_bus {
public:
    /**
     * @brief Get the singleton instance
     */
    static message_bus& instance();

    using handler_fn = std::function<void(const message&)>;

    /**
     * @brief Subscribe to messages on a topic
     * @param topic Topic pattern (supports wildcards)
     * @param handler Handler function
     * @return Subscription ID
     */
    std::string subscribe(const std::string& topic, handler_fn handler);

    /**
     * @brief Unsubscribe from a topic
     * @param subscription_id ID returned from subscribe
     * @return true if unsubscribed
     */
    bool unsubscribe(const std::string& subscription_id);

    /**
     * @brief Publish a message to the bus
     * @param msg Message to publish
     * @return Number of handlers that received the message
     */
    size_t publish(const message& msg);

    /**
     * @brief Publish a message asynchronously
     * @param msg Message to publish
     * @return Future that resolves to the number of handlers
     */
    std::future<size_t> publish_async(message msg);

    /**
     * @brief Get the number of active subscriptions
     */
    size_t subscription_count() const;

    /**
     * @brief Clear all subscriptions
     */
    void clear();

private:
    message_bus();
    ~message_bus() = default;

    message_bus(const message_bus&) = delete;
    message_bus& operator=(const message_bus&) = delete;

    topic_router router_;
};

} // namespace kcenon::messaging

// =============================================================================
// Message Broker
// =============================================================================

export namespace kcenon::messaging {

/**
 * @class message_broker
 * @brief Advanced message routing with subscriptions and queues
 *
 * Provides more advanced messaging features including:
 * - Named queues
 * - Durable subscriptions
 * - Message persistence
 */
class message_broker {
public:
    explicit message_broker(const std::string& name = "default");
    ~message_broker();

    // Non-copyable
    message_broker(const message_broker&) = delete;
    message_broker& operator=(const message_broker&) = delete;

    /**
     * @brief Get the broker name
     */
    const std::string& name() const noexcept { return name_; }

    /**
     * @brief Create a named queue
     * @param queue_name Name of the queue
     * @param max_size Maximum queue size
     * @return true if created, false if already exists
     */
    bool create_queue(const std::string& queue_name, size_t max_size = 10000);

    /**
     * @brief Delete a named queue
     * @param queue_name Name of the queue
     * @return true if deleted, false if not found
     */
    bool delete_queue(const std::string& queue_name);

    /**
     * @brief Publish a message to a queue
     * @param queue_name Target queue name
     * @param msg Message to publish
     * @return true if published, false if queue not found or full
     */
    bool publish(const std::string& queue_name, message msg);

    /**
     * @brief Consume a message from a queue
     * @param queue_name Source queue name
     * @param timeout Maximum time to wait
     * @return Message if available, nullopt otherwise
     */
    std::optional<message> consume(const std::string& queue_name,
                                   std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    /**
     * @brief Get the number of messages in a queue
     * @param queue_name Queue name
     * @return Number of messages, or 0 if queue not found
     */
    size_t queue_size(const std::string& queue_name) const;

    /**
     * @brief Get list of queue names
     */
    std::vector<std::string> queue_names() const;

    /**
     * @brief Start the broker
     */
    void start();

    /**
     * @brief Stop the broker
     */
    void stop();

    /**
     * @brief Check if broker is running
     */
    bool is_running() const noexcept { return running_.load(); }

private:
    std::string name_;
    std::unordered_map<std::string, std::unique_ptr<message_queue>> queues_;
    mutable std::shared_mutex mutex_;
    std::atomic<bool> running_{false};
};

} // namespace kcenon::messaging

// =============================================================================
// Interfaces
// =============================================================================

export namespace kcenon::messaging {

/**
 * @class publisher_interface
 * @brief Interface for message publishers
 */
class publisher_interface {
public:
    virtual ~publisher_interface() = default;

    /**
     * @brief Publish a message
     * @param msg Message to publish
     * @return Result indicating success or failure
     */
    virtual kcenon::common::VoidResult publish(const message& msg) = 0;

    /**
     * @brief Publish multiple messages
     * @param messages Messages to publish
     * @return Result indicating success or failure
     */
    virtual kcenon::common::VoidResult publish_batch(const std::vector<message>& messages) = 0;
};

/**
 * @class subscriber_interface
 * @brief Interface for message subscribers
 */
class subscriber_interface {
public:
    virtual ~subscriber_interface() = default;

    using handler_fn = std::function<void(const message&)>;

    /**
     * @brief Subscribe to a topic
     * @param topic Topic pattern
     * @param handler Handler function
     * @return Subscription ID
     */
    virtual std::string subscribe(const std::string& topic, handler_fn handler) = 0;

    /**
     * @brief Unsubscribe from a topic
     * @param subscription_id Subscription ID
     * @return true if unsubscribed
     */
    virtual bool unsubscribe(const std::string& subscription_id) = 0;
};

/**
 * @class queue_interface
 * @brief Interface for message queues
 */
class queue_interface {
public:
    virtual ~queue_interface() = default;

    /**
     * @brief Enqueue a message
     * @param msg Message to enqueue
     * @return true if enqueued
     */
    virtual bool enqueue(message msg) = 0;

    /**
     * @brief Try to dequeue a message
     * @return Message if available
     */
    virtual std::optional<message> try_dequeue() = 0;

    /**
     * @brief Get queue size
     */
    virtual size_t size() const = 0;

    /**
     * @brief Check if queue is empty
     */
    virtual bool empty() const = 0;
};

/**
 * @class message_handler_interface
 * @brief Interface for message handlers
 */
class message_handler_interface {
public:
    virtual ~message_handler_interface() = default;

    /**
     * @brief Handle a message
     * @param msg Message to handle
     * @return Result indicating success or failure
     */
    virtual kcenon::common::VoidResult handle(const message& msg) = 0;

    /**
     * @brief Check if handler can process the message
     * @param msg Message to check
     * @return true if handler can process
     */
    virtual bool can_handle(const message& msg) const = 0;
};

} // namespace kcenon::messaging
