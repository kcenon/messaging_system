/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include "job_queue.h"
#include "lockfree/queues/lockfree_job_queue.h"
#include "lockfree/queues/adaptive_job_queue.h"
#include "callback_job.h"
#include <thread>
#include <vector>
#include <atomic>
#include <random>

using namespace thread_module;

// Test configuration
constexpr size_t BATCH_SIZE = 100;
constexpr size_t SMALL_WORKLOAD = 1000;
constexpr size_t MEDIUM_WORKLOAD = 10000;
constexpr size_t LARGE_WORKLOAD = 100000;

// Helper function to create a simple job
std::unique_ptr<job> create_simple_job(std::atomic<size_t>& counter) {
    return std::make_unique<callback_job>([&counter]() -> result_void {
        counter.fetch_add(1, std::memory_order_relaxed);
        return result_void();
    });
}

// Benchmark: Single Producer Single Consumer (SPSC)
template<typename QueueType>
static void BM_SPSC(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    
    for (auto _ : state) {
        QueueType queue;
        std::atomic<size_t> counter{0};
        
        std::thread producer([&queue, &counter, num_operations]() {
            for (size_t i = 0; i < num_operations; ++i) {
                queue.enqueue(create_simple_job(counter));
            }
        });
        
        std::thread consumer([&queue, num_operations]() {
            size_t consumed = 0;
            while (consumed < num_operations) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    result.value()->do_work();
                    consumed++;
                }
            }
        });
        
        producer.join();
        consumer.join();
        
        state.counters["throughput"] = benchmark::Counter(
            static_cast<double>(num_operations), 
            benchmark::Counter::kIsRate
        );
    }
}

