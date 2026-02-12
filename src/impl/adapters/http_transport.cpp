// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file http_transport.cpp
 * @brief HTTP transport implementation using network_system v2.0 facade API
 */

#include <kcenon/messaging/adapters/http_transport.h>

#if KCENON_WITH_NETWORK_SYSTEM

#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/messaging/serialization/message_serializer.h>
#include <kcenon/network/facade/http_facade.h>
#include <kcenon/network/interfaces/i_protocol_client.h>
#include <kcenon/network/interfaces/connection_observer.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <sstream>

namespace kcenon::messaging::adapters {

using namespace kcenon::common;
// Note: We don't use 'using namespace kcenon::network' to avoid
// Result type ambiguity between common::Result and network::Result

// ============================================================================
// transport_observer - bridges network_system observer to transport handlers
// ============================================================================

class transport_observer : public network::interfaces::null_connection_observer {
public:
	void on_receive(std::span<const uint8_t> data) override {
		std::lock_guard lock(mutex_);
		last_response_.assign(data.begin(), data.end());
		response_received_ = true;
		cv_.notify_one();
	}

	void on_connected() override {
		// HTTP facade start() is synchronous, no additional handling needed
	}

	void on_error(std::error_code ec) override {
		std::lock_guard lock(mutex_);
		last_error_ = ec;
		response_received_ = true;
		cv_.notify_one();
	}

	// Wait for response from an HTTP request and return it
	std::optional<std::vector<uint8_t>> wait_for_response(
		std::chrono::milliseconds timeout) {
		std::unique_lock lock(mutex_);
		if (cv_.wait_for(lock, timeout, [this] { return response_received_; })) {
			response_received_ = false;
			if (last_error_) {
				last_error_.reset();
				return std::nullopt;
			}
			auto result = std::move(last_response_);
			last_response_.clear();
			return result;
		}
		return std::nullopt;
	}

	void reset() {
		std::lock_guard lock(mutex_);
		response_received_ = false;
		last_response_.clear();
		last_error_.reset();
	}

private:
	mutable std::mutex mutex_;
	std::condition_variable cv_;
	bool response_received_ = false;
	std::vector<uint8_t> last_response_;
	std::optional<std::error_code> last_error_;
};

// ============================================================================
// http_transport::impl
// ============================================================================

class http_transport::impl {
public:
	explicit impl(const http_transport_config& config)
		: config_(config)
		, state_(transport_state::disconnected)
		, observer_(std::make_shared<transport_observer>())
		, serializer_() {
		// Initialize default headers
		headers_ = config_.default_headers;

		// Create HTTP client via facade
		network::facade::http_facade facade;
		client_ = facade.create_client({
			.timeout = config.request_timeout,
			.use_ssl = config.use_ssl,
			.path = config.base_path + config.publish_endpoint
		});
		client_->set_observer(observer_);
	}

	~impl() {
		disconnect();
	}

	// ========================================================================
	// transport_interface implementation
	// ========================================================================

	VoidResult connect() {
		std::lock_guard lock(mutex_);

		if (state_ == transport_state::connected) {
			return ok();
		}

		if (state_ == transport_state::connecting) {
			return VoidResult::err(error_info(
				error::already_running,
				"Connection already in progress"));
		}

		state_ = transport_state::connecting;
		notify_state_change(transport_state::connecting);

		// HTTP is stateless, so we just validate configuration
		if (config_.host.empty()) {
			state_ = transport_state::error;
			notify_state_change(transport_state::error);
			return VoidResult::err(error_info(
				error::invalid_message,
				"HTTP transport host is not configured"));
		}

		if (config_.port == 0) {
			// Use default port based on SSL setting
			config_.port = config_.use_ssl ? 443 : 80;
		}

		// Start the facade client (sets target host:port)
		auto start_result = client_->start(config_.host, config_.port);
		if (start_result.is_err()) {
			state_ = transport_state::error;
			notify_state_change(transport_state::error);
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to start HTTP client: " +
					start_result.error().message));
		}

		state_ = transport_state::connected;
		notify_state_change(transport_state::connected);

