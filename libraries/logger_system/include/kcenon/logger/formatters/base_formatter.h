#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../interfaces/log_formatter_interface.h"
#include "../interfaces/log_entry.h"
#include <sstream>
#include <iomanip>
#include <thread>

namespace kcenon::logger {

/**
 * @class base_formatter
 * @brief Base implementation for log formatters
 * 
 * Provides common formatting functionality for derived formatters.
 */
class base_formatter : public log_formatter_interface {
public:
    virtual ~base_formatter() = default;
    
    /**
     * @brief Format a log entry into a string
     * @param entry The log entry to format
     * @return Formatted string representation
     */
    virtual std::string format(const log_entry& entry) const override = 0;
    
protected:
    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return String representation
     */
    std::string level_to_string(kcenon::thread::log_level level) const {
        switch (level) {
            case kcenon::thread::log_level::critical: return "CRITICAL";
            case kcenon::thread::log_level::error:    return "ERROR";
            case kcenon::thread::log_level::warning:  return "WARNING";
            case kcenon::thread::log_level::info:     return "INFO";
            case kcenon::thread::log_level::debug:    return "DEBUG";
            case kcenon::thread::log_level::trace:    return "TRACE";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * @brief Format timestamp to ISO8601 string
     * @param timestamp Time point to format
     * @return ISO8601 formatted string
     */
    std::string format_timestamp(const std::chrono::system_clock::time_point& timestamp) const {
        auto time_t = std::chrono::system_clock::to_time_t(timestamp);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }
    
    /**
     * @brief Get current thread ID as string
     * @return Thread ID string
     */
    std::string get_thread_id() const {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        return oss.str();
    }
};

/**
 * @class plain_formatter
 * @brief Simple plain text formatter
 */
class plain_formatter : public base_formatter {
public:
    std::string format(const log_entry& entry) const override {
        std::ostringstream oss;
        
        // Format: [TIMESTAMP] [LEVEL] [THREAD] MESSAGE [FILE:LINE:FUNCTION]
        oss << "[" << format_timestamp(entry.timestamp) << "] ";
        oss << "[" << level_to_string(entry.level) << "] ";
        
        if (entry.thread_id) {
            oss << "[" << std::string_view(*entry.thread_id) << "] ";
        } else {
            oss << "[" << get_thread_id() << "] ";
        }
        
        oss << std::string_view(entry.message);
        
        if (entry.location) {
            oss << " [" << std::string_view(entry.location->file)
                << ":" << entry.location->line 
                << ":" << std::string_view(entry.location->function) << "]";
        }
        
        return oss.str();
    }
    
    std::string get_format_type() const override {
        return "plain";
    }
};

/**
 * @class json_formatter
 * @brief JSON formatter for structured logging
 */
class json_formatter : public base_formatter {
public:
    std::string format(const log_entry& entry) const override {
        std::ostringstream oss;
        
        oss << "{";
        oss << "\"timestamp\":\"" << format_timestamp(entry.timestamp) << "\",";
        oss << "\"level\":\"" << level_to_string(entry.level) << "\",";
        oss << "\"message\":\"" << escape_json(entry.message) << "\",";
        oss << "\"thread\":\"" << (entry.thread_id ? std::string(*entry.thread_id) : get_thread_id()) << "\"";
        
        if (entry.location) {
            oss << ",\"location\":{";
            oss << "\"file\":\"" << escape_json(entry.location->file) << "\",";
            oss << "\"line\":" << entry.location->line << ",";
            oss << "\"function\":\"" << escape_json(entry.location->function) << "\"";
            oss << "}";
        }
        
        if (entry.category) {
            oss << ",\"category\":\"" << escape_json(*entry.category) << "\"";
        }
        
        oss << "}";
        
        return oss.str();
    }
    
    std::string get_format_type() const override {
        return "json";
    }
    
private:
    template<typename StringType>
    std::string escape_json(const StringType& str) const {
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
};

/**
 * @class compact_formatter
 * @brief Compact formatter for minimal output
 */
class compact_formatter : public base_formatter {
public:
    std::string format(const log_entry& entry) const override {
        std::ostringstream oss;
        
        // Ultra-compact format: LEVEL|MESSAGE
        oss << level_to_string(entry.level)[0] << "|" << entry.message;
        
        return oss.str();
    }
    
    std::string get_format_type() const override {
        return "compact";
    }
};

} // namespace kcenon::logger