// Benchmark: Multiple Producer Multiple Consumer (MPMC)
template<typename QueueType>
static void BM_MPMC(benchmark::State& state) {
    const size_t num_producers = state.range(0);
    const size_t num_consumers = state.range(1);
    const size_t operations_per_producer = state.range(2);
    const size_t total_operations = num_producers * operations_per_producer;
    
    for (auto _ : state) {
        QueueType queue;
        std::atomic<size_t> produced{0};
        std::atomic<size_t> consumed{0};
        std::atomic<size_t> counter{0};
        
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        
        // Start producers
        for (size_t i = 0; i < num_producers; ++i) {
            producers.emplace_back([&queue, &produced, &counter, operations_per_producer]() {
                for (size_t j = 0; j < operations_per_producer; ++j) {
                    queue.enqueue(create_simple_job(counter));
                    produced.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        
        // Start consumers
        for (size_t i = 0; i < num_consumers; ++i) {
            consumers.emplace_back([&queue, &consumed, total_operations]() {
                while (consumed.load(std::memory_order_relaxed) < total_operations) {
                    auto result = queue.dequeue();
                    if (result.has_value()) {
                        result.value()->do_work();
                        consumed.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }
        
        // Wait for completion
        for (auto& t : producers) t.join();
        for (auto& t : consumers) t.join();
        
        state.counters["throughput"] = benchmark::Counter(
            static_cast<double>(total_operations), 
            benchmark::Counter::kIsRate
        );
        state.counters["producers"] = num_producers;
        state.counters["consumers"] = num_consumers;
    }
}

// Benchmark: Batch Operations
template<typename QueueType>
static void BM_BatchOperations(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    const size_t num_batches = state.range(1);
    
    for (auto _ : state) {
        QueueType queue;
        std::atomic<size_t> counter{0};
        
        // Enqueue in batches
        for (size_t i = 0; i < num_batches; ++i) {
            std::vector<std::unique_ptr<job>> batch;
            for (size_t j = 0; j < batch_size; ++j) {
                batch.push_back(create_simple_job(counter));
            }
            queue.enqueue_batch(std::move(batch));
        }
        
        // Dequeue in batches
        size_t total_dequeued = 0;
        while (total_dequeued < batch_size * num_batches) {
            auto batch = queue.dequeue_batch();
            for (auto& job : batch) {
                job->do_work();
            }
            total_dequeued += batch.size();
        }
        
        state.counters["throughput"] = benchmark::Counter(
            static_cast<double>(batch_size * num_batches), 
            benchmark::Counter::kIsRate
        );
        state.counters["batch_size"] = batch_size;
    }
}

// Benchmark: High Contention Scenario
template<typename QueueType>
static void BM_HighContention(benchmark::State& state) {
    const size_t num_threads = state.range(0);
    const size_t operations_per_thread = state.range(1);
    
    for (auto _ : state) {
        QueueType queue;
        std::atomic<size_t> counter{0};
        std::vector<std::thread> threads;
        
        // Each thread acts as both producer and consumer
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&queue, &counter, operations_per_thread]() {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 1);
                
                for (size_t j = 0; j < operations_per_thread; ++j) {
                    if (dis(gen) == 0) {
                        // Produce
                        queue.enqueue(create_simple_job(counter));
                    } else {
                        // Consume
                        auto result = queue.dequeue();
                        if (result.has_value()) {
                            result.value()->do_work();
                        }
                    }
                }
            });
        }
        
        for (auto& t : threads) t.join();
        
        state.counters["throughput"] = benchmark::Counter(
            static_cast<double>(num_threads * operations_per_thread), 
            benchmark::Counter::kIsRate
        );
        state.counters["threads"] = num_threads;
    }
}

// Register benchmarks for mutex-based queue
BENCHMARK_TEMPLATE(BM_SPSC, job_queue)
    ->Arg(SMALL_WORKLOAD)
    ->Arg(MEDIUM_WORKLOAD)
    ->Arg(LARGE_WORKLOAD);

BENCHMARK_TEMPLATE(BM_MPMC, job_queue)
    ->Args({2, 2, 5000})   // 2 producers, 2 consumers
    ->Args({4, 4, 2500})   // 4 producers, 4 consumers
    ->Args({8, 8, 1250});  // 8 producers, 8 consumers

BENCHMARK_TEMPLATE(BM_BatchOperations, job_queue)
    ->Args({10, 1000})     // batch size 10
    ->Args({100, 100})     // batch size 100
    ->Args({1000, 10});    // batch size 1000

BENCHMARK_TEMPLATE(BM_HighContention, job_queue)
    ->Args({4, 10000})     // 4 threads
    ->Args({8, 5000})      // 8 threads
    ->Args({16, 2500});    // 16 threads

// Register benchmarks for lock-free MPMC queue
BENCHMARK_TEMPLATE(BM_SPSC, lockfree_job_queue)
    ->Arg(SMALL_WORKLOAD)
    ->Arg(MEDIUM_WORKLOAD)
    ->Arg(LARGE_WORKLOAD);

BENCHMARK_TEMPLATE(BM_MPMC, lockfree_job_queue)
    ->Args({2, 2, 5000})   // 2 producers, 2 consumers
    ->Args({4, 4, 2500})   // 4 producers, 4 consumers
    ->Args({8, 8, 1250});  // 8 producers, 8 consumers

BENCHMARK_TEMPLATE(BM_BatchOperations, lockfree_job_queue)
    ->Args({10, 1000})     // batch size 10
    ->Args({100, 100})     // batch size 100
    ->Args({1000, 10});    // batch size 1000

BENCHMARK_TEMPLATE(BM_HighContention, lockfree_job_queue)
    ->Args({4, 10000})     // 4 threads
    ->Args({8, 5000})      // 8 threads
    ->Args({16, 2500});    // 16 threads

// Register benchmarks for adaptive queue
BENCHMARK_TEMPLATE(BM_SPSC, adaptive_job_queue)
    ->Arg(SMALL_WORKLOAD)
    ->Arg(MEDIUM_WORKLOAD)
    ->Arg(LARGE_WORKLOAD);

BENCHMARK_TEMPLATE(BM_MPMC, adaptive_job_queue)
    ->Args({2, 2, 5000})   // 2 producers, 2 consumers
    ->Args({4, 4, 2500})   // 4 producers, 4 consumers
    ->Args({8, 8, 1250});  // 8 producers, 8 consumers

// Main function
BENCHMARK_MAIN();