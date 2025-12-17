// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file message_bus_collector.h
 * @brief Metric collector for message bus performance monitoring
 *
 * This header provides a metric collector plugin that integrates with
 * monitoring_system to collect message bus performance metrics.
 *
 * Collected Metrics:
 * - Message throughput (messages/sec)
 * - Message latency
 * - Queue depth
 * - Subscriber count per topic
 *
 * Prometheus-compatible metric names:
 * - messaging_messages_published_total
 * - messaging_messages_processed_total
 * - messaging_messages_failed_total
 * - messaging_messages_dropped_total
 * - messaging_queue_depth
 * - messaging_throughput_per_second
 * - messaging_latency_ms
 * - messaging_subscribers_per_topic
 */

#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/message_bus.h"

#ifdef BUILD_WITH_MONITORING_SYSTEM
#include <kcenon/monitoring/collectors/plugin_metric_collector.h>
#include <kcenon/monitoring/core/event_bus.h>
#include <kcenon/monitoring/core/event_types.h>
#endif

namespace kcenon::messaging::collectors {

/**
 * @struct message_bus_stats
 * @brief Statistics snapshot from message bus
 */
struct message_bus_stats {
    // Message counts
    uint64_t messages_published{0};
    uint64_t messages_processed{0};
    uint64_t messages_failed{0};
    uint64_t messages_dropped{0};

    // Queue metrics
    size_t queue_depth{0};
    size_t queue_capacity{0};
    double queue_utilization_percent{0.0};

    // Throughput metrics
    double throughput_per_second{0.0};
    double average_latency_ms{0.0};
    double max_latency_ms{0.0};
    double min_latency_ms{0.0};

    // Topic metrics
    size_t topic_count{0};
    size_t total_subscriber_count{0};
    std::unordered_map<std::string, size_t> subscribers_per_topic;

    // Worker metrics
    size_t worker_thread_count{0};
    bool is_running{false};
};

/**
 * @struct latency_sample
 * @brief Sample data for latency calculation
 */
struct latency_sample {
    double latency_ms;
    std::chrono::steady_clock::time_point timestamp;
};

#ifdef BUILD_WITH_MONITORING_SYSTEM

/**
 * @struct messaging_metric_event
 * @brief Event type for messaging metrics
 */
struct messaging_metric_event {
    std::string metric_name;
    double value;
    std::unordered_map<std::string, std::string> labels;
    std::chrono::steady_clock::time_point timestamp;
};

/**
 * @class message_bus_collector
 * @brief Metric collector plugin for message bus monitoring
 *
 * Collects metrics from message bus instances and publishes them to
 * the monitoring_system. Supports Prometheus-compatible metric names
 * and standard labeling conventions.
 *
 * Thread Safety:
 * - All public methods are thread-safe
 * - Internal state protected by mutexes
 *
 * Usage Example:
 * @code
 * auto bus = std::make_shared<message_bus>(...);
 * message_bus_collector collector;
 *
 * // Initialize with configuration
 * std::unordered_map<std::string, std::string> config;
 * config["enable_latency_tracking"] = "true";
 * config["latency_sample_size"] = "1000";
 * collector.initialize(config);
 *
 * // Register message bus
 * collector.set_message_bus(bus);
 *
 * // Collect metrics
 * auto metrics = collector.collect();
 * @endcode
 */
class message_bus_collector : public monitoring::metric_collector_plugin {
public:
    message_bus_collector();
    ~message_bus_collector() override;

    // Disable copy
    message_bus_collector(const message_bus_collector&) = delete;
    message_bus_collector& operator=(const message_bus_collector&) = delete;

    // Disable move (contains mutex)
    message_bus_collector(message_bus_collector&&) = delete;
    message_bus_collector& operator=(message_bus_collector&&) = delete;

    // ========================================================================
    // metric_collector_plugin interface
    // ========================================================================

    /**
     * @brief Initialize the collector with configuration
     * @param config Configuration map with the following options:
     *        - enable_latency_tracking: "true" or "false" (default: true)
     *        - latency_sample_size: number of samples to keep (default: 1000)
     *        - enable_topic_metrics: "true" or "false" (default: true)
     *        - use_event_bus: "true" or "false" (default: true)
     * @return true if initialization successful
     */
    bool initialize(const std::unordered_map<std::string, std::string>& config) override;

    /**
     * @brief Collect metrics from registered message buses
     * @return Vector of collected metrics
     */
    std::vector<monitoring::metric> collect() override;

    /**
     * @brief Get the collector name
     * @return "message_bus_collector"
     */
    std::string get_name() const override { return "message_bus_collector"; }

