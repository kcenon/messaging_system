#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file time_series.h
 * @brief Time-series data storage for efficient metric history
 * 
 * This file provides time-series data structures optimized for metric
 * storage with configurable retention policies and efficient querying.
 */

#include "../core/result_types.h"
#include "../core/error_codes.h"
#include "metric_types.h"
#include "ring_buffer.h"
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <memory>
#include <mutex>

namespace monitoring_system {

/**
 * @struct time_series_config
 * @brief Configuration for time series storage
 */
struct time_series_config {
    std::chrono::seconds retention_period{3600};      // How long to keep data
    std::chrono::milliseconds resolution{1000};       // Time resolution for aggregation
    size_t max_points = 3600;                         // Maximum data points to store
    bool enable_compression = true;                    // Enable data compression
    double compression_threshold = 0.01;              // Threshold for compression
    
    /**
     * @brief Validate configuration
     */
    result_void validate() const {
        if (retention_period.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Retention period must be positive");
        }
        
        if (resolution.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Resolution must be positive");
        }
        
        if (max_points == 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Max points must be positive");
        }
        
        return result_void::success();
    }
};

/**
 * @struct time_point_data
 * @brief Single data point in time series
 */
struct time_point_data {
    std::chrono::system_clock::time_point timestamp;
    double value;
    uint32_t sample_count;  // Number of samples aggregated into this point
    
    time_point_data() noexcept : value(0.0), sample_count(0) {}
    
    time_point_data(std::chrono::system_clock::time_point ts, double val, uint32_t count = 1) noexcept
        : timestamp(ts), value(val), sample_count(count) {}
    
    /**
     * @brief Merge another data point (for aggregation)
     */
    void merge(const time_point_data& other) {
        if (sample_count == 0) {
            *this = other;
        } else if (other.sample_count > 0) {
            double total_weight = sample_count + other.sample_count;
            value = (value * sample_count + other.value * other.sample_count) / total_weight;
            sample_count += other.sample_count;
            
            // Use the later timestamp
            if (other.timestamp > timestamp) {
                timestamp = other.timestamp;
            }
        }
    }
    
    /**
     * @brief Check if this point is within retention period
     */
    bool is_valid(std::chrono::system_clock::time_point cutoff) const noexcept {
        return timestamp >= cutoff;
    }
};

/**
 * @struct time_series_query
 * @brief Query parameters for time series data
 */
struct time_series_query {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::chrono::milliseconds step{1000};  // Aggregation step size
    
    time_series_query() {
        auto now = std::chrono::system_clock::now();
        end_time = now;
        start_time = now - std::chrono::hours(1);  // Default: last hour
    }
    
    time_series_query(std::chrono::system_clock::time_point start,
                     std::chrono::system_clock::time_point end,
                     std::chrono::milliseconds step_size = std::chrono::milliseconds(1000))
        : start_time(start), end_time(end), step(step_size) {}
    
    /**
     * @brief Validate query parameters
     */
    result_void validate() const {
        if (start_time >= end_time) {
            return result_void(monitoring_error_code::invalid_argument,
                             "Start time must be before end time");
        }
        
        if (step.count() <= 0) {
            return result_void(monitoring_error_code::invalid_argument,
                             "Step size must be positive");
        }
        
        return result_void::success();
    }
};

/**
 * @struct aggregation_result
 * @brief Result of time series aggregation
 */
struct aggregation_result {
    std::vector<time_point_data> points;
    std::chrono::system_clock::time_point query_start;
    std::chrono::system_clock::time_point query_end;
    size_t total_samples;
    
    aggregation_result() : total_samples(0) {}
    
    /**
     * @brief Get statistics for the aggregated data
     */
    summary_data get_summary() const {
        summary_data summary;
        for (const auto& point : points) {
            summary.add_sample(point.value);
        }
        return summary;
    }
    
    /**
     * @brief Get average value over the time period
     */
    double get_average() const {
        if (points.empty()) return 0.0;
        
        double sum = 0.0;
        uint64_t total_weight = 0;
        
        for (const auto& point : points) {
            sum += point.value * point.sample_count;
            total_weight += point.sample_count;
        }
        
        return total_weight > 0 ? sum / total_weight : 0.0;
    }
    
    /**
     * @brief Get rate of change (per second)
     */
    double get_rate() const {
        if (points.size() < 2) return 0.0;
        
        const auto& first = points.front();
        const auto& last = points.back();
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            last.timestamp - first.timestamp);
        
        if (duration.count() <= 0) return 0.0;
        
        return (last.value - first.value) / duration.count();
    }
};

/**
 * @class time_series
 * @brief Thread-safe time series data storage
 */
class time_series {
private:
    mutable std::mutex mutex_;
    std::vector<time_point_data> data_;
    time_series_config config_;
    std::string series_name_;
    
    /**
     * @brief Cleanup old data points
     */
    void cleanup_old_data() {
        auto cutoff = std::chrono::system_clock::now() - config_.retention_period;
        
        auto it = std::remove_if(data_.begin(), data_.end(),
            [cutoff](const time_point_data& point) {
                return !point.is_valid(cutoff);
            });
        
        data_.erase(it, data_.end());
    }
    
