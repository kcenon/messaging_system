/*
 * BSD 3-Clause License
 * 
 * Copyright (c) 2025, DongCheol Shin
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
 * @file gbench_thread_pool.cpp
 * @brief Google Benchmark tests for thread pool
 */

#include <benchmark/benchmark.h>
#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/thread_base/jobs/callback_job.h"
#include <atomic>
#include <memory>
#include <vector>

using namespace kcenon::thread;
using namespace kcenon::thread;

// Helper function to create thread pool with workers
static std::shared_ptr<thread_pool> create_pool_with_workers(size_t worker_count) {
    auto pool = std::make_shared<thread_pool>("benchmark_pool");
    
    std::vector<std::unique_ptr<thread_worker>> workers;
    workers.reserve(worker_count);
    for (size_t i = 0; i < worker_count; ++i) {
        workers.push_back(std::make_unique<thread_worker>());
    }
    
    pool->enqueue_batch(std::move(workers));
    return pool;
}

// Benchmark thread pool creation
static void BM_ThreadPoolCreation(benchmark::State& state) {
    const size_t worker_count = state.range(0);
    
    for (auto _ : state) {
        auto pool = create_pool_with_workers(worker_count);
        benchmark::DoNotOptimize(pool);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ThreadPoolCreation)->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16);

// Benchmark empty job submission
static void BM_EmptyJobSubmission(benchmark::State& state) {
    const size_t worker_count = state.range(0);
    auto pool = create_pool_with_workers(worker_count);
    pool->start();
    
    for (auto _ : state) {
        auto job = std::make_unique<callback_job>([]() -> result_void {
            return result_void();
        });
        auto result = pool->enqueue(std::move(job));
        benchmark::DoNotOptimize(result);
    }
    
    pool->stop();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_EmptyJobSubmission)->Arg(1)->Arg(2)->Arg(4)->Arg(8);

// Benchmark job throughput
static void BM_JobThroughput(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t job_duration_us = state.range(1);
    
    auto pool = create_pool_with_workers(num_workers);
    pool->start();
    
    std::atomic<size_t> jobs_completed{0};
    
    for (auto _ : state) {
        auto job = std::make_unique<callback_job>([&jobs_completed, job_duration_us]() -> result_void {
            if (job_duration_us > 0) {
                auto start = std::chrono::high_resolution_clock::now();
                while (std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - start).count() < job_duration_us) {
                    // Busy wait
                }
            }
            jobs_completed.fetch_add(1);
            return result_void();
        });
        pool->enqueue(std::move(job));
    }
    
    // Wait for all jobs to complete
    pool->stop();
    
    state.SetItemsProcessed(jobs_completed.load());
    state.counters["jobs/sec"] = benchmark::Counter(jobs_completed.load(), benchmark::Counter::kIsRate);
}
// Workers x Job duration (microseconds)
BENCHMARK(BM_JobThroughput)->Args({1, 0})->Args({2, 0})->Args({4, 0})->Args({8, 0})
                           ->Args({8, 1})->Args({8, 10})->Args({8, 100})->Args({8, 1000});

// Benchmark batch job submission
static void BM_BatchJobSubmission(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t batch_size = state.range(1);
    
    auto pool = create_pool_with_workers(num_workers);
    pool->start();
    
    for (auto _ : state) {
        std::vector<std::unique_ptr<job>> batch;
        batch.reserve(batch_size);
        
        for (size_t i = 0; i < batch_size; ++i) {
            batch.push_back(std::make_unique<callback_job>([]() -> result_void {
                return result_void();
            }));
        }
        
        auto result = pool->enqueue_batch(std::move(batch));
        benchmark::DoNotOptimize(result);
    }
    
    pool->stop();
    state.SetItemsProcessed(state.iterations() * batch_size);
}
// Workers x Batch size
BENCHMARK(BM_BatchJobSubmission)->Args({4, 10})->Args({4, 100})->Args({4, 1000})
                                ->Args({8, 10})->Args({8, 100})->Args({8, 1000});

// Benchmark scaling efficiency
static void BM_ScalingEfficiency(benchmark::State& state) {
    const size_t num_workers = state.range(0);
    const size_t total_jobs = 10000;
    
    auto pool = create_pool_with_workers(num_workers);
    pool->start();
    
    std::atomic<size_t> jobs_completed{0};
    
    for (auto _ : state) {
        state.PauseTiming();
        jobs_completed = 0;
        
        // Submit all jobs
        for (size_t i = 0; i < total_jobs; ++i) {
            pool->enqueue(std::make_unique<callback_job>([&jobs_completed]() -> result_void {
                // Simulate some work
                volatile int sum = 0;
                for (int j = 0; j < 1000; ++j) {
                    sum += j;
                }
                jobs_completed.fetch_add(1);
                return result_void();
            }));
        }
        state.ResumeTiming();
        
        // Wait for completion
        while (jobs_completed.load() < total_jobs) {
            std::this_thread::yield();
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * total_jobs);
    
    // Calculate scaling efficiency compared to single thread
    if (num_workers > 1) {
        state.counters["workers"] = num_workers;
        state.counters["jobs_per_worker"] = total_jobs / num_workers;
    }
}
BENCHMARK(BM_ScalingEfficiency)->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16);

BENCHMARK_MAIN();