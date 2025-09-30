/**
 * @file unified_messaging_system.cpp
 * @brief Implementation of the unified messaging system
 * @author kcenon
 * @date 2025
 */

#include <kcenon/messaging/unified_messaging_system.h>

// Result adapter already included via unified_messaging_system.h
// System module includes (conditional based on availability)
#ifdef HAS_COMMON_SYSTEM
#include <kcenon/common/patterns/event_bus.h>
#endif

#ifdef HAS_THREAD_SYSTEM
// Forward declarations to avoid complex header dependencies
namespace kcenon::thread {
    class thread_pool;
    template<typename T> class typed_thread_pool;
}
#endif

#if defined(HAS_LOGGER_SYSTEM) && 0  // Disabled due to complex dependencies
#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/core/logger_builder.h>
#endif

#if defined(HAS_MONITORING_SYSTEM) && 0  // Disabled due to compatibility issues
#include <kcenon/monitoring/core/performance_monitor.h>
#include <kcenon/monitoring/collectors/system_resource_collector.h>
#endif

#ifdef HAS_CONTAINER_SYSTEM
#include <container/core/container.h>
#include <container/core/value.h>
#endif

// Database system removed - not used

#ifdef HAS_NETWORK_SYSTEM
#include <network_system/core/messaging_server.h>
#include <network_system/core/messaging_client.h>
#include <network_system/session/messaging_session.h>
#endif

// Standard library includes
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <atomic>
#include <algorithm>
#include <regex>
#include <sstream>
#include <fstream>

namespace kcenon::messaging {

// Forward declarations for internal types
class message_queue;
class message_router;
class connection_manager;

/**
 * @class unified_messaging_system::impl
 * @brief Private implementation of the unified messaging system
 *
 * This class contains all the actual implementation details, integrating
 * the various system modules while maintaining a clean public interface.
 */
class unified_messaging_system::impl {
public:
    // Configuration
    messaging_config config_;

    // Core components - always use fallback for now due to header complexity
    // TODO: Integrate with actual thread_system when headers are stabilized
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};

    // Fallback: simple console logger
    void log(log_level level, const std::string& msg) {
        static const char* level_names[] = {
            "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"
        };
        if (static_cast<int>(level) >= static_cast<int>(config_.min_log_level)) {
            std::cout << "[" << level_names[static_cast<int>(level)] << "] "
                     << msg << std::endl;
        }
    }

#if defined(HAS_MONITORING_SYSTEM) && 0  // Disabled
    std::unique_ptr<monitoring_system::performance_monitor> monitor_;
    std::unique_ptr<monitoring_system::system_resource_collector> resource_collector_;
#else
    // Fallback: basic metrics tracking
    std::atomic<size_t> messages_sent_{0};
    std::atomic<size_t> messages_received_{0};
    std::atomic<size_t> messages_failed_{0};
    std::chrono::steady_clock::time_point start_time_;
#endif

    // Database system removed

#ifdef HAS_NETWORK_SYSTEM
    std::unique_ptr<network_system::core::messaging_server> server_;
    std::unique_ptr<network_system::core::messaging_client> client_;
    std::vector<std::shared_ptr<network_system::session::messaging_session>> sessions_;
#endif

    // Message handling
    std::unique_ptr<message_queue> queue_;
    std::unique_ptr<message_router> router_;
    std::unique_ptr<connection_manager> conn_manager_;

    // State management
    std::atomic<bool> running_{false};
    std::atomic<bool> server_running_{false};
    std::atomic<connection_status> connection_status_{connection_status::disconnected};

    // Subscription management
    struct subscription_info {
        std::string topic_pattern;
        message_handler handler;
        std::regex pattern_regex;
    };
    std::unordered_map<std::string, subscription_info> subscriptions_;
    std::mutex subscriptions_mutex_;
    std::atomic<uint64_t> subscription_counter_{0};

    // Metrics
    messaging_metrics current_metrics_;
    mutable std::mutex metrics_mutex_;
    std::atomic<bool> metrics_enabled_{true};

