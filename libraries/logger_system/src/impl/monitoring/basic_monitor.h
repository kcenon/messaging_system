#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file basic_monitor.h
 * @brief Basic monitoring implementation with no external dependencies
 * 
 * This provides a lightweight monitoring implementation that tracks
 * essential metrics without requiring any external monitoring framework.
 */

#include "monitoring_interface.h"
#include <mutex>
#include <memory>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <cstdint>

namespace kcenon::logger {

/**
 * @brief Basic monitoring implementation
 * 
 * This implementation provides essential monitoring capabilities
 * with minimal overhead and no external dependencies.
 */
class basic_monitor : public monitoring_interface {
private:
    mutable std::mutex mutex_;
    std::atomic<bool> enabled_{true};
    
    // Core metrics
    std::atomic<uint64_t> messages_logged_{0};
    std::atomic<uint64_t> messages_dropped_{0};
    std::atomic<uint64_t> errors_encountered_{0};
    std::atomic<uint64_t> writers_failed_{0};
    
    // Performance metrics
    std::atomic<uint64_t> total_processing_time_us_{0};
    std::atomic<uint64_t> max_processing_time_us_{0};
    std::atomic<uint64_t> min_processing_time_us_{UINT64_MAX};
    
    // Resource metrics
    std::atomic<size_t> buffer_usage_bytes_{0};
    std::atomic<size_t> max_buffer_usage_bytes_{0};
    std::atomic<size_t> queue_size_{0};
    std::atomic<size_t> max_queue_size_{0};
    
    // Health check state
    mutable std::vector<std::string> health_issues_;
    
    // Additional counters and gauges
    mutable std::unordered_map<std::string, std::atomic<double>> counters_;
    mutable std::unordered_map<std::string, std::atomic<double>> gauges_;
    mutable std::unordered_map<std::string, std::vector<double>> histograms_;
    
    // Start time for uptime calculation
    std::chrono::system_clock::time_point start_time_;
    
public:
    /**
     * @brief Constructor
     */
    basic_monitor() : start_time_(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Destructor
     */
    ~basic_monitor() override = default;
    
    /**
     * @brief Collect current metrics
     * @return Result containing monitoring data
     */
    result<monitoring_data> collect_metrics() const override {
        if (!enabled_.load()) {
            return make_logger_error<monitoring_data>(error_code::metrics_not_available,
                            "Monitoring is disabled");
        }
        
        monitoring_data data;
        
        // Core metrics
        data.add_metric("messages_logged", 
                       static_cast<double>(messages_logged_.load()), 
                       metric_type::counter);
        data.add_metric("messages_dropped", 
                       static_cast<double>(messages_dropped_.load()), 
                       metric_type::counter);
        data.add_metric("errors_encountered", 
                       static_cast<double>(errors_encountered_.load()), 
                       metric_type::counter);
        data.add_metric("writers_failed", 
                       static_cast<double>(writers_failed_.load()), 
                       metric_type::counter);
        
        // Performance metrics
        uint64_t total_time = total_processing_time_us_.load();
        uint64_t messages = messages_logged_.load();
        if (messages > 0) {
            data.add_metric("avg_processing_time_us", 
                           static_cast<double>(total_time) / messages, 
                           metric_type::gauge);
        }
        data.add_metric("max_processing_time_us", 
                       static_cast<double>(max_processing_time_us_.load()), 
                       metric_type::gauge);
        
        uint64_t min_time = min_processing_time_us_.load();
        if (min_time != UINT64_MAX) {
            data.add_metric("min_processing_time_us", 
                           static_cast<double>(min_time), 
                           metric_type::gauge);
        }
        
        // Resource metrics
        data.add_metric("buffer_usage_bytes", 
                       static_cast<double>(buffer_usage_bytes_.load()), 
                       metric_type::gauge);
        data.add_metric("max_buffer_usage_bytes", 
                       static_cast<double>(max_buffer_usage_bytes_.load()), 
                       metric_type::gauge);
        data.add_metric("queue_size", 
                       static_cast<double>(queue_size_.load()), 
                       metric_type::gauge);
        data.add_metric("max_queue_size", 
                       static_cast<double>(max_queue_size_.load()), 
                       metric_type::gauge);
        
        // Uptime
        auto now = std::chrono::system_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            now - start_time_).count();
        data.add_metric("uptime_seconds", 
                       static_cast<double>(uptime), 
                       metric_type::gauge);
        
        // Add custom counters and gauges
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            for (const auto& [name, value] : counters_) {
                data.add_metric(name, value.load(), metric_type::counter);
            }
            
            for (const auto& [name, value] : gauges_) {
                data.add_metric(name, value.load(), metric_type::gauge);
            }
            
            // Calculate histogram summaries
            for (const auto& [name, values] : histograms_) {
                if (!values.empty()) {
                    double sum = 0;
                    double min_val = values[0];
                    double max_val = values[0];
                    
                    for (double v : values) {
                        sum += v;
                        min_val = std::min(min_val, v);
                        max_val = std::max(max_val, v);
                    }
                    
                    double avg = sum / values.size();
                    data.add_metric(name + "_avg", avg, metric_type::summary);
                    data.add_metric(name + "_min", min_val, metric_type::summary);
                    data.add_metric(name + "_max", max_val, metric_type::summary);
                    data.add_metric(name + "_count", 
                                   static_cast<double>(values.size()), 
                                   metric_type::summary);
                }
            }
        }
        