		return ok();
	}

	VoidResult disconnect() {
		std::lock_guard lock(mutex_);

		if (state_ == transport_state::disconnected) {
			return ok();
		}

		state_ = transport_state::disconnecting;
		notify_state_change(transport_state::disconnecting);

		// Stop the facade client
		[[maybe_unused]] auto stop_result = client_->stop();

		state_ = transport_state::disconnected;
		notify_state_change(transport_state::disconnected);

		return ok();
	}

	bool is_connected() const {
		return state_ == transport_state::connected;
	}

	transport_state get_state() const {
		return state_;
	}

	VoidResult send(const message& msg) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::not_connected,
				"HTTP transport is not connected"));
		}

		auto serialized = serializer_.serialize(msg);
		if (serialized.is_err()) {
			return VoidResult::err(error_info(
				error::message_serialization_failed,
				"Failed to serialize message: " +
					serialized.error().message));
		}

		observer_->reset();
		auto send_result = client_->send(std::move(serialized.value()));
		if (send_result.is_err()) {
			++stats_.errors;
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to send message: " +
					send_result.error().message));
		}

		// Wait for response
		auto response = observer_->wait_for_response(config_.request_timeout);
		if (response.has_value() && !response->empty()) {
			++stats_.messages_received;
			stats_.bytes_received += response->size();

			// Try to deserialize and notify handler
			auto msg_result = serializer_.deserialize_message(response.value());
			if (msg_result.is_ok()) {
				std::lock_guard lock(handler_mutex_);
				if (message_handler_) {
					message_handler_(msg_result.value());
				}
			}
		}

		++stats_.messages_sent;
		return ok();
	}

	VoidResult send_binary(const std::vector<uint8_t>& data) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::not_connected,
				"HTTP transport is not connected"));
		}

		std::vector<uint8_t> data_copy = data;
		observer_->reset();
		auto result = client_->send(std::move(data_copy));
		if (result.is_err()) {
			++stats_.errors;
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to send binary data: " + result.error().message));
		}

		stats_.bytes_sent += data.size();
		return ok();
	}

	void set_message_handler(
		std::function<void(const message&)> handler) {
		std::lock_guard lock(handler_mutex_);
		message_handler_ = std::move(handler);
	}

	void set_binary_handler(
		std::function<void(const std::vector<uint8_t>&)> handler) {
		std::lock_guard lock(handler_mutex_);
		binary_handler_ = std::move(handler);
	}

	void set_state_handler(
		std::function<void(transport_state)> handler) {
		std::lock_guard lock(handler_mutex_);
		state_handler_ = std::move(handler);
	}

	void set_error_handler(
		std::function<void(const std::string&)> handler) {
		std::lock_guard lock(handler_mutex_);
		error_handler_ = std::move(handler);
	}

	transport_statistics get_statistics() const {
		return stats_;
	}

	void reset_statistics() {
		stats_ = transport_statistics{};
	}

	// ========================================================================
	// HTTP-specific methods
	// ========================================================================

	Result<message> post(const std::string& endpoint, const message& msg) {
		if (!is_connected()) {
			return Result<message>::err(error_info(
				error::not_connected,
				"HTTP transport is not connected"));
		}

		return post_internal(endpoint, msg);
	}

	Result<message> get(
		const std::string& endpoint,
		const std::map<std::string, std::string>& query) {
		if (!is_connected()) {
			return Result<message>::err(error_info(
				error::not_connected,
				"HTTP transport is not connected"));
		}

		// With facade API, GET is implemented as a POST with query params
		// encoded in the request body, using a per-endpoint client
		network::facade::http_facade facade;
		auto endpoint_client = facade.create_client({
			.timeout = config_.request_timeout,
			.use_ssl = config_.use_ssl,
			.path = config_.base_path + endpoint
		});

		auto endpoint_observer = std::make_shared<transport_observer>();
		endpoint_client->set_observer(endpoint_observer);

		auto start_result = endpoint_client->start(config_.host, config_.port);
		if (start_result.is_err()) {
			++stats_.errors;
			notify_error("GET request failed: " + start_result.error().message);
			return Result<message>::err(error_info(
				error::publication_failed,
				"GET request failed: " + start_result.error().message));
		}

		// Encode query parameters as simple key=value pairs in the body
		std::ostringstream query_body;
		bool first = true;
		for (const auto& [key, value] : query) {
			if (!first) query_body << '&';
			query_body << key << '=' << value;
			first = false;
		}
		auto query_str = query_body.str();
		std::vector<uint8_t> query_data(query_str.begin(), query_str.end());

		auto send_result = endpoint_client->send(std::move(query_data));
		if (send_result.is_err()) {
			++stats_.errors;
			notify_error("GET request failed: " + send_result.error().message);
			[[maybe_unused]] auto _ = endpoint_client->stop();
			return Result<message>::err(error_info(
				error::publication_failed,
				"GET request failed: " + send_result.error().message));
		}

		auto response = endpoint_observer->wait_for_response(config_.request_timeout);
		[[maybe_unused]] auto _ = endpoint_client->stop();

		if (!response.has_value()) {
			++stats_.errors;
			return Result<message>::err(error_info(
				error::publication_failed,
				"GET request timed out"));
		}

		++stats_.messages_received;
		stats_.bytes_received += response->size();

		// Try to deserialize response as message
		auto msg_result = serializer_.deserialize_message(response.value());
		if (msg_result.is_ok()) {
			return ok(std::move(msg_result.value()));
		}

		// If not a message, create one from the response body
		auto response_msg = message_builder()
			.topic(endpoint)
			.type(message_type::reply)
			.build();

		if (response_msg.is_ok()) {
			auto& built_msg = response_msg.value();
			std::string body_str(response->begin(), response->end());
			built_msg.payload().set("body", body_str);
			return ok(std::move(built_msg));
		}

		return Result<message>::err(error_info(
			error::message_deserialization_failed,
			"Failed to parse response"));
	}

	void set_header(const std::string& key, const std::string& value) {
		std::lock_guard lock(mutex_);
		headers_[key] = value;
	}

	void remove_header(const std::string& key) {
		std::lock_guard lock(mutex_);
		headers_.erase(key);
	}

