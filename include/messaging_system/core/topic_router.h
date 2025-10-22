#pragma once

#include <kcenon/common/patterns/result.h>
#include <kcenon/common/interfaces/executor_interface.h>
#include "messaging_container.h"
#include <functional>
#include <regex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <atomic>

namespace messaging {

using common::VoidResult;

class TopicRouter {
public:
    using SubscriberCallback = std::function<VoidResult(const MessagingContainer&)>;
    using Filter = std::function<bool(const MessagingContainer&)>;

    struct Subscription {
        uint64_t id;
        std::string topic_pattern;  // Supports: "user.*.created", "order.#"
        SubscriberCallback callback;
        Filter filter;
        int priority;  // 0-10, higher = more priority
    };

private:
    std::unordered_map<std::string, std::vector<Subscription>> subscriptions_;
    std::shared_ptr<common::interfaces::IExecutor> executor_;
    std::atomic<uint64_t> next_subscription_id_{1};
    mutable std::shared_mutex mutex_;

public:
    TopicRouter(
        std::shared_ptr<common::interfaces::IExecutor> executor,
        size_t queue_capacity = 10000
    );

    // Subscription management
    common::Result<uint64_t> subscribe(
        const std::string& topic_pattern,
        SubscriberCallback callback,
        Filter filter = nullptr,
        int priority = 5
    );

    VoidResult unsubscribe(uint64_t subscription_id);

    // Message routing
    VoidResult route(const MessagingContainer& msg);

    // Statistics
    size_t subscriber_count() const;
    size_t pending_messages() const;

private:
    std::vector<Subscription> find_matching_subscriptions(const std::string& topic);
    bool match_pattern(const std::string& topic, const std::string& pattern);
    std::regex pattern_to_regex(const std::string& pattern);
};

} // namespace messaging
