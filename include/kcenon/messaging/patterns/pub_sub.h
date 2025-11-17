#pragma once

#include "../core/message_bus.h"
#include <kcenon/common/patterns/result.h>
#include <memory>
#include <string>
#include <vector>

namespace kcenon::messaging::patterns {

/**
 * @class publisher
 * @brief High-level publisher for pub/sub pattern
 *
 * Provides a simplified interface for publishing messages to topics.
 * Wraps message_bus and provides convenient methods for message publishing.
 */
class publisher {
	std::shared_ptr<message_bus> bus_;
	std::string default_topic_;

public:
	/**
	 * @brief Construct a publisher
	 * @param bus Message bus to publish to
	 * @param default_topic Default topic for messages (optional)
	 */
	publisher(std::shared_ptr<message_bus> bus, std::string default_topic = "");

	/**
	 * @brief Publish a message to the default topic
	 * @param msg Message to publish
	 * @return Result indicating success or error
	 */
	common::VoidResult publish(message msg);

	/**
	 * @brief Publish a message to a specific topic
	 * @param topic Topic to publish to
	 * @param msg Message to publish
	 * @return Result indicating success or error
	 */
	common::VoidResult publish(const std::string& topic, message msg);

	/**
	 * @brief Get the default topic
	 * @return Default topic string
	 */
	const std::string& get_default_topic() const { return default_topic_; }

	/**
	 * @brief Set the default topic
	 * @param topic New default topic
	 */
	void set_default_topic(std::string topic) { default_topic_ = std::move(topic); }

	/**
	 * @brief Check if publisher is ready
	 * @return true if the underlying message bus is running
	 */
	bool is_ready() const { return bus_ && bus_->is_running(); }
};

/**
 * @class subscriber
 * @brief High-level subscriber for pub/sub pattern
 *
 * Provides a simplified interface for subscribing to topics.
 * Manages subscription lifecycle and automatically unsubscribes on destruction.
 */
class subscriber {
	std::shared_ptr<message_bus> bus_;
	std::vector<uint64_t> subscription_ids_;
	mutable std::mutex mutex_;

public:
	/**
	 * @brief Construct a subscriber
	 * @param bus Message bus to subscribe to
	 */
	explicit subscriber(std::shared_ptr<message_bus> bus);

	/**
	 * @brief Destructor - automatically unsubscribes from all topics
	 */
	~subscriber();

	// Non-copyable
	subscriber(const subscriber&) = delete;
	subscriber& operator=(const subscriber&) = delete;

	// Movable
	subscriber(subscriber&&) noexcept = default;
	subscriber& operator=(subscriber&&) noexcept = default;

	/**
	 * @brief Subscribe to a topic pattern
	 * @param topic_pattern Topic pattern to subscribe to (supports wildcards)
	 * @param callback Callback function to invoke for matching messages
	 * @param filter Optional message filter
	 * @param priority Subscription priority (default: 5)
	 * @return Subscription ID or error
	 */
	common::Result<uint64_t> subscribe(
		const std::string& topic_pattern,
		subscription_callback callback,
		message_filter filter = nullptr,
		int priority = 5
	);

	/**
	 * @brief Unsubscribe from a specific subscription
	 * @param subscription_id Subscription ID to remove
	 * @return Result indicating success or error
	 */
	common::VoidResult unsubscribe(uint64_t subscription_id);

	/**
	 * @brief Unsubscribe from all topics
	 * @return Result indicating success or error
	 */
	common::VoidResult unsubscribe_all();

	/**
	 * @brief Get the number of active subscriptions
	 * @return Number of active subscriptions
	 */
	size_t subscription_count() const;

	/**
	 * @brief Check if subscriber has any active subscriptions
	 * @return true if there are active subscriptions
	 */
	bool has_subscriptions() const { return subscription_count() > 0; }
};

}  // namespace kcenon::messaging::patterns
