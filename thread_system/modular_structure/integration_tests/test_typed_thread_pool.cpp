#include <gtest/gtest.h>
#include <thread_system_core/typed_thread_pool/pool/typed_thread_pool.h>
#include <thread_system_core/typed_thread_pool/pool/pool_builder.h>
#include <thread_system_core/typed_thread_pool/jobs/callback_typed_job.h>
#include <atomic>
#include <vector>

using namespace typed_thread_pool_module;

class TypedThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool_ = pool_builder()
            .with_name("typed_test_pool")
            .with_worker_count(4)
            .with_queue_strategy(queue_strategy::lockfree)
            .build();
    }

    void TearDown() override {
        if (pool_) {
            pool_->stop();
        }
    }

    std::shared_ptr<typed_thread_pool_t<priority_job_types>> pool_;
};

TEST_F(TypedThreadPoolTest, PriorityOrdering) {
    std::vector<int> execution_order;
    std::mutex order_mutex;
    
    EXPECT_FALSE(pool_->start().has_value());
    
    // Block the pool with a long job
    std::atomic<bool> blocker_can_finish(false);
    pool_->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
        [&blocker_can_finish]() {
            while (!blocker_can_finish) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        },
        "blocker"
    ));
    
    // Wait for blocker to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Now enqueue jobs with different priorities
    pool_->enqueue<low_job>(std::make_unique<callback_typed_job<low_job>>(
        [&execution_order, &order_mutex]() {
            std::lock_guard<std::mutex> lock(order_mutex);
            execution_order.push_back(3);  // Low priority
        },
        "low_priority"
    ));
    
    pool_->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
        [&execution_order, &order_mutex]() {
            std::lock_guard<std::mutex> lock(order_mutex);
            execution_order.push_back(2);  // Normal priority
        },
        "normal_priority"
    ));
    
    pool_->enqueue<high_job>(std::make_unique<callback_typed_job<high_job>>(
        [&execution_order, &order_mutex]() {
            std::lock_guard<std::mutex> lock(order_mutex);
            execution_order.push_back(1);  // High priority
        },
        "high_priority"
    ));
    
    // Let blocker finish
    blocker_can_finish = true;
    
    // Wait for all jobs to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // High priority job should execute first (1), then normal (2), then low (3)
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1);  // High priority first
    EXPECT_EQ(execution_order[1], 2);  // Normal priority second
    EXPECT_EQ(execution_order[2], 3);  // Low priority last
}

TEST_F(TypedThreadPoolTest, TypeSafeEnqueue) {
    std::atomic<int> high_count(0);
    std::atomic<int> normal_count(0);
    std::atomic<int> low_count(0);
    
    EXPECT_FALSE(pool_->start().has_value());
    
    // Enqueue different job types
    for (int i = 0; i < 10; ++i) {
        pool_->enqueue<high_job>(std::make_unique<callback_typed_job<high_job>>(
            [&high_count]() { high_count++; },
            "high_" + std::to_string(i)
        ));
        
        pool_->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
            [&normal_count]() { normal_count++; },
            "normal_" + std::to_string(i)
        ));
        
        pool_->enqueue<low_job>(std::make_unique<callback_typed_job<low_job>>(
            [&low_count]() { low_count++; },
            "low_" + std::to_string(i)
        ));
    }
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    EXPECT_EQ(high_count.load(), 10);
    EXPECT_EQ(normal_count.load(), 10);
    EXPECT_EQ(low_count.load(), 10);
}

TEST_F(TypedThreadPoolTest, BatchEnqueueTyped) {
    std::atomic<int> job_count(0);
    
    EXPECT_FALSE(pool_->start().has_value());
    
    // Create batch of high priority jobs
    std::vector<std::unique_ptr<typed_job<high_job>>> high_jobs;
    for (int i = 0; i < 20; ++i) {
        high_jobs.push_back(std::make_unique<callback_typed_job<high_job>>(
            [&job_count]() { job_count++; },
            "batch_high_" + std::to_string(i)
        ));
    }
    
    // Enqueue batch
    EXPECT_FALSE(pool_->enqueue_batch<high_job>(std::move(high_jobs)).has_value());
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    EXPECT_EQ(job_count.load(), 20);
}

TEST_F(TypedThreadPoolTest, QueueSizeByType) {
    EXPECT_FALSE(pool_->start().has_value());
    
    // Block all workers
    std::atomic<bool> can_proceed(false);
    for (int i = 0; i < 4; ++i) {  // 4 workers
        pool_->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
            [&can_proceed]() {
                while (!can_proceed) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            },
            "blocker_" + std::to_string(i)
        ));
    }
    
    // Wait for blockers to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Enqueue different types
    for (int i = 0; i < 5; ++i) {
        pool_->enqueue<high_job>(std::make_unique<callback_typed_job<high_job>>(
            []() {}, "high"
        ));
        pool_->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
            []() {}, "normal"
        ));
        pool_->enqueue<low_job>(std::make_unique<callback_typed_job<low_job>>(
            []() {}, "low"
        ));
    }
    
    // Check queue sizes
    EXPECT_EQ(pool_->size<high_job>(), 5);
    EXPECT_GT(pool_->size<normal_job>(), 5);  // Has blockers too
    EXPECT_EQ(pool_->size<low_job>(), 5);
    
    // Total size
    EXPECT_GT(pool_->size(), 15);
    
    // Let everything complete
    can_proceed = true;
}

TEST_F(TypedThreadPoolTest, DifferentQueueStrategies) {
    // Test adaptive queue
    auto adaptive_pool = pool_builder()
        .with_name("adaptive_pool")
        .with_worker_count(2)
        .with_queue_strategy(queue_strategy::adaptive)
        .build();
    
    EXPECT_FALSE(adaptive_pool->start().has_value());
    
    std::atomic<int> counter(0);
    for (int i = 0; i < 10; ++i) {
        adaptive_pool->enqueue<normal_job>(std::make_unique<callback_typed_job<normal_job>>(
            [&counter]() { counter++; },
            "adaptive_job"
        ));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter.load(), 10);
    
    adaptive_pool->stop();
}

TEST_F(TypedThreadPoolTest, PoolBuilderConfiguration) {
    // Test various builder configurations
    auto custom_pool = pool_builder()
        .with_name("custom_pool")
        .with_worker_count(8)
        .with_queue_strategy(queue_strategy::lockfree)
        .with_max_threads(16)
        .build();
    
    ASSERT_NE(custom_pool, nullptr);
    EXPECT_EQ(custom_pool->to_string(), "[typed_thread_pool: custom_pool]");
    
    custom_pool->start();
    custom_pool->stop();
}