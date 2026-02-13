#include <kcenon/messaging/patterns/request_reply.h>

#include <kcenon/messaging/error/messaging_error_category.h>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace kcenon::messaging::patterns {

using namespace kcenon::common;
using namespace kcenon::common::error;

// ============================================================================
// request_reply_handler implementation
// ============================================================================

request_reply_handler::request_reply_handler(
	std::shared_ptr<message_bus> bus,
	std::string service_topic,
	std::string reply_topic
)
	: bus_(std::move(bus)),
	  service_topic_(std::move(service_topic)),
	  reply_topic_(reply_topic.empty() ? service_topic_ + ".reply" : std::move(reply_topic)) {
}

request_reply_handler::~request_reply_handler() {
	cleanup_reply_subscription();
	if (service_subscription_id_ != 0 && bus_) {
		bus_->unsubscribe(service_subscription_id_);
	}
}

Result<message> request_reply_handler::request(
	message req,
	std::chrono::milliseconds timeout
) {
	if (!bus_) {
		return Result<message>::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!bus_->is_running()) {
		return Result<message>::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	// Setup reply subscription if not already done
	if (reply_subscription_id_ == 0) {
		auto setup_result = setup_reply_subscription();
		if (setup_result.is_err()) {
			return Result<message>(setup_result.error());
		}
	}

	// Generate correlation ID if not already set
	if (req.metadata().correlation_id.empty()) {
		req.metadata().correlation_id = generate_correlation_id();
	}
	std::string correlation_id = req.metadata().correlation_id;
	req.metadata().topic = service_topic_;

	// Create promise for reply
	std::promise<message> reply_promise;
	auto reply_future = reply_promise.get_future();

	{
		std::lock_guard lock(mutex_);
		pending_requests_[correlation_id] = std::move(reply_promise);
	}

	// Publish request
	auto pub_result = bus_->publish(req);
	if (pub_result.is_err()) {
		// Clean up pending request
		std::lock_guard lock(mutex_);
		pending_requests_.erase(correlation_id);
		return Result<message>(pub_result.error());
	}

	// Wait for reply with timeout
	auto status = reply_future.wait_for(timeout);
	if (status == std::future_status::timeout) {
		// Clean up pending request
		std::lock_guard lock(mutex_);
		pending_requests_.erase(correlation_id);
		return Result<message>::err(make_typed_error_code(messaging_error_category::publication_failed));
	}

	return reply_future.get();
}

VoidResult request_reply_handler::register_handler(
	std::function<Result<message>(const message&)> handler
) {
	if (!bus_) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!bus_->is_running()) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	if (!handler) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::subscription_failed));
	}

	// Unregister previous handler if exists
	if (service_subscription_id_ != 0) {
		auto unsub_result = bus_->unsubscribe(service_subscription_id_);
		if (unsub_result.is_err()) {
			return unsub_result;
		}
		service_subscription_id_ = 0;
	}

	request_handler_ = std::move(handler);

	// Subscribe to service topic
	auto sub_result = bus_->subscribe(
		service_topic_,
		[this](const message& msg) -> VoidResult {
			handle_request(msg);
			return ok();
		}
	);

	if (sub_result.is_err()) {
		return VoidResult(sub_result.error());
	}

	service_subscription_id_ = sub_result.unwrap();
	return ok();
}

VoidResult request_reply_handler::unregister_handler() {
	if (service_subscription_id_ == 0) {
		return ok();  // Nothing to unregister
	}

	if (!bus_) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	auto result = bus_->unsubscribe(service_subscription_id_);
	if (result.is_ok()) {
		service_subscription_id_ = 0;
		request_handler_ = nullptr;
	}

	return result;
}

bool request_reply_handler::has_handler() const {
	return request_handler_ != nullptr && service_subscription_id_ != 0;
}

void request_reply_handler::handle_reply(const message& reply) {
	std::string correlation_id = reply.metadata().correlation_id;
	if (correlation_id.empty()) {
		return;  // Invalid reply - no correlation ID
	}

	std::lock_guard lock(mutex_);
	auto it = pending_requests_.find(correlation_id);
	if (it != pending_requests_.end()) {
		it->second.set_value(reply);
		pending_requests_.erase(it);
	}
}

void request_reply_handler::handle_request(const message& request) {
	if (!request_handler_) {
		return;  // No handler registered
	}

	// Process request
	auto reply_result = request_handler_(request);

	// Send reply if correlation ID is present
	if (!request.metadata().correlation_id.empty()) {
		message reply_msg;

		if (reply_result.is_ok()) {
			reply_msg = reply_result.unwrap();
		} else {
			// Create error reply
			reply_msg = message();
			reply_msg.metadata().type = message_type::reply;
			// Store error info in headers
			reply_msg.metadata().headers["error"] = "true";
			reply_msg.metadata().headers["error_code"] =
				std::to_string(reply_result.error().code);
			reply_msg.metadata().headers["error_message"] =
				reply_result.error().message;
		}

		reply_msg.metadata().correlation_id = request.metadata().correlation_id;
		reply_msg.metadata().topic = reply_topic_;
		reply_msg.metadata().type = message_type::reply;

		bus_->publish(reply_msg);
	}
}

std::string request_reply_handler::generate_correlation_id() {
	static std::random_device rd;
	static std::mt19937_64 gen(rd());
	static std::uniform_int_distribution<uint64_t> dis;

	auto now = std::chrono::system_clock::now().time_since_epoch().count();
	auto random = dis(gen);

	std::ostringstream oss;
	oss << std::hex << std::setfill('0') << std::setw(16) << now
		<< std::setw(16) << random;

	return oss.str();
}

VoidResult request_reply_handler::setup_reply_subscription() {
	if (!bus_) {
		return VoidResult::err(make_typed_error_code(messaging_error_category::broker_unavailable));
	}

	auto sub_result = bus_->subscribe(
		reply_topic_,
		[this](const message& msg) -> VoidResult {
			handle_reply(msg);
			return ok();
		}
	);

	if (sub_result.is_err()) {
		return VoidResult(sub_result.error());
	}

	reply_subscription_id_ = sub_result.unwrap();
	return ok();
}

void request_reply_handler::cleanup_reply_subscription() {
	if (reply_subscription_id_ != 0 && bus_) {
		bus_->unsubscribe(reply_subscription_id_);
		reply_subscription_id_ = 0;
	}
}

// ============================================================================
// request_client implementation
// ============================================================================

request_client::request_client(
	std::shared_ptr<message_bus> bus,
	std::string service_topic
)
	: handler_(std::make_shared<request_reply_handler>(
		  std::move(bus), std::move(service_topic))) {
}

Result<message> request_client::request(
	message req,
	std::chrono::milliseconds timeout
) {
	return handler_->request(std::move(req), timeout);
}

// ============================================================================
// request_server implementation
// ============================================================================

request_server::request_server(
	std::shared_ptr<message_bus> bus,
	std::string service_topic
)
	: handler_(std::make_shared<request_reply_handler>(
		  std::move(bus), std::move(service_topic))) {
}

VoidResult request_server::register_handler(
	std::function<Result<message>(const message&)> handler
) {
	return handler_->register_handler(std::move(handler));
}

VoidResult request_server::stop() {
	return handler_->unregister_handler();
}

}  // namespace kcenon::messaging::patterns
