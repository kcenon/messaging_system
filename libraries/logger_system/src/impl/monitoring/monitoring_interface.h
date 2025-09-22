#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file monitoring_interface.h
 * @brief Abstract interface for monitoring and metrics collection
 * 
 * This interface defines the contract for monitoring implementations,
 * allowing different backends without creating external dependencies.
 */

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <atomic>
#include <kcenon/logger/core/error_codes.h>

namespace kcenon::logger {

/**
 * @brief Health status levels
 */
enum class health_status {
    healthy,        ///< Everything is working normally
    degraded,       ///< Some issues but still operational
    unhealthy,      ///< Major issues, may not be operational
    unknown         ///< Status cannot be determined
};

/**
 * @brief Metric types for categorization
 */
enum class metric_type {
    counter,        ///< Monotonically increasing value
    gauge,          ///< Value that can go up or down
    histogram,      ///< Distribution of values
    summary         ///< Statistical summary
};

/**
 * @brief Single metric value with metadata
 */
struct metric_value {
    std::string name;
    double value;
    metric_type type;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> labels;
    
    metric_value() : value(0.0), type(metric_type::gauge),
                    timestamp(std::chrono::system_clock::now()) {}
    
    metric_value(const std::string& n, double v, metric_type t = metric_type::gauge)
        : name(n), value(v), type(t), 
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Collection of metrics
 */
class monitoring_data {
private:
    std::vector<metric_value> metrics_;
    std::chrono::system_clock::time_point collection_time_;
    
public:
    monitoring_data() : collection_time_(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Add a metric to the collection
     * @param name Metric name
     * @param value Metric value
     * @param type Metric type
     */
    void add_metric(const std::string& name, double value, 
                   metric_type type = metric_type::gauge) {
        metrics_.emplace_back(name, value, type);
    }
    
    /**
     * @brief Add a pre-built metric
     * @param metric The metric to add
     */
    void add_metric(const metric_value& metric) {
        metrics_.push_back(metric);
    }
    
    /**
     * @brief Get all metrics
     * @return Vector of metrics
     */
    const std::vector<metric_value>& get_metrics() const {
        return metrics_;
    }
    
    /**
     * @brief Get collection timestamp
     * @return Time when metrics were collected
     */
    std::chrono::system_clock::time_point get_collection_time() const {
        return collection_time_;
    }
    
    /**
     * @brief Clear all metrics
     */
    void clear() {
        metrics_.clear();
    }
    
    /**
     * @brief Get number of metrics
     * @return Number of metrics in collection
     */
    size_t size() const {
        return metrics_.size();
    }
};

/**
 * @brief Health check result
 */
class health_check_result {
private:
    health_status status_;
    std::string message_;
    std::vector<std::string> issues_;
    std::chrono::system_clock::time_point check_time_;
    
public:
    health_check_result(health_status status = health_status::healthy)
        : status_(status), check_time_(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Set health status
     * @param status The health status
     */
    void set_status(health_status status) {
        status_ = status;
    }
    
    /**
     * @brief Get health status
     * @return Current health status
     */
    health_status get_status() const {
        return status_;
    }
    
    /**
     * @brief Add an issue
     * @param issue Description of the issue
     */
    void add_issue(const std::string& issue) {
        issues_.push_back(issue);
        // Automatically degrade status when issues are added
        if (status_ == health_status::healthy) {
            status_ = health_status::degraded;
        }
    }
    
    /**
     * @brief Get all issues
     * @return Vector of issue descriptions
     */
    const std::vector<std::string>& get_issues() const {
        return issues_;
    }
    
    /**
     * @brief Set status message
     * @param message Status message
     */
    void set_message(const std::string& message) {
        message_ = message;
    }
    
    /**
     * @brief Get status message
     * @return Status message
     */
    const std::string& get_message() const {
        return message_;
    }
    
    /**
     * @brief Check if healthy
     * @return true if status is healthy
     */
    bool is_healthy() const {
        return status_ == health_status::healthy;
    }
    
    /**
     * @brief Get check timestamp
     * @return Time when health check was performed
     */
    std::chrono::system_clock::time_point get_check_time() const {
        return check_time_;
    }
};

/**
 * @brief Abstract monitoring interface
 * 
 * This interface defines the contract for monitoring implementations,
 * allowing different backends to be plugged in.
 */
class monitoring_interface {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~monitoring_interface() = default;
    
    /**
     * @brief Collect current metrics
     * @return Result containing monitoring data or error
     */
    virtual result<monitoring_data> collect_metrics() const = 0;
    
    /**
     * @brief Perform health check
     * @return Result containing health check result or error
     */
    virtual result<health_check_result> check_health() const = 0;
    
    /**
     * @brief Reset all metrics
     * @return Result indicating success or error
     */
    virtual result_void reset_metrics() = 0;
    
    /**
     * @brief Enable or disable metric collection
     * @param enable true to enable, false to disable
     * @return Result indicating success or error
     */
    virtual result_void set_enabled(bool enable) = 0;
    
    /**
     * @brief Check if monitoring is enabled
     * @return true if monitoring is enabled
     */
    virtual bool is_enabled() const = 0;
    
    /**
     * @brief Get monitoring backend name
     * @return Name of the monitoring backend
     */
    virtual std::string get_backend_name() const = 0;
    
    /**
     * @brief Record a counter increment
     * @param name Counter name
     * @param value Increment value (default 1)
     */
    virtual void increment_counter(const std::string& name, double value = 1.0) = 0;
    
    /**
     * @brief Update a gauge value
     * @param name Gauge name
     * @param value New value
     */
    virtual void update_gauge(const std::string& name, double value) = 0;
    
    /**
     * @brief Record a value in a histogram
     * @param name Histogram name
     * @param value Value to record
     */
    virtual void record_histogram(const std::string& name, double value) = 0;
};

/**
 * @brief Convert health status to string
 * @param status The health status
 * @return String representation
 */
inline std::string health_status_to_string(health_status status) {
    switch (status) {
        case health_status::healthy:
            return "healthy";
        case health_status::degraded:
            return "degraded";
        case health_status::unhealthy:
            return "unhealthy";
        case health_status::unknown:
            return "unknown";
        default:
            return "unknown";
    }
}

/**
 * @brief Convert metric type to string
 * @param type The metric type
 * @return String representation
 */
inline std::string metric_type_to_string(metric_type type) {
    switch (type) {
        case metric_type::counter:
            return "counter";
        case metric_type::gauge:
            return "gauge";
        case metric_type::histogram:
            return "histogram";
        case metric_type::summary:
            return "summary";
        default:
            return "unknown";
    }
}

} // namespace kcenon::logger