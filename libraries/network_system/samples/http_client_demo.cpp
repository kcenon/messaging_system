/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Network System Project
 */

#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <future>
#include <iomanip>
#include "../network.h"

using namespace network_system;

class http_demo {
public:
    http_demo() {
        http_client_ = std::make_shared<http_client>();
        setup_test_urls();
    }
    
    void run_demo() {
        std::cout << "=== Network System - HTTP Client Demo ===" << std::endl;
        
        test_basic_get_requests();
        test_post_requests();
        test_headers_and_authentication();
        test_file_operations();
        test_error_handling();
        test_concurrent_requests();
        test_performance_benchmark();
        
        std::cout << "\n=== HTTP Client Demo completed ===" << std::endl;
    }

private:
    std::shared_ptr<http_client> http_client_;
    std::map<std::string, std::string> test_urls_;
    
    void setup_test_urls() {
        test_urls_ = {
            {"httpbin_base", "https://httpbin.org"},
            {"httpbin_get", "https://httpbin.org/get"},
            {"httpbin_post", "https://httpbin.org/post"},
            {"httpbin_put", "https://httpbin.org/put"},
            {"httpbin_delete", "https://httpbin.org/delete"},
            {"httpbin_headers", "https://httpbin.org/headers"},
            {"httpbin_auth", "https://httpbin.org/basic-auth/user/passwd"},
            {"httpbin_status", "https://httpbin.org/status/"},
            {"httpbin_delay", "https://httpbin.org/delay/1"},
            {"json_placeholder", "https://jsonplaceholder.typicode.com/posts/1"},
            {"localhost", "http://localhost:8080/test"}
        };
    }
    
    void test_basic_get_requests() {
        std::cout << "\n1. Basic GET Requests:" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        // Simple GET request
        std::cout << "Testing simple GET request..." << std::endl;
        auto response = http_client_->get(test_urls_["httpbin_get"]);
        
        if (response) {
            std::cout << "✓ GET request successful" << std::endl;
            std::cout << "Response size: " << response->body.size() << " bytes" << std::endl;
            std::cout << "Status code: " << response->status_code << std::endl;
            std::cout << "Content-Type: " << get_header_value(*response, "content-type") << std::endl;
            
            // Show first 200 characters
            if (response->body.size() > 200) {
                std::cout << "Response preview: " << response->body.substr(0, 200) << "..." << std::endl;
            }
        } else {
            std::cout << "✗ GET request failed (network may be unavailable)" << std::endl;
        }
        
        // GET with query parameters
        std::cout << "\nTesting GET with query parameters..." << std::endl;
        std::map<std::string, std::string> query_params = {
            {"param1", "value1"},
            {"param2", "value with spaces"},
            {"param3", "special&chars=test"}
        };
        
        auto param_response = http_client_->get(test_urls_["httpbin_get"], query_params);
        if (param_response) {
            std::cout << "✓ GET with parameters successful" << std::endl;
            std::cout << "Status code: " << param_response->status_code << std::endl;
        } else {
            std::cout << "✗ GET with parameters failed" << std::endl;
        }
        
        // JSON API test
        std::cout << "\nTesting JSON API..." << std::endl;
        auto json_response = http_client_->get(test_urls_["json_placeholder"]);
        if (json_response) {
            std::cout << "✓ JSON API request successful" << std::endl;
            std::cout << "Response: " << json_response->body.substr(0, 100) << "..." << std::endl;
        } else {
            std::cout << "✗ JSON API request failed" << std::endl;
        }
    }
    
