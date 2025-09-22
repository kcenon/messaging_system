#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file metric_storage.h
 * @brief Memory-efficient metric storage with ring buffers
 * 
 * This file implements the P1 task: Memory-efficient metric storage
 * using ring buffers for optimal performance and memory usage.
 */

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <kcenon/monitoring/utils/ring_buffer.h>
#include <kcenon/monitoring/utils/metric_types.h>
#include <kcenon/monitoring/utils/time_series.h>
#include <kcenon/monitoring/interfaces/monitoring_interface.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace monitoring_system {

/**
 * @struct metric_storage_config
 * @brief Configuration for metric storage system
 */
struct metric_storage_config {
    size_t ring_buffer_capacity = 8192;               // Ring buffer size per metric
    size_t max_metrics = 10000;                       // Maximum number of different metrics
    std::chrono::seconds retention_period{3600};      // Data retention period
    std::chrono::milliseconds flush_interval{1000};   // Flush interval for persistence
    bool enable_compression = true;                    // Enable data compression
    bool enable_background_processing = true;         // Enable background processing
    size_t batch_size = 256;                          // Batch size for processing
    
    /**
     * @brief Validate configuration
     */
    result_void validate() const {
        if (ring_buffer_capacity == 0 || (ring_buffer_capacity & (ring_buffer_capacity - 1)) != 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Ring buffer capacity must be a power of 2");
        }
        
        if (max_metrics == 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Max metrics must be positive");
        }
        
        if (retention_period.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Retention period must be positive");
        }
        
        if (flush_interval.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Flush interval must be positive");
        }
        
        return result_void::success();
    }
};

/**
 * @struct metric_storage_stats
 * @brief Statistics for metric storage performance
 */
struct metric_storage_stats {
    std::atomic<size_t> total_metrics_stored{0};
    std::atomic<size_t> total_metrics_dropped{0};
    std::atomic<size_t> active_metric_series{0};
    std::atomic<size_t> memory_usage_bytes{0};
    std::atomic<size_t> compression_saves_bytes{0};
    std::atomic<size_t> background_flushes{0};
    std::atomic<size_t> storage_errors{0};
    std::chrono::system_clock::time_point creation_time;
    
    metric_storage_stats() : creation_time(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Get storage efficiency percentage
     */
    double get_storage_efficiency() const {
        auto stored = total_metrics_stored.load();
        auto dropped = total_metrics_dropped.load();
        auto total = stored + dropped;
        return total > 0 ? (static_cast<double>(stored) / total) * 100.0 : 100.0;
    }
    
    /**
     * @brief Get average memory per metric
     */
    double get_avg_memory_per_metric() const {
        auto active = active_metric_series.load();
        auto memory = memory_usage_bytes.load();
        return active > 0 ? static_cast<double>(memory) / active : 0.0;
    }
};

/**
 * @class metric_storage
 * @brief Memory-efficient metric storage using ring buffers
 */
class metric_storage {
private:
    struct metric_series_entry {
        std::unique_ptr<ring_buffer<compact_metric_value>> buffer;
        std::unique_ptr<time_series> series;
        std::chrono::system_clock::time_point last_access;
        metric_metadata metadata;
        
        metric_series_entry(metric_metadata meta, const metric_storage_config& config) 
            : last_access(std::chrono::system_clock::now()), metadata(meta) {
            
            ring_buffer_config ring_config;
            ring_config.capacity = config.ring_buffer_capacity;
            ring_config.overwrite_old = true;
            ring_config.batch_size = config.batch_size;
            
            buffer = std::make_unique<ring_buffer<compact_metric_value>>(ring_config);
            
            time_series_config ts_config;
            ts_config.retention_period = config.retention_period;
            ts_config.enable_compression = config.enable_compression;
            
            series = std::make_unique<time_series>("metric_" + std::to_string(meta.name_hash), ts_config);
        }
    };
    
    mutable std::mutex storage_mutex_;
    std::unordered_map<uint32_t, std::unique_ptr<metric_series_entry>> metric_series_;
    metric_storage_config config_;
    metric_storage_stats stats_;
    
    // Background processing
    std::atomic<bool> running_{false};
    std::thread background_thread_;
    std::condition_variable background_cv_;
    mutable std::mutex background_mutex_;
    
