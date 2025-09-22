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
#include "values/string_value.h"
#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/container_value.h"
#include "values/numeric_value.h"

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
            container->add(std::make_shared<string_value>(key, value));
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
        
        // Count entries manually since size() is not available
        size_t container_size = 0;
        for (int i = 0; i < iterations; ++i) {
            std::string key = "key_" + std::to_string(i);
            auto values = container->value_array(key);
            container_size += values.size();
        }
        std::cout << "Container final size: " << container_size << " entries" << std::endl;
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
                    container->add(std::make_shared<string_value>(key, value));
                    estimated_memory += key.length() + value.length() + 64; // Overhead estimate
                } else if (i % 4 == 1) {
                    // Boolean values
                    std::string key = "bool_key_" + std::to_string(i);
                    container->add(std::make_shared<bool_value>(key, i % 2 == 0));
                    estimated_memory += key.length() + 1 + 32;
                } else if (i % 4 == 2) {
                    // Binary data
                    std::string key = "bytes_key_" + std::to_string(i);
                    std::vector<uint8_t> data(100 + (i % 900)); // Variable size binary data
                    std::fill(data.begin(), data.end(), static_cast<uint8_t>(i % 256));
                    container->add(std::make_shared<bytes_value>(key, data));
                    estimated_memory += key.length() + data.size() + 32;
                } else {
                    // Nested containers
                    std::string key = "nested_key_" + std::to_string(i);
                    auto nested = std::make_shared<container_value>(key);
                    nested->add(std::make_shared<string_value>("nested_data", "nested_value_" + std::to_string(i)));
                    container->add(nested);
                    estimated_memory += key.length() + 200; // Rough estimate for nested container
                }
            }
            
            // Count entries manually
            size_t actual_size = 0;
            for (int i = 0; i < size; ++i) {
                if (i % 4 == 0) {
                    auto values = container->value_array("str_key_" + std::to_string(i));
                    actual_size += values.size();
                } else if (i % 4 == 1) {
                    auto values = container->value_array("bool_key_" + std::to_string(i));
                    actual_size += values.size();
                } else if (i % 4 == 2) {
                    auto values = container->value_array("bytes_key_" + std::to_string(i));
                    actual_size += values.size();
                } else {
                    auto values = container->value_array("nested_key_" + std::to_string(i));
                    actual_size += values.size();
                }
            }
            std::cout << "Container with " << size << " mixed entries:" << std::endl;
            std::cout << "  Actual container size: " << actual_size << " entries" << std::endl;
            std::cout << "  Estimated memory usage: " << estimated_memory / 1024 << " KB" << std::endl;
            std::cout << "  Average bytes per entry: " << estimated_memory / actual_size << " bytes" << std::endl;
            
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
        
        // Test thread-safe value_container (has built-in thread safety)
        auto safe_container = std::make_shared<value_container>();
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
                        safe_container->add(std::make_shared<string_value>(key, value));
                    } else if (operation == 1) {
                        // Read operation
                        auto value = safe_container->get_value(key);
                        volatile bool exists = (value != nullptr);
                        (void)exists;
                    } else {
                        // Mixed operation (read-modify-write)
                        auto existing = safe_container->get_value(key);
                        if (existing && !existing->is_null()) {
                            std::string new_value = existing->to_string() + "_modified";
                            safe_container->remove(key);
                            safe_container->add(std::make_shared<string_value>(key, new_value));
                        } else {
                            safe_container->add(std::make_shared<string_value>(key, "new_value"));
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
        // Count entries by iterating through all thread keys
        size_t safe_container_size = 0;
        for (int t = 0; t < num_threads; ++t) {
            for (int i = 0; i < ops_per_thread; ++i) {
                std::string key = "thread_" + std::to_string(t) + "_key_" + std::to_string(i);
                auto values = safe_container->value_array(key);
                safe_container_size += values.size();
            }
        }
        std::cout << "  Final container size: approximately " << safe_container_size << " entries" << std::endl;
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
        container->add(std::make_shared<bytes_value>("large_data", large_binary_data));
        auto end = std::chrono::high_resolution_clock::now();
        
        auto set_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        start = std::chrono::high_resolution_clock::now();
        auto retrieved = container->get_value("large_data");
        end = std::chrono::high_resolution_clock::now();
        
        auto get_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Large binary data (" << data_size << " bytes):" << std::endl;
        std::cout << "  Set operation: " << set_duration.count() << " μs" << std::endl;
        std::cout << "  Get operation: " << get_duration.count() << " μs" << std::endl;
        
        if (retrieved && retrieved->type() == value_types::bytes_value) {
            auto retrieved_data = retrieved->to_bytes();
            
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
            container->add(std::make_shared<string_value>(key, value));
        }
        
        return container;
    }
};

int main() {
    performance_benchmark benchmark;
    benchmark.run_all_benchmarks();
    return 0;
}