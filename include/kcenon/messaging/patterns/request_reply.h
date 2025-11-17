#pragma once

#include "../core/message_bus.h"
#include <kcenon/common/patterns/result.h>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace kcenon::messaging::patterns {

/**
 * @class request_reply_handler
 * @brief Handles request-reply messaging pattern
 *
 * Implements synchronous request-reply over asynchronous pub/sub messaging.
 * Uses correlation IDs to match requests with replies.
 */
class request_reply_handler {
	std::shared_ptr<message_bus> bus_;
	std::string service_topic_;
	std::string reply_topic_;
	uint64_t reply_subscription_id_{0};

	// Pending requests waiting for replies
	std::unordered_map<std::string, std::promise<message>> pending_requests_;
	mutable std::mutex mutex_;

	// Service-side request handler
	std::function<common::Result<message>(const message&)> request_handler_;

public:
	/**
	 * @brief Construct a request-reply handler
	 * @param bus Message bus to use
	 * @param service_topic Topic for service requests
	 * @param reply_topic Topic for replies (default: service_topic + ".reply")
	 */
	request_reply_handler(
		std::shared_ptr<message_bus> bus,
		std::string service_topic,
		std::string reply_topic = ""
	);

	/**
	 * @brief Destructor - cleanup subscriptions
	 */
	~request_reply_handler();

	// Non-copyable
	request_reply_handler(const request_reply_handler&) = delete;
	request_reply_handler& operator=(const request_reply_handler&) = delete;

	// Movable
	request_reply_handler(request_reply_handler&&) noexcept = default;
	request_reply_handler& operator=(request_reply_handler&&) noexcept = default;

	/**
	 * @brief Send a request and wait for reply (client side)
	 * @param req Request message
	 * @param timeout Maximum time to wait for reply
	 * @return Reply message or error
	 */
	common::Result<message> request(
		message req,
		std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
	);

	/**
	 * @brief Register a request handler (service side)
	 * @param handler Function to handle incoming requests
	 * @return Result indicating success or error
	 */
	common::VoidResult register_handler(
		std::function<common::Result<message>(const message&)> handler
	);

	/**
	 * @brief Unregister the request handler
	 * @return Result indicating success or error
	 */
	common::VoidResult unregister_handler();

	/**
	 * @brief Check if handler is registered
	 * @return true if a request handler is registered
	 */
	bool has_handler() const;

	/**
	 * @brief Get the service topic
	 * @return Service topic string
	 */
	const std::string& get_service_topic() const { return service_topic_; }

	/**
	 * @brief Get the reply topic
	 * @return Reply topic string
	 */
	const std::string& get_reply_topic() const { return reply_topic_; }

private:
	/**
	 * @brief Handle incoming reply messages
	 * @param reply Reply message
	 */
	void handle_reply(const message& reply);

	/**
	 * @brief Handle incoming request messages (service side)
	 * @param request Request message
	 */
	void handle_request(const message& request);

	/**
	 * @brief Generate a unique correlation ID
	 * @return Correlation ID string
	 */
	std::string generate_correlation_id();

	/**
	 * @brief Setup reply subscription
	 * @return Result indicating success or error
	 */
	common::VoidResult setup_reply_subscription();

	/**
	 * @brief Cleanup reply subscription
	 */
	void cleanup_reply_subscription();

	// Service subscription ID
	uint64_t service_subscription_id_{0};
};

/**
 * @class request_client
 * @brief Simplified client for making requests
 *
 * Provides a high-level interface for making request-reply calls.
 */
class request_client {
	std::shared_ptr<request_reply_handler> handler_;

public:
	/**
	 * @brief Construct a request client
	 * @param bus Message bus to use
	 * @param service_topic Topic to send requests to
	 */
	request_client(std::shared_ptr<message_bus> bus, std::string service_topic);

	/**
	 * @brief Send a request and wait for reply
	 * @param req Request message
	 * @param timeout Maximum time to wait for reply
	 * @return Reply message or error
	 */
	common::Result<message> request(
		message req,
		std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
	);
};

/**
 * @class request_server
 * @brief Simplified server for handling requests
 *
 * Provides a high-level interface for implementing request-reply services.
 */
class request_server {
	std::shared_ptr<request_reply_handler> handler_;

public:
	/**
	 * @brief Construct a request server
	 * @param bus Message bus to use
	 * @param service_topic Topic to listen for requests on
	 */
	request_server(std::shared_ptr<message_bus> bus, std::string service_topic);

	/**
	 * @brief Register a request handler
	 * @param handler Function to handle incoming requests
	 * @return Result indicating success or error
	 */
	common::VoidResult register_handler(
		std::function<common::Result<message>(const message&)> handler
	);

	/**
	 * @brief Stop handling requests
	 * @return Result indicating success or error
	 */
	common::VoidResult stop();
};

}  // namespace kcenon::messaging::patterns