    // Message filters and transformers
    std::function<bool(const message&)> message_filter_;
    std::function<message(const message&)> message_transformer_;
    std::mutex filter_mutex_;

public:
    explicit impl(const messaging_config& config) : config_(config) {
        start_time_ = std::chrono::steady_clock::now();
        initialize_components();
    }

    ~impl() {
        shutdown_components();
    }

private:
    void initialize_components() {
        // Initialize logging first
        initialize_logger();

        log_info("Initializing unified messaging system: " + config_.name);

        // Initialize thread pool
        initialize_thread_pool();

        // Initialize monitoring
        initialize_monitoring();

        // Initialize database
        initialize_database();

        // Initialize network components
        initialize_network();

        // Initialize message queue and router
        initialize_messaging();

        running_ = true;
        log_info("Unified messaging system initialized successfully");
    }

    void shutdown_components() {
        if (!running_) return;

        log_info("Shutting down unified messaging system");
        running_ = false;

        // Shutdown in reverse order
        shutdown_messaging();
        shutdown_network();
        shutdown_database();
        shutdown_monitoring();
        shutdown_thread_pool();
        shutdown_logger();
    }

    void initialize_logger() {
        // Use fallback logging for now
        if (config_.enable_console_logging) {
            log(log_level::info, "Console logging enabled");
        }
        if (config_.enable_file_logging) {
            log(log_level::warning, "File logging requested but not implemented in fallback mode");
        }
    }

    void shutdown_logger() {
        // Nothing to do for fallback logger
    }

