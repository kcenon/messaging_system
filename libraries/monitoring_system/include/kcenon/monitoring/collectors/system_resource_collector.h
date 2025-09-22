#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifdef __APPLE__
    #include <mach/mach.h>
    #include <mach/mach_host.h>
    #include <mach/processor_info.h>
    #include <mach/vm_map.h>
    #include <sys/sysctl.h>
    #include <sys/types.h>
#elif __linux__
    #include <sys/sysinfo.h>
    #include <unistd.h>
#elif _WIN32
    #include <psapi.h>
    #include <windows.h>
#endif

#include "plugin_metric_collector.h"

namespace monitoring_system {

/**
 * System resource information structure
 */
struct system_resources {
    // CPU metrics
    double cpu_usage_percent{0.0};
    double cpu_user_percent{0.0};
    double cpu_system_percent{0.0};
    double cpu_idle_percent{0.0};
    size_t cpu_count{0};
    double load_average_1min{0.0};
    double load_average_5min{0.0};
    double load_average_15min{0.0};

    // Memory metrics
    size_t total_memory_bytes{0};
    size_t available_memory_bytes{0};
    size_t used_memory_bytes{0};
    double memory_usage_percent{0.0};
    size_t swap_total_bytes{0};
    size_t swap_used_bytes{0};
    double swap_usage_percent{0.0};

    // Disk metrics
    size_t disk_total_bytes{0};
    size_t disk_used_bytes{0};
    size_t disk_available_bytes{0};
    double disk_usage_percent{0.0};
    size_t disk_read_bytes_per_sec{0};
    size_t disk_write_bytes_per_sec{0};

    // Network metrics
    size_t network_rx_bytes_per_sec{0};
    size_t network_tx_bytes_per_sec{0};
    size_t network_rx_packets_per_sec{0};
    size_t network_tx_packets_per_sec{0};
    size_t network_errors{0};
    size_t network_drops{0};

    // Process metrics
    size_t process_count{0};
    size_t thread_count{0};
    size_t handle_count{0};
    size_t open_file_descriptors{0};
};

/**
 * Platform-specific system resource collector implementation
 */
class system_info_collector {
  public:
    system_info_collector();
    ~system_info_collector();

    /**
     * Collect current system resources
     * @return System resource information
     */
    system_resources collect();

    /**
     * Get system uptime in seconds
     * @return Uptime in seconds
     */
    std::chrono::seconds get_uptime() const;

    /**
     * Get system hostname
     * @return Hostname string
     */
    std::string get_hostname() const;

    /**
     * Get operating system information
     * @return OS information string
     */
    std::string get_os_info() const;

  private:
    // CPU tracking for usage calculation
    struct cpu_stats {
        uint64_t user{0};
        uint64_t nice{0};
        uint64_t system{0};
        uint64_t idle{0};
        uint64_t iowait{0};
        uint64_t irq{0};
        uint64_t softirq{0};
        uint64_t steal{0};
    };

    mutable std::mutex stats_mutex_;
    cpu_stats last_cpu_stats_;
    std::chrono::steady_clock::time_point last_collection_time_;

    // Network tracking
    struct network_stats {
        uint64_t rx_bytes{0};
        uint64_t tx_bytes{0};
        uint64_t rx_packets{0};
        uint64_t tx_packets{0};
        uint64_t errors{0};
        uint64_t drops{0};
    };
    network_stats last_network_stats_;

    // Disk I/O tracking
    struct disk_stats {
        uint64_t read_bytes{0};
        uint64_t write_bytes{0};
    };
    disk_stats last_disk_stats_;

    // Platform-specific collection methods
    void collect_cpu_stats(system_resources& resources);
    void collect_memory_stats(system_resources& resources);
    void collect_disk_stats(system_resources& resources);
    void collect_network_stats(system_resources& resources);
    void collect_process_stats(system_resources& resources);

#ifdef __APPLE__
    void collect_macos_cpu_stats(system_resources& resources);
    void collect_macos_memory_stats(system_resources& resources);
#elif __linux__
    void collect_linux_cpu_stats(system_resources& resources);
    void collect_linux_memory_stats(system_resources& resources);
    cpu_stats parse_proc_stat();
#elif _WIN32
    void collect_windows_cpu_stats(system_resources& resources);
    void collect_windows_memory_stats(system_resources& resources);
#endif
};

/**
 * System resource collector plugin implementation
 */
class system_resource_collector : public metric_collector_plugin {
  public:
    system_resource_collector();
    ~system_resource_collector() override = default;

