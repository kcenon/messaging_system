#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file buffering_strategy.h
 * @brief Configurable buffering strategies for metric collection
 * 
 * This file implements P3 task: Configurable buffering strategies
 * for optimizing metric collection and storage based on different scenarios.
 */

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <kcenon/monitoring/utils/metric_types.h>
#include <kcenon/monitoring/utils/ring_buffer.h>
#include <memory>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <algorithm>

namespace monitoring_system {

/**
 * @enum buffering_strategy_type
 * @brief Types of buffering strategies
 */
enum class buffering_strategy_type : uint8_t {
    immediate,          // Immediate processing (no buffering)
    fixed_size,         // Fixed size buffer with configurable behavior
    time_based,         // Time-based batching
    adaptive,           // Adaptive buffering based on load
    priority_based,     // Priority-based buffering
    compression_based   // Compression-aware buffering
};

/**
 * @enum buffer_overflow_policy
 * @brief Policies for handling buffer overflow
 */
enum class buffer_overflow_policy : uint8_t {
    drop_oldest,        // Drop oldest items (FIFO)
    drop_newest,        // Drop newest items
    drop_lowest_priority, // Drop lowest priority items
    compress,           // Compress buffer contents
    flush_immediately,  // Force immediate flush
    block_until_space   // Block until space is available
};

/**
 * @enum buffer_flush_trigger
 * @brief Triggers for buffer flushing
 */
enum class buffer_flush_trigger : uint8_t {
    size_threshold,     // Flush when size threshold reached
    time_interval,      // Flush at regular time intervals
    priority_threshold, // Flush when priority threshold reached
    memory_pressure,    // Flush when memory pressure detected
    manual,            // Manual flush only
    adaptive           // Adaptive flushing based on conditions
};

/**
 * @struct buffering_config
 * @brief Configuration for buffering strategy
 */
struct buffering_config {
    buffering_strategy_type strategy = buffering_strategy_type::fixed_size;
    buffer_overflow_policy overflow_policy = buffer_overflow_policy::drop_oldest;
    buffer_flush_trigger flush_trigger = buffer_flush_trigger::size_threshold;
    
    // Size-based configuration
    size_t max_buffer_size = 1024;
    size_t flush_threshold_size = 512;
    
    // Time-based configuration
    std::chrono::milliseconds flush_interval{1000};
    std::chrono::milliseconds max_age{5000};
    
    // Priority-based configuration
    uint8_t min_priority = 0;
    uint8_t max_priority = 255;
    uint8_t flush_priority_threshold = 128;
    
    // Adaptive configuration
    double load_factor_threshold = 0.8;
    std::chrono::milliseconds adaptive_check_interval{100};
    
    // Compression configuration
    double compression_ratio_threshold = 0.5;
    bool enable_compression = false;
    
    // Performance tuning
    size_t batch_size = 64;
    bool enable_background_flushing = true;
    std::chrono::milliseconds background_flush_interval{500};
    
    /**
     * @brief Validate configuration
     */
    result_void validate() const {
        if (max_buffer_size == 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Max buffer size must be positive");
        }
        
        if (flush_threshold_size > max_buffer_size) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Flush threshold cannot exceed max buffer size");
        }
        
        if (flush_interval.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Flush interval must be positive");
        }
        
        if (max_age.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Max age must be positive");
        }
        
        if (min_priority > max_priority) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Min priority cannot exceed max priority");
        }
        
        if (load_factor_threshold <= 0.0 || load_factor_threshold > 1.0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Load factor threshold must be between 0 and 1");
        }
        
        if (compression_ratio_threshold <= 0.0 || compression_ratio_threshold > 1.0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Compression ratio threshold must be between 0 and 1");
        }
        
        return result_void::success();
    }
};

/**
 * @struct buffered_metric
 * @brief Metric with buffering metadata
 */
struct buffered_metric {
    compact_metric_value metric;
    std::chrono::system_clock::time_point timestamp;
    uint8_t priority;
    size_t sequence_number;
    
    buffered_metric() : priority(128), sequence_number(0) {
        timestamp = std::chrono::system_clock::now();
    }
    
    buffered_metric(compact_metric_value m, uint8_t p = 128)
        : metric(std::move(m)), priority(p), sequence_number(0) {
        timestamp = std::chrono::system_clock::now();
    }
    
