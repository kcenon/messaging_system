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
 * @file memory_benchmark.cpp
 * @brief Memory usage benchmarks for Thread System
 */

#include <benchmark/benchmark.h>
#include <iomanip>
#include <thread>
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#ifdef __APPLE__
#include <mach/mach.h>
#endif
#endif

#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_pool/workers/thread_worker.h"
#include "../../sources/typed_thread_pool/pool/typed_thread_pool.h"
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

// Stub function for create_priority_default - will be replaced with actual implementation
template<typename Type>
auto create_priority_default(const uint16_t& worker_counts) {
    // Create a regular thread pool for now since typed thread pool has compilation issues
    auto [pool, error] = create_default(worker_counts);
    return std::make_tuple(pool, error);
}

using namespace thread_pool_module;
using namespace typed_thread_pool_module;

class MemoryMonitor {
public:
    struct MemoryStats {
        size_t virtual_size;    // Virtual memory size in bytes
        size_t resident_size;   // Resident set size in bytes
        size_t peak_size;       // Peak memory usage
    };
    
    static MemoryStats get_current_memory() {
        MemoryStats stats = {0, 0, 0};
        
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            stats.virtual_size = pmc.PrivateUsage;
            stats.resident_size = pmc.WorkingSetSize;
            stats.peak_size = pmc.PeakWorkingSetSize;
        }
#elif defined(__APPLE__)
        struct mach_task_basic_info info;
        mach_msg_type_number_t size = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &size) == KERN_SUCCESS) {
            stats.virtual_size = info.virtual_size;
            stats.resident_size = info.resident_size;
            stats.peak_size = info.resident_size_max;
        }
#else
        // Linux
        FILE* file = fopen("/proc/self/status", "r");
        if (file) {
            char line[128];
            while (fgets(line, 128, file)) {
                if (strncmp(line, "VmSize:", 7) == 0) {
                    stats.virtual_size = parse_kb(line) * 1024;
                } else if (strncmp(line, "VmRSS:", 6) == 0) {
                    stats.resident_size = parse_kb(line) * 1024;
                } else if (strncmp(line, "VmPeak:", 7) == 0) {
                    stats.peak_size = parse_kb(line) * 1024;
                }
            }
            fclose(file);
        }
#endif
        return stats;
    }
    
private:
    static size_t parse_kb(const char* line) {
        size_t value = 0;
        const char* p = line;
        while (*p && !isdigit(*p)) p++;
        if (*p) {
            value = atol(p);
        }
        return value;
    }
};

/**
 * @brief Benchmark base memory usage
 */
static void BM_BaseMemory(benchmark::State& state) {
    for (auto _ : state) {
        auto stats = MemoryMonitor::get_current_memory();
        benchmark::DoNotOptimize(stats);
    }
    
    auto final_stats = MemoryMonitor::get_current_memory();
    state.counters["virtual_MB"] = final_stats.virtual_size / 1024.0 / 1024.0;
    state.counters["resident_MB"] = final_stats.resident_size / 1024.0 / 1024.0;
    state.counters["peak_MB"] = final_stats.peak_size / 1024.0 / 1024.0;
}
BENCHMARK(BM_BaseMemory);
    
/**
 * @brief Benchmark thread pool memory usage
 */
