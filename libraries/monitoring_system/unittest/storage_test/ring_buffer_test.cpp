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
#include <monitoring/storage/ring_buffer.h>
#include <vector>
#include <thread>

using namespace monitoring_module;

class RingBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer_ = std::make_unique<ring_buffer<int>>(5); // Small size for testing
    }

    std::unique_ptr<ring_buffer<int>> buffer_;
};

// Test basic ring buffer construction
TEST_F(RingBufferTest, ConstructorTest) {
    EXPECT_NE(buffer_, nullptr);
    EXPECT_TRUE(buffer_->empty());
    EXPECT_EQ(buffer_->size(), 0u);
    EXPECT_EQ(buffer_->capacity(), 5u);
}

// Test push operation
TEST_F(RingBufferTest, PushOperation) {
    buffer_->push(1);
    EXPECT_FALSE(buffer_->empty());
    EXPECT_EQ(buffer_->size(), 1u);
    
    buffer_->push(2);
    buffer_->push(3);
    EXPECT_EQ(buffer_->size(), 3u);
}

// Test overflow behavior
TEST_F(RingBufferTest, OverflowBehavior) {
    // Fill buffer to capacity
    for (int i = 1; i <= 5; ++i) {
        buffer_->push(i);
    }
    
    EXPECT_EQ(buffer_->size(), 5u);
    EXPECT_TRUE(buffer_->full());
    
    // Add one more - should overwrite oldest
    buffer_->push(6);
    EXPECT_EQ(buffer_->size(), 5u);
    EXPECT_TRUE(buffer_->full());
}

// Test clear operation
TEST_F(RingBufferTest, ClearOperation) {
    buffer_->push(1);
    buffer_->push(2);
    buffer_->push(3);
    
    EXPECT_EQ(buffer_->size(), 3u);
    EXPECT_FALSE(buffer_->empty());
    
    buffer_->clear();
    
    EXPECT_EQ(buffer_->size(), 0u);
    EXPECT_TRUE(buffer_->empty());
    EXPECT_FALSE(buffer_->full());
}

// Test get recent operation
TEST_F(RingBufferTest, GetRecentOperation) {
    // Add some data
    for (int i = 1; i <= 3; ++i) {
        buffer_->push(i);
    }
    
    auto recent = buffer_->get_recent(2);
    EXPECT_EQ(recent.size(), 2u);
    
    // Should get most recent items (3, 2)
    EXPECT_EQ(recent[0], 3);
    EXPECT_EQ(recent[1], 2);
    
    // Test getting more than available
    auto all = buffer_->get_recent(10);
    EXPECT_EQ(all.size(), 3u);
    
    // Test getting zero items
    auto empty = buffer_->get_recent(0);
    EXPECT_TRUE(empty.empty());
}

// Test with different types
TEST(RingBufferGenericTest, DifferentTypes) {
    ring_buffer<std::string> string_buffer(3);
    
    string_buffer.push("first");
    string_buffer.push("second");
    string_buffer.push("third");
    
    EXPECT_EQ(string_buffer.size(), 3u);
    
    auto recent = string_buffer.get_recent(2);
    EXPECT_EQ(recent.size(), 2u);
    EXPECT_EQ(recent[0], "third");
    EXPECT_EQ(recent[1], "second");
}

// Test multithreaded access
TEST_F(RingBufferTest, MultithreadedAccess) {
    const int num_threads = 4;
    const int items_per_thread = 25;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, items_per_thread]() {
            for (int i = 0; i < items_per_thread; ++i) {
                buffer_->push(t * 100 + i);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Buffer should be full and contain the most recent items
    EXPECT_TRUE(buffer_->full());
    EXPECT_EQ(buffer_->size(), 5u);
}

// Test edge cases
TEST_F(RingBufferTest, EdgeCases) {
    // Test with capacity 1
    ring_buffer<int> tiny_buffer(1);
    
    tiny_buffer.push(1);
    EXPECT_EQ(tiny_buffer.size(), 1u);
    EXPECT_TRUE(tiny_buffer.full());
    
    tiny_buffer.push(2);
    EXPECT_EQ(tiny_buffer.size(), 1u);
    
    auto recent = tiny_buffer.get_recent(1);
    EXPECT_EQ(recent.size(), 1u);
    EXPECT_EQ(recent[0], 2);
}

// Test large buffer
TEST(RingBufferLargeTest, LargeBuffer) {
    ring_buffer<int> large_buffer(1000);
    
    // Fill with 500 items
    for (int i = 0; i < 500; ++i) {
        large_buffer.push(i);
    }
    
    EXPECT_EQ(large_buffer.size(), 500u);
    EXPECT_FALSE(large_buffer.full());
    
    // Get recent 10 items
    auto recent = large_buffer.get_recent(10);
    EXPECT_EQ(recent.size(), 10u);
    
    // Should be 499, 498, 497, ..., 490
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(recent[i], 499 - i);
    }
}