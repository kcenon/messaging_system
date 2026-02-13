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

// =============================================================================
// Error Codes
// =============================================================================

export namespace kcenon::messaging::error {

/**
 * @brief Base value for all messaging_system error codes
 *
 * messaging_system uses error code range -700 to -799.
 * Error Code Organization:
 * - -700 to -719: Message errors
 * - -720 to -739: Routing errors
 * - -740 to -759: Queue errors
 * - -760 to -779: Subscription errors
 * - -780 to -799: Publishing errors
 */
constexpr int base = -700;

// Message Errors (-700 to -719)
constexpr int invalid_message = base - 0;
constexpr int message_too_large = base - 1;
constexpr int message_expired = base - 2;
constexpr int invalid_payload = base - 3;
constexpr int message_serialization_failed = base - 4;
constexpr int message_deserialization_failed = base - 5;

// Task-specific errors
constexpr int task_not_found = base - 6;
constexpr int task_already_running = base - 7;
constexpr int task_cancelled = base - 8;
constexpr int task_timeout = base - 9;
constexpr int task_failed = base - 10;
constexpr int task_handler_not_found = base - 11;
constexpr int task_spawner_not_configured = base - 12;
constexpr int task_invalid_argument = base - 13;
constexpr int task_operation_failed = base - 14;
constexpr int schedule_already_exists = base - 15;

// Routing Errors (-720 to -739)
constexpr int routing_failed = base - 20;
constexpr int unknown_topic = base - 21;
constexpr int no_subscribers = base - 22;
constexpr int invalid_topic_pattern = base - 23;
constexpr int route_not_found = base - 24;

// Queue Errors (-740 to -759)
constexpr int queue_full = base - 40;
constexpr int queue_empty = base - 41;
constexpr int queue_stopped = base - 42;
constexpr int enqueue_failed = base - 43;
constexpr int dequeue_failed = base - 44;
constexpr int queue_timeout = base - 45;

// Dead Letter Queue errors
constexpr int dlq_full = base - 46;
constexpr int dlq_empty = base - 47;
constexpr int dlq_message_not_found = base - 48;
constexpr int dlq_replay_failed = base - 49;
constexpr int dlq_not_configured = base - 50;

// Subscription Errors (-760 to -779)
constexpr int subscription_failed = base - 60;
constexpr int subscription_not_found = base - 61;
constexpr int duplicate_subscription = base - 62;
constexpr int unsubscribe_failed = base - 63;
constexpr int invalid_subscription = base - 64;

// Publishing Errors (-780 to -799)
constexpr int publication_failed = base - 80;
constexpr int no_route_found = base - 81;
constexpr int message_rejected = base - 82;
constexpr int broker_unavailable = base - 83;
constexpr int broker_not_started = base - 84;
constexpr int already_running = base - 85;
constexpr int not_running = base - 86;
constexpr int backend_not_ready = base - 87;
constexpr int request_timeout = base - 88;
constexpr int not_supported = base - 89;

// Transport-specific errors
constexpr int connection_failed = base - 90;
constexpr int send_timeout = base - 91;
constexpr int receive_timeout = base - 92;
constexpr int authentication_failed = base - 93;
constexpr int not_connected = base - 94;

/**
 * @brief Get human-readable error message for error code
 * @param code Error code
 * @return Error message string
 */
constexpr std::string_view get_error_message(int code) noexcept {
    switch (code) {
        case invalid_message: return "Invalid message";
        case message_too_large: return "Message too large";
        case message_expired: return "Message expired";
        case invalid_payload: return "Invalid message payload";
        case message_serialization_failed: return "Message serialization failed";
        case message_deserialization_failed: return "Message deserialization failed";
        case task_not_found: return "Task not found";
        case task_already_running: return "Task already running";
        case task_cancelled: return "Task cancelled";
        case task_timeout: return "Task timeout";
        case task_failed: return "Task execution failed";
        case task_handler_not_found: return "Task handler not found";
        case task_spawner_not_configured: return "Subtask spawner not configured";
        case task_invalid_argument: return "Invalid task argument";
        case task_operation_failed: return "Task operation failed";
        case schedule_already_exists: return "Schedule already exists";
        case routing_failed: return "Message routing failed";
        case unknown_topic: return "Unknown topic";
        case no_subscribers: return "No subscribers for topic";
        case invalid_topic_pattern: return "Invalid topic pattern";
        case route_not_found: return "Route not found";
        case queue_full: return "Message queue full";
        case queue_empty: return "Message queue empty";
        case queue_stopped: return "Message queue stopped";
        case enqueue_failed: return "Failed to enqueue message";
        case dequeue_failed: return "Failed to dequeue message";
        case queue_timeout: return "Queue operation timeout";
        case dlq_full: return "Dead letter queue full";
        case dlq_empty: return "Dead letter queue empty";
        case dlq_message_not_found: return "Message not found in dead letter queue";
        case dlq_replay_failed: return "Failed to replay message from dead letter queue";
        case dlq_not_configured: return "Dead letter queue not configured";
        case subscription_failed: return "Subscription failed";
        case subscription_not_found: return "Subscription not found";
        case duplicate_subscription: return "Duplicate subscription";
        case unsubscribe_failed: return "Unsubscribe failed";
        case invalid_subscription: return "Invalid subscription";
        case publication_failed: return "Publication failed";
        case no_route_found: return "No route found for message";
        case message_rejected: return "Message rejected";
        case broker_unavailable: return "Message broker unavailable";
        case broker_not_started: return "Message broker not started";
        case already_running: return "Message bus already running";
        case not_running: return "Message bus not running";
        case backend_not_ready: return "Backend not ready";
        case request_timeout: return "Request timeout";
        case not_supported: return "Feature not supported";
        case connection_failed: return "Connection failed";
        case send_timeout: return "Send operation timed out";
        case receive_timeout: return "Receive operation timed out";
        case authentication_failed: return "Authentication failed";
        case not_connected: return "Transport not connected";
        default: return "Unknown messaging error";
    }
}

} // namespace kcenon::messaging::error

// =============================================================================
// Error Category
// =============================================================================

export namespace kcenon::messaging {

/**
 * @class messaging_error_category
 * @brief Error category for messaging_system typed error codes
 *
 * Integrates with common_system's typed_error_code infrastructure.
 * Singleton pattern, thread-safe via C++11 static local initialization.
 */
class messaging_error_category : public common::error_category {
public:
    static const messaging_error_category& instance() noexcept {
        static messaging_error_category inst;
        return inst;
    }

    std::string_view name() const noexcept override {
        return "messaging";
    }

    std::string message(int code) const override {
        return std::string(error::get_error_message(code));
    }

private:
    messaging_error_category() = default;
};

/**
 * @brief Create a typed_error_code for a messaging error code
 * @param code Messaging error code (from error:: namespace)
 * @return typed_error_code with messaging_error_category
 */
inline common::typed_error_code make_messaging_error_code(int code) noexcept {
    return common::typed_error_code(code, messaging_error_category::instance());
}

} // namespace kcenon::messaging
