#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <memory>
#include <vector>
#include <atomic>
#include <thread>

// Conditional include based on build mode
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    // Integration mode: Use thread_system's interface for ecosystem compatibility
    #include <kcenon/thread/interfaces/logger_interface.h>
#else
    // Standalone mode: Use local interface for independence
    #include <kcenon/logger/interfaces/logger_interface.h>
#endif

#include "error_codes.h"
#include "metrics/logger_metrics.h"
#include "di/di_container_interface.h"
#include "di/di_container_factory.h"
#include "monitoring/monitoring_interface.h"
#include "monitoring/monitoring_factory.h"
#include <kcenon/logger/interfaces/logger_types.h>

/**
 * @file logger.h
 * @brief High-performance, thread-safe logging system with asynchronous capabilities
 * @author 🍀☀🌕🌥 🌊
 * @since 1.0.0
 * 
 * @details This file defines the main logger class that provides a comprehensive
 * logging solution with support for multiple output destinations, asynchronous
 * processing, metrics collection, and dependency injection. The logger is designed
 * to be thread-safe and supports both synchronous and asynchronous operation modes.
 * 
 * @note The logger integrates with the thread_system when USE_THREAD_SYSTEM is defined,
 * providing seamless compatibility with the broader thread management infrastructure.
 * 
 * @example Basic usage:
 * @code
 * // Create a logger with default settings
 * kcenon::logger::logger logger;
 * 
 * // Add a console writer
 * logger.add_writer(std::make_unique<console_writer>());
 * 
 * // Start the logger in async mode
 * logger.start();
 * 
 * // Log messages
 * logger.log(log_level::info, "Application started");
 * logger.log(log_level::error, "An error occurred", __FILE__, __LINE__, __FUNCTION__);
 * 
 * // Flush and stop
 * logger.flush();
 * logger.stop();
 * @endcode
 * 
 * @example Advanced configuration with builder:
 * @code
 * auto result = logger_builder()
 *     .with_async(true)
 *     .with_buffer_size(16384)
 *     .with_min_level(log_level::debug)
 *     .with_metrics(true)
 *     .add_writer("console", std::make_unique<console_writer>())
 *     .add_writer("file", std::make_unique<file_writer>("logs/app.log"))
 *     .build();
 * 
 * if (result) {
 *     auto logger = std::move(result.value());
 *     // Use logger...
 * }
 * @endcode
 */

namespace kcenon::logger {

// Type aliases for consistency across modes
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    // In integration mode, use thread_module types
    using log_level = kcenon::thread::log_level;
#else
    // In standalone mode, use logger_system types
    using log_level = logger_system::log_level;
#endif

// Type aliases for convenience and compatibility
using logger_metrics = metrics::logger_performance_stats;
using performance_metrics = metrics::logger_performance_stats; // Alias for examples
using monitoring_metrics = monitoring::monitoring_data;
using monitoring_interface = monitoring::monitoring_interface;
using di_container_interface = di::di_container_interface;
using di_container_factory = di::di_container_factory;
using monitoring_factory = monitoring::monitoring_factory;

// Metric type enum
enum class metric_type {
    gauge,
    counter,
    histogram
};

// Forward declarations
class log_collector;
class base_writer;
class logger_metrics_collector;
class log_filter;
class log_router;

/**
 * @class logger
 * @brief Main logger implementation that implements thread_system's logger_interface
 * 
 * @details The logger class provides a high-performance, thread-safe logging system with:
 * - Asynchronous logging with configurable batching for optimal throughput
 * - Multiple writer support for outputting to different destinations simultaneously
 * - Real-time metrics collection and performance monitoring
 * - Dependency injection support for flexible writer management
 * - Configurable filtering and routing of log messages
 * - Integration with monitoring backends for production observability
 * 
 * The logger uses the PIMPL idiom to hide implementation details and maintain ABI stability.
 * 
 * @warning When using asynchronous mode, ensure proper shutdown by calling stop() and flush()
 * before destroying the logger to prevent loss of buffered messages.
 * 
 * @since 1.0.0
 */
#ifdef USE_THREAD_SYSTEM_INTEGRATION
class logger : public kcenon::thread::logger_interface {
#else
class logger : public logger_system::logger_interface {
#endif
public:
    /**
     * @brief Constructor with optional configuration
     * @param async Enable asynchronous logging (default: true)
     * @param buffer_size Size of the log buffer in bytes (default: 8192)
     * 
     * @details Creates a logger instance with the specified configuration.
     * In async mode, a background thread is created to process log messages,
     * providing better performance for high-throughput applications.
     * 
     * @note The buffer_size parameter affects memory usage and batching efficiency.
     * Larger buffers can improve throughput but increase memory consumption.
     * 
     * @since 1.0.0
     */
    explicit logger(bool async = true, std::size_t buffer_size = 8192);
    
