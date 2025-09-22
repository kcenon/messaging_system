#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../adapters/logger_system_adapter.h"
#include "../core/event_bus.h"
#include "../core/event_types.h"
#include "plugin_metric_collector.h"

namespace monitoring_system {

/**
 * Log level enumeration
 */
enum class log_level {
    trace,
    debug,
    info,
    warning,
    error,
    critical,
    fatal
};

/**
 * Logging system statistics
 */
struct logging_stats {
    // Log volume metrics
    size_t total_log_count{0};
    size_t logs_per_second{0};
    std::unordered_map<log_level, size_t> logs_by_level;

    // Buffer metrics
    size_t buffer_size_bytes{0};
    size_t buffer_capacity_bytes{0};
    double buffer_usage_percent{0.0};
    size_t dropped_logs{0};

    // Performance metrics
    double average_log_latency_us{0.0};
    double max_log_latency_us{0.0};
    double min_log_latency_us{0.0};

    // File I/O metrics
    size_t files_open{0};
    size_t total_bytes_written{0};
    size_t write_operations{0};
    double average_write_size_bytes{0.0};

    // Error metrics
    size_t write_errors{0};
    size_t format_errors{0};
    size_t rotation_errors{0};

    // Log rotation metrics
    size_t rotations_performed{0};
    size_t archived_files{0};
    size_t total_archive_size_bytes{0};
};

/**
 * Log pattern analysis results
 */
struct log_pattern_analysis {
    struct pattern {
        std::string regex;
        size_t occurrences{0};
        double frequency_per_minute{0.0};
        log_level most_common_level{log_level::info};
        std::vector<std::string> sample_messages;
    };

    std::vector<pattern> detected_patterns;
    std::unordered_map<std::string, size_t> error_categories;
    std::unordered_map<std::string, size_t> component_frequencies;
    std::chrono::steady_clock::time_point analysis_time;
};

/**
 * Logger system metrics collector plugin
 * Collects metrics from logging systems and analyzes log patterns
 */
class logger_system_collector : public metric_collector_plugin {
  public:
    logger_system_collector();
    ~logger_system_collector() override;

    // metric_collector_plugin implementation
    bool initialize(const std::unordered_map<std::string, std::string>& config) override;
    std::vector<metric> collect() override;
    std::string get_name() const override { return "logger_system_collector"; }
    std::vector<std::string> get_metric_types() const override;
    bool is_healthy() const override;
    std::unordered_map<std::string, double> get_statistics() const override;

    /**
     * Set the logger system adapter for metric collection
     * @param adapter Logger system adapter instance
     */
    void set_logger_system_adapter(std::shared_ptr<logger_system_adapter> adapter);

    /**
     * Register a custom log source for monitoring
     * @param source_name Name of the log source
     * @param stats_provider Function to retrieve logging statistics
     */
    void register_log_source(const std::string& source_name,
                              std::function<logging_stats()> stats_provider);

    /**
     * Unregister a log source from monitoring
     * @param source_name Name of the log source
     */
    void unregister_log_source(const std::string& source_name);

    /**
     * Get statistics for a specific log source
     * @param source_name Name of the log source
     * @return Logging statistics if available
     */
    std::optional<logging_stats> get_source_stats(const std::string& source_name) const;

    /**
     * Enable log pattern analysis
     * @param enable True to enable pattern analysis
     * @param sample_size Number of logs to analyze
     */
    void set_pattern_analysis(bool enable, size_t sample_size = 1000);

    /**
     * Get latest pattern analysis results
     * @return Pattern analysis results if available
     */
    std::optional<log_pattern_analysis> get_pattern_analysis() const;

    /**
     * Set log level distribution tracking
     * @param enable True to enable level distribution tracking
     */
    void set_level_distribution_tracking(bool enable);

    /**
     * Get log level distribution over time
     * @param window_seconds Time window in seconds
     * @return Map of log levels to counts
     */
    std::unordered_map<log_level, size_t> get_level_distribution(size_t window_seconds = 60) const;

  private:
    // Logger system integration
    std::shared_ptr<logger_system_adapter> logger_adapter_;
    std::shared_ptr<event_bus> event_bus_;

