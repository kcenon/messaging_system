/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system
All rights reserved.
*****************************************************************************/

/**
 * @file test_integration_e2e.cpp
 * @brief End-to-end integration tests for the monitoring system
 * 
 * Tests complete workflows and interactions between all major components
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <random>
#include <fstream>
#include <filesystem>

#include <monitoring/core/result_types.h>
#include <monitoring/interfaces/monitoring_interface.h>
#include <monitoring/tracing/distributed_tracer.h>
#include <monitoring/performance/performance_monitor.h>
#include <monitoring/adaptive/adaptive_monitor.h>
#include <monitoring/health/health_monitor.h>
#include <monitoring/reliability/circuit_breaker.h>
#include <monitoring/reliability/retry_policy.h>
#include <monitoring/reliability/fault_tolerance_manager.h>
#include <monitoring/export/opentelemetry_adapter.h>
#include <monitoring/export/trace_exporters.h>
#include <monitoring/storage/storage_backends.h>

using namespace monitoring_system;

class IntegrationE2ETest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temp directory for test outputs
        test_dir_ = std::filesystem::temp_directory_path() / "monitoring_e2e_test";
        std::filesystem::create_directories(test_dir_);
    }
    
    void TearDown() override {
        // Cleanup test directory
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
    }
    
    std::filesystem::path test_dir_;
};

/**
 * Test 1: Storage Backend Integration
 * Multiple backends → Concurrent operations → Data consistency
 */
TEST_F(IntegrationE2ETest, StorageBackendIntegration) {
    // 1. Create multiple storage backends
    storage_config file_config;
    file_config.type = storage_backend_type::file_json;
    file_config.path = (test_dir_ / "metrics.json").string();
    file_config.max_capacity = 100;
    
    storage_config memory_config;
    memory_config.type = storage_backend_type::memory_buffer;
    memory_config.max_capacity = 100;
    
    auto file_backend = std::make_unique<file_storage_backend>(file_config);
    auto memory_backend = std::make_unique<memory_storage_backend>(memory_config);
    
    // 2. Create test data
    std::vector<metrics_snapshot> snapshots;
    for (int i = 0; i < 50; ++i) {
        metrics_snapshot snapshot;
        snapshot.add_metric("metric_" + std::to_string(i), i * 1.5);
        snapshots.push_back(snapshot);
    }
    
    // 3. Store data in both backends concurrently
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    // Thread for file backend
    threads.emplace_back([&file_backend, &snapshots, &success_count]() {
        for (const auto& snapshot : snapshots) {
            auto result = file_backend->store(snapshot);
            if (result) success_count++;
        }
    });
    
    // Thread for memory backend
    threads.emplace_back([&memory_backend, &snapshots, &success_count]() {
        for (const auto& snapshot : snapshots) {
            auto result = memory_backend->store(snapshot);
            if (result) success_count++;
        }
    });
    
    // 4. Wait for completion
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(success_count, snapshots.size() * 2);
    
    // 5. Verify data consistency
    EXPECT_EQ(file_backend->size(), 50);
    EXPECT_EQ(memory_backend->size(), 50);
    
    // 6. Test retrieval
    auto file_result = file_backend->retrieve(0);
    auto memory_result = memory_backend->retrieve(0);
    
    EXPECT_TRUE(file_result);
    EXPECT_TRUE(memory_result);
    
    // 7. Test flush
    auto flush_file = file_backend->flush();
    auto flush_memory = memory_backend->flush();
    
    EXPECT_TRUE(flush_file);
    EXPECT_TRUE(flush_memory);
}

/**
 * Test 2: Distributed Tracing End-to-End
 * Span creation → Context propagation → Export
 */
