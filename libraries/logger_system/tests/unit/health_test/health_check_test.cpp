/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "../../sources/logger/health/health_check_system.h"
#include "../../sources/logger/writers/base_writer.h"
#include "../../sources/logger/core/log_collector.h"
#include <thread>
#include <chrono>

using namespace logger_module;
using namespace std::chrono_literals;

// Mock writer for testing
class mock_writer : public base_writer {
public:
    mock_writer() : fail_writes_(false), write_count_(0) {}
    
    result_void write(thread_module::log_level /* level */,
                     const std::string& /* message */,
                     const std::string& /* file */,
                     int /* line */,
                     const std::string& /* function */,
                     const std::chrono::system_clock::time_point& /* timestamp */) override {
        write_count_++;
        if (fail_writes_) {
            return result_void(logger_error_code::file_write_failed);
        }
        return result_void{};
    }
    
    result_void flush() override {
        return result_void{};
    }
    
    std::string get_name() const override {
        return "mock_writer";
    }
    
    void set_fail_writes(bool fail) {
        fail_writes_ = fail;
    }
    
    int get_write_count() const {
        return write_count_;
    }
    
private:
    bool fail_writes_;
    int write_count_;
};

class HealthCheckTest : public ::testing::Test {
protected:
    void SetUp() override {
        health_system_ = std::make_unique<health_check_system>();
    }
    
    void TearDown() override {
        health_system_.reset();
    }
    
    std::unique_ptr<health_check_system> health_system_;
};

TEST_F(HealthCheckTest, DefaultCriteria) {
    auto criteria = health_system_->get_criteria();
    
    EXPECT_EQ(criteria.max_consecutive_write_failures, 5);
    EXPECT_EQ(criteria.max_write_latency, 1000ms);
    EXPECT_EQ(criteria.max_buffer_usage_percent, 90.0);
    EXPECT_EQ(criteria.max_queue_usage_percent, 85.0);
}

TEST_F(HealthCheckTest, WriterRegistration) {
    mock_writer writer1;
    mock_writer writer2;
    
    health_system_->register_writer("writer1", &writer1);
    health_system_->register_writer("writer2", &writer2);
    
    auto all_health = health_system_->get_all_writer_health();
    EXPECT_EQ(all_health.size(), 2);
    EXPECT_TRUE(all_health.find("writer1") != all_health.end());
    EXPECT_TRUE(all_health.find("writer2") != all_health.end());
    
    health_system_->unregister_writer("writer1");
    all_health = health_system_->get_all_writer_health();
    EXPECT_EQ(all_health.size(), 1);
    EXPECT_TRUE(all_health.find("writer2") != all_health.end());
}

TEST_F(HealthCheckTest, WriterHealthTracking) {
    mock_writer writer;
    health_system_->register_writer("test_writer", &writer);
    
    // Successful writes
    for (int i = 0; i < 10; ++i) {
        health_system_->update_writer_stats("test_writer", true, 10ms);
    }
    
    auto result = health_system_->check_writer_health("test_writer");
    ASSERT_TRUE(result.has_value());
    
    auto info = result.value();
    EXPECT_EQ(info.status, health_status::healthy);
    EXPECT_EQ(info.total_writes, 10);
    EXPECT_EQ(info.failed_writes, 0);
    EXPECT_EQ(info.consecutive_failures, 0);
}

TEST_F(HealthCheckTest, WriterFailureDetection) {
    mock_writer writer;
    health_system_->register_writer("test_writer", &writer);
    
    // Simulate consecutive failures
    for (int i = 0; i < 5; ++i) {
        health_system_->update_writer_stats("test_writer", false, 10ms);
    }
    
    auto result = health_system_->check_writer_health("test_writer");
    ASSERT_TRUE(result.has_value());
    
    auto info = result.value();
    EXPECT_EQ(info.status, health_status::unhealthy);
    EXPECT_EQ(info.consecutive_failures, 5);
    EXPECT_EQ(info.failed_writes, 5);
}

TEST_F(HealthCheckTest, WriterLatencyDetection) {
    mock_writer writer;
    health_system_->register_writer("test_writer", &writer);
    
    // Simulate high latency write
    health_system_->update_writer_stats("test_writer", true, 2000ms);
    
    auto result = health_system_->check_writer_health("test_writer");
    ASSERT_TRUE(result.has_value());
    
    auto info = result.value();
    EXPECT_EQ(info.status, health_status::unhealthy);
    EXPECT_EQ(info.max_write_latency, 2000ms);
}

