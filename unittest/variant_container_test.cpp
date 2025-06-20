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
// Internal implementation testing - not part of public API
#ifdef CONTAINER_TEST_INTERNALS
#include "container/internal/variant_value.h"
#include "container/internal/thread_safe_container.h"
#include "container/internal/simd_processor.h"
#endif
#include <thread>
#include <vector>
#include <chrono>
#include <random>

#ifdef CONTAINER_TEST_INTERNALS
using namespace container_module;
using namespace container_module::simd;
using namespace std::chrono_literals;

// Test variant_value type safety
TEST(variant_value, type_safety)
{
    // Test null value
    variant_value null_val("null_test");
    EXPECT_TRUE(null_val.is_null());
    EXPECT_EQ(null_val.type_index(), 0);
    
    // Test bool value
    variant_value bool_val("bool_test", true);
    EXPECT_FALSE(bool_val.is_null());
    EXPECT_EQ(bool_val.type_index(), 1);
    EXPECT_EQ(bool_val.get<bool>().value_or(false), true);
    EXPECT_FALSE(bool_val.get<int>().has_value());
    
    // Test numeric values
    variant_value int_val("int_test", int32_t(42));
    EXPECT_EQ(int_val.get<int32_t>().value_or(0), 42);
    EXPECT_EQ(int_val.type_index(), 5);
    
    variant_value float_val("float_test", 3.14159f);
    EXPECT_FLOAT_EQ(float_val.get<float>().value_or(0.0f), 3.14159f);
    
    // Test string value
    variant_value str_val("str_test", std::string("Hello, World!"));
    EXPECT_EQ(str_val.get<std::string>().value_or(""), "Hello, World!");
    
    // Test bytes value
    std::vector<uint8_t> bytes = {0x01, 0x02, 0x03, 0x04};
    variant_value bytes_val("bytes_test", bytes);
    auto retrieved_bytes = bytes_val.get<std::vector<uint8_t>>();
    ASSERT_TRUE(retrieved_bytes.has_value());
    EXPECT_EQ(retrieved_bytes.value(), bytes);
}

// Test variant_value visitor pattern
TEST(variant_value, visitor_pattern)
{
    variant_value val("test", 42.0);
    
    bool visited = false;
    val.visit([&visited](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, double>) {
            EXPECT_DOUBLE_EQ(value, 42.0);
            visited = true;
        }
    });
    
    EXPECT_TRUE(visited);
    
    // Test visitor with return value
    auto result = val.visit([](auto&& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, double>) {
            return "double: " + std::to_string(value);
        }
        return "unknown";
    });
    
    EXPECT_EQ(result, "double: 42.000000");
}

// Test thread-safe container basic operations
TEST(thread_safe_container, basic_operations)
{
    thread_safe_container container;
    
    // Test set and get
    container.set("key1", 42);
    container.set("key2", std::string("value2"));
    container.set("key3", 3.14159);
    
    EXPECT_EQ(container.size(), 3);
    EXPECT_FALSE(container.empty());
    
    // Test typed get
    auto int_val = container.get_typed<int32_t>("key1");
    ASSERT_TRUE(int_val.has_value());
    EXPECT_EQ(*int_val, 42);
    
    auto str_val = container.get_typed<std::string>("key2");
    ASSERT_TRUE(str_val.has_value());
    EXPECT_EQ(*str_val, "value2");
    
    auto double_val = container.get_typed<double>("key3");
    ASSERT_TRUE(double_val.has_value());
    EXPECT_DOUBLE_EQ(*double_val, 3.14159);
    
    // Test contains
    EXPECT_TRUE(container.contains("key1"));
    EXPECT_FALSE(container.contains("key4"));
    
    // Test remove
    EXPECT_TRUE(container.remove("key2"));
    EXPECT_FALSE(container.remove("key4"));
    EXPECT_EQ(container.size(), 2);
    
    // Test clear
    container.clear();
    EXPECT_TRUE(container.empty());
    EXPECT_EQ(container.size(), 0);
}

