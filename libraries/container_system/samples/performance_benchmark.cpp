/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>
#include <thread>
#include "container.h"
#include "../internal/thread_safe_container.h"

using namespace container_module;

class performance_benchmark {
public:
    void run_all_benchmarks() {
        std::cout << "=== Container System - Performance Benchmark ===" << std::endl;
        
        benchmark_basic_operations();
        benchmark_serialization();
        benchmark_memory_usage();
        benchmark_concurrent_access();
        benchmark_simd_operations();
        
        std::cout << "\n=== All benchmarks completed ===" << std::endl;
    }

private:
    void benchmark_basic_operations() {
        std::cout << "\n1. Basic Operations Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        const int iterations = 100000;
        auto container = std::make_shared<value_container>();
        container->set_message_type("benchmark_container");
        
        // Benchmark: Set operations
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string key = "key_" + std::to_string(i);
            std::string value = "value_" + std::to_string(i);
            container->set_value(key, std::make_shared<string_value>(value));
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        auto set_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double set_ops_per_sec = (double)iterations / set_duration.count() * 1000000;
        
        std::cout << "Set operations:" << std::endl;
        std::cout << "  " << iterations << " operations in " << set_duration.count() << " μs" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << set_ops_per_sec << " ops/sec" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(3) << (double)set_duration.count() / iterations << " μs/op" << std::endl;
        
        // Benchmark: Get operations
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string key = "key_" + std::to_string(i);
            auto value = container->get_value(key);
            volatile bool exists = (value != nullptr); // Prevent optimization
            (void)exists;
        }
        end = std::chrono::high_resolution_clock::now();
        
        auto get_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double get_ops_per_sec = (double)iterations / get_duration.count() * 1000000;
        
        std::cout << "Get operations:" << std::endl;
        std::cout << "  " << iterations << " operations in " << get_duration.count() << " μs" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << get_ops_per_sec << " ops/sec" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(3) << (double)get_duration.count() / iterations << " μs/op" << std::endl;
        