TEST_F(HealthCheckTest, BufferHealthTracking) {
    // Update buffer stats
    health_system_->update_buffer_stats(8000, 10000, false);
    
    auto buffer_health = health_system_->check_buffer_health();
    EXPECT_EQ(buffer_health.status, health_status::healthy);
    EXPECT_EQ(buffer_health.usage_percent, 80.0);
    EXPECT_EQ(buffer_health.available_capacity, 2000);
    
    // Simulate high usage
    health_system_->update_buffer_stats(9500, 10000, false);
    buffer_health = health_system_->check_buffer_health();
    EXPECT_EQ(buffer_health.status, health_status::unhealthy);
    EXPECT_EQ(buffer_health.usage_percent, 95.0);
}

TEST_F(HealthCheckTest, QueueHealthTracking) {
    // Update queue stats
    health_system_->update_queue_stats(5000, 10000, false, 100ms);
    
    auto queue_health = health_system_->check_queue_health();
    EXPECT_EQ(queue_health.status, health_status::healthy);
    EXPECT_EQ(queue_health.usage_percent, 50.0);
    
    // Simulate high usage
    health_system_->update_queue_stats(9000, 10000, false, 100ms);
    queue_health = health_system_->check_queue_health();
    EXPECT_EQ(queue_health.status, health_status::unhealthy);
    
    // Simulate dropped messages
    health_system_->update_queue_stats(7000, 10000, true, 100ms);
    queue_health = health_system_->check_queue_health();
    EXPECT_EQ(queue_health.dropped_messages, 1);
    EXPECT_NE(queue_health.status, health_status::healthy);
}

TEST_F(HealthCheckTest, ComprehensiveHealthCheck) {
    mock_writer writer1, writer2;
    health_system_->register_writer("writer1", &writer1);
    health_system_->register_writer("writer2", &writer2);
    
    // Set up mixed health states
    health_system_->update_writer_stats("writer1", true, 10ms);
    health_system_->update_writer_stats("writer2", false, 10ms);
    health_system_->update_writer_stats("writer2", false, 10ms);
    
    health_system_->update_buffer_stats(5000, 10000, false);
    health_system_->update_queue_stats(4000, 10000, false, 50ms);
    
    auto result = health_system_->perform_health_check();
    
    EXPECT_NE(result.get_status(), health_status::unknown);
    EXPECT_FALSE(result.get_message().empty());
}

TEST_F(HealthCheckTest, CustomHealthCheck) {
    bool custom_check_healthy = true;
    
    health_system_->register_custom_check("custom_check",
        [&custom_check_healthy]() {
            return custom_check_healthy ? 
                health_status::healthy : health_status::unhealthy;
        });
    
    auto result = health_system_->perform_health_check();
    EXPECT_EQ(result.get_status(), health_status::healthy);
    
    custom_check_healthy = false;
    result = health_system_->perform_health_check();
    EXPECT_NE(result.get_status(), health_status::healthy);
    
    const auto& issues = result.get_issues();
    bool found_custom_issue = false;
    for (const auto& issue : issues) {
        if (issue.find("custom_check") != std::string::npos) {
            found_custom_issue = true;
            break;
        }
    }
    EXPECT_TRUE(found_custom_issue);
}

TEST_F(HealthCheckTest, MonitoringStartStop) {
    EXPECT_FALSE(health_system_->is_monitoring());
    
    int callback_count = 0;
    health_system_->start_monitoring(1s,
        [&callback_count](const health_check_result&) {
            callback_count++;
        });
    
    EXPECT_TRUE(health_system_->is_monitoring());
    
    std::this_thread::sleep_for(2500ms);
    
    health_system_->stop_monitoring();
    EXPECT_FALSE(health_system_->is_monitoring());
    
    EXPECT_GE(callback_count, 2);
}

TEST_F(HealthCheckTest, ConfigureChecks) {
    // Disable all checks except writers
    health_system_->configure_checks(true, false, false);
    
    mock_writer writer;
    health_system_->register_writer("writer", &writer);
    health_system_->update_writer_stats("writer", false, 10ms);
    
    // High buffer usage (should be ignored)
    health_system_->update_buffer_stats(9500, 10000, false);
    
    auto result = health_system_->perform_health_check();
    
    // Only writer issues should be reported
    const auto& issues = result.get_issues();
    for (const auto& issue : issues) {
        EXPECT_TRUE(issue.find("Writer") != std::string::npos ||
                   issue.find("writer") != std::string::npos);
    }
}

