#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file aggregation_processor.h
 * @brief High-level aggregation processor for metric streams
 * 
 * This file provides a comprehensive aggregation processor that integrates
 * stream aggregation with metric storage for real-time analytics.
 */

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <kcenon/monitoring/utils/stream_aggregator.h>
#include <kcenon/monitoring/utils/metric_storage.h>
#include <kcenon/monitoring/utils/metric_types.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace monitoring_system {

/**
 * @struct aggregation_rule
 * @brief Rule for metric aggregation
 */
struct aggregation_rule {
    std::string source_metric;                           // Source metric name
    std::string target_metric_prefix;                   // Target metric prefix
    std::chrono::milliseconds aggregation_interval{60000}; // Aggregation interval
    std::vector<double> percentiles{0.5, 0.95, 0.99};  // Percentiles to compute
    bool compute_rate = false;                          // Compute rate metrics
    bool detect_outliers = true;                        // Enable outlier detection
    
    /**
     * @brief Validate rule
     */
    result_void validate() const {
        if (source_metric.empty()) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Source metric name cannot be empty");
        }
        
        if (target_metric_prefix.empty()) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Target metric prefix cannot be empty");
        }
        
        if (aggregation_interval.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Aggregation interval must be positive");
        }
        
        for (double p : percentiles) {
            if (p < 0.0 || p > 1.0) {
                return result_void(monitoring_error_code::invalid_configuration,
                                 "Percentiles must be between 0 and 1");
            }
        }
        
        return result_void::success();
    }
};

/**
 * @struct metric_aggregation_result
 * @brief Result of metric aggregation
 */
struct metric_aggregation_result {
    std::string source_metric;
    stream_statistics statistics;
    std::chrono::system_clock::time_point aggregation_time;
    std::chrono::milliseconds processing_duration{0};
    size_t samples_processed = 0;
    
    metric_aggregation_result() : aggregation_time(std::chrono::system_clock::now()) {}
};

/**
 * @class aggregation_processor
 * @brief High-level processor for metric aggregation
 */
class aggregation_processor {
private:
    struct metric_aggregator_state {
        std::unique_ptr<stream_aggregator> aggregator;
        aggregation_rule rule;
        std::chrono::system_clock::time_point last_aggregation;
        uint64_t total_samples_processed = 0;
        
        metric_aggregator_state(const aggregation_rule& r) : rule(r) {
            stream_aggregator_config config;
            config.window_duration = r.aggregation_interval;
            config.enable_outlier_detection = r.detect_outliers;
            
            aggregator = std::make_unique<stream_aggregator>(config);
            last_aggregation = std::chrono::system_clock::now();
        }
    };
    
    std::unordered_map<std::string, std::unique_ptr<metric_aggregator_state>> aggregators_;
    std::shared_ptr<metric_storage> storage_;
    mutable std::mutex mutex_;
    
    // Background processing
    std::atomic<bool> running_{false};
    std::thread background_thread_;
    std::condition_variable background_cv_;
    mutable std::mutex background_mutex_;
    std::chrono::milliseconds processing_interval_{10000}; // 10 seconds
    
    // Callbacks
    std::function<void(const metric_aggregation_result&)> aggregation_callback_;
    
