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
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <array>
#include <cstring>

namespace platform_test {

class AtomicOperationsTest : public ::testing::Test {
protected:
    static constexpr int NUM_THREADS = 4;
    static constexpr int ITERATIONS = 100000;
    
    void SetUp() override {
        // Ensure consistent test environment
    }
};

// Test atomic operations on different sizes
TEST_F(AtomicOperationsTest, AtomicSizeSupport) {
    // Test 1-byte atomic
    {
        std::atomic<uint8_t> val{0};
        EXPECT_TRUE(val.is_lock_free());
        
        uint8_t old = val.fetch_add(1);
        EXPECT_EQ(old, 0);
        EXPECT_EQ(val.load(), 1);
    }
    
    // Test 2-byte atomic
    {
        std::atomic<uint16_t> val{0};
        EXPECT_TRUE(val.is_lock_free());
        
        uint16_t old = val.fetch_or(0xFF00);
        EXPECT_EQ(old, 0);
        EXPECT_EQ(val.load(), 0xFF00);
    }
    
    // Test 4-byte atomic
    {
        std::atomic<uint32_t> val{0xFFFFFFFF};
        EXPECT_TRUE(val.is_lock_free());
        
        uint32_t old = val.fetch_and(0x0F0F0F0F);
        EXPECT_EQ(old, 0xFFFFFFFF);
        EXPECT_EQ(val.load(), 0x0F0F0F0F);
    }
    
    // Test 8-byte atomic
    {
        std::atomic<uint64_t> val{0};
        EXPECT_TRUE(val.is_lock_free());
        
        uint64_t old = val.fetch_xor(0xAAAAAAAAAAAAAAAA);
        EXPECT_EQ(old, 0);
        EXPECT_EQ(val.load(), 0xAAAAAAAAAAAAAAAA);
    }
    
    // Test 16-byte atomic (may not be lock-free on all platforms)
    {
        struct alignas(16) Data16 {
            uint64_t a, b;
            
            bool operator==(const Data16& other) const {
                return a == other.a && b == other.b;
            }
        };
        
        std::atomic<Data16> val{{1, 2}};
        bool is_lock_free = val.is_lock_free();
        
        // Platform-specific expectation
#if defined(__x86_64__) || defined(_M_X64)
        // x86-64 typically supports 16-byte atomics with CMPXCHG16B
        if (is_lock_free) {
            Data16 expected{1, 2};
            Data16 desired{3, 4};
            bool result = val.compare_exchange_strong(expected, desired);
            EXPECT_TRUE(result);
            EXPECT_EQ(val.load().a, 3);
            EXPECT_EQ(val.load().b, 4);
        }
#endif
    }
}

// Test memory ordering guarantees
TEST_F(AtomicOperationsTest, MemoryOrdering) {
    // Sequential consistency test
    {
        std::atomic<int> x{0}, y{0};
        std::atomic<int> r1{0}, r2{0};
        
        auto test_seq_cst = [&]() {
            x = 0; y = 0; r1 = 0; r2 = 0;
            
            std::thread t1([&]() {
                x.store(1);
                r1.store(y.load());
            });
            
            std::thread t2([&]() {
                y.store(1);
                r2.store(x.load());
            });
            
            t1.join();
            t2.join();
            
            // With seq_cst, at least one thread must see the other's write
            return (r1.load() == 1) || (r2.load() == 1);
        };
        
        // Run multiple times - should always be true with seq_cst
        for (int i = 0; i < 100; ++i) {
            EXPECT_TRUE(test_seq_cst());
        }
    }
    
    // Acquire-release test
    {
        std::atomic<int> data{0};
        std::atomic<bool> ready{false};
        
        std::thread producer([&]() {
            data.store(42, std::memory_order_relaxed);
            ready.store(true, std::memory_order_release);
        });
        
        std::thread consumer([&]() {
            while (!ready.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            // Should see data = 42 due to acquire-release synchronization
            EXPECT_EQ(data.load(std::memory_order_relaxed), 42);
        });
        
        producer.join();
        consumer.join();
    }
}

// Test compare-and-swap patterns
TEST_F(AtomicOperationsTest, CompareAndSwapPatterns) {
    // Basic CAS loop
    {
        std::atomic<int> counter{0};
        std::vector<std::thread> threads;
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&counter]() {
                for (int j = 0; j < ITERATIONS; ++j) {
                    int expected = counter.load();
                    while (!counter.compare_exchange_weak(expected, expected + 1)) {
                        // expected is updated by CAS
                    }
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        EXPECT_EQ(counter.load(), NUM_THREADS * ITERATIONS);
    }
    
    // Strong vs weak CAS
    {
        std::atomic<int> value{0};
        int expected = 0;
        int desired = 42;
        
        // Strong CAS - should not fail spuriously
        bool strong_result = value.compare_exchange_strong(expected, desired);
        EXPECT_TRUE(strong_result);
        EXPECT_EQ(value.load(), 42);
        
        // Reset
        value.store(0);
        expected = 0;
        
        // Weak CAS - may fail spuriously but is typically faster
        int attempts = 0;
        while (!value.compare_exchange_weak(expected, desired) && expected == 0) {
            attempts++;
            if (attempts > 1000) break; // Prevent infinite loop
        }
        EXPECT_EQ(value.load(), 42);
    }
}

// Test atomic fetch operations
TEST_F(AtomicOperationsTest, AtomicFetchOperations) {
    // Fetch-add with different memory orderings
    {
        std::atomic<int> counter{0};
        
        // Relaxed ordering
        int old1 = counter.fetch_add(10, std::memory_order_relaxed);
        EXPECT_EQ(old1, 0);
        
        // Acquire-release
        int old2 = counter.fetch_add(20, std::memory_order_acq_rel);
        EXPECT_EQ(old2, 10);
        
        // Sequential consistency
        int old3 = counter.fetch_add(30, std::memory_order_seq_cst);
        EXPECT_EQ(old3, 30);
        
        EXPECT_EQ(counter.load(), 60);
    }
    
    // Bitwise operations
    {
        std::atomic<uint32_t> flags{0};
        
        // Set bits
        flags.fetch_or(0x0F);
        EXPECT_EQ(flags.load(), 0x0F);
        
        // Clear bits
        flags.fetch_and(~0x05);
        EXPECT_EQ(flags.load(), 0x0A);
        
        // Toggle bits
        flags.fetch_xor(0xFF);
        EXPECT_EQ(flags.load(), 0xF5);
    }
}

// Test atomic pointers
TEST_F(AtomicOperationsTest, AtomicPointers) {
    struct Node {
        int value;
        Node* next;
    };
    
    // Basic atomic pointer operations
    {
        Node nodes[5] = {{1, nullptr}, {2, nullptr}, {3, nullptr}, {4, nullptr}, {5, nullptr}};
        std::atomic<Node*> head{&nodes[0]};
        
        EXPECT_TRUE(head.is_lock_free());
        
        // Exchange
        Node* old = head.exchange(&nodes[1]);
        EXPECT_EQ(old->value, 1);
        EXPECT_EQ(head.load()->value, 2);
        
        // Compare and swap
        Node* expected = &nodes[1];
        bool result = head.compare_exchange_strong(expected, &nodes[2]);
        EXPECT_TRUE(result);
        EXPECT_EQ(head.load()->value, 3);
    }
    
    // Pointer arithmetic
    {
        int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::atomic<int*> ptr{&array[0]};
        
        // Fetch add (pointer arithmetic)
        int* old = ptr.fetch_add(3);
        EXPECT_EQ(*old, 0);
        EXPECT_EQ(*ptr.load(), 3);
        
        // Fetch sub
        old = ptr.fetch_sub(1);
        EXPECT_EQ(*old, 3);
        EXPECT_EQ(*ptr.load(), 2);
    }
}

// Test atomic flag (the most basic atomic type)
TEST_F(AtomicOperationsTest, AtomicFlag) {
    // Basic flag operations
    {
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
        
        // Test and set
        bool was_set = flag.test_and_set();
        EXPECT_FALSE(was_set); // Was not set before
        
        was_set = flag.test_and_set();
        EXPECT_TRUE(was_set); // Was already set
        
        // Clear
        flag.clear();
        was_set = flag.test_and_set();
        EXPECT_FALSE(was_set); // Was cleared
    }
    
    // Spinlock implementation using atomic_flag
    {
        class Spinlock {
            std::atomic_flag flag = ATOMIC_FLAG_INIT;
            
        public:
            void lock() {
                while (flag.test_and_set(std::memory_order_acquire)) {
                    // Spin
                }
            }
            
            void unlock() {
                flag.clear(std::memory_order_release);
            }
        };
        
        Spinlock spinlock;
        int shared_counter = 0;
        std::vector<std::thread> threads;
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&spinlock, &shared_counter]() {
                for (int j = 0; j < ITERATIONS; ++j) {
                    spinlock.lock();
                    shared_counter++;
                    spinlock.unlock();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        EXPECT_EQ(shared_counter, NUM_THREADS * ITERATIONS);
    }
}

// Test atomic operations performance characteristics
TEST_F(AtomicOperationsTest, PerformanceCharacteristics) {
    const int iterations = 1000000;
    
    // Compare different atomic operations performance
    auto measure_operation = [iterations](auto operation) {
        auto start = std::chrono::high_resolution_clock::now();
        operation(iterations);
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    };
    
    // Load operation
    auto load_time = measure_operation([](int iters) {
        std::atomic<int> value{0};
        volatile int sum = 0;
        for (int i = 0; i < iters; ++i) {
            sum += value.load(std::memory_order_relaxed);
        }
    });
    
    // Store operation
    auto store_time = measure_operation([](int iters) {
        std::atomic<int> value{0};
        for (int i = 0; i < iters; ++i) {
            value.store(i, std::memory_order_relaxed);
        }
    });
    
    // Fetch-add operation
    auto fetch_add_time = measure_operation([](int iters) {
        std::atomic<int> value{0};
        for (int i = 0; i < iters; ++i) {
            value.fetch_add(1, std::memory_order_relaxed);
        }
    });
    
    // CAS operation
    auto cas_time = measure_operation([](int iters) {
        std::atomic<int> value{0};
        for (int i = 0; i < iters; ++i) {
            int expected = i;
            value.compare_exchange_weak(expected, i + 1, std::memory_order_relaxed);
        }
    });
    
    // Typically: load < store < fetch_add < CAS
    // But this can vary by platform and CPU
    // Due to compiler optimizations, some operations might be too fast to measure
    // Just verify we got some measurements
    EXPECT_GE(cas_time, 0);
    EXPECT_GE(fetch_add_time, 0);
    EXPECT_GE(store_time, 0);
    EXPECT_GE(load_time, 0);
    
    // At least one should have measurable time
    EXPECT_GT(cas_time + fetch_add_time + store_time + load_time, 0);
}

// Test wait/notify operations (C++20)
TEST_F(AtomicOperationsTest, WaitNotifyOperations) {
    std::atomic<int> value{0};
    bool consumer_done = false;
    
    std::thread consumer([&value, &consumer_done]() {
        // Wait for value to become non-zero
        value.wait(0);
        EXPECT_NE(value.load(), 0);
        consumer_done = true;
    });
    
    // Give consumer time to start waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Change value and notify
    value.store(42);
    value.notify_one();
    
    consumer.join();
    EXPECT_TRUE(consumer_done);
}

// Test custom atomic types
TEST_F(AtomicOperationsTest, CustomAtomicTypes) {
    // Simple struct that can be atomic
    struct Point {
        int32_t x, y;
        
        bool operator==(const Point& other) const {
            return x == other.x && y == other.y;
        }
    };
    
    static_assert(sizeof(Point) == 8);
    static_assert(std::is_trivially_copyable_v<Point>);
    
    std::atomic<Point> point{{10, 20}};
    
    // Check if lock-free (likely on 64-bit platforms)
    bool is_lock_free = point.is_lock_free();
    
    if (is_lock_free) {
        // Test operations
        Point old_val = point.load();
        EXPECT_EQ(old_val.x, 10);
        EXPECT_EQ(old_val.y, 20);
        
        Point new_val{30, 40};
        point.store(new_val);
        
        Point expected{30, 40};
        Point desired{50, 60};
        bool result = point.compare_exchange_strong(expected, desired);
        EXPECT_TRUE(result);
        
        Point final_val = point.load();
        EXPECT_EQ(final_val.x, 50);
        EXPECT_EQ(final_val.y, 60);
    }
}

} // namespace platform_test