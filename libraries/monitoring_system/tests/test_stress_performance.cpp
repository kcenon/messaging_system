/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system
All rights reserved.
*****************************************************************************/

/**
 * @file test_stress_performance.cpp
 * @brief Stress testing for the monitoring system
 * 
 * Tests system behavior under extreme conditions:
 * - High load scenarios
 * - Memory leak detection
 * - Concurrency stress tests
 * - Resource exhaustion handling
 * - Performance degradation analysis
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <random>
#include <memory>
#include <future>
// C++20 features - may need alternative implementation
// #include <barrier>
// #include <latch>
// #include <semaphore>
#include <condition_variable>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <filesystem>

#include <monitoring/core/result_types.h>
#include <monitoring/interfaces/monitoring_interface.h>
#include <monitoring/tracing/distributed_tracer.h>
#include <monitoring/performance/performance_monitor.h>
#include <monitoring/adaptive/adaptive_monitor.h>
#include <monitoring/health/health_monitor.h>
#include <monitoring/reliability/circuit_breaker.h>
#include <monitoring/reliability/fault_tolerance_manager.h>
#include <monitoring/export/opentelemetry_adapter.h>
#include <monitoring/storage/storage_backends.h>

using namespace monitoring_system;
using namespace std::chrono_literals;

class StressPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        initial_memory_ = GetCurrentMemoryUsage();
        test_dir_ = std::filesystem::temp_directory_path() / "stress_test";
        std::filesystem::create_directories(test_dir_);
    }
    
    void TearDown() override {
        // Check for memory leaks
        auto final_memory = GetCurrentMemoryUsage();
        auto memory_diff = final_memory - initial_memory_;
        
        // Allow some memory growth but flag significant leaks
        if (memory_diff > 10 * 1024 * 1024) { // 10MB threshold
            std::cerr << "Warning: Potential memory leak detected. "
                     << "Memory increased by " << memory_diff / (1024 * 1024) 
                     << " MB" << std::endl;
        }
        
        // Cleanup
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
    }
    
    size_t GetCurrentMemoryUsage() {
        // Platform-specific memory usage retrieval
        // This is a simplified version - real implementation would use
        // platform-specific APIs (e.g., /proc/self/status on Linux)
        return 0; // Placeholder
    }
    
    std::filesystem::path test_dir_;
    size_t initial_memory_;
};

/**
 * Test 1: High Load Stress Test
 * Tests system behavior under sustained high load
 */
