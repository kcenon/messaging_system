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

#include <string>
#include <chrono>

// Conditional include based on build mode
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    #include <kcenon/thread/interfaces/logger_interface.h>
#else
    #include <kcenon/logger/interfaces/logger_interface.h>
#endif

#include <kcenon/logger/core/error_codes.h>
#include "../interfaces/log_writer_interface.h"
#include "../interfaces/log_entry.h"

/**
 * @file base_writer.h
 * @brief Abstract base class for all log output writers
 * @author 🍀☀🌕🌥 🌊
 * @since 1.0.0
 * 
 * @details This file defines the abstract base class for all log writers in the system.
 * Writers are responsible for outputting formatted log messages to various destinations
 * such as console, files, network endpoints, databases, or custom targets.
 * 
 * The base_writer class provides:
 * - A common interface for all writer implementations
 * - Compatibility layer between legacy and modern APIs
 * - Built-in formatting utilities for consistent output
 * - Color support for terminals that support ANSI codes
 * 
 * @note Writers must be thread-safe if used in async logging mode.
 * 
 * @example Implementing a custom writer:
 * @code
 * class custom_writer : public base_writer {
 * public:
 *     result_void write(logger_system::log_level level,
 *                      const std::string& message,
 *                      const std::string& file,
 *                      int line,
 *                      const std::string& function,
 *                      const std::chrono::system_clock::time_point& timestamp) override {
 *         // Format the message
 *         std::string formatted = format_log_entry(level, message, file, line, function, timestamp);
 *         
 *         // Write to your custom destination
 *         if (!write_to_destination(formatted)) {
 *             return logger_error_code::write_failed;
 *         }
 *         
 *         return {}; // Success
 *     }
 *     
 *     result_void flush() override {
 *         // Flush any buffered data
 *         return flush_destination();
 *     }
 *     
 *     std::string get_name() const override {
 *         return "custom_writer";
 *     }
 * };
 * @endcode
 */

namespace kcenon::logger {

/**
 * @class base_writer
 * @brief Abstract base class for all log output writers
 * 
 * @details Writers are responsible for outputting log messages to various destinations.
 * This class provides a compatibility layer between the old API and the new
 * interface-based approach, implementing log_writer_interface while providing
 * backward compatibility for existing code.
 * 
 * Key responsibilities:
 * - Format log messages for output
 * - Handle destination-specific output logic
 * - Manage buffering and flushing
 * - Provide health status information
 * 
 * @note All derived classes must implement the pure virtual methods.
 * 
 * @warning Writers used in async mode must be thread-safe.
 * 
 * @since 1.0.0
 */
class base_writer : public log_writer_interface {
public:
    virtual ~base_writer() = default;
    
    /**
     * @brief Write a log entry using the new interface
     * @param entry The log entry to write
     * @return result_void Success or error code
     * 
     * @details This method provides compatibility with the modern log_entry structure.
     * The default implementation converts the entry to the legacy API format for
     * backward compatibility. Derived classes can override this for optimized handling.
     * 
     * @note This method extracts source location information if present in the entry.
     * 
     * @since 1.0.0
     */
    virtual result_void write(const log_entry& entry) override {
        // Convert to old API for backward compatibility
        std::string file = entry.location ? entry.location->file.to_string() : "";
        int line = entry.location ? entry.location->line : 0;
        std::string function = entry.location ? entry.location->function.to_string() : "";
        
        return write(entry.level, entry.message.to_string(), file, line, function, entry.timestamp);
    }
    
    /**
     * @brief Write a log entry (legacy API for backward compatibility)
     * @param level Log severity level
     * @param message Log message text
     * @param file Source file path (empty if not available)
     * @param line Source line number (0 if not available)
     * @param function Function name (empty if not available)
     * @param timestamp Time when the log entry was created
     * @return result_void Success or error code
     * 
     * @details This is the main method that derived classes must implement.
     * It receives all log information and is responsible for outputting it
     * to the writer's specific destination.
     * 
     * @note Implementations should handle empty file/function strings gracefully.
     * 
     * @warning This method may be called from multiple threads in async mode.
     * Implementations must be thread-safe.
     * 
     * @since 1.0.0
     */
    virtual result_void write(logger_system::log_level level,
                              const std::string& message,
                              const std::string& file,
                              int line,
                              const std::string& function,
                              const std::chrono::system_clock::time_point& timestamp) = 0;
    
