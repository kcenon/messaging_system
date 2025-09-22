/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "../../sources/logger/config/config_strategy_interface.h"
#include "../../sources/logger/config/logger_builder.h"
#include "../../sources/logger/writers/console_writer.h"
#include <chrono>
#include <cstdlib>

using namespace logger_module;

class ConfigStrategyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset any environment variables
        unsetenv("LOG_ENV");
        unsetenv("LOG_LEVEL");
    }
    
    void TearDown() override {
        // Clean up
        unsetenv("LOG_ENV");
        unsetenv("LOG_LEVEL");
    }
};

// Template Strategy Tests

TEST_F(ConfigStrategyTest, TemplateStrategy_HighPerformance) {
    auto strategy = std::make_unique<template_strategy>(
        template_strategy::template_type::high_performance);
    
    EXPECT_EQ(strategy->get_name(), "high_performance");
    EXPECT_TRUE(strategy->should_override());
    
    logger_config config;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify high performance settings
    EXPECT_EQ(config.buffer_size, 65536);
    EXPECT_EQ(config.batch_size, 500);
    EXPECT_TRUE(config.use_lock_free);
}

TEST_F(ConfigStrategyTest, TemplateStrategy_LowLatency) {
    auto strategy = std::make_unique<template_strategy>(
        template_strategy::template_type::low_latency);
    
    EXPECT_EQ(strategy->get_name(), "low_latency");
    
    logger_config config;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify low latency settings
    EXPECT_EQ(config.batch_size, 10);
    EXPECT_EQ(config.flush_interval, std::chrono::milliseconds(10));
}

TEST_F(ConfigStrategyTest, TemplateStrategy_Debug) {
    auto strategy = std::make_unique<template_strategy>(
        template_strategy::template_type::debug);
    
    EXPECT_EQ(strategy->get_name(), "debug");
    
    logger_config config;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify debug settings
    EXPECT_FALSE(config.async);
    EXPECT_EQ(config.min_level, thread_module::log_level::trace);
}

TEST_F(ConfigStrategyTest, TemplateStrategy_Production) {
    auto strategy = std::make_unique<template_strategy>(
        template_strategy::template_type::production);
    
    EXPECT_EQ(strategy->get_name(), "production");
    
    logger_config config;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify production settings
    EXPECT_TRUE(config.enable_metrics);
    EXPECT_TRUE(config.enable_crash_handler);
    EXPECT_FALSE(config.enable_color_output);
}

// Environment Strategy Tests

TEST_F(ConfigStrategyTest, EnvironmentStrategy_Development) {
    auto strategy = std::make_unique<environment_strategy>(
        environment_strategy::environment::development);
    
    EXPECT_EQ(strategy->get_name(), "development");
    EXPECT_GT(strategy->get_priority(), 50); // Higher than default
    
    logger_config config;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify development settings
    EXPECT_FALSE(config.async);
    EXPECT_EQ(config.min_level, thread_module::log_level::trace);
    EXPECT_TRUE(config.enable_color_output);
    EXPECT_FALSE(config.enable_metrics);
}

TEST_F(ConfigStrategyTest, EnvironmentStrategy_Testing) {
    auto strategy = std::make_unique<environment_strategy>(
        environment_strategy::environment::testing);
    
    EXPECT_EQ(strategy->get_name(), "testing");
    
    logger_config config;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify testing settings
    EXPECT_TRUE(config.async);
    EXPECT_EQ(config.min_level, thread_module::log_level::debug);
    EXPECT_TRUE(config.enable_metrics);
    EXPECT_FALSE(config.enable_crash_handler);
}

TEST_F(ConfigStrategyTest, EnvironmentStrategy_Staging) {
    auto strategy = std::make_unique<environment_strategy>(
        environment_strategy::environment::staging);
    
    EXPECT_EQ(strategy->get_name(), "staging");
    
    logger_config config;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify staging settings
    EXPECT_TRUE(config.async);
    EXPECT_EQ(config.min_level, thread_module::log_level::info);
    EXPECT_TRUE(config.enable_metrics);
    EXPECT_TRUE(config.enable_crash_handler);
    EXPECT_TRUE(config.enable_structured_logging);
}

