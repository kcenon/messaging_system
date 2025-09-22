/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Google Benchmark-based MPMC Queue Performance Tests
*****************************************************************************/

/**
 * @file simple_mpmc_benchmark.cpp
 * @brief Google Benchmark tests for MPMC queue implementations
 * 
 * This file benchmarks different queue implementations:
 * - Mutex-based queue (job_queue)
 * - Lock-free MPMC queue (lockfree_job_queue)  
 * - Adaptive queue (adaptive_job_queue)
 * 
 * Tests measure throughput, latency, and scalability under various
 * contention scenarios following Google Benchmark best practices.
 */

#include <benchmark/benchmark.h>
#include <atomic>
#include <vector>
#include <thread>
#include "job_queue.h"
#include "lockfree/queues/lockfree_job_queue.h"
#include "lockfree/queues/adaptive_job_queue.h"
#include "callback_job.h"

using namespace thread_module;

/**
 * @brief Benchmark mutex-based queue with single producer/consumer
 * 
 * Measures the baseline performance with no contention.
 */
static void BM_MutexQueue_SPSC(benchmark::State& state) {
    job_queue queue;
    const size_t ops_per_iteration = state.range(0);
    
    for (auto _ : state) {
        std::atomic<size_t> consumed{0};
        
        // Producer thread
        std::thread producer([&]() {
            for (size_t i = 0; i < ops_per_iteration; ++i) {
                auto job = std::make_unique<callback_job>([]() -> result_void {
                    return result_void();
                });
                queue.enqueue(std::move(job));
            }
        });
        
        // Consumer thread
        std::thread consumer([&]() {
            while (consumed.load() < ops_per_iteration) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    result.value()->do_work();
                    consumed.fetch_add(1);
                }
            }
        });
        
        producer.join();
        consumer.join();
    }
    
    state.SetItemsProcessed(state.iterations() * ops_per_iteration);
    state.counters["throughput"] = benchmark::Counter(
        state.iterations() * ops_per_iteration,
        benchmark::Counter::kIsRate
    );
}
BENCHMARK(BM_MutexQueue_SPSC)->Arg(10000)->Arg(100000);

/**
 * @brief Benchmark lock-free queue with single producer/consumer
 */
static void BM_LockFreeQueue_SPSC(benchmark::State& state) {
    lockfree_job_queue queue;
    const size_t ops_per_iteration = state.range(0);
    
    for (auto _ : state) {
        std::atomic<size_t> consumed{0};
        
        // Producer thread
        std::thread producer([&]() {
            for (size_t i = 0; i < ops_per_iteration; ++i) {
                auto job = std::make_unique<callback_job>([]() -> result_void {
                    return result_void();
                });
                queue.enqueue(std::move(job));
            }
        });
        
        // Consumer thread
        std::thread consumer([&]() {
            while (consumed.load() < ops_per_iteration) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    result.value()->do_work();
                    consumed.fetch_add(1);
                }
            }
        });
        
        producer.join();
        consumer.join();
    }
    
    state.SetItemsProcessed(state.iterations() * ops_per_iteration);
    state.counters["throughput"] = benchmark::Counter(
        state.iterations() * ops_per_iteration,
        benchmark::Counter::kIsRate
    );
}
BENCHMARK(BM_LockFreeQueue_SPSC)->Arg(10000)->Arg(100000);

/**
 * @brief Benchmark adaptive queue with single producer/consumer
 */
static void BM_AdaptiveQueue_SPSC(benchmark::State& state) {
    adaptive_job_queue queue;
    const size_t ops_per_iteration = state.range(0);
    
    for (auto _ : state) {
        std::atomic<size_t> consumed{0};
        
        // Producer thread
        std::thread producer([&]() {
            for (size_t i = 0; i < ops_per_iteration; ++i) {
                auto job = std::make_unique<callback_job>([]() -> result_void {
                    return result_void();
                });
                queue.enqueue(std::move(job));
            }
        });
        
        // Consumer thread
        std::thread consumer([&]() {
            while (consumed.load() < ops_per_iteration) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    result.value()->do_work();
                    consumed.fetch_add(1);
                }
            }
        });
        
        producer.join();
        consumer.join();
    }
    
    state.SetItemsProcessed(state.iterations() * ops_per_iteration);
    state.counters["throughput"] = benchmark::Counter(
        state.iterations() * ops_per_iteration,
        benchmark::Counter::kIsRate
    );
}
BENCHMARK(BM_AdaptiveQueue_SPSC)->Arg(10000)->Arg(100000);

/**
 * @brief Template function for MPMC benchmarks
 * 
 * Tests queue performance with multiple producers and consumers.
 */
template<typename QueueType>
static void BM_Queue_MPMC(benchmark::State& state) {
    const size_t num_producers = state.range(0);
    const size_t num_consumers = state.range(1);
    const size_t ops_per_thread = state.range(2);
    
    for (auto _ : state) {
        QueueType queue;
        std::atomic<size_t> produced{0};
        std::atomic<size_t> consumed{0};
        
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        
        // Start producers
        for (size_t i = 0; i < num_producers; ++i) {
            producers.emplace_back([&]() {
                for (size_t j = 0; j < ops_per_thread; ++j) {
                    auto job = std::make_unique<callback_job>([]() -> result_void {
                        return result_void();
                    });
                    queue.enqueue(std::move(job));
                    produced.fetch_add(1);
                }
            });
        }
        
        // Start consumers
        const size_t total_operations = num_producers * ops_per_thread;
        for (size_t i = 0; i < num_consumers; ++i) {
            consumers.emplace_back([&]() {
                while (consumed.load() < total_operations) {
                    auto result = queue.dequeue();
                    if (result.has_value()) {
                        result.value()->do_work();
                        consumed.fetch_add(1);
                    } else if (produced.load() >= total_operations) {
                        if (consumed.load() >= total_operations) {
                            break;
                        }
                        std::this_thread::yield();
                    }
                }
            });
        }
        
        // Wait for completion
        for (auto& t : producers) t.join();
        for (auto& t : consumers) t.join();
    }
    
    const size_t total_ops = state.iterations() * num_producers * ops_per_thread;
    state.SetItemsProcessed(total_ops);
    state.counters["throughput"] = benchmark::Counter(
        total_ops,
        benchmark::Counter::kIsRate
    );
    state.counters["producers"] = num_producers;
    state.counters["consumers"] = num_consumers;
}

