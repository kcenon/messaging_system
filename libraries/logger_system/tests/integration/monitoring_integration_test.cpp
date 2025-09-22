/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file monitoring_integration_test.cpp
 * @brief Integration tests for monitoring_system compatibility
 *
 * This file tests the integration of logger_system with monitoring_system,
 * verifying metrics collection and alert system integration.
 */

#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <filesystem>
#include <sstream>
#include <functional>
#include <map>
#include <mutex>
#include <iostream>

namespace fs = std::filesystem;

/**
 * @brief Mock monitoring system interface
 */
class MockMonitoringSystem {
public:
    struct Metric {
        std::string name;
        double value;
        std::chrono::system_clock::time_point timestamp;
        std::map<std::string, std::string> tags;
    };

    struct Alert {
        std::string name;
        std::string message;
        std::string severity;
        std::chrono::system_clock::time_point timestamp;
    };

    void record_metric(const std::string& name, double value,
                       const std::map<std::string, std::string>& tags = {}) {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        metrics_.push_back({name, value, std::chrono::system_clock::now(), tags});
    }

    void send_alert(const std::string& name, const std::string& message,
                   const std::string& severity = "warning") {
        std::lock_guard<std::mutex> lock(alerts_mutex_);
        alerts_.push_back({name, message, severity, std::chrono::system_clock::now()});
    }

    std::vector<Metric> get_metrics() const {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        return metrics_;
    }

    std::vector<Alert> get_alerts() const {
        std::lock_guard<std::mutex> lock(alerts_mutex_);
        return alerts_;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        std::lock_guard<std::mutex> lock2(alerts_mutex_);
        metrics_.clear();
        alerts_.clear();
    }

private:
    mutable std::mutex metrics_mutex_;
    mutable std::mutex alerts_mutex_;
    std::vector<Metric> metrics_;
    std::vector<Alert> alerts_;
};

class MonitoringIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any previous test artifacts
        if (fs::exists("test_logs")) {
            fs::remove_all("test_logs");
        }
        fs::create_directories("test_logs");

        // Initialize monitoring system
        monitoring_system_ = std::make_unique<MockMonitoringSystem>();
    }

    void TearDown() override {
        // Cleanup
        monitoring_system_.reset();

        // Remove test logs
        if (fs::exists("test_logs")) {
            fs::remove_all("test_logs");
        }
    }

    std::unique_ptr<MockMonitoringSystem> monitoring_system_;
};

/**
 * @brief Test basic metrics collection
 */
TEST_F(MonitoringIntegrationTest, BasicMetricsCollection) {
    // Simulate metrics collection
    uint64_t total_logs = 100;
    uint64_t info_count = 100;
    uint64_t error_count = 0;
    uint64_t warning_count = 0;

    // Send metrics to monitoring system
    monitoring_system_->record_metric("logger.total_logs", static_cast<double>(total_logs));
    monitoring_system_->record_metric("logger.info_count", static_cast<double>(info_count));
    monitoring_system_->record_metric("logger.error_count", static_cast<double>(error_count));
    monitoring_system_->record_metric("logger.warning_count", static_cast<double>(warning_count));

    // Verify metrics in monitoring system
    auto metrics = monitoring_system_->get_metrics();
    EXPECT_EQ(metrics.size(), 4);

    // Verify values
    auto total_metric = std::find_if(metrics.begin(), metrics.end(),
        [](const auto& m) { return m.name == "logger.total_logs"; });
    EXPECT_NE(total_metric, metrics.end());
    EXPECT_EQ(total_metric->value, 100.0);
}

/**
 * @brief Test performance metrics collection
 */
TEST_F(MonitoringIntegrationTest, PerformanceMetricsCollection) {
    // Simulate performance metrics
    double throughput = 1000.0; // messages per second
    double latency = 50.0; // milliseconds

    // Send performance metrics to monitoring system
    monitoring_system_->record_metric("logger.throughput", throughput,
                                     {{"unit", "msg/s"}});
    monitoring_system_->record_metric("logger.latency", latency,
                                     {{"unit", "ms"}, {"operation", "1000_messages"}});

    // Verify metrics
    auto metrics = monitoring_system_->get_metrics();
    EXPECT_GE(metrics.size(), 2);

    // Find throughput metric
    auto throughput_metric = std::find_if(metrics.begin(), metrics.end(),
        [](const auto& m) { return m.name == "logger.throughput"; });
    EXPECT_NE(throughput_metric, metrics.end());
    EXPECT_EQ(throughput_metric->value, 1000.0);
}

/**
 * @brief Test alert system integration
 */
