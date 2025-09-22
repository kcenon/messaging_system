/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Network System Project
 */

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include "../network.h"

using namespace network_system;

int main() {
    std::cout << "=== Network System - Basic Usage Example ===" << std::endl;
    
    // 1. Network manager creation and configuration
    std::cout << "\n1. Network Manager Setup:" << std::endl;
    
    auto network_manager = std::make_shared<network_manager>();
    std::cout << "Network manager created" << std::endl;
    
    // Set network configuration
    network_manager->set_timeout(5000);  // 5 seconds timeout
    network_manager->set_buffer_size(8192);  // 8KB buffer
    std::cout << "Network configuration set" << std::endl;
    
    // 2. TCP Server operations
    std::cout << "\n2. TCP Server Operations:" << std::endl;
    
    const std::string server_address = "127.0.0.1";
    const int server_port = 8080;
    
    std::cout << "Starting TCP server on " << server_address << ":" << server_port << std::endl;
    bool server_started = network_manager->start_tcp_server(server_address, server_port);
    
    if (server_started) {
        std::cout << "✓ TCP server started successfully" << std::endl;
        std::cout << "Server status: " << (network_manager->is_server_running() ? "Running" : "Stopped") << std::endl;
        
        // 3. TCP Client operations
        std::cout << "\n3. TCP Client Operations:" << std::endl;
        
        // Give server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "Connecting to TCP server..." << std::endl;
        bool client_connected = network_manager->connect_tcp_client(server_address, server_port);
        
        if (client_connected) {
            std::cout << "✓ TCP client connected successfully" << std::endl;
            std::cout << "Connection status: " << (network_manager->is_client_connected() ? "Connected" : "Disconnected") << std::endl;
            
            // 4. Data transmission
            std::cout << "\n4. Data Transmission:" << std::endl;
            
            // Send data from client to server
            std::string test_message = "Hello from TCP client!";
            std::cout << "Sending message: \"" << test_message << "\"" << std::endl;
            
            bool sent = network_manager->send_data(test_message);
            if (sent) {
                std::cout << "✓ Message sent successfully" << std::endl;
                
                // Receive response from server
                std::cout << "Waiting for response..." << std::endl;
                auto response = network_manager->receive_data();
                
                if (response) {
                    std::cout << "✓ Response received: \"" << *response << "\"" << std::endl;
                } else {
                    std::cout << "✗ No response received (this is expected in basic demo)" << std::endl;
                }
            } else {
                std::cout << "✗ Failed to send message" << std::endl;
            }
            
            // 5. Binary data transmission
            std::cout << "\n5. Binary Data Transmission:" << std::endl;
            
            std::vector<uint8_t> binary_data = {0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFE, 0xFD};
            std::cout << "Sending binary data (" << binary_data.size() << " bytes)" << std::endl;
            
            bool binary_sent = network_manager->send_binary_data(binary_data);
            if (binary_sent) {
                std::cout << "✓ Binary data sent successfully" << std::endl;
                
                // Receive binary response
                auto binary_response = network_manager->receive_binary_data();
                if (binary_response && !binary_response->empty()) {
                    std::cout << "✓ Binary response received (" << binary_response->size() << " bytes)" << std::endl;
                } else {
                    std::cout << "✗ No binary response received (this is expected in basic demo)" << std::endl;
                }
            } else {
                std::cout << "✗ Failed to send binary data" << std::endl;
            }
            
            // 6. Connection information
            std::cout << "\n6. Connection Information:" << std::endl;
            
            auto local_address = network_manager->get_local_address();
            auto remote_address = network_manager->get_remote_address();
            
            if (local_address) {
                std::cout << "Local address: " << *local_address << std::endl;
            }
            if (remote_address) {
                std::cout << "Remote address: " << *remote_address << std::endl;
            }
            
            auto stats = network_manager->get_connection_stats();
            if (stats) {
                std::cout << "Connection statistics:" << std::endl;
                std::cout << "  Bytes sent: " << stats->bytes_sent << std::endl;
                std::cout << "  Bytes received: " << stats->bytes_received << std::endl;
                std::cout << "  Messages sent: " << stats->messages_sent << std::endl;
                std::cout << "  Messages received: " << stats->messages_received << std::endl;
            }
            
            // Disconnect client
            network_manager->disconnect_client();
            std::cout << "✓ TCP client disconnected" << std::endl;
            
        } else {
            std::cout << "✗ Failed to connect TCP client" << std::endl;
            std::cout << "Note: This is expected when running as standalone demo" << std::endl;
        }
        
        // Stop server
        network_manager->stop_server();
        std::cout << "✓ TCP server stopped" << std::endl;
        
    } else {
        std::cout << "✗ Failed to start TCP server" << std::endl;
        std::cout << "Note: This may occur if port is already in use" << std::endl;
    }
    
    // 7. UDP operations
    std::cout << "\n7. UDP Operations:" << std::endl;
    
    const int udp_port = 8081;
    std::cout << "Starting UDP server on port " << udp_port << std::endl;
    
    bool udp_started = network_manager->start_udp_server(server_address, udp_port);
    if (udp_started) {
        std::cout << "✓ UDP server started successfully" << std::endl;
        
        // UDP client operations
        std::cout << "Connecting UDP client..." << std::endl;
        bool udp_connected = network_manager->connect_udp_client(server_address, udp_port);
        
        if (udp_connected) {
            std::cout << "✓ UDP client connected successfully" << std::endl;
            
            // Send UDP message
            std::string udp_message = "Hello from UDP client!";
            std::cout << "Sending UDP message: \"" << udp_message << "\"" << std::endl;
            
            bool udp_sent = network_manager->send_udp_data(udp_message);
            if (udp_sent) {
                std::cout << "✓ UDP message sent successfully" << std::endl;
            } else {
                std::cout << "✗ Failed to send UDP message" << std::endl;
            }
            
            network_manager->disconnect_udp_client();
            std::cout << "✓ UDP client disconnected" << std::endl;
            
        } else {
            std::cout << "✗ Failed to connect UDP client" << std::endl;
        }
        
        network_manager->stop_udp_server();
        std::cout << "✓ UDP server stopped" << std::endl;
        
    } else {
        std::cout << "✗ Failed to start UDP server" << std::endl;
    }
    
    // 8. HTTP operations (if supported)
    std::cout << "\n8. HTTP Operations:" << std::endl;
    
    std::cout << "Testing HTTP client capabilities..." << std::endl;
    
    // HTTP GET request
    std::string test_url = "http://httpbin.org/get";
    std::cout << "Sending HTTP GET request to: " << test_url << std::endl;
    
    auto http_response = network_manager->http_get(test_url);
    if (http_response) {
        std::cout << "✓ HTTP GET response received" << std::endl;
        std::cout << "Response size: " << http_response->size() << " bytes" << std::endl;
        
        // Show first 200 characters of response
        if (http_response->size() > 200) {
            std::cout << "Response preview: " << http_response->substr(0, 200) << "..." << std::endl;
        } else {
            std::cout << "Response: " << *http_response << std::endl;
        }
    } else {
        std::cout << "✗ HTTP GET request failed (this is expected without internet or HTTP support)" << std::endl;
    }
    
    // HTTP POST request
    std::string post_url = "http://httpbin.org/post";
    std::string post_data = "{\"message\": \"Hello from network system!\"}";
    std::cout << "Sending HTTP POST request with JSON data..." << std::endl;
    
    auto post_response = network_manager->http_post(post_url, post_data, "application/json");
    if (post_response) {
        std::cout << "✓ HTTP POST response received" << std::endl;
        std::cout << "Response size: " << post_response->size() << " bytes" << std::endl;
    } else {
        std::cout << "✗ HTTP POST request failed (this is expected without internet or HTTP support)" << std::endl;
    }
    
    // 9. Network utilities
    std::cout << "\n9. Network Utilities:" << std::endl;
    
    // Test network connectivity
    std::cout << "Testing network connectivity..." << std::endl;
    bool connectivity = network_manager->test_connectivity();
    std::cout << "Network connectivity: " << (connectivity ? "Available" : "Unavailable") << std::endl;
    
    // Get network interface information
    auto interfaces = network_manager->get_network_interfaces();
    if (interfaces && !interfaces->empty()) {
        std::cout << "Available network interfaces:" << std::endl;
        for (const auto& interface : *interfaces) {
            std::cout << "  " << interface << std::endl;
        }
    } else {
        std::cout << "No network interfaces detected or feature not implemented" << std::endl;
    }
    
    // DNS resolution
    std::string hostname = "localhost";
    std::cout << "Resolving hostname: " << hostname << std::endl;
    auto resolved_ip = network_manager->resolve_hostname(hostname);
    if (resolved_ip) {
        std::cout << "✓ Resolved " << hostname << " to " << *resolved_ip << std::endl;
    } else {
        std::cout << "✗ Failed to resolve hostname" << std::endl;
    }
    
    // 10. Cleanup and summary
    std::cout << "\n10. Cleanup and Summary:" << std::endl;
    
    // Final cleanup
    network_manager->cleanup();
    std::cout << "✓ Network manager cleanup completed" << std::endl;
    
    // Summary
    std::cout << "\nNetwork System Basic Usage Demo Summary:" << std::endl;
    std::cout << "- Demonstrated TCP server/client operations" << std::endl;
    std::cout << "- Showed UDP communication capabilities" << std::endl;
    std::cout << "- Tested HTTP client functionality" << std::endl;
    std::cout << "- Explored network utilities and diagnostics" << std::endl;
    std::cout << "- Performed proper cleanup" << std::endl;
    
    std::cout << "\nNote: Some operations may fail in a standalone demo environment." << std::endl;
    std::cout << "This is normal and demonstrates error handling capabilities." << std::endl;
    
    std::cout << "\n=== Basic Usage Example completed ===" << std::endl;
    return 0;
}