// Register MPMC benchmarks for each queue type
BENCHMARK_TEMPLATE(BM_Queue_MPMC, job_queue)
    ->Args({2, 2, 5000})   // Low contention
    ->Args({4, 4, 2500})   // Medium contention
    ->Args({8, 8, 1250});  // High contention

BENCHMARK_TEMPLATE(BM_Queue_MPMC, lockfree_job_queue)
    ->Args({2, 2, 5000})
    ->Args({4, 4, 2500})
    ->Args({8, 8, 1250});

BENCHMARK_TEMPLATE(BM_Queue_MPMC, adaptive_job_queue)
    ->Args({2, 2, 5000})
    ->Args({4, 4, 2500})
    ->Args({8, 8, 1250});

/**
 * @brief Benchmark queue latency under contention
 * 
 * Measures the time for a single enqueue/dequeue operation
 * when multiple threads are competing.
 */
template<typename QueueType>
static void BM_Queue_Latency(benchmark::State& state) {
    const size_t num_threads = state.range(0);
    QueueType queue;
    
    // Pre-fill queue
    for (size_t i = 0; i < 1000; ++i) {
        auto job = std::make_unique<callback_job>([]() -> result_void {
            return result_void();
        });
        queue.enqueue(std::move(job));
    }
    
    // Create contention with background threads
    std::atomic<bool> stop_flag{false};
    std::vector<std::thread> background_threads;
    
    for (size_t i = 0; i < num_threads - 1; ++i) {
        background_threads.emplace_back([&]() {
            while (!stop_flag.load()) {
                auto job = std::make_unique<callback_job>([]() -> result_void {
                    return result_void();
                });
                queue.enqueue(std::move(job));
                
                auto result = queue.dequeue();
                if (result.has_value()) {
                    result.value()->do_work();
                }
            }
        });
    }
    
    // Measure latency
    for (auto _ : state) {
        auto job = std::make_unique<callback_job>([]() -> result_void {
            return result_void();
        });
        
        auto start = std::chrono::high_resolution_clock::now();
        queue.enqueue(std::move(job));
        auto result = queue.dequeue();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        state.SetIterationTime(elapsed.count() / 1e9);
    }
    
    // Stop background threads
    stop_flag.store(true);
    for (auto& t : background_threads) t.join();
    
    state.SetItemsProcessed(state.iterations());
    state.counters["threads"] = num_threads;
}

// Register latency benchmarks
BENCHMARK_TEMPLATE(BM_Queue_Latency, job_queue)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)
    ->UseManualTime();

BENCHMARK_TEMPLATE(BM_Queue_Latency, lockfree_job_queue)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)
    ->UseManualTime();

BENCHMARK_TEMPLATE(BM_Queue_Latency, adaptive_job_queue)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)
    ->UseManualTime();

/**
 * @brief Benchmark batch operations
 * 
 * Measures performance when enqueuing/dequeuing multiple items at once.
 */
template<typename QueueType>
static void BM_Queue_Batch(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    QueueType queue;
    
    // Prepare batch of jobs
    std::vector<std::unique_ptr<job>> batch;
    for (size_t i = 0; i < batch_size; ++i) {
        batch.push_back(std::make_unique<callback_job>([]() -> result_void {
            return result_void();
        }));
    }
    
    for (auto _ : state) {
        // Enqueue batch
        for (size_t i = 0; i < batch_size; ++i) {
            auto job = std::make_unique<callback_job>([]() -> result_void {
                return result_void();
            });
            queue.enqueue(std::move(job));
        }
        
        // Dequeue batch
        std::vector<std::unique_ptr<job>> dequeued;
        if constexpr (std::is_same_v<QueueType, job_queue>) {
            // job_queue supports batch dequeue (no size argument)
            auto result = queue.dequeue_batch();
            if (!result.empty()) {
                for (auto& job : result) {
                    dequeued.push_back(std::move(job));
                }
            }
        } else {
            // Manual batch dequeue for other queue types
            for (size_t i = 0; i < batch_size; ++i) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    dequeued.push_back(std::move(result.value()));
                }
            }
        }
        
        // Process dequeued jobs
        for (auto& job : dequeued) {
            job->do_work();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * batch_size * 2); // enqueue + dequeue
    state.counters["batch_size"] = batch_size;
}

// Register batch benchmarks
BENCHMARK_TEMPLATE(BM_Queue_Batch, job_queue)
    ->Arg(10)->Arg(100)->Arg(1000);

BENCHMARK_TEMPLATE(BM_Queue_Batch, lockfree_job_queue)
    ->Arg(10)->Arg(100)->Arg(1000);

BENCHMARK_TEMPLATE(BM_Queue_Batch, adaptive_job_queue)
    ->Arg(10)->Arg(100)->Arg(1000);

// Main function to run all benchmarks
BENCHMARK_MAIN();