    /**
     * @brief Check if metric has expired
     */
    bool is_expired(std::chrono::milliseconds max_age) const {
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp);
        return age > max_age;
    }
    
    /**
     * @brief Get age in milliseconds
     */
    std::chrono::milliseconds get_age() const {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp);
    }
};

/**
 * @struct buffer_statistics
 * @brief Statistics for buffer performance
 */
struct buffer_statistics {
    std::atomic<size_t> total_items_buffered{0};
    std::atomic<size_t> total_items_flushed{0};
    std::atomic<size_t> items_dropped_overflow{0};
    std::atomic<size_t> items_dropped_expired{0};
    std::atomic<size_t> total_flushes{0};
    std::atomic<size_t> forced_flushes{0};
    std::atomic<size_t> compression_operations{0};
    std::atomic<size_t> bytes_saved_compression{0};
    
    std::chrono::system_clock::time_point creation_time;
    
    buffer_statistics() : creation_time(std::chrono::system_clock::now()) {}
    
    // Copy constructor for result<buffer_statistics>
    buffer_statistics(const buffer_statistics& other) 
        : total_items_buffered(other.total_items_buffered.load())
        , total_items_flushed(other.total_items_flushed.load())
        , items_dropped_overflow(other.items_dropped_overflow.load())
        , items_dropped_expired(other.items_dropped_expired.load())
        , total_flushes(other.total_flushes.load())
        , forced_flushes(other.forced_flushes.load())
        , compression_operations(other.compression_operations.load())
        , bytes_saved_compression(other.bytes_saved_compression.load())
        , creation_time(other.creation_time) {}
    
    /**
     * @brief Get buffer efficiency percentage
     */
    double get_efficiency() const {
        auto buffered = total_items_buffered.load();
        auto dropped = items_dropped_overflow.load() + items_dropped_expired.load();
        auto total = buffered + dropped;
        return total > 0 ? (static_cast<double>(buffered) / total) * 100.0 : 100.0;
    }
    
    /**
     * @brief Get average items per flush
     */
    double get_avg_items_per_flush() const {
        auto flushes = total_flushes.load();
        auto flushed = total_items_flushed.load();
        return flushes > 0 ? static_cast<double>(flushed) / flushes : 0.0;
    }
    
    /**
     * @brief Get compression ratio
     */
    double get_compression_ratio() const {
        auto operations = compression_operations.load();
        auto saved = bytes_saved_compression.load();
        return operations > 0 ? static_cast<double>(saved) / operations : 0.0;
    }
};

/**
 * @class buffer_strategy_interface
 * @brief Abstract interface for buffering strategies
 */
class buffer_strategy_interface {
public:
    virtual ~buffer_strategy_interface() = default;
    
    /**
     * @brief Add metric to buffer
     */
    virtual result_void add_metric(buffered_metric&& metric) = 0;
    
    /**
     * @brief Flush buffer contents
     */
    virtual result<std::vector<buffered_metric>> flush() = 0;
    
    /**
     * @brief Check if flush is needed
     */
    virtual bool should_flush() const = 0;
    
    /**
     * @brief Get current buffer size
     */
    virtual size_t size() const = 0;
    
    /**
     * @brief Get buffer statistics
     */
    virtual const buffer_statistics& get_statistics() const = 0;
    
    /**
     * @brief Clear buffer
     */
    virtual void clear() = 0;
    
    /**
     * @brief Get configuration
     */
    virtual const buffering_config& get_config() const = 0;
};

/**
 * @class immediate_strategy
 * @brief Immediate processing strategy (no buffering)
 */
class immediate_strategy : public buffer_strategy_interface {
private:
    buffering_config config_;
    mutable buffer_statistics stats_;
    mutable std::mutex mutex_;
    
public:
    explicit immediate_strategy(const buffering_config& config = {})
        : config_(config) {
        config_.strategy = buffering_strategy_type::immediate;
    }
    
    result_void add_metric([[maybe_unused]] buffered_metric&& metric) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Immediate processing - no actual buffering
        stats_.total_items_buffered.fetch_add(1, std::memory_order_relaxed);
        stats_.total_items_flushed.fetch_add(1, std::memory_order_relaxed);
        stats_.total_flushes.fetch_add(1, std::memory_order_relaxed);
        
