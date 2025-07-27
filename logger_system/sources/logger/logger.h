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

// Import interface from thread_system
#include "logger_interface.h"
#include "metrics/logger_metrics.h"

namespace logger_module {

// Re-export log_level from thread_module for convenience
using thread_module::log_level;

// Forward declarations
class log_collector;
class base_writer;
class logger_metrics_collector;
class log_filter;
class log_router;

/**
 * @brief Main logger implementation that implements thread_system's logger_interface
 * 
 * This logger provides:
 * - Lock-free logging for high performance
 * - Multiple writer support (console, file, custom)
 * - Asynchronous processing
 * - Thread-safe operations
 */
class logger : public thread_module::logger_interface {
public:
    /**
     * @brief Constructor with optional configuration
     * @param async Enable asynchronous logging (default: true)
     * @param buffer_size Size of the log buffer (default: 8192)
     */
    explicit logger(bool async = true, std::size_t buffer_size = 8192);
    
    /**
     * @brief Destructor - ensures all logs are flushed
     */
    ~logger() override;
    
    // Implement logger_interface
    void log(thread_module::log_level level, const std::string& message) override;
    
    void log(thread_module::log_level level, const std::string& message,
             const std::string& file, int line, const std::string& function) override;
    
    bool is_enabled(thread_module::log_level level) const override;
    
    void flush() override;
    
    // Additional logger-specific methods
    
    /**
     * @brief Add a writer to output logs
     * @param writer Unique pointer to the writer
     */
    void add_writer(std::unique_ptr<base_writer> writer);
    
    /**
     * @brief Remove all writers
     */
    void clear_writers();
    
    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log
     */
    void set_min_level(thread_module::log_level level);
    
    /**
     * @brief Get the minimum log level
     * @return Current minimum log level
     */
    thread_module::log_level get_min_level() const;
    
    /**
     * @brief Start the logger (for async mode)
     */
    void start();
    
    /**
     * @brief Stop the logger
     */
    void stop();
    
    /**
     * @brief Check if logger is running
     * @return true if running
     */
    bool is_running() const;
    
    /**
     * @brief Enable or disable metrics collection
     * @param enable true to enable metrics collection
     */
    void enable_metrics_collection(bool enable = true);
    
    /**
     * @brief Check if metrics collection is enabled
     * @return true if metrics collection is enabled
     */
    bool is_metrics_collection_enabled() const;
    
    /**
     * @brief Get current performance metrics
     * @return Snapshot of current metrics
     */
    performance_metrics get_current_metrics() const;
    
    /**
     * @brief Get metrics history for a specific duration
     * @param duration How far back to retrieve metrics
     * @return Unique pointer to metrics snapshot (for Phase 1, only current snapshot)
     */
    std::unique_ptr<performance_metrics> get_metrics_history(std::chrono::seconds duration) const;
    
    /**
     * @brief Reset performance metrics
     */
    void reset_metrics();
    
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
    
private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace logger_module