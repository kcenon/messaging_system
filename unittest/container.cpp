#include "gtest/gtest.h"

#include <limits.h>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>

// #include "converting.h" - Module not available yet

#include "container/core/container.h"
#include "container/values/bool_value.h"
#include "container/values/container_value.h"
#include "container/values/numeric_value.h"
#include "container/values/string_value.h"
#include "container/core/value_types.h"

using namespace container_module;
// using namespace converting; - Namespace not available yet

// Temporary replacement for converter functions
namespace test_utils {
    std::string to_string(const std::string& str) {
        return str;
    }
    
    std::string to_string(const std::vector<uint8_t>& data) {
        return std::string(data.begin(), data.end());
    }
}

TEST(container, basic_functionality)
{
	value_container data;
	data.add(std::make_shared<bool_value>("false_value", false));
	data.add(std::make_shared<bool_value>("true_value", true));
	data.add(std::make_shared<float_value>("float_value", (float)1.234567890123456789));
	data.add(std::make_shared<double_value>("double_value", (double)1.234567890123456789));

	// Test serialize-deserialize round trip
	std::string data_serialized = data.serialize();
	value_container data2(data_serialized);

	EXPECT_STREQ(data_serialized.c_str(), data2.serialize().c_str());

	// Add more values to data2
	data2.add(std::make_shared<long_value>("long_value", LONG_MAX));
	data2.add(std::make_shared<ulong_value>("ulong_value", ULONG_MAX));
	data2.add(std::make_shared<llong_value>("llong_value", LLONG_MAX));
	data2.add(std::make_shared<ullong_value>("ullong_value", ULLONG_MAX));

	EXPECT_STRNE(data.serialize().c_str(), data2.serialize().c_str());

	// Create data3 from serialized data2 instead of copy constructor
	std::string data2_serialized = data2.serialize();
	value_container data3(data2_serialized);
	data3.remove("long_value");
	data3.remove("ulong_value");
	data3.remove("llong_value");
	data3.remove("ullong_value");

	// Compare serialized strings
	EXPECT_STREQ(data.serialize().c_str(), data3.serialize().c_str());
}

// Phase 1: Move semantics tests
TEST(container, move_constructor)
{
	value_container original;
	original.set_source("source1", "sub1");
	original.set_target("target1", "sub1");
	original.set_message_type("test_message");
	original.add(std::make_shared<int_value>("test_int", 42));
	original.add(std::make_shared<string_value>("test_string", "hello"));
	
	// Save original data for comparison
	std::string original_data = original.serialize();
	
	// Move construct
	value_container moved(std::move(original));
	
	// Verify moved object has the data
	EXPECT_EQ(moved.source_id(), "source1");
	EXPECT_EQ(moved.source_sub_id(), "sub1");
	EXPECT_EQ(moved.target_id(), "target1");
	EXPECT_EQ(moved.target_sub_id(), "sub1");
	EXPECT_EQ(moved.message_type(), "test_message");
	EXPECT_EQ(moved.serialize(), original_data);
	
	// Verify values were moved
	auto values = moved.value_array("test_int");
	EXPECT_EQ(values.size(), 1);
	EXPECT_EQ(values[0]->to_int(), 42);
}

TEST(container, move_assignment)
{
	value_container original;
	original.set_source("source2", "sub2");
	original.set_target("target2", "sub2");
	original.add(std::make_shared<double_value>("test_double", 3.14));
	
	value_container target;
	target.add(std::make_shared<int_value>("old_value", 99));
	
	// Save original data
	std::string original_data = original.serialize();
	
	// Move assign
	target = std::move(original);
	
	// Verify target has original's data
	EXPECT_EQ(target.source_id(), "source2");
	EXPECT_EQ(target.source_sub_id(), "sub2");
	EXPECT_EQ(target.serialize(), original_data);
	
	// Verify old value is gone
	auto old_values = target.value_array("old_value");
	EXPECT_EQ(old_values.size(), 0);
	
	// Verify new value exists
	auto new_values = target.value_array("test_double");
	EXPECT_EQ(new_values.size(), 1);
	EXPECT_DOUBLE_EQ(new_values[0]->to_double(), 3.14);
}

// Phase 1: string_view usage tests
TEST(container, string_view_parameters)
{
	value_container container;
	
	// Test set_source with string_view
	std::string source = "dynamic_source";
	std::string sub = "dynamic_sub";
	container.set_source(source, sub);
	EXPECT_EQ(container.source_id(), source);
	EXPECT_EQ(container.source_sub_id(), sub);
	
	// Test set_target with string literals (implicit string_view)
	container.set_target("literal_target", "literal_sub");
	EXPECT_EQ(container.target_id(), "literal_target");
	EXPECT_EQ(container.target_sub_id(), "literal_sub");
	
	// Test set_message_type with string_view
	container.set_message_type("test_message_type");
	EXPECT_EQ(container.message_type(), "test_message_type");
	
	// Test remove with string_view
	container.add(std::make_shared<int_value>("removable", 123));
	EXPECT_EQ(container.value_array("removable").size(), 1);
	container.remove("removable");
	EXPECT_EQ(container.value_array("removable").size(), 0);
	
	// Test value_array with string_view
	container.add(std::make_shared<string_value>("test_key", "test_value"));
	auto values = container.value_array("test_key");
	EXPECT_EQ(values.size(), 1);
	
	// Test get_value with string_view
	auto value = container.get_value("test_key", 0);
	EXPECT_EQ(value->to_string(), "test_value");
}

