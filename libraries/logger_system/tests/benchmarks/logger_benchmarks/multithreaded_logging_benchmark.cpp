/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <logger/logger.h>
#include <memory>
#include <string>
#include <thread>
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

static void BM_MultithreadedLogging(benchmark::State& state) {
    auto logger_instance = std::make_unique<logger>(true);
    logger_instance->add_writer(std::make_unique<null_writer>());
    logger_instance->start();
    
    const int num_threads = state.range(0);
    const int messages_per_thread = state.range(1);
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&logger_instance, messages_per_thread, t]() {
                for (int i = 0; i < messages_per_thread; ++i) {
                    logger_instance->log(thread_module::log_level::info,
                                       "Thread " + std::to_string(t) + " message " + std::to_string(i));
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        logger_instance->flush();
    }
    
    logger_instance->stop();
    state.SetItemsProcessed(state.iterations() * num_threads * messages_per_thread);
}
BENCHMARK(BM_MultithreadedLogging)->Ranges({{2, 16}, {10, 1000}});

BENCHMARK_MAIN();