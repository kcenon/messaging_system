/**
 * @file adaptive_monitor.h
 * @brief Adaptive monitoring implementation that adjusts behavior based on system load
 * @date 2025
 * 
 * Provides adaptive monitoring capabilities that automatically adjust
 * collection intervals, sampling rates, and metric granularity based
 * on current system resource utilization.
 */

#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <functional>
#include <thread>
#include <cmath>
#include <random>

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <kcenon/monitoring/interfaces/monitoring_interface.h>
#include <kcenon/monitoring/core/performance_monitor.h>

namespace monitoring_system {

/**
 * @brief Adaptation strategy for monitoring behavior
 */
enum class adaptation_strategy {
    conservative,  // Prefer system stability over monitoring detail
    balanced,      // Balance between monitoring and performance
    aggressive     // Prefer monitoring detail over system resources
};

/**
 * @brief System load levels
 */
enum class load_level {
    idle,          // < 20% CPU
    low,           // 20-40% CPU
    moderate,      // 40-60% CPU
    high,          // 60-80% CPU
    critical       // > 80% CPU
};

/**
 * @brief Adaptive configuration parameters
 */
struct adaptive_config {
    // Thresholds for load levels (CPU percentage)
    double idle_threshold{20.0};
    double low_threshold{40.0};
    double moderate_threshold{60.0};
    double high_threshold{80.0};
    
    // Memory thresholds (percentage)
    double memory_warning_threshold{70.0};
    double memory_critical_threshold{85.0};
    
    // Collection intervals by load level (milliseconds)
    std::chrono::milliseconds idle_interval{100};
    std::chrono::milliseconds low_interval{250};
    std::chrono::milliseconds moderate_interval{500};
    std::chrono::milliseconds high_interval{1000};
    std::chrono::milliseconds critical_interval{5000};
    
    // Sampling rates by load level (0.0 to 1.0)
    double idle_sampling_rate{1.0};
    double low_sampling_rate{0.8};
    double moderate_sampling_rate{0.5};
    double high_sampling_rate{0.2};
    double critical_sampling_rate{0.1};
    
    // Adaptation parameters
    adaptation_strategy strategy{adaptation_strategy::balanced};
    std::chrono::seconds adaptation_interval{10};
    double smoothing_factor{0.7};  // Exponential smoothing for load average
    
    /**
     * @brief Get collection interval for load level
     */
    std::chrono::milliseconds get_interval_for_load(load_level level) const {
        switch (level) {
            case load_level::idle: return idle_interval;
            case load_level::low: return low_interval;
            case load_level::moderate: return moderate_interval;
            case load_level::high: return high_interval;
            case load_level::critical: return critical_interval;
        }
        return moderate_interval;
    }
    
    /**
     * @brief Get sampling rate for load level
     */
    double get_sampling_rate_for_load(load_level level) const {
        switch (level) {
            case load_level::idle: return idle_sampling_rate;
            case load_level::low: return low_sampling_rate;
            case load_level::moderate: return moderate_sampling_rate;
            case load_level::high: return high_sampling_rate;
            case load_level::critical: return critical_sampling_rate;
        }
        return moderate_sampling_rate;
    }
};

/**
 * @brief Adaptation statistics
 */
struct adaptation_stats {
    std::uint64_t total_adaptations{0};
    std::uint64_t upscale_count{0};
    std::uint64_t downscale_count{0};
    std::uint64_t samples_dropped{0};
    std::uint64_t samples_collected{0};
    double average_cpu_usage{0.0};
    double average_memory_usage{0.0};
    load_level current_load_level{load_level::moderate};
    std::chrono::milliseconds current_interval;
    double current_sampling_rate{1.0};
    std::chrono::system_clock::time_point last_adaptation;
};

/**
 * @brief Adaptive collector wrapper
 */
class adaptive_collector {
private:
    std::shared_ptr<monitoring_system::metrics_collector> collector_;
    adaptive_config config_;
    adaptation_stats stats_;
    std::atomic<bool> enabled_{true};
    std::atomic<double> current_sampling_rate_{1.0};
    mutable std::mutex stats_mutex_;
    
public:
    adaptive_collector(
        std::shared_ptr<monitoring_system::metrics_collector> collector,
        const adaptive_config& config = {}
    ) : collector_(collector), config_(config) {
        stats_.current_interval = config_.moderate_interval;
        stats_.last_adaptation = std::chrono::system_clock::now();
    }
    
