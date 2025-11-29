// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file transport_interface.h
 * @brief Abstract interface for network transport adapters
 *
 * This interface defines the contract for network transport implementations
 * that enable message transmission over different protocols (HTTP, WebSocket, etc.)
 */

#pragma once

#include <kcenon/common/patterns/result.h>
#include <kcenon/messaging/core/message.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace kcenon::messaging::adapters {

/**
 * @enum transport_state
 * @brief Transport connection state
 */
enum class transport_state {
    disconnected,
    connecting,
    connected,
    disconnecting,
    error
};

/**
 * @struct transport_config
 * @brief Base configuration for transports
 */
struct transport_config {
    std::string host;
    uint16_t port = 0;
    std::chrono::milliseconds connect_timeout{10000};
    std::chrono::milliseconds request_timeout{30000};
    bool auto_reconnect = false;
    std::size_t max_retries = 3;
    std::chrono::milliseconds retry_delay{1000};
};

/**
 * @struct transport_statistics
 * @brief Transport performance statistics
 */
struct transport_statistics {
    uint64_t messages_sent = 0;
    uint64_t messages_received = 0;
    uint64_t bytes_sent = 0;
    uint64_t bytes_received = 0;
    uint64_t errors = 0;
    std::chrono::milliseconds avg_latency{0};
};

/**
 * @interface transport_interface
 * @brief Abstract interface for network transports
 */
class transport_interface {
public:
    virtual ~transport_interface() = default;

    /**
     * @brief Connect to remote endpoint
     * @return Result indicating success or error
     */
    virtual common::VoidResult connect() = 0;

    /**
     * @brief Disconnect from remote endpoint
     * @return Result indicating success or error
     */
    virtual common::VoidResult disconnect() = 0;

    /**
     * @brief Check if transport is connected
     * @return true if connected
     */
    virtual bool is_connected() const = 0;

    /**
     * @brief Get current transport state
     * @return Current state
     */
    virtual transport_state get_state() const = 0;

    /**
     * @brief Send a message
     * @param msg Message to send
     * @return Result indicating success or error
     */
    virtual common::VoidResult send(const message& msg) = 0;

    /**
     * @brief Send binary data
     * @param data Binary data to send
     * @return Result indicating success or error
     */
    virtual common::VoidResult send_binary(const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Set message received callback
     * @param handler Callback function for received messages
     */
    virtual void set_message_handler(
        std::function<void(const message&)> handler) = 0;

    /**
     * @brief Set binary data received callback
     * @param handler Callback function for received binary data
     */
    virtual void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)> handler) = 0;

    /**
     * @brief Set connection state change callback
     * @param handler Callback for state changes
     */
    virtual void set_state_handler(
        std::function<void(transport_state)> handler) = 0;

    /**
     * @brief Set error callback
     * @param handler Callback for errors
     */
    virtual void set_error_handler(
        std::function<void(const std::string&)> handler) = 0;

    /**
     * @brief Get transport statistics
     * @return Current statistics
     */
    virtual transport_statistics get_statistics() const = 0;

    /**
     * @brief Reset transport statistics
     */
    virtual void reset_statistics() = 0;
};

} // namespace kcenon::messaging::adapters
