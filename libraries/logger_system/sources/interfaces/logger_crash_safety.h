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
#include <fstream>
#include <chrono>
#include <queue>
#include <thread>

namespace logger_module {

/**
 * @brief Logger-specific crash safety levels
 */
enum class logger_crash_safety_level {
    minimal,     // Basic log flushing on crash
    standard,    // Standard recovery with emergency logging
    paranoid     // Maximum safety with backup files and redundancy
};

/**
 * @brief Emergency log entry for crash scenarios
 */
struct emergency_log_entry {
    std::chrono::system_clock::time_point timestamp;
    std::string level;
    std::string message;
    std::string thread_id;
    int signal_number = 0;
};

/**
 * @brief Logger crash safety manager
 * 
 * Provides comprehensive crash safety for logging systems:
 * - Emergency log flushing on crash
 * - Backup file creation
 * - Signal-safe logging
 * - Log corruption prevention
 * - Recovery mechanisms
 */
class logger_crash_safety {
public:
    /**
     * @brief Get the global logger crash safety instance
     */
    static logger_crash_safety& instance();

    /**
     * @brief Initialize crash safety for logger system
     * @param level Safety level to configure
     * @param emergency_log_path Path for emergency crash logs
     * @param backup_interval_ms Backup creation interval
     */
    void initialize(logger_crash_safety_level level = logger_crash_safety_level::standard,
                   const std::string& emergency_log_path = "./emergency.log",
                   uint32_t backup_interval_ms = 5000);

    /**
     * @brief Register a logger for crash protection
     * @param logger_name Unique name for the logger
     * @param flush_callback Function to flush logger on crash
     * @param backup_callback Function to create backup
     */
    void register_logger(const std::string& logger_name,
                        std::function<void()> flush_callback,
                        std::function<void(const std::string&)> backup_callback = nullptr);

    /**
     * @brief Unregister a logger from crash protection
     * @param logger_name Name of logger to unregister
     */
    void unregister_logger(const std::string& logger_name);

    /**
     * @brief Write emergency log entry (signal-safe)
     * @param level Log level as string
     * @param message Message to log
     */
    void emergency_log(const std::string& level, const std::string& message);

    /**
     * @brief Set emergency log file path
     * @param path Path to emergency log file
     */
    void set_emergency_log_path(const std::string& path);

    /**
     * @brief Enable/disable automatic backups
     * @param enable Whether to create automatic backups
     * @param interval_ms Backup interval in milliseconds
     */
    void set_auto_backup(bool enable, uint32_t interval_ms = 5000);

    /**
     * @brief Force immediate flush of all registered loggers
     */
    void force_flush_all();

    /**
     * @brief Force immediate backup of all registered loggers
     */
    void force_backup_all();

    /**
     * @brief Check if currently handling a crash
     * @return true if handling crash
     */
    bool is_handling_crash() const;

    /**
     * @brief Set maximum emergency log entries to keep in memory
     * @param max_entries Maximum number of entries
     */
    void set_max_emergency_entries(size_t max_entries);

    /**
     * @brief Get emergency log statistics
     */
    struct emergency_log_stats {
        size_t total_emergency_logs;
        size_t successful_flushes;
        size_t failed_flushes;
        size_t backup_count;
        std::chrono::system_clock::time_point last_emergency_time;
    };
    emergency_log_stats get_stats() const;

    /**
     * @brief Recovery check - detect and recover from previous crashes
     * @return true if recovery actions were taken
     */
    bool check_and_recover();

private:
    logger_crash_safety() = default;
    ~logger_crash_safety();

    // Non-copyable, non-movable
    logger_crash_safety(const logger_crash_safety&) = delete;
    logger_crash_safety& operator=(const logger_crash_safety&) = delete;

    struct logger_entry {
        std::string name;
        std::function<void()> flush_callback;
        std::function<void(const std::string&)> backup_callback;
    };

    // Internal crash handling
    static void signal_handler(int signal);
    void handle_logger_crash(int signal);
    void flush_all_loggers();
    void backup_all_loggers();
    void write_emergency_log_entry(const emergency_log_entry& entry);
    void cleanup_old_emergency_logs();
    void start_backup_thread();
    void stop_backup_thread();
    void backup_thread_worker();

