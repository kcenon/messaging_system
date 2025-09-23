#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file thread_system_monitor_adapter.h
 * @brief Adapter for thread_system's monitoring capabilities
 * 
 * This adapter allows integration with thread_system's monitoring
 * infrastructure when available, providing advanced monitoring features.
 */

#include "monitoring_interface.h"
#include "basic_monitor.h"

#ifdef USE_THREAD_SYSTEM
#include <kcenon/thread/interfaces/monitorable_interface.h>

namespace kcenon::logger {

/**
 * @brief Adapter to use thread_system's monitoring capabilities
 * 
 * This adapter wraps thread_system's monitorable_interface to provide
 * compatibility with our monitoring interface, enabling advanced
 * monitoring features when thread_system is available.
 */
class thread_system_monitor_adapter : public monitoring_interface {
private:
    kcenon::thread::monitorable_interface* monitorable_;
    std::unique_ptr<basic_monitor> fallback_monitor_;
    bool owns_monitorable_;
    std::atomic<bool> enabled_{true};
    
public:
    /**
     * @brief Constructor with external monitorable
     * @param monitorable Pointer to existing monitorable interface
     */
    explicit thread_system_monitor_adapter(
        kcenon::thread::monitorable_interface* monitorable)
        : monitorable_(monitorable), 
          fallback_monitor_(std::make_unique<basic_monitor>()),
          owns_monitorable_(false) {
        if (!monitorable_) {
            throw std::invalid_argument("Monitorable cannot be null");
        }
    }
    
    /**
     * @brief Default constructor with fallback to basic monitor
     */
    thread_system_monitor_adapter()
        : monitorable_(nullptr),
          fallback_monitor_(std::make_unique<basic_monitor>()),
          owns_monitorable_(false) {}
    
    /**
     * @brief Destructor
     */
    ~thread_system_monitor_adapter() override {
        if (owns_monitorable_ && monitorable_) {
            delete monitorable_;
        }
    }
    
    /**
     * @brief Collect current metrics
     * @return Result containing monitoring data
     */
    result<monitoring_data> collect_metrics() const override {
        if (!enabled_.load()) {
            return error_code::metrics_not_available;
        }
        
        // Try thread_system first
        if (monitorable_) {
            try {
                auto ts_metrics = monitorable_->get_metrics();
                if (ts_metrics) {
                    // Convert thread_system metrics to our format
                    monitoring_data data;
                    
                    // Extract metrics from thread_system monitoring_data
                    // Note: This assumes thread_system has similar metric structure
                    for (const auto& [key, value] : ts_metrics.value().get_values()) {
                        data.add_metric(key, value);
                    }
                    
                    // Also add fallback monitor metrics for logger-specific data
                    if (fallback_monitor_) {
                        auto fallback_result = fallback_monitor_->collect_metrics();
                        if (fallback_result) {
                            for (const auto& metric : fallback_result.value().get_metrics()) {
                                data.add_metric(metric);
                            }
                        }
                    }
                    
                    return data;
                }
            } catch (...) {
                // Fall through to fallback
            }
        }
        
        // Fallback to basic monitor
        if (fallback_monitor_) {
            return fallback_monitor_->collect_metrics();
        }
        
        return error_code::metrics_not_available;
    }
    
    /**
     * @brief Perform health check
     * @return Result containing health check result
     */
    result<health_check_result> check_health() const override {
        health_check_result result;
        
        // Check thread_system health if available
        if (monitorable_) {
            try {
                auto ts_health = monitorable_->get_health_status();
                if (ts_health) {
                    // Convert thread_system health to our format
                    auto status = ts_health.value();
                    
                    if (!status.is_healthy()) {
                        result.set_status(health_status::degraded);
                        result.add_issue("Thread system reports unhealthy status");
                        
                        for (const auto& issue : status.get_issues()) {
                            result.add_issue("Thread system: " + issue);
                        }
                    }
                }
            } catch (...) {
                result.add_issue("Failed to check thread_system health");
            }
        }
        
        // Also check fallback monitor for logger-specific health
        if (fallback_monitor_) {
            auto fallback_health = fallback_monitor_->check_health();
            if (fallback_health) {
                auto& fb_result = fallback_health.value();
                
                // Merge health status
                if (!fb_result.is_healthy()) {
                    if (result.get_status() == health_status::healthy) {
                        result.set_status(fb_result.get_status());
                    } else if (fb_result.get_status() == health_status::unhealthy) {
                        result.set_status(health_status::unhealthy);
                    }
                    
                    // Add logger-specific issues
                    for (const auto& issue : fb_result.get_issues()) {
                        result.add_issue(issue);
                    }
                }
            }
        }
        
        // Set overall message
        if (result.is_healthy()) {
            result.set_message("All systems operational");
        } else {
            result.set_message("Issues detected in monitoring");
        }
        
        return result;
    }
    
