// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file integration.cppm
 * @brief Integration partition for messaging_system module.
 *
 * This partition provides integration components including:
 * - Transport interfaces and implementations (WebSocket, HTTP)
 * - Backend implementations (standalone, integration)
 * - Dependency injection container
 * - Monitoring collectors
 * - Configuration and feature flags
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
#include <format>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <variant>
#include <vector>

// Third-party headers
#include <core/container.h>

export module kcenon.messaging:integration;

export import kcenon.common;
export import kcenon.thread;
export import kcenon.logger;
export import :core;

// =============================================================================
// Forward Declarations
// =============================================================================

export namespace kcenon::messaging::adapters {

class transport_interface;
class websocket_transport;
class http_transport;
class resilient_transport;

} // namespace kcenon::messaging::adapters

export namespace kcenon::messaging::backends {

class backend_interface;
class standalone_backend;
class integration_backend;

} // namespace kcenon::messaging::backends

export namespace kcenon::messaging::di {

class messaging_container;
class service_registry;

} // namespace kcenon::messaging::di

export namespace kcenon::messaging::collectors {

class message_bus_collector;

} // namespace kcenon::messaging::collectors

export namespace kcenon::messaging::config {

class feature_flags;

} // namespace kcenon::messaging::config

// =============================================================================
// Transport Interface
// =============================================================================

export namespace kcenon::messaging::adapters {

/**
 * @enum transport_state
 * @brief Transport connection state
 */
enum class transport_state {
    disconnected,
    connecting,
    connected,
    disconnecting,
    error
};

/**
 * @brief Convert transport_state to string
 */
constexpr std::string_view to_string(transport_state state) noexcept {
    switch (state) {
        case transport_state::disconnected: return "disconnected";
        case transport_state::connecting: return "connecting";
        case transport_state::connected: return "connected";
        case transport_state::disconnecting: return "disconnecting";
        case transport_state::error: return "error";
        default: return "unknown";
    }
}

/**
 * @struct transport_config
 * @brief Base configuration for transports
 */
struct transport_config {
    std::string host;
    uint16_t port = 0;
    std::chrono::milliseconds connect_timeout{10000};
    std::chrono::milliseconds request_timeout{30000};
    bool auto_reconnect = false;
    std::size_t max_retries = 3;
    std::chrono::milliseconds retry_delay{1000};
};

/**
 * @struct transport_statistics
 * @brief Transport performance statistics
 */
struct transport_statistics {
    uint64_t messages_sent = 0;
    uint64_t messages_received = 0;
    uint64_t bytes_sent = 0;
    uint64_t bytes_received = 0;
    uint64_t errors = 0;
    std::chrono::milliseconds avg_latency{0};
};

/**
 * @class transport_interface
 * @brief Abstract interface for network transports
 *
 * Provides an abstraction for different transport mechanisms
 * (WebSocket, HTTP, etc.) for sending and receiving messages.
 */
class transport_interface {
public:
    virtual ~transport_interface() = default;

    // Connection management
    virtual kcenon::common::VoidResult connect() = 0;
    virtual kcenon::common::VoidResult disconnect() = 0;
    virtual bool is_connected() const = 0;
    virtual transport_state get_state() const = 0;

    // Message sending
    virtual kcenon::common::VoidResult send(const message& msg) = 0;
    virtual kcenon::common::VoidResult send_binary(
        const std::vector<uint8_t>& data) = 0;

    // Callbacks
    virtual void set_message_handler(
        std::function<void(const message&)> handler) = 0;
    virtual void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)> handler) = 0;
    virtual void set_state_handler(
        std::function<void(transport_state)> handler) = 0;
    virtual void set_error_handler(
        std::function<void(const std::string&)> handler) = 0;

    // Statistics
    virtual transport_statistics get_statistics() const = 0;
    virtual void reset_statistics() = 0;
};

/**
 * @struct websocket_transport_config
 * @brief Configuration for WebSocket transport
 */
