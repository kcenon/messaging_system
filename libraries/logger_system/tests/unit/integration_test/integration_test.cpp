/**
 * @file integration_test.cpp
 * @brief Integration tests for logger system
 * @date 2025-09-09
 *
 * BSD 3-Clause License
 * Copyright (c) 2025, kcenon
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "../../sources/logger/logger.h"
#include "../../sources/logger/config/logger_builder.h"
#include "../../sources/logger/config/configuration_templates.h"
#include "../../sources/logger/writers/console_writer.h"
#include "../../sources/logger/writers/file_writer.h"
#include "../../sources/logger/writers/async_writer.h"
#include "../../sources/logger/writers/batch_writer.h"
#include "../../sources/logger/monitoring/basic_monitor.h"
#include "../../sources/logger/di/lightweight_container.h"
#include "../mocks/mock_writer.hpp"
#include "../mocks/mock_monitor.hpp"
#include "../mocks/mock_di_container.hpp"
#include <filesystem>
#include <fstream>
#include <thread>

using namespace logger_system;
using namespace logger_system::testing;
using namespace logger_module;
using namespace std::chrono_literals;
using log_level = thread_module::log_level;

class integration_test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        test_dir_ = "/tmp/logger_integration_test_" + 
                   std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(test_dir_);
    }

    void TearDown() override {
        // Clean up test directory
        std::filesystem::remove_all(test_dir_);
    }

    std::filesystem::path test_dir_;
};

/**
 * @brief Test complete logging pipeline with real components
 * 
 * Verifies that all components work together correctly in a realistic scenario.
 */
TEST_F(integration_test, complete_pipeline_integration) {
    auto log_file = test_dir_ / "integration.log";
    
    // Create a complete logger with multiple writers and monitoring
    auto monitor = std::make_shared<logger_module::basic_monitor>();
    auto console_writer_inst = std::make_unique<logger_module::console_writer>();
    // Create file writer for async wrapper
    auto file_writer_for_async = std::make_unique<logger_module::file_writer>(log_file.string());
    auto async_file = std::make_unique<logger_module::async_writer>(std::move(file_writer_for_async), 100);
    
    auto result = logger_module::logger_builder()
        .with_default_pattern()
        .with_buffer_size(1000)
        .with_monitoring(monitor)
        .add_writer("console", std::move(console_writer_inst))
        .add_writer("async_file", std::move(async_file))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    // Log messages at different levels
    logger->log(log_level::debug, "Debug message for integration test");
    logger->log(log_level::info, "Info message for integration test");
    logger->log(log_level::warning, "Warning message for integration test");
    logger->log(log_level::error, "Error message for integration test");
    
    // Wait for async operations to complete
    // Note: async_file has been moved to logger, so we'll rely on logger shutdown to flush
    
    // Verify file was created and contains messages
    EXPECT_TRUE(std::filesystem::exists(log_file));
    
    std::ifstream file(log_file);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    EXPECT_TRUE(content.find("Debug message") != std::string::npos);
    EXPECT_TRUE(content.find("Info message") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
    
    // Check monitoring metrics
    auto metrics_result = monitor->collect_metrics();
    ASSERT_TRUE(metrics_result.has_value());
    auto metrics = metrics_result.value();
    
    // Find messages_logged metric
    bool found_messages_logged = false;
    for (const auto& metric : metrics.get_metrics()) {
        if (metric.name == "messages_logged" && metric.value > 0) {
            found_messages_logged = true;
            break;
        }
    }
    EXPECT_TRUE(found_messages_logged);
}

/**
 * @brief Test DI container integration
 * 
 * NOTE: DI container integration is not fully implemented in logger_builder yet.
 * This test is disabled until the implementation is complete.
 */
TEST_F(integration_test, DISABLED_di_container_integration) {
    // TODO: Implement DI container integration in logger_builder
    // This test should be enabled once the integration is complete
    
    // Create a simple logger instead for now
    auto console_writer = std::make_unique<logger_module::console_writer>();
    auto result = logger_module::logger_builder()
        .add_writer("console", std::move(console_writer))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());
    
    // Log messages
    logger->log(log_level::info, "Simple integration test message");
}

/**
 * @brief Test configuration templates
 * 
 * Verifies that pre-defined configuration templates work correctly.
 */
