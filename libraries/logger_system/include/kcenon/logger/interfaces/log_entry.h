#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file log_entry.h
 * @brief Data structures for representing log entries and source locations
 * @author 🍀☀🌕🌥 🌊
 * @since 1.0.0
 * 
 * @details This file defines the core data structures used to represent log messages
 * throughout the logging system. The structures are optimized for performance using
 * small string optimization (SSO) and move semantics.
 * 
 * @note The log_entry structure is move-only to avoid unnecessary copying of
 * log messages, which improves performance in high-throughput scenarios.
 * 
 * @example Creating a log entry:
 * @code
 * // Simple log entry
 * log_entry entry(log_level::info, "Application started");
 * 
 * // Log entry with source location
 * log_entry detailed_entry(
 *     log_level::error,
 *     "Database connection failed",
 *     __FILE__,
 *     __LINE__,
 *     __FUNCTION__
 * );
 * 
 * // Log entry with optional fields
 * log_entry custom_entry(log_level::debug, "Debug message");
 * custom_entry.category = "database";
 * custom_entry.thread_id = std::to_string(std::this_thread::get_id());
 * @endcode
 */

#include <string>
#include <chrono>
#include <optional>
#include "../core/small_string.h"

// Conditional include based on build mode
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    #include <kcenon/thread/interfaces/logger_interface.h>
#else
    #include <kcenon/logger/interfaces/logger_interface.h>
#endif

#include <kcenon/logger/interfaces/logger_types.h>

namespace kcenon::logger {

/**
 * @struct source_location
 * @brief Source code location information for debugging
 * 
 * @details Captures the source file, line number, and function name where
 * a log message originated. This information is invaluable for debugging
 * and tracing issues in production.
 * 
 * The structure uses small string optimization (SSO) to avoid heap allocations
 * for typical file paths and function names.
 * 
 * @note File paths up to 256 characters and function names up to 128 characters
 * are stored without heap allocation.
 * 
 * @since 1.0.0
 */
struct source_location {
    /**
     * @brief Source file path
     * @details Uses small_string_256 for efficient storage of file paths
     */
    small_string_256 file;
    
    /**
     * @brief Line number in the source file
     * @details Set to 0 if line information is not available
     */
    int line;
    
    /**
     * @brief Function or method name
     * @details Uses small_string_128 for efficient storage of function names
     */
    small_string_128 function;
    
    /**
     * @brief Construct source location from std::string
     * @param f Source file path (default: empty)
     * @param l Line number (default: 0)
     * @param func Function name (default: empty)
     */
    source_location(const std::string& f = "", int l = 0, const std::string& func = "")
        : file(f), line(l), function(func) {}
    
    /**
     * @brief Construct source location from C-strings
     * @param f Source file path (default: empty)
     * @param l Line number (default: 0)
     * @param func Function name (default: empty)
     * 
     * @note This overload avoids temporary std::string creation when using
     * __FILE__ and __FUNCTION__ macros.
     */
    source_location(const char* f = "", int l = 0, const char* func = "")
        : file(f), line(l), function(func) {}
};

/**
 * @struct log_entry
 * @brief Represents a single log entry with all associated metadata
 * 
 * @details The log_entry structure is the core data type that flows through
 * the logging system. It contains the log message, severity level, timestamp,
 * and optional metadata such as source location and thread information.
 * 
 * The structure is designed for high performance:
 * - Move-only semantics to avoid copying
 * - Small string optimization for common string sizes
 * - Optional fields to minimize memory usage
 * 
 * @note This structure is move-only. Use std::move() when passing log entries
 * to avoid compilation errors.
 * 
 * @warning Copy operations are explicitly deleted to prevent performance degradation
 * from accidental copies in high-throughput scenarios.
 * 
 * @since 1.0.0
 */
struct log_entry {
    // Required fields
    
    /**
     * @brief Severity level of the log message
     * @details Determines the importance and routing of the message
     */
    logger_system::log_level level;
    
    /**
     * @brief The actual log message
     * @details Uses small_string_256 for efficient storage of messages up to 256 characters
     * @note Messages longer than 256 characters will use heap allocation
     */
    small_string_256 message;
    
    /**
     * @brief Timestamp when the log entry was created
     * @details Uses system clock for wall-clock time representation
     */
    std::chrono::system_clock::time_point timestamp;
    
    // Optional fields
    
    /**
     * @brief Optional source code location information
     * @details Present when log() is called with file/line/function parameters
     */
    std::optional<source_location> location;
    
    /**
     * @brief Optional thread identifier
     * @details Can be set to track which thread generated the log message
     * @note Thread IDs are stored efficiently using small_string_64
     */
    std::optional<small_string_64> thread_id;
    
    /**
     * @brief Optional category for log filtering and routing
     * @details Allows grouping related log messages (e.g., "database", "network", "security")
     * @note Categories are stored efficiently using small_string_128
     */
    std::optional<small_string_128> category;
    
    /**
     * @brief Constructor for basic log entry
     * @param lvl Log severity level
     * @param msg Log message
     * @param ts Timestamp (default: current time)
     * 
     * @details Creates a log entry with the minimum required information.
     * The timestamp defaults to the current system time if not specified.
     * 
     * @example
     * @code
     * log_entry entry(log_level::info, "Server started on port 8080");
     * @endcode
     * 
     * @since 1.0.0
     */
    log_entry(logger_system::log_level lvl, 
              const std::string& msg,
              std::chrono::system_clock::time_point ts = std::chrono::system_clock::now())
        : level(lvl), message(msg), timestamp(ts) {}
    
    /**
     * @brief Constructor with source location information
     * @param lvl Log severity level
     * @param msg Log message
     * @param file Source file path
     * @param line Line number in source file
     * @param function Function name
     * @param ts Timestamp (default: current time)
     * 
     * @details Creates a log entry with complete source location information.
     * This constructor is typically used with __FILE__, __LINE__, and __FUNCTION__ macros.
     * 
     * @example
     * @code
     * log_entry entry(
     *     log_level::error,
     *     "Failed to connect to database",
     *     __FILE__,
     *     __LINE__,
     *     __FUNCTION__
     * );
     * @endcode
     * 
     * @since 1.0.0
     */
    log_entry(logger_system::log_level lvl,
              const std::string& msg,
              const std::string& file,
              int line,
              const std::string& function,
              std::chrono::system_clock::time_point ts = std::chrono::system_clock::now())
        : level(lvl), message(msg), timestamp(ts),
          location(source_location{file, line, function}) {}
    
    /**
     * @brief Move constructor
     * @details Enables efficient transfer of log entries without copying
     * @since 1.0.0
     */
    log_entry(log_entry&&) noexcept = default;
    
    /**
     * @brief Move assignment operator
     * @details Enables efficient transfer of log entries without copying
     * @since 1.0.0
     */
    log_entry& operator=(log_entry&&) noexcept = default;
    
    /**
     * @brief Deleted copy constructor
     * @details Prevents accidental copying for performance reasons
     * @since 1.0.0
     */
    log_entry(const log_entry&) = delete;
    
    /**
     * @brief Deleted copy assignment operator
     * @details Prevents accidental copying for performance reasons
     * @since 1.0.0
     */
    log_entry& operator=(const log_entry&) = delete;
};

} // namespace kcenon::logger