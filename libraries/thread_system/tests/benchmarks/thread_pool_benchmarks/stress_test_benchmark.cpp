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
 * @file stress_test_benchmark.cpp
 * @brief Stress tests and edge case benchmarks for Thread System
 * 
 * Tests extreme conditions:
 * - Maximum load scenarios
 * - Resource exhaustion
 * - Error recovery
 * - Edge cases
 */

#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <atomic>
#include <random>
#include <thread>
#include <future>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <numeric>

#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/typed_thread_pool/pool/typed_thread_pool.h"
#include "../../sources/typed_thread_pool/scheduling/typed_thread_worker.h"
#include "../../sources/typed_thread_pool/jobs/callback_typed_job.h"
#include "../../sources/utilities/core/formatter.h"
// Helper function to create thread pool
auto create_default(const uint16_t& worker_counts)
    -> std::tuple<std::shared_ptr<kcenon::thread::thread_pool>, std::optional<std::string>>
{
    std::shared_ptr<kcenon::thread::thread_pool> pool;
    try {
        pool = std::make_shared<kcenon::thread::thread_pool>();
    } catch (const std::bad_alloc& e) {
        return { nullptr, std::string(e.what()) };
    }
    
    std::optional<std::string> error_message = std::nullopt;
    std::vector<std::unique_ptr<kcenon::thread::thread_worker>> workers;
    workers.reserve(worker_counts);
    for (uint16_t i = 0; i < worker_counts; ++i) {
        workers.push_back(std::make_unique<kcenon::thread::thread_worker>());
    }
    
    error_message = pool->enqueue_batch(std::move(workers));
    if (error_message.has_value()) {
        return { nullptr, formatter::format("cannot enqueue to workers: {}", 
                                           error_message.value_or("unknown error")) };
    }
    
    return { pool, std::nullopt };
}

// Helper function to create typed thread pool
template<typename Type>
auto create_priority_default(const uint16_t& worker_counts)
    -> std::tuple<std::shared_ptr<typed_kcenon::thread::typed_thread_pool_t<Type>>, std::optional<std::string>>
{
    std::shared_ptr<typed_kcenon::thread::typed_thread_pool_t<Type>> pool;
    try {
        pool = std::make_shared<typed_kcenon::thread::typed_thread_pool_t<Type>>();
    } catch (const std::bad_alloc& e) {
        return { nullptr, std::string(e.what()) };
    }
    
    std::vector<std::unique_ptr<typed_kcenon::thread::typed_thread_worker_t<Type>>> workers;
    workers.reserve(worker_counts);
    for (uint16_t i = 0; i < worker_counts; ++i) {
        workers.push_back(std::make_unique<typed_kcenon::thread::typed_thread_worker_t<Type>>(
            std::vector<Type>{}, true));
    }
    
    auto enqueue_result = pool->enqueue_batch(std::move(workers));
    if (enqueue_result.has_error()) {
        return { nullptr, formatter::format("cannot enqueue to workers: {}", 
                                           enqueue_result.get_error().message()) };
    }
    
    return { pool, std::nullopt };
}

using namespace kcenon::thread;
using namespace kcenon::thread;
using namespace utility_module;

// Forward declare Type enum before using it
enum class Type { 
    Highest = 1,
    High = 10,
    Medium = 50,
    Low = 100,
    Lowest = 1000
};

// Add formatter for Type enum when using std::format
#ifdef USE_STD_FORMAT
template<>
struct std::formatter<Type> : std::formatter<int> {
    auto format(Type t, std::format_context& ctx) const {
        return std::formatter<int>::format(static_cast<int>(t), ctx);
    }
};
#endif

/**
 * @brief Benchmark maximum thread creation
 */
