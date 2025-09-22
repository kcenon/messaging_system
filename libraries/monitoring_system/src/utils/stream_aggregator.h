#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file stream_aggregator.h
 * @brief Statistical aggregation functions for stream processing
 * 
 * This file implements P2 task: Statistical aggregation functions
 * for real-time stream processing with memory-efficient algorithms.
 */

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <kcenon/monitoring/utils/metric_types.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <deque>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <atomic>
#include <mutex>

namespace monitoring_system {

/**
 * @struct stream_aggregator_config
 * @brief Configuration for stream aggregation
 */
struct stream_aggregator_config {
    size_t window_size = 1000;                           // Moving window size
    std::chrono::milliseconds window_duration{60000};    // Window duration (1 minute)
    double percentile_precision = 0.01;                  // Precision for percentile calculation
    size_t max_unique_values = 10000;                    // Max unique values for cardinality
    bool enable_outlier_detection = true;                // Enable outlier detection
    double outlier_threshold = 3.0;                      // Standard deviations for outliers
    
    /**
     * @brief Validate configuration
     */
    result_void validate() const {
        if (window_size == 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Window size must be positive");
        }
        
        if (window_duration.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Window duration must be positive");
        }
        
        if (percentile_precision <= 0 || percentile_precision >= 1) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Percentile precision must be between 0 and 1");
        }
        
        return result_void::success();
    }
};

/**
 * @struct stream_statistics
 * @brief Comprehensive streaming statistics
 */
struct stream_statistics {
    // Basic statistics
    uint64_t count = 0;
    double sum = 0.0;
    double mean = 0.0;
    double variance = 0.0;
    double std_deviation = 0.0;
    double min_value = std::numeric_limits<double>::max();
    double max_value = std::numeric_limits<double>::lowest();
    
    // Advanced statistics
    double skewness = 0.0;           // Measure of asymmetry
    double kurtosis = 0.0;           // Measure of tail heaviness
    double median = 0.0;             // 50th percentile
    double q1 = 0.0;                 // 25th percentile
    double q3 = 0.0;                 // 75th percentile
    double iqr = 0.0;                // Interquartile range
    
    // Percentiles
    std::unordered_map<double, double> percentiles;
    
    // Rate statistics
    double rate_per_second = 0.0;
    double rate_per_minute = 0.0;
    
    // Outlier statistics
    uint64_t outlier_count = 0;
    std::vector<double> outliers;
    
    // Time information
    std::chrono::system_clock::time_point first_timestamp;
    std::chrono::system_clock::time_point last_timestamp;
    std::chrono::milliseconds window_duration{0};
    
    /**
     * @brief Reset all statistics
     */
    void reset() {
        *this = stream_statistics{};
    }
    
    /**
     * @brief Get coefficient of variation
     */
    double coefficient_of_variation() const {
        return mean != 0.0 ? std_deviation / std::abs(mean) : 0.0;
    }
    
    /**
     * @brief Check if distribution is normal (rough heuristic)
     */
    bool is_approximately_normal() const {
        // Simple check: skewness close to 0, kurtosis close to 3
        return std::abs(skewness) < 1.0 && std::abs(kurtosis - 3.0) < 2.0;
    }
};

/**
 * @class online_statistics
 * @brief Online algorithm for computing statistics
 */
class online_statistics {
private:
    uint64_t n_ = 0;
    double mean_ = 0.0;
    double m2_ = 0.0;    // Sum of squares of differences from mean
    double m3_ = 0.0;    // Third moment
    double m4_ = 0.0;    // Fourth moment
    double min_ = std::numeric_limits<double>::max();
    double max_ = std::numeric_limits<double>::lowest();
    double sum_ = 0.0;
    
public:
    /**
     * @brief Add a value to the statistics
     */
    void add_value(double value) {
        n_++;
        sum_ += value;
        min_ = std::min(min_, value);
        max_ = std::max(max_, value);
        
        // Welford's online algorithm for variance
        double delta = value - mean_;
        mean_ += delta / n_;
        double delta2 = value - mean_;
        m2_ += delta * delta2;
        
        // Online algorithm for higher moments
        double n = static_cast<double>(n_);
        if (n_ >= 2) {
            double delta_n = delta / n;
            double delta_n2 = delta_n * delta_n;
            double term1 = delta * delta2 * (n - 1);
            
            m4_ += term1 * delta_n2 * (n * n - 3 * n + 3) + 6 * delta_n2 * m2_ - 4 * delta_n * m3_;
            m3_ += term1 * delta_n * (n - 2) - 3 * delta_n * m2_;
        }
    }
    
