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
        // Common setup for network tests
    }
    
    void TearDown() override {
        // Cleanup
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
                continue;
            }
        }
        return 0;
    }
};

// ============================================================================
// Messaging Server Tests
// ============================================================================

TEST_F(NetworkTest, ServerConstruction) {
    auto server = std::make_shared<messaging_server>("test_server");
    EXPECT_NE(server, nullptr);
}

TEST_F(NetworkTest, ServerStartStop) {
    auto server = std::make_shared<messaging_server>("test_server");
    
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    EXPECT_NO_THROW(server->start_server(port));
    
    // Give it time to start
    std::this_thread::sleep_for(100ms);
    
    // Stop server
    EXPECT_NO_THROW(server->stop_server());
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
        std::this_thread::sleep_for(50ms);
    }
}

TEST_F(NetworkTest, ServerPortAlreadyInUse) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    auto server1 = std::make_shared<messaging_server>("server1");
    auto server2 = std::make_shared<messaging_server>("server2");
    
    // Start first server
    server1->start_server(port);
    std::this_thread::sleep_for(100ms);
    
    // Second server on same port should handle gracefully
    EXPECT_NO_THROW(server2->start_server(port));
    std::this_thread::sleep_for(100ms);
    
    server1->stop_server();
    server2->stop_server();
}

// ============================================================================
// Messaging Client Tests
// ============================================================================

TEST_F(NetworkTest, ClientConstruction) {
    auto client = std::make_shared<messaging_client>("test_client");
    EXPECT_NE(client, nullptr);
}

TEST_F(NetworkTest, ClientConnectToNonExistentServer) {
    auto client = std::make_shared<messaging_client>("test_client");
    
    // Start client connecting to non-existent server
    // This should not throw, but connection will fail
    EXPECT_NO_THROW(client->start_client("127.0.0.1", 59999)); // Unlikely port
    
    // Give it a moment to try connecting
    std::this_thread::sleep_for(100ms);
    
    // Stop the client
    client->stop_client();
}

// ============================================================================
// Client-Server Connection Tests
// ============================================================================

TEST_F(NetworkTest, ClientServerBasicConnection) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create and connect client
    auto client = std::make_shared<messaging_client>("test_client");
    client->start_client("127.0.0.1", port);
    
    // Give it time to connect
    std::this_thread::sleep_for(100ms);
    
    client->stop_client();
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
    std::vector<std::shared_ptr<messaging_client>> clients;
    
    for (int i = 0; i < client_count; ++i) {
        auto client = std::make_shared<messaging_client>(
            "client_" + std::to_string(i));
        client->start_client("127.0.0.1", port);
        clients.push_back(client);
    }
    
    // Let them all connect
    std::this_thread::sleep_for(200ms);
    
    // Disconnect all clients
    for (auto& client : clients) {
        client->stop_client();
    }
    
    server->stop_server();
}

// ============================================================================
// Message Transfer Tests
// ============================================================================

TEST_F(NetworkTest, BasicMessageTransfer) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    // Give server time to start
    std::this_thread::sleep_for(100ms);
    
    // Create client
    auto client = std::make_shared<messaging_client>("test_client");
    client->start_client("127.0.0.1", port);
    
    // Give time to connect
    std::this_thread::sleep_for(100ms);
    
    // Create a message
    auto message = std::make_shared<value_container>();
    message->add(std::make_shared<string_value>("type", "test_message"));
    message->add(std::make_shared<string_value>("content", "Hello, Server!"));
    message->add(std::make_shared<int_value>("sequence", 1));
    
    // Serialize message to bytes
    auto serialized = message->serialize_array();
    
    // Send message
    EXPECT_NO_THROW(client->send_packet(serialized));
    
    // Give time for message to be sent
    std::this_thread::sleep_for(100ms);
    
    client->stop_client();
    server->stop_server();
}

TEST_F(NetworkTest, LargeMessageTransfer) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    std::this_thread::sleep_for(100ms);
    
    // Create client
    auto client = std::make_shared<messaging_client>("test_client");
    client->start_client("127.0.0.1", port);
    
    std::this_thread::sleep_for(100ms);
    
    // Create a large message
    auto message = std::make_shared<value_container>();
    message->add(std::make_shared<string_value>("type", "large_message"));
    
    // Add 1MB of data
    std::string large_data(1024 * 1024, 'X');
    message->add(std::make_shared<string_value>("data", large_data));
    
    // Serialize and send
    auto serialized = message->serialize_array();
    EXPECT_NO_THROW(client->send_packet(serialized));
    
    std::this_thread::sleep_for(200ms);
    
    client->stop_client();
    server->stop_server();
}

