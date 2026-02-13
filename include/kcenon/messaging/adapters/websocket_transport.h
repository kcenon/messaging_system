// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file websocket_transport.h
 * @brief WebSocket transport adapter using network_system
 *
 * Provides WebSocket-based message transport with support for:
 * - Bidirectional real-time communication
 * - Pub/sub messaging pattern
 * - Topic subscriptions
 * - Automatic reconnection
 *
 * @note Requires KCENON_WITH_NETWORK_SYSTEM=1 for full functionality.
 *       When disabled, provides stub implementation that returns not_supported errors.
 */

#pragma once

#include <kcenon/messaging/config/feature_flags.h>
#include <kcenon/messaging/adapters/transport_interface.h>
#include <kcenon/messaging/error/messaging_error_category.h>
#include <kcenon/common/interfaces/executor_interface.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace kcenon::messaging::adapters {

#if KCENON_WITH_NETWORK_SYSTEM

/**
 * @struct websocket_transport_config
 * @brief Configuration for WebSocket transport
 */
struct websocket_transport_config : transport_config {
    std::string path = "/ws";
    bool use_ssl = false;
    std::chrono::milliseconds ping_interval{30000};
    bool auto_pong = true;
    std::size_t max_message_size = 10 * 1024 * 1024;  // 10MB

    // Reconnection settings
    std::chrono::milliseconds reconnect_delay{1000};
    double reconnect_backoff_multiplier = 2.0;
    std::chrono::milliseconds max_reconnect_delay{30000};

    // Optional executor for background tasks (recommended)
    // If provided, reconnection uses it instead of std::thread
    std::shared_ptr<common::interfaces::IExecutor> executor = nullptr;
};

/**
 * @class websocket_transport
 * @brief WebSocket transport implementation using network_system::messaging_ws_client
 *
 * This transport is ideal for:
 * - Real-time pub/sub messaging
 * - Event streaming
 * - Low-latency bidirectional communication
 *
 * Features:
 * - Topic-based subscriptions with wildcards (* and #)
 * - Automatic reconnection with exponential backoff
 * - Ping/pong keepalive
 * - Binary and text message support
 *
 * Usage Example:
 * @code
 * websocket_transport_config config;
 * config.host = "ws.example.com";
 * config.port = 8080;
 * config.path = "/messaging";
 * config.auto_reconnect = true;
 *
 * auto transport = std::make_shared<websocket_transport>(config);
 *
 * transport->set_message_handler([](const message& msg) {
 *     std::cout << "Received: " << msg.metadata().topic << std::endl;
 * });
 *
 * transport->connect();
 * transport->subscribe("events.user.*");
 * transport->subscribe("events.order.#");
 *
 * // Send message
 * auto msg = message_builder()
 *     .topic("events.user.login")
 *     .source("client-001")
 *     .build();
 * transport->send(msg.value());
 * @endcode
 */
class websocket_transport : public transport_interface {
public:
    /**
     * @brief Indicates if WebSocket transport is available at compile time
     *
     * When KCENON_WITH_NETWORK_SYSTEM is enabled, this is true and the
     * transport provides full functionality. When disabled, this is false
     * and all operations return not_supported errors.
     */
    static constexpr bool is_available = true;

    /**
     * @brief Construct WebSocket transport with configuration
     * @param config Transport configuration
     */
    explicit websocket_transport(const websocket_transport_config& config);

    /**
     * @brief Destructor
     */
    ~websocket_transport() override;

    // transport_interface implementation
    common::VoidResult connect() override;
    common::VoidResult disconnect() override;
    bool is_connected() const override;
    transport_state get_state() const override;

    common::VoidResult send(const message& msg) override;
    common::VoidResult send_binary(const std::vector<uint8_t>& data) override;

