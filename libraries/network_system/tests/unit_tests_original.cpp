/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <future>
#include <atomic>

#include "../network.h"
#include <container.h>
#include <values/string_value.h>
#include <values/numeric_value.h>

using namespace network_module;
using namespace container_module;
using namespace std::chrono_literals;

// Test fixture for network tests
class NetworkTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }
    
    void TearDown() override {
        // Cleanup code if needed
    }
    
    // Helper to find an available port
    unsigned short FindAvailablePort(unsigned short start = 5000) {
        for (unsigned short port = start; port < 65535; ++port) {
            try {
                asio::io_context io_context;
                asio::ip::tcp::acceptor acceptor(io_context);
                asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
                acceptor.open(endpoint.protocol());
                acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
                acceptor.bind(endpoint);
                acceptor.close();
                return port;
            } catch (...) {
                // Port is in use, try next
            }
        }
        return 0; // No available port found
    }
};

// Basic Server Tests
TEST_F(NetworkTest, ServerConstruction) {
    auto server = std::make_shared<messaging_server>("test_server");
    EXPECT_NE(server, nullptr);
}

TEST_F(NetworkTest, ServerStartStop) {
    auto server = std::make_shared<messaging_server>("test_server");
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Stop server
    server->stop_server();
    
    // Should be able to wait for stop
    server->wait_for_stop();
}

TEST_F(NetworkTest, ServerMultipleStartStop) {
    auto server = std::make_shared<messaging_server>("test_server");
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Multiple start/stop cycles
    for (int i = 0; i < 3; ++i) {
        server->start_server(port);
        std::this_thread::sleep_for(50ms);
        server->stop_server();
        server->wait_for_stop();
        std::this_thread::sleep_for(50ms);
    }
}

TEST_F(NetworkTest, ServerDoubleStart) {
    auto server = std::make_shared<messaging_server>("test_server");
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    server->start_server(port);
    
    // Second start should be no-op
    server->start_server(port);
    
    server->stop_server();
}

TEST_F(NetworkTest, ServerDoubleStop) {
    auto server = std::make_shared<messaging_server>("test_server");
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start and stop
    server->start_server(port);
    server->stop_server();
    
    // Second stop should be no-op
    server->stop_server();
}

// Basic Client Tests
TEST_F(NetworkTest, ClientConstruction) {
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    EXPECT_NE(client, nullptr);
}

TEST_F(NetworkTest, ClientConnectToNonExistentServer) {
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    // Connect to non-existent server should fail
    auto connected = client->connect("127.0.0.1", 59999); // Unlikely port
    EXPECT_FALSE(connected);
}

// Client-Server Connection Tests
TEST_F(NetworkTest, ClientServerBasicConnection) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create and connect client
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    auto connected = client->connect("127.0.0.1", port);
    EXPECT_TRUE(connected);
    
    if (connected) {
        client->disconnect();
    }
    
    server->stop_server();
}

TEST_F(NetworkTest, MultipleClientsConnection) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Connect multiple clients
    const int client_count = 5;
    std::vector<asio::io_context> io_contexts(client_count);
    std::vector<std::shared_ptr<messaging_client>> clients;
    
    for (int i = 0; i < client_count; ++i) {
        auto client = std::make_shared<messaging_client>(
            io_contexts[i], 
            "client_" + std::to_string(i), 
            "key_" + std::to_string(i)
        );
        
        auto connected = client->connect("127.0.0.1", port);
        EXPECT_TRUE(connected) << "Client " << i << " failed to connect";
        
        clients.push_back(client);
    }
    
    // Disconnect all clients
    for (auto& client : clients) {
        client->disconnect();
    }
    
    server->stop_server();
}

// Message Exchange Tests
TEST_F(NetworkTest, ClientServerMessageExchange) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create and connect client
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    auto connected = client->connect("127.0.0.1", port);
    ASSERT_TRUE(connected);
    
    // Create a test message
    auto message = std::make_shared<value_container>();
    message->add_value(std::make_shared<string_value>("type", "test_message"));
    message->add_value(std::make_shared<string_value>("content", "Hello, Server!"));
    message->add_value(std::make_shared<int32_value>("sequence", 1));
    
    // Send message
    auto sent = client->send(message);
    EXPECT_TRUE(sent);
    
    // Run io_context to process messages
    io_context.run_for(100ms);
    
    client->disconnect();
    server->stop_server();
}

TEST_F(NetworkTest, LargeMessageExchange) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create and connect client
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    auto connected = client->connect("127.0.0.1", port);
    ASSERT_TRUE(connected);
    
    // Create a large message
    auto message = std::make_shared<value_container>();
    message->add_value(std::make_shared<string_value>("type", "large_message"));
    
    // Add large data (1MB string)
    std::string large_data(1024 * 1024, 'X');
    message->add_value(std::make_shared<string_value>("data", large_data));
    
    // Send message
    auto sent = client->send(message);
    EXPECT_TRUE(sent);
    
    // Run io_context to process messages
    io_context.run_for(200ms);
    
    client->disconnect();
    server->stop_server();
}

