#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../filters/log_filter.h"
#include "../writers/base_writer.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace logger_module {

/**
 * @struct log_route
 * @brief Represents a routing rule
 */
struct log_route {
    std::unique_ptr<log_filter> filter;
    std::vector<std::string> writer_names;
    bool stop_propagation;  // If true, stop processing other routes
    
    log_route(std::unique_ptr<log_filter> f, 
              std::vector<std::string> writers, 
              bool stop = false)
        : filter(std::move(f))
        , writer_names(std::move(writers))
        , stop_propagation(stop) {}
    
    // Move constructor
    log_route(log_route&& other) noexcept
        : filter(std::move(other.filter))
        , writer_names(std::move(other.writer_names))
        , stop_propagation(other.stop_propagation) {}
    
    // Move assignment
    log_route& operator=(log_route&& other) noexcept {
        if (this != &other) {
            filter = std::move(other.filter);
            writer_names = std::move(other.writer_names);
            stop_propagation = other.stop_propagation;
        }
        return *this;
    }
};

/**
 * @class log_router
 * @brief Routes log messages to specific writers based on filters
 */
class log_router {
public:
    log_router() = default;
    
    /**
     * @brief Add a routing rule
     * @param filter Filter to apply
     * @param writer_names Names of writers to route to
     * @param stop_propagation Stop processing other routes if this matches
     */
    void add_route(std::unique_ptr<log_filter> filter,
                   const std::vector<std::string>& writer_names,
                   bool stop_propagation = false) {
        routes_.emplace_back(std::move(filter), writer_names, stop_propagation);
    }
    
    /**
     * @brief Route a log message to appropriate writers
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @param timestamp Timestamp
     * @param writers Map of available writers
     * @return Vector of writers that should process this log
     */
    std::vector<base_writer*> route(
        thread_module::log_level level,
        const std::string& message,
        const std::string& file,
        int line,
        const std::string& function,
        const std::chrono::system_clock::time_point& timestamp,
        const std::unordered_map<std::string, std::unique_ptr<base_writer>>& writers) const {
        (void)timestamp;  // Not used in current implementation
        
        std::vector<base_writer*> selected_writers;
        std::unordered_set<std::string> added_writers;
        
        // Check each route
        for (const auto& route : routes_) {
            if (route.filter->should_log(level, message, file, line, function)) {
                // Add writers from this route
                for (const auto& writer_name : route.writer_names) {
                    if (added_writers.find(writer_name) == added_writers.end()) {
                        auto it = writers.find(writer_name);
                        if (it != writers.end()) {
                            selected_writers.push_back(it->second.get());
                            added_writers.insert(writer_name);
                        }
                    }
                }
                
                // Stop if this route has stop_propagation
                if (route.stop_propagation) {
                    break;
                }
            }
        }
        
        // If no routes matched, use default (all writers)
        if (selected_writers.empty() && !has_exclusive_routes_) {
            for (const auto& [name, writer] : writers) {
                selected_writers.push_back(writer.get());
            }
        }
        
        return selected_writers;
    }
    
    /**
     * @brief Set whether routes are exclusive (no default fallback)
     */
    void set_exclusive_routes(bool exclusive) {
        has_exclusive_routes_ = exclusive;
    }
    
    /**
     * @brief Clear all routes
     */
    void clear_routes() {
        routes_.clear();
    }
    
private:
    std::vector<log_route> routes_;
    bool has_exclusive_routes_ = false;
};

/**
 * @class router_builder
 * @brief Fluent builder for creating routing rules
 */
class router_builder {
public:
    explicit router_builder(log_router& router) : router_(router) {}
    
    // Start building a route with a level filter
    router_builder& when_level(thread_module::log_level min_level) {
        current_filter_ = std::make_unique<level_filter>(min_level);
        return *this;
    }
    
    // Start building a route with a regex filter
    router_builder& when_matches(const std::string& pattern) {
        current_filter_ = std::make_unique<regex_filter>(pattern, true);
        return *this;
    }
    
    // Start building a route with an exclusion regex filter
    router_builder& when_not_matches(const std::string& pattern) {
        current_filter_ = std::make_unique<regex_filter>(pattern, false);
        return *this;
    }
    
    // Add custom filter
    router_builder& when(std::unique_ptr<log_filter> filter) {
        current_filter_ = std::move(filter);
        return *this;
    }
    
    // Route to specific writers
    router_builder& route_to(const std::vector<std::string>& writer_names,
                            bool stop_propagation = false) {
        if (current_filter_) {
            router_.add_route(std::move(current_filter_), writer_names, stop_propagation);
        }
        return *this;
    }
    
    // Route to a single writer
    router_builder& route_to(const std::string& writer_name,
                            bool stop_propagation = false) {
        return route_to(std::vector<std::string>{writer_name}, stop_propagation);
    }
    
private:
    log_router& router_;
    std::unique_ptr<log_filter> current_filter_;
};

} // namespace logger_module