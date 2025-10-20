#include "messaging_system/core/topic_router.h"
#include "messaging_system/error_codes.h"
#include <kcenon/common/patterns/error_info.h>
#include <algorithm>

namespace messaging {

TopicRouter::TopicRouter(
    std::shared_ptr<common::IExecutor> executor,
    size_t queue_capacity
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

    return common::Result<uint64_t>::ok(id);
}

common::Result<void> TopicRouter::unsubscribe(uint64_t subscription_id) {
    std::unique_lock lock(mutex_);

    for (auto& [topic, subs] : subscriptions_) {
        auto it = std::find_if(subs.begin(), subs.end(),
            [subscription_id](const Subscription& sub) {
                return sub.id == subscription_id;
            });

        if (it != subs.end()) {
            subs.erase(it);
            return common::VoidResult::ok();
        }
    }

    return common::VoidResult::error(
        common::error_info{
            error::SUBSCRIPTION_FAILED,
            "Subscription not found",
            "TopicRouter::unsubscribe",
            ""
        }
    );
}

common::Result<void> TopicRouter::route(const MessagingContainer& msg) {
    std::shared_lock lock(mutex_);

    auto matching = find_matching_subscriptions(msg.topic());

    if (matching.empty()) {
        return common::VoidResult::error(
            common::error_info{
                error::NO_SUBSCRIBERS,
                "No subscribers for topic: " + msg.topic(),
                "TopicRouter::route",
                ""
            }
        );
    }

    // Execute callbacks
    for (const auto& sub : matching) {
        // Apply filter if present
        if (sub.filter && !sub.filter(msg)) {
            continue;
        }

        // TODO: Execute via executor for async processing
        auto result = sub.callback(msg);
        if (result.is_error()) {
            // Log error but continue with other subscribers
        }
    }

    return common::VoidResult::ok();
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

    // TODO: Implement wildcard pattern matching
    // "user.*" should match "user.created", "user.deleted"
    // "order.#" should match "order", "order.placed", "order.placed.confirmed"

    return false;
}

std::regex TopicRouter::pattern_to_regex(const std::string& pattern) {
    // TODO: Convert topic pattern to regex
    // "user.*" -> "user\\.[^.]*"
    // "order.#" -> "order(\\..*)?"

    return std::regex(pattern);
}

} // namespace messaging
