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
#include <logger/logger.h>
#include <memory>
#include <vector>
#include <random>
#include <sstream>

using namespace logger_module;

/**
 * @brief Comprehensive benchmark suite for logger system
 * 
 * This benchmark measures:
 * - End-to-end logging performance
 * - Configuration impact
 * - Message size effects
 * - Queue behavior under load
 */

// Global null writer for benchmarks
class null_writer : public base_writer {
public:
    result_void write(thread_module::log_level level,
                     const std::string& message,
                     const std::string& file,
                     int line,
                     const std::string& function,
                     const std::chrono::system_clock::time_point& timestamp) override {
        benchmark::DoNotOptimize(message.data());
        benchmark::ClobberMemory();
        return result_void{};
    }
    
    result_void flush() override { return result_void{}; }
    std::string get_name() const override { return "null_writer"; }
};

/**
 * @brief Generate random log message of specified size
 */
std::string generate_message(size_t size) {
    static const char charset[] = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        " .,!?";
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string result;
    result.reserve(size);
    
    for (size_t i = 0; i < size; ++i) {
        result += charset[dis(gen)];
    }
    
    return result;
}

/**
 * @brief Benchmark different configuration templates
 */
static void BM_ConfigurationTemplates(benchmark::State& state) {
    int config_type = state.range(0);
    
    logger_config config;
    std::string label;
    
    switch (config_type) {
        case 0:
            config = logger_config::default_config();
            label = "default";
            break;
        case 1:
            config = logger_config::high_performance();
            label = "high_performance";
            break;
        case 2:
            config = logger_config::low_latency();
            label = "low_latency";
            break;
        case 3:
            config = logger_config::production();
            label = "production";
            break;
        case 4:
            config = logger_config::debug_config();
            label = "debug";
            break;
    }
    
    logger_builder builder;
    auto logger = builder
        .with_config(config)
        .add_writer("null", std::make_unique<null_writer>())
        .build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    
    if (config.async) {
        log->start();
    }
    
    const std::string message = "Configuration benchmark test message";
    
    for (auto _ : state) {
        log->log(thread_module::log_level::info, message);
    }
    
    if (config.async) {
        log->stop();
    }
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel(label);
}
BENCHMARK(BM_ConfigurationTemplates)
    ->Arg(0)  // default
    ->Arg(1)  // high_performance
    ->Arg(2)  // low_latency
    ->Arg(3)  // production
    ->Arg(4); // debug

/**
 * @brief Benchmark message size impact
 */
static void BM_MessageSize(benchmark::State& state) {
    size_t message_size = state.range(0);
    
    logger_builder builder1;
    auto logger = builder1
        .with_async(true)
        .with_buffer_size(65536)
        .with_batch_size(100)
        .with_batch_writing(true)
        .add_writer("null", std::make_unique<null_writer>())
        .build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    log->start();
    
    const std::string message = generate_message(message_size);
    
    for (auto _ : state) {
        log->log(thread_module::log_level::info, message);
    }
    
    log->stop();
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * message_size);
    state.SetLabel(std::to_string(message_size) + " bytes");
}
BENCHMARK(BM_MessageSize)
    ->Arg(10)     // 10 bytes
    ->Arg(100)    // 100 bytes
    ->Arg(256)    // 256 bytes (SSO threshold)
    ->Arg(1024)   // 1 KB
    ->Arg(4096)   // 4 KB
    ->Arg(16384); // 16 KB

/**
 * @brief Benchmark queue behavior under different loads
 */
