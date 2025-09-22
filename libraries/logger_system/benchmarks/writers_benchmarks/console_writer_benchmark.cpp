/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <logger/writers/console_writer.h>
#include <memory>
#include <string>
#include <chrono>

using namespace logger_module;

static void BM_ConsoleWriter_Write(benchmark::State& state) {
    auto writer = std::make_unique<console_writer>();
    const std::string message = "Console writer benchmark message";
    auto timestamp = std::chrono::system_clock::now();
    
    for (auto _ : state) {
        writer->write(thread_module::log_level::info, message, "", 0, "", timestamp);
    }
    
    writer->flush();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ConsoleWriter_Write);

static void BM_ConsoleWriter_WithColor(benchmark::State& state) {
    auto writer = std::make_unique<console_writer>();
    writer->set_use_color(true);
    
    const std::string message = "Colored console message";
    auto timestamp = std::chrono::system_clock::now();
    
    for (auto _ : state) {
        writer->write(thread_module::log_level::error, message, "", 0, "", timestamp);
    }
    
    writer->flush();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ConsoleWriter_WithColor);

static void BM_ConsoleWriter_WithLocation(benchmark::State& state) {
    auto writer = std::make_unique<console_writer>();
    const std::string message = "Message with location";
    auto timestamp = std::chrono::system_clock::now();
    
    for (auto _ : state) {
        writer->write(thread_module::log_level::warning, message, 
                     __FILE__, __LINE__, __func__, timestamp);
    }
    
    writer->flush();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ConsoleWriter_WithLocation);

BENCHMARK_MAIN();