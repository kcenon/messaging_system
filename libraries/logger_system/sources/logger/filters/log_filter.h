#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../logger_interface.h"
#include <string>
#include <regex>
#include <memory>
#include <functional>

namespace logger_module {

/**
 * @class log_filter
 * @brief Base class for log filtering
 */
class log_filter {
public:
    virtual ~log_filter() = default;
    
    /**
     * @brief Check if a log entry should be processed
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @return true if the log should be processed
     */
    virtual bool should_log(thread_module::log_level level,
                           const std::string& message,
                           const std::string& file,
                           int line,
                           const std::string& function) const = 0;
};

/**
 * @class level_filter
 * @brief Filter logs by minimum level
 */
class level_filter : public log_filter {
public:
    explicit level_filter(thread_module::log_level min_level)
        : min_level_(min_level) {}
    
    bool should_log(thread_module::log_level level,
                   const std::string& message,
                   const std::string& file,
                   int line,
                   const std::string& function) const override {
        (void)message;
        (void)file;
        (void)line;
        (void)function;
        return level <= min_level_;
    }
    
    void set_min_level(thread_module::log_level level) {
        min_level_ = level;
    }
    
private:
    thread_module::log_level min_level_;
};

/**
 * @class regex_filter
 * @brief Filter logs by regex pattern
 */
class regex_filter : public log_filter {
public:
    explicit regex_filter(const std::string& pattern, bool include = true)
        : pattern_(pattern), include_(include) {}
    
    bool should_log(thread_module::log_level level,
                   const std::string& message,
                   const std::string& file,
                   int line,
                   const std::string& function) const override {
        (void)level;
        (void)file;
        (void)line;
        (void)function;
        bool matches = std::regex_search(message, pattern_);
        return include_ ? matches : !matches;
    }
    
private:
    std::regex pattern_;
    bool include_;  // true = include matching, false = exclude matching
};

/**
 * @class function_filter
 * @brief Filter logs by custom function
 */
class function_filter : public log_filter {
public:
    using filter_function = std::function<bool(thread_module::log_level,
                                               const std::string&,
                                               const std::string&,
                                               int,
                                               const std::string&)>;
    
    explicit function_filter(filter_function func)
        : filter_func_(std::move(func)) {}
    
    bool should_log(thread_module::log_level level,
                   const std::string& message,
                   const std::string& file,
                   int line,
                   const std::string& function) const override {
        return filter_func_(level, message, file, line, function);
    }
    
private:
    filter_function filter_func_;
};

/**
 * @class composite_filter
 * @brief Combine multiple filters with AND/OR logic
 */
class composite_filter : public log_filter {
public:
    enum class logic_type {
        AND,  // All filters must pass
        OR    // At least one filter must pass
    };
    
    explicit composite_filter(logic_type logic = logic_type::AND)
        : logic_(logic) {}
    
    void add_filter(std::unique_ptr<log_filter> filter) {
        filters_.push_back(std::move(filter));
    }
    
    bool should_log(thread_module::log_level level,
                   const std::string& message,
                   const std::string& file,
                   int line,
                   const std::string& function) const override {
        if (filters_.empty()) {
            return true;  // No filters = pass all
        }
        
        if (logic_ == logic_type::AND) {
            for (const auto& filter : filters_) {
                if (!filter->should_log(level, message, file, line, function)) {
                    return false;
                }
            }
            return true;
        } else {  // OR logic
            for (const auto& filter : filters_) {
                if (filter->should_log(level, message, file, line, function)) {
                    return true;
                }
            }
            return false;
        }
    }
    
private:
    logic_type logic_;
    std::vector<std::unique_ptr<log_filter>> filters_;
};

} // namespace logger_module