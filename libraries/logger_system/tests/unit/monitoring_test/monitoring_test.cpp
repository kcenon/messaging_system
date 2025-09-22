/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file monitoring_test.cpp
 * @brief Unit tests for monitoring implementations
 */

#include <gtest/gtest.h>
#include "../../sources/logger/monitoring/basic_monitor.h"
#include "../../sources/logger/monitoring/monitoring_factory.h"
#include <thread>
#include <chrono>

using namespace logger_module;
using namespace std::chrono_literals;

// Test fixture for monitoring tests
class monitoring_test : public ::testing::Test {
protected:
    std::unique_ptr<monitoring_interface> monitor_;
    
    void SetUp() override {
        monitor_ = std::make_unique<basic_monitor>();
    }
    
    void TearDown() override {
        monitor_.reset();
    }
};

// Basic Monitor Tests
TEST_F(monitoring_test, basic_monitor_initial_state) {
    EXPECT_TRUE(monitor_->is_enabled());
    EXPECT_EQ(monitor_->get_backend_name(), "basic");
}

TEST_F(monitoring_test, basic_monitor_enable_disable) {
    // Initially enabled
    EXPECT_TRUE(monitor_->is_enabled());
    
    // Disable
    auto result = monitor_->set_enabled(false);
    ASSERT_TRUE(result);
    EXPECT_FALSE(monitor_->is_enabled());
    
    // Re-enable
    result = monitor_->set_enabled(true);
    ASSERT_TRUE(result);
    EXPECT_TRUE(monitor_->is_enabled());
}

