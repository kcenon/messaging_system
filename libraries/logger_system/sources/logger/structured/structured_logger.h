#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../logger_interface.h"
#include <any>
#include <chrono>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <iomanip>
// #include <nlohmann/json.hpp> // TODO: Add nlohmann/json dependency

// Platform-specific includes for hostname and process ID
#ifdef _WIN32
    #include <WinSock2.h>
    #include <Windows.h>
#else
    #include <unistd.h>
#endif

#include <thread>
#include <cstdlib>

namespace logger_module {

// Re-export types from thread_module
using thread_module::log_level;
using thread_module::logger_interface;

// Forward declaration
class structured_logger;

/**
 * @class log_entry
 * @brief Builder pattern for structured log entries
 */
class log_entry {
public:
    log_entry(structured_logger* logger, log_level level, const std::string& message)
        : logger_(logger), level_(level), message_(message) {
        // Automatically add timestamp
        timestamp_ = std::chrono::system_clock::now();
    }
    
    // Add field to the log entry
    template<typename T>
    log_entry& field(const std::string& key, const T& value) {
        fields_[key] = value;
        return *this;
    }
    
    // Add context field
    template<typename T>
    log_entry& context(const std::string& key, const T& value) {
        context_[key] = value;
        return *this;
    }
    
    // Set custom timestamp
    log_entry& timestamp(std::chrono::system_clock::time_point tp) {
        timestamp_ = tp;
        return *this;
    }
    
    // Add error information
    log_entry& error(const std::exception& e) {
        fields_["error_type"] = typeid(e).name();
        fields_["error_message"] = e.what();
        return *this;
    }
    
    // Add duration
    log_entry& duration(std::chrono::nanoseconds ns) {
        fields_["duration_ns"] = ns.count();
        fields_["duration_ms"] = std::chrono::duration<double, std::milli>(ns).count();
        return *this;
    }
    
    // Commit the log entry
    void commit();
    
private:
    structured_logger* logger_;
    log_level level_;
    std::string message_;
    std::chrono::system_clock::time_point timestamp_;
    std::unordered_map<std::string, std::any> fields_;
    std::unordered_map<std::string, std::any> context_;
    
    friend class structured_logger;
};

/**
 * @struct standard_fields
 * @brief Standard field names for structured logging
 */
struct standard_fields {
    static constexpr auto TIMESTAMP = "@timestamp";
    static constexpr auto LEVEL = "level";
    static constexpr auto MESSAGE = "message";
    static constexpr auto LOGGER = "logger";
    static constexpr auto THREAD_ID = "thread_id";
    static constexpr auto PROCESS_ID = "process_id";
    static constexpr auto HOST = "host";
    static constexpr auto SERVICE = "service";
    static constexpr auto VERSION = "version";
    static constexpr auto ENVIRONMENT = "environment";
};

/**
 * @class structured_logger
 * @brief Logger with structured logging support
 */
class structured_logger {
public:
    enum class output_format {
        json,
        logfmt,  // key=value format
        plain    // traditional format
    };
    
    structured_logger(std::shared_ptr<logger_interface> logger, 
                     output_format format = output_format::json)
        : logger_(logger), format_(format) {
        initialize_defaults();
    }
    
    // Structured logging methods
    log_entry trace(const std::string& message) {
        return log_entry(this, log_level::trace, message);
    }
    
    log_entry debug(const std::string& message) {
        return log_entry(this, log_level::debug, message);
    }
    
    log_entry info(const std::string& message) {
        return log_entry(this, log_level::info, message);
    }
    
    log_entry warning(const std::string& message) {
        return log_entry(this, log_level::warning, message);
    }
    
    log_entry error(const std::string& message) {
        return log_entry(this, log_level::error, message);
    }
    
    log_entry critical(const std::string& message) {
        return log_entry(this, log_level::critical, message);
    }
    
    // Set global context
    template<typename T>
    void set_context(const std::string& key, const T& value) {
        global_context_[key] = value;
    }
    
    // Clear global context
    void clear_context() {
        global_context_.clear();
    }
    
