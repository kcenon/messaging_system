/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/file_writer.h>
#include <logger/writers/rotating_file_writer.h>
#include <logger/writers/network_writer.h>
#include <logger/writers/encrypted_writer.h>
#include <logger/structured/structured_logger.h>
#include <logger/filters/log_filter.h>
#include <logger/routing/log_router.h>
#include <logger/server/log_server.h>
#include <logger/analysis/log_analyzer.h>
#include <logger/security/log_sanitizer.h>
#include <thread>
#include <chrono>
#include <filesystem>

using namespace logger_module;
using namespace std::chrono_literals;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up test files
        cleanup_test_files();
    }
    
    void TearDown() override {
        // Clean up test files
        cleanup_test_files();
    }
    
    void cleanup_test_files() {
        std::vector<std::string> test_files = {
            "test_integration.log",
            "test_rotating.log",
            "test_encrypted.log",
            "test_secure.log",
            "test.key"
        };
        
        for (const auto& file : test_files) {
            std::filesystem::remove(file);
        }
        
        // Remove rotating file backups
        for (int i = 1; i <= 5; ++i) {
            std::filesystem::remove("test_rotating.log." + std::to_string(i));
        }
    }
};

TEST_F(IntegrationTest, FullPipelineTest) {
    // Create logger with multiple writers
    auto logger = std::make_shared<logger_module::logger>();
    
    // Add various writers
    logger->add_writer("console", std::make_unique<console_writer>());
    logger->add_writer("file", std::make_unique<file_writer>("test_integration.log"));
    logger->add_writer("rotating", std::make_unique<rotating_file_writer>(
        "test_rotating.log", 1024, 3));
    
    // Enable metrics
    logger->enable_metrics_collection(true);
    
    // Add filters
    auto level_filter = std::make_unique<logger_module::level_filter>(
        thread_module::log_level::debug);
    logger->set_filter(std::move(level_filter));
    
    // Configure routing
    auto& router = logger->get_router();
    router_builder(router)
        .when_level(thread_module::log_level::error)
        .route_to("file", true);
    
    // Log various messages
    for (int i = 0; i < 100; ++i) {
        if (i % 10 == 0) {
            logger->log(thread_module::log_level::error, 
                       "Error message " + std::to_string(i));
        } else if (i % 5 == 0) {
            logger->log(thread_module::log_level::warning,
                       "Warning message " + std::to_string(i));
        } else {
            logger->log(thread_module::log_level::info,
                       "Info message " + std::to_string(i));
        }
    }
    
    // Check metrics
    auto metrics = logger->get_current_metrics();
    EXPECT_GT(metrics.messages_enqueued.load(), 0);
    EXPECT_GT(metrics.get_messages_per_second(), 0);
    
    logger->flush();
    
    // Verify files exist
    EXPECT_TRUE(std::filesystem::exists("test_integration.log"));
    EXPECT_TRUE(std::filesystem::exists("test_rotating.log"));
}

TEST_F(IntegrationTest, StructuredLoggingTest) {
    auto logger = std::make_shared<logger_module::logger>();
    logger->add_writer(std::make_unique<file_writer>("test_integration.log"));
    
    // Test different formats
    std::vector<structured_logger::output_format> formats = {
        structured_logger::output_format::json,
        structured_logger::output_format::logfmt,
        structured_logger::output_format::plain
    };
    
    for (auto format : formats) {
        auto structured = std::make_shared<structured_logger>(logger, format);
        
        structured->info("Test message")
            .field("format", static_cast<int>(format))
            .field("string", "value")
            .field("number", 42)
            .field("float", 3.14)
            .field("bool", true)
            .commit();
    }
    
    logger->flush();
    
    // Verify file has content
    std::ifstream file("test_integration.log");
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    EXPECT_FALSE(content.empty());
    
    // Check for format-specific content
    EXPECT_NE(content.find("{"), std::string::npos);  // JSON
    EXPECT_NE(content.find("format="), std::string::npos);  // logfmt
}

TEST_F(IntegrationTest, NetworkLoggingTest) {
    // Start log server
    auto server = std::make_unique<log_server>(9998, true);
    
    std::atomic<int> received_count{0};
    server->add_handler([&received_count](const log_server::network_log_entry& entry) {
        (void)entry;  // Suppress unused parameter warning
        received_count++;
    });
    
    ASSERT_TRUE(server->start());
    std::this_thread::sleep_for(100ms);  // Let server start
    
    // Create logger with network writer
    auto logger = std::make_shared<logger_module::logger>();
    logger->add_writer(std::make_unique<network_writer>(
        "127.0.0.1", 9998, network_writer::protocol_type::tcp));
    
    // Send some logs
    for (int i = 0; i < 10; ++i) {
        logger->log(thread_module::log_level::info,
                   "Network message " + std::to_string(i));
    }
    
    logger->flush();
    std::this_thread::sleep_for(200ms);  // Let messages arrive
    
    // Verify messages received
    EXPECT_GT(received_count.load(), 0);
    
    server->stop();
}

