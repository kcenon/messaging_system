// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file patterns.cppm
 * @brief Messaging patterns partition for messaging_system module.
 *
 * This partition provides enterprise messaging patterns including:
 * - Publish/Subscribe (Pub/Sub)
 * - Request/Reply
 * - Event Streaming
 * - Message Pipeline
 *
 * These patterns build on the core messaging infrastructure to provide
 * higher-level abstractions for common messaging scenarios.
 *
 * @see kcenon.messaging for the primary module interface
 * @see kcenon.messaging:core for core messaging types
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
#include <variant>
#include <vector>

// Third-party headers
#include <core/container.h>

export module kcenon.messaging:patterns;

export import kcenon.common;
export import :core;

// =============================================================================
// Forward Declarations
// =============================================================================

export namespace kcenon::messaging::patterns {

class pub_sub_broker;
class request_reply_client;
class event_stream;
class message_pipeline;

} // namespace kcenon::messaging::patterns

// =============================================================================
// Pub/Sub Pattern
// =============================================================================

export namespace kcenon::messaging::patterns {

/**
 * @class pub_sub_broker
 * @brief Publish/Subscribe messaging pattern implementation
 *
 * Provides decoupled communication between publishers and subscribers.
 * Publishers send messages to topics without knowing who will receive them.
 * Subscribers register interest in topics and receive matching messages.
 *
 * Features:
 * - Topic wildcard matching (* and #)
 * - Subscription groups for load balancing
 * - Message filtering
 * - Async message delivery
 *
 * @code
 * pub_sub_broker broker;
 *
 * // Subscribe to all order events
 * broker.subscribe("orders.*", [](const message& msg) {
 *     std::cout << "Order event: " << msg.metadata().topic << std::endl;
 * });
 *
 * // Publish an order event
 * auto msg = message_builder().topic("orders.created").build().value();
 * broker.publish(msg);
 * @endcode
 */
class pub_sub_broker {
public:
    using handler_fn = std::function<void(const message&)>;
    using filter_fn = std::function<bool(const message&)>;

    pub_sub_broker();
    ~pub_sub_broker();

    // Non-copyable
    pub_sub_broker(const pub_sub_broker&) = delete;
    pub_sub_broker& operator=(const pub_sub_broker&) = delete;

    /**
     * @brief Subscribe to a topic pattern
     * @param pattern Topic pattern (supports * and # wildcards)
     * @param handler Handler function for received messages
     * @return Subscription ID for unsubscribing
     */
    std::string subscribe(const std::string& pattern, handler_fn handler);

    /**
     * @brief Subscribe with a message filter
     * @param pattern Topic pattern
     * @param filter Filter function to select messages
     * @param handler Handler function
     * @return Subscription ID
     */
    std::string subscribe_filtered(const std::string& pattern,
                                   filter_fn filter,
                                   handler_fn handler);

    /**
     * @brief Subscribe to a subscription group
     * @param pattern Topic pattern
     * @param group Group name for load balancing
     * @param handler Handler function
     * @return Subscription ID
     *
     * Messages are distributed among group members (only one receives each message).
     */
    std::string subscribe_group(const std::string& pattern,
                                const std::string& group,
                                handler_fn handler);

    /**
     * @brief Unsubscribe from a topic
     * @param subscription_id Subscription ID
     * @return true if unsubscribed
     */
    bool unsubscribe(const std::string& subscription_id);

    /**
     * @brief Publish a message
     * @param msg Message to publish
     * @return Number of subscribers that received the message
     */
    size_t publish(const message& msg);

    /**
     * @brief Publish a message asynchronously
     * @param msg Message to publish
     * @return Future resolving to subscriber count
     */
    std::future<size_t> publish_async(message msg);

    /**
     * @brief Get number of subscriptions
     */
    size_t subscription_count() const;

    /**
     * @brief Get subscription count for a topic
     * @param topic Topic to check
     */
    size_t subscription_count(const std::string& topic) const;

    /**
     * @brief Clear all subscriptions
     */
    void clear();

private:
    struct subscription_info {
        std::string id;
        std::string pattern;
        std::string group;
        handler_fn handler;
        filter_fn filter;
    };

    std::vector<subscription_info> subscriptions_;
    std::unordered_map<std::string, std::vector<std::string>> groups_;
    std::unordered_map<std::string, size_t> group_indexes_;
    mutable std::shared_mutex mutex_;
    std::atomic<uint64_t> next_id_{0};
};

} // namespace kcenon::messaging::patterns

// =============================================================================
// Request/Reply Pattern
// =============================================================================

