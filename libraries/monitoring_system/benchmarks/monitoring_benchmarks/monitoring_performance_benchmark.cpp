/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <monitoring/monitoring.h>
#include <memory>

using namespace monitoring_module;

static std::unique_ptr<monitoring> monitor_instance;

static void setup_monitor() {
    if (!monitor_instance) {
        monitor_instance = std::make_unique<monitoring>(1000, 100);
        monitor_instance->start();
    }
}

static void cleanup_monitor() {
    if (monitor_instance) {
        monitor_instance->stop();
        monitor_instance.reset();
    }
}

static void BM_SystemMetricsUpdate(benchmark::State& state) {
    setup_monitor();
    
    system_metrics metrics;
    metrics.cpu_usage_percentage = 50.0;
    metrics.memory_usage_bytes = 1024 * 1024 * 500;
    metrics.thread_count = 8;
    metrics.process_count = 100;
    
    for (auto _ : state) {
        monitor_instance->update_system_metrics(metrics);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SystemMetricsUpdate);

static void BM_ThreadPoolMetricsUpdate(benchmark::State& state) {
    setup_monitor();
    
    thread_pool_metrics metrics;
    metrics.active_threads = 4;
    metrics.queued_jobs = 10;
    metrics.completed_jobs = 1000;
    metrics.failed_jobs = 5;
    metrics.average_execution_time_ms = 25.5;
    
    for (auto _ : state) {
        monitor_instance->update_thread_pool_metrics(metrics);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ThreadPoolMetricsUpdate);

static void BM_WorkerMetricsUpdate(benchmark::State& state) {
    setup_monitor();
    
    worker_metrics metrics;
    metrics.jobs_processed = 100;
    metrics.total_processing_time_ms = 2500;
    metrics.idle_time_ms = 500;
    metrics.last_activity = std::chrono::system_clock::now();
    
    int worker_id = 0;
    
    for (auto _ : state) {
        monitor_instance->update_worker_metrics(worker_id, metrics);
        worker_id = (worker_id + 1) % 8; // Cycle through 8 workers
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WorkerMetricsUpdate);

static void BM_GetCurrentSnapshot(benchmark::State& state) {
    setup_monitor();
    
    // Update some metrics first
    system_metrics sys_metrics;
    sys_metrics.cpu_usage_percentage = 75.0;
    monitor_instance->update_system_metrics(sys_metrics);
    
    for (auto _ : state) {
        auto snapshot = monitor_instance->get_current_snapshot();
        benchmark::DoNotOptimize(snapshot);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_GetCurrentSnapshot);

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    
    benchmark::RunSpecifiedBenchmarks();
    
    cleanup_monitor();
    return 0;
}