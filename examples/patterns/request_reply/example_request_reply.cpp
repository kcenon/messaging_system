/**
 * @file example_request_reply.cpp
 * @brief Example demonstrating Request-Reply messaging pattern
 *
 * This example shows how to use request-reply pattern for synchronous
 * communication over asynchronous messaging infrastructure.
 */

#include <kcenon/messaging/patterns/request_reply.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace kcenon;
using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;

// Simple calculator service
class CalculatorService {
public:
    static common::Result<message> handle_request(const message& request) {
        std::cout << "  [Server] Processing request: " << request.metadata().id << std::endl;

        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Create reply
        message reply("reply", message_type::reply);
        reply.metadata().correlation_id = request.metadata().id;
        reply.metadata().source = "calculator-service";

        // In a real application, you would process the request and generate results
        std::cout << "  [Server] Sending reply for: " << request.metadata().id << std::endl;

        return common::ok(std::move(reply));
    }
};

void run_server(std::shared_ptr<message_bus> bus) {
    std::cout << "\n[Server Thread] Starting calculator service..." << std::endl;

    request_server server(bus, "service.calculator");

    auto register_result = server.register_handler(CalculatorService::handle_request);
    if (!register_result.is_ok()) {
        std::cerr << "[Server Thread] Failed to register handler" << std::endl;
        return;
    }

    auto start_result = server.start();
    if (!start_result.is_ok()) {
        std::cerr << "[Server Thread] Failed to start server" << std::endl;
        return;
    }

    std::cout << "[Server Thread] Calculator service started" << std::endl;

    // Keep server running
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "[Server Thread] Stopping calculator service..." << std::endl;
    server.stop();
}

void run_client(std::shared_ptr<message_bus> bus) {
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "\n[Client Thread] Starting client..." << std::endl;

    request_client client(bus);

    // Make several requests
    for (int i = 1; i <= 3; ++i) {
        std::cout << "\n[Client Thread] Making request #" << i << "..." << std::endl;

        message request("service.calculator", message_type::query);
        request.metadata().source = "client-app";

        auto reply_result = client.request(
            "service.calculator",
            std::move(request),
            std::chrono::seconds{2}
        );

        if (reply_result.is_ok()) {
            auto reply = reply_result.unwrap();
            std::cout << "[Client Thread] Received reply for request #" << i << std::endl;
            std::cout << "  Correlation ID: " << reply.metadata().correlation_id << std::endl;
        } else {
            std::cerr << "[Client Thread] Request #" << i << " failed: "
                      << reply_result.get_error().message << std::endl;
        }
    }

    std::cout << "\n[Client Thread] Client completed" << std::endl;
}

int main() {
    std::cout << "=== Request-Reply Pattern Example ===" << std::endl;

    // 1. Create backend and message bus
    std::cout << "\n1. Setting up message bus..." << std::endl;
    auto backend = std::make_shared<standalone_backend>(4);
    message_bus_config config;
    config.worker_threads = 4;
    config.queue_capacity = 100;
    auto bus = std::make_shared<message_bus>(backend, config);

    auto start_result = bus->start();
    if (!start_result.is_ok()) {
        std::cerr << "Failed to start message bus" << std::endl;
        return 1;
    }
    std::cout << "Message bus started successfully" << std::endl;

    // 2. Run server and client in separate threads
    std::cout << "\n2. Starting server and client..." << std::endl;

    std::thread server_thread(run_server, bus);
    std::thread client_thread(run_client, bus);

    // 3. Wait for completion
    server_thread.join();
    client_thread.join();

    // 4. Display statistics
    std::cout << "\n3. Statistics:" << std::endl;
    auto stats = bus->get_statistics();
    std::cout << "  Total messages published: " << stats.messages_published << std::endl;
    std::cout << "  Total messages processed: " << stats.messages_processed << std::endl;

    // 5. Cleanup
    std::cout << "\n4. Cleaning up..." << std::endl;
    bus->stop();

    std::cout << "\n=== Example completed successfully ===" << std::endl;

    return 0;
}
