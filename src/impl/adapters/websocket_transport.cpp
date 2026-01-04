// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file websocket_transport.cpp
 * @brief WebSocket transport implementation using network_system
 */

#include <kcenon/messaging/adapters/websocket_transport.h>

#if KCENON_WITH_NETWORK_SYSTEM

#include <kcenon/messaging/error/error_codes.h>
#include <kcenon/messaging/serialization/message_serializer.h>
#include <kcenon/network/core/messaging_ws_client.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace kcenon::messaging::adapters {

using namespace kcenon::common;
// Note: We don't use 'using namespace kcenon::network' to avoid
// VoidResult ambiguity between common::VoidResult and network::VoidResult

// Simple IJob wrapper for reconnect tasks
class reconnect_job : public common::interfaces::IJob {
public:
	explicit reconnect_job(std::function<void()> func)
		: func_(std::move(func)) {
	}

	common::VoidResult execute() override {
		func_();
		return common::ok();
	}

	std::string get_name() const override { return "websocket_reconnect_job"; }
	int get_priority() const override { return 0; }

private:
	std::function<void()> func_;
};

// ============================================================================
// websocket_transport::impl
// ============================================================================

class websocket_transport::impl {
public:
	explicit impl(const websocket_transport_config& config)
		: config_(config)
		, state_(transport_state::disconnected)
		, client_(std::make_shared<network::core::messaging_ws_client>(
			  generate_client_id()))
		, serializer_()
		, reconnect_attempts_(0)
		, current_reconnect_delay_(config.reconnect_delay) {
		setup_callbacks();
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

		network::core::ws_client_config ws_config;
		ws_config.host = config_.host;
		ws_config.port = config_.port;
		ws_config.path = config_.path;
		ws_config.connect_timeout = config_.connect_timeout;
		ws_config.ping_interval = config_.ping_interval;
		ws_config.auto_pong = config_.auto_pong;
		ws_config.auto_reconnect = false;  // We handle reconnection ourselves
		ws_config.max_message_size = config_.max_message_size;

		auto result = client_->start_client(ws_config);
		if (result.is_err()) {
			state_ = transport_state::error;
			notify_state_change(transport_state::error);
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to start WebSocket client: " +
					result.error().message));
		}

		return ok();
	}

	VoidResult disconnect() {
		std::lock_guard lock(mutex_);

		if (state_ == transport_state::disconnected) {
			return ok();
		}

		state_ = transport_state::disconnecting;
		notify_state_change(transport_state::disconnecting);

		// Cancel any pending reconnection
		reconnect_attempts_ = config_.max_retries + 1;

		auto result = client_->stop_client();
		client_->wait_for_stop();

		state_ = transport_state::disconnected;
		notify_state_change(transport_state::disconnected);

		return ok();
	}

	bool is_connected() const {
		return state_ == transport_state::connected && client_->is_connected();
	}

	transport_state get_state() const {
		return state_;
	}

	VoidResult send(const message& msg) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::broker_unavailable,
				"WebSocket transport is not connected"));
		}

		auto serialized = serializer_.serialize(msg);
		if (serialized.is_err()) {
			return VoidResult::err(error_info(
				error::message_serialization_failed,
				"Failed to serialize message: " +
					serialized.error().message));
		}

		auto& data = serialized.value();
		auto result = client_->send_binary(std::move(data));
		if (result.is_err()) {
			++stats_.errors;
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to send message: " +
					result.error().message));
		}

		++stats_.messages_sent;
		return ok();
	}

	VoidResult send_binary(const std::vector<uint8_t>& data) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::broker_unavailable,
				"WebSocket transport is not connected"));
		}

		std::vector<uint8_t> data_copy = data;
		auto result = client_->send_binary(std::move(data_copy));
		if (result.is_err()) {
			++stats_.errors;
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to send binary data: " +
					result.error().message));
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
	// WebSocket-specific methods
	// ========================================================================

	VoidResult subscribe(const std::string& topic_pattern) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::broker_unavailable,
				"WebSocket transport is not connected"));
		}

		{
			std::lock_guard lock(subscriptions_mutex_);
			subscriptions_.insert(topic_pattern);
		}

		// Send subscription request as a control message
		auto subscribe_msg = create_subscribe_message(topic_pattern);
		return send(subscribe_msg);
	}

	VoidResult unsubscribe(const std::string& topic_pattern) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::broker_unavailable,
				"WebSocket transport is not connected"));
		}

		{
			std::lock_guard lock(subscriptions_mutex_);
			subscriptions_.erase(topic_pattern);
		}

		// Send unsubscription request as a control message
		auto unsubscribe_msg = create_unsubscribe_message(topic_pattern);
		return send(unsubscribe_msg);
	}

	VoidResult unsubscribe_all() {
		std::lock_guard lock(subscriptions_mutex_);

		if (!is_connected()) {
			subscriptions_.clear();
			return ok();
		}

		VoidResult last_result = ok();
		for (const auto& topic : subscriptions_) {
			auto unsubscribe_msg = create_unsubscribe_message(topic);
			auto result = send(unsubscribe_msg);
			if (result.is_err()) {
				last_result = result;
			}
		}

		subscriptions_.clear();
		return last_result;
	}

	std::set<std::string> get_subscriptions() const {
		std::lock_guard lock(subscriptions_mutex_);
		return subscriptions_;
	}

	VoidResult send_text(const std::string& text) {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::broker_unavailable,
				"WebSocket transport is not connected"));
		}

		std::string text_copy = text;
		auto result = client_->send_text(std::move(text_copy));
		if (result.is_err()) {
			++stats_.errors;
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to send text: " +
					result.error().message));
		}

		stats_.bytes_sent += text.size();
		return ok();
	}

	VoidResult ping() {
		if (!is_connected()) {
			return VoidResult::err(error_info(
				error::broker_unavailable,
				"WebSocket transport is not connected"));
		}

		auto result = client_->send_ping();
		if (result.is_err()) {
			return VoidResult::err(error_info(
				error::publication_failed,
				"Failed to send ping: " +
					result.error().message));
		}

		return ok();
	}

	void set_disconnect_handler(
		std::function<void(uint16_t code, const std::string& reason)> handler) {
		std::lock_guard lock(handler_mutex_);
		disconnect_handler_ = std::move(handler);
	}

