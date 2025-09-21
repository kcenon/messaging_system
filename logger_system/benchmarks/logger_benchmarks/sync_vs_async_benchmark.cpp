/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace logger_module;

// Null writer for performance testing
class null_writer : public base_writer {
public:
    void write(thread_module::log_level level,
              const std::string& message,
              const std::string& file,
              int line,
              const std::string& function,
              const std::chrono::system_clock::time_point& timestamp) override {
        (void)level; (void)message; (void)file; (void)line; (void)function; (void)timestamp;
    }
    
    void flush() override {}
};

// Benchmark sync vs async with single thread
static void BM_SyncSingleThread(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(false);
    logger_instance->add_writer(std::make_unique<null_writer>());
    
    const std::string message = "Single thread sync message";
    
    for (auto _ : state) {
        logger_instance->log(thread_module::log_level::info, message);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SyncSingleThread);

static void BM_AsyncSingleThread(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(true);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->start();
    
    const std::string message = "Single thread async message";
    
    for (auto _ : state) {
        logger_instance->log(thread_module::log_level::info, message);
    }
    
    logger_instance->flush();
    logger_instance->stop();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_AsyncSingleThread);

// Benchmark sync vs async with multiple threads
static void BM_SyncMultiThread(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(false);
    logger_instance->add_writer(std::make_unique<null_writer>());
    
    const int num_threads = state.range(0);
    const std::string message = "Multi thread sync message";
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&logger_instance, &message]() {
                logger_instance->log(thread_module::log_level::info, message);
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_threads);
}
BENCHMARK(BM_SyncMultiThread)->RangeMultiplier(2)->Range(1, 16);

static void BM_AsyncMultiThread(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(true);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->start();
    
    const int num_threads = state.range(0);
    const std::string message = "Multi thread async message";
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&logger_instance, &message]() {
                logger_instance->log(thread_module::log_level::info, message);
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    logger_instance->flush();
    logger_instance->stop();
    state.SetItemsProcessed(state.iterations() * num_threads);
}
BENCHMARK(BM_AsyncMultiThread)->RangeMultiplier(2)->Range(1, 16);

// Benchmark with different buffer sizes for async logger
static void BM_AsyncBufferSize(benchmark::State& state) {
    const size_t buffer_size = state.range(0);
    auto logger_instance = std::make_unique<logger>(true, buffer_size);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->start();
    
    const std::string message = "Buffer size test message";
    
    for (auto _ : state) {
        logger_instance->log(thread_module::log_level::info, message);
    }
    
    logger_instance->flush();
    logger_instance->stop();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_AsyncBufferSize)->RangeMultiplier(2)->Range(512, 65536);

// Benchmark throughput comparison
static void BM_SyncThroughput(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(false);
    logger_instance->add_writer(std::make_unique<null_writer>());
    
    const std::string message = "Throughput test message";
    const int batch_size = state.range(0);
    
    for (auto _ : state) {
        for (int i = 0; i < batch_size; ++i) {
            logger_instance->log(thread_module::log_level::info, message + std::to_string(i));
        }
    }
    
    state.SetItemsProcessed(state.iterations() * batch_size);
}
BENCHMARK(BM_SyncThroughput)->RangeMultiplier(10)->Range(10, 10000);

static void BM_AsyncThroughput(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(true);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->start();
    
    const std::string message = "Throughput test message";
    const int batch_size = state.range(0);
    
    for (auto _ : state) {
        for (int i = 0; i < batch_size; ++i) {
            logger_instance->log(thread_module::log_level::info, message + std::to_string(i));
        }
        logger_instance->flush();  // Ensure all messages are processed
    }
    
    logger_instance->stop();
    state.SetItemsProcessed(state.iterations() * batch_size);
}
BENCHMARK(BM_AsyncThroughput)->RangeMultiplier(10)->Range(10, 10000);

BENCHMARK_MAIN();