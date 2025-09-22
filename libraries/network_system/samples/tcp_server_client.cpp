/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Network System Project
 */

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <future>
#include "../network.h"

using namespace network_system;

class tcp_demo_server {
public:
    tcp_demo_server(const std::string& address, int port) 
        : address_(address), port_(port), running_(false) {}
    
    void start() {
        std::cout << "=== TCP Server Demo ===" << std::endl;
        std::cout << "Starting server on " << address_ << ":" << port_ << std::endl;
        
        server_thread_ = std::thread([this]() {
            auto server = std::make_shared<tcp_server>();
            
            if (server->start(address_, port_)) {
                std::cout << "✓ TCP Server started successfully" << std::endl;
                running_ = true;
                
                // Server message handler
                server->set_message_handler([this](const std::string& message, const std::string& client_id) {
                    std::cout << "[Server] Received from " << client_id << ": " << message << std::endl;
                    
                    // Echo response with timestamp
                    auto now = std::chrono::system_clock::now();
                    auto time_t = std::chrono::system_clock::to_time_t(now);
                    std::string response = "Echo: " + message + " (server time: " + std::to_string(time_t) + ")";
                    
                    return response;
                });
                
                // Binary data handler
                server->set_binary_handler([this](const std::vector<uint8_t>& data, const std::string& client_id) {
                    std::cout << "[Server] Received binary data from " << client_id 
                              << " (" << data.size() << " bytes)" << std::endl;
                    
                    // Echo binary data with additional byte
                    std::vector<uint8_t> response = data;
                    response.push_back(0xAA);  // Add marker byte
                    
                    return response;
                });
                
                // Client connection handlers
                server->set_connection_handler([](const std::string& client_id) {
                    std::cout << "[Server] Client connected: " << client_id << std::endl;
                });
                
                server->set_disconnection_handler([](const std::string& client_id) {
                    std::cout << "[Server] Client disconnected: " << client_id << std::endl;
                });
                
                // Keep server running
                while (running_) {
                    server->process_connections();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                server->stop();
                
            } else {
                std::cout << "✗ Failed to start TCP server" << std::endl;
            }
        });
    }
    
    void stop() {
        running_ = false;
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        std::cout << "✓ TCP Server stopped" << std::endl;
    }
    
    bool is_running() const { return running_; }

private:
    std::string address_;
    int port_;
    std::atomic<bool> running_;
    std::thread server_thread_;
};

class tcp_demo_client {
public:
    tcp_demo_client(int client_id, const std::string& server_address, int server_port)
        : client_id_(client_id), server_address_(server_address), server_port_(server_port) {}
    
    void run_demo() {
        std::cout << "\n=== TCP Client " << client_id_ << " Demo ===" << std::endl;
        
        auto client = std::make_shared<tcp_client>();
        
        std::cout << "[Client " << client_id_ << "] Connecting to server..." << std::endl;
        
        if (client->connect(server_address_, server_port_)) {
            std::cout << "✓ [Client " << client_id_ << "] Connected successfully" << std::endl;
            
            // Test text message communication
            test_text_communication(client);
            
            // Test binary data communication
            test_binary_communication(client);
            
            // Test concurrent operations
            test_concurrent_operations(client);
            
            // Performance test
            performance_test(client);
            
            client->disconnect();
            std::cout << "✓ [Client " << client_id_ << "] Disconnected" << std::endl;
            
        } else {
            std::cout << "✗ [Client " << client_id_ << "] Failed to connect" << std::endl;
        }
    }

private:
    int client_id_;
    std::string server_address_;
    int server_port_;
    
