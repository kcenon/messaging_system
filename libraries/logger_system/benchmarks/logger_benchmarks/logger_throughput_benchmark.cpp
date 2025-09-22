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

static void BM_ThroughputTest(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(true);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->start();
    
    const std::string message = "Throughput test message";
    
    for (auto _ : state) {
        logger_instance->log(thread_module::log_level::info, message);
    }
    
    logger_instance->flush();
    logger_instance->stop();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ThroughputTest);

BENCHMARK_MAIN();