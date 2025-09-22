#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../logger_interface.h"
#include <unordered_map>
#include <deque>
#include <mutex>
#include <chrono>
#include <functional>
#include <atomic>
#include <regex>
#include <vector>

namespace logger_module {

/**
 * @class log_analyzer
 * @brief Real-time log analysis and statistics
 */
class log_analyzer {
public:
    /**
     * @struct time_window_stats
     * @brief Statistics for a time window
     */
    struct time_window_stats {
        std::chrono::system_clock::time_point window_start;
        std::chrono::system_clock::time_point window_end;
        
        // Message counts by level
        std::unordered_map<thread_module::log_level, uint64_t> level_counts;
        
        // Top sources (file:line)
        std::unordered_map<std::string, uint64_t> source_counts;
        
        // Pattern matches
        std::unordered_map<std::string, uint64_t> pattern_matches;
        
        // Rate metrics
        double messages_per_second = 0.0;
        double bytes_per_second = 0.0;
        
        // Totals
        uint64_t total_messages = 0;
        uint64_t total_bytes = 0;
    };
    
    /**
     * @struct alert_rule
     * @brief Rule for generating alerts
     */
    struct alert_rule {
        std::string name;
        std::function<bool(const time_window_stats&)> condition;
        std::function<void(const std::string&, const time_window_stats&)> action;
    };
    
    /**
     * @brief Constructor
     * @param window_size Size of analysis window
     * @param max_windows Maximum number of windows to keep
     */
    log_analyzer(std::chrono::seconds window_size = std::chrono::seconds(60),
                 size_t max_windows = 60);
    
    /**
     * @brief Destructor
     */
    ~log_analyzer() = default;
    
    /**
     * @brief Analyze a log entry
     */
    void analyze(thread_module::log_level level,
                const std::string& message,
                const std::string& file,
                int line,
                const std::string& function,
                const std::chrono::system_clock::time_point& timestamp);
    
    /**
     * @brief Add pattern to track
     * @param name Pattern name
     * @param pattern Regex pattern to match
     */
    void add_pattern(const std::string& name, const std::string& pattern);
    
    /**
     * @brief Add alert rule
     * @param rule Alert rule to add
     */
    void add_alert_rule(const alert_rule& rule);
    
    /**
     * @brief Get current window statistics
     */
    time_window_stats get_current_stats() const;
    
    /**
     * @brief Get historical statistics
     * @param count Number of windows to retrieve
     */
    std::vector<time_window_stats> get_historical_stats(size_t count) const;
    
    /**
     * @brief Get aggregate statistics over time range
     * @param duration Time range to aggregate
     */
    time_window_stats get_aggregate_stats(std::chrono::seconds duration) const;
    
    /**
     * @brief Generate report
     * @param duration Time range for report
     * @return Formatted report string
     */
    std::string generate_report(std::chrono::seconds duration) const;
    
private:
    // Update current window or create new one
    void update_window(const std::chrono::system_clock::time_point& timestamp);
    
    // Check alert rules
    void check_alerts();
    
    // Pattern matching
    bool match_patterns(const std::string& message, 
                       std::unordered_map<std::string, uint64_t>& matches);
    
private:
    std::chrono::seconds window_size_;
    size_t max_windows_;
    
    // Time windows
    mutable std::mutex windows_mutex_;
    std::deque<time_window_stats> windows_;
    time_window_stats current_window_;
    
    // Pattern tracking
    std::unordered_map<std::string, std::regex> patterns_;
    mutable std::mutex patterns_mutex_;
    
    // Alert rules
    std::vector<alert_rule> alert_rules_;
    mutable std::mutex alerts_mutex_;
    
    // Rate calculation
    std::chrono::system_clock::time_point last_update_;
};

/**
 * @class log_aggregator
 * @brief Aggregates logs from multiple sources
 */
class log_aggregator {
public:
    /**
     * @struct source_stats
     * @brief Statistics per log source
     */
    struct source_stats {
        std::string source_id;
        uint64_t total_messages = 0;
        uint64_t total_bytes = 0;
        std::unordered_map<thread_module::log_level, uint64_t> level_counts;
        std::chrono::system_clock::time_point first_seen;
        std::chrono::system_clock::time_point last_seen;
        double average_message_rate = 0.0;
    };
    
    /**
     * @brief Constructor
     */
    log_aggregator() = default;
    
    /**
     * @brief Add log from a source
     */
    void add_log(const std::string& source_id,
                thread_module::log_level level,
                const std::string& message,
                size_t message_size);
    
    /**
     * @brief Get statistics for all sources
     */
    std::unordered_map<std::string, source_stats> get_all_stats() const;
    
    /**
     * @brief Get statistics for specific source
     */
    source_stats get_source_stats(const std::string& source_id) const;
    
    /**
     * @brief Reset statistics for a source
     */
    void reset_source(const std::string& source_id);
    
    /**
     * @brief Reset all statistics
     */
    void reset_all();
    
private:
    mutable std::mutex stats_mutex_;
    std::unordered_map<std::string, source_stats> source_stats_;
};

} // namespace logger_module