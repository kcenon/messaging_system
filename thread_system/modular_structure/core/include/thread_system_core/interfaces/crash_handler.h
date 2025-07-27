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
#include <csignal>
#include <memory>

namespace thread_module {

/**
 * @brief Crash safety levels for different scenarios
 */
enum class crash_safety_level {
    minimal,     // Basic signal handling
    standard,    // Standard recovery with logging
    paranoid     // Maximum safety with redundancy
};

/**
 * @brief Crash context information
 */
struct crash_context {
    int signal_number;
    std::string signal_name;
    void* fault_address;
    std::string stack_trace;
    std::chrono::system_clock::time_point crash_time;
    std::thread::id crashing_thread;
};

/**
 * @brief Callback function type for crash handling
 */
using crash_callback = std::function<void(const crash_context&)>;

/**
 * @brief Thread-safe crash handler for the entire thread system
 * 
 * Provides comprehensive crash safety mechanisms including:
 * - Signal handling (SIGSEGV, SIGABRT, SIGFPE, etc.)
 * - Stack trace generation
 * - Graceful shutdown coordination
 * - Resource cleanup callbacks
 * - Cross-platform crash reporting
 */
class crash_handler {
public:
    /**
     * @brief Get the global crash handler instance
     * @return Reference to the singleton crash handler
     */
    static crash_handler& instance();

    /**
     * @brief Initialize crash handling with specified safety level
     * @param level Safety level to configure
     * @param enable_core_dumps Whether to enable core dump generation
     */
    void initialize(crash_safety_level level = crash_safety_level::standard,
                   bool enable_core_dumps = false);

    /**
     * @brief Register a callback to be called during crash handling
     * @param name Unique name for the callback
     * @param callback Function to call during crash
     * @param priority Priority order (lower numbers called first)
     * @return Registration ID for later removal
     */
    size_t register_crash_callback(const std::string& name,
                                  crash_callback callback,
                                  int priority = 100);

    /**
     * @brief Unregister a crash callback
     * @param registration_id ID returned from register_crash_callback
     */
    void unregister_crash_callback(size_t registration_id);

    /**
     * @brief Register a resource cleanup function
     * @param name Unique name for cleanup
     * @param cleanup Function to call for cleanup
     * @param timeout_ms Maximum time to wait for cleanup
     */
    void register_cleanup(const std::string& name,
                         std::function<void()> cleanup,
                         uint32_t timeout_ms = 1000);

    /**
     * @brief Set custom crash log directory
     * @param directory Path to directory for crash logs
     */
    void set_crash_log_directory(const std::string& directory);

    /**
     * @brief Enable/disable automatic stack trace generation
     * @param enable Whether to generate stack traces
     */
    void set_stack_trace_enabled(bool enable);

    /**
     * @brief Manually trigger crash handling (for testing)
     * @param context Custom crash context
     */
    void trigger_crash_handling(const crash_context& context);

    /**
     * @brief Check if crash handler is currently active
     * @return true if currently handling a crash
     */
    bool is_handling_crash() const;

    /**
     * @brief Get crash statistics
     */
    struct crash_stats {
        size_t total_crashes_handled;
        size_t successful_cleanups;
        size_t failed_cleanups;
        std::chrono::system_clock::time_point last_crash_time;
    };
    crash_stats get_stats() const;

private:
    crash_handler() = default;
    ~crash_handler();

    // Non-copyable, non-movable
    crash_handler(const crash_handler&) = delete;
    crash_handler& operator=(const crash_handler&) = delete;

    struct callback_entry {
        size_t id;
        std::string name;
        crash_callback callback;
        int priority;
    };

    struct cleanup_entry {
        std::string name;
        std::function<void()> cleanup;
        uint32_t timeout_ms;
    };

    // Internal crash handling
    static void signal_handler(int signal);
    void handle_crash(int signal);
    void execute_callbacks(const crash_context& context);
    void execute_cleanups();
    std::string generate_stack_trace();
    void write_crash_log(const crash_context& context);

    // Configuration
    crash_safety_level safety_level_ = crash_safety_level::standard;
    bool enable_core_dumps_ = false;
    bool stack_trace_enabled_ = true;
    std::string crash_log_directory_ = "./crash_logs";

    // Callbacks and cleanup
    std::vector<callback_entry> callbacks_;
    std::vector<cleanup_entry> cleanups_;
    mutable std::mutex callbacks_mutex_;
    std::atomic<size_t> next_callback_id_{1};

    // State tracking
    std::atomic<bool> initialized_{false};
    std::atomic<bool> handling_crash_{false};
    std::atomic<size_t> total_crashes_{0};
    std::atomic<size_t> successful_cleanups_{0};
    std::atomic<size_t> failed_cleanups_{0};
    std::chrono::system_clock::time_point last_crash_time_;

    // Platform-specific data
    #ifdef _WIN32
    void* previous_handler_ = nullptr;
    #else
    struct sigaction previous_handlers_[32];
    #endif
};

/**
 * @brief RAII helper for automatic crash callback registration
 */
class scoped_crash_callback {
public:
    scoped_crash_callback(const std::string& name, crash_callback callback, int priority = 100)
        : registration_id_(crash_handler::instance().register_crash_callback(name, callback, priority)) {}

    ~scoped_crash_callback() {
        crash_handler::instance().unregister_crash_callback(registration_id_);
    }

    // Non-copyable, movable
    scoped_crash_callback(const scoped_crash_callback&) = delete;
    scoped_crash_callback& operator=(const scoped_crash_callback&) = delete;
    scoped_crash_callback(scoped_crash_callback&&) = default;
    scoped_crash_callback& operator=(scoped_crash_callback&&) = default;

private:
    size_t registration_id_;
};

/**
 * @brief Thread pool crash safety extensions
 */
class thread_pool_crash_safety {
public:
    /**
     * @brief Enable crash safety for a thread pool
     * @param pool_name Unique name for the thread pool
     * @param pool Reference to the thread pool
     */
    static void enable_for_pool(const std::string& pool_name, class thread_pool& pool);

    /**
     * @brief Register job crash handler
     * @param handler Function to call when a job crashes
     */
    static void set_job_crash_handler(std::function<void(const std::string& pool_name, 
                                                        const crash_context&)> handler);

private:
    static void handle_job_crash(const std::string& pool_name, const crash_context& context);
};

} // namespace thread_module