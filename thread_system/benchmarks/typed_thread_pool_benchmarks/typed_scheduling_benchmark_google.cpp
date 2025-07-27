/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file typed_scheduling_benchmark_google.cpp
 * @brief Priority-based thread pool scheduling benchmark using Google Benchmark
 * 
 * Tests priority scheduling effectiveness, fairness, and performance
 * under various load conditions and priority distributions.
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <queue>
#include <map>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <future>

#include "../../sources/typed_thread_pool/pool/typed_thread_pool.h"
#include "../../sources/utilities/core/formatter.h"

using namespace typed_thread_pool_module;
using namespace std::chrono;

// Job execution record for analysis
struct JobExecutionRecord {
    size_t job_id;
    job_types priority;
    high_resolution_clock::time_point submit_time;
    high_resolution_clock::time_point start_time;
    high_resolution_clock::time_point complete_time;
    
    auto queue_latency_ms() const {
        return duration_cast<milliseconds>(start_time - submit_time).count();
    }
    
    auto total_latency_ms() const {
        return duration_cast<milliseconds>(complete_time - submit_time).count();
    }
};

// Global storage for execution records
static std::vector<JobExecutionRecord> g_execution_records;
static std::mutex g_records_mutex;

// Helper function to submit test job
static void submit_test_job(std::shared_ptr<typed_thread_pool> pool,
                          size_t job_id, 
                          job_types priority, 
                          milliseconds work_duration) {
    auto submit_time = high_resolution_clock::now();
    
    auto job = std::make_unique<typed_job_t<job_types>>(
        priority,
        [job_id, priority, submit_time, work_duration]() -> result_void {
            auto start_time = high_resolution_clock::now();
            
            // Simulate work
            auto work_end = start_time + work_duration;
            while (high_resolution_clock::now() < work_end) {
                volatile int sum = 0;
                for (int i = 0; i < 1000; ++i) {
                    sum += i;
                }
            }
            
            auto complete_time = high_resolution_clock::now();
            
            // Record execution
            JobExecutionRecord record;
            record.job_id = job_id;
            record.priority = priority;
            record.submit_time = submit_time;
            record.start_time = start_time;
            record.complete_time = complete_time;
            
            {
                std::lock_guard<std::mutex> lock(g_records_mutex);
                g_execution_records.push_back(record);
            }
            
            return {};
        }
    );
    
    pool->enqueue(std::move(job));
}

// Helper function to setup priority pool
static std::shared_ptr<typed_thread_pool> setup_priority_pool(size_t num_workers = 4) {
    auto pool = std::make_shared<typed_thread_pool>();
    
    // Add workers with different priority responsibilities
    for (size_t i = 0; i < num_workers; ++i) {
        std::vector<job_types> responsibilities;
        if (i < num_workers / 2) {
            // High-priority workers
            responsibilities = {job_types::Critical, job_types::RealTime};
        } else {
            // General workers
            responsibilities = {job_types::RealTime, job_types::Batch, job_types::Background};
        }
        
        auto worker = std::make_unique<typed_thread_worker_t<job_types>>(
            pool, responsibilities);
        pool->enqueue(std::move(worker));
    }
    
    auto result = pool->start();
    if (result.has_error()) {
        throw std::runtime_error("Failed to start priority pool");
    }
    
    return pool;
}

/**
 * @brief Benchmark basic priority ordering
 */