    void set_message_handler(
        std::function<void(const message&)> handler) override;
    void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)> handler) override;
    void set_state_handler(
        std::function<void(transport_state)> handler) override;
    void set_error_handler(
        std::function<void(const std::string&)> handler) override;

    transport_statistics get_statistics() const override;
    void reset_statistics() override;

    // WebSocket-specific methods

    /**
     * @brief Subscribe to a topic pattern
     * @param topic_pattern Topic pattern with optional wildcards
     *        - '*' matches one level (e.g., "events.*.created")
     *        - '#' matches multiple levels (e.g., "events.#")
     * @return Result indicating success or error
     */
    common::VoidResult subscribe(const std::string& topic_pattern);

    /**
     * @brief Unsubscribe from a topic pattern
     * @param topic_pattern Topic pattern to unsubscribe
     * @return Result indicating success or error
     */
    common::VoidResult unsubscribe(const std::string& topic_pattern);

    /**
     * @brief Unsubscribe from all topics
     * @return Result indicating success or error
     */
    common::VoidResult unsubscribe_all();

    /**
     * @brief Get current subscriptions
     * @return Set of subscribed topic patterns
     */
    std::set<std::string> get_subscriptions() const;

    /**
     * @brief Send text message directly
     * @param text Text message to send
     * @return Result indicating success or error
     */
    common::VoidResult send_text(const std::string& text);

    /**
     * @brief Send ping to check connection
     * @return Result indicating success or error
     */
    common::VoidResult ping();

    /**
     * @brief Set callback for disconnection events
     * @param handler Callback with close code and reason
     */
    void set_disconnect_handler(
        std::function<void(uint16_t code, const std::string& reason)> handler);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

#else // !KCENON_WITH_NETWORK_SYSTEM

/**
 * @struct websocket_transport_config
 * @brief Configuration for WebSocket transport (stub version)
 */
struct websocket_transport_config : transport_config {
    std::string path = "/ws";
    bool use_ssl = false;
    std::chrono::milliseconds ping_interval{30000};
    bool auto_pong = true;
    std::size_t max_message_size = 10 * 1024 * 1024;
    std::chrono::milliseconds reconnect_delay{1000};
    double reconnect_backoff_multiplier = 2.0;
    std::chrono::milliseconds max_reconnect_delay{30000};
};

/**
 * @class websocket_transport
 * @brief Stub WebSocket transport when network_system is not available
 *
 * All operations return error::not_supported to indicate that the
 * transport functionality requires network_system to be enabled.
 *
 * Enable with: cmake -DKCENON_WITH_NETWORK_SYSTEM=ON
 */
class websocket_transport : public transport_interface {
public:
    /**
     * @brief Indicates if WebSocket transport is available at compile time
     *
     * This is false when KCENON_WITH_NETWORK_SYSTEM is disabled.
     * Use this to conditionally handle unavailable transport at compile time.
     */
    static constexpr bool is_available = false;

    explicit websocket_transport(const websocket_transport_config& /*config*/) {}
    ~websocket_transport() override = default;

    common::VoidResult connect() override {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    common::VoidResult disconnect() override {
        return common::ok();
    }

    bool is_connected() const override { return false; }

    transport_state get_state() const override {
        return transport_state::disconnected;
    }

    common::VoidResult send(const message& /*msg*/) override {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    common::VoidResult send_binary(const std::vector<uint8_t>& /*data*/) override {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    void set_message_handler(std::function<void(const message&)> /*handler*/) override {}
    void set_binary_handler(std::function<void(const std::vector<uint8_t>&)> /*handler*/) override {}
    void set_state_handler(std::function<void(transport_state)> /*handler*/) override {}
    void set_error_handler(std::function<void(const std::string&)> /*handler*/) override {}

    transport_statistics get_statistics() const override { return {}; }
    void reset_statistics() override {}

    // WebSocket-specific stubs
    common::VoidResult subscribe(const std::string& /*topic_pattern*/) {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    common::VoidResult unsubscribe(const std::string& /*topic_pattern*/) {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    common::VoidResult unsubscribe_all() {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    std::set<std::string> get_subscriptions() const { return {}; }

    common::VoidResult send_text(const std::string& /*text*/) {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    common::VoidResult ping() {
        return common::VoidResult::err(
            make_typed_error_code(messaging_error_category::not_supported));
    }

    void set_disconnect_handler(
        std::function<void(uint16_t, const std::string&)> /*handler*/) {}
};

#endif // KCENON_WITH_NETWORK_SYSTEM

} // namespace kcenon::messaging::adapters
