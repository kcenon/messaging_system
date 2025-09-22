/**
 * @file logger_types.h
 * @brief Common types and enumerations for logger system
 * @date 2025-09-09
 *
 * BSD 3-Clause License
 * Copyright (c) 2025, kcenon
 * All rights reserved.
 */

#pragma once

#include <chrono>
#include <string>

namespace logger_system {

/**
 * @brief Log levels enumeration
 */
enum class log_level {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    warning = 3,  // Alias for warn for compatibility
    error = 4,
    fatal = 5,
    critical = 5, // Alias for fatal for compatibility
    off = 6
};

/**
 * @brief Overflow policy for when buffers are full
 */
enum class overflow_policy {
    block,          // Block until space is available
    drop_oldest,    // Drop oldest messages
    drop_newest,    // Drop newest messages  
    grow           // Dynamically grow buffer
};

/**
 * @brief Health status enumeration
 */
enum class health_status {
    healthy,        // System is operating normally
    degraded,       // System has some issues but still operational
    unhealthy,      // System has serious issues
    critical        // System is in critical state
};

/**
 * @brief Logger error codes
 */
enum class logger_error_code {
    success = 0,
    invalid_configuration,
    writer_not_found,
    writer_already_exists,
    write_failed,
    flush_failed,
    buffer_full,
    invalid_level,
    invalid_pattern,
    file_open_failed,
    network_error,
    encryption_error,
    compression_error,
    monitoring_failed,
    component_not_found,
    di_resolution_failed,
    health_check_failed,
    writer_closed,
    unknown_error
};

/**
 * @brief Convert log level to string
 */
inline const char* log_level_to_string(log_level level) {
    switch (level) {
        case log_level::trace: return "TRACE";
        case log_level::debug: return "DEBUG";
        case log_level::info:  return "INFO";
        case log_level::warn:  return "WARN";
        case log_level::error: return "ERROR";
        case log_level::fatal: return "FATAL";
        case log_level::off:   return "OFF";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to log level
 */
inline log_level string_to_log_level(const std::string& str) {
    if (str == "TRACE" || str == "trace") return log_level::trace;
    if (str == "DEBUG" || str == "debug") return log_level::debug;
    if (str == "INFO" || str == "info") return log_level::info;
    if (str == "WARN" || str == "warn" || str == "WARNING" || str == "warning") return log_level::warn;
    if (str == "ERROR" || str == "error") return log_level::error;
    if (str == "FATAL" || str == "fatal") return log_level::fatal;
    if (str == "OFF" || str == "off") return log_level::off;
    return log_level::info;  // Default
}

} // namespace logger_system

// Compatibility with kcenon::logger namespace
namespace kcenon::logger {
    using namespace logger_system;
}

// Note: kcenon::thread::log_level is defined in logger_interface.h
// to avoid circular dependencies and maintain compatibility