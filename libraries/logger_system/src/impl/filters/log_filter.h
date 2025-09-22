#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

// Conditional include based on build mode
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    #include <kcenon/thread/interfaces/logger_interface.h>
#else
    #include <kcenon/logger/interfaces/logger_interface.h>
#endif

#include <kcenon/logger/interfaces/log_filter_interface.h>
#include <kcenon/logger/interfaces/log_entry.h>
#include <string>
#include <regex>
#include <memory>
#include <functional>

namespace kcenon::logger {

/**
 * @class log_filter
 * @brief Base class for log filtering
 * 
 * This class provides a compatibility layer between the old API and the new
 * interface-based approach. It implements log_filter_interface.
 */
class log_filter : public log_filter_interface {
public:
    virtual ~log_filter() = default;
    
    /**
     * @brief Check if a log entry should be processed (new interface)
     * @param entry The log entry to check
     * @return true if the log should be processed
     */
    virtual bool should_log(const log_entry& entry) const override {
        std::string file = entry.location ? entry.location->file.to_string() : "";
        int line = entry.location ? entry.location->line : 0;
        std::string function = entry.location ? entry.location->function.to_string() : "";
        
        return should_log(entry.level, entry.message.to_string(), file, line, function);
    }
    
    /**
     * @brief Check if a log entry should be processed (legacy API)
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @return true if the log should be processed
     */
    virtual bool should_log(kcenon::thread::log_level level,
                           const std::string& message,
                           const std::string& file,
                           int line,
                           const std::string& function) const = 0;
    
    /**
     * @brief Get the name of this filter
     * @return Filter name for identification
     */
    virtual std::string get_name() const override {
        return "base_filter";
    }
};

/**
 * @class level_filter
 * @brief Filter logs by minimum level
 */
class level_filter : public log_filter {
public:
    explicit level_filter(kcenon::thread::log_level min_level)
        : min_level_(min_level) {}
    
    bool should_log(kcenon::thread::log_level level,
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
    
    void set_min_level(kcenon::thread::log_level level) {
        min_level_ = level;
    }
    
    std::string get_name() const override {
        return "level_filter";
    }
    
private:
    kcenon::thread::log_level min_level_;
};

/**
 * @class regex_filter
 * @brief Filter logs by regex pattern
 */
class regex_filter : public log_filter {
public:
    explicit regex_filter(const std::string& pattern, bool include = true)
        : pattern_(pattern), include_(include) {}
    
    bool should_log(kcenon::thread::log_level level,
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
    
    std::string get_name() const override {
        return "regex_filter";
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
    using filter_function = std::function<bool(kcenon::thread::log_level,
                                               const std::string&,
                                               const std::string&,
                                               int,
                                               const std::string&)>;
    
    explicit function_filter(filter_function func)
        : filter_func_(std::move(func)) {}
    
    bool should_log(kcenon::thread::log_level level,
                   const std::string& message,
                   const std::string& file,
                   int line,
                   const std::string& function) const override {
        return filter_func_(level, message, file, line, function);
    }
    
    std::string get_name() const override {
        return "function_filter";
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
    
    bool should_log(kcenon::thread::log_level level,
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
    
    std::string get_name() const override {
        return "composite_filter";
    }
    
private:
    logic_type logic_;
    std::vector<std::unique_ptr<log_filter>> filters_;
};

} // namespace kcenon::logger