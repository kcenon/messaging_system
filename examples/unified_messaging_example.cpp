/**
 * @file unified_messaging_example.cpp
 * @brief Example demonstrating the unified messaging system with all integrated modules
 * @author kcenon
 * @date 2025
 *
 * This example shows how to use the unified messaging system that integrates:
 * - Thread pool for async processing
 * - Logging system
 * - Monitoring system
 * - Container system for data management
 * - Database persistence
 * - Network communication
 */

#include <kcenon/messaging/unified_messaging_system.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

/**
 * @brief Create a test message with sample data
 */
message create_test_message(const std::string& topic, const std::string& content,
                           message_priority priority = message_priority::normal) {
    static size_t message_counter = 0;

    message msg;
    msg.id = "msg_" + std::to_string(++message_counter);
    msg.type = message_type::notification;
    msg.priority = priority;
    msg.sender = "example_app";
    msg.recipient = "all";
    msg.topic = topic;
    msg.timestamp = std::chrono::system_clock::now();

    // Set payload
    msg.payload = std::vector<uint8_t>(content.begin(), content.end());

    return msg;
}

/**
 * @brief Print message details
 */
void print_message(const message& msg) {
    std::cout << "📨 Message received:\n"
              << "  ID: " << msg.id << "\n"
              << "  Topic: " << msg.topic << "\n"
              << "  Priority: " << static_cast<int>(msg.priority) << "\n"
              << "  Sender: " << msg.sender << "\n"
              << "  Payload size: " << msg.payload.size() << " bytes\n"
              << "  Content: " << std::string(msg.payload.begin(), msg.payload.end()) << "\n"
              << std::endl;
}

/**
 * @brief Example 1: Basic messaging with logging and monitoring
 */
void example_basic_messaging() {
    std::cout << "\n=== Example 1: Basic Messaging ===\n" << std::endl;

    // Create messaging system with custom configuration
    messaging_config config;
    config.name = "BasicExample";
    config.worker_threads = 4;
    config.enable_console_logging = true;
    config.enable_monitoring = true;
    config.min_log_level = log_level::debug;

    unified_messaging_system messaging(config);

    // Initialize the system
    auto result = messaging.initialize();
    if (!kcenon::common::is_ok(result)) {
        std::cerr << "Failed to initialize messaging system" << std::endl;
        return;
    }

    // Subscribe to a topic
    messaging.on_message("sensors/temperature", [](const message& msg) {
        print_message(msg);
    });

    // Send some messages
    for (int i = 1; i <= 5; ++i) {
        auto msg = create_test_message(
            "sensors/temperature",
            "Temperature: " + std::to_string(20.0 + i * 0.5) + "°C"
        );

        auto future = messaging.send(msg);
        auto send_result = future.get();

        if (kcenon::common::is_ok(send_result)) {
            std::cout << "✅ Message " << i << " sent successfully" << std::endl;
        }

        std::this_thread::sleep_for(100ms);
    }

    // Get metrics
    auto metrics = messaging.get_metrics();
    std::cout << "\n📊 Metrics:\n"
              << "  Messages sent: " << metrics.messages_sent << "\n"
              << "  Messages received: " << metrics.messages_received << "\n"
              << "  Queue size: " << metrics.messages_in_queue << "\n"
              << std::endl;
}

/**
 * @brief Example 2: Priority-based message processing
 */
void example_priority_messaging() {
    std::cout << "\n=== Example 2: Priority-based Messaging ===\n" << std::endl;

    messaging_config config;
    config.name = "PriorityExample";
    config.use_priority_queue = true;
    config.enable_monitoring = true;

    unified_messaging_system messaging(config);
    messaging.initialize();

    // Subscribe to critical alerts
    messaging.on_message("alerts/*", [](const message& msg) {
        if (msg.priority == message_priority::critical) {
            std::cout << "🚨 CRITICAL ALERT: ";
        } else if (msg.priority == message_priority::high) {
            std::cout << "⚠️  HIGH PRIORITY: ";
        } else {
            std::cout << "ℹ️  INFO: ";
        }
        std::cout << std::string(msg.payload.begin(), msg.payload.end()) << std::endl;
    });

    // Send messages with different priorities
    std::vector<message> messages = {
        create_test_message("alerts/system", "Regular system check", message_priority::low),
        create_test_message("alerts/security", "Security scan complete", message_priority::normal),
        create_test_message("alerts/performance", "High CPU usage detected", message_priority::high),
        create_test_message("alerts/critical", "System failure imminent!", message_priority::critical),
        create_test_message("alerts/info", "Backup completed", message_priority::low)
    };

    // Send all messages quickly
    for (const auto& msg : messages) {
        messaging.send(msg);
    }

    // Wait for processing
    std::this_thread::sleep_for(500ms);
    messaging.wait_for_completion();
}