    // Log source monitoring
    mutable std::mutex sources_mutex_;
    std::unordered_map<std::string, std::function<logging_stats()>> source_providers_;
    std::unordered_map<std::string, logging_stats> last_source_stats_;

    // Configuration
    bool enable_pattern_analysis_{false};
    size_t pattern_sample_size_{1000};
    bool track_level_distribution_{true};
    bool use_event_bus_{true};

    // Pattern analysis
    mutable std::mutex pattern_mutex_;
    log_pattern_analysis last_pattern_analysis_;
    std::chrono::steady_clock::time_point last_analysis_time_;

    // Level distribution tracking
    struct level_entry {
        log_level level;
        std::chrono::steady_clock::time_point timestamp;
    };
    mutable std::mutex distribution_mutex_;
    std::deque<level_entry> level_history_;
    const size_t max_history_size_{10000};

    // Statistics tracking
    mutable std::mutex stats_mutex_;
    std::atomic<size_t> collection_count_{0};
    std::atomic<size_t> collection_errors_{0};
    std::atomic<bool> is_healthy_{true};
    std::chrono::steady_clock::time_point init_time_;

    // Performance tracking
    struct throughput_tracker {
        std::chrono::steady_clock::time_point window_start;
        size_t logs_in_window{0};
        double current_throughput{0.0};
    };
    std::unordered_map<std::string, throughput_tracker> throughput_trackers_;

    // Helper methods
    std::vector<metric> collect_from_adapter();
    std::vector<metric> collect_from_sources();
    void add_source_metrics(std::vector<metric>& metrics, const std::string& source_name,
                            const logging_stats& stats);
    void perform_pattern_analysis();
    void update_level_distribution(const logging_stats& stats);
    void update_throughput_tracking(const std::string& source_name, const logging_stats& stats);
    metric create_metric(const std::string& name, double value, const std::string& source_name,
                         const std::string& unit = "") const;
    void subscribe_to_events();
    void handle_logging_event(const logging_metric_event& event);
};

/**
 * Log anomaly detector
 * Detects anomalies in logging patterns and volumes
 */
class log_anomaly_detector {
  public:
    enum class anomaly_type {
        volume_spike,
        volume_drop,
        error_spike,
        new_error_pattern,
        unusual_pattern,
        performance_degradation
    };

    struct anomaly {
        anomaly_type type;
        std::string description;
        double severity_score{0.0};  // 0.0 to 1.0
        std::unordered_map<std::string, std::string> details;
        std::chrono::steady_clock::time_point detected_at;
    };

    struct detection_config {
        // Volume thresholds
        double volume_spike_threshold{2.0};     // 2x normal volume
        double volume_drop_threshold{0.1};      // 10% of normal volume
        size_t volume_window_seconds{300};      // 5 minute window

        // Error thresholds
        double error_spike_threshold{3.0};      // 3x normal error rate
        size_t error_window_seconds{60};        // 1 minute window

        // Pattern detection
        bool enable_pattern_detection{true};
        double pattern_confidence_threshold{0.8};

        // Performance thresholds
        double latency_spike_threshold{2.0};    // 2x normal latency
    };

    explicit log_anomaly_detector(const detection_config& config = {});

    /**
     * Analyze logging statistics for anomalies
     * @param stats Current logging statistics
     * @param source_name Name of the log source
     * @return Vector of detected anomalies
     */
    std::vector<anomaly> detect_anomalies(const logging_stats& stats, const std::string& source_name);

    /**
     * Train the detector with normal behavior
     * @param stats Normal logging statistics
     * @param source_name Name of the log source
     */
    void train(const logging_stats& stats, const std::string& source_name);

    /**
     * Update detection configuration
     * @param config New detection configuration
     */
    void update_config(const detection_config& config);

    /**
     * Get current configuration
     * @return Current detection configuration
     */
    detection_config get_config() const;

    /**
     * Get anomaly history
     * @param source_name Optional source name filter
     * @param max_count Maximum number of anomalies
     * @return Vector of historical anomalies
     */
    std::vector<anomaly> get_anomaly_history(const std::optional<std::string>& source_name = std::nullopt,
                                              size_t max_count = 100) const;

    /**
     * Clear training data and history
     */
    void reset();

  private:
    mutable std::mutex config_mutex_;
    detection_config config_;