// Phase 1: constexpr type mapping tests
TEST(value_types, constexpr_type_conversion)
{
	// Test string to type conversion
	EXPECT_EQ(container_module::get_type_from_string("1"), value_types::bool_value);
	EXPECT_EQ(container_module::get_type_from_string("2"), value_types::short_value);
	EXPECT_EQ(container_module::get_type_from_string("3"), value_types::ushort_value);
	EXPECT_EQ(container_module::get_type_from_string("4"), value_types::int_value);
	EXPECT_EQ(container_module::get_type_from_string("5"), value_types::uint_value);
	EXPECT_EQ(container_module::get_type_from_string("6"), value_types::long_value);
	EXPECT_EQ(container_module::get_type_from_string("7"), value_types::ulong_value);
	EXPECT_EQ(container_module::get_type_from_string("8"), value_types::llong_value);
	EXPECT_EQ(container_module::get_type_from_string("9"), value_types::ullong_value);
	EXPECT_EQ(container_module::get_type_from_string("10"), value_types::float_value);
	EXPECT_EQ(container_module::get_type_from_string("11"), value_types::double_value);
	EXPECT_EQ(container_module::get_type_from_string("12"), value_types::bytes_value);
	EXPECT_EQ(container_module::get_type_from_string("13"), value_types::string_value);
	EXPECT_EQ(container_module::get_type_from_string("14"), value_types::container_value);
	EXPECT_EQ(container_module::get_type_from_string("100"), value_types::null_value); // unknown
	
	// Test type to string conversion
	EXPECT_EQ(container_module::get_string_from_type(value_types::bool_value), "1");
	EXPECT_EQ(container_module::get_string_from_type(value_types::short_value), "2");
	EXPECT_EQ(container_module::get_string_from_type(value_types::int_value), "4");
	EXPECT_EQ(container_module::get_string_from_type(value_types::double_value), "11");
	EXPECT_EQ(container_module::get_string_from_type(value_types::bytes_value), "12");
	EXPECT_EQ(container_module::get_string_from_type(value_types::string_value), "13");
	EXPECT_EQ(container_module::get_string_from_type(value_types::container_value), "14");
	EXPECT_EQ(container_module::get_string_from_type(value_types::null_value), "0");
}

// Performance comparison test
TEST(container, performance_move_vs_copy)
{
	const int iterations = 1000;
	
	// Create a large container
	value_container source;
	for (int i = 0; i < 100; ++i)
	{
		source.add(std::make_shared<int_value>("int_" + std::to_string(i), i));
		source.add(std::make_shared<double_value>("double_" + std::to_string(i), i * 3.14));
		source.add(std::make_shared<string_value>("string_" + std::to_string(i), 
			"This is a test string number " + std::to_string(i)));
	}
	
	// Measure copy performance
	auto copy_start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < iterations; ++i)
	{
		value_container copied(source);
	}
	auto copy_end = std::chrono::high_resolution_clock::now();
	auto copy_duration = std::chrono::duration_cast<std::chrono::microseconds>(copy_end - copy_start);
	
	// Measure move performance
	auto move_start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < iterations; ++i)
	{
		value_container temp(source);
		value_container moved(std::move(temp));
	}
	auto move_end = std::chrono::high_resolution_clock::now();
	auto move_duration = std::chrono::duration_cast<std::chrono::microseconds>(move_end - move_start);
	
	// Move should be faster than copy
	EXPECT_LT(move_duration.count(), copy_duration.count());
	
	// Log performance results
	std::cout << "Copy duration: " << copy_duration.count() << " microseconds\n";
	std::cout << "Move duration: " << move_duration.count() << " microseconds\n";
	std::cout << "Performance improvement: " 
			  << (double)copy_duration.count() / move_duration.count() 
			  << "x faster\n";
}

// Thread safety basic test (for future enhancement)
TEST(container, concurrent_read_safety)
{
	value_container container;
	for (int i = 0; i < 100; ++i)
	{
		container.add(std::make_shared<int_value>("value_" + std::to_string(i), i));
	}
	
	const int num_threads = 4;
	const int reads_per_thread = 1000;
	std::vector<std::thread> threads;
	std::atomic<int> success_count{0};
	
	// Multiple threads reading concurrently
	for (int t = 0; t < num_threads; ++t)
	{
		threads.emplace_back([&container, &success_count]() {
			for (int i = 0; i < reads_per_thread; ++i)
			{
				int idx = i % 100;
				auto value = container.get_value("value_" + std::to_string(idx), 0);
				if (value->to_int() == idx)
				{
					success_count++;
				}
			}
		});
	}
	
	// Wait for all threads
	for (auto& thread : threads)
	{
		thread.join();
	}
	
	// All reads should be successful
	EXPECT_EQ(success_count.load(), num_threads * reads_per_thread);
}