/**
 * @file performance_monitor.h
 * @brief Performance monitoring and profiling implementation
 * @date 2025
 * 
 * Provides performance monitoring capabilities including CPU, memory,
 * and custom performance metrics collection with minimal overhead.
 */

#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>
#include <thread>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <shared_mutex>

#include "../core/result_types.h"
#include "../core/error_codes.h"
#include "../interfaces/monitoring_interface.h"

namespace monitoring_system {

/**
 * @brief Performance metrics for a specific operation
 */
struct performance_metrics {
    std::string operation_name;
    std::chrono::nanoseconds min_duration{std::chrono::nanoseconds::max()};
    std::chrono::nanoseconds max_duration{std::chrono::nanoseconds::zero()};
    std::chrono::nanoseconds total_duration{std::chrono::nanoseconds::zero()};
    std::chrono::nanoseconds mean_duration{std::chrono::nanoseconds::zero()};
    std::chrono::nanoseconds median_duration{std::chrono::nanoseconds::zero()};
    std::chrono::nanoseconds p95_duration{std::chrono::nanoseconds::zero()};
    std::chrono::nanoseconds p99_duration{std::chrono::nanoseconds::zero()};
    std::uint64_t call_count{0};
    std::uint64_t error_count{0};
    double throughput{0.0};  // Operations per second
    
    /**
     * @brief Calculate percentile from sorted durations
     */
    static std::chrono::nanoseconds calculate_percentile(
        const std::vector<std::chrono::nanoseconds>& sorted_durations,
        double percentile) {
        
        if (sorted_durations.empty()) {
            return std::chrono::nanoseconds::zero();
        }
        
        size_t index = static_cast<size_t>(
            (percentile / 100.0) * (sorted_durations.size() - 1)
        );
        return sorted_durations[index];
    }
    
    /**
     * @brief Update statistics with new duration samples
     */
    void update_statistics(const std::vector<std::chrono::nanoseconds>& durations) {
        if (durations.empty()) return;
        
        auto sorted = durations;
        std::sort(sorted.begin(), sorted.end());
        
        min_duration = sorted.front();
        max_duration = sorted.back();
        median_duration = calculate_percentile(sorted, 50.0);
        p95_duration = calculate_percentile(sorted, 95.0);
        p99_duration = calculate_percentile(sorted, 99.0);
        
        total_duration = std::accumulate(
            sorted.begin(), sorted.end(),
            std::chrono::nanoseconds::zero()
        );
        
        mean_duration = total_duration / sorted.size();
    }
};

/**
 * @brief System resource metrics
 */
struct system_metrics {
    double cpu_usage_percent{0.0};
    double memory_usage_percent{0.0};
    std::size_t memory_usage_bytes{0};
    std::size_t available_memory_bytes{0};
    std::uint32_t thread_count{0};
    std::uint32_t handle_count{0};
    double disk_io_read_rate{0.0};   // MB/s
    double disk_io_write_rate{0.0};  // MB/s
    double network_io_recv_rate{0.0}; // MB/s
    double network_io_send_rate{0.0}; // MB/s
    
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief Performance profiler for code sections
 */
class performance_profiler {
private:
    struct profile_data {
        std::vector<std::chrono::nanoseconds> samples;
        std::atomic<std::uint64_t> call_count{0};
        std::atomic<std::uint64_t> error_count{0};
        mutable std::mutex mutex;
    };
    
    std::unordered_map<std::string, std::unique_ptr<profile_data>> profiles_;
    mutable std::shared_mutex profiles_mutex_;
    std::atomic<bool> enabled_{true};
    std::size_t max_samples_per_operation_{10000};
    
public:
    /**
     * @brief Record a performance sample
     */
    monitoring_system::result<bool> record_sample(
        const std::string& operation_name,
        std::chrono::nanoseconds duration,
        bool success = true
    );
    
    /**
     * @brief Get performance metrics for an operation
     */
    monitoring_system::result<performance_metrics> get_metrics(
        const std::string& operation_name
    ) const;
    
