/*
 * BSD 3-Clause License
 * 
 * Copyright (c) 2024, DongCheol Shin
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file throughput_detailed_benchmark_google.cpp
 * @brief Detailed job throughput analysis for Thread System using Google Benchmark
 * 
 * This benchmark provides in-depth analysis of job throughput under various conditions:
 * - Different job sizes and complexities
 * - Various queue configurations
 * - Different worker counts
 * - Impact of job dependencies
 * - Effect of memory allocation patterns
 * - Throughput degradation over time
 */

#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <atomic>
#include <random>
#include <iomanip>
#include <map>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <memory>
#include <fstream>
#include <future>

#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/typed_thread_pool/pool/typed_thread_pool.h"
#include "../../sources/typed_thread_pool/scheduling/typed_thread_worker.h"
#include "../../sources/typed_thread_pool/jobs/callback_typed_job.h"
#include "../../sources/utilities/core/formatter.h"
// Helper function to create thread pool
auto create_default(const uint16_t& worker_counts)
    -> std::tuple<std::shared_ptr<thread_pool_module::thread_pool>, std::optional<std::string>>
{
    std::shared_ptr<thread_pool_module::thread_pool> pool;
    try {
        pool = std::make_shared<thread_pool_module::thread_pool>();
    } catch (const std::bad_alloc& e) {
        return { nullptr, std::string(e.what()) };
    }
    
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    workers.reserve(worker_counts);
    for (uint16_t i = 0; i < worker_counts; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    }
    
    auto enqueue_result = pool->enqueue_batch(std::move(workers));
    if (enqueue_result.has_value()) {
        return { nullptr, formatter::format("cannot enqueue to workers: {}", 
                                           enqueue_result.value()) };
    }
    
    return { pool, std::nullopt };
}

// Helper function to create typed thread pool
template<typename Type>
auto create_priority_default(const uint16_t& worker_counts)
    -> std::tuple<std::shared_ptr<typed_thread_pool_module::typed_thread_pool_t<Type>>, std::optional<std::string>>
{
    std::shared_ptr<typed_thread_pool_module::typed_thread_pool_t<Type>> pool;
    try {
        pool = std::make_shared<typed_thread_pool_module::typed_thread_pool_t<Type>>();
    } catch (const std::bad_alloc& e) {
        return { nullptr, std::string(e.what()) };
    }
    
    std::vector<std::unique_ptr<typed_thread_pool_module::typed_thread_worker_t<Type>>> workers;
    workers.reserve(worker_counts);
    for (uint16_t i = 0; i < worker_counts; ++i) {
        workers.push_back(std::make_unique<typed_thread_pool_module::typed_thread_worker_t<Type>>(std::vector<Type>{}, true));
    }
    
    auto enqueue_result = pool->enqueue_batch(std::move(workers));
    if (enqueue_result.has_error()) {
        return { nullptr, formatter::format("cannot enqueue to workers: {}", 
                                           enqueue_result.get_error().message()) };
    }
    
    return { pool, std::nullopt };
}

using namespace std::chrono;
using namespace thread_pool_module;
using namespace typed_thread_pool_module;

// Job complexity levels
enum class JobComplexity {
    Empty,          // No operation
    Trivial,        // Simple arithmetic
    Light,          // Light computation
    Medium,         // Medium computation
    Heavy,          // Heavy computation
    VeryHeavy,      // Very heavy computation
    Mixed           // Random mix
};

// Job memory patterns
enum class MemoryPattern {
    None,           // No memory allocation
    Small,          // Small allocations (<1KB)
    Medium,         // Medium allocations (1KB-100KB)
    Large,          // Large allocations (100KB-1MB)
    VeryLarge,      // Very large allocations (>1MB)
    Random          // Random sizes
};

// Helper function to execute jobs with a specific complexity
static void execute_job_with_complexity(JobComplexity complexity) {
    switch (complexity) {
        case JobComplexity::Empty:
            // No operation
            break;
            
        case JobComplexity::Trivial:
            {
                volatile int x = 42;
                x = x * 2 + 1;
            }
            break;
            
        case JobComplexity::Light:
            {
                volatile double sum = 0;
                for (int i = 0; i < 100; ++i) {
                    sum += std::sqrt(i);
                }
            }
            break;
            
        case JobComplexity::Medium:
            {
                volatile double sum = 0;
                for (int i = 0; i < 1000; ++i) {
                    sum += std::sin(i) * std::cos(i);
                }
            }
            break;
            
        case JobComplexity::Heavy:
            {
                volatile double sum = 0;
                for (int i = 0; i < 10000; ++i) {
                    sum += std::pow(std::sin(i), 2) + std::pow(std::cos(i), 2);
                }
            }
            break;
            
        case JobComplexity::VeryHeavy:
            {
                volatile double sum = 0;
                for (int i = 0; i < 100000; ++i) {
                    sum += std::log(std::abs(std::sin(i)) + 1) * std::exp(-i/10000.0);
                }
            }
            break;
            
        case JobComplexity::Mixed:
            {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_int_distribution<> dis(0, 4);
                
                JobComplexity random_complexity = static_cast<JobComplexity>(dis(gen));
                execute_job_with_complexity(random_complexity);
            }
            break;
    }
}

