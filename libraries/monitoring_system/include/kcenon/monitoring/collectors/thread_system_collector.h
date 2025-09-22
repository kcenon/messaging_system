#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../adapters/thread_system_adapter.h"
#include "../core/event_bus.h"
#include "../core/event_types.h"
#include "plugin_metric_collector.h"

namespace monitoring_system {

/**
 * Thread pool statistics
 */
struct thread_pool_stats {
    // Pool configuration
    size_t pool_size{0};
    size_t max_pool_size{0};
    size_t min_pool_size{0};

    // Thread states
    size_t active_threads{0};
    size_t idle_threads{0};
    size_t waiting_threads{0};

    // Task queue metrics
    size_t queued_tasks{0};
    size_t max_queue_size{0};
    size_t completed_tasks{0};
    size_t failed_tasks{0};
    size_t rejected_tasks{0};

    // Performance metrics
    double average_task_duration_ms{0.0};
    double max_task_duration_ms{0.0};
    double min_task_duration_ms{0.0};
    double task_throughput_per_sec{0.0};

    // Thread utilization
    double thread_utilization_percent{0.0};
    double cpu_usage_percent{0.0};

    // Queue wait times
    double average_queue_wait_ms{0.0};
    double max_queue_wait_ms{0.0};
};

/**
 * Thread system metrics collector plugin
 * Collects metrics from thread pools and thread management systems
 */
class thread_system_collector : public metric_collector_plugin {
  public:
    thread_system_collector();
    ~thread_system_collector() override;

    // metric_collector_plugin implementation
    bool initialize(const std::unordered_map<std::string, std::string>& config) override;
    std::vector<metric> collect() override;
    std::string get_name() const override { return "thread_system_collector"; }
    std::vector<std::string> get_metric_types() const override;
    bool is_healthy() const override;
    std::unordered_map<std::string, double> get_statistics() const override;

    /**
     * Set the thread system adapter for metric collection
     * @param adapter Thread system adapter instance
     */
    void set_thread_system_adapter(std::shared_ptr<thread_system_adapter> adapter);

    /**
     * Register a thread pool for monitoring
     * @param pool_name Name of the thread pool
     * @param stats_provider Function to retrieve pool statistics
     */
    void register_thread_pool(const std::string& pool_name,
                               std::function<thread_pool_stats()> stats_provider);

    /**
     * Unregister a thread pool from monitoring
     * @param pool_name Name of the thread pool
     */
    void unregister_thread_pool(const std::string& pool_name);

    /**
     * Get statistics for a specific thread pool
     * @param pool_name Name of the thread pool
     * @return Thread pool statistics if available
     */
    std::optional<thread_pool_stats> get_pool_stats(const std::string& pool_name) const;

    /**
     * Get all monitored thread pool names
     * @return Vector of pool names
     */
    std::vector<std::string> get_monitored_pools() const;

    /**
     * Enable/disable detailed thread metrics
     * @param enable True to enable detailed metrics
     */
    void set_detailed_metrics(bool enable);

    /**
     * Set collection interval for continuous monitoring
     * @param interval Collection interval
     */
    void set_collection_interval(std::chrono::milliseconds interval);

  private:
    // Thread system integration
    std::shared_ptr<thread_system_adapter> thread_adapter_;
    std::shared_ptr<event_bus> event_bus_;

    // Thread pool monitoring
    mutable std::mutex pools_mutex_;
    std::unordered_map<std::string, std::function<thread_pool_stats()>> pool_providers_;
    std::unordered_map<std::string, thread_pool_stats> last_pool_stats_;

    // Configuration
    bool collect_detailed_metrics_{false};
    bool use_event_bus_{true};
    std::chrono::milliseconds collection_interval_{1000};

    // Statistics tracking
    mutable std::mutex stats_mutex_;
    std::atomic<size_t> collection_count_{0};
    std::atomic<size_t> collection_errors_{0};
    std::atomic<bool> is_healthy_{true};
    std::chrono::steady_clock::time_point init_time_;

    // Performance tracking
    struct performance_tracker {
        size_t total_tasks{0};
        double total_duration_ms{0.0};
        double total_wait_time_ms{0.0};
        std::chrono::steady_clock::time_point last_reset;
    };
    std::unordered_map<std::string, performance_tracker> performance_trackers_;

    // Helper methods
    std::vector<metric> collect_from_adapter();
    std::vector<metric> collect_from_pools();
    void add_pool_metrics(std::vector<metric>& metrics, const std::string& pool_name,
                          const thread_pool_stats& stats);
    void update_performance_tracking(const std::string& pool_name, const thread_pool_stats& stats);
    metric create_metric(const std::string& name, double value, const std::string& pool_name,
                         const std::string& unit = "") const;
    void subscribe_to_events();
    void handle_thread_pool_event(const thread_pool_metric_event& event);
};

/**
 * Thread pool health monitor
 * Monitors thread pool health and detects anomalies
 */
class thread_pool_health_monitor {
  public:
    enum class health_status {
        healthy,
        degraded,
        unhealthy,
        critical
    };

    struct health_report {
        std::string pool_name;
        health_status status;
        std::vector<std::string> issues;
        std::unordered_map<std::string, double> metrics;
        std::chrono::steady_clock::time_point timestamp;
    };

    struct health_thresholds {
        // Task queue thresholds
        double queue_saturation_warn{0.7};     // 70% queue full
        double queue_saturation_critical{0.9};  // 90% queue full