struct websocket_transport_config : transport_config {
    std::string path = "/ws";
    bool use_ssl = false;
    std::chrono::milliseconds ping_interval{30000};
    bool auto_pong = true;
    std::size_t max_message_size = 10 * 1024 * 1024;  // 10MB
    std::chrono::milliseconds reconnect_delay{1000};
    double reconnect_backoff_multiplier = 2.0;
    std::chrono::milliseconds max_reconnect_delay{30000};
    std::shared_ptr<kcenon::common::interfaces::IExecutor> executor = nullptr;
};

/**
 * @class websocket_transport
 * @brief WebSocket-based message transport
 *
 * Provides bidirectional message transport over WebSocket protocol.
 * Requires KCENON_WITH_NETWORK_SYSTEM for full functionality.
 */
class websocket_transport : public transport_interface {
public:
    static constexpr bool is_available =
#if KCENON_WITH_NETWORK_SYSTEM
        true;
#else
        false;
#endif

    explicit websocket_transport(const websocket_transport_config& config);
    ~websocket_transport() override;

    // Non-copyable
    websocket_transport(const websocket_transport&) = delete;
    websocket_transport& operator=(const websocket_transport&) = delete;

    // transport_interface implementation
    kcenon::common::VoidResult connect() override;
    kcenon::common::VoidResult disconnect() override;
    bool is_connected() const override;
    transport_state get_state() const override;
    kcenon::common::VoidResult send(const message& msg) override;
    kcenon::common::VoidResult send_binary(const std::vector<uint8_t>& data) override;
    void set_message_handler(
        std::function<void(const message&)> handler) override;
    void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)> handler) override;
    void set_state_handler(
        std::function<void(transport_state)> handler) override;
    void set_error_handler(
        std::function<void(const std::string&)> handler) override;
    transport_statistics get_statistics() const override;
    void reset_statistics() override;

    // WebSocket-specific methods
    kcenon::common::VoidResult subscribe(const std::string& topic_pattern);
    kcenon::common::VoidResult unsubscribe(const std::string& topic_pattern);
    kcenon::common::VoidResult unsubscribe_all();
    std::set<std::string> get_subscriptions() const;
    kcenon::common::VoidResult send_text(const std::string& text);
    kcenon::common::VoidResult ping();
    void set_disconnect_handler(
        std::function<void(uint16_t code, const std::string& reason)> handler);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

/**
 * @enum http_content_type
 * @brief HTTP content types for message serialization
 */
enum class http_content_type {
    json,
    binary,
    msgpack
};

/**
 * @struct http_transport_config
 * @brief Configuration for HTTP transport
 */
struct http_transport_config : transport_config {
    std::string base_path = "/api/messages";
    http_content_type content_type = http_content_type::json;
    bool use_ssl = false;
    std::map<std::string, std::string> default_headers;
    std::string publish_endpoint = "/publish";
    std::string subscribe_endpoint = "/subscribe";
    std::string request_endpoint = "/request";
};

/**
 * @class http_transport
 * @brief HTTP-based message transport
 *
 * Provides message transport over HTTP/HTTPS protocol.
 * Requires KCENON_WITH_NETWORK_SYSTEM for full functionality.
 */
class http_transport : public transport_interface {
public:
    static constexpr bool is_available =
#if KCENON_WITH_NETWORK_SYSTEM
        true;
#else
        false;
#endif

    explicit http_transport(const http_transport_config& config);
    ~http_transport() override;

    // Non-copyable
    http_transport(const http_transport&) = delete;
    http_transport& operator=(const http_transport&) = delete;

    // transport_interface implementation
    kcenon::common::VoidResult connect() override;
    kcenon::common::VoidResult disconnect() override;
    bool is_connected() const override;
    transport_state get_state() const override;
    kcenon::common::VoidResult send(const message& msg) override;
    kcenon::common::VoidResult send_binary(const std::vector<uint8_t>& data) override;
    void set_message_handler(
        std::function<void(const message&)> handler) override;
    void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)> handler) override;
    void set_state_handler(
        std::function<void(transport_state)> handler) override;
    void set_error_handler(
        std::function<void(const std::string&)> handler) override;
    transport_statistics get_statistics() const override;
    void reset_statistics() override;

    // HTTP-specific methods
    kcenon::common::Result<message> post(
        const std::string& endpoint, const message& msg);
    kcenon::common::Result<message> get(
        const std::string& endpoint,
        const std::map<std::string, std::string>& query = {});
    void set_header(const std::string& key, const std::string& value);
    void remove_header(const std::string& key);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

