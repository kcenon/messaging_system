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
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <memory>
#include <string>

using namespace logger_module;

// Null writer for performance testing (doesn't actually output)
class null_writer : public base_writer {
public:
    void write(thread_module::log_level level,
              const std::string& message,
              const std::string& file,
              int line,
              const std::string& function,
              const std::chrono::system_clock::time_point& timestamp) override {
        // Do nothing - just for performance testing
        (void)level; (void)message; (void)file; (void)line; (void)function; (void)timestamp;
    }
    
    void flush() override {
        // Do nothing
    }
};

// Global logger instances for benchmarks
static std::unique_ptr<logger> sync_logger;
static std::unique_ptr<logger> async_logger;

// Setup function
static void setup_loggers() {
    sync_logger = std::make_unique<logger>(false);  // synchronous
    async_logger = std::make_unique<logger>(true);   // asynchronous
    
    sync_logger->add_writer(std::make_unique<null_writer>());
    async_logger->add_writer(std::make_unique<null_writer>());
    
    async_logger->start();
}

// Cleanup function
static void cleanup_loggers() {
    if (async_logger) {
        async_logger->stop();
    }
}

// Benchmark synchronous logging
static void BM_SyncLogging(benchmark::State& state) {
    if (!sync_logger) setup_loggers();
    
    const std::string message = "Benchmark test message for synchronous logging";
    
    for (auto _ : state) {
        sync_logger->log(thread_module::log_level::info, message);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SyncLogging);

// Benchmark asynchronous logging
static void BM_AsyncLogging(benchmark::State& state) {
    if (!async_logger) setup_loggers();
    
    const std::string message = "Benchmark test message for asynchronous logging";
    
    for (auto _ : state) {
        async_logger->log(thread_module::log_level::info, message);
    }
    
    // Flush to ensure all messages are processed
    async_logger->flush();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_AsyncLogging);

// Benchmark logging with different message sizes
static void BM_LoggingMessageSize(benchmark::State& state) {
    if (!sync_logger) setup_loggers();
    
    const int message_size = state.range(0);
    const std::string message(message_size, 'X');
    
    for (auto _ : state) {
        sync_logger->log(thread_module::log_level::info, message);
    }
    
    state.SetBytesProcessed(state.iterations() * message_size);
}
BENCHMARK(BM_LoggingMessageSize)->Range(8, 8192);

// Benchmark logging different levels
static void BM_LoggingLevels(benchmark::State& state) {
    if (!sync_logger) setup_loggers();
    
    const std::string message = "Level test message";
    const auto levels = {
        thread_module::log_level::trace,
        thread_module::log_level::debug,
        thread_module::log_level::info,
        thread_module::log_level::warning,
        thread_module::log_level::error,
        thread_module::log_level::critical
    };
    
    int level_idx = 0;
    
    for (auto _ : state) {
        auto level = *(levels.begin() + (level_idx % levels.size()));
        sync_logger->log(level, message);
        level_idx++;
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_LoggingLevels);

// Benchmark logging with source location
static void BM_LoggingWithSourceLocation(benchmark::State& state) {
    if (!sync_logger) setup_loggers();
    
    const std::string message = "Message with source location";
    
    for (auto _ : state) {
        sync_logger->log(thread_module::log_level::info, message, 
                        __FILE__, __LINE__, __func__);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_LoggingWithSourceLocation);

// Benchmark logger construction and destruction
static void BM_LoggerConstruction(benchmark::State& state) {
    for (auto _ : state) {
        auto test_logger = std::make_unique<logger>(false);
        test_logger->add_writer(std::make_unique<null_writer>());
        benchmark::DoNotOptimize(test_logger);
    }
}
BENCHMARK(BM_LoggerConstruction);

// Benchmark async logger construction and destruction
static void BM_AsyncLoggerConstruction(benchmark::State& state) {
    for (auto _ : state) {
        auto test_logger = std::make_unique<logger>(true);
        test_logger->add_writer(std::make_unique<null_writer>());
        test_logger->start();
        test_logger->stop();
        benchmark::DoNotOptimize(test_logger);
    }
}
BENCHMARK(BM_AsyncLoggerConstruction);

// Benchmark log level checking
static void BM_LogLevelCheck(benchmark::State& state) {
    if (!sync_logger) setup_loggers();
    
    sync_logger->set_min_level(thread_module::log_level::warning);
    
    for (auto _ : state) {
        bool enabled = sync_logger->is_enabled(thread_module::log_level::info);
        benchmark::DoNotOptimize(enabled);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_LogLevelCheck);

// Benchmark flush operation
static void BM_FlushOperation(benchmark::State& state) {
    if (!async_logger) setup_loggers();
    
    // Add some messages to flush
    for (int i = 0; i < 100; ++i) {
        async_logger->log(thread_module::log_level::info, "Message to flush");
    }
    
    for (auto _ : state) {
        async_logger->flush();
    }
}
BENCHMARK(BM_FlushOperation);

// Main function with setup/cleanup
int main(int argc, char** argv) {
    setup_loggers();
    
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    
    benchmark::RunSpecifiedBenchmarks();
    
    cleanup_loggers();
    return 0;
}