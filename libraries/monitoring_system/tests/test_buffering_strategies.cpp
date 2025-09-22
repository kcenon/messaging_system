#include <gtest/gtest.h>
#include <monitoring/utils/buffering_strategy.h>
#include <monitoring/utils/buffer_manager.h>
#include <monitoring/utils/metric_storage.h>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <functional>

using namespace monitoring_system;

/**
 * @brief Test suite for Phase 3 P3: Configurable buffering strategies
 */
class BufferingStrategiesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for tests
    }
    
    void TearDown() override {
        // Common cleanup for tests
    }
    
    /**
     * @brief Create test metric
     */
    buffered_metric create_test_metric(const std::string& name, double value, uint8_t priority = 128) {
        // Create metadata with hash of name
        std::hash<std::string> hasher;
        uint32_t name_hash = static_cast<uint32_t>(hasher(name));
        metric_metadata metadata(name_hash, metric_type::gauge);
        compact_metric_value metric(metadata, value);
        return buffered_metric(std::move(metric), priority);
    }
    
    /**
     * @brief Create multiple test metrics
     */
    std::vector<buffered_metric> create_test_metrics(size_t count, const std::string& base_name = "test") {
        std::vector<buffered_metric> metrics;
        metrics.reserve(count);
        
        for (size_t i = 0; i < count; ++i) {
            std::string name = base_name + "_" + std::to_string(i);
            metrics.push_back(create_test_metric(name, static_cast<double>(i)));
        }
        
        return metrics;
    }
};

// Configuration Tests
TEST_F(BufferingStrategiesTest, BufferingConfigValidation) {
    // Test invalid configuration
    buffering_config invalid_config;
    invalid_config.max_buffer_size = 0;  // Invalid
    
    auto validation = invalid_config.validate();
    EXPECT_FALSE(validation);
    
    // Test valid configuration
    buffering_config valid_config;
    valid_config.max_buffer_size = 1024;
    valid_config.flush_threshold_size = 512;
    valid_config.flush_interval = std::chrono::milliseconds(1000);
    
    validation = valid_config.validate();
    EXPECT_TRUE(validation);
    
    // Test invalid threshold
    buffering_config invalid_threshold;
    invalid_threshold.max_buffer_size = 100;
    invalid_threshold.flush_threshold_size = 200;  // Exceeds max size
    
    validation = invalid_threshold.validate();
    EXPECT_FALSE(validation);
}

// Immediate Strategy Tests
TEST_F(BufferingStrategiesTest, ImmediateStrategy) {
    buffering_config config;
    config.strategy = buffering_strategy_type::immediate;
    
    immediate_strategy strategy(config);
    
    // Test basic properties
    EXPECT_EQ(strategy.size(), 0);
    EXPECT_FALSE(strategy.should_flush());
    
    // Add metric (should be processed immediately)
    auto metric = create_test_metric("test_metric", 42.0);
    auto result = strategy.add_metric(std::move(metric));
    EXPECT_TRUE(result);
    
    // Size should still be 0 (no buffering)
    EXPECT_EQ(strategy.size(), 0);
    
    // Flush should return empty vector
    auto flush_result = strategy.flush();
    EXPECT_TRUE(flush_result);
    EXPECT_TRUE(flush_result.value().empty());
    
    // Check statistics
    const auto& stats = strategy.get_statistics();
    EXPECT_EQ(stats.total_items_buffered.load(), 1);
    EXPECT_EQ(stats.total_items_flushed.load(), 1);
    EXPECT_EQ(stats.total_flushes.load(), 1);
}

// Fixed Size Strategy Tests
TEST_F(BufferingStrategiesTest, FixedSizeStrategy) {
    buffering_config config;
    config.strategy = buffering_strategy_type::fixed_size;
    config.max_buffer_size = 5;
    config.flush_threshold_size = 3;
    config.overflow_policy = buffer_overflow_policy::drop_oldest;
    
    fixed_size_strategy strategy(config);
    
    // Add metrics up to threshold
    for (int i = 0; i < 3; ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i));
        auto result = strategy.add_metric(std::move(metric));
        EXPECT_TRUE(result);
    }
    
    EXPECT_EQ(strategy.size(), 3);
    EXPECT_TRUE(strategy.should_flush());  // Should flush at threshold
    
    // Add more metrics to test overflow
    for (int i = 3; i < 8; ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i));
        auto result = strategy.add_metric(std::move(metric));
        EXPECT_TRUE(result);
    }
    
    EXPECT_EQ(strategy.size(), 5);  // Should not exceed max size
    
    // Flush buffer
    auto flush_result = strategy.flush();
    EXPECT_TRUE(flush_result);
    EXPECT_EQ(flush_result.value().size(), 5);
    
    // Buffer should be empty after flush
    EXPECT_EQ(strategy.size(), 0);
    EXPECT_FALSE(strategy.should_flush());
    
    // Check statistics
    const auto& stats = strategy.get_statistics();
    EXPECT_EQ(stats.total_items_buffered.load(), 8);
    EXPECT_EQ(stats.total_items_flushed.load(), 5);
    EXPECT_EQ(stats.items_dropped_overflow.load(), 3);
}