// =============================================================================
// Resilience Types
// =============================================================================

/**
 * @brief Circuit breaker state from common_system resilience module
 */
using circuit_state = kcenon::common::resilience::circuit_state;

/**
 * @brief Circuit breaker configuration from common_system resilience module
 */
using circuit_breaker_config = kcenon::common::resilience::circuit_breaker_config;

/**
 * @struct retry_config
 * @brief Configuration for retry behavior
 */
struct retry_config {
    std::size_t max_retries = 3;
    std::chrono::milliseconds initial_delay{100};
    double backoff_multiplier = 2.0;
    std::chrono::milliseconds max_delay{10000};
    bool retry_on_timeout = true;
};

/**
 * @struct resilient_transport_config
 * @brief Configuration for resilient transport
 */
struct resilient_transport_config {
    retry_config retry;
    circuit_breaker_config circuit_breaker;
    std::chrono::milliseconds operation_timeout{30000};
    bool enable_fallback = false;
};

/**
 * @struct resilience_statistics
 * @brief Statistics for resilience features
 */
struct resilience_statistics {
    uint64_t total_attempts = 0;
    uint64_t successful_first_attempts = 0;
    uint64_t successful_retries = 0;
    uint64_t failed_after_retries = 0;
    uint64_t circuit_opens = 0;
    uint64_t circuit_closes = 0;
    uint64_t rejected_by_circuit = 0;
    circuit_state current_circuit_state = circuit_state::CLOSED;
    std::chrono::milliseconds avg_success_latency{0};
    std::chrono::milliseconds avg_failure_latency{0};
};

/**
 * @class resilient_transport
 * @brief Transport wrapper with retry and circuit breaker
 *
 * Wraps any transport_interface to provide:
 * - Automatic retry with exponential backoff
 * - Circuit breaker pattern for fault isolation
 * - Timeout management
 * - Fallback support
 */
class resilient_transport : public transport_interface {
public:
    resilient_transport(
        std::shared_ptr<transport_interface> transport,
        const resilient_transport_config& config = {});
    ~resilient_transport() override;

    // transport_interface implementation
    kcenon::common::VoidResult connect() override;
    kcenon::common::VoidResult disconnect() override;
    bool is_connected() const override;
    transport_state get_state() const override;
    kcenon::common::VoidResult send(const message& msg) override;
    kcenon::common::VoidResult send_binary(const std::vector<uint8_t>& data) override;
    void set_message_handler(
        std::function<void(const message&)> handler) override;
    void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)> handler) override;
    void set_state_handler(
        std::function<void(transport_state)> handler) override;
    void set_error_handler(
        std::function<void(const std::string&)> handler) override;
    transport_statistics get_statistics() const override;
    void reset_statistics() override;

    // Resilience-specific methods
    void set_fallback(std::shared_ptr<transport_interface> fallback);
    circuit_state get_circuit_state() const;
    void force_circuit_open();
    void force_circuit_close();
    resilience_statistics get_resilience_statistics() const;
    void reset_resilience_statistics();
    void set_retry_config(const retry_config& config);
    void set_circuit_breaker_config(const circuit_breaker_config& config);
    void set_circuit_state_handler(
        std::function<void(circuit_state)> handler);
    void set_retry_handler(
        std::function<void(std::size_t attempt, std::chrono::milliseconds delay)> handler);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace kcenon::messaging::adapters

// =============================================================================
// Backend Interface
// =============================================================================

export namespace kcenon::messaging::backends {

/**
 * @class backend_interface
 * @brief Interface for messaging backend implementations
 *
 * Provides an abstraction for different messaging backends
 * that handle message persistence and delivery.
 */
class backend_interface {
public:
    virtual ~backend_interface() = default;

    /**
     * @brief Initialize the backend
     * @return Result indicating success or failure
     */
    virtual kcenon::common::VoidResult initialize() = 0;