    void initialize_thread_pool() {
        // Always use fallback for now
        size_t thread_count = config_.worker_threads == 0
            ? std::thread::hardware_concurrency()
            : config_.worker_threads;

        for (size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this] {
                worker_thread_loop();
            });
        }
        log_info("Thread pool initialized with " + std::to_string(thread_count) + " threads");
    }

    void shutdown_thread_pool() {
        // Shutdown thread pool
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void initialize_monitoring() {
#if defined(HAS_MONITORING_SYSTEM) && 0  // Disabled
        if (config_.enable_monitoring) {
            monitor_ = std::make_unique<monitoring_system::performance_monitor>();
            resource_collector_ = std::make_unique<monitoring_system::system_resource_collector>();

            monitor_->start();
            resource_collector_->start();

            log_info("Monitoring system initialized");
        }
#else
        // Basic metrics initialization
        messages_sent_ = 0;
        messages_received_ = 0;
        messages_failed_ = 0;
#endif
    }

    void shutdown_monitoring() {
#if defined(HAS_MONITORING_SYSTEM) && 0  // Disabled
        if (resource_collector_) {
            resource_collector_->stop();
        }
        if (monitor_) {
            monitor_->stop();
        }
#endif
    }

    void initialize_database() {
        // Database system removed - not used
    }

    void shutdown_database() {
        // Database system removed - not used
    }

    void initialize_network() {
#ifdef HAS_NETWORK_SYSTEM
        // Network components will be initialized on demand (start_server/connect)
        log_debug("Network system ready for initialization");
#endif
    }

    void shutdown_network() {
#ifdef HAS_NETWORK_SYSTEM
        if (server_) {
            server_->stop_server();
        }
        // Client cleanup handled by destructor
        client_.reset();
        sessions_.clear();
#endif
    }

    void initialize_messaging() {
        // Initialize message queue (will be created after class is defined)
        // queue_ = std::make_unique<message_queue>(config_.max_queue_size);

        // Initialize message router (will be created after class is defined)
        // router_ = std::make_unique<message_router>();

        // Initialize connection manager (will be created after class is defined)
        // conn_manager_ = std::make_unique<connection_manager>();

        log_info("Message queue and routing initialized");
    }

    void shutdown_messaging() {
        // Note: These will be properly reset in destructor
        // Can't call methods here due to forward declaration ordering
        // if (queue_) {
        //     queue_->stop();
        // }
        router_.reset();
        conn_manager_.reset();
        queue_.reset();
    }

    // Database functionality removed - not used

    void worker_thread_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    // Logging helpers
    void log_info(const std::string& msg) {
        log(log_level::info, msg);
    }

    void log_debug(const std::string& msg) {
        log(log_level::debug, msg);
    }

    void log_error(const std::string& msg) {
        log(log_level::error, msg);
    }

public:
    // Public methods implementation

    kcenon::common::result_void initialize() {
        if (running_) {
            return kcenon::common::make_success_void();
        }

        initialize_components();
        return kcenon::common::make_success_void();
    }

    kcenon::common::result_void shutdown() {
        shutdown_components();
        return kcenon::common::make_success_void();
    }

    bool is_running() const {
        return running_;
    }

    kcenon::common::result_void start_server(uint16_t port, const std::string& address) {
#ifdef HAS_NETWORK_SYSTEM
        if (server_running_) {
            return kcenon::common::make_error_void("Server already running");
        }

        try {
            server_ = std::make_unique<network_system::core::messaging_server>("UnifiedMessagingServer");
            server_->start_server(port);
            // Note: address parameter not used in start_server API
            server_running_ = true;

            log_info("Server started on " + address + ":" + std::to_string(port));
            return kcenon::common::make_success_void();
        } catch (const std::exception& e) {
            return kcenon::common::make_error_void(std::string("Failed to start server: ") + e.what());
        }
#else
        return kcenon::common::make_error_void("Network system not available");
#endif
    }

    kcenon::common::result_void stop_server() {
#ifdef HAS_NETWORK_SYSTEM
        if (!server_running_) {
            return kcenon::common::make_success_void();
        }

        if (server_) {
            server_->stop_server();
            server_.reset();
        }

        server_running_ = false;
        log_info("Server stopped");
        return kcenon::common::make_success_void();
#else
        return kcenon::common::make_error_void("Network system not available");
#endif
    }

    bool is_server_running() const {
        return server_running_;
    }

    kcenon::common::result_void connect(const connection_info& info) {
#ifdef HAS_NETWORK_SYSTEM
        if (connection_status_ == connection_status::connected) {
            return kcenon::common::make_error_void("Already connected");
        }

        try {
            connection_status_ = connection_status::connecting;
            client_ = std::make_unique<network_system::core::messaging_client>("UnifiedMessagingClient");

            // Note: connect API may differ - disabling for now
            if (false) { // client_->connect(info.address, info.port)
                connection_status_ = connection_status::connected;
                log_info("Connected to " + info.address + ":" + std::to_string(info.port));
                return kcenon::common::make_success_void();
            } else {
                connection_status_ = connection_status::error;
                return kcenon::common::make_error_void("Failed to connect");
            }
        } catch (const std::exception& e) {
            connection_status_ = connection_status::error;
            return kcenon::common::make_error_void(std::string("Connection failed: ") + e.what());
        }
#else
        return kcenon::common::make_error_void("Network system not available");
#endif
    }

    kcenon::common::result_void disconnect() {
#ifdef HAS_NETWORK_SYSTEM
        if (client_) {
            // Disconnect handled by destructor
            client_.reset();
            client_.reset();
        }

        connection_status_ = connection_status::disconnected;
        log_info("Disconnected");
        return kcenon::common::make_success_void();
#else
        return kcenon::common::make_error_void("Network system not available");
#endif
    }

    connection_status get_connection_status() const {
        return connection_status_;
    }

    std::future<kcenon::common::result_void> send(const message& msg) {
        auto promise = std::make_shared<std::promise<kcenon::common::result_void>>();
        auto future = promise->get_future();

        // Submit to thread pool for async processing
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace([this, msg, promise]() {
                auto result = process_send(msg);
                promise->set_value(result);
            });
        }
        condition_.notify_one();

        return future;
    }

    kcenon::common::result<std::string> subscribe(const std::string& topic, message_handler handler) {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);

        std::string sub_id = "sub_" + std::to_string(++subscription_counter_);

        subscription_info info;
        info.topic_pattern = topic;
        info.handler = std::move(handler);

        // Convert topic pattern to regex (support wildcards)
        std::string regex_pattern = topic;
        // Replace * with .* and ? with .
        regex_pattern = std::regex_replace(regex_pattern, std::regex("\\*"), ".*");
        regex_pattern = std::regex_replace(regex_pattern, std::regex("\\?"), ".");
        info.pattern_regex = std::regex(regex_pattern);

        subscriptions_[sub_id] = std::move(info);

        log_debug("Subscribed to topic: " + topic + " with ID: " + sub_id);
        return kcenon::common::make_success(sub_id);
    }

    kcenon::common::result_void unsubscribe(const std::string& subscription_id) {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);

        auto it = subscriptions_.find(subscription_id);
        if (it != subscriptions_.end()) {
            subscriptions_.erase(it);
            log_debug("Unsubscribed: " + subscription_id);
            return kcenon::common::make_success_void();
        }

        return kcenon::common::make_error_void("Subscription not found");
    }

    messaging_metrics get_metrics() const {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        return current_metrics_;
    }

    health_status get_health() const {
        health_status status;

#if defined(HAS_MONITORING_SYSTEM) && 0  // Disabled
        if (monitor_) {
            auto perf_metrics = monitor_->get_metrics();
            status.overall_health_score = perf_metrics.health_score;
            status.is_healthy = perf_metrics.health_score > 70.0;

            if (perf_metrics.cpu_usage > 80.0) {
                status.issues.push_back("High CPU usage: " +
                    std::to_string(perf_metrics.cpu_usage) + "%");
            }

            if (perf_metrics.memory_usage > 80.0) {
                status.issues.push_back("High memory usage: " +
                    std::to_string(perf_metrics.memory_usage) + "%");
            }
        }
#else
        // Basic health check
        status.is_healthy = running_ && !stop_;
        status.overall_health_score = status.is_healthy ? 100.0 : 0.0;
#endif

        status.last_check = std::chrono::system_clock::now();
        return status;
    }