TEST_F(BufferingStrategiesTest, FixedSizeStrategyDropNewest) {
    buffering_config config;
    config.strategy = buffering_strategy_type::fixed_size;
    config.max_buffer_size = 3;
    config.overflow_policy = buffer_overflow_policy::drop_newest;
    
    fixed_size_strategy strategy(config);
    
    // Fill buffer
    for (int i = 0; i < 3; ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i));
        strategy.add_metric(std::move(metric));
    }
    
    EXPECT_EQ(strategy.size(), 3);
    
    // Try to add one more (should be dropped)
    auto metric = create_test_metric("test_overflow", 999.0);
    strategy.add_metric(std::move(metric));
    
    EXPECT_EQ(strategy.size(), 3);  // Size unchanged
    
    const auto& stats = strategy.get_statistics();
    EXPECT_GT(stats.items_dropped_overflow.load(), 0);
}

// Time Based Strategy Tests
TEST_F(BufferingStrategiesTest, TimeBasedStrategy) {
    buffering_config config;
    config.strategy = buffering_strategy_type::time_based;
    config.max_buffer_size = 100;
    config.flush_interval = std::chrono::milliseconds(100);
    
    time_based_strategy strategy(config);
    
    // Add some metrics
    for (int i = 0; i < 5; ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i));
        strategy.add_metric(std::move(metric));
    }
    
    EXPECT_EQ(strategy.size(), 5);
    EXPECT_FALSE(strategy.should_flush());  // Too soon to flush
    
    // Wait for flush interval
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    EXPECT_TRUE(strategy.should_flush());  // Should flush after interval
    
    // Flush and verify
    auto flush_result = strategy.flush();
    EXPECT_TRUE(flush_result);
    EXPECT_EQ(flush_result.value().size(), 5);
    EXPECT_EQ(strategy.size(), 0);
}

TEST_F(BufferingStrategiesTest, TimeBasedStrategyBufferFull) {
    buffering_config config;
    config.strategy = buffering_strategy_type::time_based;
    config.max_buffer_size = 3;
    config.flush_interval = std::chrono::seconds(10);  // Long interval
    
    time_based_strategy strategy(config);
    
    // Fill buffer beyond capacity
    for (int i = 0; i < 5; ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i));
        strategy.add_metric(std::move(metric));
    }
    
    // Should trigger flush due to buffer full condition
    EXPECT_TRUE(strategy.should_flush());
}

// Priority Based Strategy Tests
TEST_F(BufferingStrategiesTest, PriorityBasedStrategy) {
    buffering_config config;
    config.strategy = buffering_strategy_type::priority_based;
    config.max_buffer_size = 10;
    config.flush_priority_threshold = 200;
    
    priority_based_strategy strategy(config);
    
    // Add metrics with different priorities
    std::vector<uint8_t> priorities = {100, 150, 250, 50, 220};
    
    for (size_t i = 0; i < priorities.size(); ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i), priorities[i]);
        strategy.add_metric(std::move(metric));
    }
    
    EXPECT_EQ(strategy.size(), 5);
    EXPECT_TRUE(strategy.should_flush());  // Should flush due to high priority items
    
    // Flush and check order (should be sorted by priority)
    auto flush_result = strategy.flush();
    EXPECT_TRUE(flush_result);
    
    const auto& flushed_items = flush_result.value();
    EXPECT_EQ(flushed_items.size(), 5);
    
    // Check that items are sorted by priority (highest first)
    for (size_t i = 1; i < flushed_items.size(); ++i) {
        EXPECT_GE(flushed_items[i-1].priority, flushed_items[i].priority);
    }
}

