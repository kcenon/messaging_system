#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>
#include <optional>
#include <algorithm>
#include <deque>
#include <set>
#include <map>
#include <cmath>

#include <kcenon/monitoring/alerting/rule_engine.h>

namespace kcenon::monitoring::alerting {

// Alert grouping strategy
enum class GroupingStrategy {
    BY_RULE,           // Group by rule ID
    BY_SEVERITY,       // Group by severity level
    BY_LABELS,         // Group by label combinations
    BY_TIME_WINDOW,    // Group by time windows
    BY_CUSTOM          // Custom grouping function
};

// Alert deduplication strategy
enum class DeduplicationStrategy {
    EXACT_MATCH,       // Exact alert content match
    FUZZY_MATCH,       // Similarity-based matching
    TIME_BASED,        // Time window based deduplication
    FINGERPRINT        // Content fingerprint matching
};

// Silence (mute) configuration
struct SilenceConfig {
    std::string id;
    std::string name;
    std::string comment;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::vector<std::pair<std::string, std::string>> matchers; // label key-value pairs
    std::regex pattern;
    bool is_regex = false;
    bool enabled = true;
    std::string created_by;
};

// Alert group
struct AlertGroup {
    std::string id;
    std::string key;  // Grouping key
    std::vector<Alert> alerts;
    std::chrono::system_clock::time_point first_alert_time;
    std::chrono::system_clock::time_point last_alert_time;
    size_t alert_count = 0;
    std::unordered_map<std::string, std::string> common_labels;
    AlertSeverity max_severity = AlertSeverity::INFO;
};

// Deduplication entry
struct DeduplicationEntry {
    std::string fingerprint;
    Alert original_alert;
    std::chrono::system_clock::time_point first_seen;
    std::chrono::system_clock::time_point last_seen;
    size_t occurrence_count = 1;
    std::vector<std::chrono::system_clock::time_point> occurrence_times;
};

// Alert similarity metrics
struct SimilarityMetrics {
    double label_similarity = 0.0;
    double message_similarity = 0.0;
    double time_proximity = 0.0;
    double overall_similarity = 0.0;
};

// Alert deduplication manager
class AlertDeduplication {
public:
    AlertDeduplication();
    ~AlertDeduplication();

    // Deduplication processing
    bool is_duplicate(const Alert& alert);
    std::optional<DeduplicationEntry> find_duplicate(const Alert& alert);
    void record_alert(const Alert& alert);
    void clear_old_entries(std::chrono::seconds age_limit);

    // Deduplication configuration
    void set_deduplication_strategy(DeduplicationStrategy strategy);
    void set_deduplication_window(std::chrono::seconds window);
    void set_similarity_threshold(double threshold);
    void set_max_entries(size_t max_entries);

    // Fingerprint generation
    std::string generate_fingerprint(const Alert& alert) const;
    std::string generate_fuzzy_fingerprint(const Alert& alert) const;

    // Similarity calculation
    double calculate_similarity(const Alert& alert1, const Alert& alert2) const;
    SimilarityMetrics calculate_detailed_similarity(const Alert& alert1,
                                                   const Alert& alert2) const;
    double calculate_label_similarity(const std::unordered_map<std::string, std::string>& labels1,
                                     const std::unordered_map<std::string, std::string>& labels2) const;
    double calculate_string_similarity(const std::string& s1, const std::string& s2) const;

    // Statistics
    size_t get_deduplication_count() const { return deduplication_count_.load(); }
    size_t get_entry_count() const;
    double get_deduplication_rate() const;
    std::vector<DeduplicationEntry> get_top_duplicates(size_t count) const;

private:
    mutable std::mutex entries_mutex_;
    std::unordered_map<std::string, DeduplicationEntry> dedup_entries_;
    std::deque<std::pair<std::chrono::system_clock::time_point, std::string>> entry_timeline_;

    DeduplicationStrategy strategy_ = DeduplicationStrategy::EXACT_MATCH;
    std::chrono::seconds dedup_window_ = std::chrono::seconds(300);
    double similarity_threshold_ = 0.85;
    size_t max_entries_ = 10000;

    std::atomic<size_t> total_alerts_{0};
    std::atomic<size_t> deduplication_count_{0};

    // Helper methods
    void cleanup_old_entries();
    bool is_similar_enough(const Alert& alert1, const Alert& alert2) const;
    size_t levenshtein_distance(const std::string& s1, const std::string& s2) const;
};

// Alert grouping manager
class AlertGrouping {
public:
    AlertGrouping();
    ~AlertGrouping();

    // Grouping operations
    std::string assign_to_group(const Alert& alert);
    void add_alert_to_group(const Alert& alert, const std::string& group_id);
    std::optional<AlertGroup> get_group(const std::string& group_id) const;
    std::vector<AlertGroup> get_all_groups() const;
    std::vector<AlertGroup> get_active_groups() const;

    // Grouping configuration
    void set_grouping_strategy(GroupingStrategy strategy);
    void set_grouping_keys(const std::vector<std::string>& keys);
    void set_grouping_window(std::chrono::seconds window);
    void set_custom_grouping_function(std::function<std::string(const Alert&)> func);

    // Group management
    void merge_groups(const std::string& group_id1, const std::string& group_id2);
    void split_group(const std::string& group_id,
                    std::function<bool(const Alert&)> predicate);
    void close_group(const std::string& group_id);
    void reopen_group(const std::string& group_id);
    void clear_old_groups(std::chrono::seconds age_limit);