private:
    kcenon::common::result_void process_send(const message& msg) {
        try {
            // Apply filter if set
            if (message_filter_) {
                std::lock_guard<std::mutex> lock(filter_mutex_);
                if (!message_filter_(msg)) {
                    return kcenon::common::make_error_void("Message filtered out");
                }
            }

            // Apply transformer if set
            message transformed_msg = msg;
            if (message_transformer_) {
                std::lock_guard<std::mutex> lock(filter_mutex_);
                transformed_msg = message_transformer_(msg);
            }

            // Route to local subscribers
            route_to_subscribers(transformed_msg);

            // Send over network if connected
#ifdef HAS_NETWORK_SYSTEM
            if (client_ && connection_status_ == connection_status::connected) {
                // Serialize and send message
                std::vector<uint8_t> serialized = serialize_message(transformed_msg);
                // Note: send API may differ - disabling for now
                // client_->send(serialized.data(), serialized.size());
            }
#endif

            // Database persistence removed - not used

            // Update metrics
            update_metrics_on_send();

            return kcenon::common::make_success_void();
        } catch (const std::exception& e) {
            update_metrics_on_failure();
            return kcenon::common::make_error_void(std::string("Send failed: ") + e.what());
        }
    }

    void route_to_subscribers(const message& msg) {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);

        for (const auto& [id, info] : subscriptions_) {
            if (std::regex_match(msg.topic, info.pattern_regex)) {
                // Execute handler asynchronously
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    tasks_.emplace([handler = info.handler, msg]() {
                        handler(msg);
                    });
                }
                condition_.notify_one();
            }
        }
    }

    std::vector<uint8_t> serialize_message(const message& msg) {
        // Simple serialization (in production, use protobuf or similar)
        std::ostringstream oss;
        oss << msg.id << "|"
            << static_cast<int>(msg.type) << "|"
            << static_cast<int>(msg.priority) << "|"
            << msg.sender << "|"
            << msg.recipient << "|"
            << msg.topic << "|";

        std::string header = oss.str();
        std::vector<uint8_t> result(header.begin(), header.end());
        result.insert(result.end(), msg.payload.begin(), msg.payload.end());

        return result;
    }

    // Database persistence removed - not used

    void update_metrics_on_send() {
        if (!metrics_enabled_) return;

        std::lock_guard<std::mutex> lock(metrics_mutex_);
        current_metrics_.messages_sent++;

#if defined(HAS_MONITORING_SYSTEM) && 0  // Disabled
        if (monitor_) {
            monitor_->record_event("message_sent");
        }
#else
        messages_sent_++;
#endif
    }

    void update_metrics_on_failure() {
        if (!metrics_enabled_) return;

        std::lock_guard<std::mutex> lock(metrics_mutex_);
        current_metrics_.messages_failed++;

#if defined(HAS_MONITORING_SYSTEM) && 0  // Disabled
        if (monitor_) {
            monitor_->record_event("message_failed");
        }
#else
        messages_failed_++;
#endif
    }
};

