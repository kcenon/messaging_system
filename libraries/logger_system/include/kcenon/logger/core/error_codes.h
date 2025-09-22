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
#include <stdexcept>
#include <utility>

// Use standalone error handling for better compatibility
#include <optional>
#include <variant>

namespace kcenon::logger {

/**
 * @enum logger_error_code
 * @brief Error codes specific to the logger system
 * 
 * These error codes extend the thread_module error codes with
 * logger-specific error conditions.
 */
enum class logger_error_code {
    // General errors (0-999)
    success = 0,
    unknown_error = 1,
    not_implemented = 2,
    invalid_argument = 3,
    
    // Writer errors (1000-1099)
    writer_not_found = 1000,
    writer_initialization_failed = 1001,
    writer_already_exists = 1002,
    writer_not_healthy = 1003,
    
    // File errors (1100-1199)
    file_open_failed = 1100,
    file_write_failed = 1101,
    file_rotation_failed = 1102,
    file_permission_denied = 1103,
    
    // Network errors (1200-1299)
    network_connection_failed = 1200,
    network_send_failed = 1201,
    network_timeout = 1202,
    
    // Buffer/Queue errors (1300-1399)
    buffer_overflow = 1300,
    queue_full = 1301,
    queue_stopped = 1302,
    
    // Configuration errors (1400-1499)
    invalid_configuration = 1400,
    configuration_missing = 1401,
    configuration_conflict = 1402,
    
    // Metrics errors (1500-1599)
    metrics_collection_failed = 1500,
    metrics_not_available = 1501,
    
    // Processing errors (1600-1699)
    flush_timeout = 1600,
    processing_failed = 1601,
    filter_error = 1602,
    formatter_error = 1603,
    
    // Security errors (1700-1799)
    encryption_failed = 1700,
    decryption_failed = 1701,
    authentication_failed = 1702,
    sanitization_failed = 1703,
    
    // DI Container errors (1800-1899)
    di_not_available = 1800,
    component_not_found = 1801,
    registration_failed = 1802,
    creation_failed = 1803,
    operation_failed = 1804,

    // Writer errors (1900-1999)
    writer_not_available = 1900,
    writer_configuration_error = 1901,
    writer_operation_failed = 1902
};

/**
 * @brief Convert logger_error_code to string representation
 * @param code The error code to convert
 * @return Human-readable error description
 */
inline std::string logger_error_to_string(logger_error_code code) {
    switch (code) {
        case logger_error_code::success:
            return "Success";
        case logger_error_code::unknown_error:
            return "Unknown error";
        case logger_error_code::not_implemented:
            return "Not implemented";
        case logger_error_code::invalid_argument:
            return "Invalid argument";
            
        // Writer errors
        case logger_error_code::writer_not_found:
            return "Writer not found";
        case logger_error_code::writer_initialization_failed:
            return "Writer initialization failed";
        case logger_error_code::writer_already_exists:
            return "Writer already exists";
        case logger_error_code::writer_not_healthy:
            return "Writer not healthy";
            
        // File errors
        case logger_error_code::file_open_failed:
            return "Failed to open file";
        case logger_error_code::file_write_failed:
            return "Failed to write to file";
        case logger_error_code::file_rotation_failed:
            return "File rotation failed";
        case logger_error_code::file_permission_denied:
            return "File permission denied";
            
        // Network errors
        case logger_error_code::network_connection_failed:
            return "Network connection failed";
        case logger_error_code::network_send_failed:
            return "Network send failed";
        case logger_error_code::network_timeout:
            return "Network timeout";
            
        // Buffer/Queue errors
        case logger_error_code::buffer_overflow:
            return "Buffer overflow";
        case logger_error_code::queue_full:
            return "Queue is full";
        case logger_error_code::queue_stopped:
            return "Queue is stopped";
            
        // Configuration errors
        case logger_error_code::invalid_configuration:
            return "Invalid configuration";
        case logger_error_code::configuration_missing:
            return "Configuration missing";
        case logger_error_code::configuration_conflict:
            return "Configuration conflict";
            
        // Metrics errors
        case logger_error_code::metrics_collection_failed:
            return "Metrics collection failed";
        case logger_error_code::metrics_not_available:
            return "Metrics not available";
            
        // Processing errors
        case logger_error_code::flush_timeout:
            return "Flush timeout";
        case logger_error_code::processing_failed:
            return "Processing failed";
        case logger_error_code::filter_error:
            return "Filter error";
        case logger_error_code::formatter_error:
            return "Formatter error";
            
        // Security errors
        case logger_error_code::encryption_failed:
            return "Encryption failed";
        case logger_error_code::decryption_failed:
            return "Decryption failed";
        case logger_error_code::authentication_failed:
            return "Authentication failed";
        case logger_error_code::sanitization_failed:
            return "Sanitization failed";
            
        // DI Container errors
        case logger_error_code::di_not_available:
            return "DI container not available";
        case logger_error_code::component_not_found:
            return "Component not found in DI container";
        case logger_error_code::registration_failed:
            return "Failed to register component in DI container";
        case logger_error_code::creation_failed:
            return "Failed to create component from factory";
        case logger_error_code::operation_failed:
            return "DI container operation failed";
            
        default:
            return "Unknown logger error code";
    }
}

// Use standalone mode for better compatibility and fewer dependencies
// Standalone mode - provide minimal result implementation
template<typename T>
class result {
public:
    result(T value) : value_(std::move(value)), has_value_(true) {}
    result(logger_error_code code, const std::string& msg = "") 
        : error_code_(code), error_msg_(msg), has_value_(false) {}
    
    bool has_value() const { return has_value_; }
    explicit operator bool() const { return has_value_; }
    
    T& value() { 
        if (!has_value_) throw std::runtime_error("No value");
        return value_;
    }
    const T& value() const {
        if (!has_value_) throw std::runtime_error("No value");
        return value_;
    }
    
    logger_error_code error_code() const { return error_code_; }
    const std::string& error_message() const { return error_msg_; }
    
private:
    T value_{};
    logger_error_code error_code_{logger_error_code::success};
    std::string error_msg_;
    bool has_value_;
};

class result_void {
public:
    result_void() = default;
    result_void(logger_error_code code, const std::string& msg = "")
        : error_code_(code), error_msg_(msg), has_error_(true) {}
    
    bool has_error() const { return has_error_; }
    explicit operator bool() const { return !has_error_; }
    
    logger_error_code error_code() const { return error_code_; }
    const std::string& error_message() const { return error_msg_; }
    
private:
    logger_error_code error_code_{logger_error_code::success};
    std::string error_msg_;
    bool has_error_{false};
};

inline result_void make_logger_error(logger_error_code code, const std::string& message = "") {
    return result_void{code, message.empty() ? logger_error_to_string(code) : message};
}

// Template version for result<T>
template<typename T>
inline result<T> make_logger_error(logger_error_code code, const std::string& message = "") {
    return result<T>{code, message.empty() ? logger_error_to_string(code) : message};
}

// Type alias for convenience in standalone mode
using error_code = logger_error_code;

} // namespace kcenon::logger