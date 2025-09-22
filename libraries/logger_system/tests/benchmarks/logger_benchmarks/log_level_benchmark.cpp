/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <logger/logger.h>
#include <memory>
#include <string>

using namespace logger_module;

class null_writer : public base_writer {
public:
    void write(thread_module::log_level level, const std::string& message,
              const std::string& file, int line, const std::string& function,
              const std::chrono::system_clock::time_point& timestamp) override {
        (void)level; (void)message; (void)file; (void)line; (void)function; (void)timestamp;
    }
    void flush() override {}
};

static void BM_LogLevelFiltering_Enabled(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(false);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->set_min_level(thread_module::log_level::info);  // Allow info and above
    
    const std::string message = "Enabled log message";
    
    for (auto _ : state) {
        logger_instance->log(thread_module::log_level::warning, message);  // This will be logged
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_LogLevelFiltering_Enabled);

static void BM_LogLevelFiltering_Disabled(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(false);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->set_min_level(thread_module::log_level::error);  // Only error and above
    
    const std::string message = "Disabled log message";
    
    for (auto _ : state) {
        logger_instance->log(thread_module::log_level::info, message);  // This will be filtered out
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_LogLevelFiltering_Disabled);

static void BM_IsEnabled_Check(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(false);
    logger_instance->set_min_level(thread_module::log_level::warning);
    
    for (auto _ : state) {
        bool enabled = logger_instance->is_enabled(thread_module::log_level::info);
        benchmark::DoNotOptimize(enabled);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_IsEnabled_Check);

BENCHMARK_MAIN();