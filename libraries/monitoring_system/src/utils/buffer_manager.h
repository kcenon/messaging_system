#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file buffer_manager.h
 * @brief Comprehensive buffer management system
 * 
 * This file provides a high-level buffer manager that coordinates
 * different buffering strategies and integrates with the metric storage system.
 */

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <kcenon/monitoring/utils/buffering_strategy.h>
#include <kcenon/monitoring/utils/metric_storage.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace monitoring_system {

/**
 * @struct buffer_manager_config
 * @brief Configuration for buffer manager
 */
struct buffer_manager_config {
    std::chrono::milliseconds background_check_interval{100};
    size_t max_concurrent_flushes = 4;
    bool enable_automatic_flushing = true;
    bool enable_statistics_collection = true;
    
    // Default buffering strategy for new metrics
    buffering_config default_buffering_config;
    
    /**
     * @brief Validate configuration
     */
    result_void validate() const {
        if (background_check_interval.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Background check interval must be positive");
        }
        
        if (max_concurrent_flushes == 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Max concurrent flushes must be positive");
        }
        
        return default_buffering_config.validate();
    }
};

/**
 * @struct buffer_manager_statistics
 * @brief Statistics for buffer manager
 */
struct buffer_manager_statistics {
    std::atomic<size_t> total_buffers{0};
    std::atomic<size_t> active_flushes{0};
    std::atomic<size_t> total_flushes{0};
    std::atomic<size_t> failed_flushes{0};
    std::atomic<size_t> total_metrics_processed{0};
    std::atomic<size_t> background_cycles{0};
    
    std::chrono::system_clock::time_point creation_time;
    
    buffer_manager_statistics() : creation_time(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Get flush success rate
     */
    double get_flush_success_rate() const {
        auto total = total_flushes.load();
        auto failed = failed_flushes.load();
        return total > 0 ? (1.0 - static_cast<double>(failed) / total) * 100.0 : 100.0;
    }
    
    /**
     * @brief Get average metrics per flush
     */
    double get_avg_metrics_per_flush() const {
        auto flushes = total_flushes.load();
        auto processed = total_metrics_processed.load();
        return flushes > 0 ? static_cast<double>(processed) / flushes : 0.0;
    }
};

/**
 * @class buffer_manager
 * @brief High-level buffer management system
 */
class buffer_manager {
private:
    struct metric_buffer_entry {
        std::unique_ptr<buffer_strategy_interface> strategy;
        std::string metric_name;
        std::chrono::system_clock::time_point last_flush_time;
        
        metric_buffer_entry(std::unique_ptr<buffer_strategy_interface> s, const std::string& name)
            : strategy(std::move(s)), metric_name(name) {
            last_flush_time = std::chrono::system_clock::now();
        }
    };
    
    buffer_manager_config config_;
    std::unordered_map<std::string, std::unique_ptr<metric_buffer_entry>> buffers_;
    std::shared_ptr<metric_storage> storage_;
    buffer_manager_statistics stats_;
    
    mutable std::mutex buffers_mutex_;
    
    // Background processing
    std::atomic<bool> running_{false};
    std::thread background_thread_;
    std::condition_variable background_cv_;
    mutable std::mutex background_mutex_;
    
    // Flush callbacks
    std::function<void(const std::string&, const std::vector<buffered_metric>&)> flush_callback_;
    
