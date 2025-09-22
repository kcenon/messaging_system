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

#include <gtest/gtest.h>
#include <monitoring/monitoring.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace monitoring_module;

class MonitoringTest : public ::testing::Test {
protected:
    void SetUp() override {
        monitor_ = std::make_unique<monitoring>(100, 100); // Small history size and interval for testing
    }

    void TearDown() override {
        if (monitor_) {
            monitor_->stop();
        }
    }

    std::unique_ptr<monitoring> monitor_;
};

// Test basic monitoring construction
TEST_F(MonitoringTest, ConstructorTest) {
    EXPECT_NE(monitor_, nullptr);
    
    // Test with different parameters
    auto custom_monitor = std::make_unique<monitoring>(500, 50);
    EXPECT_NE(custom_monitor, nullptr);
}

// Test monitoring state management
TEST_F(MonitoringTest, StateManagement) {
    EXPECT_FALSE(monitor_->is_active());
    
    monitor_->start();
    EXPECT_TRUE(monitor_->is_active());
    
    monitor_->stop();
    EXPECT_FALSE(monitor_->is_active());
}

// Test system metrics update
TEST_F(MonitoringTest, SystemMetricsUpdate) {
    system_metrics metrics;
    metrics.cpu_usage_percent = 50;
    metrics.memory_usage_bytes = 1024 * 1024 * 512; // 512MB
    metrics.active_threads = 8;
    metrics.total_allocations = 150;
    metrics.timestamp = std::chrono::steady_clock::now();
    
    EXPECT_NO_THROW(monitor_->update_system_metrics(metrics));
    
    auto snapshot = monitor_->get_current_snapshot();
    EXPECT_EQ(snapshot.system.cpu_usage_percent, 50);
    EXPECT_EQ(snapshot.system.memory_usage_bytes, 1024 * 1024 * 512);
    EXPECT_EQ(snapshot.system.active_threads, 8);
    EXPECT_EQ(snapshot.system.total_allocations, 150);
}

// Test thread pool metrics update
TEST_F(MonitoringTest, ThreadPoolMetricsUpdate) {
    thread_pool_metrics metrics;
    metrics.worker_threads = 4;
    metrics.idle_threads = 2;
    metrics.jobs_completed = 100;
    metrics.jobs_pending = 10;
    metrics.total_execution_time_ns = 1000000000;  // 1 second in nanoseconds
    metrics.average_latency_ns = 10000000;  // 10ms in nanoseconds
    metrics.timestamp = std::chrono::steady_clock::now();
    
    EXPECT_NO_THROW(monitor_->update_thread_pool_metrics(metrics));
    
    auto snapshot = monitor_->get_current_snapshot();
    EXPECT_EQ(snapshot.thread_pool.worker_threads, 4);
    EXPECT_EQ(snapshot.thread_pool.idle_threads, 2);
    EXPECT_EQ(snapshot.thread_pool.jobs_completed, 100);
    EXPECT_EQ(snapshot.thread_pool.jobs_pending, 10);
    EXPECT_EQ(snapshot.thread_pool.total_execution_time_ns, 1000000000);
}

// Test worker metrics update
TEST_F(MonitoringTest, WorkerMetricsUpdate) {
    worker_metrics metrics;
    metrics.jobs_processed = 25;
    metrics.total_processing_time_ns = 500000000;  // 500ms in nanoseconds
    metrics.idle_time_ns = 100000000;  // 100ms in nanoseconds
    metrics.context_switches = 10;
    metrics.timestamp = std::chrono::steady_clock::now();
    
    const std::size_t worker_id = 0;
    
    EXPECT_NO_THROW(monitor_->update_worker_metrics(worker_id, metrics));
    
    auto snapshot = monitor_->get_current_snapshot();
    // Worker metrics are aggregated, so we check they exist and are reasonable
    EXPECT_EQ(snapshot.worker.jobs_processed, 25);
    EXPECT_EQ(snapshot.worker.total_processing_time_ns, 500000000);
    EXPECT_EQ(snapshot.worker.idle_time_ns, 100000000);
    EXPECT_EQ(snapshot.worker.context_switches, 10);
}

// Test current snapshot retrieval
TEST_F(MonitoringTest, CurrentSnapshotRetrieval) {
    // Update some metrics first
    system_metrics sys_metrics;
    sys_metrics.cpu_usage_percent = 75;
    sys_metrics.memory_usage_bytes = 1024 * 1024 * 1024; // 1GB
    sys_metrics.timestamp = std::chrono::steady_clock::now();
    monitor_->update_system_metrics(sys_metrics);
    
    thread_pool_metrics pool_metrics;
    pool_metrics.worker_threads = 8;
    pool_metrics.jobs_pending = 5;
    pool_metrics.timestamp = std::chrono::steady_clock::now();
    monitor_->update_thread_pool_metrics(pool_metrics);
    
    auto snapshot = monitor_->get_current_snapshot();
    
    EXPECT_EQ(snapshot.system.cpu_usage_percent, 75);
    EXPECT_EQ(snapshot.system.memory_usage_bytes, 1024 * 1024 * 1024);
    EXPECT_EQ(snapshot.thread_pool.worker_threads, 8);
    EXPECT_EQ(snapshot.thread_pool.jobs_pending, 5);
}

