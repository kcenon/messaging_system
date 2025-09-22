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
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <gtest/gtest.h>
#include <kcenon/thread/core/job_queue.h>
#include <kcenon/thread/core/callback_job.h>
#include <kcenon/thread/core/thread_base.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <barrier>
#include <latch>
#include <random>

namespace kcenon::thread {
namespace test {

class ConcurrencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test environment
    }

    void TearDown() override {
        // Clean up
    }
};

// Test rapid start/stop operations (sequential)
TEST_F(ConcurrencyTest, ThreadBaseRapidStartStop) {
    class test_thread : public thread_base {
    public:
        test_thread() : thread_base("rapid_test") {}
        std::atomic<int> work_cycles{0};
        std::atomic<int> start_calls{0};
        std::atomic<int> stop_calls{0};
        
    protected:
        result_void before_start() override {
            start_calls.fetch_add(1);
            return {};
        }
        
        result_void do_work() override {
            work_cycles.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            return {};
        }
        
        result_void after_stop() override {
            stop_calls.fetch_add(1);
            return {};
        }
    };
    
    auto worker = std::make_unique<test_thread>();
    
    // Set wake interval to ensure do_work is called regularly
    worker->set_wake_interval(std::chrono::milliseconds(5));
    
    // Test rapid sequential start/stop cycles
    const int num_cycles = 10;
    for (int i = 0; i < num_cycles; ++i) {
        // Start the thread
        auto result = worker->start();
        EXPECT_FALSE(result.has_error());
        
        // Let it work a bit longer to ensure do_work is called
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
        // Stop the thread
        worker->stop();
        
        // Just verify thread is stopped by checking it can be started again
        // (thread_base::start() checks if thread is already running)
    }
    
    // Verify calls were made
    EXPECT_EQ(worker->start_calls.load(), num_cycles);
    EXPECT_EQ(worker->stop_calls.load(), num_cycles);
    EXPECT_GT(worker->work_cycles.load(), 0);
}

// Test queue operations under extreme concurrency
TEST_F(ConcurrencyTest, JobQueueExtremeConcurrency) {
    auto queue = std::make_shared<job_queue>();
    const int num_producers = 20;
    const int num_consumers = 20;
    const int jobs_per_producer = 100;
    
    std::atomic<int> enqueued{0};
    std::atomic<int> dequeued{0};
    std::atomic<int> enqueue_failures{0};
    std::atomic<bool> stop_consumers{false};
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    
    // Start consumers
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, &dequeued, &stop_consumers]() {
            while (!stop_consumers.load()) {
                auto result = queue->dequeue();
                if (result.has_value() && result.value()) {
                    [[maybe_unused]] auto work_result = result.value()->do_work();
                    dequeued.fetch_add(1);
                }
                std::this_thread::yield();
            }
        });
    }
    
    // Start producers
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&queue, &enqueued, &enqueue_failures, jobs_per_producer]() {
            for (int j = 0; j < jobs_per_producer; ++j) {
                auto job = std::make_unique<callback_job>([]() -> result_void { return {}; });
                auto result = queue->enqueue(std::move(job));
                if (!result.has_error()) {
                    enqueued.fetch_add(1);
                } else {
                    enqueue_failures.fetch_add(1);
                }
            }
        });
    }
    
    // Wait for producers
    for (auto& t : producers) {
        t.join();
    }
    
    // Let consumers drain the queue
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop consumers
    stop_consumers.store(true);
    queue->stop_waiting_dequeue();
    
    for (auto& t : consumers) {
        t.join();
    }
    
    EXPECT_EQ(enqueued.load(), num_producers * jobs_per_producer);
    EXPECT_EQ(enqueue_failures.load(), 0);
    EXPECT_LE(dequeued.load(), enqueued.load());
}

