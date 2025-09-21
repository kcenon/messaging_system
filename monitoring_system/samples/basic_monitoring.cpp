/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <monitoring/monitoring.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>

using namespace monitoring_module;

// Custom collector that simulates system metrics
class system_metrics_collector : public metrics_collector {
public:
    system_metrics_collector() : gen_(rd_()), cpu_dist_(20, 80), mem_dist_(1000, 4000) {}
    
    void collect(metrics_snapshot& snapshot) override {
        // Simulate CPU usage
        snapshot.system.cpu_usage_percent = cpu_dist_(gen_);
        
        // Simulate memory usage (in MB)
        snapshot.system.memory_usage_bytes = mem_dist_(gen_) * 1024 * 1024;
        
        // Simulate thread count
        snapshot.system.active_threads = std::thread::hardware_concurrency() + (gen_() % 5);
        
        // Update timestamp
        snapshot.system.timestamp = std::chrono::steady_clock::now();
    }
    
    std::string name() const override {
        return "SystemMetricsCollector";
    }
    
private:
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> cpu_dist_;
    std::uniform_int_distribution<> mem_dist_;
};

void print_metrics(const metrics_snapshot& snapshot) {
    std::cout << "=== Metrics Snapshot ===" << std::endl;
    
    // System metrics
    std::cout << "System:" << std::endl;
    std::cout << "  CPU Usage: " << snapshot.system.cpu_usage_percent << "%" << std::endl;
    std::cout << "  Memory: " << (snapshot.system.memory_usage_bytes / (1024.0 * 1024.0)) 
              << " MB" << std::endl;
    std::cout << "  Active Threads: " << snapshot.system.active_threads << std::endl;
    
    // Thread pool metrics
    if (snapshot.thread_pool.worker_threads > 0) {
        std::cout << "Thread Pool:" << std::endl;
        std::cout << "  Jobs Completed: " << snapshot.thread_pool.jobs_completed << std::endl;
        std::cout << "  Jobs Pending: " << snapshot.thread_pool.jobs_pending << std::endl;
        std::cout << "  Workers: " << snapshot.thread_pool.worker_threads 
                  << " (" << snapshot.thread_pool.idle_threads << " idle)" << std::endl;
    }
    
    // Worker metrics
    if (snapshot.worker.jobs_processed > 0) {
        std::cout << "Workers (aggregated):" << std::endl;
        std::cout << "  Jobs Processed: " << snapshot.worker.jobs_processed << std::endl;
        std::cout << "  Total Time: " << (snapshot.worker.total_processing_time_ns / 1000000.0)
                  << " ms" << std::endl;
    }
    
    std::cout << std::endl;
}

void basic_monitoring_example() {
    std::cout << "\n=== Basic Monitoring Example ===\n" << std::endl;
    
    // Create monitoring instance
    auto monitor = std::make_shared<monitoring>(100, 500); // 100 snapshots, 500ms interval
    
    // Add custom collector
    monitor->add_collector(std::make_unique<system_metrics_collector>());
    
    // Start monitoring
    monitor->start();
    
    // Simulate some activity
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        
        // Get current snapshot
        auto snapshot = monitor->get_current_snapshot();
        print_metrics(snapshot);
    }
    
    // Stop monitoring
    monitor->stop();
}

void manual_metrics_example() {
    std::cout << "\n=== Manual Metrics Update Example ===\n" << std::endl;
    
    auto monitor = std::make_shared<monitoring>();
    
    // Manually update metrics
    system_metrics sys_metrics;
    sys_metrics.cpu_usage_percent = 45;
    sys_metrics.memory_usage_bytes = 2048ULL * 1024 * 1024; // 2GB
    sys_metrics.active_threads = 8;
    monitor->update_system_metrics(sys_metrics);
    
    thread_pool_metrics pool_metrics;
    pool_metrics.jobs_completed = 1000;
    pool_metrics.jobs_pending = 50;
    pool_metrics.worker_threads = 4;
    pool_metrics.idle_threads = 1;
    monitor->update_thread_pool_metrics(pool_metrics);
    
    worker_metrics worker_m;
    worker_m.jobs_processed = 250;
    worker_m.total_processing_time_ns = 500000000; // 500ms
    monitor->update_worker_metrics(0, worker_m);
    
    // Get and print snapshot
    auto snapshot = monitor->get_current_snapshot();
    print_metrics(snapshot);
}

void historical_data_example() {
    std::cout << "\n=== Historical Data Example ===\n" << std::endl;
    
    auto monitor = std::make_shared<monitoring>(50, 100); // Keep 50 snapshots
    monitor->add_collector(std::make_unique<system_metrics_collector>());
    monitor->start();
    
    // Collect data for a while
    std::cout << "Collecting data for 2 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Get historical snapshots
    auto history = monitor->get_recent_snapshots(10);
    
    std::cout << "\nHistorical CPU usage (newest to oldest):" << std::endl;
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << "  " << i << ": " << history[i].system.cpu_usage_percent << "%" << std::endl;
    }
    
    monitor->stop();
    
    // Get monitoring statistics
    auto stats = monitor->get_stats();
    std::cout << "\nMonitoring Statistics:" << std::endl;
    std::cout << "  Total Collections: " << stats.total_collections << std::endl;
    std::cout << "  Dropped Snapshots: " << stats.dropped_snapshots << std::endl;
    std::cout << "  Collector Errors: " << stats.collector_errors << std::endl;
}

void performance_monitoring_example() {
    std::cout << "\n=== Performance Monitoring Example ===\n" << std::endl;
    
    auto monitor = std::make_shared<monitoring>(1000, 10); // 10ms interval
    monitor->start();
    
    // Simulate workload
    std::cout << "Simulating workload..." << std::endl;
    
    for (int worker = 0; worker < 4; ++worker) {
        std::thread([monitor, worker]() {
            for (int i = 0; i < 100; ++i) {
                auto start = std::chrono::steady_clock::now();
                
                // Simulate work
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                
                // Update worker metrics
                worker_metrics metrics;
                metrics.jobs_processed = 1;
                metrics.total_processing_time_ns = duration.count();
                monitor->update_worker_metrics(worker, metrics);
            }
        }).detach();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Get final metrics
    auto snapshot = monitor->get_current_snapshot();
    print_metrics(snapshot);
    
    monitor->stop();
}

int main() {
    try {
        basic_monitoring_example();
        manual_metrics_example();
        historical_data_example();
        performance_monitoring_example();
        
        std::cout << "\n=== All examples completed successfully! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}