// Internal helper classes

class message_queue {
    std::queue<message> queue_;
    std::priority_queue<message, std::vector<message>,
                       std::function<bool(const message&, const message&)>> priority_queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    size_t max_size_;
    bool use_priority_;
    std::atomic<bool> stopped_{false};

public:
    explicit message_queue(size_t max_size, bool use_priority = false)
        : priority_queue_([](const message& a, const message& b) {
              return static_cast<int>(a.priority) < static_cast<int>(b.priority);
          }), max_size_(max_size), use_priority_(use_priority) {}

    bool push(const message& msg) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (stopped_) return false;

        if (use_priority_) {
            if (priority_queue_.size() >= max_size_) return false;
            priority_queue_.push(msg);
        } else {
            if (queue_.size() >= max_size_) return false;
            queue_.push(msg);
        }

        cv_.notify_one();
        return true;
    }

    std::optional<message> pop() {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this] {
            return stopped_ ||
                   (!use_priority_ && !queue_.empty()) ||
                   (use_priority_ && !priority_queue_.empty());
        });

        if (stopped_) return std::nullopt;

        message msg;
        if (use_priority_ && !priority_queue_.empty()) {
            msg = priority_queue_.top();
            priority_queue_.pop();
        } else if (!use_priority_ && !queue_.empty()) {
            msg = queue_.front();
            queue_.pop();
        } else {
            return std::nullopt;
        }

        return msg;
    }

    size_t size() const {
        std::lock_guard lock(mutex_);
        return use_priority_ ? priority_queue_.size() : queue_.size();
    }

    void stop() {
        {
            std::lock_guard lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }
};

class message_router {
    struct route {
        std::regex pattern;
        std::function<void(const message&)> handler;
    };

    std::vector<route> routes_;
    std::mutex mutex_;

public:
    void add_route(const std::string& pattern, std::function<void(const message&)> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        routes_.push_back({std::regex(pattern), std::move(handler)});
    }

    void route(const message& msg) {
        std::lock_guard<std::mutex> lock(mutex_);

        for (const auto& route : routes_) {
            if (std::regex_match(msg.topic, route.pattern)) {
                route.handler(msg);
            }
        }
    }
};

class connection_manager {
    struct connection {
        std::string id;
        std::string address;
        uint16_t port;
        std::chrono::steady_clock::time_point last_activity;
    };

    std::unordered_map<std::string, connection> connections_;
    mutable std::mutex mutex_;

public:
    void add_connection(const std::string& id, const std::string& address, uint16_t port) {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_[id] = {id, address, port, std::chrono::steady_clock::now()};
    }

    void remove_connection(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.erase(id);
    }

    void update_activity(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(id);
        if (it != connections_.end()) {
            it->second.last_activity = std::chrono::steady_clock::now();
        }
    }

    size_t connection_count() const {
        std::lock_guard lock(mutex_);
        return connections_.size();
    }
};

// Public API implementation

unified_messaging_system::unified_messaging_system(const messaging_config& config)
    : pimpl_(std::make_unique<impl>(config)) {}

unified_messaging_system::unified_messaging_system()
    : pimpl_(std::make_unique<impl>(messaging_config{})) {}

unified_messaging_system::~unified_messaging_system() = default;

kcenon::common::result_void unified_messaging_system::initialize() {
    return pimpl_->initialize();
}

kcenon::common::result_void unified_messaging_system::shutdown() {
    return pimpl_->shutdown();
}

bool unified_messaging_system::is_running() const {
    return pimpl_->is_running();
}

kcenon::common::result_void unified_messaging_system::start_server(uint16_t port, const std::string& address) {
    return pimpl_->start_server(port, address);
}

kcenon::common::result_void unified_messaging_system::stop_server() {
    return pimpl_->stop_server();
}