TEST_F(BufferingStrategiesTest, PriorityBasedStrategyOverflow) {
    buffering_config config;
    config.strategy = buffering_strategy_type::priority_based;
    config.max_buffer_size = 3;
    config.overflow_policy = buffer_overflow_policy::drop_lowest_priority;
    
    priority_based_strategy strategy(config);
    
    // Add metrics with different priorities
    std::vector<uint8_t> priorities = {100, 200, 150, 250, 50};
    
    for (size_t i = 0; i < priorities.size(); ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i), priorities[i]);
        strategy.add_metric(std::move(metric));
    }
    
    EXPECT_EQ(strategy.size(), 3);  // Should maintain max size
    
    // Flush and verify highest priorities are kept
    auto flush_result = strategy.flush();
    EXPECT_TRUE(flush_result);
    
    const auto& flushed_items = flush_result.value();
    EXPECT_EQ(flushed_items.size(), 3);
    
    // Check that lowest priority items were dropped
    for (const auto& item : flushed_items) {
        EXPECT_GT(item.priority, 100);  // Priority 50 and 100 should be dropped
    }
}

// Adaptive Strategy Tests
TEST_F(BufferingStrategiesTest, AdaptiveStrategy) {
    buffering_config config;
    config.strategy = buffering_strategy_type::adaptive;
    config.max_buffer_size = 100;
    config.load_factor_threshold = 0.7;
    config.adaptive_check_interval = std::chrono::milliseconds(50);
    
    adaptive_strategy strategy(config);
    
    // Add some metrics
    for (int i = 0; i < 20; ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i));
        strategy.add_metric(std::move(metric));
    }
    
    EXPECT_EQ(strategy.size(), 20);
    
    // Add more metrics to increase load
    for (int i = 20; i < 80; ++i) {
        auto metric = create_test_metric("test_" + std::to_string(i), static_cast<double>(i));
        strategy.add_metric(std::move(metric));
    }
    
    // Should adapt to high load
    EXPECT_TRUE(strategy.should_flush());
}

// Factory Function Tests
TEST_F(BufferingStrategiesTest, StrategyFactory) {
    // Test immediate strategy creation
    buffering_config immediate_config;
    immediate_config.strategy = buffering_strategy_type::immediate;
    
    auto immediate_strategy = create_buffering_strategy(immediate_config);
    EXPECT_NE(immediate_strategy, nullptr);
    EXPECT_EQ(immediate_strategy->get_config().strategy, buffering_strategy_type::immediate);
    
    // Test fixed size strategy creation
    buffering_config fixed_config;
    fixed_config.strategy = buffering_strategy_type::fixed_size;
    fixed_config.max_buffer_size = 1024;
    
    auto fixed_strategy = create_buffering_strategy(fixed_config);
    EXPECT_NE(fixed_strategy, nullptr);
    EXPECT_EQ(fixed_strategy->get_config().strategy, buffering_strategy_type::fixed_size);
    
    // Test invalid configuration
    buffering_config invalid_config;
    invalid_config.max_buffer_size = 0;  // Invalid
    
    EXPECT_THROW(create_buffering_strategy(invalid_config), std::invalid_argument);
}

// Default Configurations Tests
TEST_F(BufferingStrategiesTest, DefaultConfigurations) {
    auto configs = create_default_buffering_configs();
    
    EXPECT_GT(configs.size(), 0);
    
    // Validate all default configurations
    for (const auto& config : configs) {
        auto validation = config.validate();
        EXPECT_TRUE(validation) << "Default configuration validation failed";
        
        // Test that we can create strategies with these configurations
        EXPECT_NO_THROW(create_buffering_strategy(config));
    }
}

// Buffer Manager Tests
TEST_F(BufferingStrategiesTest, BufferManagerBasic) {
    auto storage = std::make_shared<metric_storage>();
    
    buffer_manager_config config;
    config.enable_automatic_flushing = false;  // Manual control for testing
    
    buffer_manager manager(config, storage);
    
    // Add metrics for different metric names
    for (int i = 0; i < 5; ++i) {
        auto metadata = create_metric_metadata("cpu_usage", metric_type::gauge);
        compact_metric_value metric(metadata, 50.0 + i);
        
        auto result = manager.add_metric("cpu_usage", std::move(metric));
        EXPECT_TRUE(result);
    }
    
    // Check buffer size
    auto size_result = manager.get_buffer_size("cpu_usage");
    EXPECT_TRUE(size_result);
    EXPECT_EQ(size_result.value(), 5);
    
    // Force flush
    auto flush_result = manager.force_flush("cpu_usage");
    EXPECT_TRUE(flush_result);
    
    // Buffer should be empty after flush
    size_result = manager.get_buffer_size("cpu_usage");
    EXPECT_TRUE(size_result);
    EXPECT_EQ(size_result.value(), 0);
}