        return result_void::success();
    }
    
    result<std::vector<buffered_metric>> flush() override {
        // No items to flush in immediate strategy
        return make_success(std::vector<buffered_metric>{});
    }
    
    bool should_flush() const override {
        return false;  // Never needs flushing
    }
    
    size_t size() const override {
        return 0;  // No buffering
    }
    
    const buffer_statistics& get_statistics() const override {
        return stats_;
    }
    
    void clear() override {
        // Nothing to clear
    }
    
    const buffering_config& get_config() const override {
        return config_;
    }
};

/**
 * @class fixed_size_strategy
 * @brief Fixed size buffering strategy
 */
class fixed_size_strategy : public buffer_strategy_interface {
private:
    buffering_config config_;
    std::vector<buffered_metric> buffer_;
    mutable buffer_statistics stats_;
    mutable std::mutex mutex_;
    std::atomic<size_t> sequence_counter_{0};
    
    /**
     * @brief Handle buffer overflow
     */
    void handle_overflow() {
        if (buffer_.size() < config_.max_buffer_size) {
            return;
        }
        
        switch (config_.overflow_policy) {
            case buffer_overflow_policy::drop_oldest:
                if (!buffer_.empty()) {
                    buffer_.erase(buffer_.begin());
                    stats_.items_dropped_overflow.fetch_add(1, std::memory_order_relaxed);
                }
                break;
                
            case buffer_overflow_policy::drop_newest:
                // Don't add the new item (it will be dropped by caller)
                stats_.items_dropped_overflow.fetch_add(1, std::memory_order_relaxed);
                break;
                
            case buffer_overflow_policy::drop_lowest_priority: {
                auto min_it = std::min_element(buffer_.begin(), buffer_.end(),
                    [](const buffered_metric& a, const buffered_metric& b) {
                        return a.priority < b.priority;
                    });
                if (min_it != buffer_.end()) {
                    buffer_.erase(min_it);
                    stats_.items_dropped_overflow.fetch_add(1, std::memory_order_relaxed);
                }
                break;
            }
            
            case buffer_overflow_policy::flush_immediately:
                // Force flush will be handled by should_flush()
                break;
                
            default:
                // For other policies, drop oldest as fallback
                if (!buffer_.empty()) {
                    buffer_.erase(buffer_.begin());
                    stats_.items_dropped_overflow.fetch_add(1, std::memory_order_relaxed);
                }
                break;
        }
    }
    
    /**
     * @brief Remove expired items
     */
    void remove_expired_items() {
        size_t removed = 0;
        auto it = std::remove_if(buffer_.begin(), buffer_.end(),
            [this, &removed](const buffered_metric& item) {
                if (item.is_expired(config_.max_age)) {
                    ++removed;
                    return true;
                }
                return false;
            });
        
        buffer_.erase(it, buffer_.end());
        stats_.items_dropped_expired.fetch_add(removed, std::memory_order_relaxed);
    }
    
public:
    explicit fixed_size_strategy(const buffering_config& config)
        : config_(config) {
        config_.strategy = buffering_strategy_type::fixed_size;
        buffer_.reserve(config_.max_buffer_size);
    }
    
    result_void add_metric(buffered_metric&& metric) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Remove expired items first
        remove_expired_items();
        
        // Handle overflow before adding
        if (buffer_.size() >= config_.max_buffer_size) {
            if (config_.overflow_policy == buffer_overflow_policy::drop_newest) {
                stats_.items_dropped_overflow.fetch_add(1, std::memory_order_relaxed);
                return result_void::success();  // Drop the new item
            }
            handle_overflow();
        }
        
        // Set sequence number
        metric.sequence_number = sequence_counter_.fetch_add(1, std::memory_order_relaxed);
        
        // Add to buffer
        buffer_.emplace_back(std::move(metric));
        stats_.total_items_buffered.fetch_add(1, std::memory_order_relaxed);
        