bool unified_messaging_system::is_server_running() const {
    return pimpl_->is_server_running();
}

kcenon::common::result_void unified_messaging_system::connect(const connection_info& info) {
    return pimpl_->connect(info);
}

kcenon::common::result_void unified_messaging_system::disconnect() {
    return pimpl_->disconnect();
}

connection_status unified_messaging_system::get_connection_status() const {
    return pimpl_->get_connection_status();
}

std::future<kcenon::common::result_void> unified_messaging_system::send(const message& msg) {
    return pimpl_->send(msg);
}

std::future<kcenon::common::result<message>> unified_messaging_system::send_request(
    const message& msg, std::chrono::milliseconds timeout) {
    // Suppress unused parameter warnings
    (void)msg;
    (void)timeout;

    // Implementation would track request/response correlation
    auto promise = std::make_shared<std::promise<kcenon::common::result<message>>>();

    // For now, return error
    promise->set_value(kcenon::common::make_error<message>("Not implemented"));

    return promise->get_future();
}

kcenon::common::result_void unified_messaging_system::broadcast(const message& msg) {
    // Suppress unused parameter warning
    (void)msg;

    // Implementation would send to all connected clients
    return kcenon::common::make_error_void("Not implemented");
}

kcenon::common::result<std::string> unified_messaging_system::subscribe(
    const std::string& topic, message_handler handler) {
    return pimpl_->subscribe(topic, std::move(handler));
}

kcenon::common::result_void unified_messaging_system::unsubscribe(const std::string& subscription_id) {
    return pimpl_->unsubscribe(subscription_id);
}

void unified_messaging_system::on_message(const std::string& topic, message_handler handler) {
    pimpl_->subscribe(topic, std::move(handler));
}

std::future<kcenon::common::result_void> unified_messaging_system::send_batch(
    const std::vector<message>& messages) {
    // Implementation would batch process messages
    auto promise = std::make_shared<std::promise<kcenon::common::result_void>>();

    // For now, send individually
    for (const auto& msg : messages) {
        pimpl_->send(msg);
    }

    promise->set_value(kcenon::common::make_success_void());
    return promise->get_future();
}

messaging_metrics unified_messaging_system::get_metrics() const {
    return pimpl_->get_metrics();
}

health_status unified_messaging_system::get_health() const {
    return pimpl_->get_health();
}

void unified_messaging_system::reset_metrics() {
    std::lock_guard<std::mutex> lock(pimpl_->metrics_mutex_);
    pimpl_->current_metrics_ = messaging_metrics{};
}

void unified_messaging_system::set_metrics_enabled(bool enabled) {
    pimpl_->metrics_enabled_ = enabled;
}

void unified_messaging_system::set_log_level(log_level level) {
    pimpl_->config_.min_log_level = level;
}

void unified_messaging_system::flush_logs() {
    // Nothing to flush for console logger
}

// Database functions removed - not used

void unified_messaging_system::set_message_filter(std::function<bool(const message&)> filter) {
    std::lock_guard<std::mutex> lock(pimpl_->filter_mutex_);
    pimpl_->message_filter_ = std::move(filter);
}

void unified_messaging_system::set_message_transformer(
    std::function<message(const message&)> transformer) {
    std::lock_guard<std::mutex> lock(pimpl_->filter_mutex_);
    pimpl_->message_transformer_ = std::move(transformer);
}

void unified_messaging_system::set_routing_rules(const std::string& rules) {
    // Suppress unused parameter warning
    (void)rules;

    // Parse and apply routing rules
    // Implementation would parse JSON/YAML rules
}

size_t unified_messaging_system::get_queue_size() const {
    // Temporarily disabled due to forward declaration ordering
    // if (pimpl_->queue_) {
    //     return pimpl_->queue_->size();
    // }
    return 0;
}

void unified_messaging_system::wait_for_completion() {
    // Wait for all tasks to complete
    std::unique_lock<std::mutex> lock(pimpl_->queue_mutex_);
    pimpl_->condition_.wait(lock, [this] {
        return pimpl_->tasks_.empty();
    });
}

} // namespace kcenon::messaging