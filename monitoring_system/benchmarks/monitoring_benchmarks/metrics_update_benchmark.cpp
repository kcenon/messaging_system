/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <monitoring/monitoring.h>
#include <memory>
#include <thread>
#include <vector>

using namespace monitoring_module;

static void BM_HighFrequencyUpdates(benchmark::State& state) {
    auto monitor_instance = std::make_unique<monitoring>(100, 10);
    monitor_instance->start();
    
    system_metrics metrics;
    metrics.cpu_usage_percentage = 50.0;
    metrics.memory_usage_bytes = 1024 * 1024 * 500;
    
    for (auto _ : state) {
        metrics.cpu_usage_percentage += 0.1; // Vary slightly
        monitor_instance->update_system_metrics(metrics);
    }
    
    monitor_instance->stop();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_HighFrequencyUpdates);

static void BM_MultithreadedUpdates(benchmark::State& state) {
    auto monitor_instance = std::make_unique<monitoring>(1000, 50);
    monitor_instance->start();
    
    const int num_threads = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&monitor_instance, t]() {
                system_metrics metrics;
                metrics.cpu_usage_percentage = 10.0 + t * 5.0;
                metrics.memory_usage_bytes = 1024 * 1024 * (100 + t * 10);
                monitor_instance->update_system_metrics(metrics);
                
                worker_metrics worker_metrics_data;
                worker_metrics_data.jobs_processed = t * 10;
                monitor_instance->update_worker_metrics(t, worker_metrics_data);
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    monitor_instance->stop();
    state.SetItemsProcessed(state.iterations() * num_threads);
}
BENCHMARK(BM_MultithreadedUpdates)->Range(2, 16);

BENCHMARK_MAIN();