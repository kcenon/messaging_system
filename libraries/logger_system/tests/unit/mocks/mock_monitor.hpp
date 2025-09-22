/**
 * @file mock_monitor.hpp
 * @brief Mock monitoring implementation for testing
 * @date 2025-09-09
 *
 * BSD 3-Clause License
 * Copyright (c) 2025, kcenon
 * All rights reserved.
 */

#pragma once

#include "../../sources/logger/monitoring/monitoring_interface.h"
#include "../../sources/logger/error_codes.h"
#include <atomic>
#include <map>
#include <mutex>

namespace logger_system::testing {

using namespace logger_module;

/**
 * @brief Mock monitor for unit testing
 * 
 * Provides controllable monitoring behavior for testing
 * monitoring integration and health check scenarios.
 */
class mock_monitor : public monitoring_interface {
private:
    mutable std::mutex mutex_;
    std::map<std::string, double> metrics_;
    std::atomic<health_status> health_{health_status::healthy};
    mutable std::atomic<size_t> metric_query_count_{0};
    mutable std::atomic<size_t> health_check_count_{0};
    std::atomic<bool> should_fail_{false};

public:
    mock_monitor() {
        // Initialize default metrics
        metrics_["messages_logged"] = 0.0;
        metrics_["messages_dropped"] = 0.0;
        metrics_["buffer_usage"] = 0.0;
        metrics_["write_latency_ms"] = 0.0;
    }

    ~mock_monitor() override = default;

    // monitoring_interface implementation
    result<monitoring_data> get_metrics() const {
        metric_query_count_.fetch_add(1, std::memory_order_relaxed);

        if (should_fail_.load()) {
            return make_logger_error<monitoring_data>(logger_error_code::operation_failed);
        }

        monitoring_data data;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [key, value] : metrics_) {
                data.add_metric(key, value);
            }
        }
        
        return data;
    }

    result<health_status> get_health_status() const {
        health_check_count_.fetch_add(1, std::memory_order_relaxed);

        if (should_fail_.load()) {
            return make_logger_error<health_status>(logger_error_code::operation_failed);
        }

        return health_.load();
    }

    result_void record_event(const std::string& event_name, 
                            const std::string& details) {
        if (should_fail_.load()) {
            return make_logger_error(logger_error_code::operation_failed);
        }

        // Simply increment a counter for the event
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = metrics_.find(event_name);
        if (it != metrics_.end()) {
            it->second += 1.0;
        } else {
            metrics_[event_name] = 1.0;
        }

        return {};
    }

    // Mock control methods
    void set_metric(const std::string& name, double value) {
        std::lock_guard<std::mutex> lock(mutex_);
        metrics_[name] = value;
    }

    void set_health_status(health_status status) {
        health_.store(status);
    }

    void set_should_fail(bool fail) {
        should_fail_.store(fail);
    }

    void increment_metric(const std::string& name, double delta = 1.0) {
        std::lock_guard<std::mutex> lock(mutex_);
        metrics_[name] += delta;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        metrics_.clear();
        metrics_["messages_logged"] = 0.0;
        metrics_["messages_dropped"] = 0.0;
        metrics_["buffer_usage"] = 0.0;
        metrics_["write_latency_ms"] = 0.0;
        health_.store(health_status::healthy);
        metric_query_count_.store(0);
        health_check_count_.store(0);
        should_fail_.store(false);
    }

    // Inspection methods
    size_t get_metric_query_count() const {
        return metric_query_count_.load();
    }

    size_t get_health_check_count() const {
        return health_check_count_.load();
    }

    double get_metric_value(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = metrics_.find(name);
        return it != metrics_.end() ? it->second : 0.0;
    }

    std::map<std::string, double> get_all_metrics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return metrics_;
    }
};

/**
 * @brief Mock health reporter for testing health check system
 */
class mock_health_reporter {
private:
    std::shared_ptr<mock_monitor> monitor_;
    std::atomic<bool> auto_degrade_{false};
    std::atomic<size_t> error_threshold_{10};
    std::atomic<size_t> current_errors_{0};

public:
    explicit mock_health_reporter(std::shared_ptr<mock_monitor> monitor)
        : monitor_(monitor) {}

    void report_error() {
        current_errors_.fetch_add(1);
        
        if (auto_degrade_.load() && 
            current_errors_.load() >= error_threshold_.load()) {
            monitor_->set_health_status(health_status::degraded);
        }

        monitor_->increment_metric("errors", 1.0);
    }

    void report_success() {
        current_errors_.store(0);
        monitor_->increment_metric("successes", 1.0);
        
        if (auto_degrade_.load()) {
            monitor_->set_health_status(health_status::healthy);
        }
    }

    void enable_auto_degrade(size_t threshold) {
        auto_degrade_.store(true);
        error_threshold_.store(threshold);
    }

    void disable_auto_degrade() {
        auto_degrade_.store(false);
    }

    void reset() {
        current_errors_.store(0);
        monitor_->set_health_status(health_status::healthy);
    }
};

} // namespace logger_system::testing