private:
	// ========================================================================
	// Helper methods
	// ========================================================================

	static std::string generate_client_id() {
		static std::atomic<uint64_t> counter{0};
		return "ws_transport_" + std::to_string(++counter);
	}

	void setup_callbacks() {
		client_->set_connected_callback([this]() {
			on_connected();
		});

		client_->set_disconnected_callback(
			[this](network::internal::ws_close_code code, const std::string& reason) {
				on_disconnected(static_cast<uint16_t>(code), reason);
			});

		client_->set_binary_message_callback(
			[this](const std::vector<uint8_t>& data) {
				on_binary_message(data);
			});

		client_->set_text_message_callback(
			[this](const std::string& text) {
				on_text_message(text);
			});

		client_->set_error_callback([this](std::error_code ec) {
			on_error(ec);
		});
	}

	void on_connected() {
		{
			std::lock_guard lock(mutex_);
			state_ = transport_state::connected;
			reconnect_attempts_ = 0;
			current_reconnect_delay_ = config_.reconnect_delay;
		}
		notify_state_change(transport_state::connected);

		// Resubscribe to all topics
		resubscribe_all();
	}

	void on_disconnected(uint16_t code, const std::string& reason) {
		bool should_reconnect = false;

		{
			std::lock_guard lock(mutex_);
			if (state_ == transport_state::disconnecting) {
				state_ = transport_state::disconnected;
			} else {
				state_ = transport_state::disconnected;
				should_reconnect = config_.auto_reconnect &&
					reconnect_attempts_ < config_.max_retries;
			}
		}

		notify_state_change(transport_state::disconnected);

		{
			std::lock_guard lock(handler_mutex_);
			if (disconnect_handler_) {
				disconnect_handler_(code, reason);
			}
		}

		if (should_reconnect) {
			schedule_reconnect();
		}
	}

	void on_binary_message(const std::vector<uint8_t>& data) {
		stats_.bytes_received += data.size();
		++stats_.messages_received;

		// Try to deserialize as a message
		auto msg_result = serializer_.deserialize_message(data);
		if (msg_result.is_ok()) {
			std::lock_guard lock(handler_mutex_);
			if (message_handler_) {
				message_handler_(msg_result.value());
			}
		} else {
			// Not a message, pass as binary
			std::lock_guard lock(handler_mutex_);
			if (binary_handler_) {
				binary_handler_(data);
			}
		}
	}

	void on_text_message(const std::string& text) {
		stats_.bytes_received += text.size();
	}

	void on_error(std::error_code ec) {
		++stats_.errors;

		std::lock_guard lock(handler_mutex_);
		if (error_handler_) {
			error_handler_(ec.message());
		}
	}

	void notify_state_change(transport_state new_state) {
		std::lock_guard lock(handler_mutex_);
		if (state_handler_) {
			state_handler_(new_state);
		}
	}

	void schedule_reconnect() {
		++reconnect_attempts_;

		auto reconnect_func = [this]() {
			std::this_thread::sleep_for(current_reconnect_delay_);

			// Apply exponential backoff
			current_reconnect_delay_ = std::chrono::milliseconds(
				static_cast<long long>(
					current_reconnect_delay_.count() *
					config_.reconnect_backoff_multiplier));

			if (current_reconnect_delay_ > config_.max_reconnect_delay) {
				current_reconnect_delay_ = config_.max_reconnect_delay;
			}

			// Attempt reconnection
			{
				std::lock_guard lock(mutex_);
				if (state_ == transport_state::disconnected &&
					reconnect_attempts_ <= config_.max_retries) {
					state_ = transport_state::connecting;
				} else {
					return;  // Abort reconnection
				}
			}

			notify_state_change(transport_state::connecting);

			network::core::ws_client_config ws_config;
			ws_config.host = config_.host;
			ws_config.port = config_.port;
			ws_config.path = config_.path;
			ws_config.connect_timeout = config_.connect_timeout;
			ws_config.ping_interval = config_.ping_interval;
			ws_config.auto_pong = config_.auto_pong;
			ws_config.auto_reconnect = false;
			ws_config.max_message_size = config_.max_message_size;

			auto result = client_->start_client(ws_config);
			if (result.is_err()) {
				{
					std::lock_guard lock(mutex_);
					state_ = transport_state::error;
				}
				notify_state_change(transport_state::error);

				// Try again if we have retries left
				if (reconnect_attempts_ < config_.max_retries) {
					schedule_reconnect();
				}
			}
		};

		if (config_.executor && config_.executor->is_running()) {
			// Use executor (preferred)
			auto job = std::make_unique<reconnect_job>(std::move(reconnect_func));
			auto result = config_.executor->execute(std::move(job));
			if (!result.is_ok()) {
				// If executor fails, fall back to std::thread
				std::thread(reconnect_func).detach();
			}
		} else {
			// Fallback to std::thread for backward compatibility
			std::thread(std::move(reconnect_func)).detach();
		}
	}

	void resubscribe_all() {
		std::lock_guard lock(subscriptions_mutex_);
		for (const auto& topic : subscriptions_) {
			auto subscribe_msg = create_subscribe_message(topic);
			send(subscribe_msg);  // Ignore errors during resubscription
		}
	}

	message create_subscribe_message(const std::string& topic_pattern) {
		auto result = message_builder()
			.topic("$sys/subscribe")
			.type(message_type::command)
			.header("pattern", topic_pattern)
			.build();

		if (result.is_ok()) {
			return std::move(result.value());
		}

		// Fallback: return empty message (should never happen)
		return message("$sys/subscribe");
	}

	message create_unsubscribe_message(const std::string& topic_pattern) {
		auto result = message_builder()
			.topic("$sys/unsubscribe")
			.type(message_type::command)
			.header("pattern", topic_pattern)
			.build();

		if (result.is_ok()) {
			return std::move(result.value());
		}

		// Fallback: return empty message (should never happen)
		return message("$sys/unsubscribe");
	}

	// ========================================================================
	// Member variables
	// ========================================================================

	websocket_transport_config config_;
	std::atomic<transport_state> state_;
	std::shared_ptr<network::core::messaging_ws_client> client_;
	serialization::message_serializer serializer_;

	mutable std::mutex mutex_;
	mutable std::mutex handler_mutex_;
	mutable std::mutex subscriptions_mutex_;

	std::set<std::string> subscriptions_;
	transport_statistics stats_;

	std::size_t reconnect_attempts_;
	std::chrono::milliseconds current_reconnect_delay_;

	// Handlers
	std::function<void(const message&)> message_handler_;
	std::function<void(const std::vector<uint8_t>&)> binary_handler_;
	std::function<void(transport_state)> state_handler_;
	std::function<void(const std::string&)> error_handler_;
	std::function<void(uint16_t, const std::string&)> disconnect_handler_;
};