    /**
     * @brief Get or create metric series entry
     */
    metric_series_entry* get_or_create_series(const metric_metadata& metadata) {
        std::lock_guard<std::mutex> lock(storage_mutex_);
        
        auto it = metric_series_.find(metadata.name_hash);
        if (it != metric_series_.end()) {
            it->second->last_access = std::chrono::system_clock::now();
            return it->second.get();
        }
        
        // Check if we've reached the maximum number of metrics
        if (metric_series_.size() >= config_.max_metrics) {
            stats_.storage_errors.fetch_add(1, std::memory_order_relaxed);
            return nullptr;  // Storage full
        }
        
        // Create new series
        auto entry = std::make_unique<metric_series_entry>(metadata, config_);
        auto* ptr = entry.get();
        
        metric_series_[metadata.name_hash] = std::move(entry);
        stats_.active_metric_series.store(metric_series_.size(), std::memory_order_relaxed);
        
        return ptr;
    }
    
    /**
     * @brief Background processing thread
     */
    void background_processing_loop() {
        while (running_.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(background_mutex_);
            
            // Wait for flush interval or shutdown
            if (background_cv_.wait_for(lock, config_.flush_interval, 
                                      [this] { return !running_.load(std::memory_order_acquire); })) {
                break;  // Shutdown requested
            }
            
            lock.unlock();
            
            // Perform background tasks
            flush_ring_buffers_to_series();
            cleanup_old_series();
            update_memory_usage();
            
            stats_.background_flushes.fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    /**
     * @brief Flush ring buffers to time series
     */
    void flush_ring_buffers_to_series() {
        std::vector<uint32_t> series_to_flush;
        
        {
            std::lock_guard<std::mutex> lock(storage_mutex_);
            series_to_flush.reserve(metric_series_.size());
            
            for (const auto& [hash, entry] : metric_series_) {
                if (!entry->buffer->empty()) {
                    series_to_flush.push_back(hash);
                }
            }
        }
        
        // Process each series outside the lock
        for (uint32_t hash : series_to_flush) {
            std::lock_guard<std::mutex> lock(storage_mutex_);
            auto it = metric_series_.find(hash);
            if (it == metric_series_.end()) continue;
            
            auto& entry = it->second;
            std::vector<compact_metric_value> batch;
            
            // Read batch from ring buffer
            size_t read_count = entry->buffer->read_batch(batch, config_.batch_size);
            
            if (read_count > 0) {
                // Convert and add to time series
                std::vector<time_point_data> time_points;
                time_points.reserve(read_count);
                
                for (const auto& metric : batch) {
                    time_points.emplace_back(
                        metric.get_timestamp(),
                        metric.as_double()
                    );
                }
                
                entry->series->add_points(time_points);
            }
        }
    }
    
    /**
     * @brief Cleanup old series that haven't been accessed recently
     */
    void cleanup_old_series() {
        auto cutoff = std::chrono::system_clock::now() - config_.retention_period;
        std::vector<uint32_t> to_remove;
        
        {
            std::lock_guard<std::mutex> lock(storage_mutex_);
            
            for (const auto& [hash, entry] : metric_series_) {
                if (entry->last_access < cutoff && entry->buffer->empty()) {
                    to_remove.push_back(hash);
                }
            }
            
            for (uint32_t hash : to_remove) {
                metric_series_.erase(hash);
            }
            
            stats_.active_metric_series.store(metric_series_.size(), std::memory_order_relaxed);
        }
    }
    
    /**
     * @brief Update memory usage statistics
     */
    void update_memory_usage() {
        size_t total_memory = 0;
        
        {
            std::lock_guard<std::mutex> lock(storage_mutex_);
            
            for (const auto& [hash, entry] : metric_series_) {
                total_memory += entry->series->memory_footprint();
                total_memory += sizeof(metric_series_entry);
                // Ring buffer memory is fixed per series
                total_memory += entry->buffer->capacity() * sizeof(compact_metric_value);
            }
        }
        
        stats_.memory_usage_bytes.store(total_memory, std::memory_order_relaxed);
    }
    
public:
    /**
     * @brief Constructor
     */
    explicit metric_storage(const metric_storage_config& config = {})
        : config_(config) {
        
        auto validation = config_.validate();
        if (!validation) {
            throw std::invalid_argument("Invalid metric storage configuration: " + 
                                      validation.get_error().message);
        }
        
        if (config_.enable_background_processing) {
            running_.store(true, std::memory_order_release);
            background_thread_ = std::thread(&metric_storage::background_processing_loop, this);
        }
    }
    
    /**
     * @brief Destructor
     */
    ~metric_storage() {
        shutdown();
    }
    
    // Non-copyable and non-moveable (due to mutex)
    metric_storage(const metric_storage&) = delete;
    metric_storage& operator=(const metric_storage&) = delete;
    metric_storage(metric_storage&&) = delete;
    metric_storage& operator=(metric_storage&&) = delete;
    
    /**
     * @brief Store a metric value
     */
    result_void store_metric(const std::string& name, 
                           double value,
                           metric_type type = metric_type::gauge,
                           std::chrono::system_clock::time_point timestamp = 
                           std::chrono::system_clock::now()) {
        
        auto metadata = create_metric_metadata(name, type);
        compact_metric_value metric(metadata, value);
        metric.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            timestamp.time_since_epoch()).count();
        
        auto* series = get_or_create_series(metadata);
        if (!series) {
            stats_.total_metrics_dropped.fetch_add(1, std::memory_order_relaxed);
            return result_void(monitoring_error_code::storage_full,
                             "Storage capacity exceeded");
        }
        
        auto result = series->buffer->write(std::move(metric));
        if (result) {
            stats_.total_metrics_stored.fetch_add(1, std::memory_order_relaxed);
        } else {
            stats_.total_metrics_dropped.fetch_add(1, std::memory_order_relaxed);
        }
        
        return result;
    }
    
    /**
     * @brief Store multiple metrics in batch
     */
    size_t store_metrics_batch(const metric_batch& batch) {
        size_t stored_count = 0;
        
        for (const auto& metric : batch.metrics) {
            auto* series = get_or_create_series(metric.metadata);
            if (series) {
                auto result = series->buffer->write(compact_metric_value(metric));
                if (result) {
                    ++stored_count;
                    stats_.total_metrics_stored.fetch_add(1, std::memory_order_relaxed);
                } else {
                    stats_.total_metrics_dropped.fetch_add(1, std::memory_order_relaxed);
                }
            } else {
                stats_.total_metrics_dropped.fetch_add(1, std::memory_order_relaxed);
            }
        }
        
        return stored_count;
    }
    
    /**
     * @brief Query time series data for a metric
     */
    result<aggregation_result> query_metric(const std::string& name,
                                           const time_series_query& query) const {
        auto hash = hash_metric_name(name);
        
        std::lock_guard<std::mutex> lock(storage_mutex_);
        auto it = metric_series_.find(hash);
        
        if (it == metric_series_.end()) {
            return make_error<aggregation_result>(monitoring_error_code::collector_not_found,
                                                "Metric not found: " + name);
        }
        
        return it->second->series->query(query);
    }
    
    /**
     * @brief Get latest value for a metric
     */
    result<double> get_latest_value(const std::string& name) const {
        auto hash = hash_metric_name(name);
        
        std::lock_guard<std::mutex> lock(storage_mutex_);
        auto it = metric_series_.find(hash);
        
        if (it == metric_series_.end()) {
            return make_error<double>(monitoring_error_code::collector_not_found,
                                    "Metric not found: " + name);
        }
        
        return it->second->series->get_latest_value();
    }
    
    /**
     * @brief Get list of all metric names
     */
    std::vector<std::string> get_metric_names() const {
        std::vector<std::string> names;
        
        std::lock_guard<std::mutex> lock(storage_mutex_);
        names.reserve(metric_series_.size());
        
        for (const auto& [hash, entry] : metric_series_) {
            names.push_back(entry->series->name());
        }
        
        return names;
    }
    
    /**
     * @brief Get storage statistics
     */
    const metric_storage_stats& get_stats() const noexcept {
        return stats_;
    }
    
    /**
     * @brief Get storage configuration
     */
    const metric_storage_config& get_config() const noexcept {
        return config_;
    }
    
    /**
     * @brief Flush all ring buffers to time series immediately
     */
    void flush() {
        flush_ring_buffers_to_series();
    }
    
    /**
     * @brief Clear all stored data
     */
    void clear() {
        std::lock_guard<std::mutex> lock(storage_mutex_);
        metric_series_.clear();
        stats_.active_metric_series.store(0, std::memory_order_relaxed);
        stats_.memory_usage_bytes.store(0, std::memory_order_relaxed);
    }
    
    /**
     * @brief Shutdown background processing
     */
    void shutdown() {
        if (running_.load(std::memory_order_acquire)) {
            running_.store(false, std::memory_order_release);
            background_cv_.notify_all();
            
            if (background_thread_.joinable()) {
                background_thread_.join();
            }
            
            // Final flush
            flush_ring_buffers_to_series();
        }
    }
    
    /**
     * @brief Get current memory footprint
     */
    size_t memory_footprint() const {
        return stats_.memory_usage_bytes.load(std::memory_order_relaxed);
    }
};

/**
 * @brief Helper function to create metric storage with default configuration
 */
inline std::unique_ptr<metric_storage> make_metric_storage() {
    return std::make_unique<metric_storage>();
}

/**
 * @brief Helper function to create metric storage with custom configuration
 */
inline std::unique_ptr<metric_storage> make_metric_storage(const metric_storage_config& config) {
    return std::make_unique<metric_storage>(config);
}

} // namespace monitoring_system