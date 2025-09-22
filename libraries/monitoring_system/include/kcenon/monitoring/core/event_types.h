#pragma once

/**
 * @file event_types.h
 * @brief Common event type definitions for monitoring system
 *
 * This file defines standard event types used throughout the
 * monitoring system for inter-component communication.
 */

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "../interfaces/metric_types_adapter.h"
#include "../interfaces/event_bus_interface.h"

namespace monitoring_system {

/**
 * @class thread_pool_metric_event
 * @brief Event containing thread pool metrics
 */
class thread_pool_metric_event : public event_base {
public:
    struct thread_pool_stats {
        size_t active_threads{0};
        size_t idle_threads{0};
        size_t queued_tasks{0};
        size_t completed_tasks{0};
        double cpu_usage_percent{0.0};
        std::chrono::milliseconds avg_task_duration{0};
    };

    thread_pool_metric_event(const std::string& pool_name, const thread_pool_stats& stats)
        : pool_name_(pool_name), stats_(stats) {}

    std::string get_type_name() const override {
        return "thread_pool_metric_event";
    }

    const std::string& get_pool_name() const { return pool_name_; }
    const thread_pool_stats& get_stats() const { return stats_; }

private:
    std::string pool_name_;
    thread_pool_stats stats_;
};

/**
 * @class logging_metric_event
 * @brief Event containing logging system metrics
 */
class logging_metric_event : public event_base {
public:
    struct logging_stats {
        uint64_t total_logs{0};
        uint64_t error_count{0};
        uint64_t warning_count{0};
        uint64_t info_count{0};
        uint64_t debug_count{0};
        size_t buffer_usage_bytes{0};
        double logs_per_second{0.0};
    };

    logging_metric_event(const std::string& logger_name, const logging_stats& stats)
        : logger_name_(logger_name), stats_(stats) {}

    std::string get_type_name() const override {
        return "logging_metric_event";
    }

    const std::string& get_logger_name() const { return logger_name_; }
    const logging_stats& get_stats() const { return stats_; }

private:
    std::string logger_name_;
    logging_stats stats_;
};

/**
 * @class system_resource_event
 * @brief Event containing system resource metrics
 */
class system_resource_event : public event_base {
public:
    struct resource_stats {
        double cpu_usage_percent{0.0};
        uint64_t memory_used_bytes{0};
        uint64_t memory_total_bytes{0};
        uint64_t disk_used_bytes{0};
        uint64_t disk_total_bytes{0};
        uint64_t network_rx_bytes{0};
        uint64_t network_tx_bytes{0};
        size_t process_count{0};
        size_t thread_count{0};
    };

    explicit system_resource_event(const resource_stats& stats)
        : stats_(stats) {}

    std::string get_type_name() const override {
        return "system_resource_event";
    }

    const resource_stats& get_stats() const { return stats_; }

private:
    resource_stats stats_;
};

/**
 * @class performance_alert_event
 * @brief Event for performance-related alerts
 */
class performance_alert_event : public event_base {
public:
    enum class alert_severity {
        info,
        warning,
        critical
    };

    enum class alert_type {
        high_cpu_usage,
        high_memory_usage,
        slow_response_time,
        high_error_rate,
        resource_exhaustion,
        threshold_exceeded
    };

    performance_alert_event(alert_type type, alert_severity severity,
                           const std::string& component,
                           const std::string& message,
                           std::optional<double> threshold = std::nullopt,
                           std::optional<double> actual_value = std::nullopt)
        : type_(type), severity_(severity), component_(component),
          message_(message), threshold_(threshold), actual_value_(actual_value) {}

    std::string get_type_name() const override {
        return "performance_alert_event";
    }