    /**
     * @brief Reset all metrics
     * @return Result indicating success
     */
    result_void reset_metrics() override {
        result_void result;
        
        // Reset thread_system metrics if available
        if (monitorable_) {
            try {
                auto ts_result = monitorable_->reset_metrics();
                if (!ts_result) {
                    result = error_code::operation_failed;
                }
            } catch (...) {
                result = error_code::operation_failed;
            }
        }
        
        // Reset fallback monitor
        if (fallback_monitor_) {
            auto fb_result = fallback_monitor_->reset_metrics();
            if (!fb_result && result) {
                result = fb_result;
            }
        }
        
        return result;
    }
    
    /**
     * @brief Enable or disable monitoring
     * @param enable true to enable
     * @return Result indicating success
     */
    result_void set_enabled(bool enable) override {
        enabled_ = enable;
        
        if (fallback_monitor_) {
            fallback_monitor_->set_enabled(enable);
        }
        
        return {};
    }
    
    /**
     * @brief Check if monitoring is enabled
     * @return true if enabled
     */
    bool is_enabled() const override {
        return enabled_.load();
    }
    
    /**
     * @brief Get backend name
     * @return "thread_system" or "thread_system+basic"
     */
    std::string get_backend_name() const override {
        if (monitorable_) {
            return "thread_system+basic";
        }
        return "basic(via adapter)";
    }
    
    /**
     * @brief Increment a counter
     * @param name Counter name
     * @param value Increment value
     */
    void increment_counter(const std::string& name, double value = 1.0) override {
        if (!enabled_.load()) return;
        
        // Always use fallback for logger-specific counters
        if (fallback_monitor_) {
            fallback_monitor_->increment_counter(name, value);
        }
        
        // Also update thread_system if it supports this
        if (monitorable_) {
            try {
                monitorable_->update_metric(name, value);
            } catch (...) {
                // Ignore failures
            }
        }
    }
    
    /**
     * @brief Update a gauge
     * @param name Gauge name
     * @param value New value
     */
    void update_gauge(const std::string& name, double value) override {
        if (!enabled_.load()) return;
        
        // Always use fallback for logger-specific gauges
        if (fallback_monitor_) {
            fallback_monitor_->update_gauge(name, value);
        }
        
        // Also update thread_system if it supports this
        if (monitorable_) {
            try {
                monitorable_->update_metric(name, value);
            } catch (...) {
                // Ignore failures
            }
        }
    }
    
    /**
     * @brief Record a histogram value
     * @param name Histogram name
     * @param value Value to record
     */
    void record_histogram(const std::string& name, double value) override {
        if (!enabled_.load()) return;
        
        // Always use fallback for histograms
        if (fallback_monitor_) {
            fallback_monitor_->record_histogram(name, value);
        }
    }
    
    /**
     * @brief Set the thread_system monitorable
     * @param monitorable Pointer to monitorable interface
     * @param take_ownership Whether to take ownership
     */
    void set_monitorable(kcenon::thread::monitorable_interface* monitorable,
                        bool take_ownership = false) {
        if (owns_monitorable_ && monitorable_) {
            delete monitorable_;
        }
        
        monitorable_ = monitorable;
        owns_monitorable_ = take_ownership;
    }
    
    /**
     * @brief Get the fallback monitor for direct access
     * @return Pointer to fallback monitor
     */
    basic_monitor* get_fallback_monitor() {
        return fallback_monitor_.get();
    }
};

} // namespace kcenon::logger

#else

namespace kcenon::logger {

/**
 * @brief Fallback when thread_system is not available
 * 
 * When thread_system is not available, this adapter simply
 * wraps the basic monitor to maintain API compatibility.
 */
class thread_system_monitor_adapter : public basic_monitor {
public:
    thread_system_monitor_adapter() = default;
    ~thread_system_monitor_adapter() override = default;
    
    std::string get_backend_name() const override {
        return "basic(no thread_system)";
    }
};

} // namespace kcenon::logger

#endif // USE_THREAD_SYSTEM