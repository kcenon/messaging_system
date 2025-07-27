/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "log_analyzer.h"
#include <regex>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace logger_module {

log_analyzer::log_analyzer(std::chrono::seconds window_size, size_t max_windows)
    : window_size_(window_size)
    , max_windows_(max_windows) {
    
    auto now = std::chrono::system_clock::now();
    current_window_.window_start = now;
    current_window_.window_end = now + window_size_;
    last_update_ = now;
}

void log_analyzer::analyze(thread_module::log_level level,
                          const std::string& message,
                          const std::string& file,
                          int line,
                          const std::string& /* function */,
                          const std::chrono::system_clock::time_point& timestamp) {
    
    std::lock_guard<std::mutex> lock(windows_mutex_);
    
    // Check if we need a new window
    update_window(timestamp);
    
    // Update level counts
    current_window_.level_counts[level]++;
    
    // Update source counts
    if (!file.empty()) {
        std::string source = file + ":" + std::to_string(line);
        current_window_.source_counts[source]++;
    }
    
    // Pattern matching
    match_patterns(message, current_window_.pattern_matches);
    
    // Update totals
    current_window_.total_messages++;
    current_window_.total_bytes += message.size();
    
    // Update rates
    auto duration = timestamp - current_window_.window_start;
    double seconds = std::chrono::duration<double>(duration).count();
    if (seconds > 0) {
        current_window_.messages_per_second = current_window_.total_messages / seconds;
        current_window_.bytes_per_second = current_window_.total_bytes / seconds;
    }
    
    // Check alerts
    check_alerts();
}

void log_analyzer::add_pattern(const std::string& name, const std::string& pattern) {
    std::lock_guard<std::mutex> lock(patterns_mutex_);
    patterns_[name] = std::regex(pattern, std::regex::icase);
}

void log_analyzer::add_alert_rule(const alert_rule& rule) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    alert_rules_.push_back(rule);
}

log_analyzer::time_window_stats log_analyzer::get_current_stats() const {
    std::lock_guard<std::mutex> lock(windows_mutex_);
    return current_window_;
}

std::vector<log_analyzer::time_window_stats> 
log_analyzer::get_historical_stats(size_t count) const {
    std::lock_guard<std::mutex> lock(windows_mutex_);
    
    std::vector<time_window_stats> result;
    size_t n = std::min(count, windows_.size());
    
    // Get most recent windows
    for (auto it = windows_.rbegin(); it != windows_.rend() && result.size() < n; ++it) {
        result.push_back(*it);
    }
    
    // Add current window if requested
    if (result.size() < count) {
        result.push_back(current_window_);
    }
    
    return result;
}

log_analyzer::time_window_stats 
log_analyzer::get_aggregate_stats(std::chrono::seconds duration) const {
    std::lock_guard<std::mutex> lock(windows_mutex_);
    
    time_window_stats aggregate;
    auto cutoff_time = std::chrono::system_clock::now() - duration;
    
    // Aggregate historical windows
    for (const auto& window : windows_) {
        if (window.window_end > cutoff_time) {
            // Aggregate level counts
            for (const auto& [level, count] : window.level_counts) {
                aggregate.level_counts[level] += count;
            }
            
            // Aggregate source counts
            for (const auto& [source, count] : window.source_counts) {
                aggregate.source_counts[source] += count;
            }
            
            // Aggregate pattern matches
            for (const auto& [pattern, count] : window.pattern_matches) {
                aggregate.pattern_matches[pattern] += count;
            }
            
            aggregate.total_messages += window.total_messages;
            aggregate.total_bytes += window.total_bytes;
            
            if (aggregate.window_start.time_since_epoch().count() == 0 ||
                window.window_start < aggregate.window_start) {
                aggregate.window_start = window.window_start;
            }
        }
    }
    
    // Include current window
    if (current_window_.window_start > cutoff_time) {
        for (const auto& [level, count] : current_window_.level_counts) {
            aggregate.level_counts[level] += count;
        }
        
        for (const auto& [source, count] : current_window_.source_counts) {
            aggregate.source_counts[source] += count;
        }
        
        for (const auto& [pattern, count] : current_window_.pattern_matches) {
            aggregate.pattern_matches[pattern] += count;
        }
        
        aggregate.total_messages += current_window_.total_messages;
        aggregate.total_bytes += current_window_.total_bytes;
    }
    
    aggregate.window_end = std::chrono::system_clock::now();
    
    // Calculate rates
    auto duration_actual = aggregate.window_end - aggregate.window_start;
    double seconds = std::chrono::duration<double>(duration_actual).count();
    if (seconds > 0) {
        aggregate.messages_per_second = aggregate.total_messages / seconds;
        aggregate.bytes_per_second = aggregate.total_bytes / seconds;
    }
    
    return aggregate;
}

