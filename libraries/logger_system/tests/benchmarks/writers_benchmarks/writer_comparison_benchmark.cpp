/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/base_writer.h>
#include <memory>
#include <string>
#include <chrono>

using namespace logger_module;

// Null writer for comparison
class null_writer : public base_writer {
public:
    void write(thread_module::log_level level, const std::string& message,
              const std::string& file, int line, const std::string& function,
              const std::chrono::system_clock::time_point& timestamp) override {
        (void)level; (void)message; (void)file; (void)line; (void)function; (void)timestamp;
    }
    void flush() override {}
};

static void BM_NullWriter(benchmark::State& state) {
    auto writer = std::make_unique<null_writer>();
    const std::string message = "Null writer message";
    auto timestamp = std::chrono::system_clock::now();
    
    for (auto _ : state) {
        writer->write(thread_module::log_level::info, message, "", 0, "", timestamp);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_NullWriter);

static void BM_ConsoleWriter_Comparison(benchmark::State& state) {
    auto writer = std::make_unique<console_writer>();
    const std::string message = "Console writer comparison";
    auto timestamp = std::chrono::system_clock::now();
    
    for (auto _ : state) {
        writer->write(thread_module::log_level::info, message, "", 0, "", timestamp);
    }
    
    writer->flush();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ConsoleWriter_Comparison);

BENCHMARK_MAIN();