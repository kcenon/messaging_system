/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file multi_process_monitoring_integration.cpp
 * @brief Example demonstrating integration with multi-process monitoring system
 * 
 * This example shows how to:
 * - Use thread pools with proper instance identification
 * - Report metrics through the monitoring interface
 * - Handle multiple thread pools in the same process
 * - Integrate with process identification for multi-process scenarios
 */

#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/interfaces/monitoring_interface.h>
#include <kcenon/thread/interfaces/thread_context.h>
#include <kcenon/thread/core/callback_job.h>
#include <kcenon/thread/core/thread_worker.h>
#include <kcenon/thread/utils/formatter.h>

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <random>

using namespace kcenon::thread;
using namespace kcenon::thread;
using namespace utility_module;

// Mock implementation of multi-process monitoring
class sample_monitoring : public ::monitoring_interface::monitoring_interface {
public:
    void update_system_metrics(const ::monitoring_interface::system_metrics& metrics) override {
        std::cout << formatter::format("System metrics: CPU: {}%, Memory: {} bytes\n", 
                                      metrics.cpu_usage_percent, metrics.memory_usage_bytes);
    }
    
    void update_thread_pool_metrics(const ::monitoring_interface::thread_pool_metrics& metrics) override {
        std::cout << formatter::format("Thread pool '{}' (ID: {}): Workers: {}, Idle: {}, Pending: {}\n",
                                      metrics.pool_name, metrics.pool_instance_id,
                                      metrics.worker_threads, metrics.idle_threads, metrics.jobs_pending);
    }
    
    void update_worker_metrics(std::size_t worker_id, const ::monitoring_interface::worker_metrics& metrics) override {
        std::cout << formatter::format("Worker {}: Processed {} jobs, Total time: {} ns\n",
                                      worker_id, metrics.jobs_processed, metrics.total_processing_time_ns);
    }
    
    ::monitoring_interface::metrics_snapshot get_current_snapshot() const override { return {}; }
    std::vector<::monitoring_interface::metrics_snapshot> get_recent_snapshots(std::size_t) const override { return {}; }
    bool is_active() const override { return true; }
};

int main() {
    std::cout << "=== Multi-Process Monitoring Integration Example ===\n\n";
    
    // Create monitoring instance
    auto monitoring = std::make_shared<sample_monitoring>();
    
    // Create thread context with monitoring
    thread_context context(nullptr, monitoring);
    
    // Create multiple thread pools with unique names
    auto primary_pool = std::make_shared<thread_pool>("primary_pool", context);
    auto secondary_pool = std::make_shared<thread_pool>("secondary_pool", context);
    
    // Display pool instance IDs
    std::cout << formatter::format("Primary pool instance ID: {}\n", primary_pool->get_pool_instance_id());
    std::cout << formatter::format("Secondary pool instance ID: {}\n\n", secondary_pool->get_pool_instance_id());
    
    // Add workers then start pools
    {
        std::vector<std::unique_ptr<thread_worker>> workers;
        for (int i = 0; i < 3; ++i) workers.push_back(std::make_unique<thread_worker>());
        auto r = primary_pool->enqueue_batch(std::move(workers));
        if (r.has_error()) {
            std::cerr << "Failed to add workers to primary_pool: " << r.get_error().to_string() << "\n";
            return 1;
        }
    }
    {
        std::vector<std::unique_ptr<thread_worker>> workers;
        for (int i = 0; i < 2; ++i) workers.push_back(std::make_unique<thread_worker>());
        auto r = secondary_pool->enqueue_batch(std::move(workers));
        if (r.has_error()) {
            std::cerr << "Failed to add workers to secondary_pool: " << r.get_error().to_string() << "\n";
            return 1;
        }
    }
    
    auto start_primary = primary_pool->start();
    if (start_primary.has_error()) {
        std::cerr << "Failed to start primary_pool: " << start_primary.get_error().to_string() << "\n";
        return 1;
    }
    auto start_secondary = secondary_pool->start();
    if (start_secondary.has_error()) {
        std::cerr << "Failed to start secondary_pool: " << start_secondary.get_error().to_string() << "\n";
        return 1;
    }
    
    // Report initial metrics
    primary_pool->report_metrics();
    secondary_pool->report_metrics();
    
    std::cout << "\n--- Submitting jobs ---\n";
    
    // Submit jobs to primary pool
    for (int i = 0; i < 10; ++i) {
        auto job = std::make_unique<callback_job>(
            [i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50 + i * 10));
                std::cout << formatter::format("Primary job {} completed\n", i);
                return result_void{};
            },
            formatter::format("primary_job_{}", i)
        );
        auto r = primary_pool->enqueue(std::move(job));
        if (r.has_error()) {
            std::cerr << "enqueue to primary_pool failed: " << r.get_error().to_string() << "\n";
        }
    }
    
    // Submit jobs to secondary pool
    for (int i = 0; i < 5; ++i) {
        auto job = std::make_unique<callback_job>(
            [i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << formatter::format("Secondary job {} completed\n", i);
                return result_void{};
            },
            formatter::format("secondary_job_{}", i)
        );
        auto r = secondary_pool->enqueue(std::move(job));
        if (r.has_error()) {
            std::cerr << "enqueue to secondary_pool failed: " << r.get_error().to_string() << "\n";
        }
    }
    
    // Periodically report metrics while jobs are processing
    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "\n--- Metrics Update ---\n";
        primary_pool->report_metrics();
        secondary_pool->report_metrics();
    }
    
    // Stop pools
    std::cout << "\n--- Stopping pools ---\n";
    auto stop_primary = primary_pool->stop();
    if (stop_primary.has_error()) {
        std::cerr << "Error stopping primary_pool: " << stop_primary.get_error().to_string() << "\n";
    }
    auto stop_secondary = secondary_pool->stop();
    if (stop_secondary.has_error()) {
        std::cerr << "Error stopping secondary_pool: " << stop_secondary.get_error().to_string() << "\n";
    }
    
    // Final metrics
    std::cout << "\n--- Final Metrics ---\n";
    primary_pool->report_metrics();
    secondary_pool->report_metrics();
    
    std::cout << "\n=== Example completed ===\n";
    
    return 0;
}