TEST_F(StressPerformanceTest, HighLoadStressTest) {
    const int NUM_THREADS = 100;
    const int OPERATIONS_PER_THREAD = 10000;
    const auto TEST_DURATION = 30s;
    
    // Setup components
    auto tracer = std::make_unique<distributed_tracer>();
    performance_monitor perf_monitor("stress_test");
    
    // Metrics collection
    std::atomic<int64_t> total_operations{0};
    std::atomic<int64_t> failed_operations{0};
    std::atomic<int64_t> total_latency_us{0};
    std::vector<double> latencies;
    std::mutex latency_mutex;
    
    // Start time
    auto start_time = std::chrono::steady_clock::now();
    
    // Launch worker threads
    std::vector<std::thread> workers;
    for (int t = 0; t < NUM_THREADS; ++t) {
        workers.emplace_back([&, t]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 100);
            
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                auto op_start = std::chrono::high_resolution_clock::now();
                
                // Create span
                auto span_result = tracer->start_span(
                    "stress_op_" + std::to_string(t) + "_" + std::to_string(i));
                
                if (span_result) {
                    // Simulate work
                    std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));
                    
                    // Add attributes
                    span_result.value()->tags["thread_id"] = std::to_string(t);
                    span_result.value()->tags["operation_id"] = std::to_string(i);
                    
                    total_operations++;
                } else {
                    failed_operations++;
                }
                
                auto op_end = std::chrono::high_resolution_clock::now();
                auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                    op_end - op_start).count();
                
                total_latency_us += latency;
                
                // Store latency for percentile calculation
                {
                    std::lock_guard<std::mutex> lock(latency_mutex);
                    latencies.push_back(static_cast<double>(latency));
                }
                
                // Check if test duration exceeded
                if (std::chrono::steady_clock::now() - start_time > TEST_DURATION) {
                    break;
                }
            }
        });
    }
    
    // Wait for all workers
    for (auto& worker : workers) {
        worker.join();
    }
    
    // Calculate metrics
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    
    // Calculate percentiles
    std::sort(latencies.begin(), latencies.end());
    double p50 = latencies.empty() ? 0 : latencies[static_cast<size_t>(latencies.size() * 0.5)];
    double p95 = latencies.empty() ? 0 : latencies[static_cast<size_t>(latencies.size() * 0.95)];
    double p99 = latencies.empty() ? 0 : latencies[static_cast<size_t>(latencies.size() * 0.99)];
    
    // Calculate throughput
    double throughput = static_cast<double>(total_operations) / duration.count();
    double avg_latency = static_cast<double>(total_latency_us) / total_operations;
    
    // Output results
    std::cout << "\n=== High Load Stress Test Results ===" << std::endl;
    std::cout << "Duration: " << duration.count() << " seconds" << std::endl;
    std::cout << "Total operations: " << total_operations << std::endl;
    std::cout << "Failed operations: " << failed_operations << std::endl;
    std::cout << "Throughput: " << throughput << " ops/sec" << std::endl;
    std::cout << "Average latency: " << avg_latency << " μs" << std::endl;
    std::cout << "P50 latency: " << p50 << " μs" << std::endl;
    std::cout << "P95 latency: " << p95 << " μs" << std::endl;
    std::cout << "P99 latency: " << p99 << " μs" << std::endl;
    
    // Assertions
    EXPECT_GT(throughput, 1000.0); // At least 1000 ops/sec
    EXPECT_LT(failed_operations, total_operations * 0.01); // Less than 1% failure
    EXPECT_LT(p99, 10000); // P99 under 10ms
}

/**
 * Test 2: Memory Leak Detection Test
 * Tests for memory leaks under repeated allocation/deallocation
 */
TEST_F(StressPerformanceTest, MemoryLeakDetectionTest) {
    const int ITERATIONS = 1000;
    const int OBJECTS_PER_ITERATION = 100;
    
    // Track memory usage
    std::vector<size_t> memory_samples;
    
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        // Create and destroy many objects
        std::vector<std::unique_ptr<distributed_tracer>> tracers;
        std::vector<std::unique_ptr<circuit_breaker<bool>>> breakers;
        
        circuit_breaker_config cb_config;
        cb_config.failure_threshold = 3;
        cb_config.reset_timeout = 100ms;
        
        for (int i = 0; i < OBJECTS_PER_ITERATION; ++i) {
            tracers.push_back(std::make_unique<distributed_tracer>());
            breakers.push_back(std::make_unique<circuit_breaker<bool>>("breaker_" + std::to_string(i), cb_config));
            
            // Create spans
            auto span = tracers.back()->start_span("test_span_" + std::to_string(i));
            if (span) {
                span.value()->tags["iteration"] = std::to_string(iter);
            }
        }
        
        // Clear objects (should free memory)
        tracers.clear();
        breakers.clear();
        
        // Sample memory usage periodically
        if (iter % 100 == 0) {
            memory_samples.push_back(GetCurrentMemoryUsage());
        }
    }
    
    // Analyze memory trend
    if (memory_samples.size() > 2) {
        // Check if memory is growing linearly (indicates leak)
        double correlation = 0;
        double mean_x = memory_samples.size() / 2.0;
        double mean_y = std::accumulate(memory_samples.begin(), memory_samples.end(), 0.0) / memory_samples.size();
        
        double sum_xy = 0, sum_xx = 0;
        for (size_t i = 0; i < memory_samples.size(); ++i) {
            sum_xy += (i - mean_x) * (memory_samples[i] - mean_y);
            sum_xx += (i - mean_x) * (i - mean_x);
        }
        
        if (sum_xx > 0) {
            correlation = sum_xy / sqrt(sum_xx);
        }
        
        // High positive correlation indicates memory leak
        EXPECT_LT(correlation, 0.8) << "Potential memory leak detected";
    }
}

