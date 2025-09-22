/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file test_monitorable_interface.cpp
 * @brief Unit tests for monitorable interface and monitoring data
 */

#include <gtest/gtest.h>
#include <kcenon/monitoring/interfaces/monitorable_interface.h>
#include <thread>
#include <atomic>

using namespace monitoring_system;

/**
 * Test implementation of monitorable_interface
 */
class test_monitorable_component : public monitorable_component {
private:
    std::atomic<int> operation_count_{0};
    std::atomic<double> cpu_usage_{0.0};
    std::atomic<double> memory_usage_{0.0};
    
public:
    explicit test_monitorable_component(const std::string& id)
        : monitorable_component(id) {}
    
    result<monitoring_data> get_monitoring_data() const override {
        if (!is_monitoring_enabled()) {
            return make_error<monitoring_data>(
                monitoring_error_code::monitoring_disabled,
                "Monitoring is disabled for this component"
            );
        }
        
        monitoring_data data(get_monitoring_id());
        
        // Add metrics
        data.add_metric("operation_count", operation_count_.load());
        data.add_metric("cpu_usage", cpu_usage_.load());
        data.add_metric("memory_usage", memory_usage_.load());
        
        // Add tags
        data.add_tag("component_type", "test");
        data.add_tag("version", "1.0.0");
        data.add_tag("status", "running");
        
        return make_success(std::move(data));
    }
    
    // Test methods to update internal state
    void perform_operation() {
        operation_count_++;
    }
    
    void set_cpu_usage(double usage) {
        cpu_usage_ = usage;
    }
    
    void set_memory_usage(double usage) {
        memory_usage_ = usage;
    }
    
    int get_operation_count() const {
        return operation_count_.load();
    }
};

/**
 * Test fixture for monitorable interface tests
 */
class MonitorableInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }
    
    void TearDown() override {
        // Cleanup code if needed
    }
};

/**
 * Test monitoring_data basic operations
 */
TEST_F(MonitorableInterfaceTest, MonitoringDataBasicOperations) {
    monitoring_data data("test_component");
    
    // Test adding metrics
    data.add_metric("cpu", 75.5);
    data.add_metric("memory", 1024.0);
    
    // Test adding tags
    data.add_tag("host", "localhost");
    data.add_tag("region", "us-east");
    
    // Verify metrics
    auto cpu = data.get_metric("cpu");
    ASSERT_TRUE(cpu.has_value());
    EXPECT_DOUBLE_EQ(cpu.value(), 75.5);
    
    auto memory = data.get_metric("memory");
    ASSERT_TRUE(memory.has_value());
    EXPECT_DOUBLE_EQ(memory.value(), 1024.0);
    
    auto missing = data.get_metric("nonexistent");
    EXPECT_FALSE(missing.has_value());
    
    // Verify tags
    auto host = data.get_tag("host");
    ASSERT_TRUE(host.has_value());
    EXPECT_EQ(host.value(), "localhost");
    
    auto region = data.get_tag("region");
    ASSERT_TRUE(region.has_value());
    EXPECT_EQ(region.value(), "us-east");
    
    // Verify counts
    EXPECT_EQ(data.metric_count(), 2);
    EXPECT_EQ(data.tag_count(), 2);
    EXPECT_FALSE(data.empty());
    
    // Test component name
    EXPECT_EQ(data.get_component_name(), "test_component");
}

/**
 * Test monitoring_data merge functionality
 */