    /**
     * @brief Get supported metric types
     * @return Vector of metric type names
     */
    std::vector<std::string> get_metric_types() const override;

    /**
     * @brief Check if collector is healthy
     * @return true if operational
     */
    bool is_healthy() const override;

    /**
     * @brief Get collector statistics
     * @return Map of statistic name to value
     */
    std::unordered_map<std::string, double> get_statistics() const override;

    // ========================================================================
    // Message Bus Registration
    // ========================================================================

    /**
     * @brief Set the primary message bus for monitoring
     * @param bus Shared pointer to message bus
     */
    void set_message_bus(std::shared_ptr<message_bus> bus);

    /**
     * @brief Register additional message bus with custom name
     * @param name Name identifier for the message bus
     * @param stats_provider Function that returns current stats
     */
    void register_message_bus(const std::string& name,
                               std::function<message_bus_stats()> stats_provider);

    /**
     * @brief Unregister a message bus
     * @param name Name of the message bus to unregister
     */
    void unregister_message_bus(const std::string& name);

    /**
     * @brief Get all registered message bus names
     * @return Vector of registered bus names
     */
    std::vector<std::string> get_registered_buses() const;

    // ========================================================================
    // Latency Tracking
    // ========================================================================

    /**
     * @brief Record a message latency sample
     * @param bus_name Name of the message bus
     * @param latency_ms Latency in milliseconds
     */
    void record_latency(const std::string& bus_name, double latency_ms);

    /**
     * @brief Get latency statistics for a message bus
     * @param bus_name Name of the message bus
     * @return Optional latency statistics
     */
    std::optional<std::tuple<double, double, double>> get_latency_stats(
        const std::string& bus_name) const;

    // ========================================================================
    // Configuration
    // ========================================================================

    /**
     * @brief Enable or disable latency tracking
     * @param enable true to enable
     */
    void set_latency_tracking(bool enable);

    /**
     * @brief Enable or disable per-topic metrics
     * @param enable true to enable
     */
    void set_topic_metrics(bool enable);

    /**
     * @brief Set latency sample size
     * @param size Maximum number of samples to keep
     */
    void set_latency_sample_size(size_t size);

private:
    // Message bus management
    std::shared_ptr<message_bus> primary_bus_;
    mutable std::mutex buses_mutex_;
    std::unordered_map<std::string, std::function<message_bus_stats()>> bus_providers_;
    std::unordered_map<std::string, message_bus_stats> last_stats_;

    // Latency tracking
    mutable std::mutex latency_mutex_;
    std::unordered_map<std::string, std::deque<latency_sample>> latency_samples_;
    size_t max_latency_samples_{1000};

    // Throughput tracking
    struct throughput_tracker {
        std::chrono::steady_clock::time_point window_start;
        uint64_t messages_at_start{0};
        double current_throughput{0.0};
    };
    mutable std::mutex throughput_mutex_;
    std::unordered_map<std::string, throughput_tracker> throughput_trackers_;

    // Configuration
    bool enable_latency_tracking_{true};
    bool enable_topic_metrics_{true};
    bool use_event_bus_{true};

    // Statistics tracking
    mutable std::mutex stats_mutex_;
    std::atomic<size_t> collection_count_{0};
    std::atomic<size_t> collection_errors_{0};
    std::atomic<bool> is_healthy_{true};
    std::chrono::steady_clock::time_point init_time_;

    // Event bus integration
    std::shared_ptr<monitoring::event_bus> event_bus_;

    // Helper methods
    message_bus_stats collect_from_primary_bus();
    std::vector<monitoring::metric> collect_from_buses();
    void add_bus_metrics(std::vector<monitoring::metric>& metrics,
                         const std::string& bus_name,
                         const message_bus_stats& stats);
    void add_topic_metrics(std::vector<monitoring::metric>& metrics,
                           const std::string& bus_name,
                           const message_bus_stats& stats);
    void update_throughput_tracking(const std::string& bus_name,
                                     const message_bus_stats& stats);
    std::tuple<double, double, double> calculate_latency_stats(
        const std::deque<latency_sample>& samples) const;
    monitoring::metric create_metric(const std::string& name,
                                      double value,
                                      const std::string& bus_name) const;
    void subscribe_to_events();
    void handle_messaging_event(const messaging_metric_event& event);
};

#else // !BUILD_WITH_MONITORING_SYSTEM

/**
 * @class message_bus_collector
 * @brief Stub implementation when monitoring_system is not available
 *
 * This provides a minimal interface for code that wants to use
 * message_bus_collector but doesn't have monitoring_system available.
 */
class message_bus_collector {
public:
    message_bus_collector() = default;
    ~message_bus_collector() = default;

