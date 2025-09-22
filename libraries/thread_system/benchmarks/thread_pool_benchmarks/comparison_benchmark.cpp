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
 * @file comparison_benchmark.cpp
 * @brief Comparative benchmarks against standard library and common patterns
 * 
 * Compares Thread System performance with:
 * - std::async
 * - Raw std::thread
 * - OpenMP (if available)
 * - Custom thread pool implementations
 */

#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <numeric>
#include <algorithm>
#include <iomanip>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/typed_thread_pool/pool/typed_thread_pool.h"
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
    
    std::optional<std::string> error_message = std::nullopt;
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    workers.reserve(worker_counts);
    for (uint16_t i = 0; i < worker_counts; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
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
auto create_priority_default(const uint16_t& worker_counts, const std::vector<Type>& types)
    -> std::tuple<std::shared_ptr<typed_thread_pool_module::typed_thread_pool_t<Type>>, std::optional<std::string>>
{
    std::shared_ptr<typed_thread_pool_module::typed_thread_pool_t<Type>> pool;
    try {
        pool = std::make_shared<typed_thread_pool_module::typed_thread_pool_t<Type>>();
    } catch (const std::bad_alloc& e) {
        return { nullptr, std::string(e.what()) };
    }
    
    std::optional<std::string> error_message = std::nullopt;
    std::vector<std::unique_ptr<typed_thread_pool_module::typed_thread_worker_t<Type>>> workers;
    workers.reserve(worker_counts);
    for (uint16_t i = 0; i < worker_counts; ++i) {
        workers.push_back(std::make_unique<typed_thread_pool_module::typed_thread_worker_t<Type>>(types));
    }
    
    auto result = pool->enqueue_batch(std::move(workers));
    if (result.has_error()) {
        return { nullptr, result.get_error().message() };
    }
    
    return { pool, std::nullopt };
}

using namespace std::chrono;
using namespace thread_pool_module;
using namespace typed_thread_pool_module;

// Simple thread pool implementation for comparison
class SimpleThreadPool {
public:
    explicit SimpleThreadPool(size_t num_threads) : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() -> result_void {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        
                        if (stop_ && tasks_.empty()) return result_void();
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }
    
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        
        for (auto& worker : workers_) {
            worker.join();
        }
    }
    
    void submit(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }
    
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_;
};

// Store results for comparison across benchmarks
static std::map<std::string, double> g_baseline_times;
    
/**
 * @brief Benchmark sequential task execution baseline
 */
