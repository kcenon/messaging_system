/**
 * @file unit_tests.cpp
 * @brief Comprehensive unit tests for the container system
 * 
 * Tests cover:
 * - Value type creation and conversion
 * - Container serialization/deserialization
 * - Thread safety
 * - Memory management
 * - Error handling
 */

#include <gtest/gtest.h>
#include <container.h>
#include <values/bool_value.h>
#include <values/string_value.h>
#include <values/bytes_value.h>
#include <values/numeric_value.h>
#include <internal/thread_safe_container.h>
#include <thread>
#include <vector>
#include <random>
#include <sstream>
#include <chrono>

using namespace container_module;

// Test fixture for value tests
class ValueTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for value tests
    }
    
    void TearDown() override {
        // Cleanup
    }
};

// Test fixture for container tests
class ContainerTest : public ::testing::Test {
protected:
    std::unique_ptr<value_container> container;
    
    void SetUp() override {
        container = std::make_unique<value_container>();
    }
    
    void TearDown() override {
        container.reset();
    }
};

// ============================================================================
// Value Type Tests
// ============================================================================

TEST_F(ValueTest, NullValueCreation) {
    auto null_val = std::make_shared<value>("test_null", value_types::null_value, "");
    
    EXPECT_EQ(null_val->name(), "test_null");
    EXPECT_EQ(null_val->type(), value_types::null_value);
    EXPECT_TRUE(null_val->is_null());
    EXPECT_FALSE(null_val->is_boolean());
    EXPECT_FALSE(null_val->is_numeric());
    EXPECT_FALSE(null_val->is_string());
    EXPECT_FALSE(null_val->is_container());
}

TEST_F(ValueTest, BooleanValueCreation) {
    // Test true value
    auto true_val = std::make_shared<bool_value>("test_bool", true);
    EXPECT_EQ(true_val->name(), "test_bool");
    EXPECT_EQ(true_val->type(), value_types::bool_value);
    EXPECT_TRUE(true_val->is_boolean());
    EXPECT_TRUE(true_val->to_boolean());
    
    // Test false value
    auto false_val = std::make_shared<bool_value>("test_bool2", false);
    EXPECT_FALSE(false_val->to_boolean());
    
    // Test string boolean
    auto str_true_val = std::make_shared<bool_value>("test_bool3", "true");
    EXPECT_TRUE(str_true_val->to_boolean());
    
    auto str_false_val = std::make_shared<bool_value>("test_bool4", "false");
    EXPECT_FALSE(str_false_val->to_boolean());
}

TEST_F(ValueTest, NumericValueCreation) {
    // Test int
    auto int_val = std::make_shared<int_value>("test_int", 42);
    EXPECT_EQ(int_val->type(), value_types::int_value);
    EXPECT_TRUE(int_val->is_numeric());
    EXPECT_EQ(int_val->to_int(), 42);
    EXPECT_EQ(int_val->to_long(), 42L);
    EXPECT_DOUBLE_EQ(int_val->to_double(), 42.0);
    
    // Test long long
    auto llong_val = std::make_shared<llong_value>("test_llong", 9223372036854775807LL);
    EXPECT_EQ(llong_val->to_llong(), 9223372036854775807LL);
    
    // Test double
    auto double_val = std::make_shared<double_value>("test_double", 3.14159);
    EXPECT_DOUBLE_EQ(double_val->to_double(), 3.14159);
    
    // Test negative values
    auto neg_val = std::make_shared<int_value>("test_neg", -100);
    EXPECT_EQ(neg_val->to_int(), -100);
}

TEST_F(ValueTest, StringValueCreation) {
    auto str_val = std::make_shared<string_value>("test_string", "Hello, World!");
    
    EXPECT_EQ(str_val->type(), value_types::string_value);
    EXPECT_TRUE(str_val->is_string());
    EXPECT_EQ(str_val->to_string(), "Hello, World!");
    EXPECT_EQ(str_val->size(), 13); // Length of "Hello, World!"
}

TEST_F(ValueTest, BytesValueCreation) {
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0xFF};
    
    auto bytes_val = std::make_shared<bytes_value>("test_bytes", test_data);
    
    EXPECT_EQ(bytes_val->type(), value_types::bytes_value);
    EXPECT_TRUE(bytes_val->is_bytes());
    
    auto retrieved_bytes = bytes_val->to_bytes();
    EXPECT_EQ(retrieved_bytes.size(), test_data.size());
    EXPECT_EQ(retrieved_bytes, test_data);
}

