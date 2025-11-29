// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file websocket_realtime.cpp
 * @brief Example: Real-time messaging with WebSocket transport
 *
 * Demonstrates:
 * - WebSocket-based pub/sub messaging
 * - Topic subscriptions with wildcards
 * - Automatic reconnection
 * - Resilient transport wrapper
 */

#include <kcenon/messaging/adapters/websocket_transport.h>
#include <kcenon/messaging/adapters/resilient_transport.h>
#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/integration/messaging_container_builder.h>

#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

using namespace kcenon::messaging;
using namespace kcenon::messaging::adapters;

namespace {

/**
 * @brief Example: Market data subscriber using WebSocket
 */
class market_data_subscriber {
public:
    explicit market_data_subscriber(const std::string& server_url)
        : running_(false) {
        // Configure WebSocket transport
        websocket_transport_config config;
        config.host = "localhost";  // Use server_url in production
        config.port = 8080;
        config.path = "/market-data";
        config.auto_reconnect = true;
        config.reconnect_delay = std::chrono::milliseconds(1000);
        config.reconnect_backoff_multiplier = 2.0;
        config.max_reconnect_delay = std::chrono::milliseconds(30000);

        // Create WebSocket transport
        auto ws_transport = std::make_shared<websocket_transport>(config);

        // Wrap with resilient transport for additional reliability
        resilient_transport_config resilient_config;
        resilient_config.retry.max_retries = 3;
        resilient_config.circuit_breaker.failure_threshold = 5;
        resilient_config.circuit_breaker.reset_timeout = std::chrono::seconds(30);

        transport_ = std::make_shared<resilient_transport>(
            ws_transport, resilient_config);

        setup_handlers();
    }

    void start() {
        std::cout << "Connecting to market data server...\n";

        auto result = transport_->connect();
        if (result.is_err()) {
            std::cerr << std::format("Connection failed: {}\n",
                result.error().message);
            return;
        }

        // Subscribe to market data topics
        transport_->subscribe("market.*.quote");   // All quotes
        transport_->subscribe("market.*.trade");   // All trades
        transport_->subscribe("market.AAPL.#");    // Everything for AAPL

        running_ = true;
        std::cout << "Subscribed to market data topics\n";
    }

    void stop() {
        running_ = false;
        transport_->disconnect();
        std::cout << "Disconnected from market data server\n";
    }

    void print_statistics() const {
        auto stats = transport_->get_statistics();
        std::cout << std::format(
            "\n--- Statistics ---\n"
            "Messages received: {}\n"
            "Bytes received: {}\n"
            "Errors: {}\n"
            "Avg latency: {}ms\n",
            stats.messages_received,
            stats.bytes_received,
            stats.errors,
            stats.avg_latency.count()
        );
    }

private:
    void setup_handlers() {
        // Handle incoming messages
        transport_->set_message_handler([this](const message& msg) {
            handle_market_data(msg);
        });

        // Handle state changes
        transport_->set_state_handler([](transport_state state) {
            switch (state) {
                case transport_state::connected:
                    std::cout << "[STATE] Connected\n";
                    break;
                case transport_state::disconnected:
                    std::cout << "[STATE] Disconnected\n";
                    break;
                case transport_state::connecting:
                    std::cout << "[STATE] Connecting...\n";
                    break;
                case transport_state::error:
                    std::cout << "[STATE] Error\n";
                    break;
                default:
                    break;
            }
        });

        // Handle errors
        transport_->set_error_handler([](const std::string& error) {
            std::cerr << std::format("[ERROR] {}\n", error);
        });
    }

    void handle_market_data(const message& msg) {
        const auto& topic = msg.metadata().topic;
        const auto& payload = msg.payload();

        // Extract data from payload (using container_system)
        auto symbol = payload.get_value<std::string>("symbol");
        auto price = payload.get_value<double>("price");

        if (symbol && price) {
            std::cout << std::format("[{}] {} = ${:.2f}\n",
                topic, *symbol, *price);
        }

        quotes_received_++;
    }

