/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file scalability_benchmark.cpp
 * @brief Google Benchmark-based scalability tests for thread pools
 * 
 * Tests how thread pools scale with different numbers of threads,
 * workload types, and system configurations using Google Benchmark.
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <algorithm>
#include <numeric>
#include <map>

#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/thread_base/jobs/callback_job.h"
#include "../../sources/utilities/core/formatter.h"

using namespace kcenon::thread;
using namespace kcenon::thread;
using namespace utility_module;

/**
 * @brief Benchmark CPU-bound workload scalability
 * 
 * Measures how thread pool scales with CPU-intensive tasks.
 */
static void BM_CPUBoundScalability(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t job_count = state.range(1);
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("cpu_bound_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<size_t> completed{0};
        
        state.PauseTiming();
        // Submit CPU-intensive jobs
        for (size_t i = 0; i < job_count; ++i) {
            auto job = std::make_unique<callback_job>([&completed]() -> result_void {
                // CPU-intensive work: prime number calculation
                volatile uint64_t sum = 0;
                for (int j = 0; j < 1000; ++j) {
                    sum += j * j;
                }
                completed.fetch_add(1, std::memory_order_relaxed);
                return result_void{};
            });
            pool->enqueue(std::move(job));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (completed.load() < job_count) {
            std::this_thread::yield();
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * job_count);
    state.counters["workers"] = num_workers;
    state.counters["job_count"] = job_count;

}
// Test matrix: Workers x Job count
BENCHMARK(BM_CPUBoundScalability)
    ->Args({1, 10000})
    ->Args({2, 10000})
    ->Args({4, 10000})
    ->Args({8, 10000})
    ->Args({16, 10000})
    ->Args({1, 100000})
    ->Args({2, 100000})
    ->Args({4, 100000})
    ->Args({8, 100000})
    ->Args({16, 100000});

/**
 * @brief Benchmark I/O-bound workload scalability
 * 
 * Measures how thread pool scales with I/O-intensive tasks.
 */
static void BM_IOBoundScalability(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t job_count = state.range(1);
    const size_t io_delay_us = state.range(2);
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("io_bound_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<size_t> completed{0};
        
        state.PauseTiming();
        // Submit I/O-bound jobs
        for (size_t i = 0; i < job_count; ++i) {
            auto job = std::make_unique<callback_job>([&completed, io_delay_us]() -> result_void {
                // Simulate I/O wait
                std::this_thread::sleep_for(std::chrono::microseconds(io_delay_us));
                completed.fetch_add(1, std::memory_order_relaxed);
                return result_void{};
            });
            pool->enqueue(std::move(job));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (completed.load() < job_count) {
            std::this_thread::yield();
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * job_count);
    state.counters["workers"] = num_workers;
    state.counters["job_count"] = job_count;
    state.counters["io_delay_us"] = io_delay_us;
}
// Test matrix: Workers x Job count x I/O delay
BENCHMARK(BM_IOBoundScalability)
    ->Args({1, 10000, 100})
    ->Args({2, 10000, 100})
    ->Args({4, 10000, 100})
    ->Args({8, 10000, 100})
    ->Args({16, 10000, 100})
    ->Args({1, 10000, 1000})
    ->Args({2, 10000, 1000})
    ->Args({4, 10000, 1000})
    ->Args({8, 10000, 1000})
    ->Args({16, 10000, 1000});

/**
 * @brief Benchmark mixed workload scalability
 * 
 * Measures how thread pool scales with mixed CPU/IO/Memory workloads.
 */
static void BM_MixedWorkloadScalability(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t job_count = 50000;
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("mixed_workload_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Random number generator for workload distribution
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> workload_dist(0, 2);
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<size_t> completed{0};
        
        state.PauseTiming();
        // Submit mixed workload jobs
        for (size_t i = 0; i < job_count; ++i) {
            int workload_type = workload_dist(gen);
            
            auto job = std::make_unique<callback_job>([&completed, workload_type]() -> result_void {
                switch (workload_type) {
                    case 0: // CPU-intensive
                        {
                            volatile uint64_t sum = 0;
                            for (int j = 0; j < 500; ++j) {
                                sum += j * j;
                            }
                        }
                        break;
                    case 1: // I/O simulation
                        std::this_thread::sleep_for(std::chrono::microseconds(10));
                        break;
                    case 2: // Memory-intensive
                        {
                            std::vector<int> temp(1000);
                            std::iota(temp.begin(), temp.end(), 0);
                            std::sort(temp.begin(), temp.end(), std::greater<int>());
                        }
                        break;
                }
                completed.fetch_add(1, std::memory_order_relaxed);
                return result_void{};
            });
            pool->enqueue(std::move(job));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (completed.load() < job_count) {
            std::this_thread::yield();
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * job_count);
    state.counters["workers"] = num_workers;
}
// Test with different worker counts
BENCHMARK(BM_MixedWorkloadScalability)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(std::thread::hardware_concurrency());

/**
 * @brief Benchmark burst workload scalability
 * 
 * Measures how thread pool handles sudden bursts of jobs.
 */
static void BM_BurstWorkloadScalability(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t burst_size = 1000;
    const size_t num_bursts = 10;
    const auto burst_interval = std::chrono::milliseconds(50);
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("burst_workload_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<size_t> completed{0};
        const size_t total_jobs = burst_size * num_bursts;
        
        state.PauseTiming();
        // Submit jobs in bursts
        for (size_t burst = 0; burst < num_bursts; ++burst) {
            // Submit burst of jobs
            for (size_t i = 0; i < burst_size; ++i) {
                auto job = std::make_unique<callback_job>([&completed]() -> result_void {
                    volatile uint64_t sum = 0;
                    for (int j = 0; j < 100; ++j) {
                        sum += j;
                    }
                    completed.fetch_add(1, std::memory_order_relaxed);
                    return result_void{};
                });
                pool->enqueue(std::move(job));
            }
            
            // Wait between bursts
            if (burst < num_bursts - 1) {
                std::this_thread::sleep_for(burst_interval);
            }
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (completed.load() < total_jobs) {
            std::this_thread::yield();
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * burst_size * num_bursts);
    state.counters["workers"] = num_workers;
    state.counters["burst_size"] = burst_size;
    state.counters["num_bursts"] = num_bursts;
}
// Test with different worker counts
BENCHMARK(BM_BurstWorkloadScalability)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(std::thread::hardware_concurrency());

/**
 * @brief Benchmark scaling efficiency
 * 
 * Measures how efficiently the thread pool scales compared to single thread baseline.
 */
static void BM_ScalingEfficiency(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t total_jobs = 100000;
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("scaling_efficiency_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<size_t> completed{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Submit all jobs
        for (size_t i = 0; i < total_jobs; ++i) {
            auto job = std::make_unique<callback_job>([&completed]() -> result_void {
                // Simulate some work
                volatile int sum = 0;
                for (int j = 0; j < 1000; ++j) {
                    sum += j;
                }
                completed.fetch_add(1);
                return result_void{};
            });
            pool->enqueue(std::move(job));
        }
        
        // Wait for completion
        while (completed.load() < total_jobs) {
            std::this_thread::yield();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        state.SetIterationTime(duration.count() / 1000.0);
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * total_jobs);
    state.counters["workers"] = num_workers;
    state.counters["jobs_per_worker"] = total_jobs / num_workers;
}
// Test with different thread counts
BENCHMARK(BM_ScalingEfficiency)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(std::thread::hardware_concurrency())
    ->UseManualTime();

/**
 * @brief Benchmark weak scaling
 * 
 * Measures performance when both problem size and worker count increase proportionally.
 */
static void BM_WeakScaling(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t jobs_per_worker = 10000;
    const size_t total_jobs = num_workers * jobs_per_worker;
    
    // Create thread pool
    auto pool = std::make_shared<thread_pool>("weak_scaling_pool");
    for (size_t i = 0; i < num_workers; ++i) {
        auto worker = std::make_unique<thread_worker>();
        pool->enqueue(std::move(worker));
    }
    pool->start();
    
    // Benchmark
    for (auto _ : state) {
        std::atomic<size_t> completed{0};
        
        state.PauseTiming();
        // Submit jobs
        for (size_t i = 0; i < total_jobs; ++i) {
            auto job = std::make_unique<callback_job>([&completed]() -> result_void {
                // Fixed amount of work per job
                volatile uint64_t sum = 0;
                for (int j = 0; j < 1000; ++j) {
                    sum += j * j;
                }
                completed.fetch_add(1);
                return result_void{};
            });
            pool->enqueue(std::move(job));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (completed.load() < total_jobs) {
            std::this_thread::yield();
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * total_jobs);
    state.counters["workers"] = num_workers;
    state.counters["total_jobs"] = total_jobs;
    state.counters["jobs_per_worker"] = jobs_per_worker;
}
// Test weak scaling
BENCHMARK(BM_WeakScaling)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16);

// Main function to run benchmarks
BENCHMARK_MAIN();