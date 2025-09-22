/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Quick MPMC Performance Test
*****************************************************************************/

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <iomanip>
#include "job_queue.h"
#include "lockfree/queues/lockfree_job_queue.h"
#include "callback_job.h"

using namespace thread_module;
using namespace std::chrono;

// Test separate producer-consumer scenario
template<typename QueueType>
double test_producer_consumer(const std::string& name, size_t num_producers, size_t num_consumers, size_t ops_per_thread) {
    QueueType queue;
    std::atomic<size_t> total_produced{0};
    std::atomic<size_t> total_consumed{0};
    
    auto start = high_resolution_clock::now();
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    
    // Start producers
    for (size_t i = 0; i < num_producers; ++i) {
        producers.emplace_back([&queue, &total_produced, ops_per_thread]() {
            for (size_t j = 0; j < ops_per_thread; ++j) {
                auto job = std::make_unique<callback_job>([]() -> result_void {
                    // Simulate minimal work
                    volatile int x = 0;
                    for (int i = 0; i < 10; ++i) x = x + 1;
                    return result_void();
                });
                
                while (!queue.enqueue(std::move(job))) {
                    std::this_thread::yield();
                }
                total_produced.fetch_add(1);
            }
        });
    }
    
    // Start consumers
    for (size_t i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, &total_consumed, &total_produced, num_producers, ops_per_thread, num_consumers]() {
            size_t consumed = 0;
            size_t total_target = num_producers * ops_per_thread;
            
            while (consumed < total_target / num_consumers || 
                   total_consumed.load() < total_target) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    auto work_result = result.value()->do_work();
                    (void)work_result;
                    consumed++;
                    total_consumed.fetch_add(1);
                } else if (total_produced.load() >= total_target && 
                          total_consumed.load() >= total_target) {
                    break;
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();
    
    auto end = high_resolution_clock::now();
    auto duration_ms = duration_cast<microseconds>(end - start).count() / 1000.0;
    
    std::cout << std::setw(25) << name 
              << " - Time: " << std::fixed << std::setprecision(2) << duration_ms << " ms"
              << " - Throughput: " << std::fixed << std::setprecision(0) 
              << (total_consumed.load() / (duration_ms / 1000.0)) << " ops/sec\n";
    
    return duration_ms;
}

int main() {
    std::cout << "MPMC Queue Performance Test\n";
    std::cout << "===========================\n\n";
    
    // Test configurations
    struct Config {
        std::string name;
        size_t producers;
        size_t consumers;
        size_t ops_per_thread;
    };
    
    std::vector<Config> configs = {
        {"1P-1C (10K ops)", 1, 1, 10000},
        {"2P-2C (5K ops)", 2, 2, 5000},
        {"4P-4C (2.5K ops)", 4, 4, 2500},
    };
    
    for (const auto& config : configs) {
        std::cout << "\nTest: " << config.name << " - Total: " 
                  << config.producers * config.ops_per_thread << " operations\n";
        std::cout << std::string(70, '-') << "\n";
        
        double mutex_time = test_producer_consumer<job_queue>(
            "Mutex-based Queue", 
            config.producers, 
            config.consumers, 
            config.ops_per_thread
        );
        
        double lockfree_time = test_producer_consumer<lockfree_job_queue>(
            "Lock-free MPMC Queue", 
            config.producers, 
            config.consumers, 
            config.ops_per_thread
        );
        
        double improvement = ((mutex_time / lockfree_time) - 1) * 100;
        std::cout << "\nLock-free improvement: " << std::fixed << std::setprecision(1) 
                  << improvement << "%\n";
    }
    
    std::cout << "\n";
    return 0;
}