static void BM_SimpleTaskExecution_Sequential(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    
    for (auto _ : state) {
        std::atomic<size_t> counter{0};
        
        for (size_t i = 0; i < num_tasks; ++i) {
            counter.fetch_add(1);
        }
        
        benchmark::DoNotOptimize(counter.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
    g_baseline_times["simple_task"] = state.iterations();
}
BENCHMARK(BM_SimpleTaskExecution_Sequential)
    ->Arg(100000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark Thread System simple task execution
 */
static void BM_SimpleTaskExecution_ThreadSystem(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    
    for (auto _ : state) {
        auto [pool, error] = create_default(std::thread::hardware_concurrency());
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        std::atomic<size_t> counter{0};
        
        for (size_t i = 0; i < num_tasks; ++i) {
            pool->enqueue(std::make_unique<callback_job>([&counter]() -> result_void {
                counter.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
        benchmark::DoNotOptimize(counter.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
    if (g_baseline_times.count("simple_task")) {
        state.counters["speedup"] = g_baseline_times["simple_task"] / state.iterations();
    }
}
BENCHMARK(BM_SimpleTaskExecution_ThreadSystem)
    ->Arg(100000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark std::async simple task execution
 */
static void BM_SimpleTaskExecution_StdAsync(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    
    for (auto _ : state) {
        std::atomic<size_t> counter{0};
        std::vector<std::future<void>> futures;
        futures.reserve(num_tasks);
        
        for (size_t i = 0; i < num_tasks; ++i) {
            futures.push_back(std::async(std::launch::async, [&counter]() {
                counter.fetch_add(1);
            }));
        }
        
        for (auto& f : futures) {
            f.get();
        }
        
        benchmark::DoNotOptimize(counter.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
    if (g_baseline_times.count("simple_task")) {
        state.counters["speedup"] = g_baseline_times["simple_task"] / state.iterations();
    }
}
BENCHMARK(BM_SimpleTaskExecution_StdAsync)
    ->Arg(100000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark simple thread pool task execution
 */
static void BM_SimpleTaskExecution_SimplePool(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    
    for (auto _ : state) {
        SimpleThreadPool pool(std::thread::hardware_concurrency());
        std::atomic<size_t> counter{0};
        std::atomic<size_t> completed{0};
        
        for (size_t i = 0; i < num_tasks; ++i) {
            pool.submit([&counter, &completed]() {
                counter.fetch_add(1);
                completed.fetch_add(1);
            });
        }
        
        // Wait for completion
        while (completed.load() < num_tasks) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        benchmark::DoNotOptimize(counter.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
    if (g_baseline_times.count("simple_task")) {
        state.counters["speedup"] = g_baseline_times["simple_task"] / state.iterations();
    }
}
BENCHMARK(BM_SimpleTaskExecution_SimplePool)
    ->Arg(100000)
    ->Unit(benchmark::kMillisecond);

#ifdef _OPENMP
/**
 * @brief Benchmark OpenMP simple task execution
 */
static void BM_SimpleTaskExecution_OpenMP(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    
    for (auto _ : state) {
        std::atomic<size_t> counter{0};
        
        #pragma omp parallel for
        for (size_t i = 0; i < num_tasks; ++i) {
            counter.fetch_add(1);
        }
        
        benchmark::DoNotOptimize(counter.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
    if (g_baseline_times.count("simple_task")) {
        state.counters["speedup"] = g_baseline_times["simple_task"] / state.iterations();
    }
}
BENCHMARK(BM_SimpleTaskExecution_OpenMP)
    ->Arg(100000)
    ->Unit(benchmark::kMillisecond);
#endif
    
/**
 * @brief Benchmark sequential parallel computation baseline
 */
static void BM_ParallelComputation_Sequential(benchmark::State& state) {
    const size_t data_size = state.range(0);
    std::vector<double> data(data_size);
    
    // Initialize data
    for (size_t i = 0; i < data_size; ++i) {
        data[i] = static_cast<double>(i) * 0.1;
    }
    
    for (auto _ : state) {
        double sum = 0;
        for (const auto& val : data) {
            sum += std::sin(val) * std::cos(val);
        }
        benchmark::DoNotOptimize(sum);
    }
    
    state.SetItemsProcessed(state.iterations() * data_size);
    g_baseline_times["parallel_comp"] = state.iterations();
}
BENCHMARK(BM_ParallelComputation_Sequential)
    ->Arg(10000000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark Thread System parallel computation
 */
static void BM_ParallelComputation_ThreadSystem(benchmark::State& state) {
    const size_t data_size = state.range(0);
    std::vector<double> data(data_size);
    
    // Initialize data
    for (size_t i = 0; i < data_size; ++i) {
        data[i] = static_cast<double>(i) * 0.1;
    }
    
    const size_t num_workers = std::thread::hardware_concurrency();
    const size_t chunk_size = data_size / num_workers;
    
    for (auto _ : state) {
        auto [pool, error] = create_default(num_workers);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::vector<std::future<double>> futures;
        std::vector<std::shared_ptr<std::promise<double>>> promises;
        
        for (size_t i = 0; i < num_workers; ++i) {
            promises.push_back(std::make_shared<std::promise<double>>());
            futures.push_back(promises[i]->get_future());
        }
        
        for (size_t i = 0; i < num_workers; ++i) {
            size_t start_idx = i * chunk_size;
            size_t end_idx = (i == num_workers - 1) ? data_size : start_idx + chunk_size;
            
            auto promise_ptr = promises[i];
            pool->enqueue(std::make_unique<callback_job>([&data, start_idx, end_idx, promise_ptr]() -> result_void {
                double local_sum = 0;
                for (size_t j = start_idx; j < end_idx; ++j) {
                    local_sum += std::sin(data[j]) * std::cos(data[j]);
                }
                promise_ptr->set_value(local_sum);
                return result_void();
            }));
        }
        
        double total_sum = 0;
        for (auto& f : futures) {
            total_sum += f.get();
        }
        
        pool->stop();
        benchmark::DoNotOptimize(total_sum);
    }
    
    state.SetItemsProcessed(state.iterations() * data_size);
    if (g_baseline_times.count("parallel_comp")) {
        state.counters["speedup"] = g_baseline_times["parallel_comp"] / state.iterations();
    }
}
BENCHMARK(BM_ParallelComputation_ThreadSystem)
    ->Arg(10000000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark std::async parallel computation
 */
static void BM_ParallelComputation_StdAsync(benchmark::State& state) {
    const size_t data_size = state.range(0);
    std::vector<double> data(data_size);
    
    // Initialize data
    for (size_t i = 0; i < data_size; ++i) {
        data[i] = static_cast<double>(i) * 0.1;
    }
    
    const size_t num_workers = std::thread::hardware_concurrency();
    const size_t chunk_size = data_size / num_workers;
    
    for (auto _ : state) {
        std::vector<std::future<double>> futures;
        
        for (size_t i = 0; i < num_workers; ++i) {
            size_t start_idx = i * chunk_size;
            size_t end_idx = (i == num_workers - 1) ? data_size : start_idx + chunk_size;
            
            futures.push_back(std::async(std::launch::async, 
                [&data, start_idx, end_idx]() -> double {
                    double local_sum = 0;
                    for (size_t j = start_idx; j < end_idx; ++j) {
                        local_sum += std::sin(data[j]) * std::cos(data[j]);
                    }
                    return local_sum;
                }
            ));
        }
        
        double total_sum = 0;
        for (auto& f : futures) {
            total_sum += f.get();
        }
        
        benchmark::DoNotOptimize(total_sum);
    }
    
    state.SetItemsProcessed(state.iterations() * data_size);
    if (g_baseline_times.count("parallel_comp")) {
        state.counters["speedup"] = g_baseline_times["parallel_comp"] / state.iterations();
    }
}
BENCHMARK(BM_ParallelComputation_StdAsync)
    ->Arg(10000000)
    ->Unit(benchmark::kMillisecond);

#ifdef _OPENMP
/**
 * @brief Benchmark OpenMP parallel computation
 */
static void BM_ParallelComputation_OpenMP(benchmark::State& state) {
    const size_t data_size = state.range(0);
    std::vector<double> data(data_size);
    
    // Initialize data
    for (size_t i = 0; i < data_size; ++i) {
        data[i] = static_cast<double>(i) * 0.1;
    }
    
    for (auto _ : state) {
        double sum = 0;
        #pragma omp parallel for reduction(+:sum)
        for (size_t i = 0; i < data_size; ++i) {
            sum += std::sin(data[i]) * std::cos(data[i]);
        }
        
        benchmark::DoNotOptimize(sum);
    }
    
    state.SetItemsProcessed(state.iterations() * data_size);
    if (g_baseline_times.count("parallel_comp")) {
        state.counters["speedup"] = g_baseline_times["parallel_comp"] / state.iterations();
    }
}
BENCHMARK(BM_ParallelComputation_OpenMP)
    ->Arg(10000000)
    ->Unit(benchmark::kMillisecond);
#endif
    
/**
 * @brief Benchmark Thread System I/O bound workload with many workers
 */
static void BM_IOBound_ThreadSystem_ManyWorkers(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    const int io_delay_ms = state.range(1);
    
    for (auto _ : state) {
        auto [pool, error] = create_default(std::thread::hardware_concurrency() * 4);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        std::atomic<size_t> completed{0};
        
        for (size_t i = 0; i < num_operations; ++i) {
            pool->enqueue(std::make_unique<callback_job>([io_delay_ms, &completed]() -> result_void {
                // Simulate I/O
                std::this_thread::sleep_for(std::chrono::milliseconds(io_delay_ms));
                completed.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
        benchmark::DoNotOptimize(completed.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_operations);
    state.counters["worker_multiplier"] = 4;
}
BENCHMARK(BM_IOBound_ThreadSystem_ManyWorkers)
    ->Args({1000, 10})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark Thread System I/O bound workload with normal workers
 */
static void BM_IOBound_ThreadSystem_NormalWorkers(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    const int io_delay_ms = state.range(1);
    
    for (auto _ : state) {
        auto [pool, error] = create_default(std::thread::hardware_concurrency());
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        std::atomic<size_t> completed{0};
        
        for (size_t i = 0; i < num_operations; ++i) {
            pool->enqueue(std::make_unique<callback_job>([io_delay_ms, &completed]() -> result_void {
                std::this_thread::sleep_for(std::chrono::milliseconds(io_delay_ms));
                completed.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
        benchmark::DoNotOptimize(completed.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_operations);
    state.counters["worker_multiplier"] = 1;
}
BENCHMARK(BM_IOBound_ThreadSystem_NormalWorkers)
    ->Args({1000, 10})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark std::async I/O bound workload
 */
static void BM_IOBound_StdAsync(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    const int io_delay_ms = state.range(1);
    
    for (auto _ : state) {
        std::vector<std::future<void>> futures;
        futures.reserve(num_operations);
        
        for (size_t i = 0; i < num_operations; ++i) {
            futures.push_back(std::async(std::launch::async, [io_delay_ms]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(io_delay_ms));
            }));
        }
        
        for (auto& f : futures) {
            f.get();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_operations);
}
BENCHMARK(BM_IOBound_StdAsync)
    ->Args({1000, 10})
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark Thread System mixed CPU/IO workload
 */
static void BM_MixedWorkload_ThreadSystem(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    const int cpu_work_units = 1000;
    const int io_delay_ms = 5;
    
    auto mixed_work = [cpu_work_units, io_delay_ms]() {
        // CPU work
        volatile double result = 0;
        for (int i = 0; i < cpu_work_units; ++i) {
            result += std::sin(i) * std::cos(i);
        }
        
        // I/O work
        std::this_thread::sleep_for(std::chrono::milliseconds(io_delay_ms));
    };
    
    for (auto _ : state) {
        auto [pool, error] = create_default(std::thread::hardware_concurrency());
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        for (size_t i = 0; i < num_tasks; ++i) {
            pool->enqueue(std::make_unique<callback_job>([mixed_work]() -> result_void {
                mixed_work();
                return result_void();
            }));
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
}
BENCHMARK(BM_MixedWorkload_ThreadSystem)
    ->Arg(1000)
    ->Unit(benchmark::kMillisecond);

// Define TaskType enum for typed thread pool benchmark
enum class TaskType : int { CPU = 1, IO = 10 };

// Formatter for TaskType
template <>
struct std::formatter<TaskType> : std::formatter<int> {
    auto format(TaskType type, format_context& ctx) const {
        return formatter<int>::format(static_cast<int>(type), ctx);
    }
};

/**
 * @brief Benchmark Typed Thread System mixed workload with priority
 */
static void BM_MixedWorkload_TypedThreadSystem(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    const int cpu_work_units = 1000;
    const int io_delay_ms = 5;
    
    for (auto _ : state) {
        auto [pool, error] = create_priority_default<TaskType>(std::thread::hardware_concurrency(), {TaskType::CPU, TaskType::IO});
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        for (size_t i = 0; i < num_tasks / 2; ++i) {
            // CPU-heavy tasks get higher priority
            pool->enqueue(std::make_unique<typed_thread_pool_module::callback_typed_job_t<TaskType>>(
                [cpu_work_units]() -> result_void {
                    volatile double result = 0;
                    for (int j = 0; j < cpu_work_units * 2; ++j) {
                        result += std::sin(j) * std::cos(j);
                    }
                    return result_void();
                }, TaskType::CPU));
            
            // I/O-heavy tasks get lower priority
            pool->enqueue(std::make_unique<typed_thread_pool_module::callback_typed_job_t<TaskType>>(
                [io_delay_ms]() -> result_void {
                    std::this_thread::sleep_for(std::chrono::milliseconds(io_delay_ms * 2));
                    return result_void();
                }, TaskType::IO));
        }
        
        pool->stop();
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
}
BENCHMARK(BM_MixedWorkload_TypedThreadSystem)
    ->Arg(1000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark std::async mixed workload
 */
static void BM_MixedWorkload_StdAsync(benchmark::State& state) {
    const size_t num_tasks = state.range(0);
    const int cpu_work_units = 1000;
    const int io_delay_ms = 5;
    
    auto mixed_work = [cpu_work_units, io_delay_ms]() {
        // CPU work
        volatile double result = 0;
        for (int i = 0; i < cpu_work_units; ++i) {
            result += std::sin(i) * std::cos(i);
        }
        
        // I/O work
        std::this_thread::sleep_for(std::chrono::milliseconds(io_delay_ms));
    };
    
    for (auto _ : state) {
        std::vector<std::future<void>> futures;
        futures.reserve(num_tasks);
        
        for (size_t i = 0; i < num_tasks; ++i) {
            futures.push_back(std::async(std::launch::async, mixed_work));
        }
        
        for (auto& f : futures) {
            f.get();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
}
BENCHMARK(BM_MixedWorkload_StdAsync)
    ->Arg(1000)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark Thread System task creation overhead
 */
static void BM_TaskCreation_ThreadSystem(benchmark::State& state) {
    const size_t tasks_per_iteration = state.range(0);
    
    auto [pool, error] = create_default(4);
    if (error.has_value()) {
        state.SkipWithError("Failed to create thread pool");
        return;
    }
    
    pool->start();
    
    for (auto _ : state) {
        for (size_t i = 0; i < tasks_per_iteration; ++i) {
            pool->enqueue(std::make_unique<callback_job>([]() -> result_void {
                return result_void();
            }));
        }
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * tasks_per_iteration);
    state.counters["ns_per_task"] = benchmark::Counter(state.iterations() * tasks_per_iteration, 
                                                       benchmark::Counter::kIsRate | benchmark::Counter::kInvert,
                                                       benchmark::Counter::kIs1000);
}
BENCHMARK(BM_TaskCreation_ThreadSystem)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

/**
 * @brief Benchmark std::async task creation overhead
 */
static void BM_TaskCreation_StdAsync(benchmark::State& state) {
    const size_t tasks_per_iteration = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::future<void>> futures;
        futures.reserve(tasks_per_iteration);
        
        for (size_t i = 0; i < tasks_per_iteration; ++i) {
            futures.push_back(std::async(std::launch::deferred, [] {}));
        }
        
        futures.clear();
    }
    
    state.SetItemsProcessed(state.iterations() * tasks_per_iteration);
    state.counters["ns_per_task"] = benchmark::Counter(state.iterations() * tasks_per_iteration, 
                                                       benchmark::Counter::kIsRate | benchmark::Counter::kInvert,
                                                       benchmark::Counter::kIs1000);
}
BENCHMARK(BM_TaskCreation_StdAsync)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);

/**
 * @brief Benchmark raw lambda creation overhead
 */
static void BM_TaskCreation_RawLambda(benchmark::State& state) {
    const size_t tasks_per_iteration = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::function<void()>> tasks;
        tasks.reserve(tasks_per_iteration);
        
        for (size_t i = 0; i < tasks_per_iteration; ++i) {
            tasks.push_back([] {});
        }
        
        benchmark::DoNotOptimize(tasks.data());
    }
    
    state.SetItemsProcessed(state.iterations() * tasks_per_iteration);
    state.counters["ns_per_task"] = benchmark::Counter(state.iterations() * tasks_per_iteration, 
                                                       benchmark::Counter::kIsRate | benchmark::Counter::kInvert,
                                                       benchmark::Counter::kIs1000);
}
BENCHMARK(BM_TaskCreation_RawLambda)
    ->Arg(1000)
    ->Unit(benchmark::kMicrosecond);
    
/**
 * @brief Benchmark memory usage comparison
 * 
 * This benchmark estimates memory usage for different approaches.
 * Since we can't directly measure memory in Google Benchmark,
 * we calculate theoretical memory usage based on object sizes.
 */
static void BM_MemoryUsage_Comparison(benchmark::State& state) {
    const size_t num_queued_tasks = state.range(0);
    
    // Estimate memory per task
    size_t thread_system_memory = sizeof(job) * num_queued_tasks;
    size_t async_memory = sizeof(std::future<void>) * num_queued_tasks + 
                         sizeof(std::promise<void>) * num_queued_tasks;
    size_t simple_pool_memory = sizeof(std::function<void()>) * num_queued_tasks;
    
    for (auto _ : state) {
        // This is a meta-benchmark that just reports memory usage
        benchmark::DoNotOptimize(thread_system_memory);
        benchmark::DoNotOptimize(async_memory);
        benchmark::DoNotOptimize(simple_pool_memory);
    }
    
    state.counters["thread_system_MB"] = thread_system_memory / 1024.0 / 1024.0;
    state.counters["thread_system_bytes_per_task"] = thread_system_memory / num_queued_tasks;
    state.counters["async_MB"] = async_memory / 1024.0 / 1024.0;
    state.counters["async_bytes_per_task"] = async_memory / num_queued_tasks;
    state.counters["simple_pool_MB"] = simple_pool_memory / 1024.0 / 1024.0;
    state.counters["simple_pool_bytes_per_task"] = simple_pool_memory / num_queued_tasks;
}
BENCHMARK(BM_MemoryUsage_Comparison)
    ->Arg(100000)
    ->Unit(benchmark::kNanosecond);

// Main function to run benchmarks
BENCHMARK_MAIN();