    /**
     * @brief Background processing loop
     */
    void background_processing_loop() {
        while (running_.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(background_mutex_);
            
            // Wait for check interval or shutdown
            if (background_cv_.wait_for(lock, config_.background_check_interval,
                                      [this] { return !running_.load(std::memory_order_acquire); })) {
                break;  // Shutdown requested
            }
            
            lock.unlock();
            
            // Check all buffers for flushing
            check_and_flush_buffers();
            
            stats_.background_cycles.fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    /**
     * @brief Check all buffers and flush if needed
     */
    void check_and_flush_buffers() {
        std::vector<std::string> buffers_to_flush;
        
        {
            std::lock_guard<std::mutex> lock(buffers_mutex_);
            
            for (const auto& [name, entry] : buffers_) {
                if (entry->strategy->should_flush()) {
                    buffers_to_flush.push_back(name);
                }
            }
        }
        
        // Flush buffers (limit concurrent flushes)
        size_t concurrent_flushes = 0;
        for (const auto& buffer_name : buffers_to_flush) {
            if (concurrent_flushes >= config_.max_concurrent_flushes) {
                break;
            }
            
            auto result = flush_buffer(buffer_name);
            if (result) {
                ++concurrent_flushes;
                stats_.active_flushes.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }
    
    /**
     * @brief Flush a specific buffer
     */
    result_void flush_buffer(const std::string& metric_name) {
        std::unique_ptr<metric_buffer_entry> entry;
        
        {
            std::lock_guard<std::mutex> lock(buffers_mutex_);
            auto it = buffers_.find(metric_name);
            if (it == buffers_.end()) {
                return result_void(monitoring_error_code::collector_not_found,
                                 "Buffer not found: " + metric_name);
            }
            
            // We can't move the entry here as we need to keep it in the map
            // Just access it directly
        }
        
        // Flush the buffer
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        auto it = buffers_.find(metric_name);
        if (it == buffers_.end()) {
            return result_void(monitoring_error_code::collector_not_found,
                             "Buffer not found: " + metric_name);
        }
        
        auto flush_result = it->second->strategy->flush();
        if (!flush_result) {
            stats_.failed_flushes.fetch_add(1, std::memory_order_relaxed);
            return result_void(flush_result.get_error().code, flush_result.get_error().message);
        }
        
        auto flushed_metrics = flush_result.value();
        if (flushed_metrics.empty()) {
            return result_void::success();
        }
        
        // Store metrics if storage is available
        if (storage_) {
            metric_batch batch;
            batch.reserve(flushed_metrics.size());
            
            for (const auto& buffered_metric : flushed_metrics) {
                batch.add_metric(std::move(const_cast<compact_metric_value&>(buffered_metric.metric)));
            }
            
            storage_->store_metrics_batch(batch);
        }
        
        // Call flush callback if set
        if (flush_callback_) {
            flush_callback_(metric_name, flushed_metrics);
        }
        
        // Update statistics
        stats_.total_flushes.fetch_add(1, std::memory_order_relaxed);
        stats_.total_metrics_processed.fetch_add(flushed_metrics.size(), std::memory_order_relaxed);
        stats_.active_flushes.fetch_sub(1, std::memory_order_relaxed);
        
        // Update last flush time
        it->second->last_flush_time = std::chrono::system_clock::now();
        
        return result_void::success();
    }
    
public:
    /**
     * @brief Constructor
     */
    explicit buffer_manager(const buffer_manager_config& config = {},
                          std::shared_ptr<metric_storage> storage = nullptr)
        : config_(config), storage_(storage) {
        
        auto validation = config_.validate();
        if (!validation) {
            throw std::invalid_argument("Invalid buffer manager configuration: " + 
                                      validation.get_error().message);
        }
    }
    
    /**
     * @brief Destructor
     */
    ~buffer_manager() {
        stop_background_processing();
    }
    
    // Non-copyable and non-moveable
    buffer_manager(const buffer_manager&) = delete;
    buffer_manager& operator=(const buffer_manager&) = delete;
    buffer_manager(buffer_manager&&) = delete;
    buffer_manager& operator=(buffer_manager&&) = delete;
    
    /**
     * @brief Add metric to buffer
     */
    result_void add_metric(const std::string& metric_name,
                          compact_metric_value&& metric,
                          uint8_t priority = 128) {
        
        // Get or create buffer for this metric
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        auto it = buffers_.find(metric_name);
        if (it == buffers_.end()) {
            // Create new buffer with default configuration
            auto strategy = create_buffering_strategy(config_.default_buffering_config);
            auto entry = std::make_unique<metric_buffer_entry>(std::move(strategy), metric_name);
            
            auto insert_result = buffers_.emplace(metric_name, std::move(entry));
            it = insert_result.first;
            
            stats_.total_buffers.fetch_add(1, std::memory_order_relaxed);
        }
        
        // Create buffered metric
        buffered_metric buffered_item(std::move(metric), priority);
        
        // Add to buffer
        return it->second->strategy->add_metric(std::move(buffered_item));
    }
    
    /**
     * @brief Configure buffer strategy for a specific metric
     */
    result_void configure_metric_buffer(const std::string& metric_name,
                                       const buffering_config& config) {
        auto validation = config.validate();
        if (!validation) {
            return validation;
        }
        
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        // Create new strategy
        auto strategy = create_buffering_strategy(config);
        auto entry = std::make_unique<metric_buffer_entry>(std::move(strategy), metric_name);
        
        // Replace existing buffer or create new one
        auto it = buffers_.find(metric_name);
        if (it != buffers_.end()) {
            // Flush existing buffer first
            auto flush_result = it->second->strategy->flush();
            if (flush_result && !flush_result.value().empty()) {
                // Store flushed metrics
                if (storage_) {
                    metric_batch batch;
                    for (const auto& buffered_metric : flush_result.value()) {
                        batch.add_metric(std::move(const_cast<compact_metric_value&>(buffered_metric.metric)));
                    }
                    storage_->store_metrics_batch(batch);
                }
            }
        } else {
            stats_.total_buffers.fetch_add(1, std::memory_order_relaxed);
        }
        
        buffers_[metric_name] = std::move(entry);
        
        return result_void::success();
    }
    
    /**
     * @brief Force flush a specific metric buffer
     */
    result_void force_flush(const std::string& metric_name) {
        return flush_buffer(metric_name);
    }
    
    /**
     * @brief Force flush all buffers
     */
    result_void force_flush_all() {
        std::vector<std::string> buffer_names;
        
        {
            std::lock_guard<std::mutex> lock(buffers_mutex_);
            for (const auto& [name, entry] : buffers_) {
                buffer_names.push_back(name);
            }
        }
        
        for (const auto& name : buffer_names) {
            auto result = flush_buffer(name);
            if (!result) {
                // Continue flushing other buffers even if one fails
                stats_.failed_flushes.fetch_add(1, std::memory_order_relaxed);
            }
        }
        
        return result_void::success();
    }
    
    /**
     * @brief Get buffer statistics for a specific metric
     */
    result<buffer_statistics> get_buffer_statistics(const std::string& metric_name) const {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        auto it = buffers_.find(metric_name);
        if (it == buffers_.end()) {
            return make_error<buffer_statistics>(monitoring_error_code::collector_not_found,
                                               "Buffer not found: " + metric_name);
        }
        
        return make_success(it->second->strategy->get_statistics());
    }
    
    /**
     * @brief Get buffer size for a specific metric
     */
    result<size_t> get_buffer_size(const std::string& metric_name) const {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        auto it = buffers_.find(metric_name);
        if (it == buffers_.end()) {
            return make_error<size_t>(monitoring_error_code::collector_not_found,
                                    "Buffer not found: " + metric_name);
        }
        
        return make_success(it->second->strategy->size());
    }
    
    /**
     * @brief Get list of all metric names with buffers
     */
    std::vector<std::string> get_buffered_metrics() const {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        std::vector<std::string> metric_names;
        metric_names.reserve(buffers_.size());
        
        for (const auto& [name, entry] : buffers_) {
            metric_names.push_back(name);
        }
        
        return metric_names;
    }
    
    /**
     * @brief Start background processing
     */
    result_void start_background_processing() {
        if (running_.load(std::memory_order_acquire)) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Background processing already running");
        }
        
        if (!config_.enable_automatic_flushing) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Automatic flushing is disabled");
        }
        
        running_.store(true, std::memory_order_release);
        background_thread_ = std::thread(&buffer_manager::background_processing_loop, this);
        
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
            
            // Final flush of all buffers
            force_flush_all();
        }
    }
    
    /**
     * @brief Set flush callback
     */
    void set_flush_callback(std::function<void(const std::string&, const std::vector<buffered_metric>&)> callback) {
        flush_callback_ = std::move(callback);
    }
    
    /**
     * @brief Set metric storage
     */
    void set_storage(std::shared_ptr<metric_storage> storage) {
        storage_ = storage;
    }
    
    /**
     * @brief Get manager statistics
     */
    const buffer_manager_statistics& get_statistics() const {
        return stats_;
    }
    
    /**
     * @brief Get configuration
     */
    const buffer_manager_config& get_config() const {
        return config_;
    }
    
    /**
     * @brief Clear all buffers
     */
    void clear_all_buffers() {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        for (auto& [name, entry] : buffers_) {
            entry->strategy->clear();
        }
    }
    
    /**
     * @brief Remove buffer for a specific metric
     */
    result_void remove_buffer(const std::string& metric_name) {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        auto it = buffers_.find(metric_name);
        if (it == buffers_.end()) {
            return result_void(monitoring_error_code::collector_not_found,
                             "Buffer not found: " + metric_name);
        }
        
        // Flush before removing
        auto flush_result = it->second->strategy->flush();
        if (flush_result && !flush_result.value().empty() && storage_) {
            metric_batch batch;
            for (const auto& buffered_metric : flush_result.value()) {
                batch.add_metric(std::move(const_cast<compact_metric_value&>(buffered_metric.metric)));
            }
            storage_->store_metrics_batch(batch);
        }
        
        buffers_.erase(it);
        stats_.total_buffers.fetch_sub(1, std::memory_order_relaxed);
        
        return result_void::success();
    }
    
    /**
     * @brief Get total memory usage of all buffers
     */
    size_t get_total_memory_usage() const {
        std::lock_guard<std::mutex> lock(buffers_mutex_);
        
        size_t total_memory = 0;
        for (const auto& [name, entry] : buffers_) {
            total_memory += entry->strategy->size() * sizeof(buffered_metric);
            total_memory += name.capacity();
        }
        
        return total_memory;
    }
};

/**
 * @brief Helper function to create buffer manager with default configuration
 */
inline std::unique_ptr<buffer_manager> make_buffer_manager(
    std::shared_ptr<metric_storage> storage = nullptr) {
    return std::make_unique<buffer_manager>(buffer_manager_config{}, storage);
}

/**
 * @brief Helper function to create buffer manager with custom configuration
 */
inline std::unique_ptr<buffer_manager> make_buffer_manager(
    const buffer_manager_config& config,
    std::shared_ptr<metric_storage> storage = nullptr) {
    return std::make_unique<buffer_manager>(config, storage);
}

} // namespace monitoring_system