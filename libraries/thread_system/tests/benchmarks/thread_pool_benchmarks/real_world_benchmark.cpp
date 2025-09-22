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

/**
 * @file real_world_benchmark.cpp
 * @brief Real-world scenario benchmarks for Thread System
 * 
 * Tests that simulate actual usage patterns:
 * - Web server request handling
 * - Image processing pipeline
 * - Data analysis workloads
 * - Mixed I/O and CPU tasks
 */

#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <future>
#include <map>
#include <thread>

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

// Simulate different types of workloads
class WorkloadSimulator {
public:
    // Simulate CPU-intensive work (e.g., image processing)
    static void simulate_cpu_work(int complexity) {
        volatile double result = 0;
        for (int i = 0; i < complexity * 1000; ++i) {
            result += std::sin(i) * std::cos(i);
        }
    }
    
    // Simulate I/O operation (e.g., database query)
    static void simulate_io_work(int duration_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    }
    
    // Simulate memory-intensive work
    static void simulate_memory_work(size_t size_mb) {
        std::vector<char> buffer(size_mb * 1024 * 1024);
        // Touch memory to ensure allocation
        for (size_t i = 0; i < buffer.size(); i += 4096) {
            buffer[i] = static_cast<char>(i & 0xFF);
        }
    }
    
    // Simulate mixed workload
    static void simulate_mixed_work(int cpu_complexity, int io_duration_ms) {
        simulate_cpu_work(cpu_complexity);
        simulate_io_work(io_duration_ms);
    }
};

// Simulate different request types for web server
struct RequestType {
    std::string name;
    int cpu_work;      // CPU complexity (1-100)
    int io_duration;   // I/O duration in ms
    double frequency;  // Relative frequency (0.0-1.0)
};

static const std::vector<RequestType> g_request_types = {
    {"Static file", 1, 1, 0.5},
    {"API query", 5, 10, 0.3},
    {"Database write", 10, 50, 0.15},
    {"Complex computation", 50, 5, 0.05}
};
/**
 * @brief Benchmark web server request handling simulation
 */
static void BM_WebServerSimulation(benchmark::State& state) {
    const size_t workers = state.range(0);
    const size_t total_requests = state.range(1);
    
    for (auto _ : state) {
        auto [pool, error] = create_default(workers);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed_requests{0};
        std::atomic<size_t> total_response_time_ms{0};
        
        // Random request generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (size_t i = 0; i < total_requests; ++i) {
            // Select request type based on frequency
            double rand_val = dis(gen);
            double cumulative = 0.0;
            
            for (const auto& req_type : g_request_types) {
                cumulative += req_type.frequency;
                if (rand_val <= cumulative) {
                    pool->enqueue(std::make_unique<callback_job>([&req_type, &completed_requests, &total_response_time_ms]() -> result_void {
                        auto req_start = std::chrono::high_resolution_clock::now();
                        
                        // Process request
                        WorkloadSimulator::simulate_mixed_work(
                            req_type.cpu_work, 
                            req_type.io_duration
                        );
                        
                        auto req_end = std::chrono::high_resolution_clock::now();
                        auto response_time = std::chrono::duration_cast<std::chrono::milliseconds>(req_end - req_start).count();
                        
                        total_response_time_ms.fetch_add(response_time);
                        completed_requests.fetch_add(1);
                        return result_void();
                    }));
                    break;
                }
            }
        }
        
        pool->stop();
        
        double avg_response_time = static_cast<double>(total_response_time_ms.load()) / total_requests;
        state.counters["avg_response_ms"] = avg_response_time;
    }
    
    state.SetItemsProcessed(state.iterations() * total_requests);
    state.counters["workers"] = workers;
}
BENCHMARK(BM_WebServerSimulation)
    ->Args({8, 10000})
    ->Args({16, 10000})
    ->Args({32, 10000})
    ->Args({64, 10000})
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark image processing pipeline simulation
 */
