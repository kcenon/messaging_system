// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file http_transport.cpp
 * @brief HTTP transport implementation using network_system
 */

#include <kcenon/messaging/adapters/http_transport.h>

#if KCENON_WITH_NETWORK_SYSTEM

#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/messaging/serialization/message_serializer.h>
#include <kcenon/network/core/http_client.h>
#include <kcenon/network/internal/http_types.h>

#include <atomic>
#include <mutex>
#include <sstream>

namespace kcenon::messaging::adapters {

using namespace kcenon::common;
// Note: We don't use 'using namespace kcenon::network' to avoid
// Result type ambiguity between common::Result and network::Result

// ============================================================================
// http_transport::impl
// ============================================================================

class http_transport::impl {
public:
	explicit impl(const http_transport_config& config)
		: config_(config)
		, state_(transport_state::disconnected)
		, client_(std::make_shared<network::core::http_client>(
			  config.request_timeout))
		, serializer_() {
		// Initialize default headers
		headers_ = config_.default_headers;
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

		// HTTP is stateless, no actual disconnection needed
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

		// Use the configured publish endpoint
		auto result = post_internal(config_.publish_endpoint, msg);
		if (result.is_err()) {
			return VoidResult::err(result.error());
		}

		return ok();
	}

	VoidResult send_binary(const std::vector<uint8_t>& data) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::not_connected,
				"HTTP transport is not connected"));
		}

		auto url = build_url(config_.publish_endpoint);
		std::map<std::string, std::string> headers = headers_;
		headers["Content-Type"] = "application/octet-stream";

		auto result = client_->post(url, data, headers);
		if (result.is_err()) {
			++stats_.errors;
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to send binary data: " + result.error().message));
		}

		auto& response = result.value();
		if (response.status_code >= 400) {
			++stats_.errors;
			return VoidResult::err(error_info(
				error::publication_failed,
				"HTTP error: " + std::to_string(response.status_code) +
					" " + response.status_message));
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

		auto url = build_url(endpoint);
		std::map<std::string, std::string> headers = headers_;

		auto result = client_->get(url, query, headers);
		if (result.is_err()) {
			++stats_.errors;
			notify_error("GET request failed: " + result.error().message);
			return Result<message>::err(error_info(
				error::publication_failed,
				"GET request failed: " + result.error().message));
		}

		auto& response = result.value();
		if (response.status_code >= 400) {
			++stats_.errors;
			return Result<message>::err(error_info(
				error::publication_failed,
				"HTTP error: " + std::to_string(response.status_code) +
					" " + response.status_message));
		}

		++stats_.messages_received;
		stats_.bytes_received += response.body.size();

		// Try to deserialize response as message
		auto msg_result = serializer_.deserialize_message(response.body);
		if (msg_result.is_err()) {
			// If not a message, create one from the response body
			auto response_msg = message_builder()
				.topic(endpoint)
				.type(message_type::reply)
				.build();

			if (response_msg.is_ok()) {
				// Store response body in message payload
				auto& built_msg = response_msg.value();
				built_msg.payload().set(
					"body", response.get_body_string());
				built_msg.payload().set(
					"status_code", response.status_code);
				return ok(std::move(built_msg));
			}

			return Result<message>::err(error_info(
				error::message_deserialization_failed,
				"Failed to parse response"));
		}

		return ok(std::move(msg_result.value()));
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

	std::string build_url(const std::string& endpoint) const {
		std::ostringstream url;
		url << (config_.use_ssl ? "https://" : "http://");
		url << config_.host;
		if ((config_.use_ssl && config_.port != 443) ||
			(!config_.use_ssl && config_.port != 80)) {
			url << ":" << config_.port;
		}
		url << config_.base_path << endpoint;
		return url.str();
	}

	std::string get_content_type() const {
		switch (config_.content_type) {
		case http_content_type::json:
			return "application/json";
		case http_content_type::binary:
			return "application/octet-stream";
		case http_content_type::msgpack:
			return "application/msgpack";
		default:
			return "application/octet-stream";
		}
	}

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

		auto url = build_url(endpoint);
		std::map<std::string, std::string> headers = headers_;
		headers["Content-Type"] = get_content_type();

		auto result = client_->post(url, serialized.value(), headers);
		if (result.is_err()) {
			++stats_.errors;
			notify_error("POST request failed: " + result.error().message);
			return Result<message>::err(error_info(
				error::publication_failed,
				"POST request failed: " + result.error().message));
		}

		auto& response = result.value();
		if (response.status_code >= 400) {
			++stats_.errors;
			return Result<message>::err(error_info(
				error::publication_failed,
				"HTTP error: " + std::to_string(response.status_code) +
					" " + response.status_message));
		}

		++stats_.messages_sent;
		stats_.bytes_sent += serialized.value().size();

		// Process response
		if (!response.body.empty()) {
			++stats_.messages_received;
			stats_.bytes_received += response.body.size();

			// Try to deserialize as message
			auto msg_result = serializer_.deserialize_message(response.body);
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
				built_msg.payload().set(
					"body", response.get_body_string());
				built_msg.payload().set(
					"status_code", response.status_code);
				return ok(std::move(built_msg));
			}
		}

		// Return empty success message for responses without body
		auto success_msg = message_builder()
			.topic(endpoint)
			.type(message_type::reply)
			.build();

		if (success_msg.is_ok()) {
			success_msg.value().payload().set(
				"status_code", response.status_code);
			return ok(std::move(success_msg.value()));
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
	std::shared_ptr<network::core::http_client> client_;
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