/**
 * @brief Example 3: Batch processing and parallel execution
 */
void example_batch_processing() {
    std::cout << "\n=== Example 3: Batch Processing ===\n" << std::endl;

    messaging_config config;
    config.name = "BatchExample";
    config.worker_threads = 8;  // More threads for parallel processing
    config.enable_batching = true;
    config.batch_size = 10;

    unified_messaging_system messaging(config);
    messaging.initialize();

    // Create a batch of messages
    std::vector<message> batch;
    for (int i = 1; i <= 20; ++i) {
        batch.push_back(create_test_message(
            "batch/data",
            "Data packet " + std::to_string(i)
        ));
    }

    std::cout << "Sending batch of " << batch.size() << " messages..." << std::endl;

    auto batch_future = messaging.send_batch(batch);
    auto batch_result = batch_future.get();

    if (kcenon::common::is_ok(batch_result)) {
        std::cout << "✅ Batch sent successfully!" << std::endl;
    }

    // Process messages in parallel
    auto futures = messaging.process_parallel(batch, [](const message& msg) {
        // Simulate some processing work
        std::this_thread::sleep_for(10ms);

        std::string content(msg.payload.begin(), msg.payload.end());
        return content.size();
    });

    std::cout << "Processing " << futures.size() << " messages in parallel..." << std::endl;

    size_t total_size = 0;
    for (auto& future : futures) {
        total_size += future.get();
    }

    std::cout << "Total processed data size: " << total_size << " bytes" << std::endl;
}

/**
 * @brief Example 4: Network communication (client-server)
 */
void example_network_communication() {
    std::cout << "\n=== Example 4: Network Communication ===\n" << std::endl;

    // Server setup
    std::thread server_thread([]() {
        messaging_config server_config;
        server_config.name = "MessageServer";
        server_config.enable_console_logging = false;  // Reduce noise

        unified_messaging_system server(server_config);
        server.initialize();

        // Handle incoming messages
        server.on_message("chat/*", [](const message& msg) {
            std::string content(msg.payload.begin(), msg.payload.end());
            std::cout << "📥 Server received: " << content
                     << " (from: " << msg.sender << ")" << std::endl;
        });

        // Start server
        auto result = server.start_server(8888, "127.0.0.1");
        if (kcenon::common::is_ok(result)) {
            std::cout << "🖥️  Server started on port 8888" << std::endl;
        } else {
            std::cout << "Failed to start server (network system might not be available)" << std::endl;
            return;
        }

        // Keep server running
        std::this_thread::sleep_for(5s);

        server.stop_server();
        std::cout << "🖥️  Server stopped" << std::endl;
    });

    // Give server time to start
    std::this_thread::sleep_for(1s);

    // Client setup
    messaging_config client_config;
    client_config.name = "MessageClient";
    client_config.enable_console_logging = false;

    unified_messaging_system client(client_config);
    client.initialize();

    // Connect to server
    connection_info conn_info;
    conn_info.address = "127.0.0.1";
    conn_info.port = 8888;

    auto connect_result = client.connect(conn_info);
    if (kcenon::common::is_ok(connect_result)) {
        std::cout << "📱 Client connected to server" << std::endl;

        // Send messages
        for (int i = 1; i <= 3; ++i) {
            auto msg = create_test_message(
                "chat/room1",
                "Hello from client, message " + std::to_string(i)
            );
            msg.sender = "client_" + std::to_string(i);

            client.send(msg);
            std::this_thread::sleep_for(500ms);
        }
    } else {
        std::cout << "📱 Client connection failed (network system might not be available)" << std::endl;
    }

    // Cleanup
    client.disconnect();
    server_thread.join();
}

/**
 * @brief Example 5: Message filtering and transformation
 */