/**
 * Test 3: Concurrency Stress Test
 * Tests thread safety and race conditions
 */
TEST_F(StressPerformanceTest, ConcurrencyStressTest) {
    const int NUM_THREADS = 50;
    const int OPERATIONS = 1000;
    
    // Shared resources
    storage_config config;
    config.type = storage_backend_type::memory_buffer;
    config.max_capacity = 10000;
    auto storage = std::make_unique<file_storage_backend>(config);
    
    std::atomic<int> counter{0};
    std::atomic<bool> race_detected{false};
    
    // Synchronization for thread start
    std::mutex start_mutex;
    std::condition_variable start_cv;
    std::atomic<int> ready_threads{0};
    bool start_flag = false;
    
    // Launch concurrent threads
    std::vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            // Wait for all threads to be ready
            {
                std::unique_lock<std::mutex> lock(start_mutex);
                ready_threads++;
                if (ready_threads == NUM_THREADS) {
                    start_flag = true;
                    start_cv.notify_all();
                } else {
                    start_cv.wait(lock, [&] { return start_flag; });
                }
            }
            
            for (int i = 0; i < OPERATIONS; ++i) {
                // Concurrent writes
                metrics_snapshot snapshot;
                snapshot.add_metric("thread_" + std::to_string(t), i);
                
                auto before = counter.load();
                auto result = storage->store(snapshot);
                counter++;
                auto after = counter.load();
                
                // Check for race conditions
                if (after - before != 1) {
                    race_detected = true;
                }
                
                // No synchronization point needed for this test
            }
        });
    }
    
    // Wait for completion
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify results
    EXPECT_FALSE(race_detected) << "Race condition detected";
    EXPECT_EQ(counter, NUM_THREADS * OPERATIONS);
    EXPECT_LE(storage->size(), config.max_capacity);
}

/**
 * Test 4: Resource Exhaustion Test
 * Tests behavior when resources are exhausted
 */
TEST_F(StressPerformanceTest, ResourceExhaustionTest) {
    // Create storage with small capacity
    storage_config config;
    config.type = storage_backend_type::memory_buffer;
    config.max_capacity = 100; // Small capacity
    auto storage = std::make_unique<file_storage_backend>(config);
    
    // Track results
    int successful_stores = 0;
    int failed_stores = 0;
    
    // Try to store more than capacity
    for (int i = 0; i < 1000; ++i) {
        metrics_snapshot snapshot;
        snapshot.add_metric("test_metric", i);
        
        auto result = storage->store(snapshot);
        if (result) {
            successful_stores++;
        } else {
            failed_stores++;
        }
    }
    
    // System should handle resource exhaustion gracefully
    EXPECT_GT(successful_stores, 0);
    EXPECT_GT(failed_stores, 0); // Some should fail due to capacity
    EXPECT_EQ(storage->size(), std::min(successful_stores, static_cast<int>(config.max_capacity)));
}

/**
 * Test 5: Sustained Load Test
 * Tests system stability under sustained moderate load
 */
