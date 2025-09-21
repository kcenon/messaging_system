#pragma once

#include "../core/message_types.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <regex>
#include <string>
#include <mutex>
#include <atomic>

namespace kcenon::messaging::routing {

    // Message filter interface
    class message_filter {
    public:
        virtual ~message_filter() = default;
        virtual bool matches(const core::message& msg) const = 0;
        virtual std::string describe() const = 0;
        virtual std::unique_ptr<message_filter> clone() const = 0;
    };

    // Topic pattern filter
    class topic_pattern_filter : public message_filter {
    private:
        std::regex pattern_;
        std::string pattern_str_;

    public:
        explicit topic_pattern_filter(const std::string& pattern)
            : pattern_(pattern), pattern_str_(pattern) {}

        bool matches(const core::message& msg) const override {
            return std::regex_match(msg.payload.topic, pattern_);
        }

        std::string describe() const override {
            return "TopicPattern: " + pattern_str_;
        }

        std::unique_ptr<message_filter> clone() const override {
            return std::make_unique<topic_pattern_filter>(pattern_str_);
        }
    };

    // Priority filter
    class priority_filter : public message_filter {
    private:
        core::message_priority min_priority_;
        core::message_priority max_priority_;

    public:
        priority_filter(core::message_priority min_prio, core::message_priority max_prio)
            : min_priority_(min_prio), max_priority_(max_prio) {}

        bool matches(const core::message& msg) const override {
            return msg.metadata.priority >= min_priority_ && msg.metadata.priority <= max_priority_;
        }

        std::string describe() const override {
            return "Priority: " + std::to_string(static_cast<int>(min_priority_)) +
                   "-" + std::to_string(static_cast<int>(max_priority_));
        }

        std::unique_ptr<message_filter> clone() const override {
            return std::make_unique<priority_filter>(min_priority_, max_priority_);
        }
    };

    // Content filter
    class content_filter : public message_filter {
    private:
        std::string key_;
        std::string expected_value_;

    public:
        content_filter(const std::string& key, const std::string& value)
            : key_(key), expected_value_(value) {}

        bool matches(const core::message& msg) const override {
            auto it = msg.payload.data.find(key_);
            if (it != msg.payload.data.end()) {
                if (std::holds_alternative<std::string>(it->second)) {
                    return std::get<std::string>(it->second) == expected_value_;
                }
            }
            return false;
        }

        std::string describe() const override {
            return "Content: " + key_ + "=" + expected_value_;
        }

        std::unique_ptr<message_filter> clone() const override {
            return std::make_unique<content_filter>(key_, expected_value_);
        }
    };

    // Composite filter (AND/OR operations)
    class composite_filter : public message_filter {
    public:
        enum class operation { AND, OR };

    private:
        std::vector<std::unique_ptr<message_filter>> filters_;
        operation op_;

    public:
        explicit composite_filter(operation op) : op_(op) {}

        void add_filter(std::unique_ptr<message_filter> filter) {
            filters_.push_back(std::move(filter));
        }

        bool matches(const core::message& msg) const override {
            if (filters_.empty()) return true;

            if (op_ == operation::AND) {
                for (const auto& filter : filters_) {
                    if (!filter->matches(msg)) return false;
                }
                return true;
            } else { // OR
                for (const auto& filter : filters_) {
                    if (filter->matches(msg)) return true;
                }
                return false;
            }
        }

        std::string describe() const override {
            std::string result = "(";
            std::string op_str = (op_ == operation::AND) ? " AND " : " OR ";

            for (size_t i = 0; i < filters_.size(); ++i) {
                if (i > 0) result += op_str;
                result += filters_[i]->describe();
            }
            result += ")";
            return result;
        }

        std::unique_ptr<message_filter> clone() const override {
            auto cloned = std::make_unique<composite_filter>(op_);
            for (const auto& filter : filters_) {
                cloned->add_filter(filter->clone());
            }
            return cloned;
        }
    };

    // Route destination
    struct route_destination {
        std::string name;
        std::function<void(const core::message&)> handler;
        std::atomic<uint64_t> message_count{0};
        std::atomic<bool> enabled{true};

