/**
 * @file log_filter.h
 * @brief Log filtering functionality
 */

#pragma once

#include <kcenon/logger/interfaces/log_filter_interface.h>
#include <kcenon/logger/interfaces/log_entry.h>
#include <regex>
#include <functional>

namespace kcenon::logger::filters {

/**
 * @brief Level-based log filter
 */
class level_filter : public log_filter_interface {
private:
    logger_system::log_level min_level_;

public:
    explicit level_filter(logger_system::log_level min_level) : min_level_(min_level) {}

    bool should_log(const log_entry& entry) const override {
        return static_cast<int>(entry.level) >= static_cast<int>(min_level_);
    }

    std::string get_name() const override {
        return "level_filter";
    }
};

/**
 * @brief Regex-based log filter
 */
class regex_filter : public log_filter_interface {
private:
    std::regex pattern_;
    bool include_matches_;

public:
    regex_filter(const std::string& pattern, bool include_matches = true)
        : pattern_(pattern), include_matches_(include_matches) {}

    bool should_log(const log_entry& entry) const override {
        bool matches = std::regex_search(entry.message.to_string(), pattern_);
        return include_matches_ ? matches : !matches;
    }

    std::string get_name() const override {
        return "regex_filter";
    }
};

/**
 * @brief Composite filter with AND/OR logic
 */
class composite_filter : public log_filter_interface {
public:
    enum class logic_type { AND, OR };

private:
    std::vector<std::unique_ptr<log_filter_interface>> filters_;
    logic_type logic_;

public:
    explicit composite_filter(logic_type logic) : logic_(logic) {}

    void add_filter(std::unique_ptr<log_filter_interface> filter) {
        filters_.push_back(std::move(filter));
    }

    bool should_log(const log_entry& entry) const override {
        if (filters_.empty()) return true;

        if (logic_ == logic_type::AND) {
            for (const auto& filter : filters_) {
                if (!filter->should_log(entry)) {
                    return false;
                }
            }
            return true;
        } else { // OR logic
            for (const auto& filter : filters_) {
                if (filter->should_log(entry)) {
                    return true;
                }
            }
            return false;
        }
    }

    std::string get_name() const override {
        return logic_ == logic_type::AND ? "composite_and_filter" : "composite_or_filter";
    }
};

/**
 * @brief Function-based filter
 */
class function_filter : public log_filter_interface {
private:
    std::function<bool(const log_entry&)> predicate_;

public:
    explicit function_filter(decltype(predicate_) predicate) : predicate_(std::move(predicate)) {}

    bool should_log(const log_entry& entry) const override {
        return predicate_(entry);
    }

    std::string get_name() const override {
        return "function_filter";
    }
};

} // namespace kcenon::logger::filters