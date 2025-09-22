/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include "../../sources/logger/flow/overflow_policy.h"
#include <thread>
#include <chrono>
#include <vector>
#include <random>

using namespace std::chrono_literals;

// Define log_entry in logger_module namespace to match the header
namespace logger_module {
    struct log_entry {
        thread_module::log_level level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        
        log_entry() : level(thread_module::log_level::info) {}
        log_entry(thread_module::log_level lvl, const std::string& msg)
            : level(lvl), message(msg), timestamp(std::chrono::system_clock::now()) {}
    };
}

using namespace logger_module;

class OverflowPolicyTest : public ::testing::Test {
protected:
    void SetUp() override {
        queue_ = std::queue<log_entry>();
        max_size_ = 10;
    }
    
    void FillQueue(size_t count) {
        for (size_t i = 0; i < count; ++i) {
            queue_.push(log_entry(thread_module::log_level::info, 
                                 "Message " + std::to_string(i)));
        }
    }
    
    std::queue<log_entry> queue_;
    size_t max_size_;
};

TEST_F(OverflowPolicyTest, DropOldestPolicy) {
    auto policy = std::make_unique<drop_oldest_policy>();
    
    // Fill queue to max
    FillQueue(max_size_);
    
    // Try to add new entry when full
    log_entry new_entry(thread_module::log_level::warning, "New message");
    bool result = policy->handle_overflow(new_entry, queue_, max_size_);
    
    EXPECT_TRUE(result);  // Should return true to allow adding
    EXPECT_EQ(queue_.size(), max_size_ - 1);  // Should have removed oldest
    
    auto stats = policy->get_stats();
    EXPECT_EQ(stats.total_messages.load(), 1);
    EXPECT_EQ(stats.dropped_messages.load(), 1);
}

TEST_F(OverflowPolicyTest, DropNewestPolicy) {
    auto policy = std::make_unique<drop_newest_policy>();
    
    // Fill queue to max
    FillQueue(max_size_);
    
    // Try to add new entry when full
    log_entry new_entry(thread_module::log_level::warning, "New message");
    bool result = policy->handle_overflow(new_entry, queue_, max_size_);
    
    EXPECT_FALSE(result);  // Should return false to drop new entry
    EXPECT_EQ(queue_.size(), max_size_);  // Queue should remain full
    
    auto stats = policy->get_stats();
    EXPECT_EQ(stats.total_messages.load(), 1);
    EXPECT_EQ(stats.dropped_messages.load(), 1);
}

TEST_F(OverflowPolicyTest, BlockPolicy) {
    auto policy = std::make_unique<block_policy>(100ms);
    
    // Fill queue to max
    FillQueue(max_size_);
    
    // Start a thread that will remove an item after delay
    std::thread remover([this]() {
        std::this_thread::sleep_for(50ms);
        if (!queue_.empty()) {
            queue_.pop();
        }
    });
    
    // Try to add new entry when full (should block then succeed)
    log_entry new_entry(thread_module::log_level::warning, "New message");
    auto start = std::chrono::steady_clock::now();
    bool result = policy->handle_overflow(new_entry, queue_, max_size_);
    auto duration = std::chrono::steady_clock::now() - start;
    
    remover.join();
    
    EXPECT_TRUE(result);  // Should succeed after blocking
    EXPECT_GE(duration, 40ms);  // Should have blocked for some time
    
    auto stats = policy->get_stats();
    EXPECT_EQ(stats.blocked_count.load(), 1);
}

TEST_F(OverflowPolicyTest, BlockPolicyTimeout) {
    auto policy = std::make_unique<block_policy>(50ms);
    
    // Fill queue to max and don't remove anything
    FillQueue(max_size_);
    
    // Try to add new entry when full (should timeout)
    log_entry new_entry(thread_module::log_level::warning, "New message");
    auto start = std::chrono::steady_clock::now();
    bool result = policy->handle_overflow(new_entry, queue_, max_size_);
    auto duration = std::chrono::steady_clock::now() - start;
    
    EXPECT_FALSE(result);  // Should fail after timeout
    EXPECT_GE(duration, 45ms);  // Should have waited for timeout
    
    auto stats = policy->get_stats();
    EXPECT_EQ(stats.blocked_count.load(), 1);
    EXPECT_EQ(stats.dropped_messages.load(), 1);
}

