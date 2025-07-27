#include <gtest/gtest.h>
#include <thread_system_core/thread_pool/core/thread_pool.h>
#include <thread_system_core/thread_base/jobs/callback_job.h>
#include <atomic>
#include <chrono>

using namespace thread_pool_module;
using namespace thread_module;

class BasicThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool_ = std::make_shared<thread_pool>("test_pool");
    }

    void TearDown() override {
        if (pool_) {
            pool_->stop();
        }
    }

    std::shared_ptr<thread_pool> pool_;
};

TEST_F(BasicThreadPoolTest, StartAndStop) {
    EXPECT_FALSE(pool_->start().has_value());
    pool_->stop();
}

TEST_F(BasicThreadPoolTest, EnqueueSingleJob) {
    std::atomic<bool> job_executed(false);
    
    auto job = std::make_unique<callback_job>(
        [&job_executed]() { job_executed = true; },
        "test_job"
    );
    
    EXPECT_FALSE(pool_->start().has_value());
    EXPECT_FALSE(pool_->enqueue(std::move(job)).has_value());
    
    // Wait for job execution
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(job_executed.load());
}

TEST_F(BasicThreadPoolTest, EnqueueMultipleJobs) {
    const int num_jobs = 100;
    std::atomic<int> jobs_executed(0);
    
    EXPECT_FALSE(pool_->start().has_value());
    
    for (int i = 0; i < num_jobs; ++i) {
        auto job = std::make_unique<callback_job>(
            [&jobs_executed]() { jobs_executed++; },
            "job_" + std::to_string(i)
        );
        EXPECT_FALSE(pool_->enqueue(std::move(job)).has_value());
    }
    
    // Wait for all jobs to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    EXPECT_EQ(jobs_executed.load(), num_jobs);
}

TEST_F(BasicThreadPoolTest, BatchEnqueue) {
    const int batch_size = 50;
    std::atomic<int> jobs_executed(0);
    
    std::vector<std::unique_ptr<job>> jobs;
    for (int i = 0; i < batch_size; ++i) {
        jobs.push_back(std::make_unique<callback_job>(
            [&jobs_executed]() { jobs_executed++; },
            "batch_job_" + std::to_string(i)
        ));
    }
    
    EXPECT_FALSE(pool_->start().has_value());
    EXPECT_FALSE(pool_->enqueue_batch(std::move(jobs)).has_value());
    
    // Wait for all jobs to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    EXPECT_EQ(jobs_executed.load(), batch_size);
}

TEST_F(BasicThreadPoolTest, StopWithPendingJobs) {
    std::atomic<int> jobs_executed(0);
    
    EXPECT_FALSE(pool_->start().has_value());
    
    // Enqueue many jobs
    for (int i = 0; i < 1000; ++i) {
        auto job = std::make_unique<callback_job>(
            [&jobs_executed]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                jobs_executed++;
            },
            "slow_job_" + std::to_string(i)
        );
        pool_->enqueue(std::move(job));
    }
    
    // Stop immediately
    pool_->stop(true);
    
    // Not all jobs should have executed
    EXPECT_LT(jobs_executed.load(), 1000);
}

TEST_F(BasicThreadPoolTest, GetJobQueue) {
    auto queue = pool_->get_job_queue();
    ASSERT_NE(queue, nullptr);
    EXPECT_EQ(queue->size(), 0);
}

TEST_F(BasicThreadPoolTest, PoolInstanceId) {
    auto pool2 = std::make_shared<thread_pool>("pool2");
    EXPECT_NE(pool_->get_pool_instance_id(), pool2->get_pool_instance_id());
}