TEST_F(ValueTest, ValueTypeSerialization) {
    // Test each value type serialization
    auto bool_val = std::make_shared<bool_value>("bool", true);
    auto int_val = std::make_shared<int_value>("int", 42);
    auto str_val = std::make_shared<string_value>("str", "test");
    
    // Serialize values
    std::string bool_ser = bool_val->serialize();
    std::string int_ser = int_val->serialize();
    std::string str_ser = str_val->serialize();
    
    // Check serialization format (may vary based on implementation)
    EXPECT_FALSE(bool_ser.empty());
    EXPECT_FALSE(int_ser.empty());
    EXPECT_FALSE(str_ser.empty());
}

// ============================================================================
// Container Tests
// ============================================================================

TEST_F(ContainerTest, BasicContainerCreation) {
    container->set_source("test_source", "sub1");
    container->set_target("test_target", "sub2");
    container->set_message_type("test_message");
    
    EXPECT_EQ(container->source_id(), "test_source");
    EXPECT_EQ(container->source_sub_id(), "sub1");
    EXPECT_EQ(container->target_id(), "test_target");
    EXPECT_EQ(container->target_sub_id(), "sub2");
    EXPECT_EQ(container->message_type(), "test_message");
}

TEST_F(ContainerTest, ContainerValueManagement) {
    // Add various values
    container->add(std::make_shared<string_value>("key1", "value1"));
    container->add(std::make_shared<int_value>("key2", 100));
    container->add(std::make_shared<bool_value>("key3", true));
    
    // Retrieve values
    auto val1 = container->get_value("key1");
    auto val2 = container->get_value("key2");
    auto val3 = container->get_value("key3");
    
    EXPECT_EQ(val1->to_string(), "value1");
    EXPECT_EQ(val2->to_int(), 100);
    EXPECT_TRUE(val3->to_boolean());
    
    // Test non-existent key
    auto val4 = container->get_value("non_existent");
    EXPECT_TRUE(val4->is_null());
}

TEST_F(ContainerTest, ContainerSerialization) {
    // Setup container
    container->set_source("src", "sub");
    container->set_target("tgt", "");
    container->set_message_type("test");
    
    // Add values
    container->add(std::make_shared<string_value>("str", "hello"));
    container->add(std::make_shared<int_value>("num", 42));
    
    // Serialize
    std::string serialized = container->serialize();
    
    // Deserialize
    auto new_container = std::make_unique<value_container>(serialized);
    
    // Verify
    EXPECT_EQ(new_container->source_id(), "src");
    EXPECT_EQ(new_container->source_sub_id(), "sub");
    EXPECT_EQ(new_container->target_id(), "tgt");
    EXPECT_EQ(new_container->message_type(), "test");
    
    EXPECT_EQ(new_container->get_value("str")->to_string(), "hello");
    EXPECT_EQ(new_container->get_value("num")->to_int(), 42);
}

TEST_F(ContainerTest, NestedContainerSupport) {
    // Create nested container
    auto nested = std::make_unique<value_container>();
    nested->set_message_type("nested_msg");
    nested->add(std::make_shared<string_value>("nested_key", "nested_value"));
    
    // Serialize nested container
    std::string nested_data = nested->serialize();
    
    // Add to main container as container value
    container->add(std::make_shared<value>("child", value_types::container_value, nested_data));
    
    // Retrieve nested container
    auto child_val = container->get_value("child");
    EXPECT_TRUE(child_val->is_container());
    
    // Parse nested container
    auto child_container = std::make_unique<value_container>(child_val->data());
    EXPECT_EQ(child_container->message_type(), "nested_msg");
    EXPECT_EQ(child_container->get_value("nested_key")->to_string(), "nested_value");
}

TEST_F(ContainerTest, ContainerHeaderSwap) {
    container->set_source("src", "sub1");
    container->set_target("tgt", "sub2");
    
    container->swap_header();
    
    EXPECT_EQ(container->source_id(), "tgt");
    EXPECT_EQ(container->source_sub_id(), "sub2");
    EXPECT_EQ(container->target_id(), "src");
    EXPECT_EQ(container->target_sub_id(), "sub1");
}

