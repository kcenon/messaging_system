// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file http_messaging.cpp
 * @brief Example: HTTP-based messaging with REST API
 *
 * Demonstrates:
 * - HTTP transport for request/reply messaging
 * - RESTful message API usage
 * - JSON serialization
 * - Error handling and retries
 */

#include <kcenon/messaging/adapters/http_transport.h>
#include <kcenon/messaging/adapters/resilient_transport.h>
#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/integration/messaging_container_builder.h>

#include <chrono>
#include <format>
#include <iostream>
#include <thread>

using namespace kcenon::messaging;
using namespace kcenon::messaging::adapters;

namespace {

/**
 * @brief Example: Order service client using HTTP
 */
class order_service_client {
public:
    explicit order_service_client(const std::string& base_url) {
        // Configure HTTP transport
        http_transport_config config;
        config.host = "localhost";  // Extract from base_url in production
        config.port = 8080;
        config.base_path = "/api/v1";
        config.content_type = http_content_type::json;
        config.request_timeout = std::chrono::seconds(30);

        // Set default headers
        config.default_headers["X-API-Version"] = "1.0";
        config.default_headers["X-Client-ID"] = "order-service-client";

        // Create HTTP transport
        auto http_transport_ptr = std::make_shared<http_transport>(config);

        // Wrap with resilient transport
        resilient_transport_config resilient_config;
        resilient_config.retry.max_retries = 3;
        resilient_config.retry.initial_delay = std::chrono::milliseconds(100);
        resilient_config.retry.backoff_multiplier = 2.0;
        resilient_config.circuit_breaker.failure_threshold = 5;

        transport_ = std::make_shared<resilient_transport>(
            http_transport_ptr, resilient_config);
    }

    /**
     * @brief Submit a new order
     */
    common::Result<message> submit_order(
        const std::string& symbol,
        int quantity,
        double price,
        const std::string& side) {

        // Build order message using container builder
        auto container_result = integration::messaging_container_builder()
            .source("order-client", "main")
            .target("order-service", "processor")
            .message_type("new_order")
            .add_value("symbol", symbol)
            .add_value("quantity", quantity)
            .add_value("price", price)
            .add_value("side", side)
            .add_value("timestamp",
                std::chrono::system_clock::now().time_since_epoch().count())
            .build();

        if (!container_result.has_value()) {
            return common::Result<message>::err(
                common::error_codes::INVALID_ARGUMENT,
                "Failed to build order container");
        }

        // Create message with the container
        auto msg_result = message_builder()
            .topic("orders.new")
            .source("order-client")
            .type(message_type::command)
            .priority(message_priority::high)
            .payload(container_result.value())
            .build();

        if (msg_result.is_err()) {
            return msg_result;
        }

        // Send via HTTP POST
        auto http = std::dynamic_pointer_cast<http_transport>(
            get_underlying_transport());
        if (http) {
            return http->post("/orders", msg_result.value());
        }

        // Fallback to generic send
        auto send_result = transport_->send(msg_result.value());
        if (send_result.is_err()) {
            return common::Result<message>::err(send_result.error());
        }
        return msg_result;
    }

    /**
     * @brief Get order status
     */
    common::Result<message> get_order_status(const std::string& order_id) {
        auto http = std::dynamic_pointer_cast<http_transport>(
            get_underlying_transport());
        if (http) {
            return http->get(std::format("/orders/{}", order_id));
        }
        return common::Result<message>::err(
            common::error_codes::INTERNAL_ERROR,
            "HTTP transport not available");
    }

    /**
     * @brief Cancel an order
     */
    common::VoidResult cancel_order(const std::string& order_id) {
        auto msg_result = message_builder()
            .topic("orders.cancel")
            .source("order-client")
            .type(message_type::command)
            .build();

        if (msg_result.is_err()) {
            return common::VoidResult::err(msg_result.error());
        }

        msg_result.value().payload().set_value("order_id", order_id);
        msg_result.value().payload().set_value("reason", "user_requested");

        return transport_->send(msg_result.value());
    }

    common::VoidResult connect() {
        return transport_->connect();
    }

    void disconnect() {
        transport_->disconnect();
    }

    void print_statistics() const {
        auto stats = transport_->get_statistics();
        std::cout << std::format(
            "\n--- HTTP Transport Statistics ---\n"
            "Messages sent: {}\n"
            "Messages received: {}\n"
            "Bytes sent: {}\n"
            "Bytes received: {}\n"
            "Errors: {}\n"
            "Avg latency: {}ms\n",
            stats.messages_sent,
            stats.messages_received,
            stats.bytes_sent,
            stats.bytes_received,
            stats.errors,
            stats.avg_latency.count()
        );
    }

private:
    std::shared_ptr<transport_interface> get_underlying_transport() {
        // In a real implementation, resilient_transport would expose this
        return transport_;
    }

