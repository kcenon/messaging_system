/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <random>
#include "container.h"
#include "values/string_value.h"
#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/container_value.h"
#include "values/numeric_value.h"

using namespace container_module;

int main() {
    std::cout << "=== Container System - Thread Safety Example ===" << std::endl;
    
    // 1. Thread-safe container creation (value_container has built-in thread safety)
    std::cout << "\n1. Thread-Safe Container Creation:" << std::endl;

    auto safe_container = std::make_shared<value_container>();
    safe_container->set_message_type("shared_data");
    
    // Initialize some shared data
    safe_container->add(std::make_shared<string_value>("counter", "0"));
    safe_container->add(std::make_shared<string_value>("total_operations", "0"));
    safe_container->add(std::make_shared<string_value>("thread_count", "0"));
    
    std::cout << "Thread-safe container initialized" << std::endl;
    std::cout << "Initial counter value: " << safe_container->get_value("counter")->to_string() << std::endl;
    
    // 2. Concurrent read/write operations
    std::cout << "\n2. Concurrent Operations Test:" << std::endl;
    
    const int num_threads = 8;
    const int operations_per_thread = 1000;
    std::atomic<int> global_counter{0};
    std::atomic<int> completed_threads{0};
    
    std::vector<std::thread> workers;
    
    // Create worker threads
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back([&safe_container, &global_counter, &completed_threads, 
                             operations_per_thread, i]() {
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 100);
            
            for (int op = 0; op < operations_per_thread; ++op) {
                // Simulate different types of operations
                int operation_type = dis(gen) % 4;
                
                switch (operation_type) {
                    case 0: { // Read operation
                        auto counter_val = safe_container->get_value("counter");
                        if (counter_val) {
                            // Just read the value (simulating read-heavy workload)
                            volatile int val = std::stoi(counter_val->to_string());
                            (void)val; // Suppress unused variable warning
                        }
                        break;
                    }
                    case 1: { // Write operation (increment counter)
                        auto current = safe_container->get_value("counter");
                        if (current && !current->is_null()) {
                            int val = std::stoi(current->to_string());
                            safe_container->remove("counter");
                            safe_container->add(std::make_shared<string_value>("counter", std::to_string(val + 1)));
                            global_counter.fetch_add(1, std::memory_order_relaxed);
                        }
                        break;
                    }
                    case 2: { // Add thread-specific data
                        std::string thread_key = "thread_" + std::to_string(i) + "_op_" + std::to_string(op);
                        std::string thread_data = "data_from_thread_" + std::to_string(i);
                        safe_container->add(std::make_shared<string_value>(thread_key, thread_data));
                        break;
                    }
                    case 3: { // Update total operations
                        auto total_ops = safe_container->get_value("total_operations");
                        if (total_ops && !total_ops->is_null()) {
                            int current_total = std::stoi(total_ops->to_string());
                            safe_container->remove("total_operations");
                            safe_container->add(std::make_shared<string_value>("total_operations", std::to_string(current_total + 1)));
                        }
                        break;
                    }
                }
                
                // Small delay to increase chance of race conditions if not properly protected
                if (op % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
            
            completed_threads.fetch_add(1, std::memory_order_release);
            std::cout << "Thread " << i << " completed " << operations_per_thread << " operations" << std::endl;
        });
    }
    
    // Wait for all threads to complete
    for (auto& worker : workers) {
        worker.join();
    }
    
    std::cout << "\nAll threads completed!" << std::endl;
    
    // 3. Verify results
    std::cout << "\n3. Results Verification:" << std::endl;
    
    auto final_counter = safe_container->get_value("counter");
    auto total_ops = safe_container->get_value("total_operations");
    
    if (final_counter && total_ops && !final_counter->is_null() && !total_ops->is_null()) {
        std::cout << "Final counter value: " << final_counter->to_string() << std::endl;
        std::cout << "Total operations recorded: " << total_ops->to_string() << std::endl;
        std::cout << "Global counter (atomic): " << global_counter.load() << std::endl;

        // Count entries manually
        size_t container_size = 0;
        container_size += safe_container->value_array("counter").size();
        container_size += safe_container->value_array("total_operations").size();
        container_size += safe_container->value_array("thread_count").size();

        // Count thread-specific entries
        for (int t = 0; t < num_threads; ++t) {
            for (int i = 0; i < operations_per_thread; ++i) {
                std::string key = "thread_" + std::to_string(t) + "_op_" + std::to_string(i);
                container_size += safe_container->value_array(key).size();
            }
        }
        std::cout << "Container size: approximately " << container_size << " entries" << std::endl;
    }
    
    // 4. Performance test
    std::cout << "\n4. Performance Test:" << std::endl;
    
    const int perf_iterations = 100000;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Sequential operations for baseline
    auto baseline_container = std::make_shared<value_container>();
    baseline_container->set_message_type("performance_test");
    
    for (int i = 0; i < perf_iterations; ++i) {
        std::string key = "perf_key_" + std::to_string(i);
        std::string value = "perf_value_" + std::to_string(i);
        baseline_container->add(std::make_shared<string_value>(key, value));
    }
    
    auto baseline_time = std::chrono::high_resolution_clock::now();
    
    // Thread-safe operations
    for (int i = 0; i < perf_iterations; ++i) {
        std::string key = "safe_key_" + std::to_string(i);
        std::string value = "safe_value_" + std::to_string(i);
        safe_container->add(std::make_shared<string_value>(key, value));
    }
    
    auto safe_time = std::chrono::high_resolution_clock::now();
    
    auto baseline_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        baseline_time - start_time).count();
    auto safe_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        safe_time - baseline_time).count();
    
    std::cout << "Baseline container (" << perf_iterations << " ops): " 
              << baseline_duration << " μs" << std::endl;
    std::cout << "Thread-safe container (" << perf_iterations << " ops): " 
              << safe_duration << " μs" << std::endl;
    std::cout << "Overhead factor: " << (double)safe_duration / baseline_duration << "x" << std::endl;
    
    // 5. Serialization test with thread-safe container
    std::cout << "\n5. Thread-Safe Serialization Test:" << std::endl;
    
    std::string safe_serialized = safe_container->serialize();
    std::cout << "Thread-safe container serialized successfully" << std::endl;
    std::cout << "Serialized size: " << safe_serialized.length() << " characters" << std::endl;
    
    auto restored_safe_container = std::make_shared<value_container>(safe_serialized);
    std::cout << "Thread-safe container restored successfully" << std::endl;

    // Count restored entries manually
    size_t restored_size = 0;
    restored_size += restored_safe_container->value_array("counter").size();
    restored_size += restored_safe_container->value_array("total_operations").size();
    restored_size += restored_safe_container->value_array("thread_count").size();

    // Count performance test entries
    for (int i = 0; i < perf_iterations; ++i) {
        restored_size += restored_safe_container->value_array("safe_key_" + std::to_string(i)).size();
    }

    std::cout << "Restored container size: approximately " << restored_size << " entries" << std::endl;
    
    // Verify some restored data
    auto restored_counter = restored_safe_container->get_value("counter");
    if (restored_counter && !restored_counter->is_null()) {
        std::cout << "Restored counter value: " << restored_counter->to_string() << std::endl;
    }
    
    std::cout << "\n=== Thread Safety Example completed successfully ===" << std::endl;
    return 0;
}