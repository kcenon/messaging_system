/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file priority_scheduling_benchmark.cpp
 * @brief Comprehensive priority-based thread pool scheduling benchmark
 * 
 * Tests priority scheduling effectiveness, fairness, and performance
 * under various load conditions and priority distributions.
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <queue>
#include <map>
#include <algorithm>
#include <iomanip>

#include "../../sources/typed_thread_pool/pool/typed_thread_pool.h"
#include "../../sources/utilities/core/formatter.h"

using namespace typed_thread_pool_module;

class priority_scheduling_benchmark {
private:
    struct job_execution_record {
        size_t job_id;
        job_types priority;
        std::chrono::high_resolution_clock::time_point submit_time;
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point complete_time;
        
        auto queue_latency_ms() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(start_time - submit_time).count();
        }
        
        auto total_latency_ms() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(complete_time - submit_time).count();
        }
    };

    struct priority_metrics {
        std::map<job_types, std::vector<job_execution_record>> executions_by_priority;
        std::atomic<size_t> total_jobs_submitted{0};
        std::atomic<size_t> total_jobs_completed{0};
        std::chrono::milliseconds total_test_duration{0};
    };

    std::shared_ptr<typed_thread_pool> pool_;
    std::vector<job_execution_record> execution_records_;
    std::mutex records_mutex_;

public:
    void run_all_priority_benchmarks() {
        std::cout << "=== Type Thread Pool Scheduling Benchmark ===\n" << std::endl;

        test_basic_priority_ordering();
        test_priority_fairness();
        test_priority_inversion_scenarios();
        test_mixed_priority_loads();
        test_priority_starvation_resistance();
        test_dynamic_priority_changes();
        test_priority_vs_fifo_comparison();
    }