TEST_F(ContainerTest, MultipleValuesWithSameName) {
    // Add multiple values with same name
    container->add(std::make_shared<string_value>("item", "first"));
    container->add(std::make_shared<string_value>("item", "second"));
    container->add(std::make_shared<string_value>("item", "third"));
    
    // Get all values
    auto items = container->value_array("item");
    EXPECT_EQ(items.size(), 3);
    EXPECT_EQ(items[0]->to_string(), "first");
    EXPECT_EQ(items[1]->to_string(), "second");
    EXPECT_EQ(items[2]->to_string(), "third");
    
    // Get specific index
    EXPECT_EQ(container->get_value("item", 0)->to_string(), "first");
    EXPECT_EQ(container->get_value("item", 1)->to_string(), "second");
    EXPECT_EQ(container->get_value("item", 2)->to_string(), "third");
}

TEST_F(ContainerTest, ContainerCopy) {
    // Setup original
    container->set_message_type("original");
    container->add(std::make_shared<string_value>("key", "value"));
    
    // Deep copy
    auto copy = container->copy(true);
    EXPECT_EQ(copy->message_type(), "original");
    EXPECT_EQ(copy->get_value("key")->to_string(), "value");
    
    // Shallow copy (header only)
    auto shallow = container->copy(false);
    EXPECT_EQ(shallow->message_type(), "original");
    auto val = shallow->get_value("key");
    EXPECT_TRUE(val->is_null()); // No values in shallow copy
}

TEST_F(ContainerTest, LargeDataHandling) {
    // Create large string
    std::string large_data(1024 * 1024, 'X'); // 1MB of X's
    
    container->add(std::make_shared<string_value>("large", large_data));
    
    // Serialize and deserialize
    std::string serialized = container->serialize();
    auto restored = std::make_unique<value_container>(serialized);
    
    EXPECT_EQ(restored->get_value("large")->to_string(), large_data);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(ThreadSafetyTest, ConcurrentReads) {
    auto container = std::make_unique<value_container>();
    
    // Add test data
    for (int i = 0; i < 100; ++i) {
        container->add(std::make_shared<int_value>(
            "key" + std::to_string(i), 
            i
        ));
    }
    
    // Serialize once
    std::string serialized = container->serialize();
    
    // Multiple threads reading
    const int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> success_count(0);
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&container, &success_count]() {
            for (int i = 0; i < 100; ++i) {
                auto val = container->get_value("key" + std::to_string(i));
                if (val->to_int() == i) {
                    success_count++;
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(success_count, num_threads * 100);
}

TEST(ThreadSafetyTest, ThreadSafeContainer) {
    auto safe_container = std::make_shared<thread_safe_container>();
    
    const int num_threads = 5;
    const int ops_per_thread = 100;
    std::vector<std::thread> threads;
    
    // Writers
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([safe_container, t, ops_per_thread]() {
            for (int i = 0; i < ops_per_thread; ++i) {
                std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
                int value = t * 1000 + i;
                safe_container->set(key, value);
            }
        });
    }
    
    // Readers
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([safe_container, t, ops_per_thread]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Let writers start
            
            for (int i = 0; i < ops_per_thread; ++i) {
                std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
                auto val = safe_container->get_typed<int>(key);
                // Value might not exist yet, that's OK
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify all values were added
    for (int t = 0; t < num_threads; ++t) {
        for (int i = 0; i < ops_per_thread; ++i) {
            std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
            auto val = safe_container->get_typed<int>(key);
            EXPECT_TRUE(val.has_value());
            EXPECT_EQ(val.value(), t * 1000 + i);
        }
    }
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST(ErrorHandlingTest, InvalidSerialization) {
    // Test invalid serialization data
    EXPECT_THROW(value_container("invalid data"), std::exception);
    EXPECT_THROW(value_container("@header={};@data={[invalid];"), std::exception);
}

TEST(ErrorHandlingTest, TypeConversionErrors) {
    auto str_val = std::make_shared<string_value>("test", "not_a_number");
    
    // String to int conversion should handle gracefully
    EXPECT_EQ(str_val->to_int(), 0); // Default value for failed conversion
}

TEST(ErrorHandlingTest, NullValueConversions) {
    auto null_val = std::make_shared<value>("null", value_types::null_value, "");
    
    // Null conversions should throw
    EXPECT_THROW(null_val->to_boolean(), std::logic_error);
    EXPECT_THROW(null_val->to_int(), std::logic_error);
    EXPECT_THROW(null_val->to_double(), std::logic_error);
    EXPECT_THROW(null_val->to_string(), std::logic_error);
}

// ============================================================================
// Performance Tests (Simple Benchmarks)
// ============================================================================

TEST(PerformanceTest, SerializationSpeed) {
    auto container = std::make_unique<value_container>();
    
    // Add 1000 values
    for (int i = 0; i < 1000; ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i),
            "value" + std::to_string(i)
        ));
    }
    
    // Measure serialization time
    auto start = std::chrono::high_resolution_clock::now();
    std::string serialized = container->serialize();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Log performance
    std::cout << "Serialization of 1000 values: " << duration.count() << " microseconds" << std::endl;
    std::cout << "Serialized size: " << serialized.size() << " bytes" << std::endl;
    
    // Basic performance expectation (should be fast)
    EXPECT_LT(duration.count(), 10000); // Less than 10ms
}

TEST(PerformanceTest, DeserializationSpeed) {
    // Create and serialize container
    auto container = std::make_unique<value_container>();
    for (int i = 0; i < 1000; ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i),
            "value" + std::to_string(i)
        ));
    }
    std::string serialized = container->serialize();
    
    // Measure deserialization time
    auto start = std::chrono::high_resolution_clock::now();
    auto restored = std::make_unique<value_container>(serialized);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Deserialization of 1000 values: " << duration.count() << " microseconds" << std::endl;
    
    EXPECT_LT(duration.count(), 10000); // Less than 10ms
}