    /**
     * @brief Shutdown the backend
     */
    virtual void shutdown() = 0;

    /**
     * @brief Check if backend is running
     */
    virtual bool is_running() const noexcept = 0;

    /**
     * @brief Publish a message
     * @param msg Message to publish
     * @return Result indicating success or failure
     */
    virtual kcenon::common::VoidResult publish(const message& msg) = 0;

    /**
     * @brief Subscribe to a topic
     * @param topic Topic pattern
     * @param handler Handler for received messages
     * @return Subscription ID
     */
    virtual std::string subscribe(const std::string& topic,
                                  std::function<void(const message&)> handler) = 0;

    /**
     * @brief Unsubscribe from a topic
     * @param subscription_id Subscription ID
     * @return true if unsubscribed
     */
    virtual bool unsubscribe(const std::string& subscription_id) = 0;

    /**
     * @brief Get backend name
     */
    virtual std::string name() const = 0;
};

/**
 * @class standalone_backend
 * @brief In-process messaging backend
 *
 * Provides messaging within a single process without external dependencies.
 */
class standalone_backend : public backend_interface {
public:
    standalone_backend();
    ~standalone_backend() override;

    kcenon::common::VoidResult initialize() override;
    void shutdown() override;
    bool is_running() const noexcept override;
    kcenon::common::VoidResult publish(const message& msg) override;
    std::string subscribe(const std::string& topic,
                          std::function<void(const message&)> handler) override;
    bool unsubscribe(const std::string& subscription_id) override;
    std::string name() const override { return "standalone"; }

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

/**
 * @class integration_backend
 * @brief Backend with external system integration
 *
 * Provides messaging with integration to external systems
 * (network_system, database_system, etc.).
 */
class integration_backend : public backend_interface {
public:
    /**
     * @struct integration_options
     * @brief Options for external integration
     */
    struct integration_options {
        bool enable_persistence = false;
        bool enable_network = false;
        std::string persistence_path;
        adapters::transport_config network_options;
    };

    explicit integration_backend(const integration_options& options = {});
    ~integration_backend() override;

    kcenon::common::VoidResult initialize() override;
    void shutdown() override;
    bool is_running() const noexcept override;
    kcenon::common::VoidResult publish(const message& msg) override;
    std::string subscribe(const std::string& topic,
                          std::function<void(const message&)> handler) override;
    bool unsubscribe(const std::string& subscription_id) override;
    std::string name() const override { return "integration"; }

    /**
     * @brief Set transport for network messaging
     * @param transport Transport instance
     */
    void set_transport(std::unique_ptr<adapters::transport_interface> transport);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
    integration_options options_;
};

} // namespace kcenon::messaging::backends

// =============================================================================
// Dependency Injection
// =============================================================================

export namespace kcenon::messaging::di {

/**
 * @enum service_lifetime
 * @brief Service registration lifetime
 */
enum class service_lifetime {
    transient,  ///< New instance per request
    singleton,  ///< Single shared instance
    scoped      ///< Instance per scope
};

/**
 * @class service_registry
 * @brief Service registration for dependency injection
 */
class service_registry {
public:
    service_registry();
    ~service_registry();

    /**
     * @brief Register a service type
     * @tparam TInterface Interface type
     * @tparam TImpl Implementation type
     * @param lifetime Service lifetime
     */
    template<typename TInterface, typename TImpl>
    void register_type(service_lifetime lifetime = service_lifetime::transient);

    /**
     * @brief Register a factory function
     * @tparam T Service type
     * @param factory Factory function
     * @param lifetime Service lifetime
     */
    template<typename T>
    void register_factory(std::function<std::shared_ptr<T>()> factory,
                          service_lifetime lifetime = service_lifetime::transient);

    /**
     * @brief Register an instance
     * @tparam T Service type
     * @param instance Instance to register
     */
    template<typename T>
    void register_instance(std::shared_ptr<T> instance);

    /**
     * @brief Check if a type is registered
     * @tparam T Service type
     */
    template<typename T>
    bool is_registered() const;

private:
    friend class messaging_container;

