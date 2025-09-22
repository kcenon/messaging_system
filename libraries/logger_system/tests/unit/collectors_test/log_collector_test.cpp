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
#include <logger/core/log_collector.h>
#include <logger/writers/base_writer.h>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>

using namespace logger_module;

// Mock writer for testing log collector
class MockCollectorWriter : public base_writer {
public:
    result_void write(thread_module::log_level level,
              const std::string& message,
              const std::string& /* file */,
              int /* line */,
              const std::string& /* function */,
              const std::chrono::system_clock::time_point& /* timestamp */) override {
        write_count_++;
        last_message_ = message;
        last_level_ = level;
        messages_.push_back(message);
        return result_void{};
    }
    
    result_void flush() override {
        flush_count_++;
        return result_void{};
    }
    
    std::string get_name() const override {
        return "mock_collector";
    }
    
    std::atomic<int> write_count_{0};
    std::atomic<int> flush_count_{0};
    std::string last_message_;
    thread_module::log_level last_level_ = thread_module::log_level::trace;
    std::vector<std::string> messages_;
    mutable std::mutex messages_mutex_;
};

class LogCollectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        collector_ = std::make_unique<log_collector>(1024); // Small buffer for testing
        mock_writer_ = std::make_shared<MockCollectorWriter>();
        timestamp_ = std::chrono::system_clock::now();
    }

    void TearDown() override {
        if (collector_) {
            collector_->stop();
        }
    }

    std::unique_ptr<log_collector> collector_;
    std::shared_ptr<MockCollectorWriter> mock_writer_;
    std::chrono::system_clock::time_point timestamp_;
};

// Test basic log collector construction
TEST_F(LogCollectorTest, ConstructorTest) {
    EXPECT_NE(collector_, nullptr);
    
    // Test with different buffer sizes
    auto small_collector = std::make_unique<log_collector>(128);
    EXPECT_NE(small_collector, nullptr);
    
    auto large_collector = std::make_unique<log_collector>(65536);
    EXPECT_NE(large_collector, nullptr);
}

// Test writer management
TEST_F(LogCollectorTest, WriterManagement) {
    collector_->add_writer(mock_writer_.get());
    
    // Clear writers
    collector_->clear_writers();
    
    // Should not crash
    EXPECT_NO_THROW(collector_->clear_writers());
}

// Test basic enqueue and processing
TEST_F(LogCollectorTest, BasicEnqueueAndProcessing) {
    collector_->add_writer(mock_writer_.get());
    collector_->start();
    
    // Enqueue a message
    collector_->enqueue(
        thread_module::log_level::info,
        "Test message",
        "",
        0,
        "",
        timestamp_
    );
    
    // Give time for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    collector_->flush();
    
    // Check that message was processed
    EXPECT_GT(mock_writer_->write_count_.load(), 0);
    EXPECT_EQ(mock_writer_->last_message_, "Test message");
    
    collector_->stop();
}

// Test multiple messages
TEST_F(LogCollectorTest, MultipleMessages) {
    collector_->add_writer(mock_writer_.get());
    collector_->start();
    
    const int num_messages = 10;
    
    for (int i = 0; i < num_messages; ++i) {
        collector_->enqueue(
            thread_module::log_level::info,
            "Message " + std::to_string(i),
            "",
            0,
            "",
            timestamp_
        );
    }
    
    // Give time for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    collector_->flush();
    
    // All messages should be processed
    EXPECT_EQ(mock_writer_->write_count_.load(), num_messages);
    
    collector_->stop();
}

// Test multithreaded enqueuing
TEST_F(LogCollectorTest, MultithreadedEnqueuing) {
    collector_->add_writer(mock_writer_.get());
    collector_->start();
    
    std::vector<std::thread> threads;
    const int num_threads = 4;
    const int messages_per_thread = 25;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                collector_->enqueue(
                    thread_module::log_level::info,
                    "Thread " + std::to_string(t) + " Message " + std::to_string(i),
                    "",
                    0,
                    "",
                    timestamp_
                );
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Give time for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    collector_->flush();
    
    // All messages should be processed
    EXPECT_EQ(mock_writer_->write_count_.load(), num_threads * messages_per_thread);
    
    collector_->stop();
}