        route_destination(const std::string& dest_name,
                         std::function<void(const core::message&)> dest_handler)
            : name(dest_name), handler(std::move(dest_handler)) {}
    };

    // Routing rule
    class routing_rule {
    private:
        std::string id_;
        std::string description_;
        std::unique_ptr<message_filter> filter_;
        std::vector<std::shared_ptr<route_destination>> destinations_;
        std::atomic<uint64_t> match_count_{0};
        std::atomic<bool> enabled_{true};

    public:
        routing_rule(const std::string& id, const std::string& description,
                    std::unique_ptr<message_filter> filter)
            : id_(id), description_(description), filter_(std::move(filter)) {}

        const std::string& get_id() const { return id_; }
        const std::string& get_description() const { return description_; }

        bool is_enabled() const { return enabled_; }
        void set_enabled(bool enabled) { enabled_ = enabled; }

        uint64_t get_match_count() const { return match_count_; }

        void add_destination(std::shared_ptr<route_destination> dest) {
            destinations_.push_back(std::move(dest));
        }

        bool matches(const core::message& msg) const {
            return enabled_ && filter_->matches(msg);
        }

        void route_message(const core::message& msg) {
            if (!matches(msg)) return;

            match_count_++;
            for (auto& dest : destinations_) {
                if (dest->enabled) {
                    try {
                        dest->handler(msg);
                        dest->message_count++;
                    } catch (const std::exception& e) {
                        // Log error but continue with other destinations
                    }
                }
            }
        }

        std::string get_filter_description() const {
            return filter_->describe();
        }

        std::vector<std::shared_ptr<route_destination>> get_destinations() const {
            return destinations_;
        }
    };

    // Advanced message router
    class advanced_router {
    private:
        mutable std::shared_mutex rules_mutex_;
        std::vector<std::unique_ptr<routing_rule>> rules_;
        std::unordered_map<std::string, std::shared_ptr<route_destination>> destinations_;
        std::atomic<uint64_t> total_messages_processed_{0};
        std::atomic<uint64_t> total_messages_routed_{0};

        // Default handler for unrouted messages
        std::function<void(const core::message&)> default_handler_;

    public:
        advanced_router() = default;

        // Rule management
        void add_rule(std::unique_ptr<routing_rule> rule) {
            std::unique_lock<std::shared_mutex> lock(rules_mutex_);
            rules_.push_back(std::move(rule));
        }

        bool remove_rule(const std::string& rule_id) {
            std::unique_lock<std::shared_mutex> lock(rules_mutex_);
            auto it = std::find_if(rules_.begin(), rules_.end(),
                [&rule_id](const auto& rule) { return rule->get_id() == rule_id; });

            if (it != rules_.end()) {
                rules_.erase(it);
                return true;
            }
            return false;
        }

        routing_rule* get_rule(const std::string& rule_id) const {
            std::shared_lock<std::shared_mutex> lock(rules_mutex_);
            auto it = std::find_if(rules_.begin(), rules_.end(),
                [&rule_id](const auto& rule) { return rule->get_id() == rule_id; });

            return it != rules_.end() ? it->get() : nullptr;
        }

        std::vector<std::string> get_rule_ids() const {
            std::shared_lock<std::shared_mutex> lock(rules_mutex_);
            std::vector<std::string> ids;
            for (const auto& rule : rules_) {
                ids.push_back(rule->get_id());
            }
            return ids;
        }

        // Destination management
        void register_destination(const std::string& name,
                                 std::function<void(const core::message&)> handler) {
            auto dest = std::make_shared<route_destination>(name, std::move(handler));
            destinations_[name] = dest;
        }

        std::shared_ptr<route_destination> get_destination(const std::string& name) const {
            auto it = destinations_.find(name);
            return it != destinations_.end() ? it->second : nullptr;
        }

        void set_default_handler(std::function<void(const core::message&)> handler) {
            default_handler_ = std::move(handler);
        }

