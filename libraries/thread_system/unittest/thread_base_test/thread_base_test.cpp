/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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
OF THIS SOFTWARE, ENTIRELY BUSINESS INTERRUPTION) HOWEVER
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <gtest/gtest.h>
#include <kcenon/thread/core/job_queue.h>
#include <kcenon/thread/core/callback_job.h>
#include <kcenon/thread/core/thread_base.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace kcenon::thread;

class ThreadBaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        job_queue_instance = std::make_unique<job_queue>();
        execution_counter.store(0);
    }
    
    void TearDown() override
    {
        job_queue_instance.reset();
    }
    
    std::atomic<int> execution_counter{0};
    std::unique_ptr<job_queue> job_queue_instance;
};

TEST_F(ThreadBaseTest, JobQueueBasicOperations)
{
    EXPECT_TRUE(job_queue_instance->empty());
    
    // Create and enqueue a job
    auto test_job = std::make_unique<callback_job>(
        [this]() -> result_void
        {
            execution_counter.fetch_add(1);
            return result_void();
        },
        "test_job"
    );
    
    auto error = job_queue_instance->enqueue(std::move(test_job));
    EXPECT_FALSE(error.has_error());
    EXPECT_FALSE(job_queue_instance->empty());
}

TEST_F(ThreadBaseTest, JobQueueDequeue)
{
    // Enqueue a job
    auto test_job = std::make_unique<callback_job>(
        [this]() -> result_void
        {
            execution_counter.fetch_add(1);
            return result_void();
        },
        "dequeue_test_job"
    );
    
    auto error = job_queue_instance->enqueue(std::move(test_job));
    EXPECT_FALSE(error.has_error());
    
    // Dequeue and execute
    auto dequeued_job = job_queue_instance->dequeue();
    EXPECT_TRUE(static_cast<bool>(dequeued_job));
    if (dequeued_job.has_value()) {
        EXPECT_NE(dequeued_job.value(), nullptr);
        EXPECT_TRUE(job_queue_instance->empty());
        
        // Execute the dequeued job using do_work()
        auto execution_result = dequeued_job.value()->do_work();
        EXPECT_FALSE(execution_result.has_error()); // No error
        EXPECT_EQ(execution_counter.load(), 1);
    }
}

TEST_F(ThreadBaseTest, JobQueueMultipleJobs)
{
    const int job_count = 5; // Reduced for simpler testing
    
    // Enqueue multiple jobs
    for (int i = 0; i < job_count; ++i)
    {
        auto job = std::make_unique<callback_job>(
            [this, i]() -> result_void
            {
                execution_counter.fetch_add(i + 1);
                return result_void();
            },
            "job_" + std::to_string(i)
        );
        
        auto error = job_queue_instance->enqueue(std::move(job));
        EXPECT_FALSE(error.has_error());
    }
    
    // Dequeue and execute all jobs
    int expected_sum = 0;
    for (int i = 0; i < job_count; ++i)
    {
        auto job = job_queue_instance->dequeue();
        EXPECT_TRUE(static_cast<bool>(job));
        
        if (job.has_value()) {
            auto result = job.value()->do_work();
            EXPECT_FALSE(result.has_error()); // No error
            expected_sum += (i + 1);
        }
    }
    
    EXPECT_TRUE(job_queue_instance->empty());
    EXPECT_EQ(execution_counter.load(), expected_sum);
}

TEST_F(ThreadBaseTest, CallbackJobExecution)
{
    bool job_executed = false;
    std::string job_result;
    
    auto callback_job_instance = std::make_unique<callback_job>(
        [&job_executed, &job_result]() -> result_void
        {
            job_executed = true;
            job_result = "job completed successfully";
            return result_void();
        },
        "callback_test_job"
    );
    
    EXPECT_FALSE(job_executed);
    
    auto result = callback_job_instance->do_work();
    EXPECT_FALSE(result.has_error());
    EXPECT_TRUE(job_executed);
    EXPECT_EQ(job_result, "job completed successfully");
}

TEST_F(ThreadBaseTest, CallbackJobWithError)
{
    auto callback_job_instance = std::make_unique<callback_job>(
        []() -> result_void
        {
            return error{error_code::job_execution_failed, "job failed with error"};
        },
        "error_test_job"
    );
    
    auto result = callback_job_instance->do_work();
    EXPECT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error().message(), "job failed with error");
}

TEST_F(ThreadBaseTest, JobQueueStopWaiting)
{
    // Test that we can signal the queue to stop waiting
    EXPECT_NO_THROW({
        job_queue_instance->stop_waiting_dequeue();
    });
}

TEST_F(ThreadBaseTest, ClearQueue)
{
    // Add some jobs to the queue
    for (int i = 0; i < 3; ++i)
    {
        auto job = std::make_unique<callback_job>(
            [this]() -> result_void
            {
                execution_counter.fetch_add(1);
                return result_void();
            },
            "clear_test_job_" + std::to_string(i)
        );
        
        auto error = job_queue_instance->enqueue(std::move(job));
        EXPECT_FALSE(error.has_error());
    }
    
    EXPECT_FALSE(job_queue_instance->empty());
    
    // Clear the queue
    job_queue_instance->clear();
    EXPECT_TRUE(job_queue_instance->empty());
}

TEST_F(ThreadBaseTest, ThreadBaseBasicOperations)
{
    // Test basic thread_base functionality
    class TestThreadBase : public thread_base
    {
    public:
        TestThreadBase() : thread_base("TestThread"), execution_count(0), continue_work(true) {}
        
        std::atomic<int> execution_count;
        std::atomic<bool> continue_work;
    
    protected:
        virtual auto do_work() -> result_void override
        {
            execution_count.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            // Stop after a few iterations
            if (execution_count.load() >= 3) {
                continue_work.store(false);
            }
            
            return {};
        }
        
        virtual auto should_continue_work() const -> bool override
        {
            return continue_work.load();
        }
    };
    
    auto test_thread = std::make_unique<TestThreadBase>();
    
    // Test start and stop
    EXPECT_NO_THROW({
        auto start_result = test_thread->start();
        EXPECT_FALSE(start_result.has_error());
        
        // Give it some time to run and complete work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto stop_result = test_thread->stop();
        EXPECT_FALSE(stop_result.has_error());
    });
    
    EXPECT_GT(test_thread->execution_count.load(), 0);
}