    struct service_descriptor {
        std::type_index type;
        service_lifetime lifetime;
        std::function<std::shared_ptr<void>()> factory;
        std::shared_ptr<void> instance;
    };

    std::unordered_map<std::type_index, service_descriptor> descriptors_;
    mutable std::shared_mutex mutex_;
};

/**
 * @class messaging_container
 * @brief Dependency injection container for messaging services
 *
 * Provides service resolution and lifecycle management.
 */
class messaging_container {
public:
    explicit messaging_container(service_registry registry = {});
    ~messaging_container();

    // Non-copyable
    messaging_container(const messaging_container&) = delete;
    messaging_container& operator=(const messaging_container&) = delete;

    /**
     * @brief Resolve a service
     * @tparam T Service type
     * @return Resolved service instance or nullptr
     */
    template<typename T>
    std::shared_ptr<T> resolve();

    /**
     * @brief Resolve a required service
     * @tparam T Service type
     * @return Resolved service instance
     * @throws std::runtime_error if service not registered
     */
    template<typename T>
    std::shared_ptr<T> resolve_required();

    /**
     * @brief Create a scoped container
     * @return Scoped container instance
     */
    std::unique_ptr<messaging_container> create_scope();

    /**
     * @brief Get the service registry
     */
    service_registry& registry() { return registry_; }
    const service_registry& registry() const { return registry_; }

private:
    service_registry registry_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> scoped_instances_;
    mutable std::shared_mutex mutex_;
};

/**
 * @class messaging_bootstrapper
 * @brief Bootstrap helper for messaging container configuration
 */
class messaging_bootstrapper {
public:
    messaging_bootstrapper();

    /**
     * @brief Register default messaging services
     */
    messaging_bootstrapper& add_default_services();

    /**
     * @brief Register standalone backend
     */
    messaging_bootstrapper& add_standalone_backend();

    /**
     * @brief Register integration backend
     * @param options Integration options
     */
    messaging_bootstrapper& add_integration_backend(
        const backends::integration_backend::integration_options& options = {});

    /**
     * @brief Build the container
     * @return Configured container
     */
    std::unique_ptr<messaging_container> build();

private:
    service_registry registry_;
};

} // namespace kcenon::messaging::di

// =============================================================================
// Monitoring Collector
// =============================================================================

export namespace kcenon::messaging::collectors {

/**
 * @class message_bus_collector
 * @brief Metrics collector for message bus monitoring
 *
 * Collects and exposes metrics about message bus operations
 * for integration with monitoring_system.
 */
class message_bus_collector {
public:
    message_bus_collector();
    ~message_bus_collector();

    /**
     * @brief Record message published
     * @param topic Topic name
     */
    void record_publish(const std::string& topic);

    /**
     * @brief Record message delivered
     * @param topic Topic name
     * @param subscriber_count Number of subscribers that received
     */
    void record_delivery(const std::string& topic, size_t subscriber_count);

    /**
     * @brief Record subscription added
     * @param topic Topic name
     */
    void record_subscribe(const std::string& topic);

    /**
     * @brief Record subscription removed
     * @param topic Topic name
     */
    void record_unsubscribe(const std::string& topic);

    /**
     * @brief Record delivery latency
     * @param topic Topic name
     * @param latency Delivery latency
     */
    void record_latency(const std::string& topic, std::chrono::microseconds latency);

    /**
     * @brief Record error
     * @param topic Topic name
     * @param error Error description
     */
    void record_error(const std::string& topic, const std::string& error);

    /**
     * @brief Get total messages published
     */
    size_t total_published() const noexcept { return total_published_.load(); }

    /**
     * @brief Get total messages delivered
     */
    size_t total_delivered() const noexcept { return total_delivered_.load(); }

    /**
     * @brief Get total errors
     */
    size_t total_errors() const noexcept { return total_errors_.load(); }

    /**
     * @brief Get average latency in microseconds
     */
    double avg_latency_us() const noexcept;