    // metric_collector_plugin implementation
    bool initialize(const std::unordered_map<std::string, std::string>& config) override;
    std::vector<metric> collect() override;
    std::string get_name() const override { return "system_resource_collector"; }
    std::vector<std::string> get_metric_types() const override;
    bool is_healthy() const override;
    std::unordered_map<std::string, double> get_statistics() const override;

    /**
     * Set collection filters
     * @param enable_cpu Enable CPU metrics collection
     * @param enable_memory Enable memory metrics collection
     * @param enable_disk Enable disk metrics collection
     * @param enable_network Enable network metrics collection
     */
    void set_collection_filters(bool enable_cpu = true, bool enable_memory = true,
                                 bool enable_disk = true, bool enable_network = true);

    /**
     * Get last collected resources
     * @return Last system resources snapshot
     */
    system_resources get_last_resources() const;

  private:
    // Collector implementation
    std::unique_ptr<system_info_collector> collector_;

    // Configuration
    bool collect_cpu_metrics_{true};
    bool collect_memory_metrics_{true};
    bool collect_disk_metrics_{true};
    bool collect_network_metrics_{true};
    bool collect_process_metrics_{true};

    // Statistics
    mutable std::mutex stats_mutex_;
    std::atomic<size_t> collection_count_{0};
    std::atomic<size_t> collection_errors_{0};
    std::chrono::steady_clock::time_point init_time_;
    system_resources last_resources_;

    // Helper methods
    metric create_metric(const std::string& name, double value, const std::string& unit = "",
                         const std::unordered_map<std::string, std::string>& labels = {}) const;
    void add_cpu_metrics(std::vector<metric>& metrics, const system_resources& resources);
    void add_memory_metrics(std::vector<metric>& metrics, const system_resources& resources);
    void add_disk_metrics(std::vector<metric>& metrics, const system_resources& resources);
    void add_network_metrics(std::vector<metric>& metrics, const system_resources& resources);
    void add_process_metrics(std::vector<metric>& metrics, const system_resources& resources);
};

/**
 * Resource threshold monitor
 * Monitors system resources against configured thresholds
 */
class resource_threshold_monitor {
  public:
    struct thresholds {
        double cpu_usage_warn{75.0};
        double cpu_usage_critical{90.0};
        double memory_usage_warn{80.0};
        double memory_usage_critical{95.0};
        double disk_usage_warn{85.0};
        double disk_usage_critical{95.0};
        double swap_usage_warn{50.0};
        double swap_usage_critical{80.0};
    };

    struct alert {
        enum class severity { info, warning, critical };

        std::string resource;
        severity level;
        double current_value;
        double threshold;
        std::string message;
        std::chrono::steady_clock::time_point timestamp;
    };

    explicit resource_threshold_monitor(const thresholds& config = {});

    /**
     * Check resources against thresholds
     * @param resources System resources to check
     * @return Vector of triggered alerts
     */
    std::vector<alert> check_thresholds(const system_resources& resources);

    /**
     * Update threshold configuration
     * @param config New threshold values
     */
    void update_thresholds(const thresholds& config);

    /**
     * Get current threshold configuration
     * @return Current thresholds
     */
    thresholds get_thresholds() const;

    /**
     * Get alert history
     * @param max_count Maximum number of alerts to return
     * @return Vector of recent alerts
     */
    std::vector<alert> get_alert_history(size_t max_count = 100) const;

    /**
     * Clear alert history
     */
    void clear_history();

  private:
    mutable std::mutex config_mutex_;
    thresholds config_;

    mutable std::mutex history_mutex_;
    std::vector<alert> alert_history_;
    const size_t max_history_size_{1000};

    void check_cpu_usage(std::vector<alert>& alerts, const system_resources& resources);
    void check_memory_usage(std::vector<alert>& alerts, const system_resources& resources);
    void check_disk_usage(std::vector<alert>& alerts, const system_resources& resources);
    void check_swap_usage(std::vector<alert>& alerts, const system_resources& resources);
    void add_alert(std::vector<alert>& alerts, const std::string& resource,
                   alert::severity level, double value, double threshold, const std::string& message);
};

}  // namespace monitoring_system