    // Training data
    struct baseline_stats {
        double average_volume{0.0};
        double volume_std_dev{0.0};
        double average_error_rate{0.0};
        double error_rate_std_dev{0.0};
        double average_latency{0.0};
        double latency_std_dev{0.0};
        size_t sample_count{0};
    };
    mutable std::mutex baseline_mutex_;
    std::unordered_map<std::string, baseline_stats> baselines_;

    // Historical data for trend analysis
    struct historical_point {
        logging_stats stats;
        std::chrono::steady_clock::time_point timestamp;
    };
    mutable std::mutex history_mutex_;
    std::unordered_map<std::string, std::deque<historical_point>> source_histories_;
    const size_t max_history_points_{1000};

    // Anomaly history
    std::vector<anomaly> anomaly_history_;
    const size_t max_anomaly_history_{1000};

    // Detection methods
    void detect_volume_anomalies(std::vector<anomaly>& anomalies, const logging_stats& stats,
                                  const std::string& source_name);
    void detect_error_anomalies(std::vector<anomaly>& anomalies, const logging_stats& stats,
                                 const std::string& source_name);
    void detect_performance_anomalies(std::vector<anomaly>& anomalies, const logging_stats& stats,
                                       const std::string& source_name);
    void update_baseline(baseline_stats& baseline, const logging_stats& stats);
    double calculate_zscore(double value, double mean, double std_dev) const;
};

/**
 * Log storage optimizer
 * Optimizes log storage and retention policies
 */
class log_storage_optimizer {
  public:
    struct storage_config {
        // Retention policies
        std::unordered_map<log_level, std::chrono::hours> retention_by_level{
            {log_level::trace, std::chrono::hours(1)},
            {log_level::debug, std::chrono::hours(24)},
            {log_level::info, std::chrono::hours(24 * 7)},
            {log_level::warning, std::chrono::hours(24 * 30)},
            {log_level::error, std::chrono::hours(24 * 90)},
            {log_level::critical, std::chrono::hours(24 * 365)},
            {log_level::fatal, std::chrono::hours(24 * 365)}
        };

        // Compression settings
        bool enable_compression{true};
        size_t compression_threshold_bytes{1024 * 1024};  // 1MB
        std::string compression_algorithm{"gzip"};

        // Archival settings
        bool enable_archival{true};
        std::chrono::hours archive_after_hours{24};
        std::string archive_location{"/var/log/archive"};

        // Storage limits
        size_t max_storage_bytes{1024 * 1024 * 1024};  // 1GB
        double storage_warn_threshold{0.8};  // 80% full
    };

    struct optimization_recommendation {
        std::string action;
        std::string reason;
        double expected_savings_bytes{0.0};
        double priority_score{0.0};  // 0.0 to 1.0
    };

    explicit log_storage_optimizer(const storage_config& config = {});

    /**
     * Analyze storage usage and provide recommendations
     * @param stats Current logging statistics
     * @return Vector of optimization recommendations
     */
    std::vector<optimization_recommendation> analyze_storage(const logging_stats& stats);

    /**
     * Calculate optimal retention policy based on usage patterns
     * @param level_distribution Log level distribution
     * @param available_storage Available storage in bytes
     * @return Recommended retention policy
     */
    std::unordered_map<log_level, std::chrono::hours> calculate_optimal_retention(
        const std::unordered_map<log_level, size_t>& level_distribution,
        size_t available_storage);

    /**
     * Estimate storage requirements
     * @param stats Current logging statistics
     * @param forecast_days Number of days to forecast
     * @return Estimated storage requirement in bytes
     */
    size_t estimate_storage_requirements(const logging_stats& stats, size_t forecast_days);

    /**
     * Update storage configuration
     * @param config New storage configuration
     */
    void update_config(const storage_config& config);

    /**
     * Get current configuration
     * @return Current storage configuration
     */
    storage_config get_config() const;

  private:
    mutable std::mutex config_mutex_;
    storage_config config_;

    double calculate_compression_ratio(const logging_stats& stats) const;
    size_t calculate_retention_size(log_level level, size_t daily_volume, std::chrono::hours retention) const;
    double calculate_priority_score(size_t savings_bytes, double urgency) const;
};

}  // namespace monitoring_system