    /**
     * @brief Reset all metrics
     */
    void reset();

private:
    std::atomic<size_t> total_published_{0};
    std::atomic<size_t> total_delivered_{0};
    std::atomic<size_t> total_subscriptions_{0};
    std::atomic<size_t> total_errors_{0};
    std::atomic<size_t> latency_sum_{0};
    std::atomic<size_t> latency_count_{0};
    mutable std::shared_mutex mutex_;
};

} // namespace kcenon::messaging::collectors

// =============================================================================
// Configuration
// =============================================================================

export namespace kcenon::messaging::config {

/**
 * @class feature_flags
 * @brief Runtime feature flag management
 *
 * Provides runtime control over messaging system features.
 */
class feature_flags {
public:
    /**
     * @brief Get global instance
     */
    static feature_flags& instance();

    /**
     * @brief Check if a feature is enabled
     * @param feature Feature name
     * @return true if enabled
     */
    bool is_enabled(const std::string& feature) const;

    /**
     * @brief Enable a feature
     * @param feature Feature name
     */
    void enable(const std::string& feature);

    /**
     * @brief Disable a feature
     * @param feature Feature name
     */
    void disable(const std::string& feature);

    /**
     * @brief Set feature state
     * @param feature Feature name
     * @param enabled Enable state
     */
    void set(const std::string& feature, bool enabled);

    /**
     * @brief Get all enabled features
     */
    std::vector<std::string> enabled_features() const;

    /**
     * @brief Load flags from environment variables
     * @param prefix Environment variable prefix
     */
    void load_from_environment(const std::string& prefix = "MESSAGING_");

    // Standard feature names
    static constexpr const char* FEATURE_LOCKFREE = "lockfree";
    static constexpr const char* FEATURE_MONITORING = "monitoring";
    static constexpr const char* FEATURE_LOGGING = "logging";
    static constexpr const char* FEATURE_TRACING = "tracing";
    static constexpr const char* FEATURE_HEALTH_CHECKS = "health_checks";
    static constexpr const char* FEATURE_TLS = "tls";

private:
    feature_flags();
    ~feature_flags() = default;

    feature_flags(const feature_flags&) = delete;
    feature_flags& operator=(const feature_flags&) = delete;

    std::unordered_set<std::string> enabled_;
    mutable std::shared_mutex mutex_;
};

} // namespace kcenon::messaging::config

// =============================================================================
// Event Bridge
// =============================================================================

export namespace kcenon::messaging::integration {

/**
 * @class event_bridge
 * @brief Bridge between messaging and other systems
 *
 * Provides event bridging between the messaging system and
 * external event sources/sinks.
 */
class event_bridge {
public:
    using event_handler = std::function<void(const message&)>;

    event_bridge();
    ~event_bridge();

    /**
     * @brief Register an event source
     * @param name Source name
     * @param source Source instance
     */
    void register_source(const std::string& name,
                         std::function<void(event_handler)> source);

    /**
     * @brief Register an event sink
     * @param name Sink name
     * @param sink Sink function
     */
    void register_sink(const std::string& name, event_handler sink);

    /**
     * @brief Forward events from source to sink
     * @param source_name Source name
     * @param sink_name Sink name
     * @return Route ID for removing
     */
    std::string add_route(const std::string& source_name,
                          const std::string& sink_name);

    /**
     * @brief Remove a route
     * @param route_id Route ID
     * @return true if removed
     */
    bool remove_route(const std::string& route_id);

    /**
     * @brief Start the bridge
     */
    void start();

    /**
     * @brief Stop the bridge
     */
    void stop();

    /**
     * @brief Check if bridge is running
     */
    bool is_running() const noexcept { return running_.load(); }

private:
    struct route {
        std::string id;
        std::string source;
        std::string sink;
    };

    std::unordered_map<std::string, std::function<void(event_handler)>> sources_;
    std::unordered_map<std::string, event_handler> sinks_;
    std::vector<route> routes_;
    mutable std::shared_mutex mutex_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> next_id_{0};
};

/**
 * @class task_event_bridge
 * @brief Bridge between task system and messaging
 *
 * Publishes task lifecycle events to the message bus.
 */
class task_event_bridge {
public:
    explicit task_event_bridge(message_bus& bus);
    ~task_event_bridge();