TEST_F(NetworkTest, MultipleMessageTransfer) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    // Start server
    auto server = std::make_shared<messaging_server>("test_server");
    server->start_server(port);
    
    std::this_thread::sleep_for(100ms);
    
    // Create client
    auto client = std::make_shared<messaging_client>("test_client");
    client->start_client("127.0.0.1", port);
    
    std::this_thread::sleep_for(100ms);
    
    // Send multiple messages
    const int message_count = 10;
    for (int i = 0; i < message_count; ++i) {
        auto message = std::make_shared<value_container>();
        message->add(std::make_shared<string_value>("type", "sequence_message"));
        message->add(std::make_shared<int_value>("sequence", i));
        message->add(std::make_shared<string_value>("data", "Message " + std::to_string(i)));
        
        auto serialized = message->serialize_array();
        client->send_packet(serialized);
        
        std::this_thread::sleep_for(10ms);
    }
    
    std::this_thread::sleep_for(100ms);
    
    client->stop_client();
    server->stop_server();
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(NetworkStressTest, RapidConnectionDisconnection) {
    // Helper to find available port
    auto findPort = []() -> unsigned short {
        for (unsigned short port = 5000; port < 65535; ++port) {
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
                continue;
            }
        }
        return 0;
    };
    
    auto port = findPort();
    ASSERT_NE(port, 0) << "No available port found";
    
    auto server = std::make_shared<messaging_server>("stress_server");
    server->start_server(port);
    
    std::this_thread::sleep_for(100ms);
    
    // Rapid connect/disconnect cycles
    const int cycles = 10;
    for (int i = 0; i < cycles; ++i) {
        auto client = std::make_shared<messaging_client>("rapid_client_" + std::to_string(i));
        client->start_client("127.0.0.1", port);
        std::this_thread::sleep_for(20ms);
        client->stop_client();
    }
    
    server->stop_server();
}

TEST(NetworkStressTest, ConcurrentClients) {
    // Helper to find available port
    auto findPort = []() -> unsigned short {
        for (unsigned short port = 6000; port < 65535; ++port) {
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
                continue;
            }
        }
        return 0;
    };
    
    auto port = findPort();
    ASSERT_NE(port, 0) << "No available port found";
    
    auto server = std::make_shared<messaging_server>("concurrent_server");
    server->start_server(port);
    
    std::this_thread::sleep_for(100ms);
    
    const int num_threads = 5;
    const int clients_per_thread = 2;
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([port, t, clients_per_thread]() {
            for (int c = 0; c < clients_per_thread; ++c) {
                auto client = std::make_shared<messaging_client>(
                    "thread_" + std::to_string(t) + "_client_" + std::to_string(c));
                
                client->start_client("127.0.0.1", port);
                std::this_thread::sleep_for(50ms);
                
                // Send a message
                auto message = std::make_shared<value_container>();
                message->add(std::make_shared<string_value>("thread", std::to_string(t)));
                message->add(std::make_shared<string_value>("client", std::to_string(c)));
                
                auto serialized = message->serialize_array();
                client->send_packet(serialized);
                
                std::this_thread::sleep_for(50ms);
                client->stop_client();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    server->stop_server();
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(NetworkTest, SendWithoutConnection) {
    auto client = std::make_shared<messaging_client>("disconnected_client");
    
    // Create a message
    auto message = std::make_shared<value_container>();
    message->add(std::make_shared<string_value>("test", "data"));
    
    auto serialized = message->serialize_array();
    
    // Send should not crash when not connected
    EXPECT_NO_THROW(client->send_packet(serialized));
}

TEST_F(NetworkTest, ServerStopWhileClientsConnected) {
    auto port = FindAvailablePort();
    ASSERT_NE(port, 0) << "No available port found";
    
    auto server = std::make_shared<messaging_server>("stopping_server");
    server->start_server(port);
    
    std::this_thread::sleep_for(100ms);
    
    // Connect clients
    std::vector<std::shared_ptr<messaging_client>> clients;
    for (int i = 0; i < 3; ++i) {
        auto client = std::make_shared<messaging_client>("client_" + std::to_string(i));
        client->start_client("127.0.0.1", port);
        clients.push_back(client);
    }
    
    std::this_thread::sleep_for(100ms);
    
    // Stop server while clients are connected
    server->stop_server();
    
    std::this_thread::sleep_for(100ms);
    
    // Stop clients
    for (auto& client : clients) {
        client->stop_client();
    }
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}