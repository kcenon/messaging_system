/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <benchmark/benchmark.h>
#include <monitoring/storage/ring_buffer.h>
#include <thread>
#include <vector>

using namespace monitoring_module;

static void BM_RingBufferPush(benchmark::State& state) {
    const size_t buffer_size = state.range(0);
    ring_buffer<int> buffer(buffer_size);
    
    int value = 0;
    
    for (auto _ : state) {
        buffer.push(value++);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_RingBufferPush)->Range(64, 4096);

static void BM_RingBufferGetRecent(benchmark::State& state) {
    ring_buffer<int> buffer(1000);
    
    // Fill buffer first
    for (int i = 0; i < 1000; ++i) {
        buffer.push(i);
    }
    
    const size_t count = state.range(0);
    
    for (auto _ : state) {
        auto recent = buffer.get_recent(count);
        benchmark::DoNotOptimize(recent);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_RingBufferGetRecent)->Range(1, 100);

static void BM_RingBufferMultithreaded(benchmark::State& state) {
    ring_buffer<int> buffer(1000);
    const int num_threads = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&buffer, t]() {
                for (int i = 0; i < 10; ++i) {
                    buffer.push(t * 100 + i);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_threads * 10);
}
BENCHMARK(BM_RingBufferMultithreaded)->Range(2, 16);

BENCHMARK_MAIN();