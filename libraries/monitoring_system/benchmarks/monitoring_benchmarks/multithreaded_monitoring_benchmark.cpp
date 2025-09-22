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

static void BM_ConcurrentMetricsUpdates(benchmark::State& state) {
    auto monitor_instance = std::make_unique<monitoring>(1000, 50);
    monitor_instance->start();
    
    const int num_threads = state.range(0);
    const int updates_per_thread = state.range(1);
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&monitor_instance, t, updates_per_thread]() {
                for (int u = 0; u < updates_per_thread; ++u) {
                    system_metrics sys_metrics;
                    sys_metrics.cpu_usage_percentage = 10.0 + t + u * 0.1;
                    sys_metrics.memory_usage_bytes = 1024 * 1024 * (100 + t * 10 + u);
                    monitor_instance->update_system_metrics(sys_metrics);
                    
                    thread_pool_metrics pool_metrics;
                    pool_metrics.active_threads = t + 1;
                    pool_metrics.queued_jobs = u;
                    monitor_instance->update_thread_pool_metrics(pool_metrics);
                    
                    worker_metrics worker_metrics_data;
                    worker_metrics_data.jobs_processed = u;
                    monitor_instance->update_worker_metrics(t, worker_metrics_data);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    monitor_instance->stop();
    state.SetItemsProcessed(state.iterations() * num_threads * updates_per_thread);
}
BENCHMARK(BM_ConcurrentMetricsUpdates)->Ranges({{2, 8}, {10, 100}});

BENCHMARK_MAIN();