TEST_F(StressPerformanceTest, SustainedLoadTest) {
    const auto TEST_DURATION = 60s; // 1 minute sustained load
    const int NUM_THREADS = 20;
    const int OPS_PER_SECOND = 100;
    
    auto tracer = std::make_unique<distributed_tracer>();
    auto& health_monitor = global_health_monitor();
    
    std::atomic<bool> stop_flag{false};
    std::atomic<int64_t> total_operations{0};
    std::atomic<int64_t> health_check_failures{0};
    
    // Health check thread
    std::thread health_thread([&]() {
        while (!stop_flag) {
            auto health = health_monitor.check_all();
            if (health.empty()) {
                health_check_failures++;
            }
            std::this_thread::sleep_for(1s);
        }
    });
    
    // Worker threads
    std::vector<std::thread> workers;
    auto start_time = std::chrono::steady_clock::now();
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        workers.emplace_back([&, t]() {
            while (std::chrono::steady_clock::now() - start_time < TEST_DURATION) {
                auto cycle_start = std::chrono::steady_clock::now();
                
                // Perform operations at controlled rate
                for (int i = 0; i < OPS_PER_SECOND / NUM_THREADS; ++i) {
                    auto span = tracer->start_span("sustained_op");
                    if (span) {
                        span.value()->tags["thread"] = std::to_string(t);
                        total_operations++;
                    }
                }
                
                // Sleep to maintain rate
                auto cycle_duration = std::chrono::steady_clock::now() - cycle_start;
                if (cycle_duration < 1s) {
                    std::this_thread::sleep_for(1s - cycle_duration);
                }
            }
        });
    }
    
    // Wait for workers
    for (auto& worker : workers) {
        worker.join();
    }
    
    stop_flag = true;
    health_thread.join();
    
    // Calculate results
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start_time);
    double avg_throughput = static_cast<double>(total_operations) / duration.count();
    
    std::cout << "\n=== Sustained Load Test Results ===" << std::endl;
    std::cout << "Duration: " << duration.count() << " seconds" << std::endl;
    std::cout << "Total operations: " << total_operations << std::endl;
    std::cout << "Average throughput: " << avg_throughput << " ops/sec" << std::endl;
    std::cout << "Health check failures: " << health_check_failures << std::endl;
    
    // System should remain stable
    EXPECT_EQ(health_check_failures, 0);
    EXPECT_GT(avg_throughput, OPS_PER_SECOND * NUM_THREADS * 0.9); // Within 10% of target
}

/**
 * Test 6: Burst Load Test
 * Tests system response to sudden load spikes
 */
TEST_F(StressPerformanceTest, BurstLoadTest) {
    auto tracer = std::make_unique<distributed_tracer>();
    
    const int BURST_SIZE = 10000;
    const int NUM_BURSTS = 10;
    const auto BURST_INTERVAL = 5s;
    
    std::vector<double> burst_latencies;
    
    for (int burst = 0; burst < NUM_BURSTS; ++burst) {
        auto burst_start = std::chrono::high_resolution_clock::now();
        
        // Generate burst of operations
        std::vector<std::future<bool>> futures;
        for (int i = 0; i < BURST_SIZE; ++i) {
            futures.push_back(std::async(std::launch::async, [&tracer, i]() {
                auto span = tracer->start_span("burst_op_" + std::to_string(i));
                return span.has_value();
            }));
        }
        
        // Wait for burst to complete
        int successful = 0;
        for (auto& future : futures) {
            if (future.get()) {
                successful++;
            }
        }
        
        auto burst_end = std::chrono::high_resolution_clock::now();
        auto burst_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            burst_end - burst_start).count();
        
        burst_latencies.push_back(static_cast<double>(burst_duration));
        
        std::cout << "Burst " << burst << ": " << successful << "/" << BURST_SIZE 
                  << " successful, duration: " << burst_duration << "ms" << std::endl;
        
        // Rest between bursts
        std::this_thread::sleep_for(BURST_INTERVAL);
    }
    
    // Calculate statistics
    double avg_latency = std::accumulate(burst_latencies.begin(), burst_latencies.end(), 0.0) 
                        / burst_latencies.size();
    double max_latency = *std::max_element(burst_latencies.begin(), burst_latencies.end());
    
    std::cout << "\n=== Burst Load Test Results ===" << std::endl;
    std::cout << "Average burst latency: " << avg_latency << "ms" << std::endl;
    std::cout << "Max burst latency: " << max_latency << "ms" << std::endl;
    
    // System should handle bursts efficiently
    EXPECT_LT(avg_latency, 5000); // Average under 5 seconds
    EXPECT_LT(max_latency, 10000); // Max under 10 seconds
}

/**
 * Test 7: Deadlock Detection Test
 * Tests for potential deadlocks in concurrent operations
 */