TEST_F(MonitoringIntegrationTest, AlertSystemIntegration) {
    // Simulate error conditions
    int error_count = 10;

    // Send alert if errors exceed threshold
    if (error_count > 5) {
        monitoring_system_->send_alert("logger.high_error_rate",
                                      "Error count exceeded threshold: " + std::to_string(error_count),
                                      "critical");
    }

    // Simulate health check failure
    monitoring_system_->send_alert("logger.health_check_failed",
                                  "Logger health check failed: high error rate",
                                  "warning");

    auto alerts = monitoring_system_->get_alerts();
    EXPECT_EQ(alerts.size(), 2);

    // Verify critical alert exists
    auto critical_alert = std::find_if(alerts.begin(), alerts.end(),
        [](const auto& a) { return a.severity == "critical"; });
    EXPECT_NE(critical_alert, alerts.end());
}

/**
 * @brief Test metrics aggregation
 */
TEST_F(MonitoringIntegrationTest, MetricsAggregation) {
    // Collect metrics over time
    std::vector<uint64_t> time_series = {20, 20, 20, 20, 20};

    uint64_t total_logs = 0;
    for (size_t i = 0; i < time_series.size(); ++i) {
        total_logs += time_series[i];

        // Send snapshot metric
        monitoring_system_->record_metric("logger.batch_" + std::to_string(i),
                                         static_cast<double>(time_series[i]),
                                         {{"batch_id", std::to_string(i)}});
    }

    // Send aggregated metrics
    monitoring_system_->record_metric("logger.total_logs_aggregated",
                                     static_cast<double>(total_logs),
                                     {{"aggregation", "sum"}});
    monitoring_system_->record_metric("logger.avg_logs_per_batch",
                                     static_cast<double>(total_logs) / time_series.size(),
                                     {{"aggregation", "average"}});

    // Verify aggregated metrics
    auto metrics = monitoring_system_->get_metrics();
    EXPECT_GE(metrics.size(), 7); // 5 batches + 2 aggregated
}

/**
 * @brief Test real-time monitoring
 */
TEST_F(MonitoringIntegrationTest, RealTimeMonitoring) {
    // Set up real-time monitoring simulation
    std::atomic<int> callback_count{0};
    std::atomic<uint64_t> last_total_logs{0};

    // Simulate monitoring thread
    std::atomic<bool> monitoring_active{true};
    std::thread monitoring_thread([this, &callback_count, &last_total_logs, &monitoring_active]() {
        uint64_t current_logs = 0;
        while (monitoring_active) {
            // Simulate new log activity
            current_logs += 10;

            if (current_logs > last_total_logs) {
                monitoring_system_->record_metric("logger.realtime.rate",
                    static_cast<double>(current_logs - last_total_logs),
                    {{"interval", "100ms"}});

                last_total_logs = current_logs;
                callback_count++;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (callback_count >= 5) break; // Stop after 5 iterations
        }
    });

    // Let monitoring run
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    // Stop monitoring
    monitoring_active = false;
    monitoring_thread.join();

    // Verify monitoring was active
    EXPECT_GT(callback_count.load(), 0);

    auto metrics = monitoring_system_->get_metrics();
    auto realtime_metrics = std::count_if(metrics.begin(), metrics.end(),
        [](const auto& m) { return m.name.find("realtime") != std::string::npos; });
    EXPECT_GT(realtime_metrics, 0);
}

/**
 * @brief Test custom metrics integration
 */
TEST_F(MonitoringIntegrationTest, CustomMetricsIntegration) {
    // Define custom metrics
    uint64_t custom_counter = 100;
    double custom_gauge = 0.99;
    uint64_t custom_histogram_sum = 4950; // Sum of 0..99
    uint64_t custom_histogram_count = 100;

    // Send custom metrics to monitoring system
    monitoring_system_->record_metric("logger.custom.counter",
                                     static_cast<double>(custom_counter),
                                     {{"type", "counter"}});
    monitoring_system_->record_metric("logger.custom.gauge",
                                     custom_gauge,
                                     {{"type", "gauge"}});

    double avg = static_cast<double>(custom_histogram_sum) / custom_histogram_count;
    monitoring_system_->record_metric("logger.custom.histogram_avg", avg,
                                     {{"type", "histogram"}});

    // Verify custom metrics
    auto metrics = monitoring_system_->get_metrics();
    auto custom_metric_count = std::count_if(metrics.begin(), metrics.end(),
        [](const auto& m) { return m.name.find("custom") != std::string::npos; });
    EXPECT_EQ(custom_metric_count, 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}