    /**
     * @brief Flush any buffered data to the destination
     * @return result_void Success or error code
     * 
     * @details Forces any buffered log messages to be written immediately.
     * This is important for ensuring data persistence before shutdown or
     * when immediate output is required.
     * 
     * @note For unbuffered writers, this can be a no-op returning success.
     * 
     * @example
     * @code
     * // Ensure all logs are written before critical operation
     * writer->flush();
     * perform_critical_operation();
     * @endcode
     * 
     * @since 1.0.0
     */
    virtual result_void flush() override = 0;
    
    /**
     * @brief Set whether to use color output (if supported)
     * @param use_color true to enable color output, false to disable
     * 
     * @details Enables or disables ANSI color codes in formatted output.
     * Only affects writers that output to terminals supporting color.
     * 
     * @note Has no effect on writers that don't support color (e.g., file writers).
     * 
     * @since 1.0.0
     */
    virtual void set_use_color(bool use_color) {
        use_color_ = use_color;
    }
    
    /**
     * @brief Get current color output setting
     * @return true if color output is enabled, false otherwise
     * 
     * @since 1.0.0
     */
    bool use_color() const {
        return use_color_;
    }
    
    /**
     * @brief Get the unique name of this writer
     * @return String identifier for this writer instance
     * 
     * @details Returns a unique name identifying this writer. Used for
     * writer management, debugging, and configuration.
     * 
     * @example Common names: "console", "file", "syslog", "network"
     * 
     * @since 1.0.0
     */
    virtual std::string get_name() const override = 0;
    
    /**
     * @brief Check if the writer is healthy and operational
     * @return true if the writer is functioning correctly, false otherwise
     * 
     * @details Used for health monitoring and automatic failover. A writer
     * might be unhealthy if its destination is unavailable (e.g., disk full,
     * network disconnected).
     * 
     * @note Default implementation always returns true. Override for writers
     * that can detect failure conditions.
     * 
     * @since 1.0.0
     */
    virtual bool is_healthy() const override { return true; }
    
protected:
    /**
     * @brief Format a log entry to a human-readable string
     * @param level Log severity level
     * @param message Log message text
     * @param file Source file path
     * @param line Source line number
     * @param function Function name
     * @param timestamp Time of log entry
     * @return Formatted string ready for output
     * 
     * @details Provides a default formatting implementation that derived classes
     * can use. The format includes timestamp, level, message, and optional source
     * location. Color codes are included if color output is enabled.
     * 
     * @note Format: "[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message [file:line in function()]"
     * 
     * @example
     * @code
     * std::string formatted = format_log_entry(
     *     log_level::error,
     *     "Connection failed",
     *     "network.cpp",
     *     42,
     *     "connect",
     *     std::chrono::system_clock::now()
     * );
     * // Result: "[2025-01-10 14:30:15.123] [ERROR] Connection failed [network.cpp:42 in connect()]"
     * @endcode
     * 
     * @since 1.0.0
     */
    std::string format_log_entry(logger_system::log_level level,
                                const std::string& message,
                                const std::string& file,
                                int line,
                                const std::string& function,
                                const std::chrono::system_clock::time_point& timestamp);
    
    /**
     * @brief Convert log level to string representation
     * @param level Log level to convert
     * @return String representation of the level
     * 
     * @details Maps log levels to human-readable strings:
     * - trace -> "TRACE"
     * - debug -> "DEBUG"
     * - info -> "INFO"
     * - warning -> "WARNING"
     * - error -> "ERROR"
     * 
     * @since 1.0.0
     */
    std::string level_to_string(logger_system::log_level level) const;
    
    /**
     * @brief Get ANSI color code for the specified log level
     * @param level Log level to get color for
     * @return ANSI escape sequence for color, or empty string if color is disabled
     * 
     * @details Returns appropriate ANSI color codes for terminal output:
     * - trace -> Gray/Dim
     * - debug -> Cyan
     * - info -> Green
     * - warning -> Yellow
     * - error -> Red
     * 
     * @note Returns empty string if use_color() is false.
     * 
     * @warning Only use for terminals that support ANSI escape codes.
     * 
     * @since 1.0.0
     */
    std::string level_to_color(logger_system::log_level level) const;
    
private:
    bool use_color_ = true;
};

} // namespace kcenon::logger