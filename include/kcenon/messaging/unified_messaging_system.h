/**
 * @file unified_messaging_system.h
 * @brief Unified messaging system with integrated logging, monitoring, and threading
 * @author kcenon
 * @date 2025
 *
 * This header provides a unified interface to the messaging system that integrates:
 * - Thread pool for async message processing (thread_system)
 * - Comprehensive logging (logger_system)
 * - Performance monitoring (monitoring_system)
 * - Flexible data containers (container_system)
 * - Database persistence (database_system)
 * - Network communication (network_system)
 * - Common patterns like Result and EventBus (common_system)
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <chrono>
#include <any>
#include <optional>

// Include the result adapter for proper Result type handling
#include "result_adapter.h"

namespace kcenon::messaging {

/**
 * @brief Message priority levels
 */
enum class message_priority {
    low = 0,
    normal = 1,
    high = 2,
    critical = 3
};

/**
 * @brief Message types for routing
 */
enum class message_type {
    request,
    response,
    notification,
    broadcast,
    heartbeat
};

/**
 * @brief Connection status
 */
enum class connection_status {
    disconnected,
    connecting,
    connected,
    error
};

/**
 * @brief Log levels (matching thread_system)
 */
enum class log_level {
    trace, debug, info, warning, error, critical
};

/**
 * @brief Performance metrics for messaging system
 */
struct messaging_metrics {
    size_t messages_sent{0};
    size_t messages_received{0};
    size_t messages_failed{0};
    size_t messages_in_queue{0};
    std::chrono::nanoseconds average_processing_time{0};
    std::chrono::nanoseconds max_processing_time{0};
    double throughput_per_second{0.0};
    size_t active_connections{0};
    size_t total_connections{0};
    double cpu_usage_percent{0.0};
    double memory_usage_mb{0.0};
};

/**
 * @brief System health status
 */
struct health_status {
    bool is_healthy{true};
    double overall_health_score{100.0}; // 0-100
    std::vector<std::string> issues;
    std::chrono::system_clock::time_point last_check;
};

/**
 * @brief Message structure
 */
struct message {
    std::string id;
    message_type type{message_type::notification};
    message_priority priority{message_priority::normal};
    std::string sender;
    std::string recipient;
    std::string topic;
    std::vector<uint8_t> payload;
    std::chrono::system_clock::time_point timestamp;
    std::any metadata;

    // Helper methods
    template<typename T>
    void set_payload(const T& data);

    template<typename T>
    std::optional<T> get_payload() const;
};

/**
 * @brief Message handler callback type
 */
using message_handler = std::function<void(const message&)>;
using async_message_handler = std::function<std::future<void>(const message&)>;

/**
 * @brief Connection info for clients/servers
 */
struct connection_info {
    std::string address;
    uint16_t port;
    bool use_ssl{false};
    std::chrono::seconds timeout{30};
    std::string certificate_path;
    std::string private_key_path;
};

/**
 * @brief Unified messaging system configuration
 */
struct messaging_config {
    // System identification
    std::string name = "MessagingSystem";

    // Thread pool settings
    size_t worker_threads = 0; // 0 = auto-detect
    size_t io_threads = 2;

    // Queue settings
    size_t max_queue_size = 10000;
    bool use_priority_queue = true;

    // Logging settings
    bool enable_console_logging = true;
    bool enable_file_logging = true;
    std::string log_directory = "./logs";
    log_level min_log_level = log_level::info;

    // Monitoring settings
    bool enable_monitoring = true;
    bool enable_metrics_collection = true;
    std::chrono::seconds metrics_interval{60};

    // Database settings
    bool enable_persistence = false;
    std::string db_connection_string;
    bool archive_old_messages = true;
    std::chrono::hours message_retention{24 * 7}; // 1 week

    // Network settings
    bool enable_compression = true;
    bool enable_encryption = false;
    size_t max_message_size = 1024 * 1024; // 1MB
    std::chrono::seconds connection_timeout{30};