static void BM_ImageProcessingPipeline(benchmark::State& state) {
    const size_t num_images = state.range(0);
    
    // Simulate image processing stages
    struct ProcessingStage {
        std::string name;
        int complexity;  // Processing complexity
    };
    
    const std::vector<ProcessingStage> stages = {
        {"Load", 10},
        {"Resize", 20},
        {"Filter", 50},
        {"Compress", 30},
        {"Save", 15}
    };
    
    for (auto _ : state) {
        auto [pool, error] = create_default(std::thread::hardware_concurrency());
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> images_processed{0};
        
        // Process each image through all stages
        for (size_t img = 0; img < num_images; ++img) {
            pool->enqueue(std::make_unique<callback_job>([&stages, &images_processed]() -> result_void {
                // Simulate processing through each stage
                for (const auto& stage : stages) {
                    WorkloadSimulator::simulate_cpu_work(stage.complexity);
                }
                
                images_processed.fetch_add(1);
                return result_void();
            }));
        }
        
        pool->stop();
        
        benchmark::DoNotOptimize(images_processed.load());
    }
    
    state.SetItemsProcessed(state.iterations() * num_images);
    state.counters["images"] = num_images;
}
BENCHMARK(BM_ImageProcessingPipeline)
    ->Arg(100)
    ->Arg(500)
    ->Arg(1000)
    ->Arg(5000)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark data analysis workload (MapReduce simulation)
 */
static void BM_DataAnalysisWorkload(benchmark::State& state) {
    const size_t workers = state.range(0);
    const size_t data_size_mb = state.range(1);
    const size_t chunk_size_mb = 10;
    const size_t num_chunks = data_size_mb / chunk_size_mb;
    
    for (auto _ : state) {
        auto [pool, error] = create_default(workers);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        // Map phase
        std::vector<std::future<double>> map_results;
        std::vector<std::shared_ptr<std::promise<double>>> promises;
        
        for (size_t i = 0; i < num_chunks; ++i) {
            auto promise = std::make_shared<std::promise<double>>();
            map_results.push_back(promise->get_future());
            promises.push_back(promise);
        }
        
        // Submit map tasks
        for (size_t i = 0; i < num_chunks; ++i) {
            pool->enqueue(std::make_unique<callback_job>([i, chunk_size_mb, p = promises[i]]() mutable -> result_void {
                // Simulate data processing
                WorkloadSimulator::simulate_memory_work(chunk_size_mb);
                WorkloadSimulator::simulate_cpu_work(100);
                
                // Return partial result
                double result = static_cast<double>(i) * 3.14159;
                p->set_value(result);
                return result_void();
            }));
        }
        
        // Collect map results
        double map_sum = 0;
        for (auto& future : map_results) {
            map_sum += future.get();
        }
        
        // Reduce phase
        auto reduce_promise = std::make_shared<std::promise<double>>();
        auto reduce_future = reduce_promise->get_future();
        
        pool->enqueue(std::make_unique<callback_job>([map_sum, p = reduce_promise]() mutable -> result_void {
            // Simulate reduce operation
            WorkloadSimulator::simulate_cpu_work(50);
            p->set_value(map_sum / 2.0);
            return result_void();
        }));
        
        double final_result = reduce_future.get();
        pool->stop();
        
        benchmark::DoNotOptimize(final_result);
    }
    
    state.SetBytesProcessed(state.iterations() * data_size_mb * 1024 * 1024);
    state.counters["workers"] = workers;
    state.counters["data_size_mb"] = data_size_mb;
}
BENCHMARK(BM_DataAnalysisWorkload)
    ->Args({2, 100})
    ->Args({4, 100})
    ->Args({8, 100})
    ->Args({16, 100})
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark game engine update loop simulation
 */
