/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <monitoring/monitoring.h>
#include <memory>

using namespace monitoring_module;

static void BM_ManualCollection(benchmark::State& state) {
    auto monitor_instance = std::make_unique<monitoring>(1000, 1000);
    monitor_instance->start();
    
    // Add some metrics first
    system_metrics metrics;
    metrics.cpu_usage_percentage = 50.0;
    monitor_instance->update_system_metrics(metrics);
    
    for (auto _ : state) {
        monitor_instance->collect_now();
    }
    
    monitor_instance->stop();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ManualCollection);

static void BM_GetRecentSnapshots(benchmark::State& state) {
    auto monitor_instance = std::make_unique<monitoring>(1000, 10);
    monitor_instance->start();
    
    // Fill with some data
    for (int i = 0; i < 50; ++i) {
        system_metrics metrics;
        metrics.cpu_usage_percentage = 10.0 + i;
        monitor_instance->update_system_metrics(metrics);
        monitor_instance->collect_now();
    }
    
    const int count = state.range(0);
    
    for (auto _ : state) {
        auto snapshots = monitor_instance->get_recent_snapshots(count);
        benchmark::DoNotOptimize(snapshots);
    }
    
    monitor_instance->stop();
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_GetRecentSnapshots)->Range(1, 100);

BENCHMARK_MAIN();