private:
	// ========================================================================
	// Helper methods
	// ========================================================================

	Result<message> post_internal(
		const std::string& endpoint,
		const message& msg) {

		auto serialized = serializer_.serialize(msg);
		if (serialized.is_err()) {
			return Result<message>::err(error_info(
				error::message_serialization_failed,
				"Failed to serialize message: " +
					serialized.error().message));
		}

		// Create per-endpoint client for specific endpoint routing
		network::facade::http_facade facade;
		auto endpoint_client = facade.create_client({
			.timeout = config_.request_timeout,
			.use_ssl = config_.use_ssl,
			.path = config_.base_path + endpoint
		});

		auto endpoint_observer = std::make_shared<transport_observer>();
		endpoint_client->set_observer(endpoint_observer);

		auto start_result = endpoint_client->start(config_.host, config_.port);
		if (start_result.is_err()) {
			++stats_.errors;
			notify_error("POST request failed: " + start_result.error().message);
			return Result<message>::err(error_info(
				error::publication_failed,
				"POST request failed: " + start_result.error().message));
		}

		auto send_result = endpoint_client->send(std::move(serialized.value()));
		if (send_result.is_err()) {
			++stats_.errors;
			notify_error("POST request failed: " + send_result.error().message);
			[[maybe_unused]] auto stop_res = endpoint_client->stop();
			return Result<message>::err(error_info(
				error::publication_failed,
				"POST request failed: " + send_result.error().message));
		}

		++stats_.messages_sent;

		// Wait for response
		auto response = endpoint_observer->wait_for_response(config_.request_timeout);
		[[maybe_unused]] auto stop_res = endpoint_client->stop();

		if (!response.has_value() || response->empty()) {
			// Return empty success message for responses without body
			auto success_msg = message_builder()
				.topic(endpoint)
				.type(message_type::reply)
				.build();

			if (success_msg.is_ok()) {
				return ok(std::move(success_msg.value()));
			}

			return Result<message>::err(error_info(
				error::publication_failed,
				"Failed to create response message"));
		}

		++stats_.messages_received;
		stats_.bytes_received += response->size();

		// Process response
		auto msg_result = serializer_.deserialize_message(response.value());
		if (msg_result.is_ok()) {
			// Notify handler if set
			{
				std::lock_guard lock(handler_mutex_);
				if (message_handler_) {
					message_handler_(msg_result.value());
				}
			}
			return ok(std::move(msg_result.value()));
		}

		// Not a message, create response message from body
		auto response_msg = message_builder()
			.topic(endpoint)
			.type(message_type::reply)
			.build();

		if (response_msg.is_ok()) {
			auto& built_msg = response_msg.value();
			std::string body_str(response->begin(), response->end());
			built_msg.payload().set("body", body_str);
			return ok(std::move(built_msg));
		}

		return Result<message>::err(error_info(
			error::publication_failed,
			"Failed to create response message"));
	}

	void notify_state_change(transport_state new_state) {
		std::lock_guard lock(handler_mutex_);
		if (state_handler_) {
			state_handler_(new_state);
		}
	}

	void notify_error(const std::string& error_msg) {
		std::lock_guard lock(handler_mutex_);
		if (error_handler_) {
			error_handler_(error_msg);
		}
	}

	// ========================================================================
	// Member variables
	// ========================================================================

	http_transport_config config_;
	std::atomic<transport_state> state_;
	std::shared_ptr<network::interfaces::i_protocol_client> client_;
	std::shared_ptr<transport_observer> observer_;
	serialization::message_serializer serializer_;

	mutable std::mutex mutex_;
	mutable std::mutex handler_mutex_;

	std::map<std::string, std::string> headers_;
	transport_statistics stats_;

	// Handlers
	std::function<void(const message&)> message_handler_;
	std::function<void(const std::vector<uint8_t>&)> binary_handler_;
	std::function<void(transport_state)> state_handler_;
	std::function<void(const std::string&)> error_handler_;
};

