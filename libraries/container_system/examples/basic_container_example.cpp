#include <iostream>
#include <memory>
#include <vector>
#include <chrono>

#include "container.h"

using namespace container_module;

/**
 * @brief Basic Container System Example
 *
 * This example demonstrates fundamental usage of the container system:
 * - Creating containers
 * - Adding different types of values
 * - Serialization and deserialization
 * - Basic error handling
 */

void demonstrate_basic_usage() {
    std::cout << "=== Basic Container Usage ===" << std::endl;

    // Create a new container
    auto container = std::make_shared<value_container>();

    // Set container metadata
    container->set_source("example_client", "session_001");
    container->set_target("example_server", "main_handler");
    container->set_message_type("user_data");

    std::cout << "Container created with:" << std::endl;
    std::cout << "  Source: " << container->get_source_id() << "/" << container->get_source_sub_id() << std::endl;
    std::cout << "  Target: " << container->get_target_id() << "/" << container->get_target_sub_id() << std::endl;
    std::cout << "  Type: " << container->get_message_type() << std::endl;
}

void demonstrate_value_types() {
    std::cout << "\n=== Value Types Demonstration ===" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("value_types_demo");

    // String value
    auto string_val = std::make_shared<string_value>("username", "john_doe");
    container->add_value(string_val);
    std::cout << "Added string value: " << string_val->get_key() << " = " << string_val->get_value() << std::endl;

    // Integer value
    auto int_val = std::make_shared<int_value>("user_id", 12345);
    container->add_value(int_val);
    std::cout << "Added int value: " << int_val->get_key() << " = " << int_val->get_value() << std::endl;

    // Long value
    auto long_val = std::make_shared<long_value>("timestamp",
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    container->add_value(long_val);
    std::cout << "Added long value: " << long_val->get_key() << " = " << long_val->get_value() << std::endl;

    // Float value
    auto float_val = std::make_shared<float_value>("score", 98.5f);
    container->add_value(float_val);
    std::cout << "Added float value: " << float_val->get_key() << " = " << float_val->get_value() << std::endl;

    // Double value
    auto double_val = std::make_shared<double_value>("account_balance", 1500.75);
    container->add_value(double_val);
    std::cout << "Added double value: " << double_val->get_key() << " = " << double_val->get_value() << std::endl;

    // Boolean value
    auto bool_val = std::make_shared<bool_value>("is_active", true);
    container->add_value(bool_val);
    std::cout << "Added bool value: " << bool_val->get_key() << " = " <<
        (bool_val->get_value() ? "true" : "false") << std::endl;

    // Binary data value
    std::vector<uint8_t> binary_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello" in ASCII
    auto bytes_val = std::make_shared<bytes_value>("binary_data", binary_data);
    container->add_value(bytes_val);
    std::cout << "Added bytes value: " << bytes_val->get_key() << " (" <<
        binary_data.size() << " bytes)" << std::endl;

    std::cout << "Total values in container: " << container->get_values().size() << std::endl;
}

void demonstrate_nested_containers() {
    std::cout << "\n=== Nested Containers Demonstration ===" << std::endl;

    // Create main container
    auto main_container = std::make_shared<value_container>();
    main_container->set_source("client_app", "main_session");
    main_container->set_target("server_app", "data_processor");
    main_container->set_message_type("user_profile");

    // Add basic user data
    main_container->add_value(std::make_shared<string_value>("name", "Alice Smith"));
    main_container->add_value(std::make_shared<int_value>("age", 28));

    // Create nested container for preferences
    auto preferences_container = std::make_shared<value_container>();
    preferences_container->set_message_type("user_preferences");
    preferences_container->add_value(std::make_shared<string_value>("theme", "dark"));
    preferences_container->add_value(std::make_shared<bool_value>("notifications", true));
    preferences_container->add_value(std::make_shared<string_value>("language", "en-US"));

    // Add nested container to main container
    auto container_val = std::make_shared<container_value>("preferences", preferences_container);
    main_container->add_value(container_val);

    std::cout << "Created nested container structure:" << std::endl;
    std::cout << "  Main container values: " << main_container->get_values().size() << std::endl;
    std::cout << "  Nested container values: " << preferences_container->get_values().size() << std::endl;

    // Access nested values
    auto preferences_value = main_container->find_value("preferences");
    if (preferences_value && preferences_value->get_type() == container_value) {
        auto nested_container = std::static_pointer_cast<class container_value>(preferences_value)->get_value();
        auto theme_value = nested_container->find_value("theme");
        if (theme_value) {
            std::cout << "  Theme preference: " << theme_value->to_string() << std::endl;
        }
    }
}

void demonstrate_serialization() {
    std::cout << "\n=== Serialization Demonstration ===" << std::endl;

    // Create container with various data
    auto container = std::make_shared<value_container>();
    container->set_source("serialize_test", "test_session");
    container->set_target("deserialize_test", "test_handler");
    container->set_message_type("serialization_test");

    container->add_value(std::make_shared<string_value>("message", "Hello, Serialization!"));
    container->add_value(std::make_shared<int_value>("count", 42));
    container->add_value(std::make_shared<double_value>("pi", 3.14159));
    container->add_value(std::make_shared<bool_value>("success", true));

    // Serialize
    std::cout << "Serializing container..." << std::endl;
    std::string serialized_data = container->serialize();
    std::cout << "Serialized size: " << serialized_data.size() << " bytes" << std::endl;

    // Deserialize
    std::cout << "Deserializing container..." << std::endl;
    auto new_container = std::make_shared<value_container>();
    bool success = new_container->deserialize(serialized_data);

    if (success) {
        std::cout << "Deserialization successful!" << std::endl;
        std::cout << "Deserialized container:" << std::endl;
        std::cout << "  Source: " << new_container->get_source_id() << "/" << new_container->get_source_sub_id() << std::endl;
        std::cout << "  Target: " << new_container->get_target_id() << "/" << new_container->get_target_sub_id() << std::endl;
        std::cout << "  Type: " << new_container->get_message_type() << std::endl;
        std::cout << "  Values: " << new_container->get_values().size() << std::endl;

        // Verify specific values
        auto message_value = new_container->find_value("message");
        auto count_value = new_container->find_value("count");

        if (message_value) {
            std::cout << "  Message: " << message_value->to_string() << std::endl;
        }

        if (count_value && count_value->get_type() == int_value) {
            auto int_val = std::static_pointer_cast<class int_value>(count_value);
            std::cout << "  Count: " << int_val->get_value() << std::endl;
        }
    } else {
        std::cout << "Deserialization failed!" << std::endl;
    }
}

void demonstrate_value_access() {
    std::cout << "\n=== Value Access Demonstration ===" << std::endl;

    auto container = std::make_shared<value_container>();
    container->set_message_type("value_access_test");

    // Add sample data
    container->add_value(std::make_shared<string_value>("product_name", "Super Widget"));
    container->add_value(std::make_shared<double_value>("price", 29.99));
    container->add_value(std::make_shared<int_value>("quantity", 100));
    container->add_value(std::make_shared<bool_value>("in_stock", true));

    std::cout << "Container contains " << container->get_values().size() << " values:" << std::endl;

    // Access values by key
    std::cout << "\nAccessing values by key:" << std::endl;

    auto product_name = container->find_value("product_name");
    if (product_name) {
        std::cout << "  Product: " << product_name->to_string() << std::endl;
    }

    auto price = container->find_value("price");
    if (price && price->get_type() == double_value) {
        auto price_val = std::static_pointer_cast<class double_value>(price);
        std::cout << "  Price: $" << price_val->get_value() << std::endl;
    }

    auto quantity = container->find_value("quantity");
    if (quantity && quantity->get_type() == int_value) {
        auto qty_val = std::static_pointer_cast<class int_value>(quantity);
        std::cout << "  Quantity: " << qty_val->get_value() << std::endl;
    }

    // Iterate through all values
    std::cout << "\nIterating through all values:" << std::endl;
    for (const auto& value : container->get_values()) {
        std::cout << "  " << value->get_key() << " (" <<
            static_cast<int>(value->get_type()) << "): " << value->to_string() << std::endl;
    }
}

void demonstrate_error_handling() {
    std::cout << "\n=== Error Handling Demonstration ===" << std::endl;

    // Test with invalid serialization data
    auto container = std::make_shared<value_container>();
    std::string invalid_data = "This is not valid serialization data";

    std::cout << "Testing deserialization with invalid data..." << std::endl;
    bool result = container->deserialize(invalid_data);

    if (!result) {
        std::cout << "✓ Correctly handled invalid serialization data" << std::endl;
    } else {
        std::cout << "✗ Unexpectedly succeeded with invalid data" << std::endl;
    }

    // Test accessing non-existent value
    std::cout << "Testing access to non-existent value..." << std::endl;
    auto non_existent = container->find_value("non_existent_key");

    if (!non_existent) {
        std::cout << "✓ Correctly returned null for non-existent key" << std::endl;
    } else {
        std::cout << "✗ Unexpectedly found non-existent key" << std::endl;
    }

    // Test empty container serialization
    std::cout << "Testing empty container serialization..." << std::endl;
    auto empty_container = std::make_shared<value_container>();
    std::string empty_serialized = empty_container->serialize();

    auto empty_deserialized = std::make_shared<value_container>();
    bool empty_result = empty_deserialized->deserialize(empty_serialized);

    if (empty_result) {
        std::cout << "✓ Empty container serialization/deserialization works" << std::endl;
    } else {
        std::cout << "✗ Empty container serialization failed" << std::endl;
    }
}

void demonstrate_performance_basics() {
    std::cout << "\n=== Basic Performance Demonstration ===" << std::endl;

    const int num_operations = 1000;

    // Container creation performance
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::shared_ptr<value_container>> containers;
    containers.reserve(num_operations);

    for (int i = 0; i < num_operations; ++i) {
        auto container = std::make_shared<value_container>();
        container->set_source("perf_client", "session_" + std::to_string(i));
        container->set_target("perf_server", "handler");
        container->set_message_type("performance_test");

        container->add_value(std::make_shared<int_value>("index", i));
        container->add_value(std::make_shared<string_value>("data", "test_data_" + std::to_string(i)));

        containers.push_back(container);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    double containers_per_second = (num_operations * 1000000.0) / duration.count();

    std::cout << "Performance results:" << std::endl;
    std::cout << "  Created " << num_operations << " containers in "
              << duration.count() << " microseconds" << std::endl;
    std::cout << "  Rate: " << std::fixed << std::setprecision(2)
              << containers_per_second << " containers/second" << std::endl;

    // Serialization performance
    start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::string> serialized_data;
    serialized_data.reserve(num_operations);

    for (const auto& container : containers) {
        serialized_data.push_back(container->serialize());
    }

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    double serializations_per_second = (num_operations * 1000000.0) / duration.count();

    std::cout << "  Serialized " << num_operations << " containers in "
              << duration.count() << " microseconds" << std::endl;
    std::cout << "  Rate: " << std::fixed << std::setprecision(2)
              << serializations_per_second << " serializations/second" << std::endl;

    // Calculate total data size
    size_t total_size = 0;
    for (const auto& data : serialized_data) {
        total_size += data.size();
    }

    std::cout << "  Total serialized data: " << total_size << " bytes" << std::endl;
    std::cout << "  Average per container: " << (total_size / num_operations) << " bytes" << std::endl;
}

int main() {
    try {
        std::cout << "Container System Basic Example" << std::endl;
        std::cout << "==============================" << std::endl;

        demonstrate_basic_usage();
        demonstrate_value_types();
        demonstrate_nested_containers();
        demonstrate_serialization();
        demonstrate_value_access();
        demonstrate_error_handling();
        demonstrate_performance_basics();

        std::cout << "\n=== Basic Example Completed Successfully ===" << std::endl;
        std::cout << "This example demonstrated:" << std::endl;
        std::cout << "• Basic container creation and configuration" << std::endl;
        std::cout << "• All supported value types" << std::endl;
        std::cout << "• Nested container structures" << std::endl;
        std::cout << "• Serialization and deserialization" << std::endl;
        std::cout << "• Value access patterns" << std::endl;
        std::cout << "• Error handling" << std::endl;
        std::cout << "• Basic performance characteristics" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error in basic example: " << e.what() << std::endl;
        return 1;
    }
}