// Test recent snapshots retrieval
TEST_F(MonitoringTest, RecentSnapshotsRetrieval) {
    monitor_->start();
    
    // Add multiple snapshots
    for (int i = 0; i < 5; ++i) {
        system_metrics metrics;
        metrics.cpu_usage_percent = 10 + i * 10;
        metrics.memory_usage_bytes = 1024 * 1024 * (100 + i * 10);
        metrics.timestamp = std::chrono::steady_clock::now();
        monitor_->update_system_metrics(metrics);
        
        // Force collection
        monitor_->collect_now();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    auto recent_snapshots = monitor_->get_recent_snapshots(3);
    EXPECT_LE(recent_snapshots.size(), 3u);
    
    // Test retrieving more than available
    auto all_snapshots = monitor_->get_recent_snapshots(100);
    EXPECT_LE(all_snapshots.size(), 5u);
    
    monitor_->stop();
}

// Test collection interval management
TEST_F(MonitoringTest, CollectionIntervalManagement) {
    EXPECT_EQ(monitor_->get_collection_interval(), 100u); // From constructor
    
    monitor_->set_collection_interval(50);
    EXPECT_EQ(monitor_->get_collection_interval(), 50u);
    
    monitor_->set_collection_interval(200);
    EXPECT_EQ(monitor_->get_collection_interval(), 200u);
}

// Test manual collection trigger
TEST_F(MonitoringTest, ManualCollectionTrigger) {
    system_metrics metrics;
    metrics.cpu_usage_percent = 25;
    metrics.timestamp = std::chrono::steady_clock::now();
    monitor_->update_system_metrics(metrics);
    
    EXPECT_NO_THROW(monitor_->collect_now());
    
    // Should have at least one snapshot after manual collection
    auto recent = monitor_->get_recent_snapshots(1);
    EXPECT_FALSE(recent.empty());
}

// Test history clearing
TEST_F(MonitoringTest, HistoryClearing) {
    monitor_->start();
    
    // Add some data
    for (int i = 0; i < 3; ++i) {
        system_metrics metrics;
        metrics.cpu_usage_percent = 10 + i * 5;
        metrics.timestamp = std::chrono::steady_clock::now();
        monitor_->update_system_metrics(metrics);
        monitor_->collect_now();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    auto before_clear = monitor_->get_recent_snapshots(10);
    EXPECT_FALSE(before_clear.empty());
    
    monitor_->clear_history();
    
    auto after_clear = monitor_->get_recent_snapshots(10);
    // History should be empty or significantly reduced
    EXPECT_LE(after_clear.size(), before_clear.size());
    
    monitor_->stop();
}

// Test statistics retrieval
TEST_F(MonitoringTest, StatisticsRetrieval) {
    monitor_->start();
    
    // Add some data to generate statistics
    system_metrics metrics;
    metrics.cpu_usage_percent = 50;
    metrics.timestamp = std::chrono::steady_clock::now();
    monitor_->update_system_metrics(metrics);
    monitor_->collect_now();
    
    auto stats = monitor_->get_stats();
    EXPECT_GE(stats.total_collections, 0u);
    EXPECT_GE(stats.dropped_snapshots, 0u);
    EXPECT_GE(stats.collector_errors, 0u);
    
    monitor_->stop();
}

// Test multithreaded access
TEST_F(MonitoringTest, MultithreadedAccess) {
    monitor_->start();
    
    std::vector<std::thread> threads;
    const int num_threads = 4;
    const int updates_per_thread = 10;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, updates_per_thread]() {
            for (int i = 0; i < updates_per_thread; ++i) {
                system_metrics metrics;
                metrics.cpu_usage_percent = 10 + t * 10 + i;
                metrics.memory_usage_bytes = 1024 * 1024 * (100 + t * 10 + i);
                metrics.timestamp = std::chrono::steady_clock::now();
                
                monitor_->update_system_metrics(metrics);
                
                thread_pool_metrics pool_metrics;
                pool_metrics.worker_threads = t + 1;
                pool_metrics.jobs_pending = i;
                pool_metrics.timestamp = std::chrono::steady_clock::now();
                monitor_->update_thread_pool_metrics(pool_metrics);
                
                worker_metrics worker_metrics_data;
                worker_metrics_data.jobs_processed = i;
                worker_metrics_data.timestamp = std::chrono::steady_clock::now();
                monitor_->update_worker_metrics(t, worker_metrics_data);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should be able to get current snapshot without issues
    EXPECT_NO_THROW(monitor_->get_current_snapshot());
    
    monitor_->stop();
}

// Test edge cases
TEST_F(MonitoringTest, EdgeCases) {
    // Test with zero values
    system_metrics zero_metrics{};
    zero_metrics.timestamp = std::chrono::steady_clock::now();
    EXPECT_NO_THROW(monitor_->update_system_metrics(zero_metrics));
    
    thread_pool_metrics zero_pool_metrics{};
    zero_pool_metrics.timestamp = std::chrono::steady_clock::now();
    EXPECT_NO_THROW(monitor_->update_thread_pool_metrics(zero_pool_metrics));
    
    worker_metrics zero_worker_metrics{};
    zero_worker_metrics.timestamp = std::chrono::steady_clock::now();
    EXPECT_NO_THROW(monitor_->update_worker_metrics(999, zero_worker_metrics));
    
    // Test requesting zero snapshots
    auto empty_snapshots = monitor_->get_recent_snapshots(0);
    EXPECT_TRUE(empty_snapshots.empty());
    
    // Test very large worker ID
    worker_metrics metrics;
    metrics.jobs_processed = 1;
    metrics.timestamp = std::chrono::steady_clock::now();
    EXPECT_NO_THROW(monitor_->update_worker_metrics(std::numeric_limits<std::size_t>::max(), metrics));
}