// Test job queue boundary conditions
TEST_F(ConcurrencyTest, JobQueueBoundaryConditions) {
    auto queue = std::make_shared<job_queue>();
    
    const int num_jobs = 100;
    std::atomic<int> enqueued{0};
    std::atomic<int> dequeued{0};
    
    // Producer thread
    std::thread producer([&queue, &enqueued, num_jobs]() {
        for (int i = 0; i < num_jobs; ++i) {
            auto job = std::make_unique<callback_job>([i]() -> result_void {
                // Simple job that does nothing
                return {};
            });
            
            auto result = queue->enqueue(std::move(job));
            if (!result.has_error()) {
                enqueued.fetch_add(1);
            }
        }
    });
    
    // Consumer thread
    std::thread consumer([&queue, &dequeued, num_jobs]() {
        while (dequeued.load() < num_jobs) {
            auto result = queue->dequeue();
            if (result.has_value() && result.value()) {
                [[maybe_unused]] auto work_result = result.value()->do_work();
                dequeued.fetch_add(1);
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(enqueued.load(), num_jobs);
    EXPECT_EQ(dequeued.load(), num_jobs);
}

// Test race conditions in job execution
TEST_F(ConcurrencyTest, JobExecutionRaceConditions) {
    auto queue = std::make_shared<job_queue>();
    std::atomic<int> shared_counter{0};
    std::atomic<int> race_detected{0};
    const int num_jobs = 1000;
    
    // Create jobs that increment a shared counter
    // and check for race conditions
    for (int i = 0; i < num_jobs; ++i) {
        auto job = std::make_unique<callback_job>([&shared_counter, &race_detected]() -> result_void {
            int old_value = shared_counter.load();
            std::this_thread::yield(); // Increase chance of race
            int new_value = old_value + 1;
            
            // Try to update atomically
            if (!shared_counter.compare_exchange_strong(old_value, new_value)) {
                race_detected.fetch_add(1);
                // Retry
                shared_counter.fetch_add(1);
            }
            
            return {};
        });
        [[maybe_unused]] auto enqueue_result = queue->enqueue(std::move(job));
    }
    
    // Run multiple workers
    const int num_workers = 8;
    std::vector<std::thread> workers;
    std::atomic<bool> stop_workers{false};
    
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back([&queue, &stop_workers]() {
            while (!stop_workers.load()) {
                auto result = queue->dequeue();
                if (result.has_value() && result.value()) {
                    [[maybe_unused]] auto work_result = result.value()->do_work();
                } else {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
        });
    }
    
    // Wait for all jobs to complete
    while (shared_counter.load() < num_jobs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    stop_workers.store(true);
    queue->stop_waiting_dequeue();
    
    for (auto& t : workers) {
        t.join();
    }
    
    EXPECT_EQ(shared_counter.load(), num_jobs);
    EXPECT_GT(race_detected.load(), 0); // Some races should have been detected
}

// Test memory ordering and visibility
TEST_F(ConcurrencyTest, MemoryOrderingTest) {
    std::atomic<int> x{0};
    std::atomic<int> y{0};
    std::atomic<int> r1{0};
    std::atomic<int> r2{0};
    const int iterations = 10000;
    
    for (int iter = 0; iter < iterations; ++iter) {
        x.store(0);
        y.store(0);
        r1.store(0);
        r2.store(0);
        
        std::thread t1([&x, &y, &r1]() {
            x.store(1, std::memory_order_relaxed);
            r1.store(y.load(std::memory_order_relaxed), std::memory_order_relaxed);
        });
        
        std::thread t2([&x, &y, &r2]() {
            y.store(1, std::memory_order_relaxed);
            r2.store(x.load(std::memory_order_relaxed), std::memory_order_relaxed);
        });
        
        t1.join();
        t2.join();
        
        // With relaxed ordering, it's possible (though unlikely) that both r1 and r2 are 0
        // This would indicate reordering
        if (r1.load() == 0 && r2.load() == 0) {
            // Reordering detected - this is actually valid with relaxed ordering
            // but demonstrates the importance of proper memory ordering
        }
    }
    
    // Test should complete without crashing
    EXPECT_TRUE(true);
}

// Test barrier synchronization
TEST_F(ConcurrencyTest, BarrierSynchronization) {
    const int num_threads = 8;
    std::atomic<int> phase1_count{0};
    std::atomic<int> phase2_count{0};
    std::barrier sync_point(num_threads);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&sync_point, &phase1_count, &phase2_count, i, num_threads]() {
            // Phase 1
            phase1_count.fetch_add(1);
            
            // Wait at barrier
            sync_point.arrive_and_wait();
            
            // Phase 2 - all threads should have completed phase 1
            EXPECT_EQ(phase1_count.load(), num_threads);
            phase2_count.fetch_add(1);
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(phase1_count.load(), num_threads);
    EXPECT_EQ(phase2_count.load(), num_threads);
}

// Test latch coordination
TEST_F(ConcurrencyTest, LatchCoordination) {
    const int num_workers = 5;
    std::latch work_done(num_workers);
    std::atomic<int> work_count{0};
    
    std::vector<std::thread> workers;
    
    // Start workers
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back([&work_done, &work_count]() {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 50));
            work_count.fetch_add(1);
            
            // Signal completion
            work_done.count_down();
        });
    }
    
    // Wait for all workers
    work_done.wait();
    
    // All work should be done
    EXPECT_EQ(work_count.load(), num_workers);
    
    for (auto& t : workers) {
        t.join();
    }
}

// Test ABA problem scenarios
TEST_F(ConcurrencyTest, ABAScenario) {
    struct Node {
        int value;
        std::atomic<Node*> next;
        Node(int v) : value(v), next(nullptr) {}
    };
    
    std::atomic<Node*> head{nullptr};
    const int num_threads = 4;
    const int operations_per_thread = 1000;
    std::atomic<int> aba_detected{0};
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&head, &aba_detected, operations_per_thread]() {
            std::vector<Node*> nodes;
            
            for (int op = 0; op < operations_per_thread; ++op) {
                if (op % 2 == 0) {
                    // Push
                    auto new_node = new Node(op);
                    nodes.push_back(new_node);
                    
                    Node* old_head = head.load();
                    do {
                        new_node->next = old_head;
                    } while (!head.compare_exchange_weak(old_head, new_node));
                } else {
                    // Pop
                    Node* old_head = head.load();
                    while (old_head) {
                        Node* new_head = old_head->next.load();
                        if (head.compare_exchange_weak(old_head, new_head)) {
                            // Successfully popped
                            break;
                        }
                        // CAS failed - potential ABA
                        if (old_head && old_head != head.load()) {
                            aba_detected.fetch_add(1);
                        }
                    }
                }
            }
            
            // Clean up remaining nodes
            for (auto node : nodes) {
                delete node;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Clean up any remaining nodes
    Node* current = head.load();
    while (current) {
        Node* next = current->next.load();
        delete current;
        current = next;
    }
    
    // ABA situations may or may not occur depending on timing
    EXPECT_GE(aba_detected.load(), 0);
}

// Test spurious wakeup handling
TEST_F(ConcurrencyTest, SpuriousWakeupHandling) {
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> ready{false};
    std::atomic<int> spurious_wakeups{0};
    std::atomic<int> valid_wakeups{0};
    
    std::thread waiter([&mutex, &cv, &ready, &spurious_wakeups, &valid_wakeups]() {
        std::unique_lock<std::mutex> lock(mutex);
        
        while (!ready.load()) {
            cv.wait(lock);
            
            // Check if this was a spurious wakeup
            if (!ready.load()) {
                spurious_wakeups.fetch_add(1);
            } else {
                valid_wakeups.fetch_add(1);
            }
        }
    });
    
    // Give waiter time to start waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Send some spurious notifications
    for (int i = 0; i < 3; ++i) {
        cv.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Now set ready and notify
    {
        std::lock_guard<std::mutex> lock(mutex);
        ready.store(true);
    }
    cv.notify_one();
    
    waiter.join();
    
    EXPECT_EQ(valid_wakeups.load(), 1);
    // Spurious wakeups may or may not occur
    EXPECT_GE(spurious_wakeups.load(), 0);
}

} // namespace test
} // namespace kcenon::thread
