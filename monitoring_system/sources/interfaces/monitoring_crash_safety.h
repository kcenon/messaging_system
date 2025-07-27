#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <string>
#include <memory>
#include <chrono>
#include <queue>
#include <thread>
#include <fstream>
#include <unordered_map>

namespace monitoring_module {

/**
 * @brief Monitoring-specific crash safety levels
 */
enum class monitoring_crash_safety_level {
    minimal,     // Basic metrics preservation
    standard,    // Standard recovery with data persistence
    paranoid     // Maximum safety with real-time backup and redundancy
};

/**
 * @brief Critical metrics snapshot for crash recovery
 */
struct critical_metrics_snapshot {
    std::chrono::system_clock::time_point timestamp;
    double cpu_usage_percent;
    uint64_t memory_usage_bytes;
    uint32_t active_threads;
    uint32_t jobs_pending;
    uint64_t jobs_completed;
    uint64_t jobs_failed;
    uint64_t average_latency_ns;
    std::string crash_context;
};

/**
 * @brief Monitoring crash safety manager
 * 
 * Provides comprehensive crash safety for monitoring systems:
 * - Real-time metrics backup
 * - Critical data preservation
 * - Monitoring state recovery
 * - Performance data continuity
 * - Alert system crash protection
 */
class monitoring_crash_safety {
public:
    /**
     * @brief Get the global monitoring crash safety instance
     */
    static monitoring_crash_safety& instance();

    /**
     * @brief Initialize crash safety for monitoring system
     * @param level Safety level to configure
     * @param backup_path Path for metrics backup
     * @param backup_interval_ms Backup creation interval
     */
    void initialize(monitoring_crash_safety_level level = monitoring_crash_safety_level::standard,
                   const std::string& backup_path = "./monitoring_backup.dat",
                   uint32_t backup_interval_ms = 1000);

    /**
     * @brief Register a monitoring component for crash protection
     * @param component_name Unique name for the component
     * @param save_state_callback Function to save component state
     * @param restore_state_callback Function to restore component state
     */
    void register_monitoring_component(const std::string& component_name,
                                     std::function<std::string()> save_state_callback,
                                     std::function<void(const std::string&)> restore_state_callback);

    /**
     * @brief Unregister a monitoring component
     * @param component_name Name of component to unregister
     */
    void unregister_monitoring_component(const std::string& component_name);

    /**
     * @brief Preserve critical metrics snapshot
     * @param snapshot Metrics snapshot to preserve
     */
    void preserve_critical_metrics(const critical_metrics_snapshot& snapshot);

    /**
     * @brief Set metrics backup file path
     * @param path Path to backup file
     */
    void set_backup_path(const std::string& path);

    /**
     * @brief Enable/disable real-time metrics backup
     * @param enable Whether to enable real-time backup
     * @param interval_ms Backup interval in milliseconds
     */
    void set_realtime_backup(bool enable, uint32_t interval_ms = 1000);

    /**
     * @brief Force immediate save of all monitoring states
     */
    void force_save_all_states();

    /**
     * @brief Force immediate restore of all monitoring states
     */
    void force_restore_all_states();

    /**
     * @brief Check if currently handling a monitoring crash
     * @return true if handling crash
     */
    bool is_handling_crash() const;

    /**
     * @brief Set maximum critical metrics to keep in memory
     * @param max_snapshots Maximum number of snapshots
     */
    void set_max_critical_snapshots(size_t max_snapshots);

    /**
     * @brief Get monitoring crash safety statistics
     */
    struct monitoring_safety_stats {
        size_t total_backups_created;
        size_t successful_saves;
        size_t failed_saves;
        size_t successful_restores;
        size_t failed_restores;
        size_t critical_snapshots_preserved;
        std::chrono::system_clock::time_point last_backup_time;
    };
    monitoring_safety_stats get_stats() const;

    /**
     * @brief Recovery check - detect and recover from previous crashes
     * @return true if recovery actions were taken
     */
    bool check_and_recover();

    /**
     * @brief Get last preserved critical metrics
     * @return Vector of preserved metrics snapshots
     */
    std::vector<critical_metrics_snapshot> get_preserved_metrics() const;

private:
    monitoring_crash_safety() = default;
    ~monitoring_crash_safety();

    // Non-copyable, non-movable
    monitoring_crash_safety(const monitoring_crash_safety&) = delete;
    monitoring_crash_safety& operator=(const monitoring_crash_safety&) = delete;

    struct monitoring_component {
        std::string name;
        std::function<std::string()> save_state_callback;
        std::function<void(const std::string&)> restore_state_callback;
    };

    // Internal crash handling
    static void signal_handler(int signal);
    void handle_monitoring_crash(int signal);
    void save_all_component_states();
    void restore_all_component_states();
    void backup_critical_metrics();
    void start_backup_thread();
    void stop_backup_thread();
    void backup_thread_worker();

    // Data persistence
    void write_backup_file();
    bool read_backup_file();
    void serialize_critical_metrics(std::ostream& out) const;
    void deserialize_critical_metrics(std::istream& in);

    // Configuration
    monitoring_crash_safety_level safety_level_ = monitoring_crash_safety_level::standard;
    std::string backup_path_ = "./monitoring_backup.dat";
    bool realtime_backup_enabled_ = true;
    uint32_t backup_interval_ms_ = 1000;
    size_t max_critical_snapshots_ = 1000;

