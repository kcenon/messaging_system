/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <logger/logger.h>
#include <memory>
#include <string>
#include <vector>

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

static void BM_MemoryUsage_MultipleLoggers(benchmark::State& state) {
    const int num_loggers = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::unique_ptr<logger>> loggers;
        
        for (int i = 0; i < num_loggers; ++i) {
            auto logger_instance = std::make_unique<logger>(true);
            logger_instance->add_writer(std::make_unique<null_writer>());
            logger_instance->start();
            loggers.push_back(std::move(logger_instance));
        }
        
        // Clean up
        for (auto& logger_ptr : loggers) {
            logger_ptr->stop();
        }
        
        benchmark::DoNotOptimize(loggers);
    }
}
BENCHMARK(BM_MemoryUsage_MultipleLoggers)->Range(1, 100);

static void BM_MemoryUsage_LargeMessages(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(false);
    logger_instance->add_writer(std::make_unique<null_writer>());
    
    const int message_size = state.range(0);
    const std::string large_message(message_size, 'X');
    
    for (auto _ : state) {
        logger_instance->log(thread_module::log_level::info, large_message);
    }
    
    state.SetBytesProcessed(state.iterations() * message_size);
}
BENCHMARK(BM_MemoryUsage_LargeMessages)->Range(1024, 1024*1024);

BENCHMARK_MAIN();