// Helper function to allocate memory based on pattern
static std::unique_ptr<char[]> allocate_with_pattern(MemoryPattern pattern) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    size_t size = 0;
    
    switch (pattern) {
        case MemoryPattern::None:
            return nullptr;
            
        case MemoryPattern::Small:
            size = std::uniform_int_distribution<>(100, 1024)(gen);
            break;
            
        case MemoryPattern::Medium:
            size = std::uniform_int_distribution<>(1024, 102400)(gen);
            break;
            
        case MemoryPattern::Large:
            size = std::uniform_int_distribution<>(102400, 1048576)(gen);
            break;
            
        case MemoryPattern::VeryLarge:
            size = std::uniform_int_distribution<>(1048576, 10485760)(gen);
            break;
            
        case MemoryPattern::Random:
            size = std::uniform_int_distribution<>(100, 10485760)(gen);
            break;
    }
    
    if (size > 0) {
        auto buffer = std::make_unique<char[]>(size);
        // Touch memory to ensure allocation
        for (size_t i = 0; i < size; i += 4096) {
            buffer[i] = static_cast<char>(i & 0xFF);
        }
        return buffer;
    }
    
    return nullptr;
}

/**
 * @brief Benchmark job complexity impact on throughput
 */
static void BM_JobComplexity(benchmark::State& state) {
    const JobComplexity complexity = static_cast<JobComplexity>(state.range(0));
    const size_t num_jobs = state.range(1);
    const size_t worker_count = std::thread::hardware_concurrency();
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed{0};
        
        for (size_t i = 0; i < num_jobs; ++i) {
            pool->enqueue(std::make_unique<callback_job>([complexity, &completed]() -> result_void {
                execute_job_with_complexity(complexity);
                completed.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
        
        benchmark::DoNotOptimize(completed.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_jobs);
    state.counters["complexity"] = static_cast<int>(complexity);
}
// Test different complexities
BENCHMARK(BM_JobComplexity)
    ->Args({static_cast<int>(JobComplexity::Empty), 100000})
    ->Args({static_cast<int>(JobComplexity::Trivial), 50000})
    ->Args({static_cast<int>(JobComplexity::Light), 10000})
    ->Args({static_cast<int>(JobComplexity::Medium), 5000})
    ->Args({static_cast<int>(JobComplexity::Heavy), 500})
    ->Args({static_cast<int>(JobComplexity::VeryHeavy), 50})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark worker count scaling
 */
static void BM_WorkerScaling(benchmark::State& state) {
    const size_t worker_count = state.range(0);
    const JobComplexity complexity = static_cast<JobComplexity>(state.range(1));
    const size_t num_jobs = 10000;
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed{0};
        
        for (size_t i = 0; i < num_jobs; ++i) {
            pool->enqueue(std::make_unique<callback_job>([complexity, &completed]() -> result_void {
                execute_job_with_complexity(complexity);
                completed.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * num_jobs);
    state.counters["workers"] = worker_count;
    state.counters["complexity"] = static_cast<int>(complexity);
}
// Test scaling with different worker counts and complexities
BENCHMARK(BM_WorkerScaling)
    ->Args({1, static_cast<int>(JobComplexity::Light)})
    ->Args({2, static_cast<int>(JobComplexity::Light)})
    ->Args({4, static_cast<int>(JobComplexity::Light)})
    ->Args({8, static_cast<int>(JobComplexity::Light)})
    ->Args({16, static_cast<int>(JobComplexity::Light)})
    ->Args({1, static_cast<int>(JobComplexity::Medium)})
    ->Args({2, static_cast<int>(JobComplexity::Medium)})
    ->Args({4, static_cast<int>(JobComplexity::Medium)})
    ->Args({8, static_cast<int>(JobComplexity::Medium)})
    ->Args({16, static_cast<int>(JobComplexity::Medium)})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark memory allocation impact
 */
static void BM_MemoryAllocationImpact(benchmark::State& state) {
    const MemoryPattern pattern = static_cast<MemoryPattern>(state.range(0));
    const size_t num_jobs = state.range(1);
    const size_t worker_count = std::thread::hardware_concurrency();
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed{0};
        
        for (size_t i = 0; i < num_jobs; ++i) {
            pool->enqueue(std::make_unique<callback_job>([pattern, &completed]() -> result_void {
                auto buffer = allocate_with_pattern(pattern);
                execute_job_with_complexity(JobComplexity::Light);
                completed.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * num_jobs);
    state.counters["memory_pattern"] = static_cast<int>(pattern);
}
// Test different memory patterns
BENCHMARK(BM_MemoryAllocationImpact)
    ->Args({static_cast<int>(MemoryPattern::None), 50000})
    ->Args({static_cast<int>(MemoryPattern::Small), 50000})
    ->Args({static_cast<int>(MemoryPattern::Medium), 25000})
    ->Args({static_cast<int>(MemoryPattern::Large), 5000})
    ->Args({static_cast<int>(MemoryPattern::VeryLarge), 500})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark queue depth impact
 */
static void BM_QueueDepth(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    const size_t total_jobs = state.range(1);
    const size_t worker_count = 8;
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed{0};
        
        // Submit jobs in batches
        for (size_t i = 0; i < total_jobs; i += batch_size) {
            size_t batch_end = std::min(i + batch_size, total_jobs);
            
            for (size_t j = i; j < batch_end; ++j) {
                pool->enqueue(std::make_unique<callback_job>([&completed]() -> result_void {
                    execute_job_with_complexity(JobComplexity::Medium);
                    completed.fetch_add(1);
                    return result_void();
                }));
            }
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * total_jobs);
    state.counters["batch_size"] = batch_size;
}
// Test different batch sizes
BENCHMARK(BM_QueueDepth)
    ->Args({1, 10000})
    ->Args({10, 10000})
    ->Args({100, 10000})
    ->Args({1000, 10000})
    ->Args({10000, 10000})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark burst pattern handling
 */
static void BM_BurstPattern(benchmark::State& state) {
    const size_t burst_size = state.range(0);
    const int quiet_period_ms = state.range(1);
    const int num_bursts = 10;
    const size_t worker_count = std::thread::hardware_concurrency();
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed{0};
        
        for (int burst = 0; burst < num_bursts; ++burst) {
            // Submit burst
            for (size_t i = 0; i < burst_size; ++i) {
                pool->enqueue(std::make_unique<callback_job>([&completed]() -> result_void {
                    execute_job_with_complexity(JobComplexity::Light);
                    completed.fetch_add(1);
                    return result_void();
                }));
            }
            
            // Quiet period
            if (burst < num_bursts - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(quiet_period_ms));
            }
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * burst_size * num_bursts);
    state.counters["burst_size"] = burst_size;
    state.counters["quiet_ms"] = quiet_period_ms;
}
// Test different burst patterns
BENCHMARK(BM_BurstPattern)
    ->Args({100, 10})
    ->Args({1000, 10})
    ->Args({1000, 100})
    ->Args({10000, 100})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark job dependencies impact
 */
static void BM_JobDependencies(benchmark::State& state) {
    const size_t chain_length = state.range(0);
    const size_t num_chains = state.range(1);
    const size_t worker_count = std::thread::hardware_concurrency();
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed{0};
        
        for (size_t chain = 0; chain < num_chains; ++chain) {
            // Create chain of dependent jobs
            std::vector<std::promise<void>> promises(chain_length);
            std::vector<std::future<void>> futures;
            
            for (auto& promise : promises) {
                futures.push_back(promise.get_future());
            }
            
            for (size_t i = 0; i < chain_length; ++i) {
                pool->enqueue(std::make_unique<callback_job>([i, &futures, &promises, &completed, chain_length]() -> result_void {
                    // Wait for previous job in chain
                    if (i > 0) {
                        futures[i-1].get();
                    }
                    
                    execute_job_with_complexity(JobComplexity::Light);
                    completed.fetch_add(1);
                    
                    // Signal completion
                    if (i < chain_length) {
                        promises[i].set_value();
                    }
                    return result_void();
                }));
            }
        }
        
        // Wait for all jobs to complete
        while (completed.load() < num_chains * chain_length) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * num_chains * chain_length);
    state.counters["chain_length"] = chain_length;
    state.counters["num_chains"] = num_chains;
}
// Test different dependency patterns
BENCHMARK(BM_JobDependencies)
    ->Args({1, 10000})    // Independent jobs
    ->Args({5, 2000})     // Short chains
    ->Args({20, 500})     // Medium chains
    ->Args({100, 100})    // Long chains
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark typed thread pool priority impact
 */
static void BM_PriorityImpact(benchmark::State& state) {
    using Priority = int;
    constexpr Priority Critical = 1;
    constexpr Priority High = 10;
    constexpr Priority Normal = 50;
    constexpr Priority Low = 100;
    constexpr Priority Background = 1000;
    
    const size_t jobs_per_priority = state.range(0);
    const size_t worker_count = std::thread::hardware_concurrency();
    
    for (auto _ : state) {
        auto [pool, error] = create_priority_default<Priority>(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create typed thread pool");
            return;
        }
        
        pool->start();
        
        std::map<Priority, std::atomic<size_t>> completed;
        for (auto p : {Critical, High, Normal, Low, Background}) {
            completed[p] = 0;
        }
        
        // Submit jobs with different priorities
        for (size_t i = 0; i < jobs_per_priority; ++i) {
            for (auto priority : {Critical, High, Normal, Low, Background}) {
                pool->enqueue(std::make_unique<typed_thread_pool_module::callback_typed_job_t<Priority>>([&completed, priority]() -> result_void {
                    execute_job_with_complexity(JobComplexity::Light);
                    completed[priority].fetch_add(1);
                    return result_void();
                }, priority));
            }
        }
        
        pool->stop();
        
        // Report completion counts by priority
        state.counters["critical"] = completed[Critical].load();
        state.counters["high"] = completed[High].load();
        state.counters["normal"] = completed[Normal].load();
        state.counters["low"] = completed[Low].load();
        state.counters["background"] = completed[Background].load();
    }
    
    state.SetItemsProcessed(state.iterations() * jobs_per_priority * 5);
}
BENCHMARK(BM_PriorityImpact)
    ->Arg(2000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark mixed workload throughput
 */
static void BM_MixedWorkload(benchmark::State& state) {
    const double cpu_light_pct = state.range(0);
    const double cpu_heavy_pct = state.range(1);
    const double io_pct = state.range(2);
    const double memory_pct = 100.0 - cpu_light_pct - cpu_heavy_pct - io_pct;
    
    const size_t total_jobs = 10000;
    const size_t worker_count = std::thread::hardware_concurrency();
    
    auto generate_job = [=]() -> std::function<void()> {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 100.0);
        
        double roll = dis(gen);
        
        if (roll < cpu_light_pct) {
            return [] { execute_job_with_complexity(JobComplexity::Light); };
        } else if (roll < cpu_light_pct + cpu_heavy_pct) {
            return [] { execute_job_with_complexity(JobComplexity::Heavy); };
        } else if (roll < cpu_light_pct + cpu_heavy_pct + io_pct) {
            return [] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); };
        } else {
            return [] { 
                auto buffer = allocate_with_pattern(MemoryPattern::Medium);
                execute_job_with_complexity(JobComplexity::Light);
            };
        }
    };
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        for (size_t i = 0; i < total_jobs; ++i) {
            auto job = generate_job();
            pool->enqueue(std::make_unique<callback_job>([job]() -> result_void {
                job();
                return result_void();
            }));
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * total_jobs);
    state.counters["cpu_light%"] = cpu_light_pct;
    state.counters["cpu_heavy%"] = cpu_heavy_pct;
    state.counters["io%"] = io_pct;
    state.counters["memory%"] = memory_pct;
}
// Test different workload mixes
BENCHMARK(BM_MixedWorkload)
    ->Args({100, 0, 0})      // CPU only (light)
    ->Args({0, 100, 0})      // CPU only (heavy)
    ->Args({0, 0, 100})      // I/O only
    ->Args({25, 25, 25})     // Balanced
    ->Args({60, 10, 25})     // Web server
    ->Args({20, 50, 10})     // Data processing
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark sustained throughput over time
 */
static void BM_SustainedThroughput(benchmark::State& state) {
    const int duration_seconds = state.range(0);
    const size_t worker_count = std::thread::hardware_concurrency();
    
    for (auto _ : state) {
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> jobs_submitted{0};
        std::atomic<size_t> jobs_completed{0};
        std::atomic<bool> running{true};
        
        // Job submission thread
        std::thread submitter([&pool, &jobs_submitted, &running, &jobs_completed]() {
            while (running.load()) {
                pool->enqueue(std::make_unique<callback_job>([&jobs_completed]() -> result_void {
                    execute_job_with_complexity(JobComplexity::Medium);
                    jobs_completed.fetch_add(1);
                    return result_void();
                }));
                jobs_submitted.fetch_add(1);
                
                // Small delay to prevent overwhelming
                if (jobs_submitted.load() % 1000 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
        });
        
        // Run for specified duration
        std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));
        
        running = false;
        submitter.join();
        pool->stop();
        
        state.counters["jobs_completed"] = jobs_completed.load();
        state.counters["jobs_per_second"] = jobs_completed.load() / duration_seconds;
    }
    
    state.SetIterationTime(duration_seconds);
}
BENCHMARK(BM_SustainedThroughput)
    ->Arg(5)   // 5 second test
    ->Arg(10)  // 10 second test
    ->UseManualTime()
    ->Unit(benchmark::kSecond);

// Main function to run benchmarks
BENCHMARK_MAIN();