TEST_F(monitoring_test, basic_monitor_counter_increment) {
    // Increment a counter
    monitor_->increment_counter("test_counter", 1.0);
    monitor_->increment_counter("test_counter", 2.0);
    monitor_->increment_counter("test_counter", 3.0);
    
    // Collect metrics
    auto result = monitor_->collect_metrics();
    ASSERT_TRUE(result);
    
    auto& data = result.value();
    auto& metrics = data.get_metrics();
    
    // Find the counter
    bool found = false;
    for (const auto& metric : metrics) {
        if (metric.name == "test_counter") {
            EXPECT_EQ(metric.value, 6.0);  // 1 + 2 + 3
            EXPECT_EQ(metric.type, metric_type::counter);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Counter 'test_counter' not found in metrics";
}

TEST_F(monitoring_test, basic_monitor_gauge_update) {
    // Update a gauge
    monitor_->update_gauge("test_gauge", 10.0);
    monitor_->update_gauge("test_gauge", 20.0);
    monitor_->update_gauge("test_gauge", 15.0);
    
    // Collect metrics
    auto result = monitor_->collect_metrics();
    ASSERT_TRUE(result);
    
    auto& data = result.value();
    auto& metrics = data.get_metrics();
    
    // Find the gauge
    bool found = false;
    for (const auto& metric : metrics) {
        if (metric.name == "test_gauge") {
            EXPECT_EQ(metric.value, 15.0);  // Last value
            EXPECT_EQ(metric.type, metric_type::gauge);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Gauge 'test_gauge' not found in metrics";
}

TEST_F(monitoring_test, basic_monitor_histogram_recording) {
    // Record histogram values
    monitor_->record_histogram("test_histogram", 10.0);
    monitor_->record_histogram("test_histogram", 20.0);
    monitor_->record_histogram("test_histogram", 30.0);
    monitor_->record_histogram("test_histogram", 40.0);
    monitor_->record_histogram("test_histogram", 50.0);
    
    // Collect metrics
    auto result = monitor_->collect_metrics();
    ASSERT_TRUE(result);
    
    auto& data = result.value();
    auto& metrics = data.get_metrics();
    
    // Check histogram summary metrics
    bool found_avg = false, found_min = false, found_max = false, found_count = false;
    
    for (const auto& metric : metrics) {
        if (metric.name == "test_histogram_avg") {
            EXPECT_EQ(metric.value, 30.0);  // Average
            EXPECT_EQ(metric.type, metric_type::summary);
            found_avg = true;
        } else if (metric.name == "test_histogram_min") {
            EXPECT_EQ(metric.value, 10.0);  // Min
            EXPECT_EQ(metric.type, metric_type::summary);
            found_min = true;
        } else if (metric.name == "test_histogram_max") {
            EXPECT_EQ(metric.value, 50.0);  // Max
            EXPECT_EQ(metric.type, metric_type::summary);
            found_max = true;
        } else if (metric.name == "test_histogram_count") {
            EXPECT_EQ(metric.value, 5.0);  // Count
            EXPECT_EQ(metric.type, metric_type::summary);
            found_count = true;
        }
    }
    
    EXPECT_TRUE(found_avg) << "Histogram average not found";
    EXPECT_TRUE(found_min) << "Histogram min not found";
    EXPECT_TRUE(found_max) << "Histogram max not found";
    EXPECT_TRUE(found_count) << "Histogram count not found";
}

TEST_F(monitoring_test, basic_monitor_health_check_healthy) {
    // Initial health check should be healthy
    auto result = monitor_->check_health();
    ASSERT_TRUE(result);
    
    auto& health = result.value();
    EXPECT_TRUE(health.is_healthy());
    EXPECT_EQ(health.get_status(), health_status::healthy);
    EXPECT_TRUE(health.get_issues().empty());
}

TEST_F(monitoring_test, basic_monitor_health_check_with_errors) {
    auto* basic = dynamic_cast<basic_monitor*>(monitor_.get());
    ASSERT_NE(basic, nullptr);
    
    // Simulate errors
    basic->increment_counter("errors_encountered", 20.0);
    basic->increment_counter("messages_logged", 100.0);
    
    // Health check should detect high error rate
    auto result = monitor_->check_health();
    ASSERT_TRUE(result);
    
    auto& health = result.value();
    EXPECT_FALSE(health.is_healthy());
    EXPECT_EQ(health.get_status(), health_status::unhealthy);
    EXPECT_FALSE(health.get_issues().empty());
}

TEST_F(monitoring_test, basic_monitor_health_check_degraded) {
    auto* basic = dynamic_cast<basic_monitor*>(monitor_.get());
    ASSERT_NE(basic, nullptr);
    
    // Simulate moderate issues
    basic->increment_counter("messages_dropped", 5.0);
    basic->increment_counter("messages_logged", 100.0);
    
    // Health check should be degraded
    auto result = monitor_->check_health();
    ASSERT_TRUE(result);
    
    auto& health = result.value();
    EXPECT_FALSE(health.is_healthy());
    EXPECT_EQ(health.get_status(), health_status::degraded);
}

TEST_F(monitoring_test, basic_monitor_reset_metrics) {
    // Add some metrics
    monitor_->increment_counter("test_counter", 10.0);
    monitor_->update_gauge("test_gauge", 20.0);
    
    // Reset
    auto result = monitor_->reset_metrics();
    ASSERT_TRUE(result);
    
    // Collect metrics - should be reset
    auto metrics_result = monitor_->collect_metrics();
    ASSERT_TRUE(metrics_result);
    
    auto& data = metrics_result.value();
    auto& metrics = data.get_metrics();
    
    // Check that counters are reset
    for (const auto& metric : metrics) {
        if (metric.name == "messages_logged" || 
            metric.name == "messages_dropped" ||
            metric.name == "errors_encountered") {
            EXPECT_EQ(metric.value, 0.0);
        }
    }
}

TEST_F(monitoring_test, basic_monitor_disabled_metrics) {
    // Disable monitoring
    monitor_->set_enabled(false);
    
    // Try to collect metrics
    auto result = monitor_->collect_metrics();
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error_code(), error_code::metrics_not_available);
}

TEST_F(monitoring_test, basic_monitor_core_metrics) {
    auto* basic = dynamic_cast<basic_monitor*>(monitor_.get());
    ASSERT_NE(basic, nullptr);
    
    // Simulate some activity
    basic->increment_counter("messages_logged", 100.0);
    basic->increment_counter("messages_dropped", 5.0);
    basic->increment_counter("errors_encountered", 2.0);
    basic->increment_counter("writers_failed", 1.0);
    
    // Collect metrics
    auto result = monitor_->collect_metrics();
    ASSERT_TRUE(result);
    
    auto& data = result.value();
    auto& metrics = data.get_metrics();
    
    // Verify core metrics
    std::unordered_map<std::string, double> expected = {
        {"messages_logged", 100.0},
        {"messages_dropped", 5.0},
        {"errors_encountered", 2.0},
        {"writers_failed", 1.0}
    };
    
    for (const auto& metric : metrics) {
        if (expected.count(metric.name)) {
            EXPECT_EQ(metric.value, expected[metric.name])
                << "Metric " << metric.name << " has unexpected value";
        }
    }
}

// Factory Tests
TEST(monitoring_factory_test, create_basic_monitor) {
    auto monitor = monitoring_factory::create_monitor(
        monitoring_factory::monitor_type::basic
    );
    
    ASSERT_NE(monitor, nullptr);
    EXPECT_EQ(monitor->get_backend_name(), "basic");
}

TEST(monitoring_factory_test, create_automatic_monitor) {
    auto monitor = monitoring_factory::create_monitor(
        monitoring_factory::monitor_type::automatic
    );
    
    ASSERT_NE(monitor, nullptr);
    EXPECT_FALSE(monitor->get_backend_name().empty());
}

TEST(monitoring_factory_test, create_best_available) {
    auto monitor = monitoring_factory::create_best_available();
    
    ASSERT_NE(monitor, nullptr);
    EXPECT_TRUE(monitor->is_enabled());
}

TEST(monitoring_factory_test, get_monitor_type_name) {
    EXPECT_STREQ(monitoring_factory::get_monitor_type_name(
        monitoring_factory::monitor_type::basic), "basic");
    EXPECT_STREQ(monitoring_factory::get_monitor_type_name(
        monitoring_factory::monitor_type::automatic), "automatic");
}

TEST(monitoring_factory_test, get_available_type) {
    auto type = monitoring_factory::get_available_type();
    
    // Should be either basic or thread_system
    EXPECT_TRUE(type == monitoring_factory::monitor_type::basic ||
                type == monitoring_factory::monitor_type::thread_system);
}

// Health Status Tests
TEST(health_status_test, string_conversion) {
    EXPECT_EQ(health_status_to_string(health_status::healthy), "healthy");
    EXPECT_EQ(health_status_to_string(health_status::degraded), "degraded");
    EXPECT_EQ(health_status_to_string(health_status::unhealthy), "unhealthy");
    EXPECT_EQ(health_status_to_string(health_status::unknown), "unknown");
}

TEST(metric_type_test, string_conversion) {
    EXPECT_EQ(metric_type_to_string(metric_type::counter), "counter");
    EXPECT_EQ(metric_type_to_string(metric_type::gauge), "gauge");
    EXPECT_EQ(metric_type_to_string(metric_type::histogram), "histogram");
    EXPECT_EQ(metric_type_to_string(metric_type::summary), "summary");
}

// Concurrency Tests
TEST_F(monitoring_test, basic_monitor_thread_safety) {
    const int num_threads = 10;
    const int operations_per_thread = 1000;
    std::vector<std::thread> threads;
    
    // Spawn threads that update metrics concurrently
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                monitor_->increment_counter("thread_counter", 1.0);
                monitor_->update_gauge("thread_gauge", i * 100.0 + j);
                monitor_->record_histogram("thread_histogram", j);
                
                // Occasionally collect metrics
                if (j % 100 == 0) {
                    auto result = monitor_->collect_metrics();
                    EXPECT_TRUE(result);
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Final metrics collection
    auto result = monitor_->collect_metrics();
    ASSERT_TRUE(result);
    
    auto& data = result.value();
    auto& metrics = data.get_metrics();
    
    // Verify thread counter
    for (const auto& metric : metrics) {
        if (metric.name == "thread_counter") {
            double expected = num_threads * operations_per_thread;
            EXPECT_EQ(metric.value, expected)
                << "Thread counter should be " << expected;
            break;
        }
    }
}

// Performance Metrics Tests
TEST_F(monitoring_test, basic_monitor_performance_metrics) {
    auto* basic = dynamic_cast<basic_monitor*>(monitor_.get());
    ASSERT_NE(basic, nullptr);
    
    // Record processing times
    basic->record_histogram("processing_time_us", 100.0);
    basic->record_histogram("processing_time_us", 200.0);
    basic->record_histogram("processing_time_us", 150.0);
    basic->record_histogram("processing_time_us", 300.0);
    basic->record_histogram("processing_time_us", 50.0);
    
    // Log some messages
    basic->increment_counter("messages_logged", 5.0);
    
    // Collect metrics
    auto result = monitor_->collect_metrics();
    ASSERT_TRUE(result);
    
    auto& data = result.value();
    auto& metrics = data.get_metrics();
    
    // Check for performance metrics
    bool found_avg = false, found_max = false, found_min = false;
    
    for (const auto& metric : metrics) {
        if (metric.name == "avg_processing_time_us") {
            EXPECT_EQ(metric.value, 160.0);  // (100+200+150+300+50)/5
            found_avg = true;
        } else if (metric.name == "max_processing_time_us") {
            EXPECT_EQ(metric.value, 300.0);
            found_max = true;
        } else if (metric.name == "min_processing_time_us") {
            EXPECT_EQ(metric.value, 50.0);
            found_min = true;
        }
    }
    
    EXPECT_TRUE(found_avg) << "Average processing time not found";
    EXPECT_TRUE(found_max) << "Max processing time not found";
    EXPECT_TRUE(found_min) << "Min processing time not found";
}

// Uptime Test
TEST_F(monitoring_test, basic_monitor_uptime) {
    // Wait a bit
    std::this_thread::sleep_for(100ms);
    
    // Collect metrics
    auto result = monitor_->collect_metrics();
    ASSERT_TRUE(result);
    
    auto& data = result.value();
    auto& metrics = data.get_metrics();
    
    // Find uptime metric
    bool found = false;
    for (const auto& metric : metrics) {
        if (metric.name == "uptime_seconds") {
            EXPECT_GE(metric.value, 0.0);
            EXPECT_LE(metric.value, 1.0);  // Should be less than 1 second
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Uptime metric not found";
}