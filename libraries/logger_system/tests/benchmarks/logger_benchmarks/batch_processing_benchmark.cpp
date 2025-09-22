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

#include <benchmark/benchmark.h>
#include <logger/config/logger_builder.h>
#include <logger/writers/console_writer.h>
#include <memory>
#include <thread>
#include <vector>

using namespace logger_module;

/**
 * @brief Placeholder benchmark for batch processing
 * 
 * This benchmark simulates batch processing behavior
 * until the actual batch_writer implementation is available.
 */

// Null writer for benchmarking
class benchmark_writer : public base_writer {
private:
    std::atomic<size_t> write_count_{0};
    
public:
    result_void write(thread_module::log_level level,
                     const std::string& message,
                     const std::string& file,
                     int line,
                     const std::string& function,
                     const std::chrono::system_clock::time_point& timestamp) override {
        write_count_++;
        benchmark::DoNotOptimize(message.size());
        return result_void{};
    }
    
    result_void flush() override {
        return result_void{};
    }
    
    std::string get_name() const override {
        return "benchmark_writer";
    }
    
    size_t get_write_count() const { return write_count_; }
};

/**
 * @brief Benchmark simulating batch vs direct writing
 */
static void BM_SimulatedBatchWriting(benchmark::State& state) {
    bool simulate_batch = state.range(0);
    size_t batch_size = state.range(1);
    
    auto writer = std::make_unique<benchmark_writer>();
    auto* writer_ptr = writer.get();
    
    const std::string message = "Simulated batch processing test message";
    const auto timestamp = std::chrono::system_clock::now();
    
    std::vector<std::pair<thread_module::log_level, std::string>> batch;
    
    for (auto _ : state) {
        if (simulate_batch) {
            // Simulate batching behavior
            batch.push_back({thread_module::log_level::info, message});
            
            if (batch.size() >= batch_size) {
                // Simulate batch flush
                for (const auto& [level, msg] : batch) {
                    writer->write(level, msg, __FILE__, __LINE__, __FUNCTION__, timestamp);
                }
                batch.clear();
            }
        } else {
            // Direct write
            writer->write(thread_module::log_level::info, 
                         message, __FILE__, __LINE__, __FUNCTION__, timestamp);
        }
    }
    
    // Flush remaining batch
    if (!batch.empty()) {
        for (const auto& [level, msg] : batch) {
            writer->write(level, msg, __FILE__, __LINE__, __FUNCTION__, timestamp);
        }
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel(simulate_batch ? 
                   "batch_size: " + std::to_string(batch_size) : 
                   "direct");
}
BENCHMARK(BM_SimulatedBatchWriting)
    ->Args({0, 0})    // Direct writing
    ->Args({1, 10})   // Batch size 10
    ->Args({1, 50})   // Batch size 50
    ->Args({1, 100})  // Batch size 100
    ->Args({1, 500}); // Batch size 500

/**
 * @brief Benchmark logger with batch configuration
 */
static void BM_LoggerWithBatchConfig(benchmark::State& state) {
    size_t batch_size = state.range(0);
    
    logger_builder builder;
    auto logger = builder
        .with_async(true)
        .with_batch_size(batch_size)
        .add_writer("benchmark", std::make_unique<benchmark_writer>())
        .build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    log->start();
    
    const std::string message = "Logger batch configuration test";
    
    for (auto _ : state) {
        log->log(thread_module::log_level::info, message);
    }
    
    log->stop();
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("batch_size: " + std::to_string(batch_size));
}
BENCHMARK(BM_LoggerWithBatchConfig)
    ->Arg(1)
    ->Arg(10)
    ->Arg(50)
    ->Arg(100)
    ->Arg(500);

/**
 * @brief Benchmark multi-threaded logging with batching
 */
static void BM_MultithreadedBatchSimulation(benchmark::State& state) {
    int thread_count = state.range(0);
    size_t batch_size = state.range(1);
    
    logger_builder builder;
    auto logger = builder
        .with_async(true)
        .with_batch_size(batch_size)
        .add_writer("benchmark", std::make_unique<benchmark_writer>())
        .build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    log->start();
    
    const std::string message = "Multi-threaded batch test";
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        threads.reserve(thread_count);
        
        auto work = [&log, &message]() {
            for (int i = 0; i < 100; ++i) {
                log->log(thread_module::log_level::info, message);
            }
        };
        
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back(work);
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    log->stop();
    
    state.SetItemsProcessed(state.iterations() * thread_count * 100);
    state.SetLabel("threads: " + std::to_string(thread_count) + 
                   ", batch: " + std::to_string(batch_size));
}
BENCHMARK(BM_MultithreadedBatchSimulation)
    ->Args({1, 10})
    ->Args({2, 10})
    ->Args({4, 10})
    ->Args({1, 100})
    ->Args({2, 100})
    ->Args({4, 100});

BENCHMARK_MAIN();