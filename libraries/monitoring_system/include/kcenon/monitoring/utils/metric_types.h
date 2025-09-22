#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file metric_types.h
 * @brief Common metric type definitions for efficient storage
 * 
 * This file defines standardized metric types optimized for memory efficiency
 * and cache-friendly access patterns in the monitoring system.
 */

#include "../core/result_types.h"
#include "../core/error_codes.h"
#include <string>
#include <chrono>
#include <unordered_map>
#include <variant>
#include <cstdint>

namespace monitoring_system {

/**
 * @enum metric_type
 * @brief Types of metrics supported by the system
 */
enum class metric_type : uint8_t {
    counter = 0,        // Monotonically increasing value
    gauge,              // Instantaneous value
    histogram,          // Distribution of values
    summary,            // Summary statistics
    timer,              // Duration measurements
    set                 // Unique value counting
};

/**
 * @brief Convert metric type to string
 */
constexpr const char* metric_type_to_string(metric_type type) noexcept {
    switch (type) {
        case metric_type::counter:   return "counter";
        case metric_type::gauge:     return "gauge";
        case metric_type::histogram: return "histogram";
        case metric_type::summary:   return "summary";
        case metric_type::timer:     return "timer";
        case metric_type::set:       return "set";
        default:                     return "unknown";
    }
}

/**
 * @struct metric_metadata
 * @brief Compact metadata for metrics
 */
struct metric_metadata {
    uint32_t name_hash;                              // Hashed metric name for fast lookup
    metric_type type;                                // Type of metric
    uint8_t tag_count;                              // Number of tags (max 255)
    uint16_t reserved;                              // Reserved for future use
    
    metric_metadata() noexcept 
        : name_hash(0), type(metric_type::gauge), tag_count(0), reserved(0) {}
    
    metric_metadata(uint32_t hash, metric_type mt, uint8_t tags = 0) noexcept
        : name_hash(hash), type(mt), tag_count(tags), reserved(0) {}
};

/**
 * @struct compact_metric_value
 * @brief Memory-efficient metric value storage
 */
struct compact_metric_value {
    using value_type = std::variant<
        double,           // Numeric value
        int64_t,          // Integer value
        std::string       // String value (for sets)
    >;
    
    metric_metadata metadata;
    value_type value;
    uint64_t timestamp_us;  // Microseconds since epoch for precision
    
    compact_metric_value() noexcept 
        : value(0.0), timestamp_us(0) {}
    
    compact_metric_value(const metric_metadata& meta, double val) noexcept
        : metadata(meta), value(val) {
        timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    compact_metric_value(const metric_metadata& meta, int64_t val) noexcept
        : metadata(meta), value(val) {
        timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    compact_metric_value(const metric_metadata& meta, std::string val) noexcept
        : metadata(meta), value(std::move(val)) {
        timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    /**
     * @brief Get value as double
     */
    double as_double() const {
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        } else if (std::holds_alternative<int64_t>(value)) {
            return static_cast<double>(std::get<int64_t>(value));
        }
        return 0.0;
    }
    
    /**
     * @brief Get value as integer
     */
    int64_t as_int64() const {
        if (std::holds_alternative<int64_t>(value)) {
            return std::get<int64_t>(value);
        } else if (std::holds_alternative<double>(value)) {
            return static_cast<int64_t>(std::get<double>(value));
        }
        return 0;
    }
    
    /**
     * @brief Get value as string
     */
    std::string as_string() const {
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        } else if (std::holds_alternative<double>(value)) {
            return std::to_string(std::get<double>(value));
        } else if (std::holds_alternative<int64_t>(value)) {
            return std::to_string(std::get<int64_t>(value));
        }
        return "";
    }
    
    /**
     * @brief Get timestamp as time_point
     */
    std::chrono::system_clock::time_point get_timestamp() const {
        return std::chrono::system_clock::time_point(
            std::chrono::microseconds(timestamp_us));
    }
    
    /**
     * @brief Check if metric is numeric
     */
    bool is_numeric() const noexcept {
        return std::holds_alternative<double>(value) || 
               std::holds_alternative<int64_t>(value);
    }
    
    /**
     * @brief Get memory footprint in bytes
     */
    size_t memory_footprint() const noexcept {
        size_t base_size = sizeof(metric_metadata) + sizeof(timestamp_us) + sizeof(value_type);
        if (std::holds_alternative<std::string>(value)) {
            base_size += std::get<std::string>(value).capacity();
        }
        return base_size;
    }
};

/**
 * @struct metric_batch
 * @brief Batch of metrics for efficient processing
 */
struct metric_batch {
    std::vector<compact_metric_value> metrics;
    std::chrono::system_clock::time_point batch_timestamp;
    size_t batch_id;
    