        // Thread utilization thresholds
        double thread_utilization_low{0.2};    // 20% utilization (underutilized)
        double thread_utilization_high{0.9};   // 90% utilization (overloaded)

        // Task performance thresholds
        double task_failure_rate_warn{0.05};      // 5% failure rate
        double task_failure_rate_critical{0.1};   // 10% failure rate
        double task_rejection_rate_warn{0.01};    // 1% rejection rate
        double task_rejection_rate_critical{0.05}; // 5% rejection rate

        // Queue wait time thresholds (ms)
        double queue_wait_warn_ms{1000};      // 1 second wait
        double queue_wait_critical_ms{5000};  // 5 seconds wait
    };

    explicit thread_pool_health_monitor(const health_thresholds& thresholds = {});

    /**
     * Analyze thread pool health
     * @param stats Thread pool statistics
     * @param pool_name Name of the pool
     * @return Health report
     */
    health_report analyze_health(const thread_pool_stats& stats, const std::string& pool_name);

    /**
     * Get overall health status from multiple pools
     * @param pool_stats Map of pool names to statistics
     * @return Overall health status
     */
    health_status get_overall_health(const std::unordered_map<std::string, thread_pool_stats>& pool_stats);

    /**
     * Update health thresholds
     * @param thresholds New threshold values
     */
    void update_thresholds(const health_thresholds& thresholds);

    /**
     * Get current thresholds
     * @return Current health thresholds
     */
    health_thresholds get_thresholds() const;

    /**
     * Get health history
     * @param pool_name Optional pool name filter
     * @param max_count Maximum number of reports
     * @return Vector of health reports
     */
    std::vector<health_report> get_health_history(const std::optional<std::string>& pool_name = std::nullopt,
                                                   size_t max_count = 100) const;

    /**
     * Clear health history
     */
    void clear_history();

  private:
    mutable std::mutex thresholds_mutex_;
    health_thresholds thresholds_;

    mutable std::mutex history_mutex_;
    std::vector<health_report> health_history_;
    const size_t max_history_size_{1000};

    health_status calculate_status(const std::vector<std::string>& issues) const;
    void check_queue_saturation(health_report& report, const thread_pool_stats& stats);
    void check_thread_utilization(health_report& report, const thread_pool_stats& stats);
    void check_task_performance(health_report& report, const thread_pool_stats& stats);
    void check_queue_wait_times(health_report& report, const thread_pool_stats& stats);
};

/**
 * Thread pool auto-scaler
 * Automatically adjusts thread pool size based on load
 */
class thread_pool_auto_scaler {
  public:
    struct scaling_config {
        // Scaling boundaries
        size_t min_threads{1};
        size_t max_threads{std::thread::hardware_concurrency() * 2};

        // Scaling triggers
        double scale_up_threshold{0.8};    // 80% utilization
        double scale_down_threshold{0.3};  // 30% utilization

        // Scaling parameters
        size_t scale_up_increment{2};
        size_t scale_down_decrement{1};

        // Timing parameters
        std::chrono::seconds scale_up_cooldown{30};
        std::chrono::seconds scale_down_cooldown{60};
        std::chrono::seconds evaluation_interval{10};

        // Stability parameters
        size_t min_stable_evaluations{3};  // Number of consistent evaluations before scaling
    };

    struct scaling_decision {
        enum class action { none, scale_up, scale_down };

        action recommended_action;
        size_t current_size;
        size_t recommended_size;
        std::string reason;
        std::chrono::steady_clock::time_point timestamp;
    };

    explicit thread_pool_auto_scaler(const scaling_config& config = {});

    /**
     * Evaluate scaling decision based on current statistics
     * @param stats Current thread pool statistics
     * @return Scaling decision
     */
    scaling_decision evaluate(const thread_pool_stats& stats);

    /**
     * Apply scaling decision
     * @param decision Scaling decision to apply
     * @param resize_function Function to resize the thread pool
     * @return True if scaling was applied
     */
    bool apply_scaling(const scaling_decision& decision,
                       std::function<bool(size_t)> resize_function);

    /**
     * Update scaling configuration
     * @param config New scaling configuration
     */
    void update_config(const scaling_config& config);

    /**
     * Get current configuration
     * @return Current scaling configuration
     */
    scaling_config get_config() const;

    /**
     * Get scaling history
     * @param max_count Maximum number of decisions
     * @return Vector of scaling decisions
     */
    std::vector<scaling_decision> get_scaling_history(size_t max_count = 100) const;

    /**
     * Reset scaler state
     */
    void reset();

  private:
    mutable std::mutex config_mutex_;
    scaling_config config_;

    mutable std::mutex state_mutex_;
    std::chrono::steady_clock::time_point last_scale_up_;
    std::chrono::steady_clock::time_point last_scale_down_;
    std::vector<double> utilization_history_;
    std::vector<scaling_decision> scaling_history_;
    const size_t max_history_size_{1000};

    bool should_scale_up(const thread_pool_stats& stats) const;
    bool should_scale_down(const thread_pool_stats& stats) const;
    bool is_in_cooldown(scaling_decision::action action) const;
    bool has_stable_utilization(double threshold, bool above) const;
    size_t calculate_new_size(size_t current_size, scaling_decision::action action) const;
};

}  // namespace monitoring_system