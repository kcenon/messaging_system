/**
 * @file basic_demo.cpp
 * @brief Basic demonstration of messaging system functionality
 */

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

#include "container/core/container.h"
#include "container/core/value.h"
#include "container/core/value_types.h"

void demonstrate_container_basics() {
    std::cout << "=== Container Basics Demo ===\n\n";
    
    // Create a container
    auto container = std::make_shared<container_module::value_container>();
    
    // Set message properties
    container->set_message_type("demo_message");
    container->set_source("demo_client", "module_a");
    container->set_target("demo_server", "module_b");
    
    // Create and add values
    auto value1 = container->add(std::make_shared<container_module::value>(
        "message", 
        container_module::value_types::string_value, 
        "Hello, Messaging System!"
    ));
    
    auto value2 = container->add(std::make_shared<container_module::value>(
        "count",
        container_module::value_types::int_value,
        "42"
    ));
    
    auto value3 = container->add(std::make_shared<container_module::value>(
        "pi",
        container_module::value_types::double_value,
        "3.14159"
    ));
    
    // Display container information
    std::cout << "Container created:\n";
    std::cout << "  Type: " << container->message_type() << "\n";
    std::cout << "  Source: " << container->source_id() << "/" << container->source_sub_id() << "\n";
    std::cout << "  Target: " << container->target_id() << "/" << container->target_sub_id() << "\n\n";
    
    // Access values
    std::cout << "Values:\n";
    auto msg_value = container->get_value("message");
    if (msg_value && !msg_value->is_null()) {
        std::cout << "  message: " << msg_value->to_string() << "\n";
    }
    
    auto count_value = container->get_value("count");
    if (count_value && !count_value->is_null()) {
        std::cout << "  count: " << count_value->to_int() << "\n";
    }
    
    auto pi_value = container->get_value("pi");
    if (pi_value && !pi_value->is_null()) {
        std::cout << "  pi: " << pi_value->to_double() << "\n";
    }
}

void demonstrate_serialization() {
    std::cout << "\n=== Serialization Demo ===\n\n";
    
    // Create a container with data
    auto original = std::make_shared<container_module::value_container>();
    original->set_message_type("config");
    original->add(std::make_shared<container_module::value>(
        "server", container_module::value_types::string_value, "localhost"
    ));
    original->add(std::make_shared<container_module::value>(
        "port", container_module::value_types::int_value, "8080"
    ));
    
    // Serialize
    std::string serialized = original->serialize();
    std::cout << "Serialized (" << serialized.size() << " bytes):\n";
    std::cout << serialized.substr(0, 100) << "...\n\n";
    
    // Deserialize
    auto deserialized = std::make_shared<container_module::value_container>(serialized);
    
    // Verify
    std::cout << "Deserialized:\n";
    std::cout << "  Type: " << deserialized->message_type() << "\n";
    
    auto server = deserialized->get_value("server");
    if (server) {
        std::cout << "  Server: " << server->to_string() << "\n";
    }
    
    auto port = deserialized->get_value("port");
    if (port) {
        std::cout << "  Port: " << port->to_int() << "\n";
    }
}

void demonstrate_nested_containers() {
    std::cout << "\n=== Nested Containers Demo ===\n\n";
    
    // Create root container
    auto root = std::make_shared<container_module::value_container>();
    root->set_message_type("user_info");
    
    // Add simple value
    root->add(std::make_shared<container_module::value>(
        "username", container_module::value_types::string_value, "johndoe"
    ));
    
    // Create nested container
    auto profile = std::make_shared<container_module::value>("profile");
    profile->add(std::make_shared<container_module::value>(
        "age", container_module::value_types::int_value, "30"
    ));
    profile->add(std::make_shared<container_module::value>(
        "city", container_module::value_types::string_value, "New York"
    ));
    
    root->add(profile);
    
    std::cout << "Created nested structure:\n";
    std::cout << "- username: " << root->get_value("username")->to_string() << "\n";
    
    auto profile_values = root->value_array("profile");
    if (!profile_values.empty()) {
        std::cout << "- profile:\n";
        for (const auto& child : profile_values[0]->children()) {
            std::cout << "  - " << child->name() << ": " << child->to_string() << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Messaging System Basic Demo\n";
    std::cout << "==========================\n\n";
    
    try {
        demonstrate_container_basics();
        demonstrate_serialization();
        demonstrate_nested_containers();
        
        std::cout << "\nDemo completed successfully!\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}