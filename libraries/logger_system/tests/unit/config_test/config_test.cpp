/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include <logger/config/logger_config.h>
#include <logger/config/logger_builder.h>
#include <logger/writers/console_writer.h>
#include <logger/filters/log_filter.h>

using namespace logger_module;

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }
    
    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test default configuration
TEST_F(ConfigTest, DefaultConfigValidation) {
    logger_config config = logger_config::default_config();
    
    // Default config should be valid
    auto result = config.validate();
    EXPECT_TRUE(result);
    
    // Check default values
    EXPECT_TRUE(config.async);
    EXPECT_EQ(config.buffer_size, 8192);
    EXPECT_EQ(config.min_level, thread_module::log_level::info);
    EXPECT_EQ(config.batch_size, 100);
    EXPECT_EQ(config.flush_interval.count(), 1000);
    EXPECT_FALSE(config.use_lock_free);
}

// Test invalid buffer size
TEST_F(ConfigTest, InvalidBufferSize) {
    logger_config config;
    
    // Zero buffer size
    config.buffer_size = 0;
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Extremely large buffer size
    config.buffer_size = std::numeric_limits<std::size_t>::max();
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid batch size
TEST_F(ConfigTest, InvalidBatchSize) {
    logger_config config;
    
    // Zero batch size
    config.batch_size = 0;
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Batch size larger than buffer
    config.batch_size = 100;
    config.buffer_size = 50;
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid flush interval
TEST_F(ConfigTest, InvalidFlushInterval) {
    logger_config config;
    
    // Negative flush interval
    config.flush_interval = std::chrono::milliseconds(-100);
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Very large flush interval
    config.flush_interval = std::chrono::milliseconds(7200000);  // 2 hours
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid queue settings
TEST_F(ConfigTest, InvalidQueueSettings) {
    logger_config config;
    
    // Zero queue size
    config.max_queue_size = 0;
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Queue size smaller than batch size
    config.max_queue_size = 50;
    config.batch_size = 100;
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid file settings
TEST_F(ConfigTest, InvalidFileSettings) {
    logger_config config;
    
    // Too small file size
    config.max_file_size = 512;  // Less than 1KB
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Zero file count
    config.max_file_size = 1024 * 1024;
    config.max_file_count = 0;
    result = config.validate();
    EXPECT_FALSE(result);
    
    // Too many files
    config.max_file_count = 1001;
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid network settings
TEST_F(ConfigTest, InvalidNetworkSettings) {
    logger_config config;
    
    // Host set but no port
    config.remote_host = "localhost";
    config.remote_port = 0;
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Invalid timeout
    config.remote_port = 8080;
    config.network_timeout = std::chrono::milliseconds(0);
    result = config.validate();
    EXPECT_FALSE(result);
    
    // Too many retries
    config.network_timeout = std::chrono::milliseconds(1000);
    config.network_retry_count = 101;
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid writer settings
TEST_F(ConfigTest, InvalidWriterSettings) {
    logger_config config;
    
    // Zero writers
    config.max_writers = 0;
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Too many writers
    config.max_writers = 101;
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid thread count
TEST_F(ConfigTest, InvalidThreadCount) {
    logger_config config;
    
    // Zero threads
    config.writer_thread_count = 0;
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Too many threads
    config.writer_thread_count = 33;
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test invalid feature combinations
TEST_F(ConfigTest, InvalidFeatureCombinations) {
    logger_config config;
    
    // Lock-free with grow policy
    config.use_lock_free = true;
    config.queue_overflow_policy = logger_config::overflow_policy::grow;
    auto result = config.validate();
    EXPECT_FALSE(result);
    
    // Sync mode with batch processing
    config.use_lock_free = false;
    config.queue_overflow_policy = logger_config::overflow_policy::drop_newest;
    config.async = false;
    config.batch_size = 10;
    result = config.validate();
    EXPECT_FALSE(result);
}

// Test predefined configurations
TEST_F(ConfigTest, PredefinedConfigurations) {
    // High performance config
    auto hp_config = logger_config::high_performance();
    EXPECT_TRUE(hp_config.validate());
    EXPECT_TRUE(hp_config.use_lock_free);
    EXPECT_EQ(hp_config.buffer_size, 65536);
    
    // Low latency config
    auto ll_config = logger_config::low_latency();
    EXPECT_TRUE(ll_config.validate());
    EXPECT_TRUE(ll_config.use_lock_free);
    EXPECT_EQ(ll_config.batch_size, 10);
    
    // Debug config
    auto debug_config = logger_config::debug_config();
    EXPECT_TRUE(debug_config.validate());
    EXPECT_FALSE(debug_config.async);
    EXPECT_EQ(debug_config.min_level, thread_module::log_level::trace);
    
    // Production config
    auto prod_config = logger_config::production();
    EXPECT_TRUE(prod_config.validate());
    EXPECT_TRUE(prod_config.enable_metrics);
    EXPECT_TRUE(prod_config.enable_crash_handler);
}

// Test logger builder basic functionality
TEST_F(ConfigTest, LoggerBuilderBasic) {
    logger_builder builder;
    
    // Configure builder
    builder.with_async(true)
           .with_buffer_size(4096)
           .with_min_level(thread_module::log_level::debug)
           .with_metrics(true);
    
    // Validate configuration
    auto validation = builder.validate();
    EXPECT_TRUE(validation);
    
    // Build logger
    auto result = builder.build();
    EXPECT_TRUE(result);
    EXPECT_NE(result.value(), nullptr);
}

// Test logger builder with writers
TEST_F(ConfigTest, LoggerBuilderWithWriters) {
    logger_builder builder;
    
    // Add console writer
    builder.add_writer("console", std::make_unique<console_writer>());
    
    // Build logger
    auto result = builder.build();
    EXPECT_TRUE(result);
}

// Test logger builder with filters
TEST_F(ConfigTest, LoggerBuilderWithFilters) {
    logger_builder builder;
    
    // Add level filter
    builder.add_filter(std::make_unique<level_filter>(thread_module::log_level::warning));
    
    // Build logger
    auto result = builder.build();
    EXPECT_TRUE(result);
}

// Test logger builder templates
TEST_F(ConfigTest, LoggerBuilderTemplates) {
    logger_builder builder;
    
    // Test each template
    std::vector<std::string> templates = {
        "default",
        "high_performance",
        "low_latency",
        "debug",
        "production"
    };
    
    for (const auto& tmpl : templates) {
        builder.use_template(tmpl);
        auto validation = builder.validate();
        EXPECT_TRUE(validation) << "Template " << tmpl << " failed validation";
        
        auto result = builder.build();
        EXPECT_TRUE(result) << "Template " << tmpl << " failed to build";
    }
}

// Test logger builder with invalid config
TEST_F(ConfigTest, LoggerBuilderInvalidConfig) {
    logger_builder builder;
    
    // Set invalid configuration
    builder.with_buffer_size(0);
    
    // Validation should fail
    auto validation = builder.validate();
    EXPECT_FALSE(validation);
    
    // Build should fail
    auto result = builder.build();
    EXPECT_FALSE(result);
}

// Test logger builder fluent interface
TEST_F(ConfigTest, LoggerBuilderFluentInterface) {
    auto result = logger_builder()
        .use_template("production")
        .with_min_level(thread_module::log_level::info)
        .with_buffer_size(16384)
        .with_metrics(true)
        .with_crash_handler(true)
        .add_writer("console", std::make_unique<console_writer>())
        .build();
    
    EXPECT_TRUE(result);
    EXPECT_NE(result.value(), nullptr);
}

// Test config modification after validation
TEST_F(ConfigTest, ConfigModificationTracking) {
    logger_config config;
    
    // Initial validation should pass
    EXPECT_TRUE(config.validate());
    
    // Modify to invalid state
    config.buffer_size = 0;
    
    // Validation should now fail
    EXPECT_FALSE(config.validate());
    
    // Fix the issue
    config.buffer_size = 8192;
    
    // Validation should pass again
    EXPECT_TRUE(config.validate());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}