    // Set output format
    void set_format(output_format format) {
        format_ = format;
    }
    
    // Set service information
    void set_service_info(const std::string& service_name,
                         const std::string& version = "",
                         const std::string& environment = "") {
        if (!service_name.empty()) global_context_["service"] = service_name;
        if (!version.empty()) global_context_["version"] = version;
        if (!environment.empty()) global_context_["environment"] = environment;
    }
    
private:
    std::shared_ptr<logger_interface> logger_;
    output_format format_;
    std::unordered_map<std::string, std::any> global_context_;
    
    void initialize_defaults() {
        // Add default fields
#ifdef _WIN32
        global_context_[standard_fields::PROCESS_ID] = std::to_string(GetCurrentProcessId());

        // Initialize Winsock for hostname
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);

        // Get hostname
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            global_context_[standard_fields::HOST] = std::string(hostname);
        }

        WSACleanup();
#else
        global_context_[standard_fields::PROCESS_ID] = std::to_string(getpid());

        // Get hostname
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            global_context_[standard_fields::HOST] = std::string(hostname);
        }
#endif
    }
    
    // Format log entry based on output format
    std::string format_entry(const log_entry& entry) const {
        switch (format_) {
            case output_format::json:
                return format_json(entry);
            case output_format::logfmt:
                return format_logfmt(entry);
            case output_format::plain:
                return format_plain(entry);
            default:
                return format_json(entry);
        }
    }
    
    // Format as JSON (simple implementation without nlohmann/json)
    std::string format_json(const log_entry& entry) const {
        std::ostringstream oss;
        oss << "{";
        
        // Add timestamp
        auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp_);
        oss << "\"" << standard_fields::TIMESTAMP << "\":\"";
        oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ") << "\",";
        
        // Add standard fields
        oss << "\"" << standard_fields::LEVEL << "\":\"" << level_to_string(entry.level_) << "\",";
        oss << "\"" << standard_fields::MESSAGE << "\":\"" << escape_json(entry.message_) << "\",";
        oss << "\"" << standard_fields::THREAD_ID << "\":\"" 
            << std::hash<std::thread::id>{}(std::this_thread::get_id()) << "\"";
        
        // Add global context
        for (const auto& [key, value] : global_context_) {
            oss << ",\"" << key << "\":" << any_to_json_string(value);
        }
        
        // Add entry context
        if (!entry.context_.empty()) {
            oss << ",\"context\":{";
            bool first = true;
            for (const auto& [key, value] : entry.context_) {
                if (!first) oss << ",";
                oss << "\"" << key << "\":" << any_to_json_string(value);
                first = false;
            }
            oss << "}";
        }
        
        // Add fields
        for (const auto& [key, value] : entry.fields_) {
            oss << ",\"" << key << "\":" << any_to_json_string(value);
        }
        
        oss << "}";
        return oss.str();
    }
    
    // Format as logfmt
    std::string format_logfmt(const log_entry& entry) const {
        std::ostringstream oss;
        
        // Add timestamp
        auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp_);
        oss << "timestamp=\"" << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ") << "\" ";
        
        // Add standard fields
        oss << "level=" << level_to_string(entry.level_) << " ";
        oss << "message=\"" << escape_string(entry.message_) << "\" ";
        oss << "thread_id=" << std::hash<std::thread::id>{}(std::this_thread::get_id()) << " ";
        
        // Add all fields
        auto add_fields = [&oss, this](const auto& fields) {
            for (const auto& [key, value] : fields) {
                oss << key << "=" << format_value_logfmt(value) << " ";
            }
        };
        
        add_fields(global_context_);
        add_fields(entry.context_);
        add_fields(entry.fields_);
        
        return oss.str();
    }
    
    // Format as plain text
    std::string format_plain(const log_entry& entry) const {
        std::ostringstream oss;
        
        // Traditional format with structured data appended
        oss << entry.message_;
        
        // Add fields if present
        if (!entry.fields_.empty()) {
            oss << " [";
            bool first = true;
            for (const auto& [key, value] : entry.fields_) {
                if (!first) oss << ", ";
                oss << key << "=" << format_value_plain(value);
                first = false;
            }
            oss << "]";
        }
        
        return oss.str();
    }
    
    // Convert std::any to JSON string value
    std::string any_to_json_string(const std::any& value) const {
        if (value.type() == typeid(int)) {
            return std::to_string(std::any_cast<int>(value));
        } else if (value.type() == typeid(long)) {
            return std::to_string(std::any_cast<long>(value));
        } else if (value.type() == typeid(uint64_t)) {
            return std::to_string(std::any_cast<uint64_t>(value));
        } else if (value.type() == typeid(double)) {
            return std::to_string(std::any_cast<double>(value));
        } else if (value.type() == typeid(float)) {
            return std::to_string(std::any_cast<float>(value));
        } else if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? "true" : "false";
        } else if (value.type() == typeid(std::string)) {
            return "\"" + escape_json(std::any_cast<std::string>(value)) + "\"";
        } else if (value.type() == typeid(const char*)) {
            return "\"" + escape_json(std::any_cast<const char*>(value)) + "\"";
        } else {
            return "\"unknown\"";
        }
    }
    
    // Escape string for JSON
    std::string escape_json(const std::string& str) const {
        std::string escaped;
        for (char c : str) {
            if (c == '"') escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else if (c == '\n') escaped += "\\n";
            else if (c == '\r') escaped += "\\r";
            else if (c == '\t') escaped += "\\t";
            else if (c == '\b') escaped += "\\b";
            else if (c == '\f') escaped += "\\f";
            else escaped += c;
        }
        return escaped;
    }
    
    // Format value for logfmt
    std::string format_value_logfmt(const std::any& value) const {
        if (value.type() == typeid(std::string)) {
            return "\"" + escape_string(std::any_cast<std::string>(value)) + "\"";
        } else if (value.type() == typeid(const char*)) {
            return "\"" + escape_string(std::any_cast<const char*>(value)) + "\"";
        } else {
            // For numeric values, use plain format
            return format_value_plain(value);
        }
    }
    
    // Format value for plain text
    std::string format_value_plain(const std::any& value) const {
        if (value.type() == typeid(int)) {
            return std::to_string(std::any_cast<int>(value));
        } else if (value.type() == typeid(long)) {
            return std::to_string(std::any_cast<long>(value));
        } else if (value.type() == typeid(uint64_t)) {
            return std::to_string(std::any_cast<uint64_t>(value));
        } else if (value.type() == typeid(double)) {
            return std::to_string(std::any_cast<double>(value));
        } else if (value.type() == typeid(float)) {
            return std::to_string(std::any_cast<float>(value));
        } else if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? "true" : "false";
        } else if (value.type() == typeid(std::string)) {
            return std::any_cast<std::string>(value);
        } else if (value.type() == typeid(const char*)) {
            return std::any_cast<const char*>(value);
        } else {
            return "unknown";
        }
    }
    
    // Escape string for logfmt
    std::string escape_string(const std::string& str) const {
        std::string escaped;
        for (char c : str) {
            if (c == '"') escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else if (c == '\n') escaped += "\\n";
            else if (c == '\r') escaped += "\\r";
            else if (c == '\t') escaped += "\\t";
            else escaped += c;
        }
        return escaped;
    }
    
    // Convert log level to string
    std::string level_to_string(log_level level) const {
        switch (level) {
            case log_level::critical: return "CRITICAL";
            case log_level::error:    return "ERROR";
            case log_level::warning:  return "WARNING";
            case log_level::info:     return "INFO";
            case log_level::debug:    return "DEBUG";
            case log_level::trace:    return "TRACE";
            default: return "UNKNOWN";
        }
    }
    
    friend class log_entry;
};

// Implementation of log_entry::commit()
inline void log_entry::commit() {
    std::string formatted = logger_->format_entry(*this);
    logger_->logger_->log(level_, formatted);
}

} // namespace logger_module