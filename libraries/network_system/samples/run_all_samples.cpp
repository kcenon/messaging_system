/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Network System Project
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>
#include <iomanip>

// Sample function declarations
void run_basic_usage_sample();
void run_tcp_server_client_sample();
void run_http_client_demo_sample();

// Sample registry
struct sample_info {
    std::string name;
    std::string description;
    std::function<void()> runner;
};

class sample_runner {
public:
    sample_runner() {
        register_samples();
    }
    
    void run_all_samples() {
        std::cout << "=== Network System - All Samples Runner ===" << std::endl;
        std::cout << "Running " << samples_.size() << " samples..." << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (const auto& sample : samples_) {
            std::cout << "\n" << std::string(60, '=') << std::endl;
            std::cout << "Running sample: " << sample.name << std::endl;
            std::cout << "Description: " << sample.description << std::endl;
            std::cout << std::string(60, '=') << std::endl;
            
            auto sample_start = std::chrono::high_resolution_clock::now();
            
            try {
                sample.runner();
            } catch (const std::exception& e) {
                std::cout << "Error running sample '" << sample.name << "': " << e.what() << std::endl;
            } catch (...) {
                std::cout << "Unknown error running sample '" << sample.name << "'" << std::endl;
            }
            
            auto sample_end = std::chrono::high_resolution_clock::now();
            auto sample_duration = std::chrono::duration_cast<std::chrono::milliseconds>(sample_end - sample_start);
            
            std::cout << "\nSample '" << sample.name << "' completed in " << sample_duration.count() << " ms" << std::endl;
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "All samples completed successfully!" << std::endl;
        std::cout << "Total execution time: " << total_duration.count() << " ms" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
    
    void run_specific_sample(const std::string& sample_name) {
        for (const auto& sample : samples_) {
            if (sample.name == sample_name) {
                std::cout << "=== Network System - " << sample.name << " Sample ===" << std::endl;
                std::cout << "Description: " << sample.description << std::endl;
                std::cout << std::string(50, '-') << std::endl;
                
                auto start_time = std::chrono::high_resolution_clock::now();
                
                try {
                    sample.runner();
                } catch (const std::exception& e) {
                    std::cout << "Error running sample: " << e.what() << std::endl;
                    return;
                } catch (...) {
                    std::cout << "Unknown error running sample" << std::endl;
                    return;
                }
                
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                
                std::cout << "\nSample completed successfully in " << duration.count() << " ms" << std::endl;
                return;
            }
        }
        
        std::cout << "Sample '" << sample_name << "' not found!" << std::endl;
        list_available_samples();
    }
    
    void list_available_samples() {
        std::cout << "\nAvailable samples:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        for (const auto& sample : samples_) {
            std::cout << "  " << std::setw(20) << std::left << sample.name 
                      << " - " << sample.description << std::endl;
        }
        std::cout << std::string(50, '-') << std::endl;
    }
    
private:
    std::vector<sample_info> samples_;
    
    void register_samples() {
        samples_ = {
            {
                "basic_usage",
                "Demonstrates fundamental network operations",
                run_basic_usage_sample
            },
            {
                "tcp_server_client",
                "Shows TCP server/client communication patterns",
                run_tcp_server_client_sample
            },
            {
                "http_client_demo",
                "HTTP client functionality and web requests",
                run_http_client_demo_sample
            }
        };
    }
};

// Sample implementations (delegated to actual sample programs)
void run_basic_usage_sample() {
    std::cout << "Note: This would run the basic usage sample." << std::endl;
    std::cout << "To run the actual sample, execute: ./basic_usage" << std::endl;
    
    // Simulate basic operations demo
    std::cout << "\nNetwork System Basic Usage Simulation:" << std::endl;
    std::cout << "- TCP Server/Client operations" << std::endl;
    std::cout << "- UDP communication" << std::endl;
    std::cout << "- HTTP client requests" << std::endl;
    std::cout << "- Network utilities and diagnostics" << std::endl;
    std::cout << "- Error handling and cleanup" << std::endl;
}

void run_tcp_server_client_sample() {
    std::cout << "Note: This would run the TCP server/client sample." << std::endl;
    std::cout << "To run the actual sample, execute: ./tcp_server_client" << std::endl;
    
    // Simulate TCP operations demo
    std::cout << "\nTCP Server/Client Demo Simulation:" << std::endl;
    std::cout << "- Multi-client TCP server" << std::endl;
    std::cout << "- Concurrent client connections" << std::endl;
    std::cout << "- Text and binary data transmission" << std::endl;
    std::cout << "- Performance benchmarking" << std::endl;
    std::cout << "- Connection management" << std::endl;
}

void run_http_client_demo_sample() {
    std::cout << "Note: This would run the HTTP client demo sample." << std::endl;
    std::cout << "To run the actual sample, execute: ./http_client_demo" << std::endl;
    
    // Simulate HTTP operations demo
    std::cout << "\nHTTP Client Demo Simulation:" << std::endl;
    std::cout << "- GET and POST requests" << std::endl;
    std::cout << "- Custom headers and authentication" << std::endl;
    std::cout << "- File upload and download" << std::endl;
    std::cout << "- Error handling and status codes" << std::endl;
    std::cout << "- Concurrent requests and performance testing" << std::endl;
}

void print_usage(const char* program_name) {
    std::cout << "Network System Samples Runner" << std::endl;
    std::cout << "Usage: " << program_name << " [sample_name]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  <no args>       Run all samples" << std::endl;
    std::cout << "  sample_name     Run specific sample" << std::endl;
    std::cout << "  --list          List available samples" << std::endl;
    std::cout << "  --help          Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                     # Run all samples" << std::endl;
    std::cout << "  " << program_name << " basic_usage         # Run basic usage sample" << std::endl;
    std::cout << "  " << program_name << " tcp_server_client   # Run TCP server/client sample" << std::endl;
    std::cout << "  " << program_name << " http_client_demo    # Run HTTP client demo" << std::endl;
    std::cout << "  " << program_name << " --list              # List all samples" << std::endl;
}

int main(int argc, char* argv[]) {
    sample_runner runner;
    
    if (argc == 1) {
        // No arguments - run all samples
        runner.run_all_samples();
    } else if (argc == 2) {
        std::string arg = argv[1];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
        } else if (arg == "--list" || arg == "-l") {
            std::cout << "=== Network System - Available Samples ===" << std::endl;
            runner.list_available_samples();
        } else {
            // Run specific sample
            runner.run_specific_sample(arg);
        }
    } else {
        std::cout << "Error: Too many arguments" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}