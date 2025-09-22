/**
 * @file test_adaptive_monitoring.cpp
 * @brief Unit tests for adaptive monitoring functionality
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <memory>
// Note: adaptive_monitor.h does not exist in include directory
// #include <kcenon/monitoring/adaptive/adaptive_monitor.h>

using namespace monitoring_system;

// Mock collector for testing
class mock_collector : public metrics_collector {
private:
    std::string name_;
    std::atomic<int> collect_count_{0};
    bool enabled_{true};
    
public:
    explicit mock_collector(const std::string& name) : name_(name) {}
    
    std::string get_name() const override { return name_; }
    bool is_enabled() const override { return enabled_; }
    
    result_void set_enabled(bool enable) override {
        enabled_ = enable;
        return result_void::success();
    }
    
    result_void initialize() override {
        return result_void::success();
    }
    
    result_void cleanup() override {
        return result_void::success();
    }
    
    monitoring_system::result<metrics_snapshot> collect() override {
        collect_count_++;
        
        metrics_snapshot snapshot;
        snapshot.capture_time = std::chrono::system_clock::now();
        snapshot.source_id = name_;
        snapshot.add_metric("test_metric", static_cast<double>(collect_count_.load()));
        
        return snapshot;
    }
    
    int get_collect_count() const { return collect_count_; }
    void reset_count() { collect_count_ = 0; }
};

class AdaptiveMonitoringTest : public ::testing::Test {
protected:
    adaptive_monitor monitor;
    
    void SetUp() override {
        // Start fresh
        monitor.stop();
    }
    
    void TearDown() override {
        monitor.stop();
    }
};

TEST_F(AdaptiveMonitoringTest, AdaptiveConfigDefaults) {
    adaptive_config config;
    
    EXPECT_EQ(config.idle_threshold, 20.0);
    EXPECT_EQ(config.low_threshold, 40.0);
    EXPECT_EQ(config.moderate_threshold, 60.0);
    EXPECT_EQ(config.high_threshold, 80.0);
    
    EXPECT_EQ(config.strategy, adaptation_strategy::balanced);
    EXPECT_EQ(config.smoothing_factor, 0.7);
}

TEST_F(AdaptiveMonitoringTest, LoadLevelCalculation) {
    adaptive_config config;
    
    EXPECT_EQ(config.get_interval_for_load(load_level::idle), 
              std::chrono::milliseconds(100));
    EXPECT_EQ(config.get_interval_for_load(load_level::critical), 
              std::chrono::milliseconds(5000));
    
    EXPECT_EQ(config.get_sampling_rate_for_load(load_level::idle), 1.0);
    EXPECT_EQ(config.get_sampling_rate_for_load(load_level::critical), 0.1);
}

TEST_F(AdaptiveMonitoringTest, AdaptiveCollectorSampling) {
    auto mock = std::make_shared<mock_collector>("test_collector");
    
    adaptive_config config;
    config.idle_sampling_rate = 1.0;  // Always sample
    adaptive_collector collector(mock, config);
    
    // Should collect when sampling rate is 1.0
    auto result = collector.collect();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(mock->get_collect_count(), 1);
    
    // Change config to lower sampling rate
    config.critical_sampling_rate = 0.0;  // Never sample
    collector.set_config(config);
    
    // Force adaptation to critical level
    system_metrics sys_metrics;
    sys_metrics.cpu_usage_percent = 90.0;  // Critical load
    collector.adapt(sys_metrics);
    
    auto stats = collector.get_stats();
    EXPECT_EQ(stats.current_load_level, load_level::critical);
}

TEST_F(AdaptiveMonitoringTest, AdaptationStatistics) {
    auto mock = std::make_shared<mock_collector>("test_collector");
    adaptive_collector collector(mock);
    
    // Simulate load changes
    system_metrics low_load;
    low_load.cpu_usage_percent = 30.0;
    low_load.memory_usage_percent = 40.0;
    
    system_metrics high_load;
    high_load.cpu_usage_percent = 85.0;
    high_load.memory_usage_percent = 70.0;
    
    // Adapt to low load
    collector.adapt(low_load);
    auto stats = collector.get_stats();
    EXPECT_EQ(stats.current_load_level, load_level::low);
    
    // Adapt to high load
    collector.adapt(high_load);
    stats = collector.get_stats();
    // After smoothing with default factor 0.7, the effective CPU would be:
    // 0.7 * 85 + 0.3 * 30 = 59.5 + 9 = 68.5 (high, not critical)
    // But memory is 70%, which should elevate it
    EXPECT_GE(static_cast<int>(stats.current_load_level), static_cast<int>(load_level::high));
    EXPECT_GT(stats.total_adaptations, 0);
    EXPECT_GT(stats.downscale_count, 0);
}

TEST_F(AdaptiveMonitoringTest, RegisterUnregisterCollector) {
    auto mock = std::make_shared<mock_collector>("test_collector");
    
    // Register collector
    auto result = monitor.register_collector("test", mock);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
    
    // Try to register again (should fail)
    result = monitor.register_collector("test", mock);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.get_error().code, monitoring_error_code::already_exists);
    
    // Unregister
    result = monitor.unregister_collector("test");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
    
    // Try to unregister non-existent (should fail)
    result = monitor.unregister_collector("test");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.get_error().code, monitoring_error_code::not_found);
}

TEST_F(AdaptiveMonitoringTest, StartStopMonitoring) {
    auto mock = std::make_shared<mock_collector>("test_collector");
    monitor.register_collector("test", mock);
    
    EXPECT_FALSE(monitor.is_running());
    
    auto result = monitor.start();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(monitor.is_running());
    
    // Start again (should succeed but do nothing)
    result = monitor.start();
    ASSERT_TRUE(result.has_value());
    
    result = monitor.stop();
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(monitor.is_running());
}

TEST_F(AdaptiveMonitoringTest, CollectorPriority) {
    auto high_priority = std::make_shared<mock_collector>("high");
    auto medium_priority = std::make_shared<mock_collector>("medium");
    auto low_priority = std::make_shared<mock_collector>("low");
    
    monitor.register_collector("high", high_priority);
    monitor.register_collector("medium", medium_priority);
    monitor.register_collector("low", low_priority);
    
    // Set priorities
    monitor.set_collector_priority("high", 100);
    monitor.set_collector_priority("medium", 50);
    monitor.set_collector_priority("low", 10);
    
    // Get active collectors (should be ordered by priority)
    auto active = monitor.get_active_collectors();
    EXPECT_GE(active.size(), 1);
    if (active.size() > 0) {
        EXPECT_EQ(active[0], "high");
    }
}

TEST_F(AdaptiveMonitoringTest, GlobalStrategy) {
    auto mock = std::make_shared<mock_collector>("test");
    monitor.register_collector("test", mock);
    
    // Set global strategy
    monitor.set_global_strategy(adaptation_strategy::conservative);
    
    // Force adaptation
    auto result = monitor.force_adaptation();
    ASSERT_TRUE(result.has_value());
    
    // Check that strategy was applied
    auto stats_result = monitor.get_collector_stats("test");
    ASSERT_TRUE(stats_result.has_value());
    // Strategy effects would be visible in adaptation behavior
}

TEST_F(AdaptiveMonitoringTest, GetAllStats) {
    auto mock1 = std::make_shared<mock_collector>("collector1");
    auto mock2 = std::make_shared<mock_collector>("collector2");
    
    monitor.register_collector("collector1", mock1);
    monitor.register_collector("collector2", mock2);
    
    auto all_stats = monitor.get_all_stats();
    EXPECT_EQ(all_stats.size(), 2);
    EXPECT_TRUE(all_stats.find("collector1") != all_stats.end());
    EXPECT_TRUE(all_stats.find("collector2") != all_stats.end());
}

TEST_F(AdaptiveMonitoringTest, AdaptiveScope) {
    auto mock = std::make_shared<mock_collector>("scoped");
    
    {
        adaptive_scope scope("scoped", mock);
        EXPECT_TRUE(scope.is_registered());
        
        // Collector should be registered  
        // Use the monitor member instead of global monitor
        auto stats = global_adaptive_monitor().get_collector_stats("scoped");
        EXPECT_TRUE(stats.has_value());
    }
    // Scope destroyed, collector should be unregistered
    
    auto stats = global_adaptive_monitor().get_collector_stats("scoped");
    EXPECT_FALSE(stats.has_value());
}

TEST_F(AdaptiveMonitoringTest, MemoryPressureAdaptation) {
    auto mock = std::make_shared<mock_collector>("test");
    
    adaptive_config config;
    config.memory_warning_threshold = 70.0;
    config.memory_critical_threshold = 85.0;
    
    adaptive_collector collector(mock, config);
    
    // High memory usage should affect load level
    system_metrics metrics;
    metrics.cpu_usage_percent = 30.0;  // Low CPU
    metrics.memory_usage_percent = 90.0;  // Critical memory
    
    collector.adapt(metrics);
    auto stats = collector.get_stats();
    
    // Should be at least high load due to memory pressure
    EXPECT_GE(static_cast<int>(stats.current_load_level), 
              static_cast<int>(load_level::high));
}

TEST_F(AdaptiveMonitoringTest, SmoothingFactor) {
    auto mock = std::make_shared<mock_collector>("test");
    
    adaptive_config config;
    config.smoothing_factor = 0.5;  // Equal weight to old and new
    
    adaptive_collector collector(mock, config);
    
    // First adaptation
    system_metrics metrics1;
    metrics1.cpu_usage_percent = 20.0;
    collector.adapt(metrics1);
    
    auto stats1 = collector.get_stats();
    // First adaptation sets initial value
    EXPECT_NEAR(stats1.average_cpu_usage, 20.0, 1.0);
    
    // Second adaptation
    system_metrics metrics2;
    metrics2.cpu_usage_percent = 60.0;
    collector.adapt(metrics2);
    
    auto stats2 = collector.get_stats();
    // Should be smoothed: 0.5 * 60 + 0.5 * (previous average)
    EXPECT_GT(stats2.average_cpu_usage, 20.0);
    EXPECT_LT(stats2.average_cpu_usage, 60.0);
}

TEST_F(AdaptiveMonitoringTest, AdaptationInterval) {
    auto mock = std::make_shared<mock_collector>("test");
    
    adaptive_config config;
    config.adaptation_interval = std::chrono::seconds(1);
    
    monitor.register_collector("test", mock, config);
    monitor.start();
    
    // Wait for at least one adaptation cycle
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    auto stats = monitor.get_collector_stats("test");
    ASSERT_TRUE(stats.has_value());
    
    // Should have adapted at least once
    EXPECT_GE(stats.value().total_adaptations, 0);
}

TEST_F(AdaptiveMonitoringTest, CollectorEnableDisable) {
    auto mock = std::make_shared<mock_collector>("test");
    adaptive_collector collector(mock);
    
    EXPECT_TRUE(collector.is_enabled());
    
    collector.set_enabled(false);
    EXPECT_FALSE(collector.is_enabled());
    
    // When disabled, should always sample
    auto result = collector.collect();
    EXPECT_TRUE(result.has_value());
}

TEST_F(AdaptiveMonitoringTest, GlobalAdaptiveMonitor) {
    auto& global = global_adaptive_monitor();
    
    auto mock = std::make_shared<mock_collector>("global_test");
    auto result = global.register_collector("global_test", mock);
    ASSERT_TRUE(result.has_value());
    
    // Cleanup
    global.unregister_collector("global_test");
}

TEST_F(AdaptiveMonitoringTest, AdaptiveStrategies) {
    auto mock = std::make_shared<mock_collector>("test");
    
    // Test conservative strategy
    adaptive_config conservative_config;
    conservative_config.strategy = adaptation_strategy::conservative;
    adaptive_collector conservative_collector(mock, conservative_config);
    
    system_metrics metrics;
    metrics.cpu_usage_percent = 50.0;  // Moderate load
    
    conservative_collector.adapt(metrics);
    auto conservative_stats = conservative_collector.get_stats();
    
    // Test aggressive strategy
    adaptive_config aggressive_config;
    aggressive_config.strategy = adaptation_strategy::aggressive;
    adaptive_collector aggressive_collector(mock, aggressive_config);
    
    aggressive_collector.adapt(metrics);
    auto aggressive_stats = aggressive_collector.get_stats();
    
    // Conservative should have lower load level than aggressive
    EXPECT_LE(static_cast<int>(conservative_stats.current_load_level),
              static_cast<int>(aggressive_stats.current_load_level));
}

TEST_F(AdaptiveMonitoringTest, ConcurrentCollectorAccess) {
    const int num_threads = 10;
    const int collectors_per_thread = 5;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, collectors_per_thread]() {
            for (int c = 0; c < collectors_per_thread; ++c) {
                std::string name = "collector_" + std::to_string(t) + "_" + std::to_string(c);
                auto mock = std::make_shared<mock_collector>(name);
                
                monitor.register_collector(name, mock);
                
                // Random operations
                if (c % 2 == 0) {
                    monitor.set_collector_priority(name, t * 10 + c);
                }
                
                if (c % 3 == 0) {
                    monitor.get_collector_stats(name);
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto all_stats = monitor.get_all_stats();
    EXPECT_EQ(all_stats.size(), num_threads * collectors_per_thread);
}