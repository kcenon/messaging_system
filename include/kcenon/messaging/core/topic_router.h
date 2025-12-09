#pragma once

#include <kcenon/common/patterns/result.h>

#include "message.h"

#include <atomic>
#include <concepts>
#include <functional>
#include <shared_mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace kcenon::messaging {

// =============================================================================
// C++20 Concepts for Topic Router
// =============================================================================

/**
 * @concept SubscriptionCallable
 * @brief A callable type for message subscription callbacks.
 *
 * Types satisfying this concept can be invoked with a const message&
 * and return a VoidResult. This provides compile-time type safety
 * for subscription callbacks.
 *
 * @tparam F The callable type to validate
 */
template<typename F>
concept SubscriptionCallable = std::invocable<F, const message&> &&
	std::same_as<std::invoke_result_t<F, const message&>,
				 common::VoidResult>;

/**
 * @concept MessageFilterCallable
 * @brief A callable type for message filtering.
 *
 * Types satisfying this concept can be invoked with a const message&
 * and return a boolean indicating whether the message passes the filter.
 *
 * @tparam F The callable type to validate
 */
template<typename F>
concept MessageFilterCallable = std::invocable<F, const message&> &&
	std::convertible_to<std::invoke_result_t<F, const message&>, bool>;

/**
 * @brief Subscription callback function type
 */
using subscription_callback =
	std::function<common::VoidResult(const message&)>;

/**
 * @brief Message filter function type
 */
using message_filter = std::function<bool(const message&)>;

/**
 * @struct subscription
 * @brief Represents a topic subscription
 */
struct subscription {
	uint64_t id;
	std::string topic_pattern;	// Supports wildcards: *, #
	subscription_callback callback;
	message_filter filter;
	int priority;  // Higher = executed first

	/**
	 * @brief Check if this subscription matches the given topic
	 * @param topic Topic to match against
	 * @return true if matches, false otherwise
	 */
	bool matches(const std::string& topic) const;
};

/**
 * @class topic_router
 * @brief Routes messages based on topic patterns
 *
 * Topic pattern matching supports wildcards:
 * - '*' matches a single level (e.g., "user.*" matches "user.created" but not
 * "user.profile.updated")
 * - '#' matches multiple levels (e.g., "user.#" matches "user.created" and
 * "user.profile.updated")
 *
 * Examples:
 * - "user.*" matches "user.created", "user.updated"
 * - "user.#" matches "user.created", "user.profile.updated",
 * "user.profile.settings.changed"
 * - "*.created" matches "user.created", "order.created"
 */
class topic_router {
	std::unordered_map<std::string, std::vector<subscription>> subscriptions_;
	mutable std::shared_mutex mutex_;
	std::atomic<uint64_t> next_id_{1};

public:
	topic_router() = default;

	/**
	 * @brief Subscribe to a topic pattern
	 * @param pattern Topic pattern (supports * and # wildcards)
	 * @param callback Callback to invoke for matching messages
	 * @param filter Optional message filter
	 * @param priority Subscription priority (0-10, higher = first)
	 * @return Subscription ID or error
	 */
	common::Result<uint64_t> subscribe(const std::string& pattern,
									   subscription_callback callback,
									   message_filter filter = nullptr,
									   int priority = 5);

	/**
	 * @brief Subscribe to a topic pattern using C++20 concept constraints
	 *
	 * This overload accepts any callable that satisfies the SubscriptionCallable
	 * concept, providing better compile-time error messages.
	 *
	 * @tparam Callback A type satisfying SubscriptionCallable concept
	 * @tparam Filter A type satisfying MessageFilterCallable concept (optional)
	 * @param pattern Topic pattern (supports * and # wildcards)
	 * @param callback Any callable matching the subscription callback signature
	 * @param priority Subscription priority (0-10, higher = first)
	 * @return Subscription ID or error
	 *
	 * @example
	 * router.subscribe("user.*", [](const message& msg) -> common::VoidResult {
	 *     // Process message
	 *     return common::ok();
	 * });
	 */
	template<SubscriptionCallable Callback>
	common::Result<uint64_t> subscribe(const std::string& pattern,
									   Callback&& callback,
									   int priority = 5) {
		return subscribe(pattern,
						 subscription_callback(std::forward<Callback>(callback)),
						 nullptr,
						 priority);
	}

	/**
	 * @brief Subscribe to a topic pattern with filter using C++20 concept constraints
	 *
	 * @tparam Callback A type satisfying SubscriptionCallable concept
	 * @tparam Filter A type satisfying MessageFilterCallable concept
	 * @param pattern Topic pattern (supports * and # wildcards)
	 * @param callback Any callable matching the subscription callback signature
	 * @param filter Any callable matching the filter signature
	 * @param priority Subscription priority (0-10, higher = first)
	 * @return Subscription ID or error
	 */
	template<SubscriptionCallable Callback, MessageFilterCallable Filter>
	common::Result<uint64_t> subscribe(const std::string& pattern,
									   Callback&& callback,
									   Filter&& filter,
									   int priority = 5) {
		return subscribe(pattern,
						 subscription_callback(std::forward<Callback>(callback)),
						 message_filter(std::forward<Filter>(filter)),
						 priority);
	}

	/**
	 * @brief Unsubscribe by subscription ID
	 * @param subscription_id Subscription ID to remove
	 * @return Result indicating success or error
	 */
	common::VoidResult unsubscribe(uint64_t subscription_id);

	/**
	 * @brief Route a message to matching subscribers
	 * @param msg Message to route
	 * @return Result indicating success or error (returns error if no
	 * subscribers found or all subscribers failed)
	 */
	common::VoidResult route(const message& msg);

	/**
	 * @brief Get number of subscriptions for a topic
	 * @param topic Topic to count subscriptions for
	 * @return Number of subscriptions that would match this topic
	 */
	size_t subscriber_count(const std::string& topic) const;

	/**
	 * @brief Get all active topic patterns
	 * @return Vector of all registered topic patterns
	 */
	std::vector<std::string> get_topics() const;

	/**
	 * @brief Clear all subscriptions
	 */
	void clear();

private:
	/**
	 * @brief Find all subscriptions matching the given topic
	 * @param topic Topic to match
	 * @return Vector of subscription copies (thread-safe)
	 */
	std::vector<subscription> find_matching_subscriptions(
		const std::string& topic) const;

	/**
	 * @brief Match a topic against a pattern
	 * @param topic Topic string
	 * @param pattern Pattern string (may contain wildcards)
	 * @return true if topic matches pattern, false otherwise
	 */
	bool match_pattern(const std::string& topic,
					   const std::string& pattern) const;

	/**
	 * @brief Split a topic/pattern into segments
	 * @param str String to split
	 * @return Vector of segments
	 */
	std::vector<std::string> split_topic(const std::string& str) const;
};

}  // namespace kcenon::messaging