static void BM_GameEngineSimulation(benchmark::State& state) {
    const int target_fps = state.range(0);
    const int num_frames = state.range(1);
    const int frame_time_ms = 1000 / target_fps;
    
    // Simulate game engine subsystems
    using Type = job_types;
    
    struct Subsystem {
        std::string name;
        Type priority;
        int update_time_us;  // Microseconds per update
        int frequency;       // Updates per frame
    };
    
    const std::vector<Subsystem> subsystems = {
        {"Physics", Type::RealTime, 1000, 2},      // Highest priority
        {"AI", Type::Batch, 500, 1},
        {"Rendering", Type::Batch, 2000, 1},
        {"Audio", Type::Background, 200, 4},
        {"Network", Type::Background, 300, 2}      // Lowest priority
    };
    
    for (auto _ : state) {
        auto [pool, error] = create_priority_default<job_types>(8);
        if (error.has_value()) {
            state.SkipWithError("Failed to create typed thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<int> completed_frames{0};
        std::atomic<int> missed_frames{0};
        
        for (int frame = 0; frame < num_frames; ++frame) {
            auto frame_start = std::chrono::high_resolution_clock::now();
            std::atomic<int> subsystems_completed{0};
            int total_subsystems = 0;
            
            // Submit all subsystem updates for this frame
            for (const auto& subsystem : subsystems) {
                for (int i = 0; i < subsystem.frequency; ++i) {
                    total_subsystems++;
                    
                    pool->enqueue(std::make_unique<callback_typed_job_t<job_types>>([&subsystem, &subsystems_completed]() -> result_void {
                        // Simulate subsystem update
                        auto end_time = std::chrono::high_resolution_clock::now() + 
                                       std::chrono::microseconds(subsystem.update_time_us);
                        while (std::chrono::high_resolution_clock::now() < end_time) {
                            // Busy wait to simulate work
                        }
                        
                        subsystems_completed.fetch_add(1);
                        return result_void();
                    }, subsystem.priority));
                }
            }
            
            // Wait for frame completion or timeout
            auto frame_deadline = frame_start + std::chrono::milliseconds(frame_time_ms);
            while (subsystems_completed.load() < total_subsystems && 
                   std::chrono::high_resolution_clock::now() < frame_deadline) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            
            auto frame_end = std::chrono::high_resolution_clock::now();
            auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start).count();
            
            if (frame_duration > frame_time_ms) {
                missed_frames.fetch_add(1);
            }
            
            completed_frames.fetch_add(1);
            
            // Sleep if frame completed early
            if (frame_duration < frame_time_ms) {
                std::this_thread::sleep_for(std::chrono::milliseconds(frame_time_ms - frame_duration));
            }
        }
        
        pool->stop();
        
        state.counters["completed_frames"] = completed_frames.load();
        state.counters["missed_frames"] = missed_frames.load();
        state.counters["miss_rate_%"] = (missed_frames.load() * 100.0) / completed_frames.load();
    }
    
    state.counters["target_fps"] = target_fps;
}
BENCHMARK(BM_GameEngineSimulation)
    ->Args({60, 300})   // 60 FPS, 5 seconds
    ->Args({30, 150})   // 30 FPS, 5 seconds
    ->Args({120, 600})  // 120 FPS, 5 seconds
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();
    
/**
 * @brief Benchmark microservice communication pattern
 */
static void BM_MicroserviceCommunication(benchmark::State& state) {
    const size_t num_requests = state.range(0);
    
    // Simulate service-to-service communication
    struct Service {
        std::string name;
        int processing_time_ms;
        std::vector<std::string> dependencies;
    };
    
    const std::vector<Service> services = {
        {"Gateway", 5, {}},
        {"Auth", 10, {"Gateway"}},
        {"UserService", 15, {"Auth"}},
        {"OrderService", 20, {"Auth", "UserService"}},
        {"PaymentService", 25, {"OrderService"}},
        {"NotificationService", 10, {"OrderService", "PaymentService"}}
    };
    
    for (auto _ : state) {
        auto [pool, error] = create_default(16);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> completed_requests{0};
        std::atomic<size_t> total_latency_ms{0};
        
        for (size_t req = 0; req < num_requests; ++req) {
            pool->enqueue(std::make_unique<callback_job>([&services, &completed_requests, &total_latency_ms, &pool]() -> result_void {
                auto req_start = std::chrono::high_resolution_clock::now();
                
                // Process through service chain
                std::map<std::string, std::future<void>> service_futures;
                
                for (const auto& service : services) {
                    // Wait for dependencies
                    for (const auto& dep : service.dependencies) {
                        if (service_futures.count(dep)) {
                            service_futures[dep].get();
                        }
                    }
                    
                    // Process service
                    auto promise = std::make_shared<std::promise<void>>();
                    service_futures[service.name] = promise->get_future();
                    
                    pool->enqueue(std::make_unique<callback_job>([&service, p = promise]() mutable -> result_void {
                        WorkloadSimulator::simulate_io_work(service.processing_time_ms);
                        p->set_value();
                        return result_void();
                    }));
                }
                
                // Wait for final service
                if (service_futures.count("NotificationService")) {
                    service_futures["NotificationService"].get();
                }
                
                auto req_end = std::chrono::high_resolution_clock::now();
                auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(req_end - req_start).count();
                
                total_latency_ms.fetch_add(latency);
                completed_requests.fetch_add(1);
                return result_void();
            }));
        }
        
        // Wait for all requests
        while (completed_requests.load() < num_requests) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        pool->stop();
        
        double avg_latency = static_cast<double>(total_latency_ms.load()) / num_requests;
        state.counters["avg_latency_ms"] = avg_latency;
    }
    
    state.SetItemsProcessed(state.iterations() * num_requests);
}
BENCHMARK(BM_MicroserviceCommunication)
    ->Arg(100)
    ->Arg(500)
    ->Arg(1000)
    ->Unit(benchmark::kMillisecond);
    
/**
 * @brief Benchmark batch file processing simulation
 */
static void BM_BatchFileProcessing(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    const size_t total_files = state.range(1);
    
    // Simulate processing different file types
    struct FileType {
        std::string extension;
        int processing_complexity;
        size_t avg_size_kb;
    };
    
    const std::vector<FileType> file_types = {
        {".txt", 10, 50},
        {".csv", 20, 500},
        {".json", 30, 200},
        {".xml", 40, 300},
        {".log", 15, 1000}
    };
    
    for (auto _ : state) {
        auto [pool, error] = create_default(std::thread::hardware_concurrency() * 2);
        if (error.has_value()) {
            state.SkipWithError("Failed to create thread pool");
            return;
        }
        
        pool->start();
        
        std::atomic<size_t> files_processed{0};
        std::atomic<size_t> total_bytes_processed{0};
        
        // Process files in batches
        for (size_t i = 0; i < total_files; i += batch_size) {
            size_t current_batch_size = std::min(batch_size, total_files - i);
            
            pool->enqueue(std::make_unique<callback_job>([current_batch_size, &file_types, &files_processed, &total_bytes_processed]() -> result_void {
                size_t batch_bytes = 0;
                
                for (size_t j = 0; j < current_batch_size; ++j) {
                    // Randomly select file type
                    const auto& file_type = file_types[j % file_types.size()];
                    
                    // Simulate file processing
                    WorkloadSimulator::simulate_cpu_work(file_type.processing_complexity);
                    WorkloadSimulator::simulate_io_work(1);  // File I/O
                    
                    batch_bytes += file_type.avg_size_kb * 1024;
                }
                
                files_processed.fetch_add(current_batch_size);
                total_bytes_processed.fetch_add(batch_bytes);
                return result_void();
            }));
        }
        
        pool->stop();
        
        state.SetBytesProcessed(total_bytes_processed.load());
    }
    
    state.SetItemsProcessed(state.iterations() * total_files);
    state.counters["batch_size"] = batch_size;
    state.counters["total_files"] = total_files;
}
BENCHMARK(BM_BatchFileProcessing)
    ->Args({10, 10000})
    ->Args({50, 10000})
    ->Args({100, 10000})
    ->Args({500, 10000})
    ->Unit(benchmark::kMillisecond);

// Main function to run benchmarks
BENCHMARK_MAIN();