static void BM_BasicPriorityOrdering(benchmark::State& state) {
    const size_t jobs_per_priority = state.range(0);
    
    for (auto _ : state) {
        g_execution_records.clear();
        
        auto pool = setup_priority_pool();
        
        std::atomic<size_t> job_counter{0};
        std::atomic<size_t> completed{0};
        
        // Submit jobs in reverse priority order
        std::vector<job_types> types = {
            job_types::Background,
            job_types::Batch, 
            job_types::RealTime,
            job_types::Critical
        };
        
        for (auto priority : types) {
            for (size_t i = 0; i < jobs_per_priority; ++i) {
                submit_test_job(pool, job_counter.fetch_add(1), priority, 
                              milliseconds(10));
            }
        }
        
        // Wait for completion
        while (completed.load() < types.size() * jobs_per_priority) {
            std::this_thread::sleep_for(milliseconds(1));
            std::lock_guard<std::mutex> lock(g_records_mutex);
            completed = g_execution_records.size();
        }
        
        pool->stop();
        
        // Analyze ordering
        std::lock_guard<std::mutex> lock(g_records_mutex);
        auto sorted_records = g_execution_records;
        std::sort(sorted_records.begin(), sorted_records.end(),
                 [](const auto& a, const auto& b) { return a.start_time < b.start_time; });
        
        // Calculate priority ordering score
        int correct_orderings = 0;
        int total_comparisons = 0;
        
        for (size_t i = 0; i < sorted_records.size(); ++i) {
            for (size_t j = i + 1; j < sorted_records.size(); ++j) {
                if (sorted_records[i].priority >= sorted_records[j].priority) {
                    correct_orderings++;
                }
                total_comparisons++;
            }
        }
        
        double ordering_score = (total_comparisons > 0) ? 
                               (correct_orderings * 100.0 / total_comparisons) : 0.0;
        state.counters["ordering_score%"] = ordering_score;
    }
    
    state.SetItemsProcessed(state.iterations() * jobs_per_priority * 4);
}
BENCHMARK(BM_BasicPriorityOrdering)
    ->Arg(25)
    ->Arg(50)
    ->Arg(100)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark priority fairness under mixed load
 */
