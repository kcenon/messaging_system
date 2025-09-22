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
   this software without specific property written permission.

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
#include <logger/writers/console_writer.h>
#include <logger/writers/base_writer.h>
#include <memory>
#include <chrono>
#include <thread>

using namespace logger_module;

class ConsoleWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        writer_ = std::make_unique<console_writer>();
        timestamp_ = std::chrono::system_clock::now();
    }

    std::unique_ptr<console_writer> writer_;
    std::chrono::system_clock::time_point timestamp_;
};

// Test basic console writer construction
TEST_F(ConsoleWriterTest, ConstructorTest) {
    EXPECT_NE(writer_, nullptr);
    
    // Test with custom parameters
    auto stderr_writer = std::make_unique<console_writer>(true, false);  // use stderr, no auto color
    EXPECT_NE(stderr_writer, nullptr);
    
    auto color_writer = std::make_unique<console_writer>(false, true);   // use stdout, auto color
    EXPECT_NE(color_writer, nullptr);
}

// Test basic write functionality
TEST_F(ConsoleWriterTest, BasicWrite) {
    EXPECT_NO_THROW(writer_->write(
        thread_module::log_level::info,
        "Test message",
        "",
        0,
        "",
        timestamp_
    ));
    
    EXPECT_NO_THROW(writer_->flush());
}

// Test write with source location
TEST_F(ConsoleWriterTest, WriteWithSourceLocation) {
    EXPECT_NO_THROW(writer_->write(
        thread_module::log_level::error,
        "Error message with location",
        __FILE__,
        __LINE__,
        __func__,
        timestamp_
    ));
    
    EXPECT_NO_THROW(writer_->flush());
}

// Test all log levels
TEST_F(ConsoleWriterTest, AllLogLevels) {
    EXPECT_NO_THROW(writer_->write(thread_module::log_level::trace, "Trace", "", 0, "", timestamp_));
    EXPECT_NO_THROW(writer_->write(thread_module::log_level::debug, "Debug", "", 0, "", timestamp_));
    EXPECT_NO_THROW(writer_->write(thread_module::log_level::info, "Info", "", 0, "", timestamp_));
    EXPECT_NO_THROW(writer_->write(thread_module::log_level::warning, "Warning", "", 0, "", timestamp_));
    EXPECT_NO_THROW(writer_->write(thread_module::log_level::error, "Error", "", 0, "", timestamp_));
    EXPECT_NO_THROW(writer_->write(thread_module::log_level::critical, "Critical", "", 0, "", timestamp_));
    
    writer_->flush();
}

// Test color functionality
TEST_F(ConsoleWriterTest, ColorFunctionality) {
    // Test enabling color
    writer_->set_use_color(true);
    EXPECT_TRUE(writer_->use_color());
    
    EXPECT_NO_THROW(writer_->write(
        thread_module::log_level::error,
        "Colored error message",
        "",
        0,
        "",
        timestamp_
    ));
    
    // Test disabling color
    writer_->set_use_color(false);
    EXPECT_FALSE(writer_->use_color());
    
    EXPECT_NO_THROW(writer_->write(
        thread_module::log_level::warning,
        "Non-colored warning message",
        "",
        0,
        "",
        timestamp_
    ));
    
    writer_->flush();
}

// Test stderr usage
TEST_F(ConsoleWriterTest, StderrUsage) {
    auto stderr_writer = std::make_unique<console_writer>(true);
    
    EXPECT_NO_THROW(stderr_writer->write(
        thread_module::log_level::critical,
        "Critical message to stderr",
        "",
        0,
        "",
        timestamp_
    ));
    
    stderr_writer->flush();
}

// Test empty and special messages
TEST_F(ConsoleWriterTest, SpecialMessages) {
    // Empty message
    EXPECT_NO_THROW(writer_->write(
        thread_module::log_level::info,
        "",
        "",
        0,
        "",
        timestamp_
    ));
    
    // Very long message
    std::string long_message(1000, 'X');
    EXPECT_NO_THROW(writer_->write(
        thread_module::log_level::info,
        long_message,
        "",
        0,
        "",
        timestamp_
    ));
    
    // Message with special characters
    EXPECT_NO_THROW(writer_->write(
        thread_module::log_level::info,
        "Message with special chars: \\n\\t\\r\\0",
        "",
        0,
        "",
        timestamp_
    ));
    
    writer_->flush();
}

// Test multithreaded access
TEST_F(ConsoleWriterTest, MultithreadedAccess) {
    std::vector<std::thread> threads;
    const int num_threads = 4;
    const int messages_per_thread = 10;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                writer_->write(
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
    
    writer_->flush();
}

// Test flush functionality
TEST_F(ConsoleWriterTest, FlushFunctionality) {
    // Write several messages
    for (int i = 0; i < 5; ++i) {
        writer_->write(
            thread_module::log_level::info,
            "Message " + std::to_string(i),
            "",
            0,
            "",
            timestamp_
        );
    }
    
    // Flush should complete without issues
    EXPECT_NO_THROW(writer_->flush());
    
    // Multiple flushes should be safe
    EXPECT_NO_THROW(writer_->flush());
    EXPECT_NO_THROW(writer_->flush());
}

// Mock writer for testing base_writer functionality
class MockWriter : public base_writer {
public:
    bool write(thread_module::log_level level,
              const std::string& message,
              const std::string& file,
              int line,
              const std::string& function,
              const std::chrono::system_clock::time_point& timestamp) override {
        last_formatted_ = format_log_entry(level, message, file, line, function, timestamp);
        last_level_ = level;
        write_count_++;
        return true;
    }
    
    void flush() override {
        flush_count_++;
    }
    
    std::string get_name() const override {
        return "mock";
    }
    
    std::string last_formatted_;
    thread_module::log_level last_level_ = thread_module::log_level::trace;
    int write_count_ = 0;
    int flush_count_ = 0;
};

class BaseWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_writer_ = std::make_unique<MockWriter>();
        timestamp_ = std::chrono::system_clock::now();
    }

    std::unique_ptr<MockWriter> mock_writer_;
    std::chrono::system_clock::time_point timestamp_;
};

// Test base writer formatting
TEST_F(BaseWriterTest, MessageFormatting) {
    mock_writer_->write(
        thread_module::log_level::warning,
        "Test warning message",
        "/path/to/test.cpp",
        42,
        "test_function",
        timestamp_
    );
    
    EXPECT_EQ(mock_writer_->write_count_, 1);
    EXPECT_EQ(mock_writer_->last_level_, thread_module::log_level::warning);
    EXPECT_FALSE(mock_writer_->last_formatted_.empty());
    EXPECT_NE(mock_writer_->last_formatted_.find("WARNING"), std::string::npos);
    EXPECT_NE(mock_writer_->last_formatted_.find("Test warning message"), std::string::npos);
    EXPECT_NE(mock_writer_->last_formatted_.find("test.cpp:42"), std::string::npos);
}

// Test color functionality in base writer
TEST_F(BaseWriterTest, ColorFunctionality) {
    // Test color enabled
    mock_writer_->set_use_color(true);
    EXPECT_TRUE(mock_writer_->use_color());
    
    // Test color disabled
    mock_writer_->set_use_color(false);
    EXPECT_FALSE(mock_writer_->use_color());
}

// Test flush count
TEST_F(BaseWriterTest, FlushCount) {
    EXPECT_EQ(mock_writer_->flush_count_, 0);
    
    mock_writer_->flush();
    EXPECT_EQ(mock_writer_->flush_count_, 1);
    
    mock_writer_->flush();
    mock_writer_->flush();
    EXPECT_EQ(mock_writer_->flush_count_, 3);
}