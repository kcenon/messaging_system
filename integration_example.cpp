/**
 * @file integration_example.cpp
 * @brief Example demonstrating integration of modular systems with messaging_system
 */

#include <iostream>
#include <thread>
#include <chrono>

// Check if external systems are available
#ifdef HAS_THREAD_SYSTEM_CORE
#include <thread_system_core/thread_pool/core/thread_pool.h>
#endif

#ifdef HAS_LOGGER_SYSTEM
#include <logger_system/logger/logger.h>
#endif

#ifdef HAS_MONITORING_SYSTEM
#include <monitoring_system/monitoring/monitoring.h>
#endif

// Messaging system includes
#include "container/container.h"
#include "network/network.h"

int main() {
    std::cout << "=== Messaging System with Modular Integration ===" << std::endl;
    
#ifdef HAS_THREAD_SYSTEM_CORE
    std::cout << "✓ Thread System Core: Available" << std::endl;
    
    // Create a thread pool
    thread_system::thread_pool pool(4);
    pool.start();
    
    // Submit a simple task
    auto future = pool.enqueue_task([]() {
        std::cout << "  - Task executed in thread pool" << std::endl;
        return 42;
    });
    
    std::cout << "  - Task result: " << future.get() << std::endl;
    pool.stop();
#else
    std::cout << "✗ Thread System Core: Not available" << std::endl;
#endif
    
#ifdef HAS_LOGGER_SYSTEM
    std::cout << "✓ Logger System: Available" << std::endl;
    
    // Initialize logger
    auto& logger = logger::logger::get_instance();
    logger.set_log_level(logger::log_level::info);
    
    // Log some messages
    logger.info("Logger system integrated successfully");
    logger.debug("This is a debug message");
#else
    std::cout << "✗ Logger System: Not available" << std::endl;
#endif
    
#ifdef HAS_MONITORING_SYSTEM
    std::cout << "✓ Monitoring System: Available" << std::endl;
    
    // Initialize monitoring
    monitoring::monitoring monitor;
    
    // Record some metrics
    monitor.record_metric("cpu_usage", 45.5);
    monitor.record_metric("memory_usage", 1024 * 1024 * 512); // 512MB
    
    std::cout << "  - Metrics recorded" << std::endl;
#else
    std::cout << "✗ Monitoring System: Not available" << std::endl;
#endif
    
    // Test messaging system components
    std::cout << "\n=== Testing Messaging System Components ===" << std::endl;
    
    // Test container
    {
        messaging_system::data_container<int> container;
        container.push(1);
        container.push(2);
        container.push(3);
        
        std::cout << "✓ Container: " << container.size() << " items stored" << std::endl;
    }
    
    // Note: Network component requires more setup for a full test
    std::cout << "✓ Network: Component available (full test requires server setup)" << std::endl;
    
    std::cout << "\n=== Integration Test Complete ===" << std::endl;
    
    return 0;
}