export namespace kcenon::messaging::patterns {

/**
 * @struct reply_options
 * @brief Options for request/reply operations
 */
struct reply_options {
    std::chrono::milliseconds timeout{5000};  ///< Reply timeout
    bool allow_multiple_replies{false};        ///< Allow multiple replies
    size_t max_replies{1};                     ///< Maximum replies to wait for
};

/**
 * @class request_reply_client
 * @brief Request/Reply messaging pattern implementation
 *
 * Provides synchronous and asynchronous request/reply communication.
 * Handles correlation of requests and replies automatically.
 *
 * @code
 * request_reply_client client(broker);
 *
 * // Synchronous request
 * auto reply = client.request("orders.get", order_query);
 * if (reply.has_value()) {
 *     process_order(reply.value());
 * }
 *
 * // Asynchronous request
 * auto future = client.request_async("orders.get", order_query);
 * // ... do other work ...
 * auto result = future.get();
 * @endcode
 */
class request_reply_client {
public:
    using reply_handler_fn = std::function<void(const kcenon::common::Result<message>&)>;

    explicit request_reply_client(message_broker& broker);
    ~request_reply_client();

    // Non-copyable
    request_reply_client(const request_reply_client&) = delete;
    request_reply_client& operator=(const request_reply_client&) = delete;

    /**
     * @brief Send a synchronous request
     * @param topic Target topic
     * @param request Request message
     * @param options Reply options
     * @return Reply message or error
     */
    kcenon::common::Result<message> request(const std::string& topic,
                                            const message& request,
                                            const reply_options& options = {});

    /**
     * @brief Send an asynchronous request
     * @param topic Target topic
     * @param request Request message
     * @param options Reply options
     * @return Future containing reply or error
     */
    std::future<kcenon::common::Result<message>> request_async(
        const std::string& topic,
        message request,
        const reply_options& options = {});

    /**
     * @brief Send a request with callback
     * @param topic Target topic
     * @param request Request message
     * @param handler Callback for reply
     * @param options Reply options
     */
    void request_callback(const std::string& topic,
                          message request,
                          reply_handler_fn handler,
                          const reply_options& options = {});

    /**
     * @brief Register a request handler
     * @param topic Topic to handle
     * @param handler Handler function that returns a reply
     * @return Handler ID for unregistering
     */
    std::string register_handler(const std::string& topic,
                                 std::function<message(const message&)> handler);

    /**
     * @brief Unregister a request handler
     * @param handler_id Handler ID
     * @return true if unregistered
     */
    bool unregister_handler(const std::string& handler_id);

    /**
     * @brief Get pending request count
     */
    size_t pending_requests() const;

private:
    struct pending_request {
        std::string correlation_id;
        std::chrono::steady_clock::time_point deadline;
        reply_handler_fn handler;
    };

    message_broker& broker_;
    std::unordered_map<std::string, pending_request> pending_;
    std::unordered_map<std::string, std::function<message(const message&)>> handlers_;
    mutable std::mutex mutex_;
    std::string reply_topic_;
    std::string subscription_id_;
};

} // namespace kcenon::messaging::patterns

// =============================================================================
// Event Streaming Pattern
// =============================================================================

export namespace kcenon::messaging::patterns {

/**
 * @struct stream_options
 * @brief Options for event streaming
 */
struct stream_options {
    size_t buffer_size{1000};                          ///< Event buffer size
    std::chrono::milliseconds retention{60000};        ///< Event retention time
    bool enable_replay{true};                          ///< Allow replaying events
    std::optional<std::string> checkpoint_id;          ///< Resume from checkpoint
};

/**
 * @class event_stream
 * @brief Event streaming pattern implementation
 *
 * Provides ordered, replayable event streaming with checkpointing.
 * Events are retained for a configurable duration and can be replayed.
 *
 * @code
 * event_stream stream("orders", stream_options{.retention = 3600000});
 *
 * // Publish events
 * stream.publish(order_created_event);
 * stream.publish(order_updated_event);
 *
 * // Subscribe to real-time events
 * stream.subscribe([](const message& event) {
 *     process_event(event);
 * });
 *
 * // Replay events from a checkpoint
 * stream.replay_from("checkpoint-123", [](const message& event) {
 *     rebuild_state(event);
 * });
 * @endcode
 */
class event_stream {
public:
    using event_handler_fn = std::function<void(const message&)>;

    explicit event_stream(const std::string& name, const stream_options& options = {});
    ~event_stream();

    // Non-copyable
    event_stream(const event_stream&) = delete;
    event_stream& operator=(const event_stream&) = delete;

    /**
     * @brief Get stream name
     */
    const std::string& name() const noexcept { return name_; }

    /**
     * @brief Publish an event to the stream
     * @param event Event message
     * @return Sequence number of the event
     */
    uint64_t publish(const message& event);

    /**
     * @brief Subscribe to real-time events
     * @param handler Event handler
     * @return Subscription ID
     */
    std::string subscribe(event_handler_fn handler);

    /**
     * @brief Unsubscribe from events
     * @param subscription_id Subscription ID
     * @return true if unsubscribed
     */
    bool unsubscribe(const std::string& subscription_id);