    /**
     * @brief Destructor - ensures all logs are flushed
     * 
     * @details Properly shuts down the logger, ensuring all buffered messages
     * are written to their destinations before destruction. Automatically calls
     * stop() and flush() if the logger is still running.
     * 
     * @warning Destruction may block until all pending messages are processed.
     * 
     * @since 1.0.0
     */
    ~logger() override;
    
    /**
     * @brief Log a simple message
     * @param level Severity level of the message
     * @param message The message to log
     * 
     * @details Logs a message without source location information.
     * The message is queued for asynchronous processing if async mode is enabled.
     * 
     * @note Messages below the minimum log level are discarded for performance.
     * 
     * @since 1.0.0
     */
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    void log(kcenon::thread::log_level level, const std::string& message) override;
#else
    void log(logger_system::log_level level, const std::string& message) override;
#endif
    
    /**
     * @brief Log a message with source location
     * @param level Severity level of the message
     * @param message The message to log
     * @param file Source file name (typically __FILE__)
     * @param line Line number in source file (typically __LINE__)
     * @param function Function name (typically __FUNCTION__)
     * 
     * @details Logs a message with complete source location information for debugging.
     * This overload is useful for tracking the exact origin of log messages.
     * 
     * @example
     * @code
     * logger.log(log_level::error, "Database connection failed", 
     *           __FILE__, __LINE__, __FUNCTION__);
     * @endcode
     * 
     * @since 1.0.0
     */
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    void log(kcenon::thread::log_level level, const std::string& message,
             const std::string& file, int line, const std::string& function) override;
#else
    void log(logger_system::log_level level, const std::string& message,
             const std::string& file, int line, const std::string& function) override;
#endif
    
    /**
     * @brief Check if a log level is enabled
     * @param level The log level to check
     * @return true if messages at this level will be logged, false otherwise
     * 
     * @details Use this method to avoid expensive message construction
     * for log levels that won't be output.
     * 
     * @example
     * @code
     * if (logger.is_enabled(log_level::debug)) {
     *     std::string expensive_debug_info = gather_debug_data();
     *     logger.log(log_level::debug, expensive_debug_info);
     * }
     * @endcode
     * 
     * @since 1.0.0
     */
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    bool is_enabled(kcenon::thread::log_level level) const override;
#else
    bool is_enabled(logger_system::log_level level) const override;
#endif
    
    /**
     * @brief Flush all pending log messages
     * 
     * @details Forces immediate writing of all buffered messages to their destinations.
     * This is a blocking operation that waits until all messages are processed.
     * 
     * @note In synchronous mode, this is a no-op as messages are written immediately.
     * 
     * @warning May cause performance degradation if called frequently in async mode.
     * 
     * @since 1.0.0
     */
    void flush() override;
    
    // Additional logger-specific methods
    
    /**
     * @brief Add a writer to output logs
     * @param writer Unique pointer to the writer to add
     * @return result_void Success or error code
     * 
     * @details Adds a new output destination for log messages. Multiple writers
     * can be added to send logs to different destinations simultaneously.
     * Ownership of the writer is transferred to the logger.
     * 
     * @note Writers are processed in the order they were added.
     * 
     * @example
     * @code
     * auto result = logger.add_writer(std::make_unique<file_writer>("app.log"));
     * if (!result) {
     *     std::cerr << "Failed to add writer: " << result.error_message() << std::endl;
     * }
     * @endcode
     * 
     * @since 1.0.0
     */
    result_void add_writer(std::unique_ptr<base_writer> writer);
    