    // Performance tuning
    bool use_lockfree_queues = false;
    bool enable_batching = true;
    size_t batch_size = 100;
    std::chrono::milliseconds batch_timeout{100};
};

/**
 * @class unified_messaging_system
 * @brief Main unified messaging system class
 *
 * This class provides a unified interface to all messaging functionality,
 * integrating multiple system modules behind a clean API. It manages:
 * - Asynchronous message processing with thread pools
 * - Network communication for distributed messaging
 * - Message persistence to databases
 * - Comprehensive logging and monitoring
 * - Flexible message routing and filtering
 *
 * @example Basic usage:
 * @code
 * unified_messaging_system messaging;
 *
 * // Start as server
 * messaging.start_server(8080);
 *
 * // Register message handler
 * messaging.on_message("topic/test", [](const message& msg) {
 *     std::cout << "Received: " << msg.id << std::endl;
 * });
 *
 * // Send message
 * message msg;
 * msg.topic = "topic/test";
 * msg.set_payload("Hello World");
 * messaging.send(msg);
 * @endcode
 */
class unified_messaging_system {
public:
    /**
     * @brief Default constructor with auto-configuration
     */
    unified_messaging_system();

    /**
     * @brief Construct with configuration
     * @param config System configuration
     */
    explicit unified_messaging_system(const messaging_config& config);

    /**
     * @brief Destructor - ensures graceful shutdown
     */
    ~unified_messaging_system();

    // Non-copyable but movable
    unified_messaging_system(const unified_messaging_system&) = delete;
    unified_messaging_system& operator=(const unified_messaging_system&) = delete;
    unified_messaging_system(unified_messaging_system&&) = default;
    unified_messaging_system& operator=(unified_messaging_system&&) = default;

    // ============= Core Operations =============

    /**
     * @brief Initialize the messaging system
     * @return Success or error result
     */
    kcenon::common::result_void initialize();

    /**
     * @brief Shutdown the messaging system gracefully
     * @return Success or error result
     */
    kcenon::common::result_void shutdown();

    /**
     * @brief Check if system is running
     */
    bool is_running() const;

    // ============= Server Operations =============

    /**
     * @brief Start messaging server
     * @param port Port to listen on
     * @param address Address to bind (default: "0.0.0.0")
     * @return Success or error result
     */
    kcenon::common::result_void start_server(uint16_t port,
                                            const std::string& address = "0.0.0.0");

    /**
     * @brief Stop messaging server
     * @return Success or error result
     */
    kcenon::common::result_void stop_server();

    /**
     * @brief Check if server is running
     */
    bool is_server_running() const;

    // ============= Client Operations =============

    /**
     * @brief Connect to a messaging server
     * @param info Connection information
     * @return Success or error result
     */
    kcenon::common::result_void connect(const connection_info& info);

    /**
     * @brief Disconnect from server
     * @return Success or error result
     */
    kcenon::common::result_void disconnect();

    /**
     * @brief Get connection status
     */
    connection_status get_connection_status() const;

    // ============= Message Operations =============

    /**
     * @brief Send a message
     * @param msg Message to send
     * @return Future containing send result
     */
    std::future<kcenon::common::result_void> send(const message& msg);