        return result_void::success();
    }
    
    result<std::vector<buffered_metric>> flush() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            return make_success(std::vector<buffered_metric>{});
        }
        
        std::vector<buffered_metric> flushed_items;
        flushed_items.reserve(buffer_.size());
        
        // Move all items to flushed_items
        for (auto& item : buffer_) {
            flushed_items.emplace_back(std::move(item));
        }
        
        stats_.total_items_flushed.fetch_add(flushed_items.size(), std::memory_order_relaxed);
        stats_.total_flushes.fetch_add(1, std::memory_order_relaxed);
        
        buffer_.clear();
        
        return make_success(std::move(flushed_items));
    }
    
    bool should_flush() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        switch (config_.flush_trigger) {
            case buffer_flush_trigger::size_threshold:
                return buffer_.size() >= config_.flush_threshold_size;
                
            case buffer_flush_trigger::manual:
                return false;
                
            default:
                return buffer_.size() >= config_.flush_threshold_size;
        }
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.size();
    }
    
    const buffer_statistics& get_statistics() const override {
        return stats_;
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.clear();
    }
    
    const buffering_config& get_config() const override {
        return config_;
    }
};

/**
 * @class time_based_strategy
 * @brief Time-based buffering strategy
 */
class time_based_strategy : public buffer_strategy_interface {
private:
    buffering_config config_;
    std::vector<buffered_metric> buffer_;
    mutable buffer_statistics stats_;
    mutable std::mutex mutex_;
    std::atomic<size_t> sequence_counter_{0};
    std::chrono::system_clock::time_point last_flush_time_;
    
public:
    explicit time_based_strategy(const buffering_config& config)
        : config_(config), last_flush_time_(std::chrono::system_clock::now()) {
        config_.strategy = buffering_strategy_type::time_based;
        buffer_.reserve(config_.max_buffer_size);
    }
    
    result_void add_metric(buffered_metric&& metric) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Set sequence number
        metric.sequence_number = sequence_counter_.fetch_add(1, std::memory_order_relaxed);
        
        // Check if buffer is full
        if (buffer_.size() >= config_.max_buffer_size) {
            // Force flush if buffer is full
            stats_.forced_flushes.fetch_add(1, std::memory_order_relaxed);
        }
        
        buffer_.emplace_back(std::move(metric));
        stats_.total_items_buffered.fetch_add(1, std::memory_order_relaxed);
        
        return result_void::success();
    }
    
    result<std::vector<buffered_metric>> flush() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            return make_success(std::vector<buffered_metric>{});
        }
        
        std::vector<buffered_metric> flushed_items;
        flushed_items.reserve(buffer_.size());
        
        // Move all items to flushed_items
        for (auto& item : buffer_) {
            flushed_items.emplace_back(std::move(item));
        }
        
        stats_.total_items_flushed.fetch_add(flushed_items.size(), std::memory_order_relaxed);
        stats_.total_flushes.fetch_add(1, std::memory_order_relaxed);
        
        buffer_.clear();
        last_flush_time_ = std::chrono::system_clock::now();
        
        return make_success(std::move(flushed_items));
    }
    
    bool should_flush() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            return false;
        }
        
        auto now = std::chrono::system_clock::now();
        auto time_since_flush = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_flush_time_);
        
        // Flush if time interval reached or buffer is full
        return time_since_flush >= config_.flush_interval || 
               buffer_.size() >= config_.max_buffer_size;
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.size();
    }
    
    const buffer_statistics& get_statistics() const override {
        return stats_;
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.clear();
        last_flush_time_ = std::chrono::system_clock::now();
    }
    
    const buffering_config& get_config() const override {
        return config_;
    }
};

/**
 * @class priority_based_strategy
 * @brief Priority-based buffering strategy
 */
class priority_based_strategy : public buffer_strategy_interface {
private:
    buffering_config config_;
    std::vector<buffered_metric> buffer_;
    mutable buffer_statistics stats_;
    mutable std::mutex mutex_;
    std::atomic<size_t> sequence_counter_{0};
    
    /**
     * @brief Sort buffer by priority (highest first)
     */
    void sort_by_priority() {
        std::sort(buffer_.begin(), buffer_.end(),
            [](const buffered_metric& a, const buffered_metric& b) {
                if (a.priority != b.priority) {
                    return a.priority > b.priority;  // Higher priority first
                }
                return a.sequence_number < b.sequence_number;  // FIFO for same priority
            });
    }
    
public:
    explicit priority_based_strategy(const buffering_config& config)
        : config_(config) {
        config_.strategy = buffering_strategy_type::priority_based;
        buffer_.reserve(config_.max_buffer_size);
    }
    
