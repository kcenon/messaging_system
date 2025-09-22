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
#include "error_handling.h"
#include "job_queue.h"
#include "callback_job.h"
#include "thread_base.h"
#include <thread>
#include <chrono>

namespace kcenon::thread {
namespace test {

class ErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test environment
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(ErrorHandlingTest, ErrorCodeToString) {
    // Test all error codes have string representations
    EXPECT_FALSE(error_code_to_string(error_code::success).empty());
    EXPECT_FALSE(error_code_to_string(error_code::unknown_error).empty());
    EXPECT_FALSE(error_code_to_string(error_code::thread_already_running).empty());
    EXPECT_FALSE(error_code_to_string(error_code::queue_full).empty());
    EXPECT_FALSE(error_code_to_string(error_code::job_creation_failed).empty());
    EXPECT_FALSE(error_code_to_string(error_code::resource_allocation_failed).empty());
    EXPECT_FALSE(error_code_to_string(error_code::mutex_error).empty());
    EXPECT_FALSE(error_code_to_string(error_code::io_error).empty());
    
    // Test unknown error code
    auto unknown_code = static_cast<error_code>(9999);
    EXPECT_FALSE(error_code_to_string(unknown_code).empty());
}

TEST_F(ErrorHandlingTest, ResultVoidSuccess) {
    result_void success_result;
    
    EXPECT_FALSE(success_result.has_error());
    EXPECT_TRUE(static_cast<bool>(success_result));
}

TEST_F(ErrorHandlingTest, ResultVoidError) {
    error test_error{error_code::unknown_error, "Test error message"};
    result_void error_result{test_error};
    
    EXPECT_TRUE(error_result.has_error());
    EXPECT_FALSE(static_cast<bool>(error_result));
    EXPECT_EQ(error_result.get_error().code(), error_code::unknown_error);
    EXPECT_EQ(error_result.get_error().message(), "Test error message");
}

TEST_F(ErrorHandlingTest, ResultWithValue) {
    // Success case
    result<int> success_result{42};
    
    EXPECT_TRUE(success_result.has_value());
    EXPECT_TRUE(static_cast<bool>(success_result));
    EXPECT_EQ(success_result.value(), 42);
    
    // Error case
    result<int> error_result{error{error_code::invalid_argument, "Invalid value"}};
    
    EXPECT_FALSE(error_result.has_value());
    EXPECT_FALSE(static_cast<bool>(error_result));
    EXPECT_EQ(error_result.get_error().code(), error_code::invalid_argument);
}

TEST_F(ErrorHandlingTest, ResultValueOr) {
    result<int> success_result{42};
    EXPECT_EQ(success_result.value_or(0), 42);
    
    result<int> error_result{error{error_code::unknown_error, "Error"}};
    EXPECT_EQ(error_result.value_or(99), 99);
}

TEST_F(ErrorHandlingTest, ResultAndThenBasic) {
    result<int> success_result{42};
    
    auto transformed = success_result.and_then([](int value) -> result<int> {
        return value * 2;
    });
    
    EXPECT_TRUE(transformed.has_value());
    EXPECT_EQ(transformed.value(), 84);
    
    result<int> error_result{error{error_code::unknown_error, "Error"}};
    
    auto error_transformed = error_result.and_then([](int value) -> result<int> {
        return value * 2;
    });
    
    EXPECT_FALSE(error_transformed.has_value());
    EXPECT_EQ(error_transformed.get_error().code(), error_code::unknown_error);
}

TEST_F(ErrorHandlingTest, ResultAndThen) {
    result<int> success_result{42};
    
    auto chained = success_result.and_then([](int value) -> result<std::string> {
        if (value > 0) {
            return std::to_string(value);
        }
        return error{error_code::invalid_argument, "Negative value"};
    });
    
    EXPECT_TRUE(chained.has_value());
    EXPECT_EQ(chained.value(), "42");
}

TEST_F(ErrorHandlingTest, ResultValueOrThrow) {
    result<int> success_result{42};
    
    EXPECT_NO_THROW({
        int value = success_result.value_or_throw();
        EXPECT_EQ(value, 42);
    });
    
    result<int> error_result{error{error_code::unknown_error, "Test error"}};
    
    EXPECT_THROW({
        [[maybe_unused]] auto val = error_result.value_or_throw();
    }, std::runtime_error);
}

TEST_F(ErrorHandlingTest, JobQueueErrorStates) {
    // Test queue operations
    auto queue = std::make_unique<job_queue>();
    
    // Enqueue a job
    auto job = std::make_unique<callback_job>([]() -> result_void { return {}; });
    auto result = queue->enqueue(std::move(job));
    EXPECT_FALSE(result.has_error());
    
    // Test dequeue
    auto dequeue_result = queue->dequeue();
    EXPECT_TRUE(dequeue_result.has_value());
    EXPECT_NE(dequeue_result.value(), nullptr);
}

TEST_F(ErrorHandlingTest, JobExecutionErrors) {
    auto queue = std::make_unique<job_queue>();
    
    // Job that returns an error
    auto error_job = std::make_unique<callback_job>([]() -> result_void {
        return error{error_code::job_execution_failed, "Simulated failure"};
    });
    
    auto enqueue_result = queue->enqueue(std::move(error_job));
    EXPECT_FALSE(enqueue_result.has_error());
    
    // Dequeue and execute
    auto dequeue_result = queue->dequeue();
    ASSERT_TRUE(dequeue_result.has_value());
    auto dequeued = std::move(dequeue_result.value());
    ASSERT_NE(dequeued, nullptr);
    
    auto result = dequeued->do_work();
    EXPECT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error().code(), error_code::job_execution_failed);
}

TEST_F(ErrorHandlingTest, ThreadBaseStartStop) {
    // Create a custom thread that counts work cycles
    class test_thread : public thread_base {
    public:
        test_thread() : thread_base("test_thread") {}
        std::atomic<int> work_count{0};
        std::atomic<bool> error_occurred{false};
        
    protected:
        result_void do_work() override {
            work_count.fetch_add(1);
            if (work_count.load() >= 3) {
                error_occurred.store(true);
                return error{error_code::unknown_error, "Test error"};
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return {};
        }
    };
    
    auto worker = std::make_unique<test_thread>();
    worker->set_wake_interval(std::chrono::milliseconds(10));
    
    // Start the thread
    worker->start();
    
    // Let it run for a bit - ensure enough time for at least 3 iterations
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Stop the thread
    worker->stop();
    
    // Verify it executed multiple times
    EXPECT_GT(worker->work_count.load(), 0);
    EXPECT_GE(worker->work_count.load(), 3);  // Should have run at least 3 times
    EXPECT_TRUE(worker->error_occurred.load());
}

TEST_F(ErrorHandlingTest, ConcurrentErrorHandling) {
    const int thread_count = 4;
    const int errors_per_thread = 10;
    
    std::atomic<int> total_errors{0};
    std::atomic<int> total_successes{0};
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([t, &total_errors, &total_successes]() {
            for (int i = 0; i < errors_per_thread * 2; ++i) {
                // Alternate between success and error
                result<int> res = (i % 2 == 0) 
                    ? result<int>{i}
                    : result<int>{error{error_code::unknown_error, "Error"}};
                
                if (!res.has_value()) {
                    total_errors.fetch_add(1);
                } else {
                    total_successes.fetch_add(1);
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(total_errors.load(), thread_count * errors_per_thread);
    EXPECT_EQ(total_successes.load(), thread_count * errors_per_thread);
}

TEST_F(ErrorHandlingTest, ErrorChaining) {
    // Test chaining multiple operations that might fail
    auto chain_result = result<int>{42}
        .and_then([](int value) -> result<std::string> {
            if (value > 0) {
                return std::to_string(value);
            }
            return error{error_code::invalid_argument, "Negative value"};
        })
        .and_then([](const std::string& str) -> result<size_t> {
            return str.length();
        });
    
    EXPECT_TRUE(chain_result.has_value());
    EXPECT_EQ(chain_result.value(), 2u); // "42" has length 2
    
    // Test with error in the middle
    auto error_chain = result<int>{-1}
        .and_then([](int value) -> result<std::string> {
            if (value > 0) {
                return std::to_string(value);
            }
            return error{error_code::invalid_argument, "Negative value"};
        })
        .and_then([](const std::string& str) -> result<size_t> {
            return str.length();
        });
    
    EXPECT_FALSE(error_chain.has_value());
    EXPECT_EQ(error_chain.get_error().code(), error_code::invalid_argument);
}

TEST_F(ErrorHandlingTest, ResourceAllocationErrors) {
    // Simulate resource allocation failure scenarios
    std::vector<result<std::unique_ptr<int>>> allocations;
    
    for (int i = 0; i < 10; ++i) {
        if (i == 5) {
            // Simulate allocation failure
            allocations.push_back(
                result<std::unique_ptr<int>>{
                    error{error_code::resource_allocation_failed, "Out of memory"}
                }
            );
        } else {
            allocations.push_back(
                result<std::unique_ptr<int>>{std::make_unique<int>(i)}
            );
        }
    }
    
    int success_count = 0;
    int error_count = 0;
    
    for (const auto& alloc : allocations) {
        if (alloc.has_value()) {
            success_count++;
        } else {
            error_count++;
            EXPECT_EQ(alloc.get_error().code(), error_code::resource_allocation_failed);
        }
    }
    
    EXPECT_EQ(success_count, 9);
    EXPECT_EQ(error_count, 1);
}

} // namespace test
} // namespace kcenon::thread