    metric_batch() : batch_timestamp(std::chrono::system_clock::now()), batch_id(0) {}
    
    explicit metric_batch(size_t id) 
        : batch_timestamp(std::chrono::system_clock::now()), batch_id(id) {}
    
    /**
     * @brief Add metric to batch
     */
    void add_metric(compact_metric_value&& metric) {
        metrics.emplace_back(std::move(metric));
    }
    
    /**
     * @brief Get batch size in bytes
     */
    size_t memory_footprint() const noexcept {
        size_t total = sizeof(metric_batch);
        for (const auto& metric : metrics) {
            total += metric.memory_footprint();
        }
        return total;
    }
    
    /**
     * @brief Reserve space for metrics
     */
    void reserve(size_t count) {
        metrics.reserve(count);
    }
    
    /**
     * @brief Clear all metrics
     */
    void clear() {
        metrics.clear();
        batch_timestamp = std::chrono::system_clock::now();
    }
    
    /**
     * @brief Check if batch is empty
     */
    bool empty() const noexcept {
        return metrics.empty();
    }
    
    /**
     * @brief Get number of metrics in batch
     */
    size_t size() const noexcept {
        return metrics.size();
    }
};

/**
 * @struct histogram_bucket
 * @brief Bucket for histogram metrics
 */
struct histogram_bucket {
    double upper_bound;
    uint64_t count;
    
    histogram_bucket(double bound = 0.0, uint64_t cnt = 0) noexcept
        : upper_bound(bound), count(cnt) {}
    
    bool operator<(const histogram_bucket& other) const noexcept {
        return upper_bound < other.upper_bound;
    }
};

/**
 * @struct histogram_data
 * @brief Histogram data with buckets
 */
struct histogram_data {
    std::vector<histogram_bucket> buckets;
    uint64_t total_count = 0;
    double sum = 0.0;
    
    /**
     * @brief Add value to histogram
     */
    void add_sample(double value) {
        sum += value;
        total_count++;
        
        for (auto& bucket : buckets) {
            if (value <= bucket.upper_bound) {
                bucket.count++;
            }
        }
    }
    
    /**
     * @brief Get mean value
     */
    double mean() const noexcept {
        return total_count > 0 ? sum / total_count : 0.0;
    }
    
    /**
     * @brief Initialize standard buckets
     */
    void init_standard_buckets() {
        buckets = {
            {0.005, 0}, {0.01, 0}, {0.025, 0}, {0.05, 0}, {0.075, 0},
            {0.1, 0}, {0.25, 0}, {0.5, 0}, {0.75, 0}, {1.0, 0},
            {2.5, 0}, {5.0, 0}, {7.5, 0}, {10.0, 0}, 
            {std::numeric_limits<double>::infinity(), 0}
        };
    }
};

/**
 * @struct summary_data
 * @brief Summary statistics for metrics
 */
struct summary_data {
    uint64_t count = 0;
    double sum = 0.0;
    double min_value = std::numeric_limits<double>::max();
    double max_value = std::numeric_limits<double>::lowest();
    
    /**
     * @brief Add sample to summary
     */
    void add_sample(double value) {
        count++;
        sum += value;
        min_value = std::min(min_value, value);
        max_value = std::max(max_value, value);
    }
    
    /**
     * @brief Get mean value
     */
    double mean() const noexcept {
        return count > 0 ? sum / count : 0.0;
    }
    
    /**
     * @brief Reset summary
     */
    void reset() {
        count = 0;
        sum = 0.0;
        min_value = std::numeric_limits<double>::max();
        max_value = std::numeric_limits<double>::lowest();
    }
};

/**
 * @brief Hash function for metric names
 */
inline uint32_t hash_metric_name(const std::string& name) noexcept {
    // Simple FNV-1a hash for fast metric name hashing
    uint32_t hash = 2166136261U;
    for (char c : name) {
        hash ^= static_cast<uint32_t>(c);
        hash *= 16777619U;
    }
    return hash;
}

/**
 * @brief Create metric metadata from name and type
 */
inline metric_metadata create_metric_metadata(const std::string& name, 
                                             metric_type type,
                                             size_t tag_count = 0) {
    return metric_metadata(
        hash_metric_name(name), 
        type, 
        static_cast<uint8_t>(std::min(tag_count, size_t(255)))
    );
}

} // namespace monitoring_system