    /**
     * @brief Publish task submitted event
     * @param task_id Task ID
     * @param task_name Task name
     */
    void on_task_submitted(const std::string& task_id, const std::string& task_name);

    /**
     * @brief Publish task started event
     * @param task_id Task ID
     */
    void on_task_started(const std::string& task_id);

    /**
     * @brief Publish task completed event
     * @param task_id Task ID
     * @param success Success status
     */
    void on_task_completed(const std::string& task_id, bool success);

    /**
     * @brief Publish task failed event
     * @param task_id Task ID
     * @param error Error message
     */
    void on_task_failed(const std::string& task_id, const std::string& error);

    /**
     * @brief Set topic prefix for events
     * @param prefix Topic prefix (default: "tasks.")
     */
    void set_topic_prefix(std::string prefix);

private:
    message_bus& bus_;
    std::string topic_prefix_{"tasks."};
};

} // namespace kcenon::messaging::integration

// =============================================================================
// Executor Adapter
// =============================================================================

export namespace kcenon::messaging::integration {

/**
 * @class message_processor_job
 * @brief IJob implementation for processing messages
 *
 * This class wraps message processing logic in an IJob interface,
 * enabling execution via the common_system's IExecutor.
 */
class message_processor_job : public kcenon::common::interfaces::IJob {
public:
    using handler_t = std::function<kcenon::common::VoidResult(const message&)>;

    /**
     * @brief Construct a message processor job
     * @param msg Message to process
     * @param handler Handler function to process the message
     * @param priority Job priority (default: 0)
     */
    message_processor_job(message msg, handler_t handler, int priority = 0);

    /**
     * @brief Execute the job
     * @return VoidResult indicating success or failure
     */
    kcenon::common::VoidResult execute() override;

    /**
     * @brief Get the name of the job
     * @return Job name including topic info
     */
    std::string get_name() const override;

    /**
     * @brief Get the priority of the job
     * @return Job priority
     */
    int get_priority() const override;

    /**
     * @brief Get the message being processed
     * @return Const reference to the message
     */
    const message& get_message() const;

private:
    message msg_;
    handler_t handler_;
    int priority_;
};

/**
 * @class message_reply_job
 * @brief IJob implementation for request-reply pattern
 *
 * This job handles request processing and generates a reply message.
 */
class message_reply_job : public kcenon::common::interfaces::IJob {
public:
    using handler_t = std::function<kcenon::common::Result<message>(const message&)>;

    /**
     * @brief Construct a message reply job
     * @param request Request message
     * @param handler Handler function that returns a reply
     * @param reply_callback Callback to send the reply
     * @param priority Job priority
     */
    message_reply_job(
        message request,
        handler_t handler,
        std::function<void(kcenon::common::Result<message>)> reply_callback,
        int priority = 0);

    kcenon::common::VoidResult execute() override;
    std::string get_name() const override;
    int get_priority() const override;

private:
    message request_;
    handler_t handler_;
    std::function<void(kcenon::common::Result<message>)> reply_callback_;
    int priority_;
};

/**
 * @class executor_message_handler
 * @brief Adapter for processing messages via IExecutor
 *
 * This class provides a high-level interface for submitting message
 * processing jobs to an IExecutor implementation.
 */
class executor_message_handler {
public:
    /**
     * @brief Virtual destructor for polymorphic type support
     */
    virtual ~executor_message_handler() = default;

    /**
     * @brief Construct with an executor
     * @param executor Shared pointer to executor
     */
    explicit executor_message_handler(
        std::shared_ptr<kcenon::common::interfaces::IExecutor> executor);

    /**
     * @brief Process a message asynchronously
     * @param msg Message to process
     * @param handler Handler function
     * @param priority Job priority
     * @return Result containing future or error
     */
    kcenon::common::Result<std::future<void>> process_async(
        message msg,
        message_processor_job::handler_t handler,
        int priority = 0);

    /**
     * @brief Process a request and get a reply asynchronously
     * @param request Request message
     * @param handler Handler function that returns a reply
     * @param reply_callback Callback for the reply
     * @param priority Job priority
     * @return Result containing future for completion or error
     */
    kcenon::common::Result<std::future<void>> request_async(
        message request,
        message_reply_job::handler_t handler,
        std::function<void(kcenon::common::Result<message>)> reply_callback,
        int priority = 0);