TEST_F(MonitorableInterfaceTest, MonitoringDataMerge) {
    monitoring_data data1("component1");
    data1.add_metric("metric1", 10.0);
    data1.add_tag("tag1", "value1");
    
    monitoring_data data2("component2");
    data2.add_metric("metric2", 20.0);
    data2.add_tag("tag2", "value2");
    
    // Merge without prefix
    data1.merge(data2);
    
    EXPECT_EQ(data1.metric_count(), 2);
    EXPECT_EQ(data1.tag_count(), 2);
    
    auto metric2 = data1.get_metric("metric2");
    ASSERT_TRUE(metric2.has_value());
    EXPECT_DOUBLE_EQ(metric2.value(), 20.0);
    
    // Merge with prefix
    monitoring_data data3("component3");
    data3.add_metric("metric3", 30.0);
    data3.add_tag("tag3", "value3");
    
    data1.merge(data3, "prefix");
    
    auto prefixed_metric = data1.get_metric("prefix.metric3");
    ASSERT_TRUE(prefixed_metric.has_value());
    EXPECT_DOUBLE_EQ(prefixed_metric.value(), 30.0);
    
    auto prefixed_tag = data1.get_tag("prefix.tag3");
    ASSERT_TRUE(prefixed_tag.has_value());
    EXPECT_EQ(prefixed_tag.value(), "value3");
}

/**
 * Test monitoring_data clear and empty
 */
TEST_F(MonitorableInterfaceTest, MonitoringDataClearAndEmpty) {
    monitoring_data data("test");
    
    // Initially empty
    EXPECT_TRUE(data.empty());
    EXPECT_EQ(data.metric_count(), 0);
    EXPECT_EQ(data.tag_count(), 0);
    
    // Add data
    data.add_metric("metric", 1.0);
    data.add_tag("tag", "value");
    
    EXPECT_FALSE(data.empty());
    EXPECT_EQ(data.metric_count(), 1);
    EXPECT_EQ(data.tag_count(), 1);
    
    // Clear data
    data.clear();
    
    EXPECT_TRUE(data.empty());
    EXPECT_EQ(data.metric_count(), 0);
    EXPECT_EQ(data.tag_count(), 0);
}

/**
 * Test monitorable_component implementation
 */
TEST_F(MonitorableInterfaceTest, MonitorableComponentBasic) {
    test_monitorable_component component("test_comp_1");
    
    // Test initial state
    EXPECT_EQ(component.get_monitoring_id(), "test_comp_1");
    EXPECT_TRUE(component.is_monitoring_enabled());
    
    // Perform operations
    component.perform_operation();
    component.perform_operation();
    component.set_cpu_usage(45.5);
    component.set_memory_usage(2048.0);
    
    // Get monitoring data
    auto result = component.get_monitoring_data();
    ASSERT_TRUE(result);
    
    auto data = result.value();
    EXPECT_EQ(data.get_component_name(), "test_comp_1");
    
    // Verify metrics
    auto op_count = data.get_metric("operation_count");
    ASSERT_TRUE(op_count.has_value());
    EXPECT_DOUBLE_EQ(op_count.value(), 2.0);
    
    auto cpu = data.get_metric("cpu_usage");
    ASSERT_TRUE(cpu.has_value());
    EXPECT_DOUBLE_EQ(cpu.value(), 45.5);
    
    auto memory = data.get_metric("memory_usage");
    ASSERT_TRUE(memory.has_value());
    EXPECT_DOUBLE_EQ(memory.value(), 2048.0);
    
    // Verify tags
    auto type = data.get_tag("component_type");
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type.value(), "test");
    
    auto version = data.get_tag("version");
    ASSERT_TRUE(version.has_value());
    EXPECT_EQ(version.value(), "1.0.0");
}

/**
 * Test monitoring enable/disable
 */
TEST_F(MonitorableInterfaceTest, MonitoringEnableDisable) {
    test_monitorable_component component("test_comp");
    
    // Initially enabled
    EXPECT_TRUE(component.is_monitoring_enabled());
    
    auto result = component.get_monitoring_data();
    EXPECT_TRUE(result);
    
    // Disable monitoring
    auto disable_result = component.set_monitoring_enabled(false);
    EXPECT_TRUE(disable_result);
    EXPECT_FALSE(component.is_monitoring_enabled());
    
    // Should return error when disabled
    result = component.get_monitoring_data();
    EXPECT_FALSE(result);
    EXPECT_EQ(result.get_error().code, monitoring_error_code::monitoring_disabled);
    
    // Re-enable monitoring
    auto enable_result = component.set_monitoring_enabled(true);
    EXPECT_TRUE(enable_result);
    EXPECT_TRUE(component.is_monitoring_enabled());
    
    // Should work again
    result = component.get_monitoring_data();
    EXPECT_TRUE(result);
}