TEST_F(IntegrationE2ETest, DistributedTracingE2E) {
    // 1. Setup tracing components
    auto tracer = std::make_unique<distributed_tracer>("e2e_service");
    auto otel_adapter = create_opentelemetry_compatibility_layer("e2e_service", "1.0.0");
    
    // 2. Initialize OTEL adapter
    auto init_result = otel_adapter->initialize();
    ASSERT_TRUE(init_result);
    
    // 3. Create parent span
    auto parent_span = tracer->start_span("parent_operation");
    ASSERT_TRUE(parent_span);
    
    // 4. Create child span with parent context
    auto child_span = tracer->start_span("child_operation", parent_span.value());
    ASSERT_TRUE(child_span);
    
    // 5. Add events and attributes
    child_span.value().add_event("processing_started");
    child_span.value().set_attribute("user_id", "test_user");
    child_span.value().set_attribute("request_id", "req_123");
    
    // 6. Simulate error
    child_span.value().set_error(true, "Simulated error for testing");
    
    // 7. Export spans through OTEL adapter
    std::vector<trace_span> spans;
    spans.push_back(parent_span.value());
    spans.push_back(child_span.value());
    
    auto export_result = otel_adapter->export_spans(spans);
    EXPECT_TRUE(export_result);
    
    // 8. Verify stats
    auto stats = otel_adapter->get_stats();
    EXPECT_GT(stats.pending_spans, 0);
    
    // 9. Flush
    auto flush_result = otel_adapter->flush();
    EXPECT_TRUE(flush_result);
}

/**
 * Test 3: Health Monitoring with Fault Recovery
 * Health checks → Failure detection → Recovery → Verification
 */
TEST_F(IntegrationE2ETest, HealthMonitoringWithRecovery) {
    // 1. Setup health monitoring
    auto health_monitor = global_health_monitor::instance();
    fault_tolerance_manager ft_manager;
    
    // 2. Register health checks
    std::atomic<bool> service_healthy{true};
    
    health_monitor->register_check("database", [&service_healthy]() {
        if (service_healthy) {
            return health_check_result::healthy("Database connection OK");
        } else {
            return health_check_result::unhealthy("Database connection failed");
        }
    });
    
    health_monitor->register_check("cache", []() {
        return health_check_result::healthy("Cache service running");
    });
    
    // 3. Initial health check - should be healthy
    auto initial_health = health_monitor->check_health();
    EXPECT_TRUE(initial_health.is_healthy());
    
    // 4. Simulate failure
    service_healthy = false;
    
    // 5. Setup retry policy for recovery
    retry_policy policy;
    policy.max_attempts = 3;
    policy.initial_delay = std::chrono::milliseconds(10);
    policy.backoff_multiplier = 2.0;
    
    // 6. Attempt recovery with fault tolerance
    int recovery_attempts = 0;
    auto recovery_result = ft_manager.execute_with_retry(
        [&service_healthy, &recovery_attempts]() -> result_void {
            recovery_attempts++;
            if (recovery_attempts >= 2) {
                service_healthy = true;
                return result_void::success();
            }
            return result_void::error(monitoring_error_code::operation_failed,
                                    "Still recovering");
        },
        policy
    );
    
    EXPECT_TRUE(recovery_result);
    EXPECT_GE(recovery_attempts, 2);
    
    // 7. Verify health restored
    auto final_health = health_monitor->check_health();
    EXPECT_TRUE(final_health.is_healthy());
}

/**
 * Test 4: Performance Monitoring with Adaptive Optimization
 * Monitoring → Load detection → Adaptation → Verification
 */