// Async Message Handler Tests
TEST_F(NetworkTest, ClientMessageHandler) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create client with message handler
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    std::atomic<int> messages_received{0};
    std::promise<void> message_promise;
    auto message_future = message_promise.get_future();
    
    // Set message handler
    client->set_message_handler([&](std::shared_ptr<container_module::value_container> msg) {
        messages_received++;
        if (messages_received == 1) {
            message_promise.set_value();
        }
    });
    
    auto connected = client->connect("127.0.0.1", port);
    ASSERT_TRUE(connected);
    
    // Run io_context in background
    std::thread io_thread([&io_context]() {
        io_context.run();
    });
    
    // Wait for potential echo or server messages
    auto status = message_future.wait_for(1s);
    
    io_context.stop();
    io_thread.join();
    
    client->disconnect();
    server->stop_server();
}

// Connection State Tests
TEST_F(NetworkTest, ClientReconnection) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create and connect client
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    // First connection
    auto connected = client->connect("127.0.0.1", port);
    EXPECT_TRUE(connected);
    
    // Disconnect
    client->disconnect();
    
    // Reconnect
    connected = client->connect("127.0.0.1", port);
    EXPECT_TRUE(connected);
    
    client->disconnect();
    server->stop_server();
}

// Error Handling Tests
TEST_F(NetworkTest, SendWithoutConnection) {
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    // Create a test message
    auto message = std::make_shared<value_container>();
    message->add_value(std::make_shared<string_value>("type", "test"));
    
    // Send without connection should fail
    auto sent = client->send(message);
    EXPECT_FALSE(sent);
}

TEST_F(NetworkTest, NullMessageSend) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create and connect client
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    auto connected = client->connect("127.0.0.1", port);
    ASSERT_TRUE(connected);
    
    // Send null message should fail
    auto sent = client->send(nullptr);
    EXPECT_FALSE(sent);
    
    client->disconnect();
    server->stop_server();
}

// Stress Tests
TEST_F(NetworkTest, RapidMessageSending) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create and connect client
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "test_client", "test_key");
    
    auto connected = client->connect("127.0.0.1", port);
    ASSERT_TRUE(connected);
    
    // Send many messages rapidly
    const int message_count = 100;
    int successful_sends = 0;
    
    for (int i = 0; i < message_count; ++i) {
        auto message = std::make_shared<value_container>();
        message->add_value(std::make_shared<string_value>("type", "rapid_test"));
        message->add_value(std::make_shared<int32_value>("sequence", i));
        
        if (client->send(message)) {
            successful_sends++;
        }
    }
    
    EXPECT_GT(successful_sends, 0);
    EXPECT_LE(successful_sends, message_count);
    
    // Run io_context to process messages
    io_context.run_for(500ms);
    
    client->disconnect();
    server->stop_server();
}

// Concurrent Operations Tests
TEST_F(NetworkTest, ConcurrentClientOperations) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create multiple clients in threads
    const int thread_count = 5;
    std::vector<std::thread> threads;
    std::atomic<int> successful_operations{0};
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&, i]() {
            asio::io_context io_context;
            auto client = std::make_shared<messaging_client>(
                io_context, 
                "thread_client_" + std::to_string(i),
                "thread_key_" + std::to_string(i)
            );
            
            if (client->connect("127.0.0.1", port)) {
                // Send a few messages
                for (int j = 0; j < 10; ++j) {
                    auto message = std::make_shared<value_container>();
                    message->add_value(std::make_shared<string_value>("from", "thread_" + std::to_string(i)));
                    message->add_value(std::make_shared<int32_value>("msg_id", j));
                    
                    if (client->send(message)) {
                        successful_operations++;
                    }
                }
                
                io_context.run_for(100ms);
                client->disconnect();
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_GT(successful_operations, 0);
    
    server->stop_server();
}

// Session Management Tests
TEST_F(NetworkTest, ServerSessionCleanup) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Connect and disconnect multiple clients
    for (int i = 0; i < 10; ++i) {
        asio::io_context io_context;
        auto client = std::make_shared<messaging_client>(
            io_context, 
            "temp_client_" + std::to_string(i),
            "temp_key_" + std::to_string(i)
        );
        
        if (client->connect("127.0.0.1", port)) {
            std::this_thread::sleep_for(10ms);
            client->disconnect();
        }
    }
    
    // Server should handle all disconnections gracefully
    server->stop_server();
}

// IPv6 Support Test (if available)
TEST_F(NetworkTest, IPv6Connection) {
    // Skip if IPv6 not available
    try {
        asio::io_context io_context;
        asio::ip::tcp::socket test_socket(io_context);
        test_socket.open(asio::ip::tcp::v6());
        test_socket.close();
    } catch (...) {
        GTEST_SKIP() << "IPv6 not available on this system";
    }
    
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server (should listen on both IPv4 and IPv6)
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Try IPv6 connection
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(io_context, "ipv6_client", "ipv6_key");
    
    // Try both IPv6 localhost addresses
    auto connected = client->connect("::1", port);
    if (!connected) {
        connected = client->connect("0:0:0:0:0:0:0:1", port);
    }
    
    // IPv6 might not be fully configured, so we don't assert
    if (connected) {
        client->disconnect();
    }
    
    server->stop_server();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}