    /**
     * @brief Get all performance metrics
     */
    std::vector<performance_metrics> get_all_metrics() const;
    
    /**
     * @brief Clear samples for an operation
     */
    monitoring_system::result<bool> clear_samples(const std::string& operation_name);
    
    /**
     * @brief Clear all samples
     */
    void clear_all_samples();
    
    /**
     * @brief Enable or disable profiling
     */
    void set_enabled(bool enabled) { enabled_ = enabled; }
    
    /**
     * @brief Check if profiling is enabled
     */
    bool is_enabled() const { return enabled_; }
    
    /**
     * @brief Set maximum samples per operation
     */
    void set_max_samples(std::size_t max_samples) {
        max_samples_per_operation_ = max_samples;
    }
};

/**
 * @brief Scoped performance timer
 */
class scoped_timer {
private:
    performance_profiler* profiler_;
    std::string operation_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
    bool success_{true};
    bool completed_{false};
    
public:
    scoped_timer(performance_profiler* profiler, const std::string& operation_name)
        : profiler_(profiler)
        , operation_name_(operation_name)
        , start_time_(std::chrono::high_resolution_clock::now()) {}
    
    ~scoped_timer() {
        if (!completed_ && profiler_) {
            complete();
        }
    }
    
    /**
     * @brief Mark the operation as failed
     */
    void mark_failed() { success_ = false; }
    
    /**
     * @brief Manually complete the timing
     */
    void complete() {
        if (completed_) return;
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time_
        );
        
        if (profiler_) {
            profiler_->record_sample(operation_name_, duration, success_);
        }
        
        completed_ = true;
    }
    
    /**
     * @brief Get elapsed time without completing
     */
    std::chrono::nanoseconds elapsed() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            now - start_time_
        );
    }
};

/**
 * @brief System resource monitor
 */
class system_monitor {
private:
    struct monitor_impl;
    std::unique_ptr<monitor_impl> impl_;
    
public:
    system_monitor();
    ~system_monitor();
    
    // Disable copy
    system_monitor(const system_monitor&) = delete;
    system_monitor& operator=(const system_monitor&) = delete;
    
    // Enable move
    system_monitor(system_monitor&&) noexcept;
    system_monitor& operator=(system_monitor&&) noexcept;
    
    /**
     * @brief Get current system metrics
     */
    monitoring_system::result<system_metrics> get_current_metrics() const;
    
    /**
     * @brief Start monitoring system resources
     */
    monitoring_system::result<bool> start_monitoring(
        std::chrono::milliseconds interval = std::chrono::milliseconds(1000)
    );
    
    /**
     * @brief Stop monitoring
     */
    monitoring_system::result<bool> stop_monitoring();
    
    /**
     * @brief Check if monitoring is active
     */
    bool is_monitoring() const;
    
    /**
     * @brief Get historical metrics
     */
    std::vector<system_metrics> get_history(
        std::chrono::seconds duration = std::chrono::seconds(60)
    ) const;
};

/**
 * @brief Performance monitor combining profiling and system monitoring
 */
class performance_monitor : public metrics_collector {
private:
    performance_profiler profiler_;
    system_monitor system_monitor_;
    std::string name_;
    bool enabled_{true};
    
    // Performance thresholds for alerting
    struct thresholds {
        double cpu_threshold{80.0};
        double memory_threshold{90.0};
        std::chrono::milliseconds latency_threshold{1000};
    } thresholds_;
    
public:
    explicit performance_monitor(const std::string& name = "performance_monitor")
        : name_(name) {}
    
    // Implement metrics_collector interface
    std::string get_name() const override { return name_; }
    bool is_enabled() const override { return enabled_; }
    
    result_void set_enabled(bool enable) override {
        enabled_ = enable;
        profiler_.set_enabled(enable);
        return result_void::success();
    }
    
    result_void initialize() override {
        auto result = system_monitor_.start_monitoring();
        if (!result) {
            return result_void(result.get_error().code, result.get_error().message);
        }
        return result_void::success();
    }
    
