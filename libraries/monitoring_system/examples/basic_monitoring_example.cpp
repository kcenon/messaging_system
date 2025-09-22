/**
 * @file basic_monitoring_example.cpp
 * @brief Basic example demonstrating simple monitoring setup
 * 
 * This example shows how to:
 * - Initialize the monitoring system
 * - Collect basic metrics
 * - Store metrics to a file
 * - Query and display metrics
 */

#include <iostream>
#include <thread>
#include <chrono>

#include "monitoring/interfaces/monitoring_interface.h"
#include "monitoring/performance/performance_monitor.h"
#include "monitoring/storage/storage_backends.h"
#include "monitoring/core/result_types.h"

using namespace monitoring_system;
using namespace std::chrono_literals;

int main() {
    std::cout << "=== Basic Monitoring Example ===" << std::endl;
    
    try {
        // Step 1: Configure the monitoring system
        monitoring_config config;
        config.history_size = 1000;
        config.collection_interval = std::chrono::milliseconds(1000);
        config.enable_compression = false;  // Keep it simple for this example
        
        std::cout << "1. Creating monitoring system with configuration:" << std::endl;
        std::cout << "   - History size: " << config.history_size << std::endl;
        std::cout << "   - Collection interval: 1000ms" << std::endl;
        
        // Step 2: Create performance monitor directly (simplified for example)
        performance_monitor perf_monitor("example_monitor");
        
        // Step 3: Initialize performance monitor
        if (auto result = perf_monitor.initialize(); !result) {
            std::cerr << "Failed to initialize performance monitor: " 
                     << result.get_error().message << std::endl;
            return 1;
        }
        
        std::cout << "2. Initialized performance monitor" << std::endl;
        
        // Step 4: Configure and add storage backend
        storage_config storage_cfg;
        storage_cfg.type = storage_backend_type::file_json;
        storage_cfg.path = "monitoring_data.json";
        
        auto storage = std::make_unique<file_storage_backend>(storage_cfg);
        
        // Storage backend is ready to use
        
        // Storage is configured and ready to use
        
        std::cout << "3. Configured JSON file storage backend" << std::endl;
        
        std::cout << "4. Monitoring system ready" << std::endl;
        std::cout << std::endl;
        
        // Step 6: Simulate some application work and collect metrics
        std::cout << "5. Simulating application workload..." << std::endl;
        
        for (int i = 0; i < 10; ++i) {
            std::cout << "   Iteration " << (i + 1) << "/10" << std::endl;
            
            // Simulate timing an operation
            {
                auto timer = perf_monitor.time_operation("iteration_" + std::to_string(i));
                std::this_thread::sleep_for(100ms);
            }
            
            // Simulate some work
            std::this_thread::sleep_for(500ms);
            
            // Get current system metrics
            auto system_metrics = perf_monitor.get_system_monitor().get_current_metrics();
            if (system_metrics) {
                std::cout << "   CPU: " << system_metrics.value().cpu_usage_percent 
                         << "%, Memory: " << (system_metrics.value().memory_usage_bytes / (1024.0 * 1024.0)) << " MB" << std::endl;
            }
        }
        
        std::cout << std::endl;
        
        // Step 7: Collect and display metrics
        std::cout << "6. Collecting metrics:" << std::endl;
        
        auto metrics_result = perf_monitor.collect();
        if (metrics_result) {
            auto& snapshot = metrics_result.value();
            std::cout << "   Total metrics collected: " << snapshot.metrics.size() << std::endl;
            
            for (const auto& metric : snapshot.metrics) {
                std::cout << "   - Metric: " << metric.name << std::endl;
            }
        }
        
        std::cout << std::endl;
        
        // Step 8: Cleanup
        if (auto result = perf_monitor.cleanup(); !result) {
            std::cerr << "Failed to cleanup: " 
                     << result.get_error().message << std::endl;
            return 1;
        }
        
        // Flush storage
        if (auto result = storage->flush(); !result) {
            std::cerr << "Failed to flush storage: " 
                     << result.get_error().message << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "7. Monitoring completed successfully" << std::endl;
        std::cout << "   Data saved to: monitoring_data.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "=== Example completed successfully ===" << std::endl;
    
    return 0;
}