    /**
     * @brief Collect metrics with adaptive sampling
     */
    monitoring_system::result<monitoring_system::metrics_snapshot> collect() {
        if (!should_sample()) {
            stats_.samples_dropped++;
            return monitoring_system::make_error<monitoring_system::metrics_snapshot>(
                monitoring_system::monitoring_error_code::operation_cancelled,
                "Sample dropped due to adaptive sampling"
            );
        }
        
        stats_.samples_collected++;
        return collector_->collect();
    }
    
    /**
     * @brief Adapt collection behavior based on load
     */
    void adapt(const monitoring_system::system_metrics& sys_metrics) {
        std::lock_guard lock(stats_mutex_);
        
        // Initialize averages on first adaptation
        if (stats_.total_adaptations == 0) {
            stats_.average_cpu_usage = sys_metrics.cpu_usage_percent;
            stats_.average_memory_usage = sys_metrics.memory_usage_percent;
        } else {
            // Update average metrics using exponential smoothing
            stats_.average_cpu_usage = 
                config_.smoothing_factor * sys_metrics.cpu_usage_percent +
                (1.0 - config_.smoothing_factor) * stats_.average_cpu_usage;
            
            stats_.average_memory_usage = 
                config_.smoothing_factor * sys_metrics.memory_usage_percent +
                (1.0 - config_.smoothing_factor) * stats_.average_memory_usage;
        }
        
        // Determine load level
        auto new_level = calculate_load_level(
            stats_.average_cpu_usage,
            stats_.average_memory_usage
        );
        
        // Adapt if load level changed
        if (new_level != stats_.current_load_level) {
            if (new_level > stats_.current_load_level) {
                stats_.downscale_count++;
            } else {
                stats_.upscale_count++;
            }
            
            stats_.current_load_level = new_level;
            stats_.current_interval = config_.get_interval_for_load(new_level);
            current_sampling_rate_ = config_.get_sampling_rate_for_load(new_level);
            stats_.current_sampling_rate = current_sampling_rate_;
            stats_.total_adaptations++;
            stats_.last_adaptation = std::chrono::system_clock::now();
        }
    }
    
    /**
     * @brief Get current adaptation statistics
     */
    adaptation_stats get_stats() const {
        std::lock_guard lock(stats_mutex_);
        return stats_;
    }
    
    /**
     * @brief Get current collection interval
     */
    std::chrono::milliseconds get_current_interval() const {
        std::lock_guard lock(stats_mutex_);
        return stats_.current_interval;
    }
    
    /**
     * @brief Set adaptive configuration
     */
    void set_config(const adaptive_config& config) {
        config_ = config;
    }
    
    /**
     * @brief Get adaptive configuration
     */
    adaptive_config get_config() const {
        return config_;
    }
    
    /**
     * @brief Enable or disable adaptive behavior
     */
    void set_enabled(bool enabled) {
        enabled_ = enabled;
    }
    
    /**
     * @brief Check if adaptive behavior is enabled
     */
    bool is_enabled() const {
        return enabled_;
    }
    
private:
    /**
     * @brief Determine if current sample should be collected
     */
    bool should_sample() const {
        if (!enabled_) return true;
        
        // Use random sampling based on current rate
        static thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen) < current_sampling_rate_.load();
    }
    
    /**
     * @brief Calculate load level from metrics
     */
    load_level calculate_load_level(double cpu_usage, double memory_usage) const {
        // Consider memory pressure in load calculation
        double effective_load = cpu_usage;
        
        // Memory pressure should escalate load level
        if (memory_usage > config_.memory_critical_threshold) {
            // Critical memory -> at least high load
            effective_load = std::max(effective_load, config_.high_threshold + 1.0);
        } else if (memory_usage > config_.memory_warning_threshold) {
            // Warning memory -> at least moderate load
            effective_load = std::max(effective_load, config_.moderate_threshold + 1.0);
        }
        
        // Apply strategy-specific adjustments BEFORE determining level
        switch (config_.strategy) {
            case adaptation_strategy::conservative:
                effective_load *= 0.8;  // Be more conservative
                break;
            case adaptation_strategy::aggressive:
                effective_load *= 1.2;  // Be more aggressive
                break;
            case adaptation_strategy::balanced:
            default:
                break;
        }
        
        // Determine load level
        if (effective_load >= config_.high_threshold) {
            return load_level::critical;
        } else if (effective_load >= config_.moderate_threshold) {
            return load_level::high;
        } else if (effective_load >= config_.low_threshold) {
            return load_level::moderate;
        } else if (effective_load >= config_.idle_threshold) {
            return load_level::low;
        } else {
            return load_level::idle;
        }
    }
};