    void test_text_communication(std::shared_ptr<tcp_client> client) {
        std::cout << "[Client " << client_id_ << "] Testing text communication..." << std::endl;
        
        std::vector<std::string> test_messages = {
            "Hello Server!",
            "This is client " + std::to_string(client_id_),
            "Testing special characters: !@#$%^&*()",
            "Long message: " + std::string(100, 'A'),
            "Unicode test: 안녕하세요 🌟"
        };
        
        for (const auto& message : test_messages) {
            if (client->send_message(message)) {
                std::cout << "  [Client " << client_id_ << "] Sent: " << message.substr(0, 50) 
                          << (message.length() > 50 ? "..." : "") << std::endl;
                
                auto response = client->receive_message();
                if (response) {
                    std::cout << "  [Client " << client_id_ << "] Received: " << response->substr(0, 50) 
                              << (response->length() > 50 ? "..." : "") << std::endl;
                } else {
                    std::cout << "  [Client " << client_id_ << "] No response received" << std::endl;
                }
            } else {
                std::cout << "  [Client " << client_id_ << "] Failed to send message" << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void test_binary_communication(std::shared_ptr<tcp_client> client) {
        std::cout << "[Client " << client_id_ << "] Testing binary communication..." << std::endl;
        
        std::vector<std::vector<uint8_t>> test_data = {
            {0x01, 0x02, 0x03, 0x04},
            {0xFF, 0xFE, 0xFD, 0xFC, 0xFB},
            {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09},
            std::vector<uint8_t>(256, 0xAB),  // Large binary data
            {client_id_, 0x12, 0x34, 0x56}   // Client-specific data
        };
        
        for (const auto& data : test_data) {
            if (client->send_binary(data)) {
                std::cout << "  [Client " << client_id_ << "] Sent binary data (" 
                          << data.size() << " bytes)" << std::endl;
                
                auto response = client->receive_binary();
                if (response && !response->empty()) {
                    std::cout << "  [Client " << client_id_ << "] Received binary response (" 
                              << response->size() << " bytes)" << std::endl;
                    
                    // Verify echo + marker byte
                    if (response->size() == data.size() + 1 && response->back() == 0xAA) {
                        std::cout << "  [Client " << client_id_ << "] ✓ Binary echo verified" << std::endl;
                    }
                } else {
                    std::cout << "  [Client " << client_id_ << "] No binary response received" << std::endl;
                }
            } else {
                std::cout << "  [Client " << client_id_ << "] Failed to send binary data" << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void test_concurrent_operations(std::shared_ptr<tcp_client> client) {
        std::cout << "[Client " << client_id_ << "] Testing concurrent operations..." << std::endl;
        
        const int num_threads = 3;
        const int messages_per_thread = 5;
        std::vector<std::future<void>> futures;
        
        for (int t = 0; t < num_threads; ++t) {
            futures.push_back(std::async(std::launch::async, [this, client, t, messages_per_thread]() {
                for (int i = 0; i < messages_per_thread; ++i) {
                    std::string message = "Client " + std::to_string(client_id_) + 
                                        " Thread " + std::to_string(t) + 
                                        " Message " + std::to_string(i);
                    
                    if (client->send_message(message)) {
                        auto response = client->receive_message();
                        if (response) {
                            std::cout << "  [Client " << client_id_ << " T" << t << "] " 
                                      << "Sent/Received successfully" << std::endl;
                        }
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }));
        }
        
        // Wait for all threads to complete
        for (auto& future : futures) {
            future.wait();
        }
        
        std::cout << "  [Client " << client_id_ << "] Concurrent operations completed" << std::endl;
    }
    
    void performance_test(std::shared_ptr<tcp_client> client) {
        std::cout << "[Client " << client_id_ << "] Running performance test..." << std::endl;
        
        const int num_messages = 100;
        const std::string test_message = "Performance test message " + std::to_string(client_id_);
        
        auto start_time = std::chrono::high_resolution_clock::now();
        int successful_sends = 0;
        int successful_receives = 0;
        
        for (int i = 0; i < num_messages; ++i) {
            if (client->send_message(test_message + " #" + std::to_string(i))) {
                successful_sends++;
                
                auto response = client->receive_message();
                if (response) {
                    successful_receives++;
                }
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "  [Client " << client_id_ << "] Performance Results:" << std::endl;
        std::cout << "    Messages sent: " << successful_sends << "/" << num_messages << std::endl;
        std::cout << "    Responses received: " << successful_receives << "/" << num_messages << std::endl;
        std::cout << "    Total time: " << duration.count() << " ms" << std::endl;
        std::cout << "    Average time per round-trip: " 
                  << (successful_receives > 0 ? duration.count() / successful_receives : 0) 
                  << " ms" << std::endl;
        std::cout << "    Messages per second: " 
                  << (duration.count() > 0 ? (successful_receives * 1000) / duration.count() : 0) 
                  << std::endl;
    }
};

int main() {
    std::cout << "=== Network System - TCP Server/Client Demo ===" << std::endl;
    
    const std::string server_address = "127.0.0.1";
    const int server_port = 8080;
    const int num_clients = 3;
    
    // Start server
    tcp_demo_server server(server_address, server_port);
    server.start();
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    if (server.is_running()) {
        // Create and run multiple clients
        std::vector<std::future<void>> client_futures;
        
        for (int i = 1; i <= num_clients; ++i) {
            client_futures.push_back(
                std::async(std::launch::async, [i, server_address, server_port]() {
                    tcp_demo_client client(i, server_address, server_port);
                    client.run_demo();
                })
            );
            
            // Stagger client starts
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        // Wait for all clients to complete
        for (auto& future : client_futures) {
            future.wait();
        }
        
        std::cout << "\n=== All clients completed ===" << std::endl;
        
    } else {
        std::cout << "Server failed to start, skipping client tests" << std::endl;
    }
    
    // Stop server
    server.stop();
    
    std::cout << "\n=== TCP Server/Client Demo completed ===" << std::endl;
    return 0;
}