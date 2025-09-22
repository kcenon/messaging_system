/**
 * @file log_analyzer.h
 * @brief Log analysis and metrics functionality
 */

#pragma once

#include <kcenon/logger/interfaces/logger_types.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>

namespace kcenon::logger::analysis {

/**
 * @brief Log entry for analysis
 */
struct analyzed_log_entry {
    logger_system::log_level level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::string source_file;
    int source_line;
    std::string function_name;
};

/**
 * @brief Analysis result statistics
 */
struct analysis_stats {
    size_t total_entries = 0;
    std::unordered_map<logger_system::log_level, size_t> level_counts;
    std::chrono::system_clock::time_point earliest_timestamp;
    std::chrono::system_clock::time_point latest_timestamp;
    std::vector<std::string> most_frequent_messages;
    std::unordered_map<std::string, size_t> error_patterns;
};

/**
 * @brief Log analyzer for processing and analyzing log data
 */
class log_analyzer {
private:
    std::vector<analyzed_log_entry> entries_;
    analysis_stats cached_stats_;
    bool stats_dirty_ = true;

public:
    /**
     * @brief Add a log entry for analysis
     */
    void add_entry(const analyzed_log_entry& entry) {
        entries_.push_back(entry);
        stats_dirty_ = true;
    }

    /**
     * @brief Add multiple log entries
     */
    void add_entries(const std::vector<analyzed_log_entry>& entries) {
        entries_.insert(entries_.end(), entries.begin(), entries.end());
        stats_dirty_ = true;
    }

    /**
     * @brief Clear all entries
     */
    void clear() {
        entries_.clear();
        cached_stats_ = analysis_stats{};
        stats_dirty_ = true;
    }

    /**
     * @brief Get analysis statistics
     */
    const analysis_stats& get_stats() {
        if (stats_dirty_) {
            update_stats();
            stats_dirty_ = false;
        }
        return cached_stats_;
    }

    /**
     * @brief Filter entries by log level
     */
    std::vector<analyzed_log_entry> filter_by_level(logger_system::log_level level) const {
        std::vector<analyzed_log_entry> filtered;
        for (const auto& entry : entries_) {
            if (entry.level == level) {
                filtered.push_back(entry);
            }
        }
        return filtered;
    }

    /**
     * @brief Filter entries by time range
     */
    std::vector<analyzed_log_entry> filter_by_time_range(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const {

        std::vector<analyzed_log_entry> filtered;
        for (const auto& entry : entries_) {
            if (entry.timestamp >= start && entry.timestamp <= end) {
                filtered.push_back(entry);
            }
        }
        return filtered;
    }

    /**
     * @brief Find entries containing specific text
     */
    std::vector<analyzed_log_entry> search_messages(const std::string& search_text) const {
        std::vector<analyzed_log_entry> results;
        for (const auto& entry : entries_) {
            if (entry.message.find(search_text) != std::string::npos) {
                results.push_back(entry);
            }
        }
        return results;
    }

    /**
     * @brief Get error rate for a time window
     */
    double get_error_rate(const std::chrono::minutes& window = std::chrono::minutes(60)) const {
        auto now = std::chrono::system_clock::now();
        auto start_time = now - window;

        size_t total_in_window = 0;
        size_t errors_in_window = 0;

        for (const auto& entry : entries_) {
            if (entry.timestamp >= start_time) {
                total_in_window++;
                if (entry.level == logger_system::log_level::error ||
                    entry.level == logger_system::log_level::fatal) {
                    errors_in_window++;
                }
            }
        }

        return total_in_window > 0 ?
            static_cast<double>(errors_in_window) / total_in_window : 0.0;
    }

    /**
     * @brief Generate summary report
     */
    std::string generate_summary_report() {
        const auto& stats = get_stats();

        std::string report = "=== Log Analysis Summary ===\n";
        report += "Total Entries: " + std::to_string(stats.total_entries) + "\n";
        report += "Level Distribution:\n";

        for (const auto& [level, count] : stats.level_counts) {
            report += "  " + level_to_string(level) + ": " + std::to_string(count) + "\n";
        }

        if (stats.total_entries > 0) {
            auto duration = std::chrono::duration_cast<std::chrono::minutes>(
                stats.latest_timestamp - stats.earliest_timestamp);
            report += "Time Range: " + std::to_string(duration.count()) + " minutes\n";
        }

        return report;
    }

private:
    void update_stats() {
        cached_stats_ = analysis_stats{};
        cached_stats_.total_entries = entries_.size();

        if (entries_.empty()) {
            return;
        }

        // Initialize timestamps
        cached_stats_.earliest_timestamp = entries_[0].timestamp;
        cached_stats_.latest_timestamp = entries_[0].timestamp;

        // Count levels and update timestamps
        for (const auto& entry : entries_) {
            cached_stats_.level_counts[entry.level]++;

            if (entry.timestamp < cached_stats_.earliest_timestamp) {
                cached_stats_.earliest_timestamp = entry.timestamp;
            }
            if (entry.timestamp > cached_stats_.latest_timestamp) {
                cached_stats_.latest_timestamp = entry.timestamp;
            }
        }
    }

    std::string level_to_string(logger_system::log_level level) const {
        switch (level) {
            case logger_system::log_level::trace: return "TRACE";
            case logger_system::log_level::debug: return "DEBUG";
            case logger_system::log_level::info: return "INFO";
            case logger_system::log_level::warn: return "WARN";
            case logger_system::log_level::error: return "ERROR";
            case logger_system::log_level::fatal: return "FATAL";
            case logger_system::log_level::off: return "OFF";
            default: return "UNKNOWN";
        }
    }
};

/**
 * @brief Factory for creating log analyzers
 */
class analyzer_factory {
public:
    /**
     * @brief Create a basic log analyzer
     */
    static std::unique_ptr<log_analyzer> create_basic() {
        return std::make_unique<log_analyzer>();
    }
};

} // namespace kcenon::logger::analysis