TEST_F(IntegrationTest, SecurityFeaturesTest) {
    // Test encryption
    auto key = encrypted_writer::generate_key();
    ASSERT_EQ(key.size(), 32);  // AES-256 key size
    
    // Save and load key
    ASSERT_TRUE(encrypted_writer::save_key(key, "test.key"));
    auto loaded_key = encrypted_writer::load_key("test.key");
    ASSERT_EQ(key, loaded_key);
    
    // Create logger with encrypted writer
    auto logger = std::make_shared<logger_module::logger>();
    auto file = std::make_unique<file_writer>("test_encrypted.log");
    auto encrypted = std::make_unique<encrypted_writer>(std::move(file), key);
    logger->add_writer(std::move(encrypted));
    
    logger->log(thread_module::log_level::info, "Encrypted message");
    logger->flush();
    
    // Test sanitizer
    auto sanitizer = std::make_shared<log_sanitizer>();
    
    // Test various patterns
    EXPECT_EQ(sanitizer->sanitize("Normal message"), "Normal message");
    
    auto cc_result = sanitizer->sanitize("Card: 4532-1234-5678-9012");
    EXPECT_NE(cc_result.find("4532"), std::string::npos);
    EXPECT_NE(cc_result.find("9012"), std::string::npos);
    EXPECT_NE(cc_result.find("*"), std::string::npos);
    
    auto email_result = sanitizer->sanitize("Email: test@example.com");
    EXPECT_NE(email_result.find("t**t@example.com"), std::string::npos);
    
    // Test access control - default permission is write_info
    auto access_filter = std::make_unique<access_control_filter>(
        access_control_filter::permission_level::write_info);
    access_filter->set_user_context("test_user", 
        access_control_filter::permission_level::write_info);
    
    // Info level should be allowed
    EXPECT_TRUE(access_filter->should_log(
        thread_module::log_level::info, "test", "file.cpp", 1, "func"));
    
    // Debug level should be blocked
    EXPECT_FALSE(access_filter->should_log(
        thread_module::log_level::debug, "test", "file.cpp", 1, "func"));
}

TEST_F(IntegrationTest, AnalysisTest) {
    // Create analyzer
    auto analyzer = std::make_unique<log_analyzer>(
        std::chrono::seconds(1), 10);
    
    // Add patterns
    analyzer->add_pattern("errors", "error|fail");
    analyzer->add_pattern("warnings", "warn");
    
    // Add alert rule
    bool alert_triggered = false;
    analyzer->add_alert_rule({
        "high_error_rate",
        [](const auto& stats) {
            auto error_count = stats.level_counts.count(thread_module::log_level::error) ?
                              stats.level_counts.at(thread_module::log_level::error) : 0;
            return error_count > 5;
        },
        [&alert_triggered](const std::string& rule, const auto& stats) {
            (void)rule;  // Suppress unused parameter warning
            (void)stats;  // Suppress unused parameter warning
            alert_triggered = true;
        }
    });
    
    // Analyze logs
    auto now = std::chrono::system_clock::now();
    for (int i = 0; i < 10; ++i) {
        analyzer->analyze(
            thread_module::log_level::error,
            "Error occurred",
            "test.cpp",
            100,
            "test_func",
            now + std::chrono::milliseconds(i * 100)
        );
    }
    
    // Check alert was triggered
    EXPECT_TRUE(alert_triggered);
    
    // Get current stats
    auto current_stats = analyzer->get_current_stats();
    EXPECT_GT(current_stats.level_counts[thread_module::log_level::error], 5);
    EXPECT_GT(current_stats.pattern_matches["errors"], 0);
    
    // Generate report
    auto report = analyzer->generate_report(std::chrono::seconds(1));
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("Log Analysis Report"), std::string::npos);
}

TEST_F(IntegrationTest, StressTest) {
    // Create logger with multiple writers
    auto logger = std::make_shared<logger_module::logger>();
    logger->add_writer(std::make_unique<file_writer>("test_integration.log"));
    logger->enable_metrics_collection(true);
    
    // Add sanitizer
    auto sanitizer = std::make_shared<log_sanitizer>();
    
    // Create analyzer
    auto analyzer = std::make_unique<log_analyzer>(
        std::chrono::seconds(1), 60);
    
    // Multi-threaded logging
    const int thread_count = 4;
    const int messages_per_thread = 1000;
    std::vector<std::thread> threads;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                std::string msg = "Thread " + std::to_string(t) + 
                                 " message " + std::to_string(i);
                
                // Add some sensitive data randomly
                if (i % 10 == 0) {
                    msg += " card: 4111-1111-1111-1111";
                }
                if (i % 15 == 0) {
                    msg += " email: user@test.com";
                }
                
                // Sanitize and log
                std::string sanitized = sanitizer->sanitize(msg);
                logger->log(thread_module::log_level::info, sanitized);
                
                // Analyze
                analyzer->analyze(
                    thread_module::log_level::info,
                    sanitized,
                    __FILE__,
                    __LINE__,
                    __func__,
                    std::chrono::system_clock::now()
                );
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    // Check performance
    auto metrics = logger->get_current_metrics();
    EXPECT_EQ(metrics.messages_enqueued.load(), thread_count * messages_per_thread);
    
    std::cout << "Stress test completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Messages per second: " << metrics.get_messages_per_second() << std::endl;
    std::cout << "Average enqueue time: " << metrics.get_avg_enqueue_time_ns() << " ns" << std::endl;
    
    logger->flush();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}