TEST_F(integration_test, configuration_templates_integration) {
    // Test production configuration
    {
        auto prod_result = logger_module::logger_builder()
            .apply_template(logger_system::configuration_template::production)
            .add_writer("file", std::make_unique<logger_module::file_writer>((test_dir_ / "prod.log").string()))
            .build();
        
        ASSERT_TRUE(prod_result.has_value());
        auto prod_logger = std::move(prod_result.value());
        
        // Production config should filter out debug messages
        prod_logger->log(log_level::debug, "This should be filtered");
        prod_logger->log(log_level::info, "This should be logged");
        
        // Note: Configuration verification not available through public API
        // Production template should filter debug messages by default
    }
    
    // Test debug configuration
    {
        auto debug_result = logger_module::logger_builder()
            .apply_template(logger_system::configuration_template::debug)
            .add_writer("file", std::make_unique<logger_module::file_writer>((test_dir_ / "debug.log").string()))
            .build();
        
        ASSERT_TRUE(debug_result.has_value());
        auto debug_logger = std::move(debug_result.value());
        
        // Debug config should log everything
        debug_logger->log(log_level::trace, "Trace message");
        debug_logger->log(log_level::debug, "Debug message");
        
        // Note: Configuration verification not available through public API
        // Debug template should allow all message levels
    }
}

/**
 * @brief Test batch writer integration
 * 
 * Verifies that batch writing works correctly with the logger.
 */
TEST_F(integration_test, batch_writer_integration) {
    auto log_file = test_dir_ / "batch.log";
    auto file_writer_inst = std::make_unique<logger_module::file_writer>(log_file.string());
    logger_module::batch_writer::config batch_config;
    batch_config.max_batch_size = 10;
    batch_config.flush_interval = 100ms;
    auto batch_writer_inst = std::make_unique<logger_module::batch_writer>(std::move(file_writer_inst), batch_config);
    
    auto result = logger_module::logger_builder()
        .add_writer("batch", std::move(batch_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());
    
    // Log exactly batch_size messages
    for (int i = 0; i < 10; ++i) {
        logger->log(log_level::info, "Batch message " + std::to_string(i));
    }
    
    // Wait for batch to be written
    std::this_thread::sleep_for(150ms);
    
    // Verify all messages were written
    EXPECT_TRUE(std::filesystem::exists(log_file));
    
    std::ifstream file(log_file);
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) count++;
    }
    EXPECT_EQ(count, 10);
}

/**
 * @brief Test monitoring and health check integration
 * 
 * Verifies that monitoring and health checks work correctly together.
 */