    /**
     * @brief Process aggregation for a metric
     */
    result<metric_aggregation_result> process_metric_aggregation(const std::string& metric_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = aggregators_.find(metric_name);
        if (it == aggregators_.end()) {
            return make_error<metric_aggregation_result>(monitoring_error_code::collector_not_found,
                                                "Aggregator not found for metric: " + metric_name);
        }
        
        auto& state = it->second;
        auto now = std::chrono::system_clock::now();
        
        // Check if it's time to aggregate
        if (now - state->last_aggregation < state->rule.aggregation_interval) {
            return make_error<metric_aggregation_result>(monitoring_error_code::collection_failed,
                                                "Aggregation interval not reached");
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Get statistics
        auto stats = state->aggregator->get_statistics();
        
        // Create result
        metric_aggregation_result result;
        result.source_metric = metric_name;
        result.statistics = stats;
        result.aggregation_time = now;
        result.samples_processed = stats.count - state->total_samples_processed;
        
        auto end_time = std::chrono::high_resolution_clock::now();
        result.processing_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        
        // Store aggregated metrics
        store_aggregated_metrics(state->rule, stats, now);
        
        // Update state
        state->last_aggregation = now;
        state->total_samples_processed = stats.count;
        
        // Reset aggregator for next interval
        state->aggregator->reset();
        
        return make_success(std::move(result));
    }
    
    /**
     * @brief Store aggregated metrics to storage
     */
    void store_aggregated_metrics(const aggregation_rule& rule,
                                 const stream_statistics& stats,
                                 std::chrono::system_clock::time_point timestamp) {
        if (!storage_) return;
        
        std::string prefix = rule.target_metric_prefix;
        
        // Store basic statistics
        storage_->store_metric(prefix + ".count", static_cast<double>(stats.count), 
                             metric_type::counter, timestamp);
        storage_->store_metric(prefix + ".mean", stats.mean, 
                             metric_type::gauge, timestamp);
        storage_->store_metric(prefix + ".min", stats.min_value, 
                             metric_type::gauge, timestamp);
        storage_->store_metric(prefix + ".max", stats.max_value, 
                             metric_type::gauge, timestamp);
        storage_->store_metric(prefix + ".std_dev", stats.std_deviation, 
                             metric_type::gauge, timestamp);
        storage_->store_metric(prefix + ".variance", stats.variance, 
                             metric_type::gauge, timestamp);
        
        // Store percentiles
        for (double p : rule.percentiles) {
            auto it = stats.percentiles.find(p);
            if (it != stats.percentiles.end()) {
                std::string percentile_name = prefix + ".p" + std::to_string(static_cast<int>(p * 100));
                storage_->store_metric(percentile_name, it->second, 
                                     metric_type::gauge, timestamp);
            }
        }
        
        // Store rate metrics if enabled
        if (rule.compute_rate) {
            storage_->store_metric(prefix + ".rate_per_second", stats.rate_per_second, 
                                 metric_type::gauge, timestamp);
            storage_->store_metric(prefix + ".rate_per_minute", stats.rate_per_minute, 
                                 metric_type::gauge, timestamp);
        }
        
        // Store advanced statistics
        storage_->store_metric(prefix + ".skewness", stats.skewness, 
                             metric_type::gauge, timestamp);
        storage_->store_metric(prefix + ".kurtosis", stats.kurtosis, 
                             metric_type::gauge, timestamp);
        storage_->store_metric(prefix + ".coefficient_of_variation", 
                             stats.coefficient_of_variation(), 
                             metric_type::gauge, timestamp);
        
        // Store outlier information
        if (rule.detect_outliers) {
            storage_->store_metric(prefix + ".outlier_count", 
                                 static_cast<double>(stats.outlier_count), 
                                 metric_type::counter, timestamp);
        }
    }
    
    /**
     * @brief Background processing loop
     */
    void background_processing_loop() {
        while (running_.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(background_mutex_);
            
            // Wait for processing interval or shutdown
            if (background_cv_.wait_for(lock, processing_interval_, 
                                      [this] { return !running_.load(std::memory_order_acquire); })) {
                break;  // Shutdown requested
            }
            
            lock.unlock();
            
            // Process all metrics
            std::vector<std::string> metric_names;
            {
                std::lock_guard<std::mutex> agg_lock(mutex_);
                for (const auto& [name, state] : aggregators_) {
                    metric_names.push_back(name);
                }
            }
            
            for (const auto& metric_name : metric_names) {
                auto result = process_metric_aggregation(metric_name);
                if (result && aggregation_callback_) {
                    aggregation_callback_(result.value());
                }
            }
        }
    }
    
public:
    /**
     * @brief Constructor
     */
    explicit aggregation_processor(std::shared_ptr<metric_storage> storage = nullptr)
        : storage_(storage) {}
    
    /**
     * @brief Destructor
     */
    ~aggregation_processor() {
        stop_background_processing();
    }
    
    // Non-copyable and non-moveable
    aggregation_processor(const aggregation_processor&) = delete;
    aggregation_processor& operator=(const aggregation_processor&) = delete;
    aggregation_processor(aggregation_processor&&) = delete;
    aggregation_processor& operator=(aggregation_processor&&) = delete;
    
    /**
     * @brief Add aggregation rule
     */
    result_void add_aggregation_rule(const aggregation_rule& rule) {
        auto validation = rule.validate();
        if (!validation) {
            return validation;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (aggregators_.find(rule.source_metric) != aggregators_.end()) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Aggregation rule already exists for metric: " + rule.source_metric);
        }
        
        aggregators_[rule.source_metric] = std::make_unique<metric_aggregator_state>(rule);
        
        return result_void::success();
    }
    
    /**
     * @brief Remove aggregation rule
     */
    result_void remove_aggregation_rule(const std::string& source_metric) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = aggregators_.find(source_metric);
        if (it == aggregators_.end()) {
            return result_void(monitoring_error_code::collector_not_found,
                             "Aggregation rule not found for metric: " + source_metric);
        }
        
        aggregators_.erase(it);
        return result_void::success();
    }
    
    /**
     * @brief Process metric observation
     */
    result_void process_observation(const std::string& metric_name, 
                                   double value,
                                   std::chrono::system_clock::time_point timestamp = 
                                   std::chrono::system_clock::now()) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = aggregators_.find(metric_name);
        if (it == aggregators_.end()) {
            return result_void(monitoring_error_code::collector_not_found,
                             "No aggregation rule found for metric: " + metric_name);
        }
        