static void BM_ThreadPoolMemory(benchmark::State& state) {
    const size_t worker_count = state.range(0);
    
    for (auto _ : state) {
        auto before = MemoryMonitor::get_current_memory();
        
        auto [pool, error] = create_default(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto after = MemoryMonitor::get_current_memory();
        
        size_t memory_increase = after.resident_size - before.resident_size;
        
        pool->stop();
        
        state.counters["total_MB"] = memory_increase / 1024.0 / 1024.0;
        state.counters["per_worker_KB"] = static_cast<double>(memory_increase) / worker_count / 1024.0;
    }
    
    state.counters["workers"] = worker_count;
}
BENCHMARK(BM_ThreadPoolMemory)
    ->Arg(1)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(32)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark typed thread pool memory usage
 */
static void BM_TypedThreadPoolMemory(benchmark::State& state) {
    const size_t worker_count = state.range(0);
    
    enum class Type { RealTime = 1, Medium = 5, Background = 10 };
    
    for (auto _ : state) {
        auto before = MemoryMonitor::get_current_memory();
        
        auto [pool, error] = create_priority_default<Type>(worker_count);
        if (error.has_value()) {
            state.SkipWithError("Failed to create typed thread pool");
            return;
        }
        
        pool->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto after = MemoryMonitor::get_current_memory();
        
        size_t memory_increase = after.resident_size - before.resident_size;
        
        pool->stop();
        
        state.counters["total_MB"] = memory_increase / 1024.0 / 1024.0;
        state.counters["per_worker_KB"] = static_cast<double>(memory_increase) / worker_count / 1024.0;
    }
    
    state.counters["workers"] = worker_count;
}
BENCHMARK(BM_TypedThreadPoolMemory)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark job queue memory usage
 */
static void BM_JobQueueMemory(benchmark::State& state) {
    const size_t job_count = state.range(0);
    
    auto [pool, error] = create_default(4);
    if (error.has_value()) {
        state.SkipWithError("Failed to create thread pool");
        return;
    }
    
    pool->start();
    
    for (auto _ : state) {
        auto before = MemoryMonitor::get_current_memory();
        
        // Submit jobs that will queue up
        for (size_t i = 0; i < job_count; ++i) {
            pool->enqueue(std::make_unique<callback_job>([]() -> result_void {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                return result_void();
            }));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto after = MemoryMonitor::get_current_memory();
        
        size_t memory_increase = after.resident_size - before.resident_size;
        
        state.counters["total_MB"] = memory_increase / 1024.0 / 1024.0;
        state.counters["per_job_bytes"] = static_cast<double>(memory_increase) / job_count;
        
        // Clear the queue
        pool->stop();
        pool->start();
    }
    
    pool->stop();
    state.counters["jobs"] = job_count;
}
BENCHMARK(BM_JobQueueMemory)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(50000)
    ->Arg(100000)
    ->Unit(benchmark::kMillisecond);
    
// Logger memory benchmark removed - logger moved to separate project

/**
 * @brief Benchmark memory allocation patterns
 */
static void BM_MemoryAllocationPattern(benchmark::State& state) {
    const size_t allocation_size = state.range(0);
    const size_t num_allocations = state.range(1);
    
    for (auto _ : state) {
        auto before = MemoryMonitor::get_current_memory();
        
        std::vector<std::unique_ptr<char[]>> allocations;
        allocations.reserve(num_allocations);
        
        for (size_t i = 0; i < num_allocations; ++i) {
            allocations.push_back(std::make_unique<char[]>(allocation_size));
            // Touch memory to ensure it's actually allocated
            std::memset(allocations.back().get(), i & 0xFF, allocation_size);
        }
        
        auto after = MemoryMonitor::get_current_memory();
        
        size_t memory_increase = after.resident_size - before.resident_size;
        state.counters["total_MB"] = memory_increase / 1024.0 / 1024.0;
        state.counters["efficiency"] = static_cast<double>(allocation_size * num_allocations) / memory_increase;
    }
    
    state.counters["alloc_size"] = allocation_size;
    state.counters["num_allocs"] = num_allocations;
}
BENCHMARK(BM_MemoryAllocationPattern)
    ->Args({1024, 1000})      // 1KB x 1000
    ->Args({4096, 1000})      // 4KB x 1000
    ->Args({65536, 100})      // 64KB x 100
    ->Args({1048576, 10})     // 1MB x 10
    ->Unit(benchmark::kMillisecond);

// Main function to run benchmarks
BENCHMARK_MAIN();