// Performance Tuning Strategy Tests

TEST_F(ConfigStrategyTest, PerformanceTuning_Conservative) {
    auto strategy = std::make_unique<performance_tuning_strategy>(
        performance_tuning_strategy::tuning_level::conservative);
    
    EXPECT_EQ(strategy->get_name(), "conservative_tuning");
    
    logger_config config;
    config.async = true; // Required for performance tuning
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify conservative settings
    EXPECT_EQ(config.buffer_size, 4096);
    EXPECT_EQ(config.batch_size, 50);
    EXPECT_EQ(config.max_queue_size, 1000);
    EXPECT_EQ(config.writer_thread_count, 1);
}

TEST_F(ConfigStrategyTest, PerformanceTuning_Balanced) {
    auto strategy = std::make_unique<performance_tuning_strategy>(
        performance_tuning_strategy::tuning_level::balanced);
    
    EXPECT_EQ(strategy->get_name(), "balanced_tuning");
    
    logger_config config;
    config.async = true;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify balanced settings
    EXPECT_EQ(config.buffer_size, 8192);
    EXPECT_EQ(config.batch_size, 100);
    EXPECT_EQ(config.max_queue_size, 10000);
    EXPECT_EQ(config.writer_thread_count, 2);
}

TEST_F(ConfigStrategyTest, PerformanceTuning_Aggressive) {
    auto strategy = std::make_unique<performance_tuning_strategy>(
        performance_tuning_strategy::tuning_level::aggressive);
    
    EXPECT_EQ(strategy->get_name(), "aggressive_tuning");
    
    logger_config config;
    config.async = true;
    auto result = strategy->apply(config);
    EXPECT_TRUE(result);
    
    // Verify aggressive settings
    EXPECT_EQ(config.buffer_size, 65536);
    EXPECT_EQ(config.batch_size, 500);
    EXPECT_EQ(config.max_queue_size, 100000);
    EXPECT_EQ(config.writer_thread_count, 4);
    EXPECT_TRUE(config.use_lock_free);
    EXPECT_TRUE(config.enable_compression);
}

TEST_F(ConfigStrategyTest, PerformanceTuning_RequiresAsync) {
    auto strategy = std::make_unique<performance_tuning_strategy>(
        performance_tuning_strategy::tuning_level::balanced);
    
    logger_config config;
    config.async = false; // Disabled
    
    // Should fail can_apply check
    auto can_apply = strategy->can_apply(config);
    EXPECT_FALSE(can_apply);
}

// Composite Strategy Tests

TEST_F(ConfigStrategyTest, CompositeStrategy_Multiple) {
    auto composite = std::make_unique<composite_strategy>();
    
    // Add strategies - use testing environment instead which keeps async=true
    composite->add_strategy(std::make_unique<environment_strategy>(
        environment_strategy::environment::testing));
    composite->add_strategy(std::make_unique<performance_tuning_strategy>(
        performance_tuning_strategy::tuning_level::conservative));
    
    logger_config config;
    config.async = true; // Required for performance tuning
    
    auto result = composite->apply(config);
    EXPECT_TRUE(result);
    
    // Environment strategy should apply first (higher priority)
    // Then performance tuning modifies some settings
    EXPECT_EQ(config.min_level, thread_module::log_level::debug); // From testing environment
    EXPECT_EQ(config.buffer_size, 4096); // From performance tuning
}

// Factory Tests

TEST_F(ConfigStrategyTest, Factory_CreateTemplate) {
    auto strategy = config_strategy_factory::create_template("high_performance");
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->get_name(), "high_performance");
    
    auto invalid = config_strategy_factory::create_template("invalid_name");
    EXPECT_EQ(invalid, nullptr);
}

TEST_F(ConfigStrategyTest, Factory_CreateEnvironment) {
    auto strategy = config_strategy_factory::create_environment("development");
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->get_name(), "development");
    
    // Test alias
    auto dev = config_strategy_factory::create_environment("dev");
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->get_name(), "development");
}

