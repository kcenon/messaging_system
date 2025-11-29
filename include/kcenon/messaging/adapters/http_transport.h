// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file http_transport.h
 * @brief HTTP transport adapter using network_system
 *
 * Provides HTTP-based message transport with support for:
 * - HTTP/1.1 protocol
 * - GET, POST, PUT, DELETE methods
 * - Request/response messaging
 * - Binary and JSON serialization
 */

#pragma once

#include <kcenon/messaging/adapters/transport_interface.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace kcenon::messaging::adapters {

/**
 * @enum http_content_type
 * @brief HTTP content types for message serialization
 */
enum class http_content_type {
    json,           // application/json
    binary,         // application/octet-stream
    msgpack         // application/msgpack
};

/**
 * @struct http_transport_config
 * @brief Configuration for HTTP transport
 */
struct http_transport_config : transport_config {
    std::string base_path = "/api/messages";
    http_content_type content_type = http_content_type::json;
    bool use_ssl = false;
    std::map<std::string, std::string> default_headers;

    // Endpoints
    std::string publish_endpoint = "/publish";
    std::string subscribe_endpoint = "/subscribe";
    std::string request_endpoint = "/request";
};

/**
 * @class http_transport
 * @brief HTTP transport implementation using network_system::http_client
 *
 * This transport is suitable for:
 * - Request/reply messaging patterns
 * - REST-based message APIs
 * - Environments where WebSocket is not available
 *
 * Note: For real-time pub/sub, use websocket_transport instead.
 *
 * Usage Example:
 * @code
 * http_transport_config config;
 * config.host = "api.example.com";
 * config.port = 443;
 * config.use_ssl = true;
 * config.base_path = "/v1/messages";
 *
 * auto transport = std::make_shared<http_transport>(config);
 * auto result = transport->connect();
 *
 * if (result.is_ok()) {
 *     auto msg = message_builder()
 *         .topic("orders.new")
 *         .source("client-001")
 *         .build();
 *     transport->send(msg.value());
 * }
 * @endcode
 */
class http_transport : public transport_interface {
public:
    /**
     * @brief Construct HTTP transport with configuration
     * @param config Transport configuration
     */
    explicit http_transport(const http_transport_config& config);

    /**
     * @brief Destructor
     */
    ~http_transport() override;

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

    // HTTP-specific methods

    /**
     * @brief Send message with HTTP POST
     * @param endpoint Target endpoint (relative to base_path)
     * @param msg Message to send
     * @return Result with response message or error
     */
    common::Result<message> post(const std::string& endpoint, const message& msg);

    /**
     * @brief Send HTTP GET request
     * @param endpoint Target endpoint
     * @param query Query parameters
     * @return Result with response message or error
     */
    common::Result<message> get(
        const std::string& endpoint,
        const std::map<std::string, std::string>& query = {});

    /**
     * @brief Set custom header for all requests
     * @param key Header name
     * @param value Header value
     */
    void set_header(const std::string& key, const std::string& value);

    /**
     * @brief Remove custom header
     * @param key Header name
     */
    void remove_header(const std::string& key);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace kcenon::messaging::adapters