    std::shared_ptr<resilient_transport> transport_;
    std::atomic<bool> running_;
    std::atomic<uint64_t> quotes_received_{0};
};

/**
 * @brief Example: Chat client using WebSocket
 */
class chat_client {
public:
    chat_client(const std::string& username)
        : username_(username) {
        websocket_transport_config config;
        config.host = "localhost";
        config.port = 8080;
        config.path = "/chat";
        config.auto_reconnect = true;

        transport_ = std::make_shared<websocket_transport>(config);

        transport_->set_message_handler([this](const message& msg) {
            handle_chat_message(msg);
        });
    }

    common::VoidResult connect() {
        auto result = transport_->connect();
        if (result.is_ok()) {
            // Subscribe to chat room
            transport_->subscribe("chat.room.*");
            transport_->subscribe(std::format("chat.private.{}", username_));
        }
        return result;
    }

    common::VoidResult send_message(const std::string& room,
                                     const std::string& text) {
        auto msg_result = message_builder()
            .topic(std::format("chat.room.{}", room))
            .source(username_)
            .type(message_type::event)
            .build();

        if (msg_result.is_err()) {
            return common::VoidResult::err(msg_result.error());
        }

        auto& msg = msg_result.value();
        msg.payload().set_value("text", text);
        msg.payload().set_value("timestamp",
            std::chrono::system_clock::now().time_since_epoch().count());

        return transport_->send(msg);
    }

    void disconnect() {
        transport_->disconnect();
    }

private:
    void handle_chat_message(const message& msg) {
        const auto& payload = msg.payload();
        auto sender = payload.get_value<std::string>("sender");
        auto text = payload.get_value<std::string>("text");

        if (sender && text) {
            std::cout << std::format("[{}]: {}\n", *sender, *text);
        }
    }

    std::string username_;
    std::shared_ptr<websocket_transport> transport_;
};

} // anonymous namespace

int main() {
    std::cout << "=== WebSocket Real-time Messaging Example ===\n\n";

    // Example 1: Market Data Subscriber
    std::cout << "--- Market Data Subscriber ---\n";
    {
        market_data_subscriber subscriber("ws://localhost:8080");

        // Note: In a real application, you would connect to a real server
        // For this example, we just demonstrate the API
        std::cout << "Market data subscriber created.\n";
        std::cout << "In production, call subscriber.start() to connect.\n\n";
    }

    // Example 2: Chat Client
    std::cout << "--- Chat Client ---\n";
    {
        chat_client client("user123");

        // Note: In a real application, you would connect to a real server
        std::cout << "Chat client created.\n";
        std::cout << "In production:\n";
        std::cout << "  client.connect();\n";
        std::cout << "  client.send_message(\"general\", \"Hello!\");\n\n";
    }

    // Example 3: Direct WebSocket usage
    std::cout << "--- Direct WebSocket Usage ---\n";
    {
        websocket_transport_config config;
        config.host = "echo.websocket.org";
        config.port = 443;
        config.use_ssl = true;
        config.path = "/";
        config.ping_interval = std::chrono::seconds(30);
        config.auto_reconnect = true;

        auto transport = std::make_shared<websocket_transport>(config);

        transport->set_message_handler([](const message& msg) {
            std::cout << std::format("Echo: {}\n", msg.metadata().topic);
        });

        transport->set_state_handler([](transport_state state) {
            std::cout << std::format("State changed: {}\n",
                static_cast<int>(state));
        });

        std::cout << "WebSocket transport configured.\n";
        std::cout << "Features:\n";
        std::cout << "  - Auto reconnection\n";
        std::cout << "  - Ping/pong keepalive\n";
        std::cout << "  - Topic subscriptions with wildcards\n";
        std::cout << "  - Binary and text message support\n";
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