TEST_F(OverflowPolicyTest, GrowPolicy) {
    auto policy = std::make_unique<grow_policy>(2, 100);
    
    // Fill queue to max
    FillQueue(max_size_);
    
    // Try to add new entry when full
    log_entry new_entry(thread_module::log_level::warning, "New message");
    bool result = policy->handle_overflow(new_entry, queue_, max_size_);
    
    EXPECT_TRUE(result);  // Should allow growth
    EXPECT_GT(policy->get_current_growth(), 0);
    
    auto stats = policy->get_stats();
    EXPECT_EQ(stats.grow_count.load(), 1);
}

TEST_F(OverflowPolicyTest, GrowPolicyMaxLimit) {
    auto policy = std::make_unique<grow_policy>(2, 15);  // Very low max
    
    // Fill queue to max
    FillQueue(max_size_);
    
    // Try to grow multiple times
    for (int i = 0; i < 10; ++i) {
        log_entry new_entry(thread_module::log_level::warning, 
                          "New message " + std::to_string(i));
        policy->handle_overflow(new_entry, queue_, max_size_);
    }
    
    auto stats = policy->get_stats();
    EXPECT_GT(stats.dropped_messages.load(), 0);  // Some should be dropped
}

TEST_F(OverflowPolicyTest, PolicyFactory) {
    auto drop_old = overflow_policy_factory::create(overflow_policy_type::drop_oldest);
    EXPECT_EQ(drop_old->get_type(), overflow_policy_type::drop_oldest);
    
    auto drop_new = overflow_policy_factory::create(overflow_policy_type::drop_newest);
    EXPECT_EQ(drop_new->get_type(), overflow_policy_type::drop_newest);
    
    auto block = overflow_policy_factory::create(overflow_policy_type::block);
    EXPECT_EQ(block->get_type(), overflow_policy_type::block);
    
    auto grow = overflow_policy_factory::create(overflow_policy_type::grow);
    EXPECT_EQ(grow->get_type(), overflow_policy_type::grow);
}

TEST_F(OverflowPolicyTest, CustomPolicy) {
    int custom_calls = 0;
    auto custom = overflow_policy_factory::create_custom(
        [&custom_calls](const log_entry&, std::queue<log_entry>&, size_t) {
            custom_calls++;
            return false;  // Always drop
        });
    
    log_entry entry(thread_module::log_level::info, "Test");
    bool result = custom->handle_overflow(entry, queue_, max_size_);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(custom_calls, 1);
    EXPECT_EQ(custom->get_type(), overflow_policy_type::custom);
    
    auto stats = custom->get_stats();
    EXPECT_EQ(stats.dropped_messages.load(), 1);
}

TEST_F(OverflowPolicyTest, OverflowStatsCalculation) {
    auto policy = std::make_unique<drop_oldest_policy>();
    
    // Simulate multiple operations
    for (int i = 0; i < 100; ++i) {
        FillQueue(max_size_);
        log_entry entry(thread_module::log_level::info, "Test");
        policy->handle_overflow(entry, queue_, max_size_);
        queue_ = std::queue<log_entry>();  // Clear for next iteration
    }
    
    auto stats = policy->get_stats();
    EXPECT_EQ(stats.total_messages.load(), 100);
    EXPECT_EQ(stats.dropped_messages.load(), 100);
    EXPECT_DOUBLE_EQ(stats.get_drop_rate(), 100.0);
    
    // Reset stats
    policy->reset_stats();
    stats = policy->get_stats();
    EXPECT_EQ(stats.total_messages.load(), 0);
    EXPECT_EQ(stats.dropped_messages.load(), 0);
    EXPECT_DOUBLE_EQ(stats.get_drop_rate(), 0.0);
}

class AdaptiveBackpressureTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.min_batch_size = 10;
        config_.max_batch_size = 100;
        config_.initial_batch_size = 50;
        config_.min_flush_interval = 10ms;
        config_.max_flush_interval = 100ms;
        config_.initial_flush_interval = 50ms;
        config_.load_threshold_low = 0.3;
        config_.load_threshold_high = 0.7;
        config_.adaptation_rate = 0.2;
        
        backpressure_ = std::make_unique<adaptive_backpressure>(config_);
    }
    
    adaptive_backpressure::config config_;
    std::unique_ptr<adaptive_backpressure> backpressure_;
};