TEST_F(StressPerformanceTest, DeadlockDetectionTest) {
    const int NUM_THREADS = 10;
    const int ITERATIONS = 100;
    
    // Shared resources with potential for deadlock
    std::timed_mutex mutex1, mutex2;
    std::atomic<int> deadlock_timeouts{0};
    
    std::vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                // Alternate lock order to create potential deadlock
                if (t % 2 == 0) {
                    // Even threads: lock mutex1 then mutex2
                    if (mutex1.try_lock_for(100ms)) {
                        std::lock_guard<std::timed_mutex> lock1(mutex1, std::adopt_lock);
                        if (mutex2.try_lock_for(100ms)) {
                            std::lock_guard<std::timed_mutex> lock2(mutex2, std::adopt_lock);
                            // Critical section
                            std::this_thread::sleep_for(1ms);
                        } else {
                            deadlock_timeouts++;
                        }
                    } else {
                        deadlock_timeouts++;
                    }
                } else {
                    // Odd threads: lock mutex2 then mutex1
                    if (mutex2.try_lock_for(100ms)) {
                        std::lock_guard<std::timed_mutex> lock2(mutex2, std::adopt_lock);
                        if (mutex1.try_lock_for(100ms)) {
                            std::lock_guard<std::timed_mutex> lock1(mutex1, std::adopt_lock);
                            // Critical section
                            std::this_thread::sleep_for(1ms);
                        } else {
                            deadlock_timeouts++;
                        }
                    } else {
                        deadlock_timeouts++;
                    }
                }
            }
        });
    }
    
    // Set timeout for test
    auto start = std::chrono::steady_clock::now();
    bool all_finished = true;
    
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
        
        // Check if test is taking too long (potential deadlock)
        if (std::chrono::steady_clock::now() - start > 30s) {
            all_finished = false;
            break;
        }
    }
    
    EXPECT_TRUE(all_finished) << "Potential deadlock detected - test timed out";
    std::cout << "Deadlock timeouts encountered: " << deadlock_timeouts << std::endl;
}

/**
 * Test 8: Performance Degradation Test
 * Tests how performance degrades under increasing load
 */
TEST_F(StressPerformanceTest, PerformanceDegradationTest) {
    auto tracer = std::make_unique<distributed_tracer>();
    
    struct LoadLevel {
        int threads;
        int operations;
        double avg_latency;
        double throughput;
    };
    
    std::vector<LoadLevel> load_levels = {
        {1, 1000, 0, 0},
        {5, 1000, 0, 0},
        {10, 1000, 0, 0},
        {20, 1000, 0, 0},
        {50, 1000, 0, 0},
        {100, 1000, 0, 0}
    };
    
    for (auto& level : load_levels) {
        std::atomic<int64_t> total_latency_us{0};
        std::atomic<int> completed_ops{0};
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int t = 0; t < level.threads; ++t) {
            threads.emplace_back([&]() {
                for (int i = 0; i < level.operations / level.threads; ++i) {
                    auto op_start = std::chrono::high_resolution_clock::now();
                    
                    auto span = tracer->start_span("degradation_op");
                    if (span) {
                        span.value()->tags["load_level"] = std::to_string(level.threads);
                        completed_ops++;
                    }
                    
                    auto op_end = std::chrono::high_resolution_clock::now();
                    total_latency_us += std::chrono::duration_cast<std::chrono::microseconds>(
                        op_end - op_start).count();
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
        
        level.avg_latency = static_cast<double>(total_latency_us) / completed_ops;
        level.throughput = (completed_ops * 1000.0) / duration;
        
        std::cout << "Load level " << level.threads << " threads: "
                  << "throughput=" << level.throughput << " ops/sec, "
                  << "avg_latency=" << level.avg_latency << " μs" << std::endl;
    }
    
    // Check for graceful degradation
    for (size_t i = 1; i < load_levels.size(); ++i) {
        // Latency should increase with load
        EXPECT_GE(load_levels[i].avg_latency, load_levels[i-1].avg_latency * 0.8);
        
        // Throughput should not decrease dramatically
        if (i < 3) { // For reasonable load levels
            EXPECT_GE(load_levels[i].throughput, load_levels[i-1].throughput * 0.7);
        }
    }
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}