    result_void add_metric(buffered_metric&& metric) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Set sequence number
        metric.sequence_number = sequence_counter_.fetch_add(1, std::memory_order_relaxed);
        
        // Handle overflow by dropping lowest priority items
        if (buffer_.size() >= config_.max_buffer_size) {
            sort_by_priority();
            
            // Remove lowest priority item
            if (!buffer_.empty()) {
                buffer_.pop_back();
                stats_.items_dropped_overflow.fetch_add(1, std::memory_order_relaxed);
            }
        }
        
        buffer_.emplace_back(std::move(metric));
        stats_.total_items_buffered.fetch_add(1, std::memory_order_relaxed);
        
        return result_void::success();
    }
    
    result<std::vector<buffered_metric>> flush() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            return make_success(std::vector<buffered_metric>{});
        }
        
        // Sort by priority before flushing
        sort_by_priority();
        
        std::vector<buffered_metric> flushed_items;
        flushed_items.reserve(buffer_.size());
        
        // Move all items to flushed_items
        for (auto& item : buffer_) {
            flushed_items.emplace_back(std::move(item));
        }
        
        stats_.total_items_flushed.fetch_add(flushed_items.size(), std::memory_order_relaxed);
        stats_.total_flushes.fetch_add(1, std::memory_order_relaxed);
        
        buffer_.clear();
        
        return make_success(std::move(flushed_items));
    }
    
    bool should_flush() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            return false;
        }
        
        // Check if we have high-priority items
        for (const auto& item : buffer_) {
            if (item.priority >= config_.flush_priority_threshold) {
                return true;
            }
        }
        
        // Also flush if buffer is getting full
        return buffer_.size() >= config_.flush_threshold_size;
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.size();
    }
    
    const buffer_statistics& get_statistics() const override {
        return stats_;
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.clear();
    }
    
    const buffering_config& get_config() const override {
        return config_;
    }
};

/**
 * @class adaptive_strategy
 * @brief Adaptive buffering strategy based on system load
 */
class adaptive_strategy : public buffer_strategy_interface {
private:
    buffering_config config_;
    std::vector<buffered_metric> buffer_;
    mutable buffer_statistics stats_;
    mutable std::mutex mutex_;
    std::atomic<size_t> sequence_counter_{0};
    std::chrono::system_clock::time_point last_adaptation_;
    double current_load_factor_ = 0.0;
    
    /**
     * @brief Calculate current load factor
     */
    double calculate_load_factor() const {
        // Simple load factor based on buffer utilization
        double buffer_utilization = static_cast<double>(buffer_.size()) / config_.max_buffer_size;
        
        // Add time pressure (how long since last flush)
        auto now = std::chrono::system_clock::now();
        auto time_pressure = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_adaptation_).count() / static_cast<double>(config_.flush_interval.count());
        
        return std::min(1.0, buffer_utilization + time_pressure * 0.1);
    }
    
    /**
     * @brief Adapt buffer behavior based on load
     */
    void adapt_to_load() {
        current_load_factor_ = calculate_load_factor();
        last_adaptation_ = std::chrono::system_clock::now();
        
        // Adjust flush threshold based on load
        if (current_load_factor_ > config_.load_factor_threshold) {
            // High load - reduce flush threshold for more frequent flushing
            config_.flush_threshold_size = std::max(
                config_.batch_size,
                static_cast<size_t>(config_.max_buffer_size * 0.3)
            );
        } else {
            // Normal load - use standard threshold
            config_.flush_threshold_size = config_.max_buffer_size / 2;
        }
    }
    