static void BM_PriorityFairness(benchmark::State& state) {
    const size_t total_jobs = state.range(0);
    
    for (auto _ : state) {
        g_execution_records.clear();
        
        auto pool = setup_priority_pool();
        
        std::atomic<size_t> job_counter{0};
        std::atomic<size_t> completed{0};
        
        // Submit jobs with mixed priorities
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> priority_dist({10, 30, 40, 20}); // Background, Batch, RealTime, Critical
        
        auto types = std::vector<job_types>{
            job_types::Background, job_types::Batch, 
            job_types::RealTime, job_types::Critical
        };
        
        for (size_t i = 0; i < total_jobs; ++i) {
            auto priority = types[priority_dist(gen)];
            submit_test_job(pool, job_counter.fetch_add(1), priority,
                          milliseconds(5));
        }
        
        // Wait for completion
        while (completed.load() < total_jobs) {
            std::this_thread::sleep_for(milliseconds(1));
            std::lock_guard<std::mutex> lock(g_records_mutex);
            completed = g_execution_records.size();
        }
        
        pool->stop();
        
        // Analyze fairness
        std::lock_guard<std::mutex> lock(g_records_mutex);
        std::map<job_types, std::vector<double>> latencies_by_priority;
        
        for (const auto& record : g_execution_records) {
            latencies_by_priority[record.priority].push_back(record.total_latency_ms());
        }
        
        for (const auto& [priority, latencies] : latencies_by_priority) {
            if (latencies.empty()) continue;
            
            double avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            auto sorted_latencies = latencies;
            std::sort(sorted_latencies.begin(), sorted_latencies.end());
            double p95_latency = sorted_latencies[static_cast<size_t>(0.95 * (sorted_latencies.size() - 1))];
            
            std::string priority_str;
            switch (priority) {
                case job_types::Background: priority_str = "background"; break;
                case job_types::Batch: priority_str = "batch"; break;
                case job_types::RealTime: priority_str = "realtime"; break;
                case job_types::Critical: priority_str = "critical"; break;
            }
            
            state.counters[priority_str + "_avg_ms"] = avg_latency;
            state.counters[priority_str + "_p95_ms"] = p95_latency;
            state.counters[priority_str + "_count"] = latencies.size();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * total_jobs);
}
BENCHMARK(BM_PriorityFairness)
    ->Arg(500)
    ->Arg(1000)
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark priority inversion scenarios
 */
static void BM_PriorityInversion(benchmark::State& state) {
    const size_t low_priority_jobs = state.range(0);
    const size_t high_priority_jobs = state.range(1);
    
    for (auto _ : state) {
        g_execution_records.clear();
        
        auto pool = setup_priority_pool();
        
        std::atomic<size_t> job_counter{0};
        std::atomic<size_t> completed{0};
        
        // Fill queue with low priority jobs
        for (size_t i = 0; i < low_priority_jobs; ++i) {
            submit_test_job(pool, job_counter.fetch_add(1), job_types::Background,
                          milliseconds(100)); // Long work
        }
        
        // Wait a bit for some to start processing
        std::this_thread::sleep_for(milliseconds(50));
        
        // Submit high priority jobs
        for (size_t i = 0; i < high_priority_jobs; ++i) {
            submit_test_job(pool, job_counter.fetch_add(1), job_types::Critical,
                          milliseconds(10)); // Quick work
        }
        
        // Wait for completion
        const size_t total_jobs = low_priority_jobs + high_priority_jobs;
        while (completed.load() < total_jobs) {
            std::this_thread::sleep_for(milliseconds(1));
            std::lock_guard<std::mutex> lock(g_records_mutex);
            completed = g_execution_records.size();
        }
        
        pool->stop();
        
        // Analyze priority inversion
        std::lock_guard<std::mutex> lock(g_records_mutex);
        std::vector<double> critical_latencies;
        std::vector<double> background_latencies;
        
        for (const auto& record : g_execution_records) {
            if (record.priority == job_types::Critical) {
                critical_latencies.push_back(record.total_latency_ms());
            } else if (record.priority == job_types::Background) {
                background_latencies.push_back(record.total_latency_ms());
            }
        }
        
        if (!critical_latencies.empty()) {
            double avg_critical = std::accumulate(critical_latencies.begin(), 
                                                critical_latencies.end(), 0.0) / critical_latencies.size();
            state.counters["critical_avg_ms"] = avg_critical;
        }
        
        if (!background_latencies.empty()) {
            double avg_background = std::accumulate(background_latencies.begin(), 
                                                  background_latencies.end(), 0.0) / background_latencies.size();
            state.counters["background_avg_ms"] = avg_background;
        }
    }
    
    state.SetItemsProcessed(state.iterations() * (low_priority_jobs + high_priority_jobs));
}
BENCHMARK(BM_PriorityInversion)
    ->Args({50, 10})
    ->Args({100, 20})
    ->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark starvation resistance
 */
static void BM_StarvationResistance(benchmark::State& state) {
    const size_t high_priority_stream = state.range(0);
    const size_t low_priority_jobs = state.range(1);
    
    for (auto _ : state) {
        g_execution_records.clear();
        
        auto pool = setup_priority_pool();
        
        std::atomic<size_t> job_counter{0};
        std::atomic<size_t> completed{0};
        std::atomic<bool> stop_stream{false};
        
        // Continuous high-priority job stream
        std::thread high_priority_thread([&]() -> result_void {
            for (size_t i = 0; i < high_priority_stream && !stop_stream.load(); ++i) {
                submit_test_job(pool, job_counter.fetch_add(1), job_types::RealTime,
                              milliseconds(5));
                std::this_thread::sleep_for(milliseconds(8));
            }
        });
        
        // Low-priority jobs that shouldn't be starved
        std::thread low_priority_thread([&]() -> result_void {
            for (size_t i = 0; i < low_priority_jobs; ++i) {
                submit_test_job(pool, job_counter.fetch_add(1), job_types::Background,
                              milliseconds(20));
                std::this_thread::sleep_for(milliseconds(100));
            }
        });
        
        high_priority_thread.join();
        low_priority_thread.join();
        
        // Wait for completion
        const size_t total_jobs = high_priority_stream + low_priority_jobs;
        while (completed.load() < total_jobs) {
            std::this_thread::sleep_for(milliseconds(10));
            std::lock_guard<std::mutex> lock(g_records_mutex);
            completed = g_execution_records.size();
            if (completed.load() >= total_jobs * 0.95) {
                // Allow early exit if most jobs complete
                stop_stream = true;
                break;
            }
        }
        
        pool->stop();
        
        // Check if low priority jobs were executed
        std::lock_guard<std::mutex> lock(g_records_mutex);
        size_t low_priority_completed = 0;
        double max_low_priority_latency = 0.0;
        
        for (const auto& record : g_execution_records) {
            if (record.priority == job_types::Background) {
                low_priority_completed++;
                max_low_priority_latency = std::max(max_low_priority_latency, 
                                                  record.total_latency_ms());
            }
        }
        
        state.counters["low_priority_completed"] = low_priority_completed;
        state.counters["low_priority_completion%"] = (low_priority_completed * 100.0) / low_priority_jobs;
        state.counters["max_low_priority_latency_ms"] = max_low_priority_latency;
    }
    
    state.SetItemsProcessed(state.iterations() * (high_priority_stream + low_priority_jobs));
}
BENCHMARK(BM_StarvationResistance)
    ->Args({500, 50})
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

/**
 * @brief Benchmark mixed priority workload performance
 */
static void BM_MixedPriorityWorkload(benchmark::State& state) {
    const size_t duration_ms = state.range(0);
    
    for (auto _ : state) {
        g_execution_records.clear();
        
        auto pool = setup_priority_pool(8);
        
        std::atomic<size_t> job_counter{0};
        std::atomic<size_t> completed{0};
        std::atomic<bool> stop_generators{false};
        
        std::vector<std::thread> load_generators;
        
        // High-frequency low priority
        load_generators.emplace_back([&]() -> result_void {
            while (!stop_generators.load()) {
                submit_test_job(pool, job_counter.fetch_add(1), job_types::Background,
                              milliseconds(20));
                std::this_thread::sleep_for(milliseconds(10));
            }
        });
        
        // Medium-frequency normal priority
        load_generators.emplace_back([&]() -> result_void {
            while (!stop_generators.load()) {
                submit_test_job(pool, job_counter.fetch_add(1), job_types::Batch,
                              milliseconds(30));
                std::this_thread::sleep_for(milliseconds(25));
            }
        });
        
        // Low-frequency high priority
        load_generators.emplace_back([&]() -> result_void {
            while (!stop_generators.load()) {
                submit_test_job(pool, job_counter.fetch_add(1), job_types::RealTime,
                              milliseconds(15));
                std::this_thread::sleep_for(milliseconds(50));
            }
        });
        
        // Burst critical priority
        load_generators.emplace_back([&]() -> result_void {
            std::this_thread::sleep_for(milliseconds(duration_ms / 2));
            for (int i = 0; i < 20 && !stop_generators.load(); ++i) {
                submit_test_job(pool, job_counter.fetch_add(1), job_types::Critical,
                              milliseconds(5));
            }
        });
        
        // Run for specified duration
        std::this_thread::sleep_for(milliseconds(duration_ms));
        stop_generators = true;
        
        for (auto& thread : load_generators) {
            thread.join();
        }
        
        // Wait for remaining jobs
        std::this_thread::sleep_for(milliseconds(100));
        pool->stop();
        
        // Analyze performance by priority
        std::lock_guard<std::mutex> lock(g_records_mutex);
        std::map<job_types, std::pair<double, size_t>> priority_stats;
        
        for (const auto& record : g_execution_records) {
            auto& stats = priority_stats[record.priority];
            stats.first += record.total_latency_ms();
            stats.second++;
        }
        
        for (auto& [priority, stats] : priority_stats) {
            if (stats.second > 0) {
                stats.first /= stats.second; // Calculate average
                
                std::string priority_str;
                switch (priority) {
                    case job_types::Background: priority_str = "background"; break;
                    case job_types::Batch: priority_str = "batch"; break;
                    case job_types::RealTime: priority_str = "realtime"; break;
                    case job_types::Critical: priority_str = "critical"; break;
                }
                
                state.counters[priority_str + "_jobs"] = stats.second;
                state.counters[priority_str + "_avg_latency_ms"] = stats.first;
            }
        }
        
        state.counters["total_jobs"] = g_execution_records.size();
    }
    
    state.SetIterationTime(duration_ms / 1000.0);
}
BENCHMARK(BM_MixedPriorityWorkload)
    ->Arg(5000)  // 5 second test
    ->UseManualTime()
    ->Unit(benchmark::kSecond);

// Main function to run benchmarks
BENCHMARK_MAIN();