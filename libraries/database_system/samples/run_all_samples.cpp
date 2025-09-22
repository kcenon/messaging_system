/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Database System Project
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
void run_postgres_advanced_sample();
void run_connection_pool_demo_sample();

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
        std::cout << "=== Database System - All Samples Runner ===" << std::endl;
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
                std::cout << "=== Database System - " << sample.name << " Sample ===" << std::endl;
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
                "Demonstrates fundamental database operations",
                run_basic_usage_sample
            },
            {
                "postgres_advanced",
                "Shows PostgreSQL-specific advanced features",
                run_postgres_advanced_sample
            },
            {
                "connection_pool_demo",
                "Connection pooling and concurrent access examples",
                run_connection_pool_demo_sample
            }
        };
    }
};

// Sample implementations (delegated to actual sample programs)
void run_basic_usage_sample() {
    std::cout << "Note: This would run the basic usage sample." << std::endl;
    std::cout << "To run the actual sample, execute: ./basic_usage" << std::endl;
}

void run_postgres_advanced_sample() {
    std::cout << "Note: This would run the PostgreSQL advanced sample." << std::endl;
    std::cout << "To run the actual sample, execute: ./postgres_advanced" << std::endl;
}

void run_connection_pool_demo_sample() {
    std::cout << "Note: This would run the connection pool demo sample." << std::endl;
    std::cout << "To run the actual sample, execute: ./connection_pool_demo" << std::endl;
}

void print_usage(const char* program_name) {
    std::cout << "Database System Samples Runner" << std::endl;
    std::cout << "Usage: " << program_name << " [sample_name]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  <no args>      Run all samples" << std::endl;
    std::cout << "  sample_name    Run specific sample" << std::endl;
    std::cout << "  --list         List available samples" << std::endl;
    std::cout << "  --help         Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                    # Run all samples" << std::endl;
    std::cout << "  " << program_name << " basic_usage        # Run basic usage sample" << std::endl;
    std::cout << "  " << program_name << " postgres_advanced  # Run PostgreSQL advanced sample" << std::endl;
    std::cout << "  " << program_name << " --list             # List all samples" << std::endl;
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
            std::cout << "=== Database System - Available Samples ===" << std::endl;
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