    /**
     * @brief Get current statistics
     */
    stream_statistics get_statistics() const {
        stream_statistics stats;
        stats.count = n_;
        stats.sum = sum_;
        stats.mean = mean_;
        stats.min_value = n_ > 0 ? min_ : 0.0;
        stats.max_value = n_ > 0 ? max_ : 0.0;
        
        if (n_ >= 2) {
            stats.variance = m2_ / (n_ - 1);
            stats.std_deviation = std::sqrt(stats.variance);
            
            if (stats.variance > 0) {
                double n = static_cast<double>(n_);
                stats.skewness = std::sqrt(n * (n - 1)) / (n - 2) * m3_ / std::pow(stats.variance, 1.5);
                stats.kurtosis = (n - 1) / ((n - 2) * (n - 3)) * 
                                ((n + 1) * m4_ / (stats.variance * stats.variance) - 3 * (n - 1));
            }
        }
        
        return stats;
    }
    
    /**
     * @brief Reset statistics
     */
    void reset() {
        n_ = 0;
        mean_ = 0.0;
        m2_ = 0.0;
        m3_ = 0.0;
        m4_ = 0.0;
        min_ = std::numeric_limits<double>::max();
        max_ = std::numeric_limits<double>::lowest();
        sum_ = 0.0;
    }
    
    /**
     * @brief Get sample count
     */
    uint64_t count() const noexcept { return n_; }
    
    /**
     * @brief Get current mean
     */
    double mean() const noexcept { return mean_; }
    
    /**
     * @brief Get current variance
     */
    double variance() const noexcept {
        return n_ >= 2 ? m2_ / (n_ - 1) : 0.0;
    }
};

/**
 * @class quantile_estimator
 * @brief P² algorithm for quantile estimation
 */
class quantile_estimator {
private:
    std::vector<double> q_;     // Quantile values
    std::vector<double> dn_;    // Desired positions
    std::vector<double> np_;    // Actual positions
    std::vector<int> n_;        // Marker positions
    bool initialized_ = false;
    double p_;                  // Target quantile
    
    /**
     * @brief Parabolic prediction
     */
    double parabolic(int i, int d) const {
        return q_[i] + d / (n_[i + 1] - n_[i - 1]) * 
               ((n_[i] - n_[i - 1] + d) * (q_[i + 1] - q_[i]) / (n_[i + 1] - n_[i]) +
                (n_[i + 1] - n_[i] - d) * (q_[i] - q_[i - 1]) / (n_[i] - n_[i - 1]));
    }
    
    /**
     * @brief Linear prediction
     */
    double linear(int i, int d) const {
        return q_[i] + d * (q_[i + d] - q_[i]) / (n_[i + d] - n_[i]);
    }
    
public:
    /**
     * @brief Constructor
     */
    explicit quantile_estimator(double p) : p_(p) {
        q_.resize(5);
        dn_.resize(5);
        np_.resize(5);
        n_.resize(5);
        
        // Initialize desired positions
        dn_[0] = 0;
        dn_[1] = p / 2;
        dn_[2] = p;
        dn_[3] = (1 + p) / 2;
        dn_[4] = 1;
    }
    
    /**
     * @brief Add observation
     */
    void add_observation(double x) {
        if (!initialized_) {
            if (q_.size() < 5) {
                q_.push_back(x);
            }
            
            if (q_.size() == 5) {
                std::sort(q_.begin(), q_.end());
                for (int i = 0; i < 5; i++) {
                    n_[i] = i;
                    np_[i] = i;
                }
                initialized_ = true;
            }
            return;
        }
        
        // Find cell k
        int k = 0;
        if (x < q_[0]) {
            q_[0] = x;
            k = 0;
        } else if (x >= q_[4]) {
            q_[4] = x;
            k = 3;
        } else {
            for (int i = 1; i < 5; i++) {
                if (x < q_[i]) {
                    k = i - 1;
                    break;
                }
            }
        }
        
        // Increment positions
        for (int i = k + 1; i < 5; i++) {
            n_[i]++;
        }
        
        // Update desired positions
        for (int i = 0; i < 5; i++) {
            np_[i] += dn_[i];
        }
        
        // Adjust heights
        for (int i = 1; i < 4; i++) {
            double d = np_[i] - n_[i];
            if ((d >= 1 && n_[i + 1] - n_[i] > 1) || 
                (d <= -1 && n_[i - 1] - n_[i] < -1)) {
                
                int di = d >= 0 ? 1 : -1;
                double qnew = parabolic(i, di);
                
                if (q_[i - 1] < qnew && qnew < q_[i + 1]) {
                    q_[i] = qnew;
                } else {
                    q_[i] = linear(i, di);
                }
                
                n_[i] += di;
            }
        }
    }
    