static void BM_QueueBehavior(benchmark::State& state) {
    size_t queue_size = state.range(0);
    size_t burst_size = state.range(1);
    
    logger_builder builder2;
    auto logger = builder2
        .with_async(true)
        .with_max_queue_size(queue_size)
        .with_overflow_policy(logger_config::overflow_policy::drop_oldest)
        .add_writer("null", std::make_unique<null_writer>())
        .build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    log->start();
    
    const std::string message = "Queue behavior test message";
    
    for (auto _ : state) {
        // Generate burst of messages
        for (size_t i = 0; i < burst_size; ++i) {
            log->log(thread_module::log_level::info, message);
        }
        
        // Small delay to allow processing
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    
    log->stop();
    
    state.SetItemsProcessed(state.iterations() * burst_size);
    state.SetLabel("queue: " + std::to_string(queue_size) + 
                   ", burst: " + std::to_string(burst_size));
}
BENCHMARK(BM_QueueBehavior)
    ->Args({1000, 10})    // Small queue, small burst
    ->Args({1000, 100})   // Small queue, large burst
    ->Args({10000, 100})  // Large queue, small burst
    ->Args({10000, 1000}) // Large queue, large burst
    ->Args({100000, 5000}); // Very large queue, very large burst

/**
 * @brief Benchmark logging with different numbers of writers
 */
static void BM_MultipleWriters(benchmark::State& state) {
    int writer_count = state.range(0);
    
    logger_builder builder;
    builder
        .with_async(true)
        .with_batch_writing(true)
        .with_batch_size(100);
    
    for (int i = 0; i < writer_count; ++i) {
        builder.add_writer("null_" + std::to_string(i), 
                          std::make_unique<null_writer>());
    }
    
    auto logger = builder.build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    log->start();
    
    const std::string message = "Multiple writers test message";
    
    for (auto _ : state) {
        log->log(thread_module::log_level::info, message);
    }
    
    log->stop();
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel(std::to_string(writer_count) + " writers");
}
BENCHMARK(BM_MultipleWriters)
    ->Arg(1)
    ->Arg(2)
    ->Arg(5)
    ->Arg(10);

/**
 * @brief Benchmark filter performance impact
 */
static void BM_FilterImpact(benchmark::State& state) {
    bool use_filter = state.range(0);
    
    logger_builder builder;
    builder
        .with_async(true)
        .add_writer("null", std::make_unique<null_writer>());
    
    if (use_filter) {
        // Add a simple level filter
        builder.with_min_level(thread_module::log_level::warning);
    }
    
    auto logger = builder.build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    log->start();
    
    const std::string info_msg = "Info message (filtered)";
    const std::string warn_msg = "Warning message (not filtered)";
    
    for (auto _ : state) {
        // Mix of messages that pass and don't pass filter
        log->log(thread_module::log_level::info, info_msg);
        log->log(thread_module::log_level::warning, warn_msg);
        log->log(thread_module::log_level::info, info_msg);
        log->log(thread_module::log_level::error, warn_msg);
    }
    
    log->stop();
    
    state.SetItemsProcessed(state.iterations() * 4);
    state.SetLabel(use_filter ? "with_filter" : "no_filter");
}
BENCHMARK(BM_FilterImpact)
    ->Arg(0)  // No filter
    ->Arg(1); // With filter

/**
 * @brief Benchmark structured logging overhead
 */
static void BM_StructuredLogging(benchmark::State& state) {
    bool structured = state.range(0);
    
    logger_builder builder;
    builder
        .with_async(true)
        .with_structured_logging(structured)
        .add_writer("null", std::make_unique<null_writer>());
    
    auto logger = builder.build();
    
    if (!logger) {
        state.SkipWithError("Failed to build logger");
        return;
    }
    
    auto& log = *logger.value();
    log->start();
    
    for (auto _ : state) {
        if (structured) {
            // Simulate structured log with metadata
            std::stringstream ss;
            ss << R"({"level":"info","message":"test","timestamp":)"
               << std::chrono::system_clock::now().time_since_epoch().count()
               << R"(,"metadata":{"user_id":123,"session":"abc123"}})";
            log->log(thread_module::log_level::info, ss.str());
        } else {
            log->log(thread_module::log_level::info, "Regular log message");
        }
    }
    
    log->stop();
    
    state.SetItemsProcessed(state.iterations());
    state.SetLabel(structured ? "structured" : "plain");
}
BENCHMARK(BM_StructuredLogging)
    ->Arg(0)  // Plain
    ->Arg(1); // Structured

BENCHMARK_MAIN();