TEST_F(BufferingStrategiesTest, BufferManagerMultipleMetrics) {
    buffer_manager manager;
    
    std::vector<std::string> metric_names = {"cpu_usage", "memory_usage", "disk_io"};
    
    // Add metrics for different metric names
    for (const auto& metric_name : metric_names) {
        for (int i = 0; i < 3; ++i) {
            auto metadata = create_metric_metadata(metric_name, metric_type::gauge);
            compact_metric_value metric(metadata, static_cast<double>(i));
            
            manager.add_metric(metric_name, std::move(metric));
        }
    }
    
    // Check that all metrics have buffers
    auto buffered_metrics = manager.get_buffered_metrics();
    EXPECT_EQ(buffered_metrics.size(), 3);
    
    for (const auto& metric_name : metric_names) {
        EXPECT_NE(std::find(buffered_metrics.begin(), buffered_metrics.end(), metric_name),
                  buffered_metrics.end());
    }
    
    // Force flush all
    manager.force_flush_all();
    
    // All buffers should be empty
    for (const auto& metric_name : metric_names) {
        auto size_result = manager.get_buffer_size(metric_name);
        EXPECT_TRUE(size_result);
        EXPECT_EQ(size_result.value(), 0);
    }
}

TEST_F(BufferingStrategiesTest, BufferManagerCustomStrategy) {
    buffer_manager manager;
    
    // Configure custom buffering strategy
    buffering_config custom_config;
    custom_config.strategy = buffering_strategy_type::priority_based;
    custom_config.max_buffer_size = 10;
    custom_config.flush_priority_threshold = 200;
    
    auto result = manager.configure_metric_buffer("high_priority_metric", custom_config);
    EXPECT_TRUE(result);
    
    // Add metrics with different priorities
    std::vector<uint8_t> priorities = {100, 250, 150};
    
    for (size_t i = 0; i < priorities.size(); ++i) {
        auto metadata = create_metric_metadata("high_priority_metric", metric_type::counter);
        compact_metric_value metric(metadata, static_cast<double>(i));
        
        auto add_result = manager.add_metric("high_priority_metric", std::move(metric), priorities[i]);
        EXPECT_TRUE(add_result);
    }
    
    // Get buffer statistics
    auto stats_result = manager.get_buffer_statistics("high_priority_metric");
    EXPECT_TRUE(stats_result);
}

TEST_F(BufferingStrategiesTest, BufferManagerBackgroundProcessing) {
    auto storage = std::make_shared<metric_storage>();
    
    buffer_manager_config config;
    config.background_check_interval = std::chrono::milliseconds(50);
    config.enable_automatic_flushing = true;
    
    buffer_manager manager(config, storage);
    
    // Configure time-based strategy with short interval
    buffering_config time_config;
    time_config.strategy = buffering_strategy_type::time_based;
    time_config.flush_interval = std::chrono::milliseconds(100);
    time_config.max_buffer_size = 100;
    
    manager.configure_metric_buffer("timed_metric", time_config);
    
    // Start background processing
    auto start_result = manager.start_background_processing();
    EXPECT_TRUE(start_result);
    
    // Add metrics
    for (int i = 0; i < 5; ++i) {
        auto metadata = create_metric_metadata("timed_metric", metric_type::gauge);
        compact_metric_value metric(metadata, static_cast<double>(i));
        
        manager.add_metric("timed_metric", std::move(metric));
    }
    
    // Wait for background flush
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Buffer should be flushed automatically
    auto size_result = manager.get_buffer_size("timed_metric");
    EXPECT_TRUE(size_result);
    EXPECT_EQ(size_result.value(), 0);
    
    // Stop background processing
    manager.stop_background_processing();
}

// Thread Safety Tests
TEST_F(BufferingStrategiesTest, ThreadSafety) {
    buffer_manager manager;
    
    const int num_threads = 4;
    const int metrics_per_thread = 100;
    std::vector<std::thread> threads;
    
    // Launch multiple threads adding metrics
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&manager, t, metrics_per_thread]() {
            std::string metric_name = "thread_" + std::to_string(t) + "_metric";
            
            for (int i = 0; i < metrics_per_thread; ++i) {
                auto metadata = create_metric_metadata(metric_name, metric_type::counter);
                compact_metric_value metric(metadata, static_cast<double>(i));
                
                manager.add_metric(metric_name, std::move(metric));
                
                // Small delay to increase chance of contention
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify metrics were added
    auto buffered_metrics = manager.get_buffered_metrics();
    EXPECT_EQ(buffered_metrics.size(), num_threads);
    
    // Check each metric buffer
    for (int t = 0; t < num_threads; ++t) {
        std::string metric_name = "thread_" + std::to_string(t) + "_metric";
        auto size_result = manager.get_buffer_size(metric_name);
        EXPECT_TRUE(size_result);
        EXPECT_EQ(size_result.value(), metrics_per_thread);
    }
}