    /**
     * @brief Send message and wait for response
     * @param msg Message to send
     * @param timeout Response timeout
     * @return Future containing response message
     */
    std::future<kcenon::common::result<message>> send_request(
        const message& msg,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    /**
     * @brief Broadcast message to all connected clients
     * @param msg Message to broadcast
     * @return Success or error result
     */
    kcenon::common::result_void broadcast(const message& msg);

    /**
     * @brief Subscribe to a topic
     * @param topic Topic pattern (supports wildcards)
     * @param handler Message handler callback
     * @return Subscription ID for unsubscribing
     */
    kcenon::common::result<std::string> subscribe(
        const std::string& topic,
        message_handler handler);

    /**
     * @brief Unsubscribe from a topic
     * @param subscription_id ID returned from subscribe
     * @return Success or error result
     */
    kcenon::common::result_void unsubscribe(const std::string& subscription_id);

    /**
     * @brief Register a message handler (simplified API)
     * @param topic Topic or pattern
     * @param handler Handler function
     */
    void on_message(const std::string& topic, message_handler handler);

    // ============= Batch Operations =============

    /**
     * @brief Send multiple messages in batch
     * @param messages Messages to send
     * @return Future containing batch result
     */
    std::future<kcenon::common::result_void> send_batch(
        const std::vector<message>& messages);

    /**
     * @brief Process messages in parallel
     * @param messages Messages to process
     * @param processor Processing function
     * @return Vector of futures for each message
     */
    template<typename F>
    std::vector<std::future<typename std::invoke_result<F, message>::type>>
    process_parallel(const std::vector<message>& messages, F&& processor);

    // ============= Monitoring & Metrics =============

    /**
     * @brief Get current performance metrics
     */
    messaging_metrics get_metrics() const;

    /**
     * @brief Get system health status
     */
    health_status get_health() const;

    /**
     * @brief Reset metrics counters
     */
    void reset_metrics();

    /**
     * @brief Enable/disable metric collection
     */
    void set_metrics_enabled(bool enabled);

    // ============= Logging =============

    /**
     * @brief Log a message
     * @param level Log level
     * @param message Log message
     * @param args Format arguments
     */
    template<typename... Args>
    void log(log_level level, const std::string& message, Args&&... args);

    /**
     * @brief Set minimum log level
     */
    void set_log_level(log_level level);

    /**
     * @brief Flush all pending logs
     */
    void flush_logs();

    // ============= Persistence =============
    // Database functionality removed - not used

    // ============= Advanced Features =============

    /**
     * @brief Set custom message filter
     * @param filter Filter function
     */
    void set_message_filter(std::function<bool(const message&)> filter);

    /**
     * @brief Set custom message transformer
     * @param transformer Transform function
     */
    void set_message_transformer(
        std::function<message(const message&)> transformer);

    /**
     * @brief Enable message routing rules
     * @param rules Routing rules configuration
     */
    void set_routing_rules(const std::string& rules);

    /**
     * @brief Get queue size
     */
    size_t get_queue_size() const;

    /**
     * @brief Wait for all pending messages to be processed
     */
    void wait_for_completion();

private:
    // PIMPL idiom for implementation hiding
    class impl;
    std::unique_ptr<impl> pimpl_;
};

// Template implementations
template<typename T>
void message::set_payload(const T& data) {
    // Implementation will serialize data to payload
    // This is a placeholder - actual implementation in .cpp
    (void)data; // Suppress unused parameter warning
}

template<typename T>
std::optional<T> message::get_payload() const {
    // Implementation will deserialize payload to T
    // This is a placeholder - actual implementation in .cpp
    return std::nullopt;
}

template<typename F>
std::vector<std::future<typename std::invoke_result<F, message>::type>>
unified_messaging_system::process_parallel(const std::vector<message>& messages, F&& processor) {
    // Implementation will submit tasks to thread pool
    // This is a placeholder - actual implementation in .cpp
    (void)messages; // Suppress unused parameter warning
    (void)processor; // Suppress unused parameter warning
    std::vector<std::future<typename std::invoke_result<F, message>::type>> futures;
    return futures;
}

template<typename... Args>
void unified_messaging_system::log(log_level level, const std::string& message, Args&&... args) {
    // Implementation will forward to logger system
    // This is a placeholder - actual implementation in .cpp
    (void)level; // Suppress unused parameter warning
    (void)message; // Suppress unused parameter warning
    ((void)args, ...); // Suppress unused parameter warning for variadic args
}

} // namespace kcenon::messaging

/**
 * @brief Convenience alias for backward compatibility
 */
using messaging_system = kcenon::messaging::unified_messaging_system;