    /**
     * @brief Compress data if enabled
     */
    void compress_data() {
        if (!config_.enable_compression || data_.size() < 3) {
            return;
        }
        
        // Simple compression: remove points that don't add significant information
        std::vector<time_point_data> compressed;
        compressed.reserve(data_.size());
        
        if (!data_.empty()) {
            compressed.push_back(data_[0]);
            
            for (size_t i = 1; i < data_.size() - 1; ++i) {
                const auto& prev = data_[i - 1];
                const auto& curr = data_[i];
                const auto& next = data_[i + 1];
                
                // Check if current point is significantly different from linear interpolation
                double expected = prev.value + (next.value - prev.value) * 
                    (std::chrono::duration<double>(curr.timestamp - prev.timestamp).count() /
                     std::chrono::duration<double>(next.timestamp - prev.timestamp).count());
                
                if (std::abs(curr.value - expected) > config_.compression_threshold) {
                    compressed.push_back(curr);
                }
            }
            
            if (data_.size() > 1) {
                compressed.push_back(data_.back());
            }
        }
        
        data_ = std::move(compressed);
    }
    
    /**
     * @brief Ensure data size doesn't exceed maximum
     */
    void enforce_size_limit() {
        if (data_.size() > config_.max_points) {
            size_t remove_count = data_.size() - config_.max_points;
            data_.erase(data_.begin(), data_.begin() + remove_count);
        }
    }
    
public:
    /**
     * @brief Constructor
     */
    explicit time_series(const std::string& name, 
                        const time_series_config& config = {})
        : config_(config), series_name_(name) {
        
        auto validation = config_.validate();
        if (!validation) {
            throw std::invalid_argument("Invalid time series configuration: " + 
                                      validation.get_error().message);
        }
        
        data_.reserve(config_.max_points);
    }
    
    /**
     * @brief Add a data point
     */
    result_void add_point(double value, 
                         std::chrono::system_clock::time_point timestamp = 
                         std::chrono::system_clock::now()) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        time_point_data point(timestamp, value);
        
        // Insert in chronological order
        auto it = std::upper_bound(data_.begin(), data_.end(), point,
            [](const time_point_data& a, const time_point_data& b) {
                return a.timestamp < b.timestamp;
            });
        
        data_.insert(it, point);
        
        // Perform maintenance
        cleanup_old_data();
        compress_data();
        enforce_size_limit();
        
        return result_void::success();
    }
    
    /**
     * @brief Add multiple data points
     */
    result_void add_points(const std::vector<time_point_data>& points) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& point : points) {
            auto it = std::upper_bound(data_.begin(), data_.end(), point,
                [](const time_point_data& a, const time_point_data& b) {
                    return a.timestamp < b.timestamp;
                });
            
            data_.insert(it, point);
        }
        
        // Perform maintenance
        cleanup_old_data();
        compress_data();
        enforce_size_limit();
        
        return result_void::success();
    }
    
    /**
     * @brief Query data for a time range
     */
    result<aggregation_result> query(const time_series_query& query) const {
        auto validation = query.validate();
        if (!validation) {
            return make_error<aggregation_result>(monitoring_error_code::invalid_argument,
                                                validation.get_error().message);
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        aggregation_result result;
        result.query_start = query.start_time;
        result.query_end = query.end_time;
        
        // Find data points in the time range
        auto start_it = std::lower_bound(data_.begin(), data_.end(), query.start_time,
            [](const time_point_data& point, std::chrono::system_clock::time_point time) {
                return point.timestamp < time;
            });
        
        auto end_it = std::upper_bound(data_.begin(), data_.end(), query.end_time,
            [](std::chrono::system_clock::time_point time, const time_point_data& point) {
                return time < point.timestamp;
            });
        
        if (start_it == end_it) {
            return make_success(std::move(result));  // No data in range
        }
        
        // Aggregate data by step size
        auto current_step_start = query.start_time;
        
        while (current_step_start < query.end_time) {
            auto current_step_end = current_step_start + query.step;
            if (current_step_end > query.end_time) {
                current_step_end = query.end_time;
            }
            
            // Find points in this step
            std::vector<time_point_data> step_points;
            for (auto it = start_it; it != end_it; ++it) {
                if (it->timestamp >= current_step_start && it->timestamp < current_step_end) {
                    step_points.push_back(*it);
                    result.total_samples += it->sample_count;
                }
            }
            
            // Aggregate points in this step
            if (!step_points.empty()) {
                time_point_data aggregated_point;
                aggregated_point.timestamp = current_step_start + query.step / 2;  // Middle of step
                
                for (const auto& point : step_points) {
                    aggregated_point.merge(point);
                }
                
                result.points.push_back(aggregated_point);
            }
            
            current_step_start = current_step_end;
        }
        
        return make_success(std::move(result));
    }
    
    /**
     * @brief Get current number of data points
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.size();
    }
    
    /**
     * @brief Check if series is empty
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.empty();
    }
    
    /**
     * @brief Get series name
     */
    const std::string& name() const noexcept {
        return series_name_;
    }
    
    /**
     * @brief Get configuration
     */
    const time_series_config& get_config() const noexcept {
        return config_;
    }
    
    /**
     * @brief Clear all data
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.clear();
    }
    
    /**
     * @brief Get latest value
     */
    result<double> get_latest_value() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (data_.empty()) {
            return make_error<double>(monitoring_error_code::collection_failed,
                                    "No data available");
        }
        
        return make_success(data_.back().value);
    }
    
    /**
     * @brief Get memory footprint in bytes
     */
    size_t memory_footprint() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sizeof(time_series) + 
               data_.capacity() * sizeof(time_point_data) +
               series_name_.capacity();
    }
};

/**
 * @brief Helper function to create a time series with default configuration
 */
inline std::unique_ptr<time_series> make_time_series(const std::string& name) {
    return std::make_unique<time_series>(name);
}

/**
 * @brief Helper function to create a time series with custom configuration
 */
inline std::unique_ptr<time_series> make_time_series(const std::string& name,
                                                    const time_series_config& config) {
    return std::make_unique<time_series>(name, config);
}

} // namespace monitoring_system