private:
    void test_basic_priority_ordering() {
        std::cout << "--- Basic Type Ordering Test ---" << std::endl;
        
        setup_priority_pool();
        
        priority_metrics metrics;
        execution_records_.clear();
        
        // Submit jobs in reverse priority order to test ordering
        const size_t jobs_per_priority = 100;
        std::vector<job_types> types = {
            job_types::Background,
            job_types::Batch, 
            job_types::RealTime,
            job_types::Critical
        };
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Submit all jobs quickly
        for (auto priority : types) {
            for (size_t i = 0; i < jobs_per_priority; ++i) {
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), priority, 
                              std::chrono::milliseconds(10)); // 10ms work
            }
        }
        
        // Wait for completion
        while (metrics.total_jobs_completed.load() < types.size() * jobs_per_priority) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        metrics.total_test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        analyze_priority_ordering(metrics);
        cleanup_pool();
        std::cout << std::endl;
    }

    void test_priority_fairness() {
        std::cout << "--- Type Fairness Test ---" << std::endl;
        
        setup_priority_pool();
        
        priority_metrics metrics;
        execution_records_.clear();
        
        const size_t total_jobs = 1000;
        const auto test_duration = std::chrono::seconds(30);
        
        // Continuous job submission with mixed types
        std::thread submitter([this, &metrics, total_jobs]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::discrete_distribution<> priority_dist({10, 30, 40, 20}); // Background, Batch, RealTime, Critical
            
            auto types = std::vector<job_types>{
                job_types::Background, job_types::Batch, 
                job_types::RealTime, job_types::Critical
            };
            
            for (size_t i = 0; i < total_jobs; ++i) {
                auto priority = types[priority_dist(gen)];
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), priority,
                              std::chrono::milliseconds(50)); // Longer work
                
                std::this_thread::sleep_for(std::chrono::milliseconds(25)); // Steady rate
            }
        });
        
        submitter.join();
        
        // Wait for completion
        while (metrics.total_jobs_completed.load() < total_jobs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        analyze_priority_fairness(metrics);
        cleanup_pool();
        std::cout << std::endl;
    }

    void test_priority_inversion_scenarios() {
        std::cout << "--- Type Inversion Test ---" << std::endl;
        
        setup_priority_pool();
        
        priority_metrics metrics;
        execution_records_.clear();
        
        // Simulate priority inversion scenario
        // 1. Submit many low-priority long-running jobs
        // 2. Submit high-priority jobs that should preempt
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Fill queue with low priority jobs
        for (size_t i = 0; i < 50; ++i) {
            submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::Background,
                          std::chrono::milliseconds(100)); // Long work
        }
        
        // Wait a bit for some to start processing
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Submit high priority jobs
        for (size_t i = 0; i < 10; ++i) {
            submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::Critical,
                          std::chrono::milliseconds(10)); // Quick work
        }
        
        // Wait for completion
        while (metrics.total_jobs_completed.load() < 60) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        metrics.total_test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        analyze_priority_inversion(metrics);
        cleanup_pool();
        std::cout << std::endl;
    }

    void test_mixed_priority_loads() {
        std::cout << "--- Mixed Type Load Test ---" << std::endl;
        
        setup_priority_pool();
        
        priority_metrics metrics;
        execution_records_.clear();
        
        // Different load patterns for different types
        std::vector<std::thread> load_generators;
        
        // RealTime-frequency low priority
        load_generators.emplace_back([this, &metrics]() {
            for (size_t i = 0; i < 200; ++i) {
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::Background,
                              std::chrono::milliseconds(20));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        
        // Medium-frequency normal priority
        load_generators.emplace_back([this, &metrics]() {
            for (size_t i = 0; i < 100; ++i) {
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::Batch,
                              std::chrono::milliseconds(30));
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
        });
        
        // Background-frequency high priority
        load_generators.emplace_back([this, &metrics]() {
            for (size_t i = 0; i < 50; ++i) {
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::RealTime,
                              std::chrono::milliseconds(15));
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
        
        // Burst critical priority
        load_generators.emplace_back([this, &metrics]() {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for queue to build
            for (size_t i = 0; i < 20; ++i) {
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::Critical,
                              std::chrono::milliseconds(5));
            }
        });
        
        for (auto& thread : load_generators) {
            thread.join();
        }
        
        // Wait for completion
        const size_t expected_jobs = 200 + 100 + 50 + 20;
        while (metrics.total_jobs_completed.load() < expected_jobs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        analyze_mixed_priority_performance(metrics);
        cleanup_pool();
        std::cout << std::endl;
    }

    void test_priority_starvation_resistance() {
        std::cout << "--- Type Starvation Resistance Test ---" << std::endl;
        
        setup_priority_pool();
        
        priority_metrics metrics;
        execution_records_.clear();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Continuous high-priority job stream
        std::thread high_priority_stream([this, &metrics]() {
            for (size_t i = 0; i < 500; ++i) {
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::RealTime,
                              std::chrono::milliseconds(5));
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
            }
        });
        
        // Background-priority jobs that shouldn't be starved
        std::thread low_typed_jobs([this, &metrics]() {
            for (size_t i = 0; i < 50; ++i) {
                submit_test_job(metrics.total_jobs_submitted.fetch_add(1), job_types::Background,
                              std::chrono::milliseconds(20));
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
        
        high_priority_stream.join();
        low_typed_jobs.join();
        
        // Wait for completion
        while (metrics.total_jobs_completed.load() < 550) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        metrics.total_test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        analyze_starvation_resistance(metrics);
        cleanup_pool();
        std::cout << std::endl;
    }

    void test_dynamic_priority_changes() {
        std::cout << "--- Dynamic Type Changes Test ---" << std::endl;
        
        // This test would require priority adjustment functionality
        // For now, we'll simulate the behavior
        setup_priority_pool();
        
        std::cout << "Dynamic priority adjustment not implemented in current API" << std::endl;
        std::cout << "Would test: job priority escalation, priority aging, etc." << std::endl;
        
        cleanup_pool();
        std::cout << std::endl;
    }

    void test_priority_vs_fifo_comparison() {
        std::cout << "--- Type vs FIFO Comparison Test ---" << std::endl;
        
        // Test priority pool
        auto priority_metrics = run_priority_pool_test();
        
        // Test regular pool (FIFO behavior)
        auto fifo_metrics = run_fifo_pool_test();
        
        compare_priority_vs_fifo(priority_metrics, fifo_metrics);
        std::cout << std::endl;
    }

    void submit_test_job(size_t job_id, job_types priority, std::chrono::milliseconds work_duration) {
        auto submit_time = std::chrono::high_resolution_clock::now();
        
        auto job = std::make_unique<typed_job_t<job_types>>(
            priority,
            [this, job_id, priority, submit_time, work_duration]() -> result_void {
                auto start_time = std::chrono::high_resolution_clock::now();
                
                // Simulate work
                auto work_end = start_time + work_duration;
                while (std::chrono::high_resolution_clock::now() < work_end) {
                    volatile int sum = 0;
                    for (int i = 0; i < 1000; ++i) {
                        sum += i;
                    }
                }
                
                auto complete_time = std::chrono::high_resolution_clock::now();
                
                // Record execution
                job_execution_record record;
                record.job_id = job_id;
                record.priority = priority;
                record.submit_time = submit_time;
                record.start_time = start_time;
                record.complete_time = complete_time;
                
                {
                    std::lock_guard<std::mutex> lock(records_mutex_);
                    execution_records_.push_back(record);
                }
                
                return {};
            }
        );
        
        pool_->enqueue(std::move(job));
    }

    void setup_priority_pool() {
        pool_ = std::make_shared<typed_thread_pool>();
        
        // Add workers with different priority responsibilities
        for (size_t i = 0; i < 4; ++i) {
            std::vector<job_types> responsibilities;
            if (i < 2) {
                // RealTime-priority workers
                responsibilities = {job_types::Critical, job_types::RealTime};
            } else {
                // General workers
                responsibilities = {job_types::RealTime, job_types::Batch, job_types::Background};
            }
            
            auto worker = std::make_unique<typed_thread_worker_t<job_types>>(
                pool_, responsibilities);
            pool_->enqueue(std::move(worker));
        }
        
        auto result = pool_->start();
        if (result.has_error()) {
            throw std::runtime_error("Failed to start priority pool");
        }
    }

    void cleanup_pool() {
        if (pool_) {
            pool_->stop();
            pool_.reset();
        }
    }

    void analyze_priority_ordering(const priority_metrics& metrics) {
        std::lock_guard<std::mutex> lock(records_mutex_);
        
        if (execution_records_.empty()) {
            std::cerr << "Warning: No execution records found!" << std::endl;
            return;
        }
        
        // Sort by start time to see execution order
        auto sorted_records = execution_records_;
        std::sort(sorted_records.begin(), sorted_records.end(),
                 [](const auto& a, const auto& b) { return a.start_time < b.start_time; });
        
        // Analyze if higher types were executed first
        std::map<job_types, std::vector<size_t>> execution_positions;
        for (size_t i = 0; i < sorted_records.size(); ++i) {
            execution_positions[sorted_records[i].priority].push_back(i);
        }
        
        std::cout << "Type execution analysis:" << std::endl;
        for (const auto& [priority, positions] : execution_positions) {
            double avg_position = std::accumulate(positions.begin(), positions.end(), 0.0) / positions.size();
            std::cout << format_string("  %s: avg position %.1f (lower is better)", 
                      priority_to_string(priority).c_str(), avg_position) << std::endl;
        }
        
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
        std::cout << format_string("Type ordering score: %.1f%%", ordering_score) << std::endl;
    }

    void analyze_priority_fairness(const priority_metrics& metrics) {
        std::lock_guard<std::mutex> lock(records_mutex_);
        
        std::map<job_types, std::vector<double>> latencies_by_priority;
        for (const auto& record : execution_records_) {
            latencies_by_priority[record.priority].push_back(record.total_latency_ms());
        }
        
        std::cout << "Type fairness analysis:" << std::endl;
        for (const auto& [priority, latencies] : latencies_by_priority) {
            if (latencies.empty()) continue;
            
            double avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            auto sorted_latencies = latencies;
            std::sort(sorted_latencies.begin(), sorted_latencies.end());
            double p95_latency = sorted_latencies[static_cast<size_t>(0.95 * (sorted_latencies.size() - 1))];
            
            std::cout << format_string("  %s: count=%zu, avg=%.1fms, p95=%.1fms", 
                      priority_to_string(priority).c_str(), latencies.size(), avg_latency, p95_latency) << std::endl;
        }
    }

    void analyze_priority_inversion(const priority_metrics& metrics) {
        std::lock_guard<std::mutex> lock(records_mutex_);
        
        // Find critical priority jobs and their latencies
        std::vector<double> critical_latencies;
        std::vector<double> low_latencies;
        
        for (const auto& record : execution_records_) {
            if (record.priority == job_types::Critical) {
                critical_latencies.push_back(record.total_latency_ms());
            } else if (record.priority == job_types::Background) {
                low_latencies.push_back(record.total_latency_ms());
            }
        }
        
        if (!critical_latencies.empty() && !low_latencies.empty()) {
            double avg_critical = std::accumulate(critical_latencies.begin(), critical_latencies.end(), 0.0) / critical_latencies.size();
            double avg_low = std::accumulate(low_latencies.begin(), low_latencies.end(), 0.0) / low_latencies.size();
            
            std::cout << "Type inversion analysis:" << std::endl;
            std::cout << format_string("  Critical jobs avg latency: %.1fms", avg_critical) << std::endl;
            std::cout << format_string("  Background priority jobs avg latency: %.1fms", avg_low) << std::endl;
            
            if (avg_critical < avg_low) {
                std::cout << "  Result: No significant priority inversion detected" << std::endl;
            } else {
                std::cerr << "  Warning: Result: Potential priority inversion detected!" << std::endl;
            }
        }
    }

    void analyze_mixed_priority_performance(const priority_metrics& metrics) {
        std::lock_guard<std::mutex> lock(records_mutex_);
        
        std::map<job_types, std::pair<double, size_t>> priority_stats; // avg_latency, count
        
        for (const auto& record : execution_records_) {
            auto& stats = priority_stats[record.priority];
            stats.first += record.total_latency_ms();
            stats.second++;
        }
        
        std::cout << "Mixed priority load performance:" << std::endl;
        for (auto& [priority, stats] : priority_stats) {
            if (stats.second > 0) {
                stats.first /= stats.second; // Calculate average
                std::cout << format_string("  %s: %zu jobs, avg latency: %.1fms", 
                          priority_to_string(priority).c_str(), stats.second, stats.first) << std::endl;
            }
        }
    }

    void analyze_starvation_resistance(const priority_metrics& metrics) {
        std::lock_guard<std::mutex> lock(records_mutex_);
        
        // Check if low priority jobs were executed despite high priority load
        size_t low_priority_completed = 0;
        double max_low_priority_latency = 0.0;
        
        for (const auto& record : execution_records_) {
            if (record.priority == job_types::Background) {
                low_priority_completed++;
                max_low_priority_latency = std::max(max_low_priority_latency, record.total_latency_ms());
            }
        }
        
        std::cout << "Starvation resistance analysis:" << std::endl;
        std::cout << format_string("  Background priority jobs completed: %zu", low_priority_completed) << std::endl;
        std::cout << format_string("  Max low priority latency: %.1fms", max_low_priority_latency) << std::endl;
        
        if (low_priority_completed > 40) { // Expected ~50
            std::cout << "  Result: Good starvation resistance" << std::endl;
        } else {
            std::cerr << "  Warning: Result: Possible starvation detected" << std::endl;
        }
    }

    priority_metrics run_priority_pool_test() {
        setup_priority_pool();
        
        priority_metrics metrics;
        execution_records_.clear();
        
        // Submit mixed priority jobs
        for (size_t i = 0; i < 200; ++i) {
            job_types priority = static_cast<job_types>(i % 4);
            submit_test_job(metrics.total_jobs_submitted.fetch_add(1), priority,
                          std::chrono::milliseconds(10));
        }
        
        while (metrics.total_jobs_completed.load() < 200) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        cleanup_pool();
        return metrics;
    }

    priority_metrics run_fifo_pool_test() {
        // For comparison, we'd need a FIFO-only pool
        // This is simplified - in reality you'd use regular thread_pool
        std::cout << "FIFO comparison placeholder (would use regular thread_pool)" << std::endl;
        
        priority_metrics metrics;
        return metrics;
    }

    void compare_priority_vs_fifo(const priority_metrics& priority_metrics, 
                                const priority_metrics& fifo_metrics) {
        std::cout << "Type vs FIFO comparison:" << std::endl;
        std::cout << "  (This would compare response times for different priority levels)" << std::endl;
        std::cout << "  (Type pools should show better high-priority response times)" << std::endl;
    }

    std::string priority_to_string(job_types priority) {
        switch (priority) {
            case job_types::Background: return "Background";
            case job_types::Batch: return "Batch";
            case job_types::RealTime: return "RealTime";
            case job_types::Critical: return "Critical";
            default: return "Unknown";
        }
    }
};

int main() {
    try {
        priority_scheduling_benchmark benchmark;
        benchmark.run_all_priority_benchmarks();
    } catch (const std::exception& e) {
        std::cerr << format_string("Type benchmark failed: %s", e.what()) << std::endl;
        return 1;
    }

    return 0;
}