#include <gtest/gtest.h>
#include "monitoring/core/result_types.h"
#include "monitoring/core/error_codes.h"
#include "monitoring/interfaces/monitoring_interface.h"

using namespace monitoring_system;

/**
 * @brief Test basic Result pattern functionality
 */
class ResultTypesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ResultTypesTest, SuccessResultContainsValue) {
    auto result = make_success<int>(42);
    
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
    EXPECT_EQ(*result, 42);
}

TEST_F(ResultTypesTest, ErrorResultContainsError) {
    auto result = make_error<int>(monitoring_error_code::collector_not_found, "Test error");
    
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(result);
    EXPECT_EQ(result.get_error().code, monitoring_error_code::collector_not_found);
    EXPECT_EQ(result.get_error().message, "Test error");
}

TEST_F(ResultTypesTest, ValueOrReturnsDefaultOnError) {
    auto error_result = make_error<int>(monitoring_error_code::unknown_error);
    EXPECT_EQ(error_result.value_or(100), 100);
    
    auto success_result = make_success<int>(42);
    EXPECT_EQ(success_result.value_or(100), 42);
}

TEST_F(ResultTypesTest, MapTransformsSuccessValue) {
    auto result = make_success<int>(10);
    auto mapped = result.map([](int x) { return x * 2; });
    
    EXPECT_TRUE(mapped.has_value());
    EXPECT_EQ(mapped.value(), 20);
}

TEST_F(ResultTypesTest, MapPropagatesError) {
    auto result = make_error<int>(monitoring_error_code::invalid_configuration);
    auto mapped = result.map([](int x) { return x * 2; });
    
    EXPECT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.get_error().code, monitoring_error_code::invalid_configuration);
}

TEST_F(ResultTypesTest, AndThenChainsOperations) {
    auto result = make_success<int>(10);
    auto chained = result.and_then([](int x) {
        if (x > 5) {
            return make_success<std::string>("Large");
        }
        return make_error<std::string>(monitoring_error_code::invalid_configuration);
    });
    
    EXPECT_TRUE(chained.has_value());
    EXPECT_EQ(chained.value(), "Large");
}

TEST_F(ResultTypesTest, ResultVoidSuccess) {
    auto result = result_void::success();
    
    EXPECT_TRUE(result.is_success());
    EXPECT_TRUE(result);
}

TEST_F(ResultTypesTest, ResultVoidError) {
    auto result = result_void::error(monitoring_error_code::storage_full, "Storage is full");
    
    EXPECT_FALSE(result.is_success());
    EXPECT_FALSE(result);
    EXPECT_TRUE(result.is_error(monitoring_error_code::storage_full));
    EXPECT_EQ(result.get_error().code, monitoring_error_code::storage_full);
}

TEST_F(ResultTypesTest, ErrorCodeToString) {
    EXPECT_EQ(error_code_to_string(monitoring_error_code::success), "Success");
    EXPECT_EQ(error_code_to_string(monitoring_error_code::collector_not_found), "Collector not found");
    EXPECT_EQ(error_code_to_string(monitoring_error_code::storage_full), "Storage is full");
    EXPECT_EQ(error_code_to_string(monitoring_error_code::invalid_configuration), "Invalid configuration");
}

TEST_F(ResultTypesTest, ErrorInfoWithContext) {
    auto result = make_error_with_context<int>(
        monitoring_error_code::collection_failed,
        "Failed to collect metrics",
        "CPU collector timeout"
    );
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.get_error().code, monitoring_error_code::collection_failed);
    EXPECT_EQ(result.get_error().message, "Failed to collect metrics");
    EXPECT_TRUE(result.get_error().context.has_value());
    EXPECT_EQ(result.get_error().context.value(), "CPU collector timeout");
}

TEST_F(ResultTypesTest, MetricsSnapshotOperations) {
    metrics_snapshot snapshot;
    snapshot.add_metric("cpu_usage", 45.5);
    snapshot.add_metric("memory_usage", 2048.0);
    
    EXPECT_EQ(snapshot.metrics.size(), 2);
    
    auto cpu = snapshot.get_metric("cpu_usage");
    EXPECT_TRUE(cpu.has_value());
    EXPECT_EQ(cpu.value(), 45.5);
    
    auto unknown = snapshot.get_metric("unknown_metric");
    EXPECT_FALSE(unknown.has_value());
}

TEST_F(ResultTypesTest, MonitoringConfigValidation) {
    monitoring_config config;
    
    // Valid configuration
    config.history_size = 100;
    config.collection_interval = std::chrono::milliseconds(100);
    config.buffer_size = 1000;
    auto result = config.validate();
    EXPECT_TRUE(result.is_success());
    
    // Invalid history size
    config.history_size = 0;
    result = config.validate();
    EXPECT_FALSE(result.is_success());
    EXPECT_TRUE(result.is_error(monitoring_error_code::invalid_capacity));
    
    // Invalid interval
    config.history_size = 100;
    config.collection_interval = std::chrono::milliseconds(5);
    result = config.validate();
    EXPECT_FALSE(result.is_success());
    EXPECT_TRUE(result.is_error(monitoring_error_code::invalid_interval));
    
    // Invalid buffer size
    config.collection_interval = std::chrono::milliseconds(100);
    config.buffer_size = 50; // Less than history_size
    result = config.validate();
    EXPECT_FALSE(result.is_success());
    EXPECT_TRUE(result.is_error(monitoring_error_code::invalid_capacity));
}

TEST_F(ResultTypesTest, HealthCheckResult) {
    health_check_result health;
    
    EXPECT_EQ(health.status, health_status::unknown);
    EXPECT_FALSE(health.is_healthy());
    
    health.status = health_status::healthy;
    EXPECT_TRUE(health.is_healthy());
    
    health.status = health_status::degraded;
    health.issues.push_back("High memory usage");
    EXPECT_FALSE(health.is_healthy());
    EXPECT_EQ(health.issues.size(), 1);
}