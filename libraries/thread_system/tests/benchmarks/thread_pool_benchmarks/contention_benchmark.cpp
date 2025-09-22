/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file contention_benchmark.cpp
 * @brief Google Benchmark tests for thread pool behavior under high contention scenarios
 * 
 * Tests queue contention, lock contention, and resource competition scenarios
 * using Google Benchmark framework.
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>
#include <memory>
#include <unordered_map>

#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/thread_base/jobs/callback_job.h"
#include "../../sources/utilities/core/formatter.h"

using namespace kcenon::thread;
using namespace kcenon::thread;
using namespace utility_module;

// Cache-aligned data structure for memory contention tests
struct alignas(64) cache_line_data {
    std::atomic<uint64_t> counter{0};
    char padding[64 - sizeof(std::atomic<uint64_t>)];
};

/**
 * @brief Benchmark queue contention with multiple producers and consumers
 * 
 * Measures throughput and contention when multiple threads submit and process jobs.
 */
static void BM_QueueContention(benchmark::State& state) {
    const size_t producers = state.range(0);
    const size_t consumers = state.range(1);
    const size_t jobs_per_producer = 1000;
    
    // Create thread pool with specified consumers
    auto pool = std::make_shared<thread_pool>("contention_pool");
    for (size_t i = 0; i < consumers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<uint64_t> jobs_completed{0};
        std::atomic<uint64_t> queue_collisions{0};
        
        state.PauseTiming();
        // Start producer threads
        std::vector<std::thread> producer_threads;
        for (size_t i = 0; i < producers; ++i) {
            producer_threads.emplace_back([&pool, &jobs_completed, &queue_collisions, jobs_per_producer, i]() {
                std::random_device rd;
                std::mt19937 gen(rd() + i);
                std::uniform_int_distribution<> work_dist(100, 1000);
                
                for (size_t j = 0; j < jobs_per_producer; ++j) {
                    int work_amount = work_dist(gen);
                    
                    auto job = std::make_unique<callback_job>([&jobs_completed, work_amount]() -> result_void {
                        // Simulate work
                        volatile uint64_t sum = 0;
                        for (int k = 0; k < work_amount; ++k) {
                            sum += k;
                        }
                        jobs_completed.fetch_add(1, std::memory_order_relaxed);
                        return result_void{};
                    });
                    
                    // Measure queue contention
                    auto queue_start = std::chrono::high_resolution_clock::now();
                    pool->enqueue(std::move(job));
                    auto queue_end = std::chrono::high_resolution_clock::now();
                    
                    if (std::chrono::duration_cast<std::chrono::microseconds>(queue_end - queue_start).count() > 10) {
                        queue_collisions.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }
        state.ResumeTiming();
        
        // Wait for all producers to finish
        for (auto& t : producer_threads) {
            t.join();
        }
        
        // Wait for all jobs to complete
        const size_t total_jobs = producers * jobs_per_producer;
        while (jobs_completed.load() < total_jobs) {
            std::this_thread::yield();
        }
        
        state.counters["queue_collisions"] = queue_collisions.load();
        state.counters["contention_ratio"] = (queue_collisions.load() * 100.0) / total_jobs;
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * producers * jobs_per_producer);
    state.counters["producers"] = producers;
    state.counters["consumers"] = consumers;
}
// Test matrix: Producers x Consumers
BENCHMARK(BM_QueueContention)
    ->Args({1, 1})
    ->Args({2, 2})
    ->Args({4, 4})
    ->Args({8, 8})
    ->Args({16, 16})
    ->Args({1, 8})
    ->Args({8, 1})
    ->Args({4, 8})
    ->Args({8, 4});

/**
 * @brief Benchmark shared resource contention
 * 
 * Measures performance when multiple threads access shared data structures.
 */
static void BM_SharedResourceContention(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t num_jobs = 10000;
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("shared_resource_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Shared resources
    std::atomic<uint64_t> shared_counter{0};
    std::mutex shared_mutex;
    std::unordered_map<int, int> shared_map;
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<uint64_t> jobs_completed{0};
        std::atomic<uint64_t> lock_contentions{0};
        
        state.PauseTiming();
        shared_counter = 0;
        shared_map.clear();
        
        // Submit jobs that contend for shared resources
        for (size_t i = 0; i < num_jobs; ++i) {
            auto job = std::make_unique<callback_job>([&, i]() -> result_void {
                // Atomic operation (low contention)
                shared_counter.fetch_add(1, std::memory_order_relaxed);
                
                // Mutex-protected operation (high contention)
                auto lock_start = std::chrono::high_resolution_clock::now();
                {
                    std::lock_guard<std::mutex> lock(shared_mutex);
                    shared_map[i % 100] = i;
                }
                auto lock_end = std::chrono::high_resolution_clock::now();
                
                if (std::chrono::duration_cast<std::chrono::microseconds>(lock_end - lock_start).count() > 50) {
                    lock_contentions.fetch_add(1, std::memory_order_relaxed);
                }
                
                jobs_completed.fetch_add(1, std::memory_order_relaxed);
                return result_void{};
            });
            
            pool->enqueue(std::move(job));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (jobs_completed.load() < num_jobs) {
            std::this_thread::yield();
        }
        
        state.counters["lock_contentions"] = lock_contentions.load();
        state.counters["final_counter"] = shared_counter.load();
        state.counters["map_entries"] = shared_map.size();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * num_jobs);
    state.counters["workers"] = num_workers;
}
// Test with different worker counts
BENCHMARK(BM_SharedResourceContention)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16);

/**
 * @brief Benchmark memory contention and false sharing
 * 
 * Measures performance impact of cache line bouncing and false sharing.
 */
static void BM_MemoryContention(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t num_jobs = 50000;
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("memory_contention_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Cache-aligned data to test false sharing
    const size_t num_cache_lines = std::thread::hardware_concurrency();
    std::vector<cache_line_data> cache_lines(num_cache_lines);
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<uint64_t> jobs_completed{0};
        std::atomic<uint64_t> cache_misses{0};
        
        state.PauseTiming();
        // Reset cache lines
        for (auto& line : cache_lines) {
            line.counter = 0;
        }
        
        // Submit jobs that cause false sharing
        for (size_t i = 0; i < num_jobs; ++i) {
            auto job = std::make_unique<callback_job>([&cache_lines, &jobs_completed, &cache_misses, i, num_cache_lines]() -> result_void {
                // Access different cache lines to cause bouncing
                size_t line_index = i % num_cache_lines;
                
                // Simulate cache miss with random access pattern
                for (int j = 0; j < 100; ++j) {
                    cache_lines[line_index].counter.fetch_add(1, std::memory_order_relaxed);
                    
                    // Access other cache lines to force cache misses
                    if (j % 10 == 0) {
                        size_t other_line = (line_index + 1) % num_cache_lines;
                        volatile auto value = cache_lines[other_line].counter.load(std::memory_order_relaxed);
                        if (value % 1000 == 0) {
                            cache_misses.fetch_add(1, std::memory_order_relaxed);
                        }
                    }
                }
                
                jobs_completed.fetch_add(1, std::memory_order_relaxed);
                return result_void{};
            });
            
            pool->enqueue(std::move(job));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (jobs_completed.load() < num_jobs) {
            std::this_thread::yield();
        }
        
        // Calculate total operations
        uint64_t total_counts = 0;
        for (const auto& line : cache_lines) {
            total_counts += line.counter.load();
        }
        
        state.counters["cache_misses"] = cache_misses.load();
        state.counters["total_operations"] = total_counts;
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * num_jobs);
    state.counters["workers"] = num_workers;
}
// Test with different worker counts
BENCHMARK(BM_MemoryContention)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(std::thread::hardware_concurrency());

/**
 * @brief Benchmark producer-consumer contention patterns
 * 
 * Measures performance with high-rate producers and consumer threads.
 */
static void BM_ProducerConsumerContention(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const double jobs_per_microsecond = state.range(1) / 10.0;  // Scale down for realistic rates
    const auto test_duration = std::chrono::seconds(1);
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("producer_consumer_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<uint64_t> jobs_completed{0};
        std::atomic<uint64_t> queue_collisions{0};
        std::atomic<bool> running{true};
        
        state.PauseTiming();
        // Producer thread
        std::thread producer([&]() {
            auto start_time = std::chrono::high_resolution_clock::now();
            auto next_submit = start_time;
            const auto submit_interval = std::chrono::microseconds(static_cast<int>(1.0 / jobs_per_microsecond));
            
            while (std::chrono::high_resolution_clock::now() - start_time < test_duration && running.load()) {
                auto now = std::chrono::high_resolution_clock::now();
                if (now >= next_submit) {
                    auto job = std::make_unique<callback_job>([&jobs_completed]() -> result_void {
                        // Simulate light work
                        volatile int sum = 0;
                        for (int i = 0; i < 100; ++i) {
                            sum += i;
                        }
                        jobs_completed.fetch_add(1, std::memory_order_relaxed);
                        return result_void{};
                    });
                    
                    auto queue_start = std::chrono::high_resolution_clock::now();
                    pool->enqueue(std::move(job));
                    auto queue_end = std::chrono::high_resolution_clock::now();
                    
                    if (std::chrono::duration_cast<std::chrono::microseconds>(queue_end - queue_start).count() > 5) {
                        queue_collisions.fetch_add(1, std::memory_order_relaxed);
                    }
                    
                    next_submit += submit_interval;
                } else {
                    std::this_thread::yield();
                }
            }
        });
        state.ResumeTiming();
        
        producer.join();
        running = false;
        
        // Wait for remaining jobs to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        state.counters["queue_collisions"] = queue_collisions.load();
        state.counters["jobs_completed"] = jobs_completed.load();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations());
    state.counters["workers"] = num_workers;
    state.counters["target_rate"] = jobs_per_microsecond;
}
// Test matrix: Workers x Production rate (x10 μs)
BENCHMARK(BM_ProducerConsumerContention)
    ->Args({1, 1})    // 0.1 jobs/μs
    ->Args({2, 5})    // 0.5 jobs/μs
    ->Args({4, 10})   // 1.0 jobs/μs
    ->Args({8, 20})   // 2.0 jobs/μs
    ->Args({16, 50})  // 5.0 jobs/μs
    ->Unit(benchmark::kSecond);

/**
 * @brief Benchmark cascading job dependencies
 * 
 * Measures performance when jobs spawn other jobs in chains.
 */
static void BM_CascadingDependencies(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t chain_length = state.range(1);
    const size_t initial_jobs = state.range(2);
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("cascading_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<uint64_t> jobs_completed{0};
        const size_t expected_jobs = initial_jobs * chain_length;
        
        // Submit initial jobs that will spawn chains
        std::function<void(size_t)> submit_chain_job;
        submit_chain_job = [&](size_t remaining_depth) {
            auto job = std::make_unique<callback_job>([&, remaining_depth]() -> result_void {
                // Do some work
                volatile int sum = 0;
                for (int i = 0; i < 200; ++i) {
                    sum += i * i;
                }
                
                jobs_completed.fetch_add(1, std::memory_order_relaxed);
                
                // Spawn next job in chain if not at end
                if (remaining_depth > 1) {
                    submit_chain_job(remaining_depth - 1);
                }
                
                return result_void{};
            });
            
            pool->enqueue(std::move(job));
        };
        
        state.PauseTiming();
        for (size_t i = 0; i < initial_jobs; ++i) {
            submit_chain_job(chain_length);
        }
        state.ResumeTiming();
        
        // Wait for all jobs in all chains to complete
        while (jobs_completed.load() < expected_jobs) {
            std::this_thread::yield();
        }
        
        state.counters["total_jobs"] = jobs_completed.load();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * initial_jobs * chain_length);
    state.counters["workers"] = num_workers;
    state.counters["chain_length"] = chain_length;
    state.counters["initial_jobs"] = initial_jobs;
}
// Test matrix: Workers x Chain length x Initial jobs
BENCHMARK(BM_CascadingDependencies)
    ->Args({4, 2, 100})
    ->Args({4, 4, 100})
    ->Args({4, 8, 100})
    ->Args({8, 2, 500})
    ->Args({8, 4, 500})
    ->Args({8, 8, 500})
    ->Args({16, 2, 1000})
    ->Args({16, 4, 1000})
    ->Args({16, 8, 1000});

/**
 * @brief Benchmark extreme contention scenario
 * 
 * Tests performance under extreme contention with all workers competing for same resources.
 */
static void BM_ExtremeContention(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t num_jobs = 10000;
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("extreme_contention_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Single shared atomic counter for extreme contention
    std::atomic<uint64_t> single_counter{0};
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<uint64_t> jobs_completed{0};
        
        state.PauseTiming();
        single_counter = 0;
        
        // Submit jobs that all compete for the same counter
        for (size_t i = 0; i < num_jobs; ++i) {
            auto job = std::make_unique<callback_job>([&single_counter, &jobs_completed]() -> result_void {
                // Extreme contention on single counter
                for (int j = 0; j < 1000; ++j) {
                    single_counter.fetch_add(1, std::memory_order_seq_cst);
                }
                
                jobs_completed.fetch_add(1, std::memory_order_relaxed);
                return result_void{};
            });
            
            pool->enqueue(std::move(job));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (jobs_completed.load() < num_jobs) {
            std::this_thread::yield();
        }
        
        state.counters["final_counter"] = single_counter.load();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * num_jobs);
    state.counters["workers"] = num_workers;
}
// Test with increasing worker counts
BENCHMARK(BM_ExtremeContention)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(32);

// Main function to run benchmarks
BENCHMARK_MAIN();