    // Component management
    std::vector<monitoring_component> components_;
    mutable std::mutex components_mutex_;

    // Critical metrics preservation
    std::queue<critical_metrics_snapshot> critical_snapshots_;
    mutable std::mutex critical_snapshots_mutex_;

    // State tracking
    std::atomic<bool> initialized_{false};
    std::atomic<bool> handling_crash_{false};
    std::atomic<size_t> total_backups_{0};
    std::atomic<size_t> successful_saves_{0};
    std::atomic<size_t> failed_saves_{0};
    std::atomic<size_t> successful_restores_{0};
    std::atomic<size_t> failed_restores_{0};
    std::atomic<size_t> critical_snapshots_preserved_{0};
    std::chrono::system_clock::time_point last_backup_time_;

    // Backup thread
    std::unique_ptr<std::thread> backup_thread_;
    std::atomic<bool> backup_thread_running_{false};

    // Platform-specific signal handlers
    #ifdef _WIN32
    void* previous_handler_ = nullptr;
    #else
    struct sigaction previous_handlers_[32];
    #endif
};

/**
 * @brief RAII helper for automatic monitoring component registration
 */
class scoped_monitoring_crash_protection {
public:
    scoped_monitoring_crash_protection(const std::string& name,
                                      std::function<std::string()> save_callback,
                                      std::function<void(const std::string&)> restore_callback)
        : component_name_(name) {
        monitoring_crash_safety::instance().register_monitoring_component(name, save_callback, restore_callback);
    }

    ~scoped_monitoring_crash_protection() {
        monitoring_crash_safety::instance().unregister_monitoring_component(component_name_);
    }

    // Non-copyable, movable
    scoped_monitoring_crash_protection(const scoped_monitoring_crash_protection&) = delete;
    scoped_monitoring_crash_protection& operator=(const scoped_monitoring_crash_protection&) = delete;
    scoped_monitoring_crash_protection(scoped_monitoring_crash_protection&&) = default;
    scoped_monitoring_crash_protection& operator=(scoped_monitoring_crash_protection&&) = default;

private:
    std::string component_name_;
};

/**
 * @brief Ring buffer crash safety extensions
 */
class ring_buffer_crash_safety {
public:
    /**
     * @brief Configure crash safety for ring buffer
     * @param buffer_name Name of the ring buffer
     * @param persistent_backup Whether to create persistent backup
     * @param backup_threshold Backup when buffer reaches this percentage full
     */
    static void configure_ring_buffer_safety(const std::string& buffer_name,
                                            bool persistent_backup = true,
                                            double backup_threshold = 0.8);

    /**
     * @brief Set ring buffer overflow handler
     * @param buffer_name Name of the buffer
     * @param overflow_callback Function to handle overflow
     */
    static void set_overflow_handler(const std::string& buffer_name,
                                   std::function<void(size_t lost_entries)> overflow_callback);

    /**
     * @brief Create emergency snapshot of ring buffer data
     * @param buffer_name Name of the buffer
     * @param data_snapshot Buffer data to preserve
     */
    static void create_emergency_snapshot(const std::string& buffer_name,
                                        const std::vector<uint8_t>& data_snapshot);

private:
    static void handle_ring_buffer_crash(const std::string& buffer_name);
};

/**
 * @brief Alert system crash safety
 */
class alert_system_crash_safety {
public:
    /**
     * @brief Configure crash safety for alert system
     * @param alert_system_name Name of the alert system
     * @param immediate_alert_on_crash Whether to send immediate crash alert
     * @param persistent_alert_queue Whether to persist pending alerts
     */
    static void configure_alert_safety(const std::string& alert_system_name,
                                     bool immediate_alert_on_crash = true,
                                     bool persistent_alert_queue = true);

    /**
     * @brief Set crash alert handler
     * @param handler Function to handle crash alerts
     */
    static void set_crash_alert_handler(std::function<void(const std::string& crash_info)> handler);

    /**
     * @brief Preserve pending alerts before crash
     * @param alerts Vector of pending alert messages
     */
    static void preserve_pending_alerts(const std::vector<std::string>& alerts);

    /**
     * @brief Restore preserved alerts after recovery
     * @return Vector of restored alert messages
     */
    static std::vector<std::string> restore_preserved_alerts();

private:
    static void handle_alert_system_crash(const std::string& alert_system_name);
    static std::string alerts_backup_file_;
    static std::function<void(const std::string&)> crash_alert_handler_;
};

/**
 * @brief Metrics collection crash safety
 */
class metrics_collection_crash_safety {
public:
    /**
     * @brief Configure crash safety for metrics collection
     * @param collector_name Name of the metrics collector
     * @param auto_resume_collection Whether to auto-resume collection after crash
     * @param preserve_collection_state Whether to preserve collection state
     */
    static void configure_collector_safety(const std::string& collector_name,
                                          bool auto_resume_collection = true,
                                          bool preserve_collection_state = true);

    /**
     * @brief Set collection state preservation callback
     * @param collector_name Name of the collector
     * @param state_callback Function to save/restore collection state
     */
    static void set_state_preservation_callback(const std::string& collector_name,
                                               std::function<std::string()> save_state,
                                               std::function<void(const std::string&)> restore_state);

private:
    static void handle_collector_crash(const std::string& collector_name);
};

} // namespace monitoring_module