    /**
     * @brief Remove all writers
     * @return result_void Success or error code
     * 
     * @details Removes all currently registered writers from the logger.
     * After this call, log messages will not be output anywhere until
     * new writers are added.
     * 
     * @warning This operation cannot be undone. Removed writers are destroyed.
     * 
     * @since 1.0.0
     */
    result_void clear_writers();
    
    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log
     * 
     * @details Sets the threshold for message logging. Messages with a level
     * below this threshold are discarded for performance optimization.
     * 
     * @note This is a thread-safe operation that takes effect immediately.
     * 
     * @example
     * @code
     * // In production, only log warnings and errors
     * logger.set_min_level(log_level::warning);
     * 
     * // In development, log everything
     * logger.set_min_level(log_level::trace);
     * @endcode
     * 
     * @since 1.0.0
     */
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    void set_min_level(kcenon::thread::log_level level);
#else
    void set_min_level(logger_system::log_level level);
#endif
    
    /**
     * @brief Get the minimum log level
     * @return Current minimum log level
     * 
     * @details Returns the current threshold level for message logging.
     * 
     * @since 1.0.0
     */
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    kcenon::thread::log_level get_min_level() const;
#else
    logger_system::log_level get_min_level() const;
#endif
    
    /**
     * @brief Start the logger (for async mode)
     * @return result_void Success or error code
     * 
     * @details Starts the background processing thread for asynchronous logging.
     * This method must be called before logging in async mode. Has no effect
     * in synchronous mode.
     * 
     * @note Calling start() on an already running logger is a no-op.
     * 
     * @warning Not calling start() in async mode will cause log messages to queue
     * indefinitely without being processed.
     * 
     * @example
     * @code
     * kcenon::logger::logger logger(true, 16384); // async mode
     * auto result = logger.start();
     * if (!result) {
     *     std::cerr << "Failed to start logger: " << result.error_message() << std::endl;
     * }
     * @endcode
     * 
     * @since 1.0.0
     */
    result_void start();
    
    /**
     * @brief Stop the logger
     * @return result_void Success or error code
     * 
     * @details Stops the background processing thread and flushes all pending messages.
     * This is a blocking operation that waits for all queued messages to be processed.
     * 
     * @note After stopping, the logger can be restarted with start().
     * 
     * @warning Stopping the logger may take time if there are many pending messages.
     * 
     * @since 1.0.0
     */
    result_void stop();
    
    /**
     * @brief Check if logger is running
     * @return true if the logger is currently running, false otherwise
     * 
     * @details In async mode, returns true if the background processing thread
     * is active. In sync mode, always returns true.
     * 
     * @since 1.0.0
     */
    bool is_running() const;
    
    /**
     * @brief Enable or disable metrics collection
     * @param enable true to enable metrics collection
     * @return result_void indicating success or error
     */
    result_void enable_metrics_collection(bool enable = true);
    
    /**
     * @brief Check if metrics collection is enabled
     * @return true if metrics collection is enabled
     */
    bool is_metrics_collection_enabled() const;
    
    /**
     * @brief Get current performance metrics
     * @return Result containing metrics or error
     */
    result<metrics::logger_performance_stats> get_current_metrics() const;
    
    /**
     * @brief Get metrics history for a specific duration
     * @param duration How far back to retrieve metrics
     * @return Result containing metrics snapshot or error
     */
    result<std::unique_ptr<metrics::logger_performance_stats>> get_metrics_history(std::chrono::seconds duration) const;
    
    /**
     * @brief Reset performance metrics
     * @return result_void indicating success or error
     */
    result_void reset_metrics();
    
    /**
     * @brief Get metrics collector for direct access
     * @return Pointer to metrics collector (may be null if not enabled)
     */
    logger_metrics_collector* get_metrics_collector();
    
