#include <benchmark/benchmark.h>
#include <thread_system_core/thread_pool/core/thread_pool.h>
#include <thread_system_core/typed_thread_pool/pool/typed_thread_pool.h>
#include <thread_system_core/typed_thread_pool/pool/pool_builder.h>
#include <thread_system_core/thread_base/jobs/callback_job.h>
#include <thread_system_core/typed_thread_pool/jobs/callback_typed_job.h>
#include <atomic>

using namespace thread_pool_module;
using namespace typed_thread_pool_module;
using namespace thread_module;

// Benchmark job enqueue/dequeue performance
static void BM_ThreadPool_EnqueueDequeue(benchmark::State& state) {
    auto pool = std::make_shared<thread_pool>("benchmark_pool");
    pool->start();
    
    std::atomic<int> counter(0);
    
    for (auto _ : state) {
        auto job = std::make_unique<callback_job>(
            [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); },
            "bench_job"
        );
        
        pool->enqueue(std::move(job));
    }
    
    // Wait for all jobs to complete
    while (counter.load() < state.iterations()) {
        std::this_thread::yield();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ThreadPool_EnqueueDequeue)->ThreadRange(1, 8);

// Benchmark batch enqueue performance
static void BM_ThreadPool_BatchEnqueue(benchmark::State& state) {
    auto pool = std::make_shared<thread_pool>("batch_benchmark_pool");
    pool->start();
    
    const int batch_size = state.range(0);
    std::atomic<int> counter(0);
    
    for (auto _ : state) {
        std::vector<std::unique_ptr<job>> jobs;
        jobs.reserve(batch_size);
        
        for (int i = 0; i < batch_size; ++i) {
            jobs.push_back(std::make_unique<callback_job>(
                [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); },
                "batch_job"
            ));
        }
        
        pool->enqueue_batch(std::move(jobs));
    }
    
    // Wait for all jobs to complete
    while (counter.load() < state.iterations() * batch_size) {
        std::this_thread::yield();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * batch_size);
}
BENCHMARK(BM_ThreadPool_BatchEnqueue)->Range(10, 1000);

// Benchmark typed thread pool with priorities
static void BM_TypedThreadPool_PriorityScheduling(benchmark::State& state) {
    auto pool = pool_builder()
        .with_name("priority_benchmark")
        .with_worker_count(4)
        .with_queue_strategy(queue_strategy::lockfree)
        .build();
    
    pool->start();
    
    std::atomic<int> high_counter(0);
    std::atomic<int> normal_counter(0);
    std::atomic<int> low_counter(0);
    
    for (auto _ : state) {
        // Enqueue mix of priorities
        pool->enqueue<high_job>(std::make_unique<callback_typed_job<high_job>>(
            [&high_counter]() { high_counter.fetch_add(1, std::memory_order_relaxed); },
            "high"
        ));
        
        pool->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
            [&normal_counter]() { normal_counter.fetch_add(1, std::memory_order_relaxed); },
            "normal"
        ));
        
        pool->enqueue<low_job>(std::make_unique<callback_typed_job<low_job>>(
            [&low_counter]() { low_counter.fetch_add(1, std::memory_order_relaxed); },
            "low"
        ));
    }
    
    // Wait for all jobs to complete
    while ((high_counter.load() + normal_counter.load() + low_counter.load()) < 
           state.iterations() * 3) {
        std::this_thread::yield();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations() * 3);
}
BENCHMARK(BM_TypedThreadPool_PriorityScheduling);

// Benchmark lockfree vs adaptive queue performance
static void BM_QueueStrategy_Comparison(benchmark::State& state) {
    queue_strategy strategy = state.range(0) == 0 ? 
        queue_strategy::lockfree : queue_strategy::adaptive;
    
    auto pool = pool_builder()
        .with_name("strategy_benchmark")
        .with_worker_count(4)
        .with_queue_strategy(strategy)
        .build();
    
    pool->start();
    
    std::atomic<int> counter(0);
    
    for (auto _ : state) {
        pool->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
            [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); },
            "bench"
        ));
    }
    
    // Wait for all jobs to complete
    while (counter.load() < state.iterations()) {
        std::this_thread::yield();
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_QueueStrategy_Comparison)->Arg(0)->Arg(1)
    ->ArgNames({"lockfree", "adaptive"});

// Benchmark job execution latency
static void BM_JobExecutionLatency(benchmark::State& state) {
    auto pool = std::make_shared<thread_pool>("latency_pool");
    pool->start();
    
    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::promise<void> promise;
        auto future = promise.get_future();
        
        pool->enqueue(std::make_unique<callback_job>(
            [&promise]() { promise.set_value(); },
            "latency_job"
        ));
        
        future.wait();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        state.SetIterationTime(elapsed.count() / 1e9);
    }
    
    pool->stop();
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_JobExecutionLatency)->UseManualTime();

// Benchmark contention with multiple producers
static void BM_MultiProducerContention(benchmark::State& state) {
    auto pool = std::make_shared<thread_pool>("contention_pool");
    pool->start();
    
    const int num_producers = state.range(0);
    std::atomic<int> counter(0);
    std::atomic<bool> stop(false);
    
    // Start producer threads
    std::vector<std::thread> producers;
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&pool, &counter, &stop]() {
            while (!stop.load()) {
                pool->enqueue(std::make_unique<callback_job>(
                    [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); },
                    "contention_job"
                ));
            }
        });
    }
    
    // Benchmark duration
    for (auto _ : state) {
        counter.store(0);
        auto start = std::chrono::high_resolution_clock::now();
        
        // Let it run for a fixed time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto jobs_processed = counter.load();
        state.SetIterationTime(0.1);  // 100ms
        state.counters["jobs_per_second"] = jobs_processed * 10;
    }
    
    // Stop producers
    stop.store(true);
    for (auto& t : producers) {
        t.join();
    }
    
    pool->stop();
}
BENCHMARK(BM_MultiProducerContention)->Range(1, 16)->UseManualTime();

BENCHMARK_MAIN();