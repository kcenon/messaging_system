/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Container System Project
 */

#include <iostream>
#include <string>
#include <vector>
#include "container.h"

using namespace container_module;

int main() {
    std::cout << "=== Container System - Basic Usage Example ===" << std::endl;
    
    // 1. Basic container creation and value setting
    std::cout << "\n1. Basic Container Operations:" << std::endl;
    
    auto container = std::make_shared<value_container>();
    container->set_message_type("user_profile");
    
    // Set various types of values
    container->set_value("user_id", std::make_shared<string_value>("12345"));
    container->set_value("username", std::make_shared<string_value>("john_doe"));
    container->set_value("age", std::make_shared<string_value>("30"));
    container->set_value("is_active", std::make_shared<bool_value>(true));
    container->set_value("balance", std::make_shared<string_value>("1000.50"));
    
    std::cout << "Container message type: " << container->get_message_type() << std::endl;
    std::cout << "Container size: " << container->size() << " values" << std::endl;
    
    // 2. Reading values from container
    std::cout << "\n2. Reading Values:" << std::endl;
    
    auto user_id = container->get_value("user_id");
    if (user_id) {
        std::cout << "User ID: " << user_id->to_string() << std::endl;
    }
    
    auto username = container->get_value("username");
    if (username) {
        std::cout << "Username: " << username->to_string() << std::endl;
    }
    
    auto is_active = container->get_value("is_active");
    if (is_active) {
        std::cout << "Is Active: " << (is_active->to_bool() ? "Yes" : "No") << std::endl;
    }
    
    // 3. Nested containers
    std::cout << "\n3. Nested Containers:" << std::endl;
    
    auto address_container = std::make_shared<value_container>();
    address_container->set_message_type("address");
    address_container->set_value("street", std::make_shared<string_value>("123 Main St"));
    address_container->set_value("city", std::make_shared<string_value>("New York"));
    address_container->set_value("zip", std::make_shared<string_value>("10001"));
    
    container->set_value("address", std::make_shared<container_value>(address_container));
    
    auto address = container->get_value("address");
    if (address && address->get_type() == value_types::container) {
        auto addr_container = std::static_pointer_cast<container_value>(address)->get_container();
        auto street = addr_container->get_value("street");
        auto city = addr_container->get_value("city");
        
        if (street && city) {
            std::cout << "Address: " << street->to_string() << ", " << city->to_string() << std::endl;
        }
    }
    
    // 4. Binary data handling
    std::cout << "\n4. Binary Data:" << std::endl;
    
    std::vector<uint8_t> binary_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    container->set_value("avatar", std::make_shared<bytes_value>(binary_data));
    
    auto avatar = container->get_value("avatar");
    if (avatar && avatar->get_type() == value_types::bytes) {
        auto bytes_val = std::static_pointer_cast<bytes_value>(avatar);
        auto data = bytes_val->get_bytes();
        std::cout << "Avatar data size: " << data.size() << " bytes" << std::endl;
        std::cout << "Avatar data (as text): ";
        for (auto byte : data) {
            std::cout << static_cast<char>(byte);
        }
        std::cout << std::endl;
    }
    
    // 5. Container serialization
    std::cout << "\n5. Serialization:" << std::endl;
    
    std::string serialized = container->serialize();
    std::cout << "Serialized container size: " << serialized.length() << " characters" << std::endl;
    std::cout << "Serialized data preview: " << serialized.substr(0, 100) << "..." << std::endl;
    
    // 6. Container deserialization
    std::cout << "\n6. Deserialization:" << std::endl;
    
    auto restored_container = std::make_shared<value_container>(serialized);
    std::cout << "Restored container message type: " << restored_container->get_message_type() << std::endl;
    std::cout << "Restored container size: " << restored_container->size() << " values" << std::endl;
    
    auto restored_username = restored_container->get_value("username");
    if (restored_username) {
        std::cout << "Restored username: " << restored_username->to_string() << std::endl;
    }
    
    // 7. Container iteration
    std::cout << "\n7. Container Iteration:" << std::endl;
    
    std::cout << "All values in container:" << std::endl;
    for (const auto& pair : *container) {
        std::cout << "  " << pair.first << ": " << pair.second->to_string() 
                  << " (type: " << static_cast<int>(pair.second->get_type()) << ")" << std::endl;
    }
    
    std::cout << "\n=== Example completed successfully ===" << std::endl;
    return 0;
}