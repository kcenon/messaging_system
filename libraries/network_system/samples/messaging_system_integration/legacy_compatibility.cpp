/**
 * @file legacy_compatibility.cpp
 * @brief Example of using network_system with legacy messaging_system code
 *
 * This example demonstrates how existing messaging_system code can
 * work with the new network_system without modification.
 *
 * @author kcenon
 * @date 2025-09-20

 */

#include <iostream>
#include <thread>
#include <chrono>
#include <string>

// Include compatibility header for legacy support
#include "network_system/compatibility.h"

// Legacy namespace usage (as in old messaging_system code)
using namespace network_module;
using namespace std::chrono_literals;

/**
 * @brief Legacy server implementation
 *
 * This code uses the old network_module namespace but works with
 * the new network_system implementation transparently.
 */
class LegacyServer {
public:
    LegacyServer(const std::string& id)
        : server_id_(id) {
        // Create server using legacy API
        server_ = create_server(server_id_);

        // Create bridge for integration
        bridge_ = create_bridge();

        std::cout << "[Legacy Server] Created server: " << server_id_ << std::endl;
    }

    void start(uint16_t port) {
        if (!server_) {
            std::cerr << "[Legacy Server] Server not initialized" << std::endl;
            return;
        }

        // Start listening
        server_->start_server(port);
        port_ = port;

        std::cout << "[Legacy Server] Started on port " << port << std::endl;
    }

    void stop() {
        if (server_) {
            server_->stop_server();
            std::cout << "[Legacy Server] Stopped" << std::endl;
        }
    }

    void show_metrics() {
        if (bridge_) {
            auto metrics = bridge_->get_metrics();
            std::cout << "\n[Legacy Server Metrics]" << std::endl;
            std::cout << "  Messages sent: " << metrics.messages_sent << std::endl;
            std::cout << "  Messages received: " << metrics.messages_received << std::endl;
            std::cout << "  Active connections: " << metrics.connections_active << std::endl;
        }
    }

private:
    std::string handle_message(const std::string& client_id, const std::string& message) {
        std::cout << "[Legacy Server] Received from " << client_id
                  << ": " << message << std::endl;

        // Echo back with prefix
        return "Echo from legacy server: " + message;
    }

    std::string server_id_;
    uint16_t port_ = 0;
    std::shared_ptr<messaging_server> server_;
    std::shared_ptr<messaging_bridge> bridge_;
};

/**
 * @brief Legacy client implementation
 */
class LegacyClient {
public:
    LegacyClient(const std::string& id)
        : client_id_(id) {
        // Create client using legacy API
        client_ = create_client(client_id_);

        std::cout << "[Legacy Client] Created client: " << client_id_ << std::endl;
    }

    void connect(const std::string& host, uint16_t port) {
        if (!client_) {
            std::cerr << "[Legacy Client] Client not initialized" << std::endl;
            return;
        }

        client_->start_client(host, port);
        std::cout << "[Legacy Client] Connecting to " << host << ":" << port << std::endl;

        // Give time to connect
        std::this_thread::sleep_for(100ms);
    }

    void send_message(const std::string& message) {
        if (client_) {
            std::vector<uint8_t> data(message.begin(), message.end());
            client_->send_packet(data);
            std::cout << "[Legacy Client] Sent: " << message << std::endl;
        }
    }

    void disconnect() {
        if (client_) {
            client_->stop_client();
            std::cout << "[Legacy Client] Disconnected" << std::endl;
        }
    }

private:
    std::string client_id_;
    std::shared_ptr<messaging_client> client_;
};

/**
 * @brief Demonstrate integration features
 */
void demonstrate_integration_features() {
    std::cout << "\n=== Integration Features ===" << std::endl;

    // Check feature support using compatibility API
    std::cout << "Container support: "
              << (network_system::compat::has_container_support() ? "YES" : "NO") << std::endl;
    std::cout << "Thread support: "
              << (network_system::compat::has_thread_support() ? "YES" : "NO") << std::endl;

    // Use thread pool if available
    auto& thread_mgr = thread_integration_manager::instance();
    auto pool = thread_mgr.get_thread_pool();

    if (pool) {
        std::cout << "\nThread pool available with "
                  << pool->worker_count() << " workers" << std::endl;

        // Submit async task
        auto future = pool->submit([]() {
            std::cout << "[Async Task] Running in thread pool" << std::endl;
            std::this_thread::sleep_for(100ms);
            std::cout << "[Async Task] Completed" << std::endl;
        });

        future.wait();
    }

    // Use container manager if available
    auto& container_mgr = container_manager::instance();
    std::string test_data = "Test serialization data";

    auto serialized = container_mgr.serialize(std::any(test_data));
    std::cout << "\nSerialized " << test_data.length()
              << " chars to " << serialized.size() << " bytes" << std::endl;

    auto deserialized = container_mgr.deserialize(serialized);
    if (deserialized.has_value()) {
        auto result = std::any_cast<std::string>(deserialized);
        std::cout << "Deserialized back: \"" << result << "\"" << std::endl;
    }
}

/**
 * @brief Main function demonstrating legacy compatibility
 */
int main(int argc, char* argv[]) {
    std::cout << "=== Legacy Messaging System Compatibility Demo ===" << std::endl;
    std::cout << "This demonstrates how legacy messaging_system code" << std::endl;
    std::cout << "works seamlessly with the new network_system." << std::endl;

    // Initialize network system
    network_system::compat::initialize();
    std::cout << "\n✓ Network system initialized for legacy support" << std::endl;

    try {
        // Create legacy server
        LegacyServer server("legacy_server_001");
        server.start(8080);

        // Give server time to start
        std::this_thread::sleep_for(500ms);

        // Create legacy client
        LegacyClient client("legacy_client_001");

        // Connect to server
        client.connect("127.0.0.1", 8080);

        // Send some messages
        for (int i = 1; i <= 3; ++i) {
            std::string msg = "Legacy message #" + std::to_string(i);
            client.send_message(msg);
            std::this_thread::sleep_for(100ms);
        }

        // Show server metrics
        server.show_metrics();

        // Disconnect client
        client.disconnect();

        // Demonstrate integration features
        demonstrate_integration_features();

        // Stop server
        server.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Shutdown network system
    network_system::compat::shutdown();
    std::cout << "\n✓ Network system shutdown complete" << std::endl;

    std::cout << "\n=== Legacy Compatibility Demo Complete ===" << std::endl;
    std::cout << "Legacy code continues to work without modification!" << std::endl;

    return 0;
}