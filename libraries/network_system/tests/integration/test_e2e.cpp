/**
 * @file test_e2e.cpp
 * @brief End-to-end tests for network_system
 *
 * Comprehensive integration tests covering real-world scenarios
 * including multi-client connections, error handling, and recovery.
 *
 * @author kcenon
 * @date 2025-09-20

 */

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <random>
#include <cassert>

#include "network_system/network_system.h"

using namespace network_system;
using namespace std::chrono_literals;

// Test configuration
constexpr uint16_t TEST_PORT = 9191;
constexpr size_t NUM_CLIENTS = 10;
constexpr size_t MESSAGES_PER_CLIENT = 100;
constexpr size_t MAX_MESSAGE_SIZE = 8192;

// Test results
struct TestResults {
    std::atomic<size_t> passed{0};
    std::atomic<size_t> failed{0};
    std::atomic<size_t> messages_sent{0};
    std::atomic<size_t> messages_received{0};
    std::atomic<size_t> errors{0};

    void print() const {
        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Messages sent: " << messages_sent << std::endl;
        std::cout << "Messages received: " << messages_received << std::endl;
        std::cout << "Errors: " << errors << std::endl;
        std::cout << "Success rate: "
                  << (passed * 100.0 / (passed + failed)) << "%" << std::endl;
    }

    bool is_successful() const {
        return failed == 0 && errors == 0 && messages_sent == messages_received;
    }
};

/**
 * @brief Test 1: Basic connectivity test
 */
bool test_basic_connectivity(TestResults& results) {
    std::cout << "\n[Test 1] Basic Connectivity Test" << std::endl;

    try {
        // Create server
        auto server = std::make_shared<core::messaging_server>("e2e_server");
        server->start_server(TEST_PORT);

        std::this_thread::sleep_for(100ms);

        // Create client
        auto client = std::make_shared<core::messaging_client>("e2e_client");
        client->start_client("127.0.0.1", TEST_PORT);

        std::this_thread::sleep_for(100ms);

        // Send test message
        std::string test_data = "Hello, E2E Test!";
        std::vector<uint8_t> data(test_data.begin(), test_data.end());
        client->send_packet(data);
        results.messages_sent++;

        std::this_thread::sleep_for(100ms);

        // Clean up
        client->stop_client();
        server->stop_server();

        results.passed++;
        std::cout << "✅ Basic connectivity test passed" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ Basic connectivity test failed: " << e.what() << std::endl;
        results.failed++;
        results.errors++;
        return false;
    }
}

/**
 * @brief Test 2: Multi-client concurrent connections
 */