public:
    explicit adaptive_strategy(const buffering_config& config)
        : config_(config), last_adaptation_(std::chrono::system_clock::now()) {
        config_.strategy = buffering_strategy_type::adaptive;
        buffer_.reserve(config_.max_buffer_size);
    }
    
    result_void add_metric(buffered_metric&& metric) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Periodic adaptation
        auto now = std::chrono::system_clock::now();
        if (now - last_adaptation_ >= config_.adaptive_check_interval) {
            adapt_to_load();
        }
        
        // Set sequence number
        metric.sequence_number = sequence_counter_.fetch_add(1, std::memory_order_relaxed);
        
        // Handle overflow adaptively
        if (buffer_.size() >= config_.max_buffer_size) {
            if (current_load_factor_ > config_.load_factor_threshold) {
                // High load - drop oldest to make space
                if (!buffer_.empty()) {
                    buffer_.erase(buffer_.begin());
                    stats_.items_dropped_overflow.fetch_add(1, std::memory_order_relaxed);
                }
            } else {
                // Normal load - trigger immediate flush
                stats_.forced_flushes.fetch_add(1, std::memory_order_relaxed);
            }
        }
        
        buffer_.emplace_back(std::move(metric));
        stats_.total_items_buffered.fetch_add(1, std::memory_order_relaxed);
        
        return result_void::success();
    }
    
    result<std::vector<buffered_metric>> flush() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            return make_success(std::vector<buffered_metric>{});
        }
        
        std::vector<buffered_metric> flushed_items;
        flushed_items.reserve(buffer_.size());
        
        // Move all items to flushed_items
        for (auto& item : buffer_) {
            flushed_items.emplace_back(std::move(item));
        }
        
        stats_.total_items_flushed.fetch_add(flushed_items.size(), std::memory_order_relaxed);
        stats_.total_flushes.fetch_add(1, std::memory_order_relaxed);
        
        buffer_.clear();
        
        return make_success(std::move(flushed_items));
    }
    
    bool should_flush() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (buffer_.empty()) {
            return false;
        }
        
        // Flush based on current load and adaptive threshold
        return buffer_.size() >= config_.flush_threshold_size ||
               current_load_factor_ > config_.load_factor_threshold;
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.size();
    }
    
    const buffer_statistics& get_statistics() const override {
        return stats_;
    }
    
    void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.clear();
    }
    
    const buffering_config& get_config() const override {
        return config_;
    }
};

/**
 * @brief Factory function to create buffering strategy
 */
inline std::unique_ptr<buffer_strategy_interface> create_buffering_strategy(
    const buffering_config& config) {
    
    auto validation = config.validate();
    if (!validation) {
        throw std::invalid_argument("Invalid buffering configuration: " + 
                                  validation.get_error().message);
    }
    
    switch (config.strategy) {
        case buffering_strategy_type::immediate:
            return std::make_unique<immediate_strategy>(config);
            
        case buffering_strategy_type::fixed_size:
            return std::make_unique<fixed_size_strategy>(config);
            
        case buffering_strategy_type::time_based:
            return std::make_unique<time_based_strategy>(config);
            
        case buffering_strategy_type::priority_based:
            return std::make_unique<priority_based_strategy>(config);
            
        case buffering_strategy_type::adaptive:
            return std::make_unique<adaptive_strategy>(config);
            
        default:
            return std::make_unique<fixed_size_strategy>(config);
    }
}

/**
 * @brief Create default buffering configurations for common scenarios
 */
inline std::vector<buffering_config> create_default_buffering_configs() {
    std::vector<buffering_config> configs;
    
    // High-throughput configuration
    buffering_config high_throughput;
    high_throughput.strategy = buffering_strategy_type::fixed_size;
    high_throughput.max_buffer_size = 4096;
    high_throughput.flush_threshold_size = 2048;
    high_throughput.overflow_policy = buffer_overflow_policy::drop_oldest;
    high_throughput.enable_background_flushing = true;
    configs.push_back(high_throughput);
    
    // Low-latency configuration
    buffering_config low_latency;
    low_latency.strategy = buffering_strategy_type::time_based;
    low_latency.max_buffer_size = 512;
    low_latency.flush_interval = std::chrono::milliseconds(100);
    low_latency.flush_threshold_size = 64;
    configs.push_back(low_latency);
    
    // Priority-sensitive configuration
    buffering_config priority_sensitive;
    priority_sensitive.strategy = buffering_strategy_type::priority_based;
    priority_sensitive.max_buffer_size = 1024;
    priority_sensitive.flush_priority_threshold = 200;
    priority_sensitive.overflow_policy = buffer_overflow_policy::drop_lowest_priority;
    configs.push_back(priority_sensitive);
    
    // Adaptive configuration
    buffering_config adaptive;
    adaptive.strategy = buffering_strategy_type::adaptive;
    adaptive.max_buffer_size = 2048;
    adaptive.load_factor_threshold = 0.7;
    adaptive.adaptive_check_interval = std::chrono::milliseconds(200);
    configs.push_back(adaptive);
    
    return configs;
}

} // namespace monitoring_system