// ============================================================================
// websocket_transport public interface
// ============================================================================

websocket_transport::websocket_transport(const websocket_transport_config& config)
	: pimpl_(std::make_unique<impl>(config)) {
}

websocket_transport::~websocket_transport() = default;

VoidResult websocket_transport::connect() {
	return pimpl_->connect();
}

VoidResult websocket_transport::disconnect() {
	return pimpl_->disconnect();
}

bool websocket_transport::is_connected() const {
	return pimpl_->is_connected();
}

transport_state websocket_transport::get_state() const {
	return pimpl_->get_state();
}

VoidResult websocket_transport::send(const message& msg) {
	return pimpl_->send(msg);
}

VoidResult websocket_transport::send_binary(const std::vector<uint8_t>& data) {
	return pimpl_->send_binary(data);
}

void websocket_transport::set_message_handler(
	std::function<void(const message&)> handler) {
	pimpl_->set_message_handler(std::move(handler));
}

void websocket_transport::set_binary_handler(
	std::function<void(const std::vector<uint8_t>&)> handler) {
	pimpl_->set_binary_handler(std::move(handler));
}

void websocket_transport::set_state_handler(
	std::function<void(transport_state)> handler) {
	pimpl_->set_state_handler(std::move(handler));
}

void websocket_transport::set_error_handler(
	std::function<void(const std::string&)> handler) {
	pimpl_->set_error_handler(std::move(handler));
}

transport_statistics websocket_transport::get_statistics() const {
	return pimpl_->get_statistics();
}

void websocket_transport::reset_statistics() {
	pimpl_->reset_statistics();
}

VoidResult websocket_transport::subscribe(const std::string& topic_pattern) {
	return pimpl_->subscribe(topic_pattern);
}

VoidResult websocket_transport::unsubscribe(const std::string& topic_pattern) {
	return pimpl_->unsubscribe(topic_pattern);
}

VoidResult websocket_transport::unsubscribe_all() {
	return pimpl_->unsubscribe_all();
}

std::set<std::string> websocket_transport::get_subscriptions() const {
	return pimpl_->get_subscriptions();
}

VoidResult websocket_transport::send_text(const std::string& text) {
	return pimpl_->send_text(text);
}

VoidResult websocket_transport::ping() {
	return pimpl_->ping();
}

void websocket_transport::set_disconnect_handler(
	std::function<void(uint16_t code, const std::string& reason)> handler) {
	pimpl_->set_disconnect_handler(std::move(handler));
}

}  // namespace kcenon::messaging::adapters

#endif  // KCENON_WITH_NETWORK_SYSTEM