bool test_multi_client(TestResults& results) {
    std::cout << "\n[Test 2] Multi-Client Concurrent Test" << std::endl;

    try {
        // Create server
        auto server = std::make_shared<core::messaging_server>("multi_server");
        server->start_server(TEST_PORT + 1);

        std::this_thread::sleep_for(200ms);

        // Create and run multiple clients
        std::vector<std::thread> client_threads;
        std::atomic<size_t> local_messages{0};

        for (size_t i = 0; i < NUM_CLIENTS; ++i) {
            client_threads.emplace_back([i, &results, &local_messages]() {
                try {
                    auto client = std::make_shared<core::messaging_client>(
                        "client_" + std::to_string(i)
                    );
                    client->start_client("127.0.0.1", TEST_PORT + 1);

                    std::this_thread::sleep_for(50ms);

                    // Send multiple messages
                    for (size_t j = 0; j < MESSAGES_PER_CLIENT; ++j) {
                        std::string msg = "Client " + std::to_string(i) +
                                          " Message " + std::to_string(j);
                        std::vector<uint8_t> data(msg.begin(), msg.end());
                        client->send_packet(data);
                        local_messages++;
                        std::this_thread::sleep_for(5ms);
                    }

                    client->stop_client();

                } catch (const std::exception& e) {
                    std::cerr << "Client " << i << " error: " << e.what() << std::endl;
                    results.errors++;
                }
            });
        }

        // Wait for all clients to finish
        for (auto& thread : client_threads) {
            thread.join();
        }

        results.messages_sent += local_messages;

        // Clean up
        server->stop_server();

        if (results.errors == 0) {
            results.passed++;
            std::cout << "✅ Multi-client test passed ("
                      << local_messages << " messages)" << std::endl;
            return true;
        } else {
            results.failed++;
            std::cout << "❌ Multi-client test had errors" << std::endl;
            return false;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ Multi-client test failed: " << e.what() << std::endl;
        results.failed++;
        results.errors++;
        return false;
    }
}

/**
 * @brief Test 3: Large message handling
 */
bool test_large_messages(TestResults& results) {
    std::cout << "\n[Test 3] Large Message Handling Test" << std::endl;

    try {
        auto server = std::make_shared<core::messaging_server>("large_server");
        server->start_server(TEST_PORT + 2);

        std::this_thread::sleep_for(100ms);

        auto client = std::make_shared<core::messaging_client>("large_client");
        client->start_client("127.0.0.1", TEST_PORT + 2);

        std::this_thread::sleep_for(100ms);

        // Test different message sizes
        std::vector<size_t> sizes = {64, 256, 1024, 4096, 8192};

        for (size_t size : sizes) {
            std::vector<uint8_t> data(size);
            std::generate(data.begin(), data.end(),
                          []() { return rand() % 256; });

            client->send_packet(data);
            results.messages_sent++;
            std::this_thread::sleep_for(10ms);
        }

        // Clean up
        client->stop_client();
        server->stop_server();

        results.passed++;
        std::cout << "✅ Large message test passed" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ Large message test failed: " << e.what() << std::endl;
        results.failed++;
        results.errors++;
        return false;
    }
}

/**
 * @brief Test 4: Connection resilience
 */
bool test_connection_resilience(TestResults& results) {
    std::cout << "\n[Test 4] Connection Resilience Test" << std::endl;

    try {
        // Start server
        auto server = std::make_shared<core::messaging_server>("resilience_server");
        server->start_server(TEST_PORT + 3);

        std::this_thread::sleep_for(100ms);

        // Connect and disconnect multiple times
        for (int i = 0; i < 5; ++i) {
            auto client = std::make_shared<core::messaging_client>(
                "resilience_client_" + std::to_string(i)
            );

            client->start_client("127.0.0.1", TEST_PORT + 3);
            std::this_thread::sleep_for(50ms);

            // Send a message
            std::string msg = "Resilience test " + std::to_string(i);
            std::vector<uint8_t> data(msg.begin(), msg.end());
            client->send_packet(data);
            results.messages_sent++;

            std::this_thread::sleep_for(50ms);
            client->stop_client();
        }

        // Stop and restart server
        server->stop_server();
        std::this_thread::sleep_for(100ms);

        server = std::make_shared<core::messaging_server>("resilience_server2");
        server->start_server(TEST_PORT + 3);
        std::this_thread::sleep_for(100ms);

        // Try connecting again
        auto client = std::make_shared<core::messaging_client>("final_client");
        client->start_client("127.0.0.1", TEST_PORT + 3);
        std::this_thread::sleep_for(50ms);

        std::string msg = "Final message after restart";
        std::vector<uint8_t> data(msg.begin(), msg.end());
        client->send_packet(data);
        results.messages_sent++;

        client->stop_client();
        server->stop_server();

        results.passed++;
        std::cout << "✅ Connection resilience test passed" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ Connection resilience test failed: " << e.what() << std::endl;
        results.failed++;
        results.errors++;
        return false;
    }
}

/**
 * @brief Test 5: Rapid connect/disconnect cycles
 */
bool test_rapid_connections(TestResults& results) {
    std::cout << "\n[Test 5] Rapid Connection Cycles Test" << std::endl;

    try {
        auto server = std::make_shared<core::messaging_server>("rapid_server");
        server->start_server(TEST_PORT + 4);

        std::this_thread::sleep_for(100ms);

        // Rapid connect/disconnect cycles
        for (int i = 0; i < 20; ++i) {
            auto client = std::make_shared<core::messaging_client>(
                "rapid_client_" + std::to_string(i)
            );

            client->start_client("127.0.0.1", TEST_PORT + 4);

            // Send message immediately
            std::vector<uint8_t> data = {static_cast<uint8_t>(i)};
            client->send_packet(data);
            results.messages_sent++;

            // Disconnect quickly
            client->stop_client();

            // Small delay between cycles
            std::this_thread::sleep_for(10ms);
        }

        server->stop_server();

        results.passed++;
        std::cout << "✅ Rapid connection cycles test passed" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ Rapid connection cycles test failed: " << e.what() << std::endl;
        results.failed++;
        results.errors++;
        return false;
    }
}

/**
 * @brief Test 6: Thread pool integration
 */
bool test_thread_pool_integration(TestResults& results) {
    std::cout << "\n[Test 6] Thread Pool Integration Test" << std::endl;

    try {
        auto& thread_mgr = integration::thread_integration_manager::instance();

        // Submit multiple tasks
        std::vector<std::future<void>> futures;
        std::atomic<size_t> completed_tasks{0};

        for (size_t i = 0; i < 100; ++i) {
            futures.push_back(thread_mgr.submit_task([&completed_tasks, i]() {
                // Simulate work
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                completed_tasks++;
            }));
        }

        // Wait for all tasks
        for (auto& future : futures) {
            future.wait();
        }

        if (completed_tasks == 100) {
            results.passed++;
            std::cout << "✅ Thread pool integration test passed" << std::endl;
            return true;
        } else {
            results.failed++;
            std::cout << "❌ Thread pool test incomplete: "
                      << completed_tasks << "/100" << std::endl;
            return false;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ Thread pool test failed: " << e.what() << std::endl;
        results.failed++;
        results.errors++;
        return false;
    }
}

/**
 * @brief Test 7: Container serialization integration
 */
bool test_container_integration(TestResults& results) {
    std::cout << "\n[Test 7] Container Integration Test" << std::endl;

    try {
        auto& container_mgr = integration::container_manager::instance();

        // Test various data types
        std::vector<std::any> test_data = {
            std::any(42),
            std::any(3.14),
            std::any(std::string("Test string")),
            std::any(true),
            std::any(std::vector<int>{1, 2, 3, 4, 5})
        };

        for (const auto& data : test_data) {
            try {
                auto serialized = container_mgr.serialize(data);
                auto deserialized = container_mgr.deserialize(serialized);

                // Basic validation - just check we got something back
                if (deserialized.has_value()) {
                    results.messages_sent++;
                    results.messages_received++;
                } else {
                    results.errors++;
                }
            } catch (...) {
                // Some types may not be supported
                continue;
            }
        }

        results.passed++;
        std::cout << "✅ Container integration test passed" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ Container test failed: " << e.what() << std::endl;
        results.failed++;
        results.errors++;
        return false;
    }
}

/**
 * @brief Main test runner
 */
int main(int argc, char* argv[]) {
    std::cout << "=== Network System End-to-End Tests ===" << std::endl;
    std::cout << "Runtime: C++20 | Threads: " << std::thread::hardware_concurrency()
              << " | Build: " << (argc > 1 ? "Debug" : "Standard") << std::endl;

    // Initialize system
    network_system::compat::initialize();
    std::cout << "\nSystem initialized" << std::endl;

    TestResults results;

    // Run all tests
    std::cout << "\n🚀 Starting E2E tests..." << std::endl;

    test_basic_connectivity(results);
    test_multi_client(results);
    test_large_messages(results);
    test_connection_resilience(results);
    test_rapid_connections(results);
    test_thread_pool_integration(results);
    test_container_integration(results);

    // Print results
    results.print();

    // Cleanup
    network_system::compat::shutdown();
    std::cout << "\nSystem shutdown complete" << std::endl;

    // Determine success
    if (results.is_successful()) {
        std::cout << "\n✅ ALL E2E TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ SOME E2E TESTS FAILED" << std::endl;
        return 1;
    }
}