    /**
     * @brief Add a writer with a specific name
     * @param name Name for the writer
     * @param writer Unique pointer to the writer
     */
    void add_writer(const std::string& name, std::unique_ptr<base_writer> writer);
    
    /**
     * @brief Remove a writer by name
     * @param name Name of the writer to remove
     * @return true if writer was found and removed
     */
    bool remove_writer(const std::string& name);
    
    /**
     * @brief Get a writer by name
     * @param name Name of the writer
     * @return Pointer to writer or nullptr if not found
     */
    base_writer* get_writer(const std::string& name);
    
    /**
     * @brief Set global filter
     * @param filter Filter to apply to all logs
     */
    void set_filter(std::unique_ptr<log_filter> filter);
    
    /**
     * @brief Get the log router for configuration
     * @return Reference to the log router
     */
    log_router& get_router();
    
    // DI Support Methods
    
    /**
     * @brief Set a DI container for writer resolution
     * @param container Pointer to DI container (not owned)
     */
    void set_di_container(di::di_container_interface* container);
    
    /**
     * @brief Check if DI container is available
     * @return true if DI container is set
     */
    bool has_di_container() const;
    
    /**
     * @brief Add a writer from DI container
     * @param name Name of the writer registered in DI container
     * @return result_void indicating success or error
     */
    result_void add_writer_from_di(const std::string& name);
    
    /**
     * @brief Register a writer factory in the internal DI container
     * @param name Name to register the writer factory under
     * @param factory Factory function to create the writer
     * @return result_void indicating success or error
     */
    result_void register_writer_factory(const std::string& name, std::function<std::shared_ptr<base_writer>()> factory);
    
    /**
     * @brief Get the DI strategy being used
     * @return Current DI strategy
     */
    di::di_container_factory::container_type get_di_strategy() const;
    
    /**
     * @brief Enable internal DI container
     * @param type Type of DI container to use
     * @return result_void indicating success or error
     */
    result_void enable_di(di::di_container_factory::container_type type =
                         di::di_container_factory::container_type::automatic);
    
    // Monitoring Support Methods
    
    /**
     * @brief Set a custom monitoring implementation
     * @param monitor Unique pointer to monitoring implementation
     */
    void set_monitor(std::unique_ptr<monitoring::monitoring_interface> monitor);
    
    /**
     * @brief Enable monitoring with specified backend
     * @param type Type of monitoring backend to use
     * @return result_void indicating success or error
     */
    result_void enable_monitoring(monitoring::monitoring_factory::monitor_type type =
                                 monitoring::monitoring_factory::monitor_type::automatic);
    
    /**
     * @brief Disable monitoring
     * @return result_void indicating success or error
     */
    result_void disable_monitoring();
    
    /**
     * @brief Check if monitoring is enabled
     * @return true if monitoring is enabled
     */
    bool is_monitoring_enabled() const;
    
    /**
     * @brief Collect current metrics
     * @return Result containing monitoring data or error
     */
    result<monitoring::monitoring_data> collect_metrics() const;
    
    /**
     * @brief Perform health check
     * @return Result containing health check result or error
     */
    result<health_status> check_health() const;
    
    /**
     * @brief Reset monitoring metrics
     * @return result_void indicating success or error
     */
    result_void reset_monitoring_metrics();
    
    /**
     * @brief Get the monitoring backend name
     * @return Name of the current monitoring backend
     */
    std::string get_monitoring_backend() const;
    
    /**
     * @brief Record a custom metric
     * @param name Metric name
     * @param value Metric value
     * @param type Metric type
     */
    void record_metric(const std::string& name, double value, 
                      metric_type type = metric_type::gauge);
    
private:
    class impl;
    std::unique_ptr<impl> pimpl_;
    
    // DI support members
    di::di_container_interface* external_di_container_ = nullptr;
    std::unique_ptr<di::di_container_interface> internal_di_container_;
    
    // Monitoring support member
    std::unique_ptr<monitoring::monitoring_interface> monitor_;
};

} // namespace kcenon::logger