// ============================================================================
// Special Cases and Edge Cases
// ============================================================================

TEST(EdgeCaseTest, EmptyContainer) {
    auto container = std::make_unique<value_container>();
    
    // Serialize empty container
    std::string serialized = container->serialize();
    
    // Deserialize
    auto restored = std::make_unique<value_container>(serialized);
    
    // Should have default values
    EXPECT_EQ(restored->source_id(), "");
    EXPECT_EQ(restored->message_type(), "data_container");
}

TEST(EdgeCaseTest, SpecialCharacters) {
    auto container = std::make_unique<value_container>();
    
    // Test special characters in values
    std::string special = "Line1\nLine2\rLine3\tTab\0Null";
    container->add(std::make_shared<string_value>("special", special));
    
    // Test special characters in names (use simpler keys)
    container->add(std::make_shared<string_value>("key_with_underscores", "value1"));
    container->add(std::make_shared<string_value>("keyWithCamelCase", "value2"));
    
    // Serialize and restore
    std::string serialized = container->serialize();
    auto restored = std::make_unique<value_container>(serialized);
    
    // Verify special characters preserved
    auto special_val = restored->get_value("special");
    EXPECT_NE(special_val->type(), value_types::null_value);
    if (special_val->type() != value_types::null_value) {
        EXPECT_EQ(special_val->to_string(), special);
    }
    
    auto underscore_val = restored->get_value("key_with_underscores");
    EXPECT_NE(underscore_val->type(), value_types::null_value);
    if (underscore_val->type() != value_types::null_value) {
        EXPECT_EQ(underscore_val->to_string(), "value1");
    }
    
    auto camel_val = restored->get_value("keyWithCamelCase");
    EXPECT_NE(camel_val->type(), value_types::null_value);
    if (camel_val->type() != value_types::null_value) {
        EXPECT_EQ(camel_val->to_string(), "value2");
    }
}

TEST(EdgeCaseTest, MaximumValues) {
    // Test maximum numeric values
    auto max_int = std::make_shared<int_value>("max_int", std::numeric_limits<int>::max());
    auto min_int = std::make_shared<int_value>("min_int", std::numeric_limits<int>::min());
    auto max_llong = std::make_shared<llong_value>("max_llong", std::numeric_limits<long long>::max());
    
    EXPECT_EQ(max_int->to_int(), std::numeric_limits<int>::max());
    EXPECT_EQ(min_int->to_int(), std::numeric_limits<int>::min());
    EXPECT_EQ(max_llong->to_llong(), std::numeric_limits<long long>::max());
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}