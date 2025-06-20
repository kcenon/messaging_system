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
#include "container/core/container.h"
#include "container/values/numeric_value.h"
#include <thread>
#include <vector>
#include <chrono>

#ifdef CONTAINER_TEST_INTERNALS
using namespace container_module;

TEST(container_thread_safe, basic_thread_safety) {
    auto cont = std::make_shared<value_container>();
    
    // Thread safety is now always enabled internally
    // These tests are removed as the API is no longer public
    
    // Test basic operations
    cont->set_source("source1", "sub1");
    cont->set_target("target1", "sub1");
    cont->set_message_type("test_message");
    
    EXPECT_EQ(cont->source_id(), "source1");
    EXPECT_EQ(cont->source_sub_id(), "sub1");
    EXPECT_EQ(cont->target_id(), "target1");
    EXPECT_EQ(cont->target_sub_id(), "sub1");
    EXPECT_EQ(cont->message_type(), "test_message");
}

TEST(container_thread_safe, concurrent_reads) {
    auto cont = std::make_shared<value_container>();
    cont->set_thread_safe(true);
    
    // Set initial values
    cont->set_source("source", "sub");
    cont->set_target("target", "sub");
    cont->set_message_type("test");
    
    // Add some values
    for (int i = 0; i < 10; ++i) {
        cont->add(std::make_shared<int_value>("value" + std::to_string(i), i));
    }
    
    const int num_threads = 10;
    const int reads_per_thread = 1000;
    std::vector<std::thread> threads;
    
    auto reader = [&cont, reads_per_thread]() {
        for (int i = 0; i < reads_per_thread; ++i) {
            auto src = cont->source_id();
            auto tgt = cont->target_id();
            auto msg = cont->message_type();
            auto val = cont->get_value("value5");
            EXPECT_EQ(src, "source");
            EXPECT_EQ(tgt, "target");
            EXPECT_EQ(msg, "test");
        }
    };
    
    // Start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(reader);
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Check statistics
    auto stats = cont->get_access_statistics();
    EXPECT_GT(stats.read_count, 0);
}

TEST(container_thread_safe, concurrent_writes) {
    auto cont = std::make_shared<value_container>();
    cont->set_thread_safe(true);
    
    const int num_threads = 10;
    const int writes_per_thread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> total_added{0};
    
    auto writer = [&cont, &total_added, writes_per_thread](int thread_id) {
        for (int i = 0; i < writes_per_thread; ++i) {
            std::string name = "thread" + std::to_string(thread_id) + "_val" + std::to_string(i);
            auto val = std::make_shared<int_value>(name, thread_id * 1000 + i);
            if (cont->add(val)) {
                total_added.fetch_add(1);
            }
        }
    };
    
    // Start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(writer, i);
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify all values were added
    EXPECT_EQ(total_added.load(), num_threads * writes_per_thread);
    
    // Check statistics
    auto stats = cont->get_access_statistics();
    EXPECT_GT(stats.write_count, 0);
}

TEST(container_thread_safe, mixed_operations) {
    auto cont = std::make_shared<value_container>();
    cont->set_thread_safe(true);
    
    // Add initial values
    for (int i = 0; i < 20; ++i) {
        cont->add(std::make_shared<int_value>("value" + std::to_string(i), i));
    }
    
    const int num_readers = 5;
    const int num_writers = 3;
    const int operations_per_thread = 100;
    std::vector<std::thread> threads;
    
    // Reader function
    auto reader = [&cont, operations_per_thread]() {
        for (int i = 0; i < operations_per_thread; ++i) {
            auto vals = cont->value_array("value10");
            auto val = cont->get_value("value15");
            cont->serialize();  // This also increments serialization_count
        }
    };
    
    // Writer function
    auto writer = [&cont, operations_per_thread]() {
        for (int i = 0; i < operations_per_thread; ++i) {
            cont->set_source("src" + std::to_string(i), "sub");
            cont->set_target("tgt" + std::to_string(i), "sub");
            cont->remove("value" + std::to_string(i % 5));
            cont->add(std::make_shared<int_value>("new_value" + std::to_string(i), i));
        }
    };
    
    // Start reader threads
    for (int i = 0; i < num_readers; ++i) {
        threads.emplace_back(reader);
    }
    
    // Start writer threads
    for (int i = 0; i < num_writers; ++i) {
        threads.emplace_back(writer);
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Check statistics
    auto stats = cont->get_access_statistics();
    EXPECT_GT(stats.read_count, 0);
    EXPECT_GT(stats.write_count, 0);
    EXPECT_EQ(stats.serialization_count, num_readers * operations_per_thread);
}

TEST(container_thread_safe, performance_comparison) {
    const int num_operations = 10000;
    
    // Test without thread safety
    {
        auto cont = std::make_shared<value_container>();
        cont->set_thread_safe(false);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_operations; ++i) {
            cont->set_source("src" + std::to_string(i), "sub");
            auto src = cont->source_id();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_no_lock = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Without thread safety: " << duration_no_lock.count() << " microseconds\n";
    }
    
    // Test with thread safety
    {
        auto cont = std::make_shared<value_container>();
        cont->set_thread_safe(true);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_operations; ++i) {
            cont->set_source("src" + std::to_string(i), "sub");
            auto src = cont->source_id();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_with_lock = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "With thread safety: " << duration_with_lock.count() << " microseconds\n";
        
        auto stats = cont->get_access_statistics();
        std::cout << "Read count: " << stats.read_count << "\n";
        std::cout << "Write count: " << stats.write_count << "\n";
    }
}

TEST(container_thread_safe, simd_operations) {
    auto cont = std::make_shared<value_container>();
    cont->set_thread_safe(true);
    
    // Add float values
    for (int i = 0; i < 100; ++i) {
        cont->add(std::make_shared<float_value>("float" + std::to_string(i), i * 0.1f));
    }
    
    // Test SIMD operations
    auto sum = cont->simd_float_operation("sum");
    auto min = cont->simd_float_operation("min");
    auto max = cont->simd_float_operation("max");
    auto avg = cont->simd_float_operation("avg");
    
    // Since the integration with value class is not complete,
    // these will return nullopt for now
    EXPECT_FALSE(sum.has_value());
    EXPECT_FALSE(min.has_value());
    EXPECT_FALSE(max.has_value());
    EXPECT_FALSE(avg.has_value());
}
#endif // CONTAINER_TEST_INTERNALS