    // Group key generation
    std::string generate_group_key(const Alert& alert) const;
    std::string generate_label_based_key(const Alert& alert,
                                        const std::vector<std::string>& label_keys) const;

    // Statistics
    size_t get_group_count() const;
    size_t get_total_alert_count() const { return total_alerts_.load(); }
    std::unordered_map<std::string, size_t> get_group_sizes() const;
    double get_average_group_size() const;

private:
    mutable std::mutex groups_mutex_;
    std::unordered_map<std::string, AlertGroup> groups_;
    std::unordered_map<std::string, std::string> alert_to_group_;

    GroupingStrategy strategy_ = GroupingStrategy::BY_RULE;
    std::vector<std::string> grouping_keys_;
    std::chrono::seconds grouping_window_ = std::chrono::seconds(3600);
    std::function<std::string(const Alert&)> custom_grouping_func_;

    std::atomic<size_t> total_alerts_{0};

    // Helper methods
    std::string generate_time_window_key(const Alert& alert) const;
    void update_group_metadata(AlertGroup& group, const Alert& alert);
    std::unordered_map<std::string, std::string> extract_common_labels(
        const std::vector<Alert>& alerts) const;
};

// Silence (mute) manager
class SilenceManager {
public:
    SilenceManager();
    ~SilenceManager();

    // Silence management
    void add_silence(const SilenceConfig& silence);
    void update_silence(const std::string& id, const SilenceConfig& silence);
    void remove_silence(const std::string& id);
    std::optional<SilenceConfig> get_silence(const std::string& id) const;
    std::vector<SilenceConfig> get_all_silences() const;
    std::vector<SilenceConfig> get_active_silences() const;

    // Check if alert is silenced
    bool is_silenced(const Alert& alert) const;
    std::vector<std::string> get_matching_silences(const Alert& alert) const;

    // Silence expiry management
    void expire_old_silences();
    std::vector<SilenceConfig> get_expiring_silences(std::chrono::seconds within) const;

    // Bulk operations
    void load_silences_from_file(const std::string& filepath);
    void save_silences_to_file(const std::string& filepath) const;

    // Statistics
    size_t get_silence_count() const;
    size_t get_active_silence_count() const;
    size_t get_silenced_alert_count() const { return silenced_alerts_.load(); }

private:
    mutable std::mutex silences_mutex_;
    std::unordered_map<std::string, SilenceConfig> silences_;
    std::atomic<size_t> silenced_alerts_{0};

    // Helper methods
    bool matches_silence(const Alert& alert, const SilenceConfig& silence) const;
    bool matches_labels(const Alert& alert,
                       const std::vector<std::pair<std::string, std::string>>& matchers) const;
    bool is_active(const SilenceConfig& silence) const;
};

// Integrated alert deduplication system
class AlertDeduplicationSystem {
public:
    AlertDeduplicationSystem();
    ~AlertDeduplicationSystem();

    // Process incoming alert
    struct ProcessResult {
        bool is_duplicate;
        bool is_silenced;
        std::string group_id;
        std::optional<DeduplicationEntry> duplicate_info;
        std::vector<std::string> matching_silences;
    };

    ProcessResult process_alert(const Alert& alert);

    // Component access
    AlertDeduplication& get_deduplication() { return deduplication_; }
    AlertGrouping& get_grouping() { return grouping_; }
    SilenceManager& get_silence_manager() { return silence_manager_; }

    // Batch processing
    std::vector<ProcessResult> process_alerts(const std::vector<Alert>& alerts);

    // Maintenance
    void cleanup(std::chrono::seconds age_limit);
    void reset_statistics();

    // Statistics
    struct Statistics {
        size_t total_processed;
        size_t duplicates_found;
        size_t silenced_alerts;
        size_t groups_created;
        double deduplication_rate;
        double silence_rate;
    };

    Statistics get_statistics() const;

private:
    AlertDeduplication deduplication_;
    AlertGrouping grouping_;
    SilenceManager silence_manager_;

    std::atomic<size_t> total_processed_{0};
    std::atomic<size_t> duplicates_found_{0};
    std::atomic<size_t> silenced_count_{0};
};

// Deduplication configuration builder
class DeduplicationConfigBuilder {
public:
    DeduplicationConfigBuilder() = default;

    DeduplicationConfigBuilder& with_strategy(DeduplicationStrategy strategy) {
        strategy_ = strategy;
        return *this;
    }

    DeduplicationConfigBuilder& with_window(std::chrono::seconds window) {
        window_ = window;
        return *this;
    }

    DeduplicationConfigBuilder& with_similarity_threshold(double threshold) {
        threshold_ = threshold;
        return *this;
    }

    DeduplicationConfigBuilder& with_max_entries(size_t max_entries) {
        max_entries_ = max_entries;
        return *this;
    }

    void apply_to(AlertDeduplication& dedup) const {
        dedup.set_deduplication_strategy(strategy_);
        dedup.set_deduplication_window(window_);
        dedup.set_similarity_threshold(threshold_);
        dedup.set_max_entries(max_entries_);
    }

private:
    DeduplicationStrategy strategy_ = DeduplicationStrategy::EXACT_MATCH;
    std::chrono::seconds window_ = std::chrono::seconds(300);
    double threshold_ = 0.85;
    size_t max_entries_ = 10000;
};

} // namespace monitoring_system::alerting