    void test_post_requests() {
        std::cout << "\n2. POST Requests:" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        // JSON POST
        std::cout << "Testing JSON POST request..." << std::endl;
        std::string json_data = R"({
            "title": "Test Post",
            "body": "This is a test post from the network system",
            "userId": 1
        })";
        
        std::map<std::string, std::string> json_headers = {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"}
        };
        
        auto post_response = http_client_->post(test_urls_["httpbin_post"], json_data, json_headers);
        if (post_response) {
            std::cout << "✓ JSON POST successful" << std::endl;
            std::cout << "Status code: " << post_response->status_code << std::endl;
            std::cout << "Response size: " << post_response->body.size() << " bytes" << std::endl;
        } else {
            std::cout << "✗ JSON POST failed" << std::endl;
        }
        
        // Form data POST
        std::cout << "\nTesting form data POST..." << std::endl;
        std::map<std::string, std::string> form_data = {
            {"username", "testuser"},
            {"password", "testpass"},
            {"email", "test@example.com"},
            {"age", "25"}
        };
        
        auto form_response = http_client_->post_form(test_urls_["httpbin_post"], form_data);
        if (form_response) {
            std::cout << "✓ Form POST successful" << std::endl;
            std::cout << "Status code: " << form_response->status_code << std::endl;
        } else {
            std::cout << "✗ Form POST failed" << std::endl;
        }
        
        // Binary data POST
        std::cout << "\nTesting binary data POST..." << std::endl;
        std::vector<uint8_t> binary_data = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}; // PNG header
        
        std::map<std::string, std::string> binary_headers = {
            {"Content-Type", "application/octet-stream"}
        };
        
        auto binary_response = http_client_->post_binary(test_urls_["httpbin_post"], binary_data, binary_headers);
        if (binary_response) {
            std::cout << "✓ Binary POST successful" << std::endl;
            std::cout << "Status code: " << binary_response->status_code << std::endl;
        } else {
            std::cout << "✗ Binary POST failed" << std::endl;
        }
    }
    
    void test_headers_and_authentication() {
        std::cout << "\n3. Headers and Authentication:" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        // Custom headers
        std::cout << "Testing custom headers..." << std::endl;
        std::map<std::string, std::string> custom_headers = {
            {"User-Agent", "NetworkSystem/1.0 HTTP Client Demo"},
            {"X-Custom-Header", "CustomValue"},
            {"Accept", "application/json"},
            {"Accept-Language", "en-US,en;q=0.9"},
            {"Accept-Encoding", "gzip, deflate"}
        };
        
        auto header_response = http_client_->get(test_urls_["httpbin_headers"], {}, custom_headers);
        if (header_response) {
            std::cout << "✓ Custom headers request successful" << std::endl;
            std::cout << "Status code: " << header_response->status_code << std::endl;
        } else {
            std::cout << "✗ Custom headers request failed" << std::endl;
        }
        
        // Basic authentication
        std::cout << "\nTesting basic authentication..." << std::endl;
        auto auth_response = http_client_->get_with_auth(test_urls_["httpbin_auth"], "user", "passwd");
        if (auth_response) {
            std::cout << "✓ Basic authentication successful" << std::endl;
            std::cout << "Status code: " << auth_response->status_code << std::endl;
        } else {
            std::cout << "✗ Basic authentication failed" << std::endl;
        }
        
        // Test authentication failure
        std::cout << "\nTesting authentication failure..." << std::endl;
        auto auth_fail_response = http_client_->get_with_auth(test_urls_["httpbin_auth"], "wrong", "credentials");
        if (auth_fail_response && auth_fail_response->status_code == 401) {
            std::cout << "✓ Authentication failure handled correctly (401)" << std::endl;
        } else {
            std::cout << "✗ Authentication failure not handled as expected" << std::endl;
        }
    }
    
    void test_file_operations() {
        std::cout << "\n4. File Operations:" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        // Download file
        std::cout << "Testing file download..." << std::endl;
        std::string download_url = "https://httpbin.org/bytes/1024";  // Download 1KB of random data
        
        auto download_response = http_client_->get(download_url);
        if (download_response && download_response->status_code == 200) {
            std::cout << "✓ File download successful" << std::endl;
            std::cout << "Downloaded " << download_response->body.size() << " bytes" << std::endl;
            
            // Save to file (demo)
            std::string filename = "downloaded_data.bin";
            bool saved = save_response_to_file(*download_response, filename);
            if (saved) {
                std::cout << "✓ File saved as " << filename << std::endl;
            }
        } else {
            std::cout << "✗ File download failed" << std::endl;
        }
        
        // File upload simulation
        std::cout << "\nTesting file upload simulation..." << std::endl;
        std::string file_content = "This is test file content for upload simulation.\n";
        file_content += "Line 2: Binary data and special characters: æøå 中文 🌟\n";
        
        std::map<std::string, std::string> upload_headers = {
            {"Content-Type", "text/plain"},
            {"Content-Disposition", "attachment; filename=\"test.txt\""}
        };
        
        auto upload_response = http_client_->post(test_urls_["httpbin_post"], file_content, upload_headers);
        if (upload_response) {
            std::cout << "✓ File upload simulation successful" << std::endl;
            std::cout << "Status code: " << upload_response->status_code << std::endl;
        } else {
            std::cout << "✗ File upload simulation failed" << std::endl;
        }
    }
    
    void test_error_handling() {
        std::cout << "\n5. Error Handling:" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        // Test different HTTP status codes
        std::vector<int> status_codes = {200, 400, 401, 403, 404, 500, 503};
        
        for (int status : status_codes) {
            std::cout << "Testing HTTP " << status << "..." << std::endl;
            std::string status_url = test_urls_["httpbin_status"] + std::to_string(status);
            
            auto status_response = http_client_->get(status_url);
            if (status_response) {
                std::cout << "  ✓ Received status " << status_response->status_code;
                if (status_response->status_code == status) {
                    std::cout << " (correct)" << std::endl;
                } else {
                    std::cout << " (expected " << status << ")" << std::endl;
                }
            } else {
                std::cout << "  ✗ Request failed" << std::endl;
            }
        }
        
        // Test timeout
        std::cout << "\nTesting timeout handling..." << std::endl;
        http_client_->set_timeout(2000);  // 2 second timeout
        std::string delay_url = test_urls_["httpbin_delay"];  // 1 second delay (should work)
        
        auto timeout_response = http_client_->get(delay_url);
        if (timeout_response) {
            std::cout << "✓ Request with delay completed within timeout" << std::endl;
        } else {
            std::cout << "✗ Request timed out or failed" << std::endl;
        }
        
        // Test invalid URL
        std::cout << "\nTesting invalid URL handling..." << std::endl;
        auto invalid_response = http_client_->get("http://invalid-domain-that-should-not-exist.com");
        if (!invalid_response) {
            std::cout << "✓ Invalid URL handled correctly" << std::endl;
        } else {
            std::cout << "✗ Invalid URL should have failed" << std::endl;
        }
        
        // Test localhost (may not be available)
        std::cout << "\nTesting localhost connection..." << std::endl;
        auto local_response = http_client_->get(test_urls_["localhost"]);
        if (local_response) {
            std::cout << "✓ Localhost connection successful" << std::endl;
        } else {
            std::cout << "✗ Localhost connection failed (expected if no local server)" << std::endl;
        }
    }
    
    void test_concurrent_requests() {
        std::cout << "\n6. Concurrent Requests:" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        const int num_requests = 5;
        std::vector<std::future<bool>> futures;
        
        std::cout << "Starting " << num_requests << " concurrent requests..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_requests; ++i) {
            futures.push_back(std::async(std::launch::async, [this, i]() -> bool {
                auto client = std::make_shared<http_client>();
                std::string url = test_urls_["httpbin_get"] + "?request=" + std::to_string(i);
                
                auto response = client->get(url);
                if (response && response->status_code == 200) {
                    std::cout << "  ✓ Concurrent request " << i << " completed" << std::endl;
                    return true;
                } else {
                    std::cout << "  ✗ Concurrent request " << i << " failed" << std::endl;
                    return false;
                }
            }));
        }
        
        // Wait for all requests to complete
        int successful_requests = 0;
        for (auto& future : futures) {
            if (future.get()) {
                successful_requests++;
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Concurrent requests completed:" << std::endl;
        std::cout << "  Successful: " << successful_requests << "/" << num_requests << std::endl;
        std::cout << "  Total time: " << duration.count() << " ms" << std::endl;
        std::cout << "  Average time per request: " << duration.count() / num_requests << " ms" << std::endl;
    }
    
    void test_performance_benchmark() {
        std::cout << "\n7. Performance Benchmark:" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        const int num_requests = 20;
        const std::string benchmark_url = test_urls_["httpbin_get"];
        
        std::cout << "Running performance benchmark with " << num_requests << " requests..." << std::endl;
        
        std::vector<std::chrono::milliseconds> request_times;
        int successful_requests = 0;
        
        auto total_start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_requests; ++i) {
            auto request_start = std::chrono::high_resolution_clock::now();
            
            auto response = http_client_->get(benchmark_url);
            
            auto request_end = std::chrono::high_resolution_clock::now();
            auto request_time = std::chrono::duration_cast<std::chrono::milliseconds>(request_end - request_start);
            
            if (response && response->status_code == 200) {
                successful_requests++;
                request_times.push_back(request_time);
            }
            
            // Small delay between requests
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        auto total_end = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start);
        
        // Calculate statistics
        if (!request_times.empty()) {
            auto min_time = *std::min_element(request_times.begin(), request_times.end());
            auto max_time = *std::max_element(request_times.begin(), request_times.end());
            
            long long total_request_time = 0;
            for (const auto& time : request_times) {
                total_request_time += time.count();
            }
            auto avg_time = total_request_time / request_times.size();
            
            std::cout << "Performance Results:" << std::endl;
            std::cout << "  Successful requests: " << successful_requests << "/" << num_requests << std::endl;
            std::cout << "  Success rate: " << std::fixed << std::setprecision(1) 
                      << (double)successful_requests / num_requests * 100 << "%" << std::endl;
            std::cout << "  Total time: " << total_time.count() << " ms" << std::endl;
            std::cout << "  Average request time: " << avg_time << " ms" << std::endl;
            std::cout << "  Minimum request time: " << min_time.count() << " ms" << std::endl;
            std::cout << "  Maximum request time: " << max_time.count() << " ms" << std::endl;
            std::cout << "  Requests per second: " << std::fixed << std::setprecision(2)
                      << (double)successful_requests / total_time.count() * 1000 << std::endl;
        } else {
            std::cout << "No successful requests for performance analysis" << std::endl;
        }
    }
    
    std::string get_header_value(const http_response& response, const std::string& header_name) {
        auto it = response.headers.find(header_name);
        return (it != response.headers.end()) ? it->second : "";
    }
    
    bool save_response_to_file(const http_response& response, const std::string& filename) {
        // This is a simplified file save operation
        // In a real implementation, you would use proper file I/O
        std::cout << "  (File save simulation for " << filename << ")" << std::endl;
        return true;
    }
};

int main() {
    http_demo demo;
    demo.run_demo();
    return 0;
}