    alert_type get_alert_type() const { return type_; }
    alert_severity get_severity() const { return severity_; }
    const std::string& get_component() const { return component_; }
    const std::string& get_message() const { return message_; }
    std::optional<double> get_threshold() const { return threshold_; }
    std::optional<double> get_actual_value() const { return actual_value_; }

private:
    alert_type type_;
    alert_severity severity_;
    std::string component_;
    std::string message_;
    std::optional<double> threshold_;
    std::optional<double> actual_value_;
};

/**
 * @class configuration_change_event
 * @brief Event fired when configuration changes
 */
class configuration_change_event : public event_base {
public:
    enum class change_type {
        added,
        modified,
        removed
    };

    configuration_change_event(const std::string& component,
                              const std::string& config_key,
                              change_type type,
                              const std::string& old_value = "",
                              const std::string& new_value = "")
        : component_(component), config_key_(config_key),
          type_(type), old_value_(old_value), new_value_(new_value) {}

    std::string get_type_name() const override {
        return "configuration_change_event";
    }

    const std::string& get_component() const { return component_; }
    const std::string& get_config_key() const { return config_key_; }
    change_type get_change_type() const { return type_; }
    const std::string& get_old_value() const { return old_value_; }
    const std::string& get_new_value() const { return new_value_; }

private:
    std::string component_;
    std::string config_key_;
    change_type type_;
    std::string old_value_;
    std::string new_value_;
};

/**
 * @class component_lifecycle_event
 * @brief Event for component lifecycle changes
 */
class component_lifecycle_event : public event_base {
public:
    enum class lifecycle_state {
        initializing,
        started,
        running,
        pausing,
        paused,
        resuming,
        stopping,
        stopped,
        error
    };

    component_lifecycle_event(const std::string& component,
                             lifecycle_state old_state,
                             lifecycle_state new_state,
                             const std::string& reason = "")
        : component_(component), old_state_(old_state),
          new_state_(new_state), reason_(reason) {}

    std::string get_type_name() const override {
        return "component_lifecycle_event";
    }

    const std::string& get_component() const { return component_; }
    lifecycle_state get_old_state() const { return old_state_; }
    lifecycle_state get_new_state() const { return new_state_; }
    const std::string& get_reason() const { return reason_; }

private:
    std::string component_;
    lifecycle_state old_state_;
    lifecycle_state new_state_;
    std::string reason_;
};

/**
 * @class metric_collection_event
 * @brief Event containing collected metrics batch
 */
class metric_collection_event : public event_base {
public:
    metric_collection_event(const std::string& collector_name,
                           std::vector<metric> metrics)
        : collector_name_(collector_name), metrics_(std::move(metrics)) {}

    std::string get_type_name() const override {
        return "metric_collection_event";
    }

    const std::string& get_collector_name() const { return collector_name_; }
    const std::vector<metric>& get_metrics() const { return metrics_; }
    size_t get_metric_count() const { return metrics_.size(); }

private:
    std::string collector_name_;
    std::vector<metric> metrics_;
};

/**
 * @class health_check_event
 * @brief Event for health check results
 */
class health_check_event : public event_base {
public:
    enum class health_status {
        healthy,
        degraded,
        unhealthy,
        unknown
    };

    struct health_check_result {
        std::string check_name;
        health_status status;
        std::string message;
        std::chrono::milliseconds response_time;
        std::optional<std::unordered_map<std::string, std::string>> metadata;
    };

    health_check_event(const std::string& component,
                      std::vector<health_check_result> results)
        : component_(component), results_(std::move(results)) {}

    std::string get_type_name() const override {
        return "health_check_event";
    }

    const std::string& get_component() const { return component_; }
    const std::vector<health_check_result>& get_results() const { return results_; }

    health_status get_overall_status() const {
        health_status overall = health_status::healthy;
        for (const auto& result : results_) {
            if (result.status == health_status::unhealthy) {
                return health_status::unhealthy;
            }
            if (result.status == health_status::degraded) {
                overall = health_status::degraded;
            }
            if (result.status == health_status::unknown && overall == health_status::healthy) {
                overall = health_status::unknown;
            }
        }
        return overall;
    }

private:
    std::string component_;
    std::vector<health_check_result> results_;
};

} // namespace monitoring_system