/**
 * Test monitoring reset
 */
TEST_F(MonitorableInterfaceTest, MonitoringReset) {
    test_monitorable_component component("test_comp");
    
    // Set some state
    component.perform_operation();
    component.perform_operation();
    component.perform_operation();
    
    EXPECT_EQ(component.get_operation_count(), 3);
    
    // Reset monitoring (note: our test implementation doesn't reset internal counters)
    auto reset_result = component.reset_monitoring();
    EXPECT_TRUE(reset_result);
    
    // Internal state remains (as we didn't override reset_monitoring to clear it)
    EXPECT_EQ(component.get_operation_count(), 3);
}

/**
 * Test monitoring_aggregator basic operations
 */
TEST_F(MonitorableInterfaceTest, AggregatorBasicOperations) {
    monitoring_aggregator aggregator("main_aggregator");
    
    // Create components
    auto comp1 = std::make_shared<test_monitorable_component>("comp1");
    auto comp2 = std::make_shared<test_monitorable_component>("comp2");
    auto comp3 = std::make_shared<test_monitorable_component>("comp3");
    
    // Set different metrics for each
    comp1->set_cpu_usage(25.0);
    comp1->set_memory_usage(1000.0);
    comp1->perform_operation();
    
    comp2->set_cpu_usage(50.0);
    comp2->set_memory_usage(2000.0);
    comp2->perform_operation();
    comp2->perform_operation();
    
    comp3->set_cpu_usage(75.0);
    comp3->set_memory_usage(3000.0);
    comp3->perform_operation();
    comp3->perform_operation();
    comp3->perform_operation();
    
    // Add components to aggregator
    aggregator.add_component(comp1);
    aggregator.add_component(comp2);
    aggregator.add_component(comp3);
    
    EXPECT_EQ(aggregator.size(), 3);
    
    // Get component IDs
    auto ids = aggregator.get_component_ids();
    EXPECT_EQ(ids.size(), 3);
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "comp1") != ids.end());
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "comp2") != ids.end());
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "comp3") != ids.end());
    
    // Get specific component
    auto retrieved = aggregator.get_component("comp2");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->get_monitoring_id(), "comp2");
}

/**
 * Test monitoring_aggregator data collection
 */
TEST_F(MonitorableInterfaceTest, AggregatorDataCollection) {
    monitoring_aggregator aggregator("test_aggregator");
    
    // Create and configure components
    auto comp1 = std::make_shared<test_monitorable_component>("comp1");
    comp1->set_cpu_usage(30.0);
    comp1->set_memory_usage(1500.0);
    
    auto comp2 = std::make_shared<test_monitorable_component>("comp2");
    comp2->set_cpu_usage(60.0);
    comp2->set_memory_usage(2500.0);
    
    aggregator.add_component(comp1);
    aggregator.add_component(comp2);
    
    // Collect all data
    auto result = aggregator.collect_all();
    ASSERT_TRUE(result);
    
    auto aggregated = result.value();
    EXPECT_EQ(aggregated.get_component_name(), "test_aggregator");
    
    // Check that metrics are prefixed
    auto comp1_cpu = aggregated.get_metric("comp1.cpu_usage");
    ASSERT_TRUE(comp1_cpu.has_value());
    EXPECT_DOUBLE_EQ(comp1_cpu.value(), 30.0);
    
    auto comp2_cpu = aggregated.get_metric("comp2.cpu_usage");
    ASSERT_TRUE(comp2_cpu.has_value());
    EXPECT_DOUBLE_EQ(comp2_cpu.value(), 60.0);
    
    // Check aggregator metadata
    auto component_count = aggregated.get_metric("aggregator.component_count");
    ASSERT_TRUE(component_count.has_value());
    EXPECT_DOUBLE_EQ(component_count.value(), 2.0);
}

/**
 * Test aggregator with disabled components
 */