// ============================================================================
// http_transport public interface
// ============================================================================

http_transport::http_transport(const http_transport_config& config)
	: pimpl_(std::make_unique<impl>(config)) {
}

http_transport::~http_transport() = default;

VoidResult http_transport::connect() {
	return pimpl_->connect();
}

VoidResult http_transport::disconnect() {
	return pimpl_->disconnect();
}

bool http_transport::is_connected() const {
	return pimpl_->is_connected();
}

transport_state http_transport::get_state() const {
	return pimpl_->get_state();
}

VoidResult http_transport::send(const message& msg) {
	return pimpl_->send(msg);
}

VoidResult http_transport::send_binary(const std::vector<uint8_t>& data) {
	return pimpl_->send_binary(data);
}

void http_transport::set_message_handler(
	std::function<void(const message&)> handler) {
	pimpl_->set_message_handler(std::move(handler));
}

void http_transport::set_binary_handler(
	std::function<void(const std::vector<uint8_t>&)> handler) {
	pimpl_->set_binary_handler(std::move(handler));
}

void http_transport::set_state_handler(
	std::function<void(transport_state)> handler) {
	pimpl_->set_state_handler(std::move(handler));
}

void http_transport::set_error_handler(
	std::function<void(const std::string&)> handler) {
	pimpl_->set_error_handler(std::move(handler));
}

transport_statistics http_transport::get_statistics() const {
	return pimpl_->get_statistics();
}

void http_transport::reset_statistics() {
	pimpl_->reset_statistics();
}

Result<message> http_transport::post(
	const std::string& endpoint,
	const message& msg) {
	return pimpl_->post(endpoint, msg);
}

Result<message> http_transport::get(
	const std::string& endpoint,
	const std::map<std::string, std::string>& query) {
	return pimpl_->get(endpoint, query);
}

void http_transport::set_header(
	const std::string& key,
	const std::string& value) {
	pimpl_->set_header(key, value);
}

void http_transport::remove_header(const std::string& key) {
	pimpl_->remove_header(key);
}

}  // namespace kcenon::messaging::adapters

#endif  // KCENON_WITH_NETWORK_SYSTEM
