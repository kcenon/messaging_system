/**
 * @file modern_usage.cpp
 * @brief Example of using network_system with modern API
 *
 * This example shows how to use the new network_system API directly
 * with all modern features including integration interfaces.
 *
 * @author kcenon
 * @date 2025-09-20

 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <future>

// Include modern network_system API
#include "network_system/network_system.h"

using namespace network_system;
using namespace std::chrono_literals;

/**
 * @brief Modern server using new API
 */
class ModernServer {
public:
    ModernServer(const std::string& id) : server_id_(id) {
        server_ = std::make_shared<core::messaging_server>(server_id_);
        bridge_ = std::make_shared<integration::messaging_bridge>();

        // Set up thread pool interface
        bridge_->set_thread_pool_interface(
            integration::thread_integration_manager::instance().get_thread_pool()
        );

        std::cout << "[Modern Server] Created with ID: " << server_id_ << std::endl;
    }

    void start(uint16_t port) {
        server_->start_server(port);
        port_ = port;

        std::cout << "[Modern Server] Started on port " << port << std::endl;
    }

    void stop() {
        server_->stop_server();
        std::cout << "[Modern Server] Stopped" << std::endl;
    }

    void enable_async_processing() {
        async_enabled_ = true;
        std::cout << "[Modern Server] Async processing enabled" << std::endl;
    }

    void show_statistics() {
        auto metrics = bridge_->get_metrics();
        auto thread_metrics = integration::thread_integration_manager::instance().get_metrics();

        std::cout << "\n=== Server Statistics ===" << std::endl;
        std::cout << "Network Metrics:" << std::endl;
        std::cout << "  Messages sent: " << metrics.messages_sent << std::endl;
        std::cout << "  Messages received: " << metrics.messages_received << std::endl;
        std::cout << "  Bytes sent: " << metrics.bytes_sent << std::endl;
        std::cout << "  Bytes received: " << metrics.bytes_received << std::endl;
        std::cout << "  Active connections: " << metrics.connections_active << std::endl;

        std::cout << "\nThread Pool Metrics:" << std::endl;
        std::cout << "  Worker threads: " << thread_metrics.worker_threads << std::endl;
        std::cout << "  Pending tasks: " << thread_metrics.pending_tasks << std::endl;
        std::cout << "  Completed tasks: " << thread_metrics.completed_tasks << std::endl;
    }

private:
    void process_message(const std::string& client_id, const std::string& message) {
        std::cout << "[Modern Server] Processing from " << client_id
                  << ": " << message << std::endl;

        if (async_enabled_) {
            // Process asynchronously using thread pool
            auto& thread_mgr = integration::thread_integration_manager::instance();
            auto future = thread_mgr.submit_task([message]() {
                // Simulate complex processing
                std::this_thread::sleep_for(50ms);
                std::cout << "[Async Processor] Completed processing: " << message << std::endl;
            });

            // Don't wait - let it process in background
            futures_.push_back(std::move(future));
        }

        // Use container system for demonstration
        auto& container_mgr = integration::container_manager::instance();
        std::string response = "Processed: " + message;

        // Serialize and deserialize to demonstrate container usage
        auto serialized = container_mgr.serialize(std::any(response));
        auto deserialized = container_mgr.deserialize(serialized);

        if (deserialized.has_value()) {
            std::cout << "[Modern Server] Container processed: "
                      << std::any_cast<std::string>(deserialized) << std::endl;
        }
    }

    std::string server_id_;
    uint16_t port_ = 0;
    bool async_enabled_ = false;
    std::shared_ptr<core::messaging_server> server_;
    std::shared_ptr<integration::messaging_bridge> bridge_;
    std::vector<std::future<void>> futures_;
};

/**
 * @brief Modern client using new API
 */
class ModernClient {
public:
    ModernClient(const std::string& id) : client_id_(id) {
        client_ = std::make_shared<core::messaging_client>(client_id_);
        std::cout << "[Modern Client] Created with ID: " << client_id_ << std::endl;
    }

    void connect(const std::string& host, uint16_t port) {
        client_->start_client(host, port);
        std::cout << "[Modern Client] Connecting to " << host << ":" << port << std::endl;
        std::this_thread::sleep_for(200ms);  // Give time to connect
    }