TEST_F(HealthCheckTest, ResetStats) {
    mock_writer writer;
    health_system_->register_writer("writer", &writer);
    
    // Add some stats
    health_system_->update_writer_stats("writer", false, 10ms);
    health_system_->update_buffer_stats(8000, 10000, true);
    health_system_->update_queue_stats(5000, 10000, true, 100ms);
    
    // Reset all stats
    health_system_->reset_stats();
    
    auto writer_result = health_system_->check_writer_health("writer");
    ASSERT_TRUE(writer_result.has_value());
    EXPECT_EQ(writer_result.value().total_writes, 0);
    EXPECT_EQ(writer_result.value().failed_writes, 0);
    
    auto buffer_health = health_system_->check_buffer_health();
    EXPECT_EQ(buffer_health.total_allocations, 0);
    EXPECT_EQ(buffer_health.failed_allocations, 0);
    
    auto queue_health = health_system_->check_queue_health();
    EXPECT_EQ(queue_health.dropped_messages, 0);
}

TEST_F(HealthCheckTest, ScopedRegistration) {
    mock_writer writer;
    
    {
        scoped_health_registration registration(*health_system_, "scoped_writer", &writer);
        
        auto all_health = health_system_->get_all_writer_health();
        EXPECT_EQ(all_health.size(), 1);
        EXPECT_TRUE(all_health.find("scoped_writer") != all_health.end());
    }
    
    // Writer should be unregistered after scope ends
    auto all_health = health_system_->get_all_writer_health();
    EXPECT_EQ(all_health.size(), 0);
}

class HealthCheckUtilsTest : public ::testing::Test {
};

TEST_F(HealthCheckUtilsTest, StatusToString) {
    EXPECT_EQ(health_check_utils::health_status_to_string(health_status::healthy), "healthy");
    EXPECT_EQ(health_check_utils::health_status_to_string(health_status::degraded), "degraded");
    EXPECT_EQ(health_check_utils::health_status_to_string(health_status::unhealthy), "unhealthy");
    EXPECT_EQ(health_check_utils::health_status_to_string(health_status::unknown), "unknown");
}

TEST_F(HealthCheckUtilsTest, AggregateStatus) {
    std::vector<health_status> all_healthy = {
        health_status::healthy,
        health_status::healthy,
        health_status::healthy
    };
    EXPECT_EQ(health_check_utils::aggregate_health_status(all_healthy), 
              health_status::healthy);
    
    std::vector<health_status> has_degraded = {
        health_status::healthy,
        health_status::degraded,
        health_status::healthy
    };
    EXPECT_EQ(health_check_utils::aggregate_health_status(has_degraded), 
              health_status::degraded);
    
    std::vector<health_status> has_unhealthy = {
        health_status::healthy,
        health_status::degraded,
        health_status::unhealthy
    };
    EXPECT_EQ(health_check_utils::aggregate_health_status(has_unhealthy), 
              health_status::unhealthy);
    
    std::vector<health_status> empty;
    EXPECT_EQ(health_check_utils::aggregate_health_status(empty), 
              health_status::unknown);
}

TEST_F(HealthCheckUtilsTest, FormatAsJson) {
    health_check_result result;
    result.set_status(health_status::degraded);
    result.set_message("System degraded");
    result.add_issue("Writer timeout");
    result.add_issue("High queue usage");
    
    auto json = health_check_utils::format_as_json(result);
    
    EXPECT_TRUE(json.find("\"status\": \"degraded\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"message\": \"System degraded\"") != std::string::npos);
    EXPECT_TRUE(json.find("Writer timeout") != std::string::npos);
    EXPECT_TRUE(json.find("High queue usage") != std::string::npos);
}

TEST_F(HealthCheckUtilsTest, FormatWriterHealth) {
    writer_health_info info;
    info.writer_name = "test_writer";
    info.status = health_status::degraded;
    info.total_writes = 100;
    info.failed_writes = 5;
    info.consecutive_failures = 2;
    info.avg_write_latency = 50ms;
    info.max_write_latency = 200ms;
    
    auto formatted = health_check_utils::format_writer_health(info);
    
    EXPECT_TRUE(formatted.find("test_writer") != std::string::npos);
    EXPECT_TRUE(formatted.find("degraded") != std::string::npos);
    EXPECT_TRUE(formatted.find("100") != std::string::npos);
    EXPECT_TRUE(formatted.find("5.00%") != std::string::npos);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}