        return it->second->aggregator->add_observation(value, timestamp);
    }
    
    /**
     * @brief Get current statistics for a metric
     */
    result<stream_statistics> get_current_statistics(const std::string& metric_name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = aggregators_.find(metric_name);
        if (it == aggregators_.end()) {
            return make_error<stream_statistics>(monitoring_error_code::collector_not_found,
                                               "Aggregation rule not found for metric: " + metric_name);
        }
        
        return make_success(it->second->aggregator->get_statistics());
    }
    
    /**
     * @brief Force aggregation for a specific metric
     */
    result<metric_aggregation_result> force_aggregation(const std::string& metric_name) {
        return process_metric_aggregation(metric_name);
    }
    
    /**
     * @brief Force aggregation for all metrics
     */
    std::vector<metric_aggregation_result> force_all_aggregations() {
        std::vector<metric_aggregation_result> results;
        std::vector<std::string> metric_names;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [name, state] : aggregators_) {
                metric_names.push_back(name);
            }
        }
        
        for (const auto& metric_name : metric_names) {
            auto result = process_metric_aggregation(metric_name);
            if (result) {
                results.push_back(result.value());
            }
        }
        
        return results;
    }
    
    /**
     * @brief Start background processing
     */
    result_void start_background_processing(std::chrono::milliseconds interval = std::chrono::milliseconds(10000)) {
        if (running_.load(std::memory_order_acquire)) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Background processing already running");
        }
        
        processing_interval_ = interval;
        running_.store(true, std::memory_order_release);
        background_thread_ = std::thread(&aggregation_processor::background_processing_loop, this);
        
        return result_void::success();
    }
    
    /**
     * @brief Stop background processing
     */
    void stop_background_processing() {
        if (running_.load(std::memory_order_acquire)) {
            running_.store(false, std::memory_order_release);
            background_cv_.notify_all();
            
            if (background_thread_.joinable()) {
                background_thread_.join();
            }
        }
    }
    
    /**
     * @brief Set aggregation callback
     */
    void set_aggregation_callback(std::function<void(const metric_aggregation_result&)> callback) {
        aggregation_callback_ = std::move(callback);
    }
    
    /**
     * @brief Set metric storage
     */
    void set_storage(std::shared_ptr<metric_storage> storage) {
        storage_ = storage;
    }
    
    /**
     * @brief Get list of configured metrics
     */
    std::vector<std::string> get_configured_metrics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> metrics;
        
        for (const auto& [name, state] : aggregators_) {
            metrics.push_back(name);
        }
        
        return metrics;
    }
    
    /**
     * @brief Get aggregation rule for a metric
     */
    result<aggregation_rule> get_aggregation_rule(const std::string& metric_name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = aggregators_.find(metric_name);
        if (it == aggregators_.end()) {
            return make_error<aggregation_rule>(monitoring_error_code::collector_not_found,
                                              "Aggregation rule not found for metric: " + metric_name);
        }
        
        return make_success(it->second->rule);
    }
    
    /**
     * @brief Clear all aggregation rules
     */
    void clear_all_rules() {
        std::lock_guard<std::mutex> lock(mutex_);
        aggregators_.clear();
    }
    
    /**
     * @brief Get processing statistics
     */
    std::unordered_map<std::string, uint64_t> get_processing_statistics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::unordered_map<std::string, uint64_t> stats;
        
        for (const auto& [name, state] : aggregators_) {
            stats[name + ".total_samples"] = state->total_samples_processed;
            stats[name + ".current_samples"] = state->aggregator->count();
        }
        
        return stats;
    }
};

/**
 * @brief Helper function to create aggregation processor
 */
inline std::unique_ptr<aggregation_processor> make_aggregation_processor(
    std::shared_ptr<metric_storage> storage = nullptr) {
    return std::make_unique<aggregation_processor>(storage);
}

/**
 * @brief Create standard aggregation rules for common metrics
 */
inline std::vector<aggregation_rule> create_standard_aggregation_rules() {
    std::vector<aggregation_rule> rules;
    
    // Response time aggregation
    aggregation_rule response_time_rule;
    response_time_rule.source_metric = "response_time";
    response_time_rule.target_metric_prefix = "response_time_stats";
    response_time_rule.percentiles = {0.5, 0.9, 0.95, 0.99};
    response_time_rule.compute_rate = false;
    response_time_rule.detect_outliers = true;
    rules.push_back(response_time_rule);
    
    // Request count aggregation
    aggregation_rule request_count_rule;
    request_count_rule.source_metric = "request_count";
    request_count_rule.target_metric_prefix = "request_rate";
    request_count_rule.percentiles = {0.5, 0.95};
    request_count_rule.compute_rate = true;
    request_count_rule.detect_outliers = false;
    rules.push_back(request_count_rule);
    
    // Error rate aggregation
    aggregation_rule error_rate_rule;
    error_rate_rule.source_metric = "error_count";
    error_rate_rule.target_metric_prefix = "error_rate";
    error_rate_rule.percentiles = {0.9, 0.99};
    error_rate_rule.compute_rate = true;
    error_rate_rule.detect_outliers = true;
    rules.push_back(error_rate_rule);
    
    return rules;
}

} // namespace monitoring_system