        std::cout << "Container final size: " << container->size() << " entries" << std::endl;
    }
    
    void benchmark_serialization() {
        std::cout << "\n2. Serialization Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        // Create containers of different sizes
        std::vector<int> sizes = {100, 1000, 10000, 50000};
        
        for (int size : sizes) {
            auto container = create_test_container(size);
            
            // Benchmark serialization
            auto start = std::chrono::high_resolution_clock::now();
            std::string serialized = container->serialize();
            auto end = std::chrono::high_resolution_clock::now();
            
            auto serialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            // Benchmark deserialization
            start = std::chrono::high_resolution_clock::now();
            auto restored = std::make_shared<value_container>(serialized);
            end = std::chrono::high_resolution_clock::now();
            
            auto deserialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << "Container size " << size << " entries:" << std::endl;
            std::cout << "  Serialization: " << serialize_duration.count() << " μs" << std::endl;
            std::cout << "  Deserialization: " << deserialize_duration.count() << " μs" << std::endl;
            std::cout << "  Serialized size: " << serialized.length() << " bytes" << std::endl;
            std::cout << "  Compression ratio: " << std::fixed << std::setprecision(2) 
                      << (double)serialized.length() / (size * 50) << std::endl; // Assume ~50 bytes per entry
        }
    }
    
    void benchmark_memory_usage() {
        std::cout << "\n3. Memory Usage Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        const std::vector<int> sizes = {1000, 10000, 100000};
        
        for (int size : sizes) {
            auto container = std::make_shared<value_container>();
            container->set_message_type("memory_test");
            
            size_t estimated_memory = 0;
            
            // Add various types of data
            for (int i = 0; i < size; ++i) {
                if (i % 4 == 0) {
                    // String values
                    std::string key = "str_key_" + std::to_string(i);
                    std::string value = "string_value_" + std::to_string(i) + "_with_some_extra_data";
                    container->set_value(key, std::make_shared<string_value>(value));
                    estimated_memory += key.length() + value.length() + 64; // Overhead estimate
                } else if (i % 4 == 1) {
                    // Boolean values
                    std::string key = "bool_key_" + std::to_string(i);
                    container->set_value(key, std::make_shared<bool_value>(i % 2 == 0));
                    estimated_memory += key.length() + 1 + 32;
                } else if (i % 4 == 2) {
                    // Binary data
                    std::string key = "bytes_key_" + std::to_string(i);
                    std::vector<uint8_t> data(100 + (i % 900)); // Variable size binary data
                    std::fill(data.begin(), data.end(), static_cast<uint8_t>(i % 256));
                    container->set_value(key, std::make_shared<bytes_value>(data));
                    estimated_memory += key.length() + data.size() + 32;
                } else {
                    // Nested containers
                    std::string key = "nested_key_" + std::to_string(i);
                    auto nested = std::make_shared<value_container>();
                    nested->set_message_type("nested_" + std::to_string(i));
                    nested->set_value("nested_data", std::make_shared<string_value>("nested_value_" + std::to_string(i)));
                    container->set_value(key, std::make_shared<container_value>(nested));
                    estimated_memory += key.length() + 200; // Rough estimate for nested container
                }
            }
            
            std::cout << "Container with " << size << " mixed entries:" << std::endl;
            std::cout << "  Actual container size: " << container->size() << " entries" << std::endl;
            std::cout << "  Estimated memory usage: " << estimated_memory / 1024 << " KB" << std::endl;
            std::cout << "  Average bytes per entry: " << estimated_memory / container->size() << " bytes" << std::endl;
            
            // Test serialization size as a proxy for memory efficiency
            std::string serialized = container->serialize();
            std::cout << "  Serialized size: " << serialized.length() / 1024 << " KB" << std::endl;
            std::cout << "  Serialization efficiency: " << std::fixed << std::setprecision(2)
                      << (double)serialized.length() / estimated_memory * 100 << "%" << std::endl;
        }
    }
    
    void benchmark_concurrent_access() {
        std::cout << "\n4. Concurrent Access Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        const int num_threads = std::thread::hardware_concurrency();
        const int ops_per_thread = 10000;
        
        std::cout << "Testing with " << num_threads << " threads, " 
                  << ops_per_thread << " operations per thread" << std::endl;
        
        // Test thread-safe container
        auto safe_container = std::make_shared<thread_safe_container>();
        safe_container->set_message_type("concurrent_test");
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&safe_container, ops_per_thread, t]() {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 2);
                
                for (int i = 0; i < ops_per_thread; ++i) {
                    std::string key = "thread_" + std::to_string(t) + "_key_" + std::to_string(i);
                    
                    int operation = dis(gen);
                    if (operation == 0) {
                        // Write operation
                        std::string value = "value_from_thread_" + std::to_string(t) + "_op_" + std::to_string(i);
                        safe_container->set_value(key, std::make_shared<string_value>(value));
                    } else if (operation == 1) {
                        // Read operation
                        auto value = safe_container->get_value(key);
                        volatile bool exists = (value != nullptr);
                        (void)exists;
                    } else {
                        // Mixed operation (read-modify-write)
                        auto existing = safe_container->get_value(key);
                        if (existing) {
                            std::string new_value = existing->to_string() + "_modified";
                            safe_container->set_value(key, std::make_shared<string_value>(new_value));
                        } else {
                            safe_container->set_value(key, std::make_shared<string_value>("new_value"));
                        }
                    }
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        int total_operations = num_threads * ops_per_thread;
        double ops_per_sec = (double)total_operations / duration.count() * 1000000;
        
        std::cout << "Concurrent operations completed:" << std::endl;
        std::cout << "  Total operations: " << total_operations << std::endl;
        std::cout << "  Total time: " << duration.count() / 1000 << " ms" << std::endl;
        std::cout << "  Operations per second: " << std::fixed << std::setprecision(2) << ops_per_sec << std::endl;
        std::cout << "  Final container size: " << safe_container->size() << " entries" << std::endl;
    }
    
    void benchmark_simd_operations() {
        std::cout << "\n5. SIMD Operations Benchmark:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        const int data_size = 100000;
        std::vector<uint8_t> large_binary_data(data_size);
        
        // Fill with pattern data
        for (int i = 0; i < data_size; ++i) {
            large_binary_data[i] = static_cast<uint8_t>(i % 256);
        }
        
        auto container = std::make_shared<value_container>();
        container->set_message_type("simd_test");
        
        // Benchmark: Large binary data operations
        auto start = std::chrono::high_resolution_clock::now();
        container->set_value("large_data", std::make_shared<bytes_value>(large_binary_data));
        auto end = std::chrono::high_resolution_clock::now();
        
        auto set_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        start = std::chrono::high_resolution_clock::now();
        auto retrieved = container->get_value("large_data");
        end = std::chrono::high_resolution_clock::now();
        
        auto get_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Large binary data (" << data_size << " bytes):" << std::endl;
        std::cout << "  Set operation: " << set_duration.count() << " μs" << std::endl;
        std::cout << "  Get operation: " << get_duration.count() << " μs" << std::endl;
        
        if (retrieved && retrieved->get_type() == value_types::bytes) {
            auto bytes_val = std::static_pointer_cast<bytes_value>(retrieved);
            auto retrieved_data = bytes_val->get_bytes();
            
            std::cout << "  Data integrity: " 
                      << (retrieved_data == large_binary_data ? "PASSED" : "FAILED") << std::endl;
            std::cout << "  Retrieved size: " << retrieved_data.size() << " bytes" << std::endl;
        }
        
        // Benchmark: Serialization of large binary data
        start = std::chrono::high_resolution_clock::now();
        std::string serialized = container->serialize();
        end = std::chrono::high_resolution_clock::now();
        
        auto serialize_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "  Serialization: " << serialize_duration.count() << " μs" << std::endl;
        std::cout << "  Serialized size: " << serialized.length() << " bytes" << std::endl;
        std::cout << "  Compression ratio: " << std::fixed << std::setprecision(2)
                  << (double)serialized.length() / data_size << std::endl;
    }
    
    std::shared_ptr<value_container> create_test_container(int size) {
        auto container = std::make_shared<value_container>();
        container->set_message_type("test_container_" + std::to_string(size));
        
        for (int i = 0; i < size; ++i) {
            std::string key = "test_key_" + std::to_string(i);
            std::string value = "test_value_" + std::to_string(i) + "_with_additional_data_for_realistic_size";
            container->set_value(key, std::make_shared<string_value>(value));
        }
        
        return container;
    }
};

int main() {
    performance_benchmark benchmark;
    benchmark.run_all_benchmarks();
    return 0;
}