/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::cout << "=== Container System - Sample Runner ===" << std::endl;
    std::cout << "This utility runs all container system samples" << std::endl;
    
    std::vector<std::string> samples = {
        "basic_usage",
        "thread_safe_example", 
        "performance_benchmark"
    };
    
    bool run_all = true;
    std::string selected_sample;
    
    if (argc > 1) {
        selected_sample = argv[1];
        run_all = false;
        
        // Check if the selected sample exists
        bool found = false;
        for (const auto& sample : samples) {
            if (sample == selected_sample) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << "Error: Sample '" << selected_sample << "' not found." << std::endl;
            std::cout << "Available samples:" << std::endl;
            for (const auto& sample : samples) {
                std::cout << "  - " << sample << std::endl;
            }
            return 1;
        }
    }
    
    if (run_all) {
        std::cout << "\nRunning all samples..." << std::endl;
        
        for (const auto& sample : samples) {
            std::cout << "\n" << std::string(60, '=') << std::endl;
            std::cout << "Running: " << sample << std::endl;
            std::cout << std::string(60, '=') << std::endl;
            
            std::string command = "./" + sample;
            int result = std::system(command.c_str());
            
            if (result == 0) {
                std::cout << "\n✓ " << sample << " completed successfully" << std::endl;
            } else {
                std::cout << "\n✗ " << sample << " failed with exit code " << result << std::endl;
            }
        }
    } else {
        std::cout << "\nRunning selected sample: " << selected_sample << std::endl;
        std::string command = "./" + selected_sample;
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "\n✓ " << selected_sample << " completed successfully" << std::endl;
        } else {
            std::cout << "\n✗ " << selected_sample << " failed with exit code " << result << std::endl;
        }
        return result;
    }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "All samples execution completed!" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    return 0;
}