    /**
     * @brief Replay events from a sequence number
     * @param from_sequence Starting sequence number
     * @param handler Handler for each event
     * @return Number of events replayed
     */
    size_t replay_from(uint64_t from_sequence, event_handler_fn handler);

    /**
     * @brief Replay events from a checkpoint
     * @param checkpoint_id Checkpoint identifier
     * @param handler Handler for each event
     * @return Number of events replayed
     */
    size_t replay_from(const std::string& checkpoint_id, event_handler_fn handler);

    /**
     * @brief Create a checkpoint at current position
     * @return Checkpoint ID
     */
    std::string create_checkpoint();

    /**
     * @brief Get current sequence number
     */
    uint64_t current_sequence() const noexcept;

    /**
     * @brief Get number of retained events
     */
    size_t event_count() const;

    /**
     * @brief Clear expired events
     * @return Number of events cleared
     */
    size_t cleanup_expired();

private:
    struct stored_event {
        uint64_t sequence;
        std::chrono::steady_clock::time_point timestamp;
        message event;
    };

    std::string name_;
    stream_options options_;
    std::deque<stored_event> events_;
    std::unordered_map<std::string, event_handler_fn> subscribers_;
    std::unordered_map<std::string, uint64_t> checkpoints_;
    mutable std::shared_mutex mutex_;
    std::atomic<uint64_t> sequence_{0};
    std::atomic<uint64_t> next_sub_id_{0};
};

} // namespace kcenon::messaging::patterns

// =============================================================================
// Message Pipeline Pattern
// =============================================================================

export namespace kcenon::messaging::patterns {

/**
 * @class message_pipeline
 * @brief Message processing pipeline pattern
 *
 * Provides a configurable pipeline of message processors.
 * Messages flow through each stage in order, with support for:
 * - Filtering
 * - Transformation
 * - Routing
 * - Error handling
 *
 * @code
 * message_pipeline pipeline;
 *
 * // Add processing stages
 * pipeline
 *     .filter([](const message& m) { return m.metadata().priority >= message_priority::high; })
 *     .transform([](message m) {
 *         m.metadata().headers["processed"] = "true";
 *         return m;
 *     })
 *     .route([](const message& m) {
 *         return m.metadata().type == message_type::command ? "commands" : "events";
 *     })
 *     .sink("commands", [](const message& m) { handle_command(m); })
 *     .sink("events", [](const message& m) { handle_event(m); });
 *
 * pipeline.process(incoming_message);
 * @endcode
 */
class message_pipeline {
public:
    using filter_fn = std::function<bool(const message&)>;
    using transform_fn = std::function<message(message)>;
    using route_fn = std::function<std::string(const message&)>;
    using sink_fn = std::function<void(const message&)>;
    using error_fn = std::function<void(const message&, const std::exception&)>;

    message_pipeline();
    ~message_pipeline();

    // Non-copyable, movable
    message_pipeline(const message_pipeline&) = delete;
    message_pipeline& operator=(const message_pipeline&) = delete;
    message_pipeline(message_pipeline&&) noexcept;
    message_pipeline& operator=(message_pipeline&&) noexcept;

    /**
     * @brief Add a filter stage
     * @param filter Filter function (returns true to continue)
     * @return Reference to this pipeline for chaining
     */
    message_pipeline& filter(filter_fn filter);

    /**
     * @brief Add a transform stage
     * @param transform Transform function
     * @return Reference to this pipeline for chaining
     */
    message_pipeline& transform(transform_fn transform);

    /**
     * @brief Add a routing stage
     * @param route Routing function (returns sink name)
     * @return Reference to this pipeline for chaining
     */
    message_pipeline& route(route_fn route);

    /**
     * @brief Add a named sink
     * @param name Sink name for routing
     * @param sink Sink function
     * @return Reference to this pipeline for chaining
     */
    message_pipeline& sink(const std::string& name, sink_fn sink);

    /**
     * @brief Set error handler
     * @param handler Error handler function
     * @return Reference to this pipeline for chaining
     */
    message_pipeline& on_error(error_fn handler);

    /**
     * @brief Process a message through the pipeline
     * @param msg Message to process
     * @return Result indicating success or failure
     */
    kcenon::common::VoidResult process(message msg);

    /**
     * @brief Process multiple messages
     * @param messages Messages to process
     * @return Results for each message
     */
    std::vector<kcenon::common::VoidResult> process_batch(
        const std::vector<message>& messages);

    /**
     * @brief Get pipeline stage count
     */
    size_t stage_count() const noexcept;

private:
    struct stage {
        enum class type { filter, transform, route };
        type stage_type;
        std::variant<filter_fn, transform_fn, route_fn> processor;
    };

    std::vector<stage> stages_;
    std::unordered_map<std::string, sink_fn> sinks_;
    error_fn error_handler_;
    std::string default_sink_;
};

} // namespace kcenon::messaging::patterns