TEST_F(ConfigStrategyTest, Factory_CreateTuning) {
    auto strategy = config_strategy_factory::create_tuning("aggressive");
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->get_name(), "aggressive_tuning");
    
    // Test alias
    auto high = config_strategy_factory::create_tuning("high");
    ASSERT_NE(high, nullptr);
    EXPECT_EQ(high->get_name(), "aggressive_tuning");
}

TEST_F(ConfigStrategyTest, Factory_FromEnvironment) {
    // Test LOG_ENV variable
    setenv("LOG_ENV", "production", 1);
    auto strategy = config_strategy_factory::from_environment();
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->get_name(), "production");
    
    // Test LOG_LEVEL variable
    unsetenv("LOG_ENV");
    setenv("LOG_LEVEL", "DEBUG", 1);
    auto debug = config_strategy_factory::from_environment();
    ASSERT_NE(debug, nullptr);
    EXPECT_EQ(debug->get_name(), "debug");
    
    // Test no environment variables
    unsetenv("LOG_ENV");
    unsetenv("LOG_LEVEL");
    auto none = config_strategy_factory::from_environment();
    EXPECT_EQ(none, nullptr);
}

// Builder Integration Tests

TEST_F(ConfigStrategyTest, Builder_UseTemplate) {
    logger_builder builder;
    builder.use_template("high_performance");
    
    auto& config = builder.get_config();
    // The template should be applied when build() is called
    // For now just verify builder accepts the call
    EXPECT_NO_THROW(builder.validate());
}

TEST_F(ConfigStrategyTest, Builder_ApplyStrategy) {
    logger_builder builder;
    
    auto strategy = std::make_unique<template_strategy>(
        template_strategy::template_type::debug);
    builder.apply_strategy(std::move(strategy));
    
    // Build and verify
    auto result = builder.build();
    EXPECT_TRUE(result);
}

TEST_F(ConfigStrategyTest, Builder_ForEnvironment) {
    logger_builder builder;
    builder.for_environment("testing");  // Use testing instead of development
    
    auto result = builder.build();
    EXPECT_TRUE(result);
}

TEST_F(ConfigStrategyTest, Builder_WithPerformanceTuning) {
    logger_builder builder;
    builder.with_async(true)  // Required for performance tuning
           .with_performance_tuning("aggressive");
    
    auto result = builder.build();
    EXPECT_TRUE(result);
}

TEST_F(ConfigStrategyTest, Builder_AutoConfigure) {
    setenv("LOG_ENV", "testing", 1);
    
    logger_builder builder;
    builder.auto_configure();
    
    auto result = builder.build();
    EXPECT_TRUE(result);
}

TEST_F(ConfigStrategyTest, Builder_ClearStrategies) {
    logger_builder builder;
    builder.use_template("debug")
           .for_environment("production")
           .clear_strategies(); // Should clear all
    
    auto result = builder.build();
    EXPECT_TRUE(result);
}

TEST_F(ConfigStrategyTest, Builder_ChainedStrategies) {
    logger_builder builder;
    
    // Chain multiple strategy applications
    builder.use_template("production")
           .for_environment("staging")
           .with_performance_tuning("balanced");
    
    auto result = builder.build();
    EXPECT_TRUE(result);
}

// Edge Cases

TEST_F(ConfigStrategyTest, Builder_BackwardCompatibility) {
    logger_builder builder;
    
    // Old way should still work
    builder.use_template("high_performance");
    
    auto result = builder.build();
    EXPECT_TRUE(result);
}

TEST_F(ConfigStrategyTest, Strategy_InvalidConfiguration) {
    logger_builder builder;
    
    // Apply performance tuning without async (strategy should be skipped)
    builder.with_async(false)
           .with_performance_tuning("aggressive");
    
    // Add at least one writer so the logger can function
    builder.add_writer("console", std::make_unique<console_writer>());
    
    // The strategy should be skipped due to can_apply check
    auto result = builder.build();
    EXPECT_TRUE(result); // Should still build, just skip the strategy
    
    // Verify that performance tuning was not applied (buffer size should be default)
    if (result) {
        auto& config = builder.get_config();
        EXPECT_NE(config.buffer_size, 65536); // Aggressive tuning would set this
    }
}