    // Signal-safe emergency logging
    void signal_safe_write(const char* data, size_t length);
    void signal_safe_emergency_log(const char* level, const char* message);

    // Configuration
    logger_crash_safety_level safety_level_ = logger_crash_safety_level::standard;
    std::string emergency_log_path_ = "./emergency.log";
    bool auto_backup_enabled_ = true;
    uint32_t backup_interval_ms_ = 5000;
    size_t max_emergency_entries_ = 1000;

    // Logger management
    std::vector<logger_entry> loggers_;
    mutable std::mutex loggers_mutex_;

    // Emergency logging
    std::queue<emergency_log_entry> emergency_log_queue_;
    mutable std::mutex emergency_log_mutex_;
    int emergency_log_fd_ = -1;

    // State tracking
    std::atomic<bool> initialized_{false};
    std::atomic<bool> handling_crash_{false};
    std::atomic<size_t> total_emergency_logs_{0};
    std::atomic<size_t> successful_flushes_{0};
    std::atomic<size_t> failed_flushes_{0};
    std::atomic<size_t> backup_count_{0};
    std::chrono::system_clock::time_point last_emergency_time_;

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
 * @brief RAII helper for automatic logger registration
 */
class scoped_logger_crash_protection {
public:
    scoped_logger_crash_protection(const std::string& name,
                                  std::function<void()> flush_callback,
                                  std::function<void(const std::string&)> backup_callback = nullptr)
        : logger_name_(name) {
        logger_crash_safety::instance().register_logger(name, flush_callback, backup_callback);
    }

    ~scoped_logger_crash_protection() {
        logger_crash_safety::instance().unregister_logger(logger_name_);
    }

    // Non-copyable, movable
    scoped_logger_crash_protection(const scoped_logger_crash_protection&) = delete;
    scoped_logger_crash_protection& operator=(const scoped_logger_crash_protection&) = delete;
    scoped_logger_crash_protection(scoped_logger_crash_protection&&) = default;
    scoped_logger_crash_protection& operator=(scoped_logger_crash_protection&&) = default;

private:
    std::string logger_name_;
};

/**
 * @brief Log file corruption detector and recovery
 */
class log_file_recovery {
public:
    /**
     * @brief Check if log file is corrupted
     * @param file_path Path to log file
     * @return true if corruption detected
     */
    static bool is_corrupted(const std::string& file_path);

    /**
     * @brief Attempt to recover corrupted log file
     * @param file_path Path to corrupted log file
     * @param recovery_path Path for recovered file
     * @return true if recovery successful
     */
    static bool recover_file(const std::string& file_path, const std::string& recovery_path);

    /**
     * @brief Create backup of log file with integrity check
     * @param source_path Source log file
     * @param backup_path Backup file path
     * @return true if backup successful
     */
    static bool create_backup_with_checksum(const std::string& source_path,
                                           const std::string& backup_path);

    /**
     * @brief Verify log file integrity using checksum
     * @param file_path Path to log file
     * @param checksum_path Path to checksum file
     * @return true if integrity verified
     */
    static bool verify_integrity(const std::string& file_path, const std::string& checksum_path);

private:
    static std::string calculate_checksum(const std::string& file_path);
    static bool write_checksum(const std::string& checksum, const std::string& checksum_path);
    static std::string read_checksum(const std::string& checksum_path);
};

/**
 * @brief Asynchronous logger crash safety extensions
 */
class async_logger_crash_safety {
public:
    /**
     * @brief Configure crash safety for async logger
     * @param logger_name Name of the async logger
     * @param flush_timeout_ms Maximum time to wait for flush
     * @param emergency_sync_mode Whether to switch to sync mode on crash
     */
    static void configure_async_safety(const std::string& logger_name,
                                     uint32_t flush_timeout_ms = 1000,
                                     bool emergency_sync_mode = true);

    /**
     * @brief Handle async logger buffer overflow
     * @param logger_name Name of the logger
     * @param overflow_callback Function to handle overflow
     */
    static void set_overflow_handler(const std::string& logger_name,
                                   std::function<void(size_t dropped_messages)> overflow_callback);

private:
    static void handle_async_crash(const std::string& logger_name);
};

} // namespace logger_module