std::string log_analyzer::generate_report(std::chrono::seconds duration) const {
    auto stats = get_aggregate_stats(duration);
    std::ostringstream report;
    
    report << "=== Log Analysis Report ===" << std::endl;
    report << "Time Range: " << duration.count() << " seconds" << std::endl;
    report << "Total Messages: " << stats.total_messages << std::endl;
    report << "Total Bytes: " << stats.total_bytes << std::endl;
    report << "Messages/sec: " << std::fixed << std::setprecision(2) 
           << stats.messages_per_second << std::endl;
    report << "Bytes/sec: " << stats.bytes_per_second << std::endl;
    
    report << "\n--- Log Levels ---" << std::endl;
    for (const auto& [level, count] : stats.level_counts) {
        std::string level_str;
        switch (level) {
            case thread_module::log_level::trace: level_str = "TRACE"; break;
            case thread_module::log_level::debug: level_str = "DEBUG"; break;
            case thread_module::log_level::info: level_str = "INFO"; break;
            case thread_module::log_level::warning: level_str = "WARNING"; break;
            case thread_module::log_level::error: level_str = "ERROR"; break;
            case thread_module::log_level::critical: level_str = "CRITICAL"; break;
        }
        double percentage = (stats.total_messages > 0) ? 
                           (count * 100.0 / stats.total_messages) : 0;
        report << level_str << ": " << count << " (" 
               << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
    }
    
    report << "\n--- Top Sources ---" << std::endl;
    std::vector<std::pair<std::string, uint64_t>> sorted_sources(
        stats.source_counts.begin(), stats.source_counts.end()
    );
    std::sort(sorted_sources.begin(), sorted_sources.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    size_t top_n = std::min(size_t(10), sorted_sources.size());
    for (size_t i = 0; i < top_n; ++i) {
        report << sorted_sources[i].first << ": " << sorted_sources[i].second << std::endl;
    }
    
    if (!stats.pattern_matches.empty()) {
        report << "\n--- Pattern Matches ---" << std::endl;
        for (const auto& [pattern, count] : stats.pattern_matches) {
            report << pattern << ": " << count << std::endl;
        }
    }
    
    return report.str();
}

void log_analyzer::update_window(const std::chrono::system_clock::time_point& timestamp) {
    if (timestamp >= current_window_.window_end) {
        // Save current window
        windows_.push_back(current_window_);
        
        // Maintain max windows
        while (windows_.size() > max_windows_) {
            windows_.pop_front();
        }
        
        // Create new window
        current_window_ = time_window_stats{};
        current_window_.window_start = current_window_.window_end;
        current_window_.window_end = current_window_.window_start + window_size_;
    }
}

void log_analyzer::check_alerts() {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    
    for (const auto& rule : alert_rules_) {
        if (rule.condition(current_window_)) {
            rule.action(rule.name, current_window_);
        }
    }
}

bool log_analyzer::match_patterns(const std::string& message,
                                 std::unordered_map<std::string, uint64_t>& matches) {
    std::lock_guard<std::mutex> lock(patterns_mutex_);
    
    bool matched = false;
    for (const auto& [name, pattern] : patterns_) {
        if (std::regex_search(message, pattern)) {
            matches[name]++;
            matched = true;
        }
    }
    
    return matched;
}

// log_aggregator implementation

void log_aggregator::add_log(const std::string& source_id,
                            thread_module::log_level level,
                            const std::string& /* message */,
                            size_t message_size) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto& stats = source_stats_[source_id];
    stats.source_id = source_id;
    
    if (stats.total_messages == 0) {
        stats.first_seen = std::chrono::system_clock::now();
    }
    stats.last_seen = std::chrono::system_clock::now();
    
    stats.total_messages++;
    stats.total_bytes += message_size;
    stats.level_counts[level]++;
    
    // Update average rate
    auto duration = stats.last_seen - stats.first_seen;
    double seconds = std::chrono::duration<double>(duration).count();
    if (seconds > 0) {
        stats.average_message_rate = stats.total_messages / seconds;
    }
}

std::unordered_map<std::string, log_aggregator::source_stats> 
log_aggregator::get_all_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return source_stats_;
}

log_aggregator::source_stats 
log_aggregator::get_source_stats(const std::string& source_id) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = source_stats_.find(source_id);
    if (it != source_stats_.end()) {
        return it->second;
    }
    
    return source_stats{};
}

void log_aggregator::reset_source(const std::string& source_id) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    source_stats_.erase(source_id);
}

void log_aggregator::reset_all() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    source_stats_.clear();
}

} // namespace logger_module