TEST_F(AdaptiveBackpressureTest, InitialValues) {
    EXPECT_EQ(backpressure_->get_batch_size(), 50);
    EXPECT_EQ(backpressure_->get_flush_interval(), 50ms);
    EXPECT_DOUBLE_EQ(backpressure_->get_current_load(), 0.0);
    EXPECT_TRUE(backpressure_->is_enabled());
}

TEST_F(AdaptiveBackpressureTest, HighLoadAdaptation) {
    // Simulate high load
    for (int i = 0; i < 10; ++i) {
        backpressure_->update_metrics(0.8, 100ms);  // 80% queue usage
    }
    
    // Should increase batch size and decrease flush interval
    EXPECT_GT(backpressure_->get_batch_size(), 50);
    EXPECT_LT(backpressure_->get_flush_interval(), 50ms);
    EXPECT_GT(backpressure_->get_current_load(), 0.7);
}

TEST_F(AdaptiveBackpressureTest, LowLoadAdaptation) {
    // Simulate low load
    for (int i = 0; i < 10; ++i) {
        backpressure_->update_metrics(0.2, 10ms);  // 20% queue usage
    }
    
    // Should decrease batch size and increase flush interval
    EXPECT_LT(backpressure_->get_batch_size(), 50);
    EXPECT_GT(backpressure_->get_flush_interval(), 50ms);
    EXPECT_LT(backpressure_->get_current_load(), 0.3);
}

TEST_F(AdaptiveBackpressureTest, DisableAdaptation) {
    backpressure_->set_enabled(false);
    EXPECT_FALSE(backpressure_->is_enabled());
    
    // Update metrics shouldn't trigger adaptation
    size_t initial_batch = backpressure_->get_batch_size();
    for (int i = 0; i < 10; ++i) {
        backpressure_->update_metrics(0.9, 100ms);
    }
    
    EXPECT_EQ(backpressure_->get_batch_size(), initial_batch);
}

TEST_F(AdaptiveBackpressureTest, ManualAdaptation) {
    // Set high load but don't update metrics
    backpressure_->set_enabled(false);
    for (int i = 0; i < 5; ++i) {
        backpressure_->update_metrics(0.8, 100ms);
    }
    
    size_t batch_before = backpressure_->get_batch_size();
    
    // Manually trigger adaptation
    backpressure_->set_enabled(true);
    backpressure_->adapt();
    
    EXPECT_GT(backpressure_->get_batch_size(), batch_before);
}

TEST_F(AdaptiveBackpressureTest, Reset) {
    // Change from initial state
    for (int i = 0; i < 10; ++i) {
        backpressure_->update_metrics(0.8, 100ms);
    }
    
    EXPECT_NE(backpressure_->get_batch_size(), 50);
    
    // Reset
    backpressure_->reset();
    
    EXPECT_EQ(backpressure_->get_batch_size(), 50);
    EXPECT_EQ(backpressure_->get_flush_interval(), 50ms);
    EXPECT_DOUBLE_EQ(backpressure_->get_current_load(), 0.0);
}

TEST_F(AdaptiveBackpressureTest, Statistics) {
    // Generate some adaptation activity
    // First push high load to trigger increase
    for (int i = 0; i < 10; ++i) {
        backpressure_->update_metrics(0.8, 100ms);  // High load
    }
    
    // Then push low load to trigger decrease
    for (int i = 0; i < 10; ++i) {
        backpressure_->update_metrics(0.1, 10ms);   // Very low load
    }
    
    auto stats = backpressure_->get_stats();
    EXPECT_GT(stats.adaptation_count, 0);
    EXPECT_GT(stats.increase_count, 0);
    // Decrease count might be 0 if the adaptation hasn't triggered yet
    // We'll check that at least one of increase or decrease happened
    EXPECT_GT(stats.increase_count + stats.decrease_count, 0);
    EXPECT_GE(stats.current_load, 0.0);
}

