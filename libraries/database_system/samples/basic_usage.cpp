/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Database System Project
 */

#include <iostream>
#include <string>
#include <memory>
#include <variant>
#include "database/database_manager.h"
#include "database/postgres_manager.h"

using namespace database;

int main() {
    std::cout << "=== Database System - Basic Usage Example ===" << std::endl;
    
    // 1. Database manager creation and configuration
    std::cout << "\n1. Database Manager Setup:" << std::endl;
    
    auto& db_manager = database_manager::handle();

    // Set database type
    db_manager.set_mode(database_types::postgres);
    std::cout << "Database type set to: PostgreSQL" << std::endl;
    
    // Connection string (modify these values for your database)
    std::string connection_string = "host=localhost port=5432 dbname=testdb user=testuser password=testpass";
    std::cout << "Connection string configured" << std::endl;
    
    // Note: This example shows the API usage, but requires an actual PostgreSQL server
    std::cout << "Note: This example demonstrates API usage. Actual database connection requires PostgreSQL server." << std::endl;
    
    // 2. Connection management
    std::cout << "\n2. Connection Management:" << std::endl;
    
    std::cout << "Attempting to connect to database..." << std::endl;
    bool connected = db_manager.connect(connection_string);
    
    if (connected) {
        std::cout << "✓ Successfully connected to database" << std::endl;
        std::cout << "Connection status: Connected" << std::endl;
        std::cout << "Database type: " << static_cast<int>(db_manager.database_type()) << std::endl;
        
        // 3. Table operations
        std::cout << "\n3. Table Operations:" << std::endl;
        
        // Create table
        std::string create_table_sql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id SERIAL PRIMARY KEY,
                username VARCHAR(50) UNIQUE NOT NULL,
                email VARCHAR(100) UNIQUE NOT NULL,
                age INTEGER CHECK (age >= 0),
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                is_active BOOLEAN DEFAULT TRUE
            )
        )";
        
        std::cout << "Creating users table..." << std::endl;
        bool table_created = db_manager.create_query(create_table_sql);
        if (table_created) {
            std::cout << "✓ Users table created successfully" << std::endl;
        } else {
            std::cout << "✗ Failed to create users table" << std::endl;
        }
        
        // 4. Data insertion
        std::cout << "\n4. Data Insertion:" << std::endl;
        
        std::vector<std::string> insert_queries = {
            "INSERT INTO users (username, email, age) VALUES ('john_doe', 'john@example.com', 30)",
            "INSERT INTO users (username, email, age) VALUES ('jane_smith', 'jane@example.com', 25)",
            "INSERT INTO users (username, email, age) VALUES ('bob_wilson', 'bob@example.com', 35)",
            "INSERT INTO users (username, email, age, is_active) VALUES ('alice_brown', 'alice@example.com', 28, FALSE)"
        };
        
        for (const auto& query : insert_queries) {
            unsigned int inserted = db_manager.insert_query(query);
            if (inserted > 0) {
                std::cout << "✓ User inserted successfully" << std::endl;
            } else {
                std::cout << "✗ Failed to insert user (may already exist)" << std::endl;
            }
        }
        
        // 5. Data selection
        std::cout << "\n5. Data Selection:" << std::endl;
        
        std::string select_all = "SELECT id, username, email, age, is_active FROM users ORDER BY id";
        auto all_users = db_manager.select_query(select_all);

        if (!all_users.empty()) {
            std::cout << "✓ All users retrieved (" << all_users.size() << " rows):" << std::endl;
            for (const auto& row : all_users) {
                std::cout << "  User: ";
                for (const auto& [key, value] : row) {
                    std::cout << key << "=";
                    std::visit([](const auto& v) { std::cout << v; }, value);
                    std::cout << " ";
                }
                std::cout << std::endl;
            }
        } else {
            std::cout << "✗ Failed to retrieve users" << std::endl;
        }
        
        // Select specific user
        std::string select_user = "SELECT username, email, age FROM users WHERE username = 'john_doe'";
        auto john_data = db_manager.select_query(select_user);

        if (!john_data.empty()) {
            std::cout << "✓ John's data retrieved:" << std::endl;
            for (const auto& row : john_data) {
                for (const auto& [key, value] : row) {
                    std::cout << "  " << key << ": ";
                    std::visit([](const auto& v) { std::cout << v; }, value);
                    std::cout << std::endl;
                }
            }
        } else {
            std::cout << "✗ John's data not found" << std::endl;
        }
        
        // 6. Data updates
        std::cout << "\n6. Data Updates:" << std::endl;
        
        std::string update_query = "UPDATE users SET age = 31 WHERE username = 'john_doe'";
        unsigned int updated = db_manager.update_query(update_query);
        
        if (updated > 0) {
            std::cout << "✓ John's age updated successfully" << std::endl;
            
            // Verify update
            auto updated_data = db_manager.select_query("SELECT username, age FROM users WHERE username = 'john_doe'");
            if (!updated_data.empty()) {
                std::cout << "Updated data: ";
                for (const auto& row : updated_data) {
                    for (const auto& [key, value] : row) {
                        std::cout << key << "=";
                        std::visit([](const auto& v) { std::cout << v; }, value);
                        std::cout << " ";
                    }
                    std::cout << std::endl;
                }
            }
        } else {
            std::cout << "✗ Failed to update John's age" << std::endl;
        }
        
        // 7. Data deletion
        std::cout << "\n7. Data Deletion:" << std::endl;
        
        std::string delete_query = "DELETE FROM users WHERE username LIKE 'temp_user%'";
        unsigned int deleted = db_manager.delete_query(delete_query);
        
        if (deleted > 0) {
            std::cout << "✓ Temporary users deleted successfully" << std::endl;
        } else {
            std::cout << "✗ Failed to delete temporary users" << std::endl;
        }
        
        // 9. Connection testing
        std::cout << "\n9. Connection Health Check:" << std::endl;
        
        // Note: test_connection method not available in current API
        std::cout << "Connection test: Assuming healthy if connected";
        std::cout << " - OK" << std::endl;
        
        // 10. Cleanup
        std::cout << "\n10. Cleanup:" << std::endl;
        
        // Optionally drop the test table (uncomment if needed)
        // std::string drop_table = "DROP TABLE IF EXISTS users";
        // bool table_dropped = db_manager->drop_query(drop_table);
        // if (table_dropped) {
        //     std::cout << "✓ Test table dropped successfully" << std::endl;
        // }
        
        // Disconnect
        db_manager.disconnect();
        std::cout << "✓ Disconnected from database" << std::endl;
        std::cout << "Connection status: Disconnected" << std::endl;
        
    } else {
        std::cout << "✗ Failed to connect to database" << std::endl;
        std::cout << "Please ensure:" << std::endl;
        std::cout << "  - PostgreSQL server is running" << std::endl;
        std::cout << "  - Database 'testdb' exists" << std::endl;
        std::cout << "  - User 'testuser' has appropriate permissions" << std::endl;
        std::cout << "  - Connection parameters are correct" << std::endl;
        
        std::cout << "\nTo test with a real database, update the connection string:" << std::endl;
        std::cout << "  host=your_host port=5432 dbname=your_db user=your_user password=your_pass" << std::endl;
    }
    
    std::cout << "\n=== Basic Usage Example completed ===" << std::endl;
    return 0;
}