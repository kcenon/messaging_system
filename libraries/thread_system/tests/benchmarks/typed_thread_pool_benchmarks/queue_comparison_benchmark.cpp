/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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
#include "typed_thread_pool/scheduling/typed_lockfree_job_queue.h"
#include "typed_thread_pool/scheduling/typed_job_queue.h"
#include "typed_thread_pool/jobs/callback_typed_job.h"
#include "thread_base/lockfree/queues/lockfree_job_queue.h"
#include "thread_base/jobs/job_queue.h"

using namespace kcenon::thread;
using namespace kcenon::thread;

// Simple job for benchmarking
class simple_test_job : public typed_job_t<job_types> {
public:
    simple_test_job(job_types type) : typed_job_t<job_types>(type) {}
    
    result_void do_work() override {
        // Minimal work
        volatile int x = 42;
        x++;
        return {};
    }
};

// Queue enqueue/dequeue benchmarks
static void BM_TypedJobQueue_Enqueue(benchmark::State& state) {
    typed_job_queue queue;
    
    for (auto _ : state) {
        auto job = std::make_unique<simple_test_job>(job_types::Batch);
        benchmark::DoNotOptimize(queue.enqueue(std::move(job)));
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("mutex-based");
}

static void BM_TypedLockfreeJobQueue_Enqueue(benchmark::State& state) {
    typed_lockfree_job_queue queue;
    
    for (auto _ : state) {
        auto job = std::make_unique<simple_test_job>(job_types::Batch);
        benchmark::DoNotOptimize(queue.enqueue(std::move(job)));
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("lock-free");
}

static void BM_TypedJobQueue_EnqueueDequeue(benchmark::State& state) {
    typed_job_queue queue;
    
    // Pre-populate queue
    for (int i = 0; i < 1000; ++i) {
        auto job = std::make_unique<simple_test_job>(static_cast<job_types>(i % 3));
        queue.enqueue(std::move(job));
    }
    
    for (auto _ : state) {
        // Enqueue
        auto job = std::make_unique<simple_test_job>(job_types::RealTime);
        queue.enqueue(std::move(job));
        
        // Dequeue
        auto result = queue.dequeue();
        benchmark::DoNotOptimize(result);
        
        if (result.has_value()) {
            benchmark::DoNotOptimize(result.value());
        }
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("mutex-based");
}

static void BM_TypedLockfreeJobQueue_EnqueueDequeue(benchmark::State& state) {
    typed_lockfree_job_queue queue;
    
    // Pre-populate queue
    for (int i = 0; i < 1000; ++i) {
        auto job = std::make_unique<simple_test_job>(static_cast<job_types>(i % 3));
        queue.enqueue(std::unique_ptr<thread_module::job>(std::move(job)));
    }
    
    for (auto _ : state) {
        // Enqueue
        auto job = std::make_unique<simple_test_job>(job_types::RealTime);
        queue.enqueue(std::unique_ptr<thread_module::job>(std::move(job)));
        
        // Dequeue
        auto result = queue.dequeue();
        benchmark::DoNotOptimize(result);
        
        if (result.has_value()) {
            benchmark::DoNotOptimize(result.value());
        }
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("lock-free");
}

// Batch operations benchmarks
static void BM_TypedJobQueue_BatchEnqueue(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    typed_job_queue queue;
    
    for (auto _ : state) {
        std::vector<std::unique_ptr<typed_job>> jobs;
        jobs.reserve(batch_size);
        
        for (size_t i = 0; i < batch_size; ++i) {
            jobs.push_back(std::make_unique<simple_test_job>(
                static_cast<job_types>(i % 3)));
        }
        
        benchmark::DoNotOptimize(queue.enqueue_batch(std::move(jobs)));
    }
    
    state.SetItemsProcessed(state.iterations() * batch_size);
    state.SetLabel("mutex-based");
}

static void BM_TypedLockfreeJobQueue_BatchEnqueue(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    typed_lockfree_job_queue queue;
    
    for (auto _ : state) {
        std::vector<std::unique_ptr<thread_module::job>> jobs;
        jobs.reserve(batch_size);
        
        for (size_t i = 0; i < batch_size; ++i) {
            jobs.push_back(std::make_unique<simple_test_job>(
                static_cast<job_types>(i % 3)));
        }
        
        benchmark::DoNotOptimize(queue.enqueue_batch(std::move(jobs)));
    }
    
    state.SetItemsProcessed(state.iterations() * batch_size);
    state.SetLabel("lock-free");
}

// Contention benchmarks
static void BM_TypedJobQueue_HighContention(benchmark::State& state) {
    const size_t thread_count = state.range(0);
    typed_job_queue queue;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<bool> start_flag{false};
        
        for (size_t t = 0; t < thread_count; ++t) {
            threads.emplace_back([&, t]() {
                while (!start_flag.load()) {
                    std::this_thread::yield();
                }
                
                for (int i = 0; i < 100; ++i) {
                    auto job = std::make_unique<simple_test_job>(
                        static_cast<job_types>((t + i) % 3));
                    queue.enqueue(std::move(job));
                    
                    auto result = queue.dequeue();
                    if (result.has_value()) {
                        benchmark::DoNotOptimize(result.value());
                    }
                }
            });
        }
        
        start_flag.store(true);
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * thread_count * 100);
    state.SetLabel("mutex-based");
}

static void BM_TypedLockfreeJobQueue_HighContention(benchmark::State& state) {
    const size_t thread_count = state.range(0);
    typed_lockfree_job_queue queue;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<bool> start_flag{false};
        
        for (size_t t = 0; t < thread_count; ++t) {
            threads.emplace_back([&, t]() {
                while (!start_flag.load()) {
                    std::this_thread::yield();
                }
                
                for (int i = 0; i < 100; ++i) {
                    auto job = std::make_unique<simple_test_job>(
                        static_cast<job_types>((t + i) % 3));
                    queue.enqueue(std::unique_ptr<thread_module::job>(std::move(job)));
                    
                    auto result = queue.dequeue();
                    if (result.has_value()) {
                        benchmark::DoNotOptimize(result.value());
                    }
                }
            });
        }
        
        start_flag.store(true);
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * thread_count * 100);
    state.SetLabel("lock-free");
}

// Priority-specific benchmarks
static void BM_TypedLockfreeJobQueue_PriorityDequeue(benchmark::State& state) {
    typed_lockfree_job_queue queue;
    
    // Pre-populate with jobs of different priorities
    for (int i = 0; i < 1000; ++i) {
        for (int p = 0; p < 3; ++p) {
            auto job = std::make_unique<simple_test_job>(static_cast<job_types>(p));
            queue.enqueue(std::unique_ptr<thread_module::job>(std::move(job)));
        }
    }
    
    for (auto _ : state) {
        // Dequeue high priority job
        auto result = queue.dequeue(job_types::RealTime);
        benchmark::DoNotOptimize(result);
        
        if (result.has_value()) {
            benchmark::DoNotOptimize(result.value());
        }
        
        // Add a new high priority job
        auto job = std::make_unique<simple_test_job>(job_types::RealTime);
        queue.enqueue(std::unique_ptr<thread_module::job>(std::move(job)));
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("priority-dequeue");
}

// Memory usage comparison
static void BM_TypedJobQueue_MemoryUsage(benchmark::State& state) {
    const size_t job_count = state.range(0);
    
    for (auto _ : state) {
        typed_job_queue queue;
        
        // Fill queue
        for (size_t i = 0; i < job_count; ++i) {
            auto job = std::make_unique<simple_test_job>(
                static_cast<job_types>(i % 3));
            queue.enqueue(std::move(job));
        }
        
        benchmark::DoNotOptimize(queue.size());
        
        // Empty queue
        while (!queue.empty()) {
            auto result = queue.dequeue();
            if (result.has_value()) {
                benchmark::DoNotOptimize(result.value());
            }
        }
    }
    
    state.SetItemsProcessed(state.iterations() * job_count);
    state.SetLabel("mutex-based");
}

static void BM_TypedLockfreeJobQueue_MemoryUsage(benchmark::State& state) {
    const size_t job_count = state.range(0);
    
    for (auto _ : state) {
        typed_lockfree_job_queue queue;
        
        // Fill queue
        for (size_t i = 0; i < job_count; ++i) {
            auto job = std::make_unique<simple_test_job>(
                static_cast<job_types>(i % 3));
            queue.enqueue(std::unique_ptr<thread_module::job>(std::move(job)));
        }
        
        benchmark::DoNotOptimize(queue.size());
        
        // Empty queue
        while (!queue.empty()) {
            auto result = queue.dequeue();
            if (result.has_value()) {
                benchmark::DoNotOptimize(result.value());
            }
        }
    }
    
    state.SetItemsProcessed(state.iterations() * job_count);
    state.SetLabel("lock-free");
}

// Register benchmarks
BENCHMARK(BM_TypedJobQueue_Enqueue)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_TypedLockfreeJobQueue_Enqueue)->Unit(benchmark::kNanosecond);

BENCHMARK(BM_TypedJobQueue_EnqueueDequeue)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_TypedLockfreeJobQueue_EnqueueDequeue)->Unit(benchmark::kNanosecond);

BENCHMARK(BM_TypedJobQueue_BatchEnqueue)->Range(8, 1024)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_TypedLockfreeJobQueue_BatchEnqueue)->Range(8, 1024)->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_TypedJobQueue_HighContention)->Range(1, 16)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_TypedLockfreeJobQueue_HighContention)->Range(1, 16)->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_TypedLockfreeJobQueue_PriorityDequeue)->Unit(benchmark::kNanosecond);

BENCHMARK(BM_TypedJobQueue_MemoryUsage)->Range(100, 10000)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_TypedLockfreeJobQueue_MemoryUsage)->Range(100, 10000)->Unit(benchmark::kMicrosecond);