TEST_F(IntegrationE2ETest, PerformanceAdaptiveOptimization) {
    // 1. Setup performance monitoring
    auto perf_monitor = performance_monitor::create();
    adaptive_monitor adapter;
    
    // 2. Configure adaptation strategy
    adaptive_config config;
    config.strategy = adaptation_strategy::balanced;
    config.cpu_threshold = 70.0;
    config.memory_threshold = 80.0;
    config.min_sampling_rate = 0.1;
    config.max_sampling_rate = 1.0;
    adapter.configure(config);
    
    // 3. Simulate varying load
    std::vector<double> cpu_loads = {30, 45, 60, 75, 85, 90, 80, 65, 50, 35};
    
    for (double load : cpu_loads) {
        // Simulate CPU load
        auto cpu_metric = perf_monitor->measure_cpu_usage();
        
        // Update adaptive monitor
        system_resource_info resources;
        resources.cpu_percent = load;
        resources.memory_percent = 60.0;
        adapter.update_system_load(resources);
        
        // Check adaptation
        auto current_load = adapter.get_current_load();
        auto sampling_rate = adapter.get_current_sampling_rate();
        
        // Higher load should reduce sampling rate
        if (load > 80) {
            EXPECT_EQ(current_load, load_level::critical);
            EXPECT_LT(sampling_rate, 0.5);
        } else if (load > 60) {
            EXPECT_EQ(current_load, load_level::high);
        } else {
            EXPECT_LE(current_load, load_level::medium);
        }
        
        // Small delay to simulate real monitoring
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 4. Verify final state
    auto final_load = adapter.get_current_load();
    EXPECT_LE(final_load, load_level::medium);
}

/**
 * Test 5: Circuit Breaker and Retry Mechanism
 * Failure injection → Circuit breaking → Recovery
 */
TEST_F(IntegrationE2ETest, CircuitBreakerAndRetry) {
    // 1. Setup resilience components
    fault_tolerance_manager ft_manager;
    circuit_breaker breaker(3, std::chrono::milliseconds(100));
    retry_policy policy;
    policy.max_attempts = 5;
    policy.initial_delay = std::chrono::milliseconds(10);
    
    // 2. Simulate component with intermittent failures
    std::atomic<int> call_count{0};
    std::atomic<bool> should_fail{true};
    
    auto unreliable_operation = [&call_count, &should_fail]() -> result_void {
        call_count++;
        
        // Fail first 3 calls, then succeed
        if (call_count <= 3 && should_fail) {
            return result_void::error(monitoring_error_code::operation_failed,
                                    "Simulated failure");
        }
        
        return result_void::success();
    };
    
    // 3. Test retry mechanism
    auto retry_result = ft_manager.execute_with_retry(unreliable_operation, policy);
    EXPECT_TRUE(retry_result);
    EXPECT_EQ(call_count, 4); // Failed 3 times, succeeded on 4th
    
    // 4. Reset and test circuit breaker
    call_count = 0;
    should_fail = true;
    
    // Trigger circuit breaker with failures
    for (int i = 0; i < 3; ++i) {
        auto cb_result = breaker.execute(unreliable_operation);
        EXPECT_FALSE(cb_result);
    }
    
    // Circuit should be open
    EXPECT_EQ(breaker.get_state(), circuit_state::open);
    
    // Further calls should fail fast (circuit open)
    auto open_result = breaker.execute(unreliable_operation);
    EXPECT_FALSE(open_result);
    
    // 5. Wait for circuit recovery
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Allow success for recovery
    should_fail = false;
    call_count = 0;
    
    // Circuit should transition to half-open and then closed
    auto recovery_result = breaker.execute(unreliable_operation);
    EXPECT_TRUE(recovery_result);
    EXPECT_EQ(breaker.get_state(), circuit_state::closed);
    
    // 6. Verify system stability after recovery
    for (int i = 0; i < 5; ++i) {
        auto stable_result = breaker.execute(unreliable_operation);
        EXPECT_TRUE(stable_result);
    }
    
    EXPECT_EQ(breaker.get_state(), circuit_state::closed);
}

/**
 * Test 6: Export Pipeline Integration
 * Trace and storage export verification
 */
TEST_F(IntegrationE2ETest, ExportPipelineIntegration) {
    // 1. Setup OTEL adapter
    auto otel_adapter = create_opentelemetry_compatibility_layer("export_test", "1.0.0");
    auto init_result = otel_adapter->initialize();
    ASSERT_TRUE(init_result);
    
    // 2. Create sample traces
    std::vector<trace_span> test_spans;
    for (int i = 0; i < 10; ++i) {
        trace_span span;
        span.trace_id = "trace_" + std::to_string(i);
        span.span_id = "span_" + std::to_string(i);
        span.operation_name = "operation_" + std::to_string(i);
        span.start_time = std::chrono::system_clock::now();
        span.end_time = span.start_time + std::chrono::milliseconds(100);
        span.set_attribute("index", std::to_string(i));
        test_spans.push_back(span);
    }
    
    // 3. Export spans
    auto export_result = otel_adapter->export_spans(test_spans);
    EXPECT_TRUE(export_result);
    
    // 4. Verify export stats
    auto stats = otel_adapter->get_stats();
    EXPECT_EQ(stats.pending_spans, test_spans.size());
    
    // 5. Create sample metrics
    monitoring_data test_data("export_test");
    test_data.add_metric("cpu_usage", 75.0);
    test_data.add_metric("memory_usage", 60.0);
    test_data.add_metric("request_count", 1000.0);
    
    // 6. Export metrics
    auto metrics_result = otel_adapter->export_metrics(test_data);
    EXPECT_TRUE(metrics_result);
    
    // 7. Verify combined stats
    stats = otel_adapter->get_stats();
    EXPECT_GT(stats.pending_metrics, 0);
    
    // 8. Flush all pending data
    auto flush_result = otel_adapter->flush();
    EXPECT_TRUE(flush_result);
    
    // 9. Verify flush completed
    stats = otel_adapter->get_stats();
    EXPECT_EQ(stats.pending_spans, 0);
    EXPECT_EQ(stats.pending_metrics, 0);
}

/**
 * Test 7: Full System Load Test
 * High volume → All components → Performance verification
 */
TEST_F(IntegrationE2ETest, FullSystemLoadTest) {
    // 1. Setup components
    auto tracer = std::make_unique<distributed_tracer>("load_test");
    auto perf_monitor = performance_monitor::create();
    auto health_monitor = global_health_monitor::instance();
    
    // 2. Configure for high load
    const int num_operations = 1000;
    const int num_threads = 10;
    
    // 3. Generate load
    auto start_time = std::chrono::steady_clock::now();
    std::vector<std::thread> load_generators;
    std::atomic<int> total_operations{0};
    
    for (int t = 0; t < num_threads; ++t) {
        load_generators.emplace_back([&tracer, &total_operations, t, num_operations]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 100.0);
            
            for (int i = 0; i < num_operations / num_threads; ++i) {
                // Create span
                auto span_result = tracer->start_span("load_test_" + std::to_string(t));
                if (span_result) {
                    span_result.value().set_attribute("thread", std::to_string(t));
                    span_result.value().set_attribute("value", std::to_string(dis(gen)));
                    total_operations++;
                }
                
                // Small delay to prevent overwhelming
                if (i % 10 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            }
        });
    }
    
    // 4. Monitor while load is running
    std::thread monitor_thread([&health_monitor, &start_time]() {
        while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(5)) {
            auto health = health_monitor->check_health();
            // System should remain healthy under load
            EXPECT_TRUE(health.is_operational());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // 5. Wait for completion
    for (auto& t : load_generators) {
        t.join();
    }
    monitor_thread.join();
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 6. Verify performance
    EXPECT_EQ(total_operations, num_operations);
    
    // Calculate throughput
    double throughput = (total_operations * 1000.0) / duration.count();
    std::cout << "Load test throughput: " << throughput << " ops/sec" << std::endl;
    
    // Should achieve reasonable throughput
    EXPECT_GT(throughput, 100.0); // At least 100 ops/sec
}

/**
 * Test 8: Cross-Component Integration
 * Multiple components working together
 */
TEST_F(IntegrationE2ETest, CrossComponentIntegration) {
    // 1. Create storage backend
    storage_config config;
    config.type = storage_backend_type::memory_buffer;
    config.max_capacity = 1000;
    auto storage = std::make_unique<memory_storage_backend>(config);
    
    // 2. Create tracer
    auto tracer = std::make_unique<distributed_tracer>("integration_test");
    
    // 3. Create performance monitor
    auto perf_monitor = performance_monitor::create();
    
    // 4. Create metrics snapshot
    metrics_snapshot snapshot;
    
    // Add performance metrics
    auto cpu_usage = perf_monitor->measure_cpu_usage();
    if (cpu_usage) {
        snapshot.add_metric("cpu_usage", cpu_usage.value());
    }
    
    auto memory_usage = perf_monitor->measure_memory_usage();
    if (memory_usage) {
        snapshot.add_metric("memory_usage", memory_usage.value());
    }
    
    // 5. Store snapshot
    auto store_result = storage->store(snapshot);
    EXPECT_TRUE(store_result);
    
    // 6. Create trace span
    auto span_result = tracer->start_span("cross_component_test");
    ASSERT_TRUE(span_result);
    
    // 7. Add metrics to span as attributes
    span_result.value().set_attribute("cpu_usage", std::to_string(cpu_usage.value_or(0)));
    span_result.value().set_attribute("memory_usage", std::to_string(memory_usage.value_or(0)));
    
    // 8. Verify storage
    EXPECT_EQ(storage->size(), 1);
    
    auto retrieved = storage->retrieve(0);
    EXPECT_TRUE(retrieved);
    
    // 9. Verify metrics in retrieved snapshot
    auto cpu_metric = retrieved.value().get_metric("cpu_usage");
    EXPECT_TRUE(cpu_metric.has_value());
    
    auto mem_metric = retrieved.value().get_metric("memory_usage");
    EXPECT_TRUE(mem_metric.has_value());
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}