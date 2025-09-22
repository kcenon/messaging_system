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

/**
 * @file data_race_benchmark.cpp
 * @brief Performance benchmark to measure the impact of data race fixes
 * 
 * This benchmark measures the performance before and after fixing data race conditions:
 * 1. wake_interval synchronization in thread_base
 * 2. cancellation_token double-check pattern fix
 * 3. job_queue consistency improvements
 */

#include "../sources/thread_base/core/thread_base.h"
#include "../sources/thread_base/jobs/job_queue.h"
#include "../sources/thread_base/sync/cancellation_token.h"
#include "../sources/thread_pool/core/thread_pool.h"
#include "../sources/utilities/core/formatter.h"

#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>

using namespace kcenon::thread;
using namespace kcenon::thread;

// Test worker that frequently accesses wake_interval
class WakeIntervalTestWorker : public thread_base {
public:
    WakeIntervalTestWorker() : thread_base("wake_interval_test"), access_count_(0) {}
    
    std::atomic<size_t> access_count_;
    
protected:
    auto do_work() -> result_void override {
        // Simulate frequent wake_interval access
        for (int i = 0; i < 100; ++i) {
            auto interval = get_wake_interval_unsafe(); // Direct access without lock
            if (interval.has_value()) {
                access_count_++;
            }
        }
        return {};
    }
    
    auto should_continue_work() const -> bool override {
        return access_count_ < 10000;
    }
    
private:
    // Unsafe method to demonstrate data race
    auto get_wake_interval_unsafe() const -> std::optional<std::chrono::milliseconds> {
        return wake_interval_; // Direct access without synchronization
    }
};

// Benchmark for wake_interval data race scenario
static void BM_WakeIntervalDataRace(benchmark::State& state) {
    for (auto _ : state) {
        auto worker = std::make_unique<WakeIntervalTestWorker>();
        worker->start();
        
        // Multiple threads modifying wake_interval concurrently
        std::vector<std::thread> threads;
        for (int i = 0; i < state.range(0); ++i) {
            threads.emplace_back([&worker, i]() {
                for (int j = 0; j < 1000; ++j) {
                    worker->set_wake_interval(std::chrono::milliseconds(i * 10 + j));
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        worker->stop();
        state.counters["accesses"] = worker->access_count_.load();
    }
}

// Benchmark for cancellation token operations
static void BM_CancellationTokenOperations(benchmark::State& state) {
    for (auto _ : state) {
        auto token = cancellation_token::create();
        std::atomic<size_t> callback_count{0};
        
        // Register many callbacks from multiple threads
        std::vector<std::thread> threads;
        for (int i = 0; i < state.range(0); ++i) {
            threads.emplace_back([&token, &callback_count]() {
                for (int j = 0; j < 100; ++j) {
                    token.register_callback([&callback_count]() {
                        callback_count++;
                    });
                }
            });
        }
        
        // Cancel token while callbacks are being registered
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        token.cancel();
        
        for (auto& t : threads) {
            t.join();
        }
        
        state.counters["callbacks"] = callback_count.load();
    }
}

// Benchmark for job_queue consistency
static void BM_JobQueueConsistency(benchmark::State& state) {
    for (auto _ : state) {
        auto queue = std::make_shared<job_queue>();
        std::atomic<size_t> enqueue_count{0};
        std::atomic<size_t> dequeue_count{0};
        std::atomic<size_t> size_checks{0};
        
        // Multiple threads enqueuing/dequeuing/checking size
        std::vector<std::thread> threads;
        
        // Enqueue threads
        for (int i = 0; i < state.range(0) / 2; ++i) {
            threads.emplace_back([&queue, &enqueue_count]() {
                for (int j = 0; j < 1000; ++j) {
                    auto job = std::make_unique<callback_job>([]() { return result_void{}; });
                    auto result = queue->enqueue(std::move(job));
                    if (!result) {
                        enqueue_count++;
                    }
                }
            });
        }
        
        // Dequeue threads
        for (int i = 0; i < state.range(0) / 2; ++i) {
            threads.emplace_back([&queue, &dequeue_count]() {
                for (int j = 0; j < 1000; ++j) {
                    auto job = queue->dequeue();
                    if (job) {
                        dequeue_count++;
                    }
                }
            });
        }
        
        // Size check thread
        threads.emplace_back([&queue, &size_checks]() {
            while (size_checks < 10000) {
                auto size = queue->size();
                auto empty = queue->empty();
                // Check consistency
                if ((size == 0 && !empty) || (size > 0 && empty)) {
                    // Inconsistency detected
                    break;
                }
                size_checks++;
            }
        });
        
        for (auto& t : threads) {
            t.join();
        }
        
        state.counters["enqueued"] = enqueue_count.load();
        state.counters["dequeued"] = dequeue_count.load();
        state.counters["size_checks"] = size_checks.load();
    }
}

// Thread pool stress test with data races
static void BM_ThreadPoolStress(benchmark::State& state) {
    for (auto _ : state) {
        auto pool = std::make_shared<thread_pool>();
        pool->start();
        
        std::atomic<size_t> completed_jobs{0};
        std::atomic<size_t> failed_jobs{0};
        
        // Submit many jobs from multiple threads
        std::vector<std::thread> submitters;
        for (int i = 0; i < 4; ++i) {
            submitters.emplace_back([&pool, &completed_jobs, &failed_jobs]() {
                for (int j = 0; j < 2500; ++j) {
                    auto result = pool->enqueue(std::make_unique<callback_job>(
                        [&completed_jobs]() {
                            completed_jobs++;
                            return result_void{};
                        }
                    ));
                    if (result.has_error()) {
                        failed_jobs++;
                    }
                }
            });
        }
        
        for (auto& t : submitters) {
            t.join();
        }
        
        // Wait for all jobs to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        pool->stop();
        
        state.counters["completed"] = completed_jobs.load();
        state.counters["failed"] = failed_jobs.load();
    }
}

// Register benchmarks with different thread counts
BENCHMARK(BM_WakeIntervalDataRace)->Arg(1)->Arg(4)->Arg(8)->Arg(16);
BENCHMARK(BM_CancellationTokenOperations)->Arg(1)->Arg(4)->Arg(8)->Arg(16);
BENCHMARK(BM_JobQueueConsistency)->Arg(2)->Arg(4)->Arg(8)->Arg(16);
BENCHMARK(BM_ThreadPoolStress)->Arg(2)->Arg(4)->Arg(8)->Arg(16);

BENCHMARK_MAIN();