    /**
     * @brief Get the executor
     * @return Shared pointer to executor
     */
    std::shared_ptr<kcenon::common::interfaces::IExecutor> get_executor() const;

    /**
     * @brief Check if executor is available and running
     * @return true if executor is available and running
     */
    bool is_available() const;

private:
    std::shared_ptr<kcenon::common::interfaces::IExecutor> executor_;
};

} // namespace kcenon::messaging::integration

// =============================================================================
// Health Check Adapters
// =============================================================================

export namespace kcenon::messaging::integration {

/**
 * @brief Map messaging health status to common_system health status
 */
inline kcenon::common::interfaces::health_status map_health_status(
    collectors::message_bus_health_status status) {
    switch (status) {
        case collectors::message_bus_health_status::healthy:
            return kcenon::common::interfaces::health_status::healthy;
        case collectors::message_bus_health_status::degraded:
            return kcenon::common::interfaces::health_status::degraded;
        case collectors::message_bus_health_status::unhealthy:
            return kcenon::common::interfaces::health_status::unhealthy;
        case collectors::message_bus_health_status::critical:
            return kcenon::common::interfaces::health_status::unhealthy;
        default:
            return kcenon::common::interfaces::health_status::unknown;
    }
}

/**
 * @class messaging_health_check
 * @brief Health check for overall message bus health
 */
class messaging_health_check : public kcenon::common::interfaces::health_check {
public:
    using stats_provider = std::function<collectors::message_bus_stats()>;

    messaging_health_check(
        std::string bus_name,
        stats_provider provider,
        const collectors::message_bus_health_thresholds& thresholds = {});

    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] kcenon::common::interfaces::health_check_type get_type() const override;
    kcenon::common::interfaces::health_check_result check() override;

private:
    std::string bus_name_;
    stats_provider stats_provider_;
    collectors::message_bus_health_monitor monitor_;
};

/**
 * @class queue_health_check
 * @brief Health check for message queue saturation
 */
class queue_health_check : public kcenon::common::interfaces::health_check {
public:
    using stats_provider = std::function<collectors::message_bus_stats()>;

    queue_health_check(
        std::string bus_name,
        stats_provider provider,
        double warn_threshold = 0.7,
        double critical_threshold = 0.9);

    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] kcenon::common::interfaces::health_check_type get_type() const override;
    [[nodiscard]] bool is_critical() const override;
    kcenon::common::interfaces::health_check_result check() override;

private:
    std::string bus_name_;
    stats_provider stats_provider_;
    double warn_threshold_;
    double critical_threshold_;
};

/**
 * @class transport_health_check
 * @brief Health check for transport layer connectivity
 */
class transport_health_check : public kcenon::common::interfaces::health_check {
public:
    transport_health_check(
        std::string name,
        std::shared_ptr<adapters::transport_interface> transport);

    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] kcenon::common::interfaces::health_check_type get_type() const override;
    kcenon::common::interfaces::health_check_result check() override;

private:
    std::string name_;
    std::shared_ptr<adapters::transport_interface> transport_;
};

/**
 * @brief Create a composite health check aggregating all messaging components
 */
std::shared_ptr<kcenon::common::interfaces::composite_health_check>
create_messaging_composite_check(
    const std::string& bus_name,
    std::function<collectors::message_bus_stats()> stats_provider,
    const std::unordered_map<std::string,
        std::shared_ptr<adapters::transport_interface>>& transports = {},
    const collectors::message_bus_health_thresholds& thresholds = {});

/**
 * @brief Register messaging health checks with the global health monitor
 */
kcenon::common::Result<bool> register_messaging_health_checks(
    const std::string& bus_name,
    std::function<collectors::message_bus_stats()> stats_provider,
    const std::unordered_map<std::string,
        std::shared_ptr<adapters::transport_interface>>& transports = {},
    const collectors::message_bus_health_thresholds& thresholds = {});

} // namespace kcenon::messaging::integration
