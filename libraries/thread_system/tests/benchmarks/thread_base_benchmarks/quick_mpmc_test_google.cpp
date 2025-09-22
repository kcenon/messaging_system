/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Quick MPMC Performance Test using Google Benchmark
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include <atomic>
#include "job_queue.h"
#include "lockfree/queues/lockfree_job_queue.h"
#include "callback_job.h"

using namespace kcenon::thread;
using namespace std::chrono;

// Helper function for producer-consumer test
template<typename QueueType>
static void run_producer_consumer_test(QueueType& queue, 
                                     size_t num_producers, 
                                     size_t num_consumers, 
                                     size_t ops_per_thread) {
    std::atomic<size_t> total_produced{0};
    std::atomic<size_t> total_consumed{0};
    
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
}

/**
 * @brief Benchmark mutex-based queue
 */
static void BM_MutexQueue(benchmark::State& state) {
    const size_t num_producers = state.range(0);
    const size_t num_consumers = state.range(1);
    const size_t ops_per_thread = state.range(2);
    
    for (auto _ : state) {
        job_queue queue;
        run_producer_consumer_test(queue, num_producers, num_consumers, ops_per_thread);
    }
    
    state.SetItemsProcessed(state.iterations() * num_producers * ops_per_thread);
    state.counters["producers"] = num_producers;
    state.counters["consumers"] = num_consumers;
    state.counters["ops_per_thread"] = ops_per_thread;
}

/**
 * @brief Benchmark lock-free queue
 */
static void BM_LockFreeQueue(benchmark::State& state) {
    const size_t num_producers = state.range(0);
    const size_t num_consumers = state.range(1);
    const size_t ops_per_thread = state.range(2);
    
    for (auto _ : state) {
        lockfree_job_queue queue;
        run_producer_consumer_test(queue, num_producers, num_consumers, ops_per_thread);
    }
    
    state.SetItemsProcessed(state.iterations() * num_producers * ops_per_thread);
    state.counters["producers"] = num_producers;
    state.counters["consumers"] = num_consumers;
    state.counters["ops_per_thread"] = ops_per_thread;
}

// Register benchmarks for different configurations
// Format: Args({producers, consumers, ops_per_thread})

// 1P-1C configuration
BENCHMARK(BM_MutexQueue)
    ->Args({1, 1, 10000})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_LockFreeQueue)
    ->Args({1, 1, 10000})
    ->Unit(benchmark::kMillisecond);

// 2P-2C configuration
BENCHMARK(BM_MutexQueue)
    ->Args({2, 2, 5000})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_LockFreeQueue)
    ->Args({2, 2, 5000})
    ->Unit(benchmark::kMillisecond);

// 4P-4C configuration
BENCHMARK(BM_MutexQueue)
    ->Args({4, 4, 2500})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_LockFreeQueue)
    ->Args({4, 4, 2500})
    ->Unit(benchmark::kMillisecond);

// 8P-8C configuration
BENCHMARK(BM_MutexQueue)
    ->Args({8, 8, 1250})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_LockFreeQueue)
    ->Args({8, 8, 1250})
    ->Unit(benchmark::kMillisecond);

// High contention scenarios
BENCHMARK(BM_MutexQueue)
    ->Args({16, 1, 625})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_LockFreeQueue)
    ->Args({16, 1, 625})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_MutexQueue)
    ->Args({1, 16, 10000})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_LockFreeQueue)
    ->Args({1, 16, 10000})
    ->Unit(benchmark::kMillisecond);

// Main function to run benchmarks
BENCHMARK_MAIN();