// Test thread-safe container concurrent access
TEST(thread_safe_container, concurrent_access)
{
    auto container = std::make_shared<thread_safe_container>();
    const int num_threads = 8;
    const int operations_per_thread = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<int> successful_writes{0};
    std::atomic<int> successful_reads{0};
    
    // Writer threads
    for (int t = 0; t < num_threads / 2; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                std::string key = "thread_" + std::to_string(t) + "_" + std::to_string(i);
                container->set(key, t * operations_per_thread + i);
                successful_writes.fetch_add(1);
            }
        });
    }
    
    // Reader threads
    for (int t = num_threads / 2; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                // Try to read random keys
                int thread_id = i % (num_threads / 2);
                int key_id = i % operations_per_thread;
                std::string key = "thread_" + std::to_string(thread_id) + "_" + std::to_string(key_id);
                
                if (container->get(key).has_value()) {
                    successful_reads.fetch_add(1);
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify results
    EXPECT_EQ(successful_writes.load(), (num_threads / 2) * operations_per_thread);
    EXPECT_GT(successful_reads.load(), 0);
    
    // Check statistics
    auto stats = container->get_statistics();
    EXPECT_GT(stats.read_count, 0);
    EXPECT_GT(stats.write_count, 0);
}

// Test SIMD operations
TEST(simd_processor, float_operations)
{
    std::vector<variant_value> values;
    std::vector<float> expected_floats;
    
    // Create test data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    
    for (int i = 0; i < 1000; ++i) {
        float val = dis(gen);
        values.emplace_back("float_" + std::to_string(i), val);
        expected_floats.push_back(val);
    }
    
    // Test sum
    float expected_sum = std::accumulate(expected_floats.begin(), 
                                        expected_floats.end(), 0.0f);
    float simd_sum = simd_processor::sum_floats(values);
    EXPECT_NEAR(simd_sum, expected_sum, 0.01f);
    
    // Test min
    float expected_min = *std::min_element(expected_floats.begin(), 
                                          expected_floats.end());
    auto simd_min = simd_processor::min_float(values);
    ASSERT_TRUE(simd_min.has_value());
    EXPECT_FLOAT_EQ(*simd_min, expected_min);
    
    // Test max
    float expected_max = *std::max_element(expected_floats.begin(), 
                                          expected_floats.end());
    auto simd_max = simd_processor::max_float(values);
    ASSERT_TRUE(simd_max.has_value());
    EXPECT_FLOAT_EQ(*simd_max, expected_max);
}

// Test SIMD performance vs scalar
TEST(simd_processor, performance_comparison)
{
    const int data_size = 1000000;
    std::vector<variant_value> values;
    
    // Create large dataset
    for (int i = 0; i < data_size; ++i) {
        values.emplace_back("float_" + std::to_string(i), 
                           static_cast<float>(i % 1000) / 100.0f);
    }
    
    // Time SIMD operation
    auto simd_start = std::chrono::high_resolution_clock::now();
    float simd_sum = simd_processor::sum_floats(values);
    auto simd_end = std::chrono::high_resolution_clock::now();
    
    // Time scalar operation
    auto scalar_start = std::chrono::high_resolution_clock::now();
    float scalar_sum = 0.0f;
    for (const auto& val : values) {
        if (auto f = val.get<float>()) {
            scalar_sum += *f;
        }
    }
    auto scalar_end = std::chrono::high_resolution_clock::now();
    
    // Compare results
    EXPECT_NEAR(simd_sum, scalar_sum, 0.1f);
    
    // Print performance info
    auto simd_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        simd_end - simd_start);
    auto scalar_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        scalar_end - scalar_start);
    
    std::cout << "SIMD sum: " << simd_duration.count() << " microseconds\n";
    std::cout << "Scalar sum: " << scalar_duration.count() << " microseconds\n";
    std::cout << "Speedup: " << static_cast<double>(scalar_duration.count()) / 
                                simd_duration.count() << "x\n";
}

// Test serialization and deserialization
TEST(variant_value, serialization)
{
    // Test various types
    variant_value int_val("int", int32_t(42));
    variant_value float_val("float", 3.14159f);
    variant_value str_val("string", std::string("Hello"));
    variant_value bytes_val("bytes", std::vector<uint8_t>{1, 2, 3, 4});
    
    // Serialize
    auto int_data = int_val.serialize();
    auto float_data = float_val.serialize();
    auto str_data = str_val.serialize();
    auto bytes_data = bytes_val.serialize();
    
    // Deserialize
    auto int_restored = variant_value::deserialize(int_data);
    auto float_restored = variant_value::deserialize(float_data);
    auto str_restored = variant_value::deserialize(str_data);
    auto bytes_restored = variant_value::deserialize(bytes_data);
    
    // Verify
    ASSERT_TRUE(int_restored.has_value());
    EXPECT_EQ(int_restored->name(), "int");
    EXPECT_EQ(int_restored->get<int32_t>().value_or(0), 42);
    
    ASSERT_TRUE(float_restored.has_value());
    EXPECT_EQ(float_restored->name(), "float");
    EXPECT_FLOAT_EQ(float_restored->get<float>().value_or(0.0f), 3.14159f);
    
    ASSERT_TRUE(str_restored.has_value());
    EXPECT_EQ(str_restored->name(), "string");
    EXPECT_EQ(str_restored->get<std::string>().value_or(""), "Hello");
    
    ASSERT_TRUE(bytes_restored.has_value());
    EXPECT_EQ(bytes_restored->name(), "bytes");
    auto restored_bytes = bytes_restored->get<std::vector<uint8_t>>();
    ASSERT_TRUE(restored_bytes.has_value());
    EXPECT_EQ(*restored_bytes, (std::vector<uint8_t>{1, 2, 3, 4}));
}

// Test lockfree reader
TEST(lockfree_reader, basic_operations)
{
    auto container = std::make_shared<thread_safe_container>();
    
    // Populate container
    container->set("key1", 42);
    container->set("key2", std::string("value"));
    container->set("key3", 3.14);
    
    // Create lockfree reader
    lockfree_reader reader(container);
    
    // Test reads
    auto val1 = reader.get<int32_t>("key1");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(*val1, 42);
    
    auto val2 = reader.get<std::string>("key2");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(*val2, "value");
    
    // Update container
    container->set("key1", 100);
    
    // Old snapshot still has old value
    auto old_val = reader.get<int32_t>("key1");
    ASSERT_TRUE(old_val.has_value());
    EXPECT_EQ(*old_val, 42);
    
    // Update snapshot
    reader.update_snapshot();
    
    // Now has new value
    auto new_val = reader.get<int32_t>("key1");
    ASSERT_TRUE(new_val.has_value());
    EXPECT_EQ(*new_val, 100);
}

// Test SIMD info
TEST(simd_support, platform_info)
{
    std::cout << simd_support::get_simd_info() << std::endl;
    std::cout << "Optimal SIMD width: " << simd_support::get_optimal_width() << std::endl;
    
    // These tests will pass/fail based on platform
    std::cout << "SSE2 support: " << (simd_support::has_sse2() ? "Yes" : "No") << std::endl;
    std::cout << "SSE4.2 support: " << (simd_support::has_sse42() ? "Yes" : "No") << std::endl;
    std::cout << "AVX2 support: " << (simd_support::has_avx2() ? "Yes" : "No") << std::endl;
    std::cout << "NEON support: " << (simd_support::has_neon() ? "Yes" : "No") << std::endl;
}
#endif // CONTAINER_TEST_INTERNALS
