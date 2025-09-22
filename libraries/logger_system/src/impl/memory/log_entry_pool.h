/**
 * @file log_entry_pool.h
 * @brief Log entry pool implementation for high-performance memory management
 */

#pragma once

#include <kcenon/logger/interfaces/log_entry.h>
#include <memory>
#include <string>
#include <chrono>

namespace kcenon::logger::memory::log_entry_pool {

/**
 * @brief Pooled log entry structure optimized for reuse
 */
struct pooled_log_entry {
    logger_system::log_level level{logger_system::log_level::info};
    std::string message;
    std::string file_path;
    int line_number{0};
    std::string function_name;
    std::chrono::system_clock::time_point timestamp;

    /**
     * @brief Reset entry for reuse
     */
    void reset() {
        level = logger_system::log_level::info;
        message.clear();
        file_path.clear();
        line_number = 0;
        function_name.clear();
        timestamp = std::chrono::system_clock::now();
    }

    /**
     * @brief Initialize with log data
     */
    void initialize(logger_system::log_level lvl,
                   const std::string& msg,
                   const std::string& file,
                   int line,
                   const std::string& func) {
        level = lvl;
        message = msg;
        file_path = file;
        line_number = line;
        function_name = func;
        timestamp = std::chrono::system_clock::now();
    }

    /**
     * @brief Convert to standard log_entry
     */
    log_entry to_log_entry() const {
        log_entry entry(level, message, timestamp);
        if (!file_path.empty() || line_number != 0 || !function_name.empty()) {
            entry.location = source_location(file_path, line_number, function_name);
        }
        return entry;
    }
};

} // namespace kcenon::logger::memory::log_entry_pool