    std::shared_ptr<resilient_transport> transport_;
};

/**
 * @brief Example: Notification service using HTTP
 */
class notification_service {
public:
    notification_service() {
        http_transport_config config;
        config.host = "notifications.example.com";
        config.port = 443;
        config.use_ssl = true;
        config.base_path = "/api/notify";
        config.content_type = http_content_type::json;

        transport_ = std::make_shared<http_transport>(config);

        // Set authorization header
        transport_->set_header("Authorization", "Bearer <token>");
    }

    common::VoidResult send_notification(
        const std::string& user_id,
        const std::string& title,
        const std::string& body) {

        auto msg_result = message_builder()
            .topic("notifications.push")
            .source("notification-service")
            .target(user_id)
            .type(message_type::notification)
            .build();

        if (msg_result.is_err()) {
            return common::VoidResult::err(msg_result.error());
        }

        auto& payload = msg_result.value().payload();
        payload.set_value("title", title);
        payload.set_value("body", body);
        payload.set_value("sent_at",
            std::chrono::system_clock::now().time_since_epoch().count());

        return transport_->send(msg_result.value());
    }

private:
    std::shared_ptr<http_transport> transport_;
};

} // anonymous namespace

int main() {
    std::cout << "=== HTTP Messaging Example ===\n\n";

    // Example 1: Order Service Client
    std::cout << "--- Order Service Client ---\n";
    {
        order_service_client client("http://localhost:8080");

        std::cout << "Order service client created.\n";
        std::cout << "In production:\n";
        std::cout << "  client.connect();\n";
        std::cout << "  auto result = client.submit_order(\"AAPL\", 100, 175.50, \"buy\");\n";
        std::cout << "  auto status = client.get_order_status(\"ORD-12345\");\n";
        std::cout << "  client.cancel_order(\"ORD-12345\");\n\n";
    }

    // Example 2: Direct HTTP Transport Usage
    std::cout << "--- Direct HTTP Transport Usage ---\n";
    {
        http_transport_config config;
        config.host = "api.example.com";
        config.port = 443;
        config.use_ssl = true;
        config.base_path = "/v1/messages";
        config.content_type = http_content_type::json;
        config.connect_timeout = std::chrono::seconds(10);
        config.request_timeout = std::chrono::seconds(30);

        auto transport = std::make_shared<http_transport>(config);

        // Set custom headers
        transport->set_header("X-API-Key", "your-api-key");
        transport->set_header("Accept", "application/json");

        std::cout << "HTTP transport configured.\n";
        std::cout << "Endpoints:\n";
        std::cout << "  POST /v1/messages/publish - Publish message\n";
        std::cout << "  GET  /v1/messages/subscribe - Long-poll for messages\n";
        std::cout << "  POST /v1/messages/request - Request/reply\n\n";
    }

    // Example 3: HTTP with Resilience
    std::cout << "--- HTTP with Resilience ---\n";
    {
        // Create base HTTP transport
        http_transport_config http_config;
        http_config.host = "api.example.com";
        http_config.port = 443;
        http_config.use_ssl = true;

        auto http = std::make_shared<http_transport>(http_config);

        // Configure resilience
        resilient_transport_config resilient_config;

        // Retry configuration
        resilient_config.retry.max_retries = 3;
        resilient_config.retry.initial_delay = std::chrono::milliseconds(100);
        resilient_config.retry.backoff_multiplier = 2.0;
        resilient_config.retry.max_delay = std::chrono::seconds(10);

        // Circuit breaker configuration
        resilient_config.circuit_breaker.failure_threshold = 5;
        resilient_config.circuit_breaker.reset_timeout = std::chrono::seconds(30);
        resilient_config.circuit_breaker.half_open_max_calls = 3;

        auto resilient = std::make_shared<resilient_transport>(
            http, resilient_config);

        // Set up monitoring
        resilient->set_circuit_state_handler([](circuit_state state) {
            switch (state) {
                case circuit_state::closed:
                    std::cout << "[Circuit] Closed - Normal operation\n";
                    break;
                case circuit_state::open:
                    std::cout << "[Circuit] Open - Failing fast\n";
                    break;
                case circuit_state::half_open:
                    std::cout << "[Circuit] Half-open - Testing recovery\n";
                    break;
            }
        });

        resilient->set_retry_handler([](std::size_t attempt,
                                        std::chrono::milliseconds delay) {
            std::cout << std::format("[Retry] Attempt {} after {}ms\n",
                attempt, delay.count());
        });

        std::cout << "Resilient HTTP transport configured.\n";
        std::cout << "Features:\n";
        std::cout << "  - Automatic retry with exponential backoff\n";
        std::cout << "  - Circuit breaker for fault isolation\n";
        std::cout << "  - Configurable timeouts\n";
    }

    // Example 4: Notification Service
    std::cout << "\n--- Notification Service ---\n";
    {
        // Note: This won't actually connect without valid credentials
        std::cout << "Notification service example.\n";
        std::cout << "In production:\n";
        std::cout << "  notification_service notifier;\n";
        std::cout << "  notifier.send_notification(\"user123\", ";
        std::cout << "\"New Order\", \"Your order has been placed.\");\n";
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