TEST_F(MonitorableInterfaceTest, AggregatorWithDisabledComponents) {
    monitoring_aggregator aggregator("test_aggregator");
    
    auto comp1 = std::make_shared<test_monitorable_component>("comp1");
    auto comp2 = std::make_shared<test_monitorable_component>("comp2");
    
    comp1->set_cpu_usage(40.0);
    comp2->set_cpu_usage(80.0);
    
    // Disable comp2
    comp2->set_monitoring_enabled(false);
    
    aggregator.add_component(comp1);
    aggregator.add_component(comp2);
    
    // Collect data
    auto result = aggregator.collect_all();
    ASSERT_TRUE(result);
    
    auto aggregated = result.value();
    
    // comp1 data should be present
    auto comp1_cpu = aggregated.get_metric("comp1.cpu_usage");
    ASSERT_TRUE(comp1_cpu.has_value());
    
    // comp2 data should not be present (it's disabled)
    auto comp2_cpu = aggregated.get_metric("comp2.cpu_usage");
    EXPECT_FALSE(comp2_cpu.has_value());
    
    // comp2 is disabled, so it's skipped (no error tag needed)
    auto comp2_error = aggregated.get_tag("comp2.error");
    EXPECT_FALSE(comp2_error.has_value());
}

/**
 * Test aggregator component removal
 */
TEST_F(MonitorableInterfaceTest, AggregatorComponentRemoval) {
    monitoring_aggregator aggregator("test_aggregator");
    
    auto comp1 = std::make_shared<test_monitorable_component>("comp1");
    auto comp2 = std::make_shared<test_monitorable_component>("comp2");
    auto comp3 = std::make_shared<test_monitorable_component>("comp3");
    
    aggregator.add_component(comp1);
    aggregator.add_component(comp2);
    aggregator.add_component(comp3);
    
    EXPECT_EQ(aggregator.size(), 3);
    
    // Remove comp2
    bool removed = aggregator.remove_component("comp2");
    EXPECT_TRUE(removed);
    EXPECT_EQ(aggregator.size(), 2);
    
    // Try to remove non-existent component
    removed = aggregator.remove_component("nonexistent");
    EXPECT_FALSE(removed);
    EXPECT_EQ(aggregator.size(), 2);
    
    // Verify comp2 is gone
    auto ids = aggregator.get_component_ids();
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "comp1") != ids.end());
    EXPECT_FALSE(std::find(ids.begin(), ids.end(), "comp2") != ids.end());
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "comp3") != ids.end());
    
    // Clear all
    aggregator.clear();
    EXPECT_EQ(aggregator.size(), 0);
}

/**
 * Test monitoring data timestamp
 */
TEST_F(MonitorableInterfaceTest, MonitoringDataTimestamp) {
    auto start_time = std::chrono::system_clock::now();
    
    monitoring_data data("test");
    
    auto timestamp = data.get_timestamp();
    auto end_time = std::chrono::system_clock::now();
    
    // Timestamp should be between start and end
    EXPECT_GE(timestamp, start_time);
    EXPECT_LE(timestamp, end_time);
}

/**
 * Test thread safety of monitorable component
 */
TEST_F(MonitorableInterfaceTest, ThreadSafetyMonitorableComponent) {
    test_monitorable_component component("thread_test");
    
    const int thread_count = 10;
    const int operations_per_thread = 1000;
    std::vector<std::thread> threads;
    
    // Launch threads that perform operations
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&component]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                component.perform_operation();
                
                // Also get monitoring data periodically
                if (j % 100 == 0) {
                    auto result = component.get_monitoring_data();
                    EXPECT_TRUE(result);
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify final count
    EXPECT_EQ(component.get_operation_count(), thread_count * operations_per_thread);
    
    // Get final monitoring data
    auto result = component.get_monitoring_data();
    ASSERT_TRUE(result);
    
    auto op_count = result.value().get_metric("operation_count");
    ASSERT_TRUE(op_count.has_value());
    EXPECT_DOUBLE_EQ(op_count.value(), 
                     static_cast<double>(thread_count * operations_per_thread));
}

// Main function provided by gtest_main