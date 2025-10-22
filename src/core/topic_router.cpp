#include "messaging_system/core/topic_router.h"
#include "messaging_system/error_codes.h"
#include <kcenon/common/patterns/result.h>
#include <algorithm>

namespace messaging {

TopicRouter::TopicRouter(
    std::shared_ptr<common::interfaces::IExecutor> executor,
    [[maybe_unused]] size_t queue_capacity
)
    : executor_(std::move(executor))
{
    // TODO: Initialize bounded_job_queue with capacity
}

common::Result<uint64_t> TopicRouter::subscribe(
    const std::string& topic_pattern,
    SubscriberCallback callback,
    Filter filter,
    int priority
) {
    std::unique_lock lock(mutex_);

    uint64_t id = next_subscription_id_++;

    Subscription sub{
        id,
        topic_pattern,
        std::move(callback),
        std::move(filter),
        priority
    };

    subscriptions_[topic_pattern].push_back(std::move(sub));

    // Sort by priority (descending)
    auto& subs = subscriptions_[topic_pattern];
    std::sort(subs.begin(), subs.end(),
        [](const Subscription& a, const Subscription& b) {
            return a.priority > b.priority;
        });

    return common::ok(id);
}

VoidResult TopicRouter::unsubscribe(uint64_t subscription_id) {
    std::unique_lock lock(mutex_);

    for (auto& [topic, subs] : subscriptions_) {
        auto it = std::find_if(subs.begin(), subs.end(),
            [subscription_id](const Subscription& sub) {
                return sub.id == subscription_id;
            });

        if (it != subs.end()) {
            subs.erase(it);
            return common::ok(std::monostate{});
        }
    }

    return common::make_error<std::monostate>(
        common::error_info{
            error::SUBSCRIPTION_FAILED,
            "Subscription not found",
            "TopicRouter::unsubscribe",
            ""
        }
    );
}

VoidResult TopicRouter::route(const MessagingContainer& msg) {
    std::shared_lock lock(mutex_);

    auto matching = find_matching_subscriptions(msg.topic());

    if (matching.empty()) {
        return common::make_error<std::monostate>(
            common::error_info{
                error::NO_SUBSCRIBERS,
                "No subscribers for topic: " + msg.topic(),
                "TopicRouter::route",
                ""
            }
        );
    }

    // Execute callbacks via executor for async processing
    for (const auto& sub : matching) {
        // Apply filter if present
        if (sub.filter && !sub.filter(msg)) {
            continue;
        }

        // Execute callback asynchronously via executor
        executor_->submit([callback = sub.callback, msg]() {
            auto result = callback(msg);
            if (result.is_err()) {
                // TODO: Log error when logger is available
                // For now, silently continue with other subscribers
            }
        });
    }

    return common::ok(std::monostate{});
}

size_t TopicRouter::subscriber_count() const {
    std::shared_lock lock(mutex_);

    size_t total = 0;
    for (const auto& [_, subs] : subscriptions_) {
        total += subs.size();
    }
    return total;
}

size_t TopicRouter::pending_messages() const {
    // TODO: Implement queue depth tracking
    return 0;
}

std::vector<TopicRouter::Subscription> TopicRouter::find_matching_subscriptions(const std::string& topic) {
    std::vector<Subscription> result;

    for (const auto& [pattern, subs] : subscriptions_) {
        if (match_pattern(topic, pattern)) {
            result.insert(result.end(), subs.begin(), subs.end());
        }
    }

    return result;
}

bool TopicRouter::match_pattern(const std::string& topic, const std::string& pattern) {
    // Exact match
    if (topic == pattern) {
        return true;
    }

    // No wildcards - no match
    if (pattern.find('*') == std::string::npos && pattern.find('#') == std::string::npos) {
        return false;
    }

    // Convert pattern to regex and match
    try {
        auto regex_pattern = pattern_to_regex(pattern);
        return std::regex_match(topic, regex_pattern);
    } catch (const std::exception&) {
        return false;
    }
}

std::regex TopicRouter::pattern_to_regex(const std::string& pattern) {
    std::string regex_str;
    regex_str.reserve(pattern.size() * 2);
    regex_str += "^";

    for (size_t i = 0; i < pattern.size(); ++i) {
        char c = pattern[i];

        if (c == '*') {
            // Single-level wildcard: matches one segment
            // "user.*" should match "user.created" but not "user.admin.created"
            regex_str += "[^.]+";
        } else if (c == '#') {
            // Multi-level wildcard: matches zero or more segments
            // "order.#" should match "order", "order.placed", "order.placed.confirmed"
            if (i > 0 && pattern[i-1] == '.') {
                // After a dot: match current and all subsequent segments
                regex_str += ".*";
            } else if (i == 0) {
                // At start: match everything
                regex_str += ".*";
            } else {
                // Treat as literal #
                regex_str += "\\#";
            }
        } else if (c == '.') {
            // Literal dot separator
            regex_str += "\\.";
        } else if (c == '^' || c == '$' || c == '\\' || c == '[' || c == ']' ||
                   c == '(' || c == ')' || c == '{' || c == '}' || c == '|' ||
                   c == '+' || c == '?') {
            // Escape regex special characters
            regex_str += '\\';
            regex_str += c;
        } else {
            // Regular character
            regex_str += c;
        }
    }

    regex_str += "$";
    return std::regex(regex_str);
}

} // namespace messaging