    /**
     * @brief Get quantile estimate
     */
    double get_quantile() const {
        return initialized_ ? q_[2] : 0.0;
    }
    
    /**
     * @brief Reset estimator
     */
    void reset() {
        q_.clear();
        q_.resize(5);
        initialized_ = false;
    }
};

/**
 * @class moving_window_aggregator
 * @brief Sliding window aggregator for time-based statistics
 */
template<typename T>
class moving_window_aggregator {
private:
    struct timestamped_value {
        T value;
        std::chrono::system_clock::time_point timestamp;
        
        timestamped_value(T v, std::chrono::system_clock::time_point ts)
            : value(v), timestamp(ts) {}
    };
    
    std::deque<timestamped_value> window_;
    std::chrono::milliseconds window_duration_;
    size_t max_size_;
    mutable std::mutex mutex_;
    
    /**
     * @brief Remove expired values
     */
    void cleanup_expired() {
        auto now = std::chrono::system_clock::now();
        auto cutoff = now - window_duration_;
        
        while (!window_.empty() && window_.front().timestamp < cutoff) {
            window_.pop_front();
        }
        
        while (window_.size() > max_size_) {
            window_.pop_front();
        }
    }
    
public:
    /**
     * @brief Constructor
     */
    moving_window_aggregator(std::chrono::milliseconds duration, size_t max_size)
        : window_duration_(duration), max_size_(max_size) {}
    
    /**
     * @brief Add value to window
     */
    void add_value(const T& value, 
                   std::chrono::system_clock::time_point timestamp = 
                   std::chrono::system_clock::now()) {
        std::lock_guard<std::mutex> lock(mutex_);
        window_.emplace_back(value, timestamp);
        cleanup_expired();
    }
    
    /**
     * @brief Get values in window
     */
    std::vector<T> get_values() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<T> values;
        values.reserve(window_.size());
        
        for (const auto& item : window_) {
            values.push_back(item.value);
        }
        
        return values;
    }
    
    /**
     * @brief Get window size
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return window_.size();
    }
    
    /**
     * @brief Clear window
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        window_.clear();
    }
    
    /**
     * @brief Get rate per second
     */
    double get_rate_per_second() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (window_.empty()) return 0.0;
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            window_.back().timestamp - window_.front().timestamp);
        
        return duration.count() > 0 ? 
               static_cast<double>(window_.size()) / duration.count() : 0.0;
    }
};

/**
 * @class stream_aggregator
 * @brief Main class for statistical stream aggregation
 */
class stream_aggregator {
private:
    stream_aggregator_config config_;
    online_statistics online_stats_;
    moving_window_aggregator<double> window_;
    std::unordered_map<double, std::unique_ptr<quantile_estimator>> percentile_estimators_;
    std::vector<double> outliers_;
    mutable std::mutex mutex_;
    
    std::chrono::system_clock::time_point first_timestamp_;
    std::chrono::system_clock::time_point last_timestamp_;
    
    /**
     * @brief Initialize percentile estimators
     */
    void initialize_percentile_estimators() {
        std::vector<double> percentiles = {0.01, 0.05, 0.1, 0.25, 0.5, 0.75, 0.9, 0.95, 0.99};
        
        for (double p : percentiles) {
            percentile_estimators_[p] = std::make_unique<quantile_estimator>(p);
        }
    }
    
    /**
     * @brief Check if value is outlier
     */
    bool is_outlier(double value) const {
        if (!config_.enable_outlier_detection || online_stats_.count() < 10) {
            return false;
        }
        
        double mean = online_stats_.mean();
        double std_dev = std::sqrt(online_stats_.variance());
        
        return std::abs(value - mean) > config_.outlier_threshold * std_dev;
    }
    
public:
    /**
     * @brief Constructor
     */
    explicit stream_aggregator(const stream_aggregator_config& config = {})
        : config_(config)
        , window_(config.window_duration, config.window_size) {
        
        auto validation = config_.validate();
        if (!validation) {
            throw std::invalid_argument("Invalid stream aggregator configuration: " + 
                                      validation.get_error().message);
        }
        
        initialize_percentile_estimators();
    }
    