    void send_batch(const std::vector<std::string>& messages) {
        std::cout << "[Modern Client] Sending batch of " << messages.size()
                  << " messages" << std::endl;

        for (const auto& msg : messages) {
            std::vector<uint8_t> data(msg.begin(), msg.end());
            client_->send_packet(data);
            std::this_thread::sleep_for(50ms);
        }
    }

    void send_async(const std::string& message) {
        auto& thread_mgr = integration::thread_integration_manager::instance();
        thread_mgr.submit_task([this, message]() {
            std::vector<uint8_t> data(message.begin(), message.end());
            client_->send_packet(data);
            std::cout << "[Modern Client] Async sent: " << message << std::endl;
        });
    }

    void disconnect() {
        client_->stop_client();
        std::cout << "[Modern Client] Disconnected" << std::endl;
    }

private:
    std::string client_id_;
    std::shared_ptr<core::messaging_client> client_;
};

/**
 * @brief Demonstrate advanced features
 */
void demonstrate_advanced_features() {
    std::cout << "\n=== Advanced Features Demo ===" << std::endl;

    // Custom container with serialization
    auto custom_container = std::make_shared<integration::basic_container>();

    // Set custom serializer for complex types
    custom_container->set_serializer([](const std::any& data) {
        std::vector<uint8_t> result;
        if (data.type() == typeid(std::vector<int>)) {
            auto vec = std::any_cast<std::vector<int>>(data);
            result.reserve(vec.size() * sizeof(int));
            for (int val : vec) {
                auto bytes = reinterpret_cast<const uint8_t*>(&val);
                result.insert(result.end(), bytes, bytes + sizeof(int));
            }
        }
        return result;
    });

    // Register custom container
    integration::container_manager::instance().register_container(
        "custom_vector_serializer", custom_container
    );

    // Test custom serialization
    std::vector<int> test_data = {1, 2, 3, 4, 5};
    auto serialized = custom_container->serialize(std::any(test_data));
    std::cout << "Custom serialized " << test_data.size()
              << " integers to " << serialized.size() << " bytes" << std::endl;

    // Thread pool advanced usage
    auto& thread_mgr = integration::thread_integration_manager::instance();
    std::vector<std::future<void>> tasks;

    // Submit multiple delayed tasks
    for (int i = 1; i <= 3; ++i) {
        auto future = thread_mgr.submit_delayed_task(
            [i]() {
                std::cout << "[Delayed Task " << i << "] Executed after delay" << std::endl;
            },
            std::chrono::milliseconds(i * 100)
        );
        tasks.push_back(std::move(future));
    }

    // Wait for all tasks
    for (auto& future : tasks) {
        future.wait();
    }

    std::cout << "All advanced features demonstrated successfully" << std::endl;
}

/**
 * @brief Main function demonstrating modern usage
 */
int main(int argc, char* argv[]) {
    std::cout << "=== Modern Network System Usage Demo ===" << std::endl;
    std::cout << "Demonstrating the new API with all integration features" << std::endl;

    // Initialize using modern API
    network_system::compat::initialize();
    std::cout << "\n✓ Network system initialized" << std::endl;

    try {
        // Create modern server
        ModernServer server("modern_server_001");
        server.enable_async_processing();
        server.start(9090);

        // Allow server to start
        std::this_thread::sleep_for(500ms);

        // Create modern client
        ModernClient client("modern_client_001");

        client.connect("127.0.0.1", 9090);

        // Send batch messages
        std::vector<std::string> batch = {
            "Modern message 1",
            "Modern message 2",
            "Modern message 3"
        };
        client.send_batch(batch);

        // Send async messages
        for (int i = 1; i <= 3; ++i) {
            client.send_async("Async message " + std::to_string(i));
        }

        // Wait for async operations
        std::this_thread::sleep_for(500ms);

        // Show server statistics
        server.show_statistics();

        // Disconnect
        client.disconnect();

        // Demonstrate advanced features
        demonstrate_advanced_features();

        // Stop server
        server.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Shutdown
    network_system::compat::shutdown();
    std::cout << "\n✓ Network system shutdown complete" << std::endl;

    std::cout << "\n=== Modern Usage Demo Complete ===" << std::endl;
    std::cout << "All modern features working perfectly!" << std::endl;

    return 0;
}