    result_void cleanup() override {
        auto result = system_monitor_.stop_monitoring();
        if (!result) {
            return result_void(result.get_error().code, result.get_error().message);
        }
        return result_void::success();
    }
    
    monitoring_system::result<metrics_snapshot> collect() override;
    
    /**
     * @brief Create a scoped timer for an operation
     */
    scoped_timer time_operation(const std::string& operation_name) {
        return scoped_timer(&profiler_, operation_name);
    }
    
    /**
     * @brief Get performance profiler
     */
    performance_profiler& get_profiler() { return profiler_; }
    const performance_profiler& get_profiler() const { return profiler_; }
    
    /**
     * @brief Get system monitor
     */
    system_monitor& get_system_monitor() { return system_monitor_; }
    const system_monitor& get_system_monitor() const { return system_monitor_; }
    
    /**
     * @brief Set performance thresholds
     */
    void set_cpu_threshold(double threshold) {
        thresholds_.cpu_threshold = threshold;
    }
    
    void set_memory_threshold(double threshold) {
        thresholds_.memory_threshold = threshold;
    }
    
    void set_latency_threshold(std::chrono::milliseconds threshold) {
        thresholds_.latency_threshold = threshold;
    }
    
    /**
     * @brief Check if any thresholds are exceeded
     */
    monitoring_system::result<bool> check_thresholds() const;
};

/**
 * @brief Global performance monitor instance
 */
performance_monitor& global_performance_monitor();

/**
 * @brief Helper macro for timing code sections
 */
#define PERF_TIMER(operation_name) \
    monitoring_system::scoped_timer _perf_timer( \
        &monitoring_system::global_performance_monitor().get_profiler(), \
        operation_name \
    )

#define PERF_TIMER_CUSTOM(profiler, operation_name) \
    monitoring_system::scoped_timer _perf_timer(profiler, operation_name)

/**
 * @brief Performance benchmark utility
 */
class performance_benchmark {
private:
    performance_profiler profiler_;
    std::string name_;
    std::uint32_t iterations_{1000};
    std::uint32_t warmup_iterations_{100};
    
public:
    explicit performance_benchmark(const std::string& name)
        : name_(name) {}
    
    /**
     * @brief Set number of iterations
     */
    void set_iterations(std::uint32_t iterations) {
        iterations_ = iterations;
    }
    
    /**
     * @brief Set warmup iterations
     */
    void set_warmup_iterations(std::uint32_t warmup) {
        warmup_iterations_ = warmup;
    }
    
    /**
     * @brief Run a benchmark
     */
    template<typename Func>
    monitoring_system::result<performance_metrics> run(
        const std::string& operation_name,
        Func&& func
    ) {
        // Warmup
        for (std::uint32_t i = 0; i < warmup_iterations_; ++i) {
            func();
        }
        
        // Actual benchmark
        for (std::uint32_t i = 0; i < iterations_; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                func();
            } catch (...) {
                // Record error
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    end - start
                );
                profiler_.record_sample(operation_name, duration, false);
                continue;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                end - start
            );
            profiler_.record_sample(operation_name, duration, true);
        }
        
        return profiler_.get_metrics(operation_name);
    }
    
    /**
     * @brief Compare two operations
     */
    template<typename Func1, typename Func2>
    monitoring_system::result<std::pair<performance_metrics, performance_metrics>> compare(
        const std::string& operation1_name,
        Func1&& func1,
        const std::string& operation2_name,
        Func2&& func2
    ) {
        auto result1 = run(operation1_name, std::forward<Func1>(func1));
        if (!result1) {
            return monitoring_system::make_error<std::pair<performance_metrics, performance_metrics>>(
                result1.get_error().code
            );
        }
        
        auto result2 = run(operation2_name, std::forward<Func2>(func2));
        if (!result2) {
            return monitoring_system::make_error<std::pair<performance_metrics, performance_metrics>>(
                result2.get_error().code
            );
        }
        
        return std::make_pair(result1.value(), result2.value());
    }
};

} // namespace monitoring_system