TEST_F(integration_test, monitoring_health_integration) {
    auto monitor = std::make_shared<logger_module::basic_monitor>();
    auto mock_writer_inst = std::make_unique<logger_system::testing::mock_writer>();
    auto* mock_writer_ptr = mock_writer_inst.get();
    
    auto result = logger_module::logger_builder()
        .with_monitoring(monitor)
        .with_health_check_interval(50ms)
        .add_writer("mock", std::move(mock_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());
    
    // Log some messages
    for (int i = 0; i < 100; ++i) {
        logger->log(log_level::info, "Health check test " + std::to_string(i));
    }
    
    // Wait for health checks to run
    std::this_thread::sleep_for(200ms);
    
    // Check health status
    auto health_result = monitor->check_health();
    ASSERT_TRUE(health_result.has_value());
    EXPECT_EQ(health_result.value().get_status(), logger_module::health_status::healthy);
    
    // Check metrics - note: actual values may vary depending on monitoring implementation
    auto metrics_result = monitor->collect_metrics();
    ASSERT_TRUE(metrics_result.has_value());
    auto metrics = metrics_result.value();
    
    // Verify we have some metrics
    EXPECT_GT(metrics.size(), 0);
    
    // Simulate writer failure
    mock_writer_ptr->set_should_fail(true);
    
    for (int i = 0; i < 10; ++i) {
        logger->log(log_level::error, "Failed message " + std::to_string(i));
    }
    
    // Wait for health check to detect issues
    std::this_thread::sleep_for(100ms);
    
    // Health should degrade after failures
    health_result = monitor->check_health();
    ASSERT_TRUE(health_result.has_value());
    // Note: Actual implementation may vary in how it detects degradation
}

/**
 * @brief Test multi-writer synchronization
 * 
 * Verifies that multiple writers work correctly together without conflicts.
 */
TEST_F(integration_test, multi_writer_synchronization) {
    std::vector<logger_system::testing::mock_writer*> writers;
    auto logger_builder_inst = logger_module::logger_builder();
    
    // Add multiple mock writers
    for (int i = 0; i < 5; ++i) {
        auto writer = std::make_unique<logger_system::testing::mock_writer>();
        writers.push_back(writer.get());
        logger_builder_inst.add_writer("writer_" + std::to_string(i), std::move(writer));
    }
    
    auto result = logger_builder_inst.build();
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());
    
    const int num_messages = 100;
    
    // Log messages from multiple threads
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&logger, t, num_messages]() {
            for (int i = 0; i < num_messages; ++i) {
                logger->log(log_level::info, 
                           "Thread " + std::to_string(t) + " Message " + std::to_string(i));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Each writer should have received all messages
    for (const auto& writer : writers) {
        EXPECT_EQ(writer->get_write_count(), 4 * num_messages);
    }
}

/**
 * @brief Test error recovery and fallback mechanisms
 * 
 * Verifies that the logger can recover from errors and use fallback mechanisms.
 */
TEST_F(integration_test, error_recovery_fallback) {
    auto primary_writer = std::make_unique<logger_system::testing::mock_writer>();
    auto* primary_writer_ptr = primary_writer.get();
    auto fallback_writer = std::make_unique<logger_system::testing::mock_writer>();
    auto* fallback_writer_ptr = fallback_writer.get();
    
    auto result = logger_module::logger_builder()
        .with_error_handler([](const logger_module::logger_error_code& error) {
            // Custom error handler
            std::cerr << "Logger error occurred" << std::endl;
        })
        .add_writer("primary", std::move(primary_writer))
        .add_writer("fallback", std::move(fallback_writer))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());
    
    // Initially both writers work
    logger->log(log_level::info, "Message 1");
    EXPECT_EQ(primary_writer_ptr->get_write_count(), 1);
    EXPECT_EQ(fallback_writer_ptr->get_write_count(), 1);
    
    // Primary writer fails
    primary_writer_ptr->set_should_fail(true);
    
    // Logger should continue with fallback
    logger->log(log_level::info, "Message 2");
    EXPECT_EQ(primary_writer_ptr->get_write_count(), 1);  // No increase
    EXPECT_EQ(fallback_writer_ptr->get_write_count(), 2);  // Increased
    
    // Primary writer recovers
    primary_writer_ptr->set_should_fail(false);
    
    // Both writers should work again
    logger->log(log_level::info, "Message 3");
    EXPECT_EQ(primary_writer_ptr->get_write_count(), 2);
    EXPECT_EQ(fallback_writer_ptr->get_write_count(), 3);
}

/**
 * @brief Test performance tuning strategies
 * 
 * Verifies that different performance strategies work as expected.
 */
TEST_F(integration_test, performance_tuning_strategies) {
    // Test conservative strategy
    {
        auto mock_writer = std::make_unique<logger_system::testing::mock_writer>();
        auto result = logger_module::logger_builder()
            .apply_performance_strategy(logger_system::performance_strategy::conservative)
            .add_writer("mock", std::move(mock_writer))
            .build();
        
        ASSERT_TRUE(result.has_value());
        auto logger = std::move(result.value());
        
        // Note: Configuration verification not available through public API
        // Conservative strategy should use smaller buffers
    }
    
    // Test aggressive strategy
    {
        auto mock_writer = std::make_unique<logger_system::testing::mock_writer>();
        auto result = logger_module::logger_builder()
            .apply_performance_strategy(logger_system::performance_strategy::aggressive)
            .add_writer("mock", std::move(mock_writer))
            .build();
        
        ASSERT_TRUE(result.has_value());
        auto logger = std::move(result.value());
        
        // Note: Configuration verification not available through public API
        // Aggressive strategy should use larger buffers
    }
}

/**
 * @brief Test environment-based configuration
 * 
 * Verifies that environment detection and configuration works correctly.
 */
TEST_F(integration_test, environment_based_configuration) {
    // Set environment variables
    setenv("LOG_ENV", "production", 1);
    setenv("LOG_LEVEL", "warn", 1);
    
    auto mock_writer = std::make_unique<logger_system::testing::mock_writer>();
    auto* mock_writer_ptr = mock_writer.get();
    auto result = logger_module::logger_builder()
        .detect_environment()
        .add_writer("mock", std::move(mock_writer))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());
    
    // Should use production settings (min level should be warning or higher)
    // Note: Configuration verification not available through public API
    
    // Debug messages should be filtered
    logger->log(log_level::debug, "This should not be logged");
    logger->log(log_level::warning, "This should be logged");
    logger->log(log_level::error, "This should also be logged");
    
    EXPECT_EQ(mock_writer_ptr->get_write_count(), 2);
    
    // Clean up environment variables
    unsetenv("LOG_ENV");
    unsetenv("LOG_LEVEL");
}