        // Message routing
        void route_message(const core::message& msg) {
            total_messages_processed_++;
            bool routed = false;

            std::shared_lock<std::shared_mutex> lock(rules_mutex_);
            for (auto& rule : rules_) {
                if (rule->matches(msg)) {
                    rule->route_message(msg);
                    routed = true;
                    total_messages_routed_++;
                }
            }

            // Use default handler if no rules matched
            if (!routed && default_handler_) {
                default_handler_(msg);
            }
        }

        // Statistics
        struct routing_statistics {
            uint64_t total_messages_processed;
            uint64_t total_messages_routed;
            uint64_t unrouted_messages;
            size_t active_rules;
            size_t total_destinations;
            std::vector<std::pair<std::string, uint64_t>> rule_stats;
            std::vector<std::pair<std::string, uint64_t>> destination_stats;
        };

        routing_statistics get_statistics() const {
            routing_statistics stats;
            stats.total_messages_processed = total_messages_processed_;
            stats.total_messages_routed = total_messages_routed_;
            stats.unrouted_messages = stats.total_messages_processed - stats.total_messages_routed;
            stats.total_destinations = destinations_.size();

            std::shared_lock<std::shared_mutex> lock(rules_mutex_);
            stats.active_rules = 0;

            for (const auto& rule : rules_) {
                if (rule->is_enabled()) stats.active_rules++;
                stats.rule_stats.emplace_back(rule->get_id(), rule->get_match_count());
            }

            for (const auto& [name, dest] : destinations_) {
                stats.destination_stats.emplace_back(name, dest->message_count.load());
            }

            return stats;
        }

        void reset_statistics() {
            total_messages_processed_ = 0;
            total_messages_routed_ = 0;

            std::shared_lock<std::shared_mutex> lock(rules_mutex_);
            for (const auto& [name, dest] : destinations_) {
                dest->message_count = 0;
            }
        }
    };

    // Router builder for easy configuration
    class router_builder {
    private:
        std::unique_ptr<advanced_router> router_;

    public:
        router_builder() : router_(std::make_unique<advanced_router>()) {}

        // Add topic-based routing
        router_builder& route_topic_pattern(const std::string& rule_id,
                                           const std::string& pattern,
                                           const std::string& destination_name,
                                           std::function<void(const core::message&)> handler) {
            auto filter = std::make_unique<topic_pattern_filter>(pattern);
            auto rule = std::make_unique<routing_rule>(rule_id,
                "Topic pattern: " + pattern, std::move(filter));

            router_->register_destination(destination_name, std::move(handler));
            auto dest = router_->get_destination(destination_name);
            rule->add_destination(dest);
            router_->add_rule(std::move(rule));

            return *this;
        }

        // Add priority-based routing
        router_builder& route_priority(const std::string& rule_id,
                                     core::message_priority min_priority,
                                     core::message_priority max_priority,
                                     const std::string& destination_name,
                                     std::function<void(const core::message&)> handler) {
            auto filter = std::make_unique<priority_filter>(min_priority, max_priority);
            auto rule = std::make_unique<routing_rule>(rule_id,
                "Priority filter", std::move(filter));

            router_->register_destination(destination_name, std::move(handler));
            auto dest = router_->get_destination(destination_name);
            rule->add_destination(dest);
            router_->add_rule(std::move(rule));

            return *this;
        }

        // Add content-based routing
        router_builder& route_content(const std::string& rule_id,
                                    const std::string& key,
                                    const std::string& value,
                                    const std::string& destination_name,
                                    std::function<void(const core::message&)> handler) {
            auto filter = std::make_unique<content_filter>(key, value);
            auto rule = std::make_unique<routing_rule>(rule_id,
                "Content filter: " + key + "=" + value, std::move(filter));

            router_->register_destination(destination_name, std::move(handler));
            auto dest = router_->get_destination(destination_name);
            rule->add_destination(dest);
            router_->add_rule(std::move(rule));

            return *this;
        }

        // Set default handler
        router_builder& set_default(std::function<void(const core::message&)> handler) {
            router_->set_default_handler(std::move(handler));
            return *this;
        }

        std::unique_ptr<advanced_router> build() {
            return std::move(router_);
        }
    };

} // namespace kcenon::messaging::routing