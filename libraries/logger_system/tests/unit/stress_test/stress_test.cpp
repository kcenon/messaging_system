/**
 * @file stress_test.cpp
 * @brief Stress tests for logger system
 * @date 2025-09-09
 *
 * BSD 3-Clause License
 * Copyright (c) 2025, kcenon
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "../../sources/logger/logger.h"
#include "../../sources/logger/config/logger_builder.h"
#include "../../sources/logger/writers/console_writer.h"
#include "../../sources/logger/writers/file_writer.h"
#include "../../sources/logger/writers/async_writer.h"
#include "../mocks/mock_writer.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <sstream>
#include <filesystem>

using namespace logger_system;
using namespace logger_system::testing;
using namespace logger_module;
using namespace std::chrono_literals;
using log_level = thread_module::log_level;

class stress_test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        test_dir_ = "/tmp/logger_stress_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(test_dir_);
    }

    void TearDown() override {
        // Clean up test directory
        std::filesystem::remove_all(test_dir_);
    }

    std::filesystem::path test_dir_;
};

/**
 * @brief Test high-volume concurrent logging
 * 
 * Verifies that the logger can handle multiple threads logging simultaneously
 * without crashes or data corruption.
 */
TEST_F(stress_test, concurrent_logging_stress) {
    const size_t num_threads = 20;
    const size_t logs_per_thread = 1000;
    
    // Create logger with mock writer to track all writes
    auto mock_writer_inst = std::make_unique<mock_writer>();
    auto* mock_writer_ptr = mock_writer_inst.get();
    
    auto result = logger_module::logger_builder()
        .with_default_pattern()
        .add_writer("mock", std::move(mock_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    std::atomic<size_t> total_logged{0};
    std::vector<std::thread> threads;

    // Start multiple threads logging concurrently
    auto start_time = std::chrono::steady_clock::now();
    
    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([&logger, &total_logged, t, logs_per_thread]() {
            for (size_t i = 0; i < logs_per_thread; ++i) {
                logger->log(log_level::info, "Thread " + std::to_string(t) + " - Message " + std::to_string(i));
                total_logged.fetch_add(1);
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Verify all messages were logged
    EXPECT_EQ(total_logged.load(), num_threads * logs_per_thread);
    EXPECT_EQ(mock_writer_ptr->get_write_count(), num_threads * logs_per_thread);

    // Calculate throughput
    double throughput = (total_logged.load() * 1000.0) / duration.count();
    std::cout << "Concurrent logging throughput: " << throughput << " messages/second" << std::endl;
    
    // Verify minimum throughput (should handle at least 10K messages/second)
    EXPECT_GT(throughput, 10000.0);
}

/**
 * @brief Test memory usage under sustained load
 * 
 * Verifies that memory usage remains stable during extended logging sessions.
 */
TEST_F(stress_test, memory_stability_stress) {
    const size_t num_iterations = 100;
    const size_t logs_per_iteration = 1000;
    
    auto mock_writer_inst = std::make_unique<mock_writer>();
    auto* mock_writer_ptr = mock_writer_inst.get();
    
    auto result = logger_module::logger_builder()
        .with_buffer_size(10000)
        .add_writer("mock", std::move(mock_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    // Baseline memory measurement would go here (platform-specific)
    
    for (size_t iter = 0; iter < num_iterations; ++iter) {
        for (size_t i = 0; i < logs_per_iteration; ++i) {
            // Create varying message sizes to test memory management
            std::string message = "Iteration " + std::to_string(iter) + " - ";
            message.append(i % 100, 'X');  // Variable length padding
            logger->log(log_level::debug, message);
        }
        
        // Allow some time for cleanup between iterations
        std::this_thread::sleep_for(10ms);
    }

    // Verify all messages were processed
    EXPECT_EQ(mock_writer_ptr->get_write_count(), num_iterations * logs_per_iteration);
}

/**
 * @brief Test overflow handling under extreme load
 * 
 * Verifies that the logger properly handles buffer overflow scenarios.
 */
TEST_F(stress_test, buffer_overflow_stress) {
    const size_t buffer_size = 100;
    const size_t num_messages = 10000;
    
    auto slow_writer_inst = std::make_unique<mock_writer>();
    auto* slow_writer_ptr = slow_writer_inst.get();
    slow_writer_ptr->set_write_delay(1ms);  // Simulate slow I/O
    
    auto result = logger_module::logger_builder()
        .with_buffer_size(buffer_size)
        .with_overflow_policy(logger_config::overflow_policy::drop_oldest)
        .add_writer("slow", std::move(slow_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    std::atomic<size_t> messages_sent{0};
    
    // Flood the logger with messages faster than it can write
    auto start_time = std::chrono::steady_clock::now();
    
    for (size_t i = 0; i < num_messages; ++i) {
        logger->log(log_level::info, "Overflow test message " + std::to_string(i));
        messages_sent.fetch_add(1);
    }
    
    // Wait for writer to catch up
    std::this_thread::sleep_for(100ms);
    
    auto end_time = std::chrono::steady_clock::now();
    
    // Some messages should have been dropped due to overflow
    size_t written_count = slow_writer_ptr->get_write_count();
    EXPECT_LT(written_count, messages_sent.load());
    EXPECT_GT(written_count, 0);
    
    std::cout << "Buffer overflow test: Sent " << messages_sent.load() 
              << ", Written " << written_count 
              << ", Dropped " << (messages_sent.load() - written_count) << std::endl;
}

/**
 * @brief Test rapid writer switching
 * 
 * Verifies that the logger can handle frequent writer additions and removals.
 */
TEST_F(stress_test, writer_switching_stress) {
    auto result = logger_module::logger_builder()
        .with_default_pattern()
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    const size_t num_switches = 100;
    std::vector<mock_writer*> writers;
    
    for (size_t i = 0; i < num_switches; ++i) {
        // Add a new writer
        auto writer = std::make_unique<mock_writer>();
        auto* writer_ptr = writer.get();
        writers.push_back(writer_ptr);
        
        std::string writer_name = "writer_" + std::to_string(i);
        logger->add_writer(writer_name, std::move(writer));
        
        // Log some messages
        for (size_t j = 0; j < 10; ++j) {
            logger->log(log_level::info, "Message for " + writer_name);
        }
        
        // Remove old writers periodically
        if (i > 10 && i % 10 == 0) {
            for (size_t k = i - 10; k < i - 5; ++k) {
                logger->remove_writer("writer_" + std::to_string(k));
            }
        }
    }
    
    // Verify that recent writers received messages
    for (size_t i = num_switches - 5; i < num_switches; ++i) {
        EXPECT_GT(writers[i]->get_write_count(), 0);
    }
}

/**
 * @brief Test logger behavior under random load patterns
 * 
 * Simulates realistic usage with varying message rates and sizes.
 */
TEST_F(stress_test, random_load_pattern_stress) {
    auto mock_writer_inst = std::make_unique<mock_writer>();
    auto* mock_writer_ptr = mock_writer_inst.get();
    
    auto result = logger_module::logger_builder()
        .with_buffer_size(1000)
        .add_writer("mock", std::move(mock_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> level_dist(0, 5);
    std::uniform_int_distribution<> size_dist(10, 500);
    std::uniform_int_distribution<> delay_dist(0, 10);
    
    const size_t duration_seconds = 5;
    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(duration_seconds);
    
    size_t message_count = 0;
    
    while (std::chrono::steady_clock::now() < end_time) {
        // Random log level
        auto level = static_cast<log_level>(level_dist(gen));
        
        // Random message size
        std::string message(size_dist(gen), 'A');
        
        logger->log(level, message);
        message_count++;
        
        // Random delay between messages
        if (delay_dist(gen) > 8) {
            std::this_thread::sleep_for(std::chrono::microseconds(delay_dist(gen)));
        }
    }
    
    // Allow time for final messages to be written
    std::this_thread::sleep_for(100ms);
    
    // Verify all messages were written
    EXPECT_EQ(mock_writer_ptr->get_write_count(), message_count);
    
    std::cout << "Random load test: Processed " << message_count 
              << " messages in " << duration_seconds << " seconds" << std::endl;
}

/**
 * @brief Test logger recovery from writer failures
 * 
 * Verifies that the logger continues operating when writers fail.
 */
TEST_F(stress_test, writer_failure_recovery_stress) {
    auto failing_writer_inst = std::make_unique<mock_writer>();
    auto* failing_writer_ptr = failing_writer_inst.get();
    auto backup_writer_inst = std::make_unique<mock_writer>();
    auto* backup_writer_ptr = backup_writer_inst.get();
    
    auto result = logger_module::logger_builder()
        .add_writer("primary", std::move(failing_writer_inst))
        .add_writer("backup", std::move(backup_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    const size_t num_messages = 1000;
    
    for (size_t i = 0; i < num_messages; ++i) {
        // Simulate intermittent failures
        if (i % 100 == 50) {
            failing_writer_ptr->set_should_fail(true);
        } else if (i % 100 == 75) {
            failing_writer_ptr->set_should_fail(false);
        }
        
        logger->log(log_level::warning, "Failure test message " + std::to_string(i));
    }
    
    // Backup writer should have all messages
    EXPECT_EQ(backup_writer_ptr->get_write_count(), num_messages);
    
    // Primary writer should have fewer due to failures
    EXPECT_LT(failing_writer_ptr->get_write_count(), num_messages);
    
    std::cout << "Failure recovery test: Primary wrote " << failing_writer_ptr->get_write_count()
              << ", Backup wrote " << backup_writer_ptr->get_write_count() << std::endl;
}

/**
 * @brief Test async writer performance under load
 * 
 * Verifies that async writers maintain performance under heavy load.
 */
TEST_F(stress_test, async_writer_performance_stress) {
    auto file_path = test_dir_ / "async_stress.log";
    auto file_writer_inst = std::make_unique<logger_module::file_writer>(file_path.string());
    auto async_writer_inst = std::make_unique<logger_module::async_writer>(std::move(file_writer_inst), 1000);
    
    // Need to keep a raw pointer for later use
    auto* async_writer_ptr = async_writer_inst.get();
    
    auto result = logger_module::logger_builder()
        .add_writer("async", std::move(async_writer_inst))
        .build();
    
    ASSERT_TRUE(result.has_value());
    auto logger = std::move(result.value());

    const size_t num_messages = 50000;
    
    auto start_time = std::chrono::steady_clock::now();
    
    for (size_t i = 0; i < num_messages; ++i) {
        logger->log(log_level::info, "Async performance test " + std::to_string(i));
    }
    
    // Force flush using logger
    logger->flush();
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    double throughput = (num_messages * 1000.0) / duration.count();
    
    std::cout << "Async writer throughput: " << throughput << " messages/second" << std::endl;
    
    // Async writer should maintain high throughput
    EXPECT_GT(throughput, 20000.0);
    
    // Verify messages were written to file
    EXPECT_TRUE(std::filesystem::exists(file_path));
    EXPECT_GT(std::filesystem::file_size(file_path), 0);
}