    bool initialize(const std::unordered_map<std::string, std::string>&) {
        return false;
    }

    std::string get_name() const { return "message_bus_collector"; }

    std::vector<std::string> get_metric_types() const {
        return {};
    }

    bool is_healthy() const { return false; }

    std::unordered_map<std::string, double> get_statistics() const {
        return {};
    }

    void set_message_bus(std::shared_ptr<message_bus>) {}

    void register_message_bus(const std::string&,
                               std::function<message_bus_stats()>) {}

    void unregister_message_bus(const std::string&) {}

    std::vector<std::string> get_registered_buses() const {
        return {};
    }

    void record_latency(const std::string&, double) {}

    std::optional<std::tuple<double, double, double>> get_latency_stats(
        const std::string&) const {
        return std::nullopt;
    }

    void set_latency_tracking(bool) {}
    void set_topic_metrics(bool) {}
    void set_latency_sample_size(size_t) {}
};

#endif // BUILD_WITH_MONITORING_SYSTEM

/**
 * @enum message_bus_health_status
 * @brief Health status levels for message bus
 */
enum class message_bus_health_status {
    healthy,
    degraded,
    unhealthy,
    critical
};

/**
 * @struct message_bus_health_thresholds
 * @brief Threshold configuration for health monitoring
 */
struct message_bus_health_thresholds {
    // Queue thresholds
    double queue_saturation_warn = 0.7;
    double queue_saturation_critical = 0.9;

    // Failure rate thresholds
    double failure_rate_warn = 0.05;
    double failure_rate_critical = 0.1;

    // Latency thresholds (ms)
    double latency_warn_ms = 100.0;
    double latency_critical_ms = 500.0;

    // Throughput thresholds
    double throughput_drop_warn = 0.5;
    double throughput_drop_critical = 0.8;
};

/**
 * @struct message_bus_health_report
 * @brief Health report for a message bus
 */
struct message_bus_health_report {
    std::string bus_name;
    message_bus_health_status status;
    std::vector<std::string> issues;
    std::unordered_map<std::string, double> metrics;
    std::chrono::steady_clock::time_point timestamp;
};

/**
 * @class message_bus_health_monitor
 * @brief Health monitor for message bus
 *
 * Monitors message bus health and detects anomalies such as:
 * - Queue saturation
 * - High failure rates
 * - Latency spikes
 * - Throughput degradation
 */
class message_bus_health_monitor {
public:
    using health_status = message_bus_health_status;
    using health_thresholds = message_bus_health_thresholds;
    using health_report = message_bus_health_report;

    explicit message_bus_health_monitor(
        const health_thresholds& thresholds = health_thresholds{});

    /**
     * @brief Analyze message bus health
     * @param stats Current message bus statistics
     * @param bus_name Name of the message bus
     * @return Health report
     */
    health_report analyze_health(const message_bus_stats& stats,
                                  const std::string& bus_name);

    /**
     * @brief Get overall health status from multiple buses
     * @param bus_stats Map of bus names to statistics
     * @return Overall health status
     */
    health_status get_overall_health(
        const std::unordered_map<std::string, message_bus_stats>& bus_stats);

    /**
     * @brief Update health thresholds
     * @param thresholds New threshold values
     */
    void update_thresholds(const health_thresholds& thresholds);

    /**
     * @brief Get current thresholds
     * @return Current health thresholds
     */
    health_thresholds get_thresholds() const;

    /**
     * @brief Get health history
     * @param bus_name Optional bus name filter
     * @param max_count Maximum number of reports
     * @return Vector of health reports
     */
    std::vector<health_report> get_health_history(
        const std::optional<std::string>& bus_name = std::nullopt,
        size_t max_count = 100) const;

    /**
     * @brief Clear health history
     */
    void clear_history();

private:
    mutable std::mutex thresholds_mutex_;
    health_thresholds thresholds_;

    mutable std::mutex history_mutex_;
    std::vector<health_report> health_history_;
    const size_t max_history_size_{1000};

    // Baseline tracking for throughput comparison
    mutable std::mutex baseline_mutex_;
    std::unordered_map<std::string, double> baseline_throughput_;

    health_status calculate_status(const std::vector<std::string>& issues) const;
    void check_queue_saturation(health_report& report, const message_bus_stats& stats);
    void check_failure_rate(health_report& report, const message_bus_stats& stats);
    void check_latency(health_report& report, const message_bus_stats& stats);
    void check_throughput(health_report& report, const message_bus_stats& stats);
};

}  // namespace kcenon::messaging::collectors