        return data;
    }
    
    /**
     * @brief Perform health check
     * @return Result containing health check result
     */
    result<health_check_result> check_health() const override {
        health_check_result result;
        
        // Check error rate
        uint64_t errors = errors_encountered_.load();
        uint64_t messages = messages_logged_.load();
        if (messages > 0) {
            double error_rate = static_cast<double>(errors) / messages;
            if (error_rate > 0.1) {  // More than 10% error rate
                result.add_issue("High error rate: " + 
                               std::to_string(error_rate * 100) + "%");
                result.set_status(health_status::unhealthy);
            } else if (error_rate > 0.05) {  // More than 5% error rate
                result.add_issue("Elevated error rate: " + 
                               std::to_string(error_rate * 100) + "%");
                result.set_status(health_status::degraded);
            }
        }
        
        // Check dropped messages
        uint64_t dropped = messages_dropped_.load();
        if (dropped > 0) {
            if (messages > 0) {
                double drop_rate = static_cast<double>(dropped) / messages;
                if (drop_rate > 0.01) {  // More than 1% drop rate
                    result.add_issue("Messages being dropped: " + 
                                   std::to_string(dropped) + " total");
                    if (result.get_status() == health_status::healthy) {
                        result.set_status(health_status::degraded);
                    }
                }
            }
        }
        
        // Check writer failures
        uint64_t writer_failures = writers_failed_.load();
        if (writer_failures > 0) {
            result.add_issue("Writer failures detected: " + 
                           std::to_string(writer_failures));
            if (result.get_status() == health_status::healthy) {
                result.set_status(health_status::degraded);
            }
        }
        
        // Check queue size
        size_t queue_size = queue_size_.load();
        size_t max_queue = max_queue_size_.load();
        if (max_queue > 0 && queue_size > max_queue * 0.9) {
            result.add_issue("Queue near capacity: " + 
                           std::to_string(queue_size) + "/" + 
                           std::to_string(max_queue));
            if (result.get_status() == health_status::healthy) {
                result.set_status(health_status::degraded);
            }
        }
        
        // Add any custom health issues
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& issue : health_issues_) {
                result.add_issue(issue);
            }
        }
        
        // Set overall message
        if (result.is_healthy()) {
            result.set_message("All systems operational");
        } else {
            result.set_message("Issues detected - check details");
        }
        
        return result;
    }
    
    /**
     * @brief Reset all metrics
     * @return Result indicating success
     */
    result_void reset_metrics() override {
        messages_logged_ = 0;
        messages_dropped_ = 0;
        errors_encountered_ = 0;
        writers_failed_ = 0;
        
        total_processing_time_us_ = 0;
        max_processing_time_us_ = 0;
        min_processing_time_us_ = UINT64_MAX;
        
        buffer_usage_bytes_ = 0;
        max_buffer_usage_bytes_ = 0;
        queue_size_ = 0;
        max_queue_size_ = 0;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            counters_.clear();
            gauges_.clear();
            histograms_.clear();
            health_issues_.clear();
        }
        
        start_time_ = std::chrono::system_clock::now();
        
        return {};
    }
    
    /**
     * @brief Enable or disable monitoring
     * @param enable true to enable
     * @return Result indicating success
     */
    result_void set_enabled(bool enable) override {
        enabled_ = enable;
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
     * @return "basic" for this implementation
     */
    std::string get_backend_name() const override {
        return "basic";
    }
    
    /**
     * @brief Increment a counter
     * @param name Counter name
     * @param value Increment value
     */
    void increment_counter(const std::string& name, double value = 1.0) override {
        if (!enabled_.load()) return;
        
        if (name == "messages_logged") {
            messages_logged_ += static_cast<uint64_t>(value);
        } else if (name == "messages_dropped") {
            messages_dropped_ += static_cast<uint64_t>(value);
        } else if (name == "errors_encountered") {
            errors_encountered_ += static_cast<uint64_t>(value);
        } else if (name == "writers_failed") {
            writers_failed_ += static_cast<uint64_t>(value);
        } else {
            std::lock_guard<std::mutex> lock(mutex_);
            counters_[name] += value;
        }
    }
    
    /**
     * @brief Update a gauge
     * @param name Gauge name
     * @param value New value
     */
    void update_gauge(const std::string& name, double value) override {
        if (!enabled_.load()) return;
        
        if (name == "buffer_usage_bytes") {
            buffer_usage_bytes_ = static_cast<size_t>(value);
            size_t max_val = max_buffer_usage_bytes_.load();
            while (value > max_val && 
                   !max_buffer_usage_bytes_.compare_exchange_weak(max_val, 
                                                                  static_cast<size_t>(value))) {
                // Loop until successful
            }
        } else if (name == "queue_size") {
            queue_size_ = static_cast<size_t>(value);
            size_t max_val = max_queue_size_.load();
            while (value > max_val && 
                   !max_queue_size_.compare_exchange_weak(max_val, 
                                                         static_cast<size_t>(value))) {
                // Loop until successful
            }
        } else {
            std::lock_guard<std::mutex> lock(mutex_);
            gauges_[name] = value;
        }
    }
    
    /**
     * @brief Record a histogram value
     * @param name Histogram name
     * @param value Value to record
     */
    void record_histogram(const std::string& name, double value) override {
        if (!enabled_.load()) return;
        
        if (name == "processing_time_us") {
            uint64_t us_value = static_cast<uint64_t>(value);
            total_processing_time_us_ += us_value;
            
            // Update max
            uint64_t max_val = max_processing_time_us_.load();
            while (us_value > max_val && 
                   !max_processing_time_us_.compare_exchange_weak(max_val, us_value)) {
                // Loop until successful
            }
            
            // Update min
            uint64_t min_val = min_processing_time_us_.load();
            while (us_value < min_val && 
                   !min_processing_time_us_.compare_exchange_weak(min_val, us_value)) {
                // Loop until successful
            }
        } else {
            std::lock_guard<std::mutex> lock(mutex_);
            histograms_[name].push_back(value);
            
            // Limit histogram size to prevent unbounded growth
            if (histograms_[name].size() > 10000) {
                // Keep only recent values
                histograms_[name].erase(histograms_[name].begin(),
                                       histograms_[name].begin() + 5000);
            }
        }
    }
    
    /**
     * @brief Add a health issue
     * @param issue Issue description
     */
    void add_health_issue(const std::string& issue) {
        std::lock_guard<std::mutex> lock(mutex_);
        health_issues_.push_back(issue);
    }
    
    /**
     * @brief Clear health issues
     */
    void clear_health_issues() {
        std::lock_guard<std::mutex> lock(mutex_);
        health_issues_.clear();
    }
};

} // namespace kcenon::logger