void example_filtering_transformation() {
    std::cout << "\n=== Example 5: Filtering and Transformation ===\n" << std::endl;

    unified_messaging_system messaging;
    messaging.initialize();

    // Set up a message filter (only process important messages)
    messaging.set_message_filter([](const message& msg) {
        return msg.priority >= message_priority::normal;
    });

    // Set up a message transformer (add timestamp to payload)
    messaging.set_message_transformer([](const message& msg) {
        message transformed = msg;

        // Add timestamp to the beginning of payload
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::string timestamp = "[" + std::string(std::ctime(&time_t));
        timestamp.pop_back();  // Remove newline
        timestamp += "] ";

        std::vector<uint8_t> new_payload(timestamp.begin(), timestamp.end());
        new_payload.insert(new_payload.end(), msg.payload.begin(), msg.payload.end());
        transformed.payload = new_payload;

        return transformed;
    });

    // Subscribe to all messages
    messaging.on_message("*", [](const message& msg) {
        std::string content(msg.payload.begin(), msg.payload.end());
        std::cout << "Processed: " << content << std::endl;
    });

    // Send messages with different priorities
    std::vector<std::pair<std::string, message_priority>> test_messages = {
        {"Low priority - should be filtered", message_priority::low},
        {"Normal priority - should pass", message_priority::normal},
        {"High priority - should pass", message_priority::high},
        {"Critical - should pass", message_priority::critical}
    };

    for (const auto& [content, priority] : test_messages) {
        auto msg = create_test_message("test/filter", content, priority);
        messaging.send(msg);
    }

    // Wait for processing
    messaging.wait_for_completion();
}

/**
 * @brief Example 6: Health monitoring and metrics
 */
void example_health_monitoring() {
    std::cout << "\n=== Example 6: Health Monitoring ===\n" << std::endl;

    messaging_config config;
    config.name = "HealthMonitor";
    config.enable_monitoring = true;
    config.enable_metrics_collection = true;
    config.worker_threads = 2;  // Small pool for demo

    unified_messaging_system messaging(config);
    messaging.initialize();

    // Simulate some load
    std::cout << "Generating load for monitoring..." << std::endl;

    for (int i = 0; i < 100; ++i) {
        auto msg = create_test_message(
            "monitoring/test",
            "Test message " + std::to_string(i)
        );
        messaging.send(msg);

        if (i % 10 == 0) {
            // Periodically check health
            auto health = messaging.get_health();
            std::cout << "Health check #" << (i/10 + 1) << ":\n"
                     << "  Healthy: " << (health.is_healthy ? "Yes" : "No") << "\n"
                     << "  Score: " << health.overall_health_score << "/100\n";

            if (!health.issues.empty()) {
                std::cout << "  Issues:\n";
                for (const auto& issue : health.issues) {
                    std::cout << "    - " << issue << "\n";
                }
            }

            auto metrics = messaging.get_metrics();
            std::cout << "  Metrics:\n"
                     << "    Messages sent: " << metrics.messages_sent << "\n"
                     << "    Queue size: " << metrics.messages_in_queue << "\n"
                     << "    CPU usage: " << metrics.cpu_usage_percent << "%\n"
                     << "    Memory usage: " << metrics.memory_usage_mb << " MB\n"
                     << std::endl;
        }

        std::this_thread::sleep_for(10ms);
    }

    messaging.wait_for_completion();

    // Final metrics
    auto final_metrics = messaging.get_metrics();
    std::cout << "\n📊 Final Metrics:\n"
              << "  Total messages sent: " << final_metrics.messages_sent << "\n"
              << "  Total messages received: " << final_metrics.messages_received << "\n"
              << "  Failed messages: " << final_metrics.messages_failed << "\n"
              << "  Throughput: " << final_metrics.throughput_per_second << " msg/s\n"
              << std::endl;

    // Reset metrics for next run
    messaging.reset_metrics();
    std::cout << "Metrics reset for next session" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║     Unified Messaging System Examples              ║" << std::endl;
    std::cout << "║     Demonstrating integration of 7 system modules  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

    try {
        // Run examples based on command line argument
        if (argc > 1) {
            std::string example = argv[1];

            if (example == "1" || example == "basic") {
                example_basic_messaging();
            } else if (example == "2" || example == "priority") {
                example_priority_messaging();
            } else if (example == "3" || example == "batch") {
                example_batch_processing();
            } else if (example == "4" || example == "network") {
                example_network_communication();
            } else if (example == "5" || example == "filter") {
                example_filtering_transformation();
            } else if (example == "6" || example == "health") {
                example_health_monitoring();
            } else {
                std::cout << "Unknown example: " << example << std::endl;
                std::cout << "Available examples: 1-6, basic, priority, batch, network, filter, health" << std::endl;
            }
        } else {
            // Run all examples
            example_basic_messaging();
            example_priority_messaging();
            example_batch_processing();
            example_network_communication();
            example_filtering_transformation();
            example_health_monitoring();
        }

        std::cout << "\n✅ All examples completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}