/**
 * @brief Adaptive monitoring controller
 */
class adaptive_monitor {
private:
    struct monitor_impl;
    std::unique_ptr<monitor_impl> impl_;
    
public:
    adaptive_monitor();
    ~adaptive_monitor();
    
    // Disable copy
    adaptive_monitor(const adaptive_monitor&) = delete;
    adaptive_monitor& operator=(const adaptive_monitor&) = delete;
    
    // Enable move
    adaptive_monitor(adaptive_monitor&&) noexcept;
    adaptive_monitor& operator=(adaptive_monitor&&) noexcept;
    
    /**
     * @brief Register a collector for adaptive monitoring
     */
    monitoring_system::result<bool> register_collector(
        const std::string& name,
        std::shared_ptr<monitoring_system::metrics_collector> collector,
        const adaptive_config& config = {}
    );
    
    /**
     * @brief Unregister a collector
     */
    monitoring_system::result<bool> unregister_collector(const std::string& name);
    
    /**
     * @brief Start adaptive monitoring
     */
    monitoring_system::result<bool> start();
    
    /**
     * @brief Stop adaptive monitoring
     */
    monitoring_system::result<bool> stop();
    
    /**
     * @brief Check if monitoring is active
     */
    bool is_running() const;
    
    /**
     * @brief Get adaptation statistics for a collector
     */
    monitoring_system::result<adaptation_stats> get_collector_stats(
        const std::string& name
    ) const;
    
    /**
     * @brief Get all collector statistics
     */
    std::unordered_map<std::string, adaptation_stats> get_all_stats() const;
    
    /**
     * @brief Set global adaptation strategy
     */
    void set_global_strategy(adaptation_strategy strategy);
    
    /**
     * @brief Force adaptation cycle
     */
    monitoring_system::result<bool> force_adaptation();
    
    /**
     * @brief Get recommended collectors based on load
     */
    std::vector<std::string> get_active_collectors() const;
    
    /**
     * @brief Set priority for a collector (higher priority = keep active longer)
     */
    monitoring_system::result<bool> set_collector_priority(
        const std::string& name,
        int priority
    );
};

/**
 * @brief Global adaptive monitor instance
 */
adaptive_monitor& global_adaptive_monitor();

/**
 * @brief Adaptive monitoring scope
 */
class adaptive_scope {
private:
    adaptive_monitor* monitor_;
    std::string collector_name_;
    bool registered_{false};
    
public:
    adaptive_scope(
        const std::string& name,
        std::shared_ptr<monitoring_system::metrics_collector> collector,
        const adaptive_config& config = {}
    ) : monitor_(&global_adaptive_monitor()), collector_name_(name) {
        auto result = monitor_->register_collector(name, collector, config);
        registered_ = result.has_value() && result.value();
    }
    
    ~adaptive_scope() {
        if (registered_ && monitor_) {
            monitor_->unregister_collector(collector_name_);
        }
    }
    
    // Disable copy
    adaptive_scope(const adaptive_scope&) = delete;
    adaptive_scope& operator=(const adaptive_scope&) = delete;
    
    // Enable move
    adaptive_scope(adaptive_scope&& other) noexcept
        : monitor_(other.monitor_)
        , collector_name_(std::move(other.collector_name_))
        , registered_(other.registered_) {
        other.monitor_ = nullptr;
        other.registered_ = false;
    }
    
    adaptive_scope& operator=(adaptive_scope&& other) noexcept {
        if (this != &other) {
            if (registered_ && monitor_) {
                monitor_->unregister_collector(collector_name_);
            }
            monitor_ = other.monitor_;
            collector_name_ = std::move(other.collector_name_);
            registered_ = other.registered_;
            other.monitor_ = nullptr;
            other.registered_ = false;
        }
        return *this;
    }
    
    bool is_registered() const { return registered_; }
};

} // namespace monitoring_system