static void BM_MaximumThreads(benchmark::State& state) {
    const size_t thread_count = state.range(0);
    
    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto [pool, error] = create_default(thread_count);
        if (error.has_value()) {
            state.SkipWithError(formatter::format("Failed to create pool with {} threads: {}", 
                                                 thread_count, *error).c_str());
            return;
        }
        
        auto result = pool->start();
        if (result.has_value()) {
            state.SkipWithError(formatter::format("Failed to start pool: {}", 
                                                 result.value()).c_str());
            return;
        }
        
        auto creation_end = std::chrono::high_resolution_clock::now();
        
        // Test basic functionality
        std::atomic<size_t> completed{0};
        const size_t test_jobs = 1000;
        
        for (size_t i = 0; i < test_jobs; ++i) {
            pool->enqueue(std::make_unique<callback_job>([&completed]() -> result_void {
                completed.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
        
        state.counters["creation_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(creation_end - start).count();
        state.counters["jobs_completed"] = completed.load();
    }
    
    state.counters["threads"] = thread_count;
}
BENCHMARK(BM_MaximumThreads)
    ->Arg(100)
    ->Arg(500)
    ->Arg(1000)
    ->Arg(2000)
    ->Unit(benchmark::kMillisecond);
/**
 * @brief Benchmark queue overflow handling
 */
static void BM_QueueOverflow(benchmark::State& state) {
    const size_t flood_size = state.range(0);
    
    auto [pool, error] = create_default(4);
    if (error.has_value()) {
        state.SkipWithError("Failed to create thread pool");
        return;
    }
    
    pool->start();
    
    // Submit jobs that take time to process
    const size_t slow_jobs = 100;
    for (size_t i = 0; i < slow_jobs; ++i) {
        pool->enqueue(std::make_unique<callback_job>([]() -> result_void {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            return result_void();
        }));
    }
    
    for (auto _ : state) {
        // Flood with many quick jobs
        try {
            for (size_t i = 0; i < flood_size; ++i) {
                pool->enqueue(std::make_unique<callback_job>([]() -> result_void {
                    // Quick job
                    return result_void();
                }));
            }
        } catch (const std::exception& e) {
            state.SkipWithError(formatter::format("Queue overflow at {} jobs: {}", 
                                                 flood_size, e.what()).c_str());
            break;
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * flood_size);
    state.counters["flood_size"] = flood_size;
}
BENCHMARK(BM_QueueOverflow)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark rapid start/stop cycles
 */
static void BM_RapidStartStop(benchmark::State& state) {
    const size_t num_cycles = state.range(0);
    
    auto [pool, error] = create_default(8);
    if (error.has_value()) {
        state.SkipWithError("Failed to create thread pool");
        return;
    }
    
    std::vector<double> cycle_times;
    cycle_times.reserve(num_cycles);
    
    for (auto _ : state) {
        size_t successful_cycles = 0;
        
        for (size_t i = 0; i < num_cycles; ++i) {
            auto cycle_start = std::chrono::high_resolution_clock::now();
            
            auto start_result = pool->start();
            if (start_result.has_value()) {
                state.SkipWithError(formatter::format("Start failed at cycle {}: {}", 
                                                     i, start_result.value()).c_str());
                break;
            }
            
            // Submit a few jobs
            std::atomic<int> counter{0};
            for (int j = 0; j < 10; ++j) {
                pool->enqueue(std::make_unique<callback_job>([&counter]() -> result_void {
                    counter.fetch_add(1);
                    return result_void();
                }));
            }
            
            pool->stop();
            
            auto cycle_end = std::chrono::high_resolution_clock::now();
            double cycle_time_us = std::chrono::duration_cast<std::chrono::microseconds>(cycle_end - cycle_start).count();
            cycle_times.push_back(cycle_time_us);
            
            successful_cycles++;
        }
        
        state.counters["successful_cycles"] = successful_cycles;
    }
    
    if (!cycle_times.empty()) {
        double avg_cycle_time = std::accumulate(cycle_times.begin(), cycle_times.end(), 0.0) 
                              / cycle_times.size();
        auto [min_it, max_it] = std::minmax_element(cycle_times.begin(), cycle_times.end());
        
        state.counters["avg_cycle_us"] = avg_cycle_time;
        state.counters["min_cycle_us"] = *min_it;
        state.counters["max_cycle_us"] = *max_it;
    }
}
BENCHMARK(BM_RapidStartStop)
    ->Arg(100)
    ->Arg(1000)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark exception handling under load
 */
static void BM_ExceptionHandling(benchmark::State& state) {
    const size_t total_jobs = state.range(0);
    const double exception_rate = 0.1;  // 10% of jobs throw exceptions
    
    for (auto _ : state) {
        auto [pool, error] = create_default(8);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> successful_jobs{0};
        std::atomic<size_t> failed_jobs{0};
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (size_t i = 0; i < total_jobs; ++i) {
            pool->enqueue(std::make_unique<callback_job>([&dis, &gen, &successful_jobs, &failed_jobs, exception_rate]() 
                         -> result_void {
                if (dis(gen) < exception_rate) {
                    failed_jobs.fetch_add(1);
                    return result_void(thread_module::error{thread_module::error_code::job_execution_failed, "Simulated job failure"});
                }
                
                // Simulate some work
                volatile int sum = 0;
                for (int j = 0; j < 1000; ++j) {
                    sum += j;
                }
                
                successful_jobs.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
        
        state.counters["successful"] = successful_jobs.load();
        state.counters["failed"] = failed_jobs.load();
    }
    
    state.SetItemsProcessed(state.iterations() * total_jobs);
}
BENCHMARK(BM_ExceptionHandling)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark memory pressure with large captures
 */
static void BM_MemoryPressure(benchmark::State& state) {
    const size_t size_mb = state.range(0);
    const size_t num_jobs = state.range(1);
    
    auto [pool, error] = create_default(8);
    if (error.has_value()) {
        state.SkipWithError("Failed to create thread pool");
        return;
    }
    
    pool->start();
    
    for (auto _ : state) {
        std::atomic<size_t> completed{0};
        
        try {
            for (size_t i = 0; i < num_jobs; ++i) {
                // Create large data to capture
                std::vector<char> large_data(size_mb * 1024 * 1024);
                std::fill(large_data.begin(), large_data.end(), 'X');
                
                pool->enqueue(std::make_unique<callback_job>([data = std::move(large_data), &completed]() -> result_void {
                    // Access data to ensure it's not optimized away
                    volatile char c = data[data.size() / 2];
                    (void)c;
                    
                    completed.fetch_add(1);
                    return result_void();
                }));
            }
            
            pool->stop();
            pool->start();  // Reset for next iteration
            
            state.counters["completed"] = completed.load();
            
        } catch (const std::bad_alloc&) {
            state.SkipWithError(formatter::format("Out of memory with {}MB per job", 
                                                 size_mb).c_str());
            break;
        }
    }
    
    state.counters["mb_per_job"] = size_mb;
    state.counters["total_mb"] = size_mb * num_jobs;
}
BENCHMARK(BM_MemoryPressure)
    ->Args({1, 100})
    ->Args({10, 100})
    ->Args({50, 20})
    ->Args({100, 10})
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark priority starvation
 */
static void BM_PriorityStarvation(benchmark::State& state) {
    const size_t jobs_per_priority = state.range(0);
    
    for (auto _ : state) {
        auto [pool, error] = create_priority_default<Type>(4);
        if (error.has_value()) {
            state.SkipWithError("Failed to create typed thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> highest_completed{0};
        std::atomic<size_t> high_completed{0};
        std::atomic<size_t> medium_completed{0};
        std::atomic<size_t> low_completed{0};
        std::atomic<size_t> lowest_completed{0};
        
        // Submit all jobs
        for (size_t i = 0; i < jobs_per_priority; ++i) {
            pool->enqueue(std::make_unique<callback_typed_job_t<Type>>([&highest_completed]() -> result_void {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                highest_completed.fetch_add(1);
                return result_void();
            }, Type::Highest));
            
            pool->enqueue(std::make_unique<callback_typed_job_t<Type>>([&high_completed]() -> result_void {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                high_completed.fetch_add(1);
                return result_void();
            }, Type::High));
            
            pool->enqueue(std::make_unique<callback_typed_job_t<Type>>([&medium_completed]() -> result_void {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                medium_completed.fetch_add(1);
                return result_void();
            }, Type::Medium));
            
            pool->enqueue(std::make_unique<callback_typed_job_t<Type>>([&low_completed]() -> result_void {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                low_completed.fetch_add(1);
                return result_void();
            }, Type::Low));
            
            pool->enqueue(std::make_unique<callback_typed_job_t<Type>>([&lowest_completed]() -> result_void {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                lowest_completed.fetch_add(1);
                return result_void();
            }, Type::Lowest));
        }
        
        // Let some jobs complete
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        pool->stop();
        
        state.counters["highest"] = highest_completed.load();
        state.counters["high"] = high_completed.load();
        state.counters["medium"] = medium_completed.load();
        state.counters["low"] = low_completed.load();
        state.counters["lowest"] = lowest_completed.load();
        
        // Check for starvation
        if (highest_completed.load() == jobs_per_priority &&
            high_completed.load() == jobs_per_priority &&
            lowest_completed.load() == 0) {
            state.counters["lowest_starved"] = 1;
        }
    }
    
    state.counters["jobs_per_priority"] = jobs_per_priority;
}
BENCHMARK(BM_PriorityStarvation)
    ->Arg(1000)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
    
/**
 * @brief Benchmark thundering herd problem
 */
static void BM_ThunderingHerd(benchmark::State& state) {
    const size_t num_waiters = state.range(0);
    
    for (auto _ : state) {
        auto [pool, error] = create_default(8);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::promise<void> start_signal;
        auto start_future = start_signal.get_future();
        
        std::atomic<size_t> started{0};
        std::atomic<size_t> completed{0};
        
        // Create many jobs that all wait for the same signal
        for (size_t i = 0; i < num_waiters; ++i) {
            pool->enqueue(std::make_unique<callback_job>([&start_future, &started, &completed]() -> result_void {
                // Wait for signal
                start_future.wait();
                started.fetch_add(1);
                
                // Simulate work
                volatile int sum = 0;
                for (int j = 0; j < 10000; ++j) {
                    sum += j;
                }
                
                completed.fetch_add(1);
                return result_void();
            }));
        }
        
        // Give jobs time to queue up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Release the herd
        auto release_time = std::chrono::high_resolution_clock::now();
        start_signal.set_value();
        
        // Monitor progress
        std::vector<std::pair<size_t, size_t>> progress;
        
        for (int i = 0; i < 50; ++i) {  // Monitor for 500ms
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            progress.push_back({started.load(), completed.load()});
        }
        
        pool->stop();
        
        // Find time to start various percentages
        size_t thresholds[] = {100, 500, 900, 950, 990, 1000};
        
        for (size_t threshold : thresholds) {
            if (threshold > num_waiters) continue;
            
            auto it = std::find_if(progress.begin(), progress.end(),
                                  [threshold](const auto& p) { 
                                      return p.first >= threshold; 
                                  });
            
            if (it != progress.end()) {
                size_t time_ms = (it - progress.begin()) * 10;
                state.counters[formatter::format("start_{}jobs_ms", threshold)] = time_ms;
            }
        }
    }
    
    state.counters["waiters"] = num_waiters;
}
BENCHMARK(BM_ThunderingHerd)
    ->Arg(1000)
    ->Arg(10000)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
    
/**
 * @brief Benchmark cascading failures
 */
static void BM_CascadingFailures(benchmark::State& state) {
    const size_t chain_length = state.range(0);
    const size_t num_chains = state.range(1);
    
    for (auto _ : state) {
        auto [pool, error] = create_default(8);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> successful_chains{0};
        std::atomic<size_t> failed_chains{0};
        
        for (size_t chain = 0; chain < num_chains; ++chain) {
            // Decide if this chain will fail
            bool will_fail = (chain % 3 == 0);  // Every 3rd chain fails
            
            std::vector<std::promise<bool>> promises(chain_length);
            std::vector<std::future<bool>> futures;
            
            for (auto& promise : promises) {
                futures.push_back(promise.get_future());
            }
            
            // Submit chain of dependent tasks
            for (size_t i = 0; i < chain_length; ++i) {
                pool->enqueue(std::make_unique<callback_job>([i, &futures, &promises, will_fail, chain_length, 
                              &successful_chains, &failed_chains]() -> result_void {
                    // Wait for previous task (except first)
                    if (i > 0) {
                        try {
                            bool prev_success = futures[i-1].get();
                            if (!prev_success) {
                                // Previous task failed, propagate failure
                                promises[i].set_value(false);
                                
                                // If this is the last task, count the failed chain
                                if (i == chain_length - 1) {
                                    failed_chains.fetch_add(1);
                                }
                                return result_void();
                            }
                        } catch (...) {
                            promises[i].set_value(false);
                            return result_void();
                        }
                    }
                    
                    // Simulate work
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    
                    // Inject failure at middle of chain
                    if (will_fail && i == chain_length / 2) {
                        promises[i].set_value(false);
                    } else {
                        promises[i].set_value(true);
                        
                        // If this is the last task and we succeeded, count it
                        if (i == chain_length - 1) {
                            successful_chains.fetch_add(1);
                        }
                    }
                    return result_void();
                }));
            }
        }
        
        pool->stop();
        
        state.counters["successful_chains"] = successful_chains.load();
        state.counters["failed_chains"] = failed_chains.load();
        state.counters["failure_rate_%"] = failed_chains.load() * 100.0 / num_chains;
    }
    
    state.counters["chain_length"] = chain_length;
    state.counters["num_chains"] = num_chains;
}
BENCHMARK(BM_CascadingFailures)
    ->Args({100, 10})
    ->Args({50, 20})
    ->Unit(benchmark::kMillisecond);
    
// Main function to run benchmarks
BENCHMARK_MAIN();