// Test different log levels
TEST_F(LogCollectorTest, DifferentLogLevels) {
    collector_->add_writer(mock_writer_.get());
    collector_->start();
    
    // Test all log levels
    collector_->enqueue(thread_module::log_level::trace, "Trace", "", 0, "", timestamp_);
    collector_->enqueue(thread_module::log_level::debug, "Debug", "", 0, "", timestamp_);
    collector_->enqueue(thread_module::log_level::info, "Info", "", 0, "", timestamp_);
    collector_->enqueue(thread_module::log_level::warning, "Warning", "", 0, "", timestamp_);
    collector_->enqueue(thread_module::log_level::error, "Error", "", 0, "", timestamp_);
    collector_->enqueue(thread_module::log_level::critical, "Critical", "", 0, "", timestamp_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    collector_->flush();
    collector_->stop();
    
    EXPECT_EQ(mock_writer_->write_count_.load(), 6);
}

// Test with source location
TEST_F(LogCollectorTest, WithSourceLocation) {
    collector_->add_writer(mock_writer_.get());
    collector_->start();
    
    collector_->enqueue(
        thread_module::log_level::error,
        "Error with location",
        __FILE__,
        __LINE__,
        __func__,
        timestamp_
    );
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    collector_->flush();
    collector_->stop();
    
    EXPECT_EQ(mock_writer_->write_count_.load(), 1);
    EXPECT_EQ(mock_writer_->last_message_, "Error with location");
    EXPECT_EQ(mock_writer_->last_level_, thread_module::log_level::error);
}

// Test flush functionality
TEST_F(LogCollectorTest, FlushFunctionality) {
    collector_->add_writer(mock_writer_.get());
    collector_->start();
    
    // Enqueue several messages
    for (int i = 0; i < 5; ++i) {
        collector_->enqueue(
            thread_module::log_level::info,
            "Flush test " + std::to_string(i),
            "",
            0,
            "",
            timestamp_
        );
    }
    
    collector_->flush();
    
    // Writer should have been flushed
    EXPECT_GT(mock_writer_->flush_count_.load(), 0);
    
    collector_->stop();
}

// Test stop and start functionality
TEST_F(LogCollectorTest, StopStartFunctionality) {
    collector_->add_writer(mock_writer_.get());
    
    // Start collector
    collector_->start();
    
    collector_->enqueue(
        thread_module::log_level::info,
        "Before stop",
        "",
        0,
        "",
        timestamp_
    );
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Stop collector
    collector_->stop();
    
    int count_after_stop = mock_writer_->write_count_.load();
    
    // Enqueue after stop (should still work but may not be processed immediately)
    collector_->enqueue(
        thread_module::log_level::info,
        "After stop",
        "",
        0,
        "",
        timestamp_
    );
    
    // Start again
    collector_->start();
    
    collector_->enqueue(
        thread_module::log_level::info,
        "After restart",
        "",
        0,
        "",
        timestamp_
    );
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    collector_->flush();
    collector_->stop();
    
    EXPECT_GT(mock_writer_->write_count_.load(), count_after_stop);
}

// Test edge cases
TEST_F(LogCollectorTest, EdgeCases) {
    collector_->add_writer(mock_writer_.get());
    collector_->start();
    
    // Empty message
    collector_->enqueue(
        thread_module::log_level::info,
        "",
        "",
        0,
        "",
        timestamp_
    );
    
    // Very long message
    std::string long_message(5000, 'L');
    collector_->enqueue(
        thread_module::log_level::info,
        long_message,
        "",
        0,
        "",
        timestamp_
    );
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    collector_->flush();
    collector_->stop();
    
    EXPECT_EQ(mock_writer_->write_count_.load(), 2);
}