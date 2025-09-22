/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <gtest/gtest.h>
#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>

using namespace logger_module;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create logger instances for testing
        sync_logger_ = std::make_unique<logger>(false); // synchronous
        async_logger_ = std::make_unique<logger>(true);  // asynchronous
    }

    void TearDown() override {
        if (sync_logger_) {
            sync_logger_->stop();
        }
        if (async_logger_) {
            async_logger_->stop();
        }
    }

    std::unique_ptr<logger> sync_logger_;
    std::unique_ptr<logger> async_logger_;
};

// Test basic logger construction
TEST_F(LoggerTest, ConstructorTest) {
    EXPECT_NE(sync_logger_, nullptr);
    EXPECT_NE(async_logger_, nullptr);
}

// Test log level filtering
TEST_F(LoggerTest, LogLevelFiltering) {
    sync_logger_->set_min_level(thread_module::log_level::warning);
    
    EXPECT_TRUE(sync_logger_->is_enabled(thread_module::log_level::critical));
    EXPECT_TRUE(sync_logger_->is_enabled(thread_module::log_level::error));
    EXPECT_TRUE(sync_logger_->is_enabled(thread_module::log_level::warning));
    EXPECT_FALSE(sync_logger_->is_enabled(thread_module::log_level::info));
    EXPECT_FALSE(sync_logger_->is_enabled(thread_module::log_level::debug));
    EXPECT_FALSE(sync_logger_->is_enabled(thread_module::log_level::trace));
}

// Test writer management
TEST_F(LoggerTest, WriterManagement) {
    auto writer = std::make_unique<console_writer>();
    sync_logger_->add_writer(std::move(writer));
    
    // Test logging after adding writer
    sync_logger_->log(thread_module::log_level::info, "Test message");
    
    // Clear writers
    sync_logger_->clear_writers();
    
    // Should still work without writers (no crash)
    sync_logger_->log(thread_module::log_level::info, "Test message after clear");
}

// Test synchronous logging
TEST_F(LoggerTest, SynchronousLogging) {
    auto writer = std::make_unique<console_writer>();
    sync_logger_->add_writer(std::move(writer));
    
    // Test all log levels
    sync_logger_->log(thread_module::log_level::trace, "Trace message");
    sync_logger_->log(thread_module::log_level::debug, "Debug message");
    sync_logger_->log(thread_module::log_level::info, "Info message");
    sync_logger_->log(thread_module::log_level::warning, "Warning message");
    sync_logger_->log(thread_module::log_level::error, "Error message");
    sync_logger_->log(thread_module::log_level::critical, "Critical message");
    
    // Test with source location
    sync_logger_->log(thread_module::log_level::info, "Message with location", 
                     __FILE__, __LINE__, __func__);
}

// Test asynchronous logging
TEST_F(LoggerTest, AsynchronousLogging) {
    auto writer = std::make_unique<console_writer>();
    async_logger_->add_writer(std::move(writer));
    
    async_logger_->start();
    
    // Test rapid logging
    for (int i = 0; i < 100; ++i) {
        async_logger_->log(thread_module::log_level::info, 
                          "Async message " + std::to_string(i));
    }
    
    // Give time for async processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    async_logger_->flush();
    async_logger_->stop();
}

// Test multithreaded logging
TEST_F(LoggerTest, MultithreadedLogging) {
    auto writer = std::make_unique<console_writer>();
    async_logger_->add_writer(std::move(writer));
    async_logger_->start();
    
    std::vector<std::thread> threads;
    const int num_threads = 4;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t]() {
            for (int i = 0; i < 25; ++i) {
                async_logger_->log(thread_module::log_level::info,
                                  "Thread " + std::to_string(t) + 
                                  " Message " + std::to_string(i));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    async_logger_->flush();
    async_logger_->stop();
}

// Test logger state management
TEST_F(LoggerTest, StateManagement) {
    EXPECT_FALSE(async_logger_->is_running());
    
    async_logger_->start();
    EXPECT_TRUE(async_logger_->is_running());
    
    async_logger_->stop();
    EXPECT_FALSE(async_logger_->is_running());
}

// Test min level getter/setter
TEST_F(LoggerTest, MinLevelGetterSetter) {
    // Default should be trace (lowest level)
    EXPECT_EQ(sync_logger_->get_min_level(), thread_module::log_level::trace);
    
    sync_logger_->set_min_level(thread_module::log_level::warning);
    EXPECT_EQ(sync_logger_->get_min_level(), thread_module::log_level::warning);
    
    sync_logger_->set_min_level(thread_module::log_level::error);
    EXPECT_EQ(sync_logger_->get_min_level(), thread_module::log_level::error);
}

// Test flush functionality
TEST_F(LoggerTest, FlushFunctionality) {
    auto writer = std::make_unique<console_writer>();
    async_logger_->add_writer(std::move(writer));
    async_logger_->start();
    
    // Log some messages
    for (int i = 0; i < 10; ++i) {
        async_logger_->log(thread_module::log_level::info, 
                          "Flush test message " + std::to_string(i));
    }
    
    // Flush should complete without issues
    EXPECT_NO_THROW(async_logger_->flush());
    
    async_logger_->stop();
}

// Test error handling
TEST_F(LoggerTest, ErrorHandling) {
    // Test logging without writers (should not crash)
    EXPECT_NO_THROW(sync_logger_->log(thread_module::log_level::info, "No writer test"));
    
    // Test empty messages
    EXPECT_NO_THROW(sync_logger_->log(thread_module::log_level::info, ""));
    
    // Test very long messages
    std::string long_message(10000, 'A');
    EXPECT_NO_THROW(sync_logger_->log(thread_module::log_level::info, long_message));
}

// Test buffer size configuration for async logger
TEST_F(LoggerTest, BufferSizeConfiguration) {
    // Test with small buffer
    auto small_buffer_logger = std::make_unique<logger>(true, 128);
    EXPECT_NE(small_buffer_logger, nullptr);
    
    // Test with large buffer
    auto large_buffer_logger = std::make_unique<logger>(true, 65536);
    EXPECT_NE(large_buffer_logger, nullptr);
}