    /**
     * @brief Add observation to stream
     */
    result_void add_observation(double value, 
                               std::chrono::system_clock::time_point timestamp = 
                               std::chrono::system_clock::now()) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Update timestamps
        if (online_stats_.count() == 0) {
            first_timestamp_ = timestamp;
        }
        last_timestamp_ = timestamp;
        
        // Check for outliers
        if (is_outlier(value)) {
            outliers_.push_back(value);
        }
        
        // Update statistics
        online_stats_.add_value(value);
        window_.add_value(value, timestamp);
        
        // Update percentile estimators
        for (auto& [p, estimator] : percentile_estimators_) {
            estimator->add_observation(value);
        }
        
        return result_void::success();
    }
    
    /**
     * @brief Get comprehensive statistics
     */
    stream_statistics get_statistics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto stats = online_stats_.get_statistics();
        
        // Add percentiles
        for (const auto& [p, estimator] : percentile_estimators_) {
            stats.percentiles[p] = estimator->get_quantile();
        }
        
        // Set common percentiles
        if (stats.percentiles.count(0.25)) stats.q1 = stats.percentiles[0.25];
        if (stats.percentiles.count(0.5)) stats.median = stats.percentiles[0.5];
        if (stats.percentiles.count(0.75)) stats.q3 = stats.percentiles[0.75];
        stats.iqr = stats.q3 - stats.q1;
        
        // Set time information
        stats.first_timestamp = first_timestamp_;
        stats.last_timestamp = last_timestamp_;
        
        if (stats.count > 1) {
            stats.window_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                last_timestamp_ - first_timestamp_);
                
            double seconds = stats.window_duration.count() / 1000.0;
            if (seconds > 0) {
                stats.rate_per_second = static_cast<double>(stats.count) / seconds;
                stats.rate_per_minute = stats.rate_per_second * 60.0;
            }
        }
        
        // Outlier information
        stats.outlier_count = outliers_.size();
        stats.outliers = outliers_;
        
        return stats;
    }
    
    /**
     * @brief Get specific percentile
     */
    result<double> get_percentile(double p) const {
        if (p < 0.0 || p > 1.0) {
            return make_error<double>(monitoring_error_code::invalid_argument,
                                    "Percentile must be between 0 and 1");
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = percentile_estimators_.find(p);
        if (it != percentile_estimators_.end()) {
            return make_success(it->second->get_quantile());
        }
        
        return make_error<double>(monitoring_error_code::collector_not_found,
                                "Percentile estimator not found");
    }
    
    /**
     * @brief Reset all statistics
     */
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        online_stats_.reset();
        window_.clear();
        outliers_.clear();
        
        for (auto& [p, estimator] : percentile_estimators_) {
            estimator->reset();
        }
    }
    
    /**
     * @brief Get current sample count
     */
    uint64_t count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return online_stats_.count();
    }
    
    /**
     * @brief Get current mean
     */
    double mean() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return online_stats_.mean();
    }
    
    /**
     * @brief Get current variance
     */
    double variance() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return online_stats_.variance();
    }
    
    /**
     * @brief Get configuration
     */
    const stream_aggregator_config& get_config() const noexcept {
        return config_;
    }
};

/**
 * @brief Helper function to create stream aggregator with default configuration
 */
inline std::unique_ptr<stream_aggregator> make_stream_aggregator() {
    return std::make_unique<stream_aggregator>();
}

/**
 * @brief Helper function to create stream aggregator with custom configuration
 */
inline std::unique_ptr<stream_aggregator> make_stream_aggregator(const stream_aggregator_config& config) {
    return std::make_unique<stream_aggregator>(config);
}

/**
 * @brief Calculate Pearson correlation coefficient between two series
 */
inline double pearson_correlation(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2) {
        return 0.0;
    }
    
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
    size_t n = x.size();
    
    for (size_t i = 0; i < n; ++i) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
        sum_y2 += y[i] * y[i];
    }
    
    double numerator = n * sum_xy - sum_x * sum_y;
    double denominator = std::sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));
    
    return denominator != 0.0 ? numerator / denominator : 0.0;
}

} // namespace monitoring_system