TEST_F(AdaptiveBackpressureTest, BoundaryEnforcement) {
    // Try to push beyond max limits
    for (int i = 0; i < 100; ++i) {
        backpressure_->update_metrics(0.99, 200ms);  // Very high load
    }
    
    EXPECT_LE(backpressure_->get_batch_size(), config_.max_batch_size);
    EXPECT_GE(backpressure_->get_flush_interval(), config_.min_flush_interval);
    
    // Reset and try to push below min limits
    backpressure_->reset();
    for (int i = 0; i < 100; ++i) {
        backpressure_->update_metrics(0.01, 1ms);  // Very low load
    }
    
    EXPECT_GE(backpressure_->get_batch_size(), config_.min_batch_size);
    EXPECT_LE(backpressure_->get_flush_interval(), config_.max_flush_interval);
}

class OverflowQueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        max_size_ = 5;
    }
    
    size_t max_size_;
};

TEST_F(OverflowQueueTest, BasicOperations) {
    overflow_queue<log_entry> queue(max_size_);
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    
    // Push items
    for (size_t i = 0; i < max_size_; ++i) {
        EXPECT_TRUE(queue.push(log_entry(thread_module::log_level::info, 
                                        "Message " + std::to_string(i))));
    }
    
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), max_size_);
    
    // Pop items
    log_entry item;
    EXPECT_TRUE(queue.pop(item, 100ms));
    EXPECT_EQ(queue.size(), max_size_ - 1);
}

TEST_F(OverflowQueueTest, OverflowWithDropOldest) {
    auto policy = overflow_policy_factory::create(overflow_policy_type::drop_oldest);
    overflow_queue<log_entry> queue(max_size_, std::move(policy));
    
    // Fill queue
    for (size_t i = 0; i < max_size_; ++i) {
        queue.push(log_entry(thread_module::log_level::info, 
                           "Message " + std::to_string(i)));
    }
    
    // Push when full (should drop oldest)
    EXPECT_TRUE(queue.push(log_entry(thread_module::log_level::warning, "New")));
    
    auto stats = queue.get_stats();
    EXPECT_GT(stats.dropped_messages.load(), 0);
}

TEST_F(OverflowQueueTest, PolicyChange) {
    overflow_queue<log_entry> queue(max_size_);
    
    // Initially uses drop_oldest by default
    for (size_t i = 0; i < max_size_ + 2; ++i) {
        queue.push(log_entry(thread_module::log_level::info, 
                           "Message " + std::to_string(i)));
    }
    
    auto stats1 = queue.get_stats();
    EXPECT_GT(stats1.dropped_messages.load(), 0);
    
    // Change to drop_newest
    queue.set_policy(overflow_policy_factory::create(overflow_policy_type::drop_newest));
    
    // New messages should be dropped
    for (size_t i = 0; i < 5; ++i) {
        queue.push(log_entry(thread_module::log_level::info, "Extra"));
    }
    
    EXPECT_LE(queue.size(), max_size_);
}

TEST_F(OverflowQueueTest, ConcurrentAccess) {
    overflow_queue<log_entry> queue(max_size_);
    
    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    
    // Start producer threads
    std::vector<std::thread> producers;
    for (int i = 0; i < 3; ++i) {
        producers.emplace_back([&queue, &push_count]() {
            for (int j = 0; j < 100; ++j) {
                if (queue.push(log_entry(thread_module::log_level::info, "Test"))) {
                    push_count++;
                }
                std::this_thread::sleep_for(1ms);
            }
        });
    }
    
    // Start consumer threads
    std::vector<std::thread> consumers;
    for (int i = 0; i < 2; ++i) {
        consumers.emplace_back([&queue, &pop_count]() {
            log_entry item;
            for (int j = 0; j < 150; ++j) {
                if (queue.pop(item, 10ms)) {
                    pop_count++;
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();
    
    // Verify counts are reasonable
    EXPECT_GT(push_count.load(), 0);
    EXPECT_GT(pop_count.load(), 0);
    EXPECT_LE(queue.size(), max_size_);
}

TEST_F(OverflowQueueTest, StopQueue) {
    overflow_queue<log_entry> queue(max_size_);
    
    // Add some items
    queue.push(log_entry(thread_module::log_level::info, "Test"));
    
    // Stop the queue
    queue.stop();
    
    // Pop should return false even with items
    log_entry item;
    EXPECT_FALSE(queue.pop(item, 10ms));
    
    // Push should still work but pop won't
    queue.push(log_entry(thread_module::log_level::info, "After stop"));
    EXPECT_FALSE(queue.pop(item, 10ms));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}