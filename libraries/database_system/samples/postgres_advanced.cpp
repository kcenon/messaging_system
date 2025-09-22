/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Database System Project
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <variant>
#include "database/postgres_manager.h"

using namespace database;

int main() {
    std::cout << "=== Database System - PostgreSQL Advanced Features Example ===" << std::endl;

    try {
        // Create PostgreSQL manager instance
        auto pg_manager = std::make_unique<postgres_manager>();

        std::cout << "\n1. Database Connection:" << std::endl;

        // Connection string (modify for your PostgreSQL server)
        std::string connection_string = "host=localhost port=5432 dbname=testdb user=testuser password=testpass";

        std::cout << "Attempting to connect to PostgreSQL..." << std::endl;
        bool connected = pg_manager->connect(connection_string);

        if (connected) {
            std::cout << "✓ Successfully connected to PostgreSQL database" << std::endl;

            // 2. Table creation with advanced features
            std::cout << "\n2. Creating Advanced Table:" << std::endl;

            std::string create_table_sql = R"(
                CREATE TABLE IF NOT EXISTS products (
                    id SERIAL PRIMARY KEY,
                    name VARCHAR(100) NOT NULL,
                    description TEXT,
                    price DECIMAL(10,2),
                    tags TEXT[],
                    metadata JSONB,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            )";

            std::cout << "Creating products table with advanced PostgreSQL features..." << std::endl;
            bool table_created = pg_manager->create_query(create_table_sql);
            if (table_created) {
                std::cout << "✓ Advanced products table created successfully" << std::endl;
            } else {
                std::cout << "✗ Failed to create products table" << std::endl;
            }

            // 3. Insert sample data
            std::cout << "\n3. Inserting Sample Data:" << std::endl;

            std::vector<std::string> insert_queries = {
                R"(INSERT INTO products (name, description, price, tags, metadata) VALUES
                   ('Gaming Laptop', 'High-performance gaming laptop', 1299.99,
                    ARRAY['gaming', 'laptop', 'computer'],
                    '{"brand": "TechCorp", "specs": {"ram": "16GB", "cpu": "Intel i7"}}'::jsonb))",

                R"(INSERT INTO products (name, description, price, tags, metadata) VALUES
                   ('Office Keyboard', 'Mechanical keyboard for office use', 79.99,
                    ARRAY['keyboard', 'office'],
                    '{"brand": "KeyMaster", "type": "mechanical"}'::jsonb))",

                R"(INSERT INTO products (name, description, price, tags, metadata) VALUES
                   ('Gaming Mouse', 'RGB gaming mouse', 49.99,
                    ARRAY['gaming', 'mouse'],
                    '{"brand": "TechCorp", "features": ["RGB", "wireless"]}'::jsonb))"
            };

            for (const auto& query : insert_queries) {
                unsigned int inserted = pg_manager->insert_query(query);
                if (inserted > 0) {
                    std::cout << "✓ Product inserted successfully" << std::endl;
                } else {
                    std::cout << "✗ Failed to insert product (may already exist)" << std::endl;
                }
            }

            // 4. Advanced queries
            std::cout << "\n4. Advanced PostgreSQL Queries:" << std::endl;

            // Array operations
            std::cout << "\nQuerying products with array operations:" << std::endl;
            std::string array_query = "SELECT name, tags FROM products WHERE 'gaming' = ANY(tags)";
            auto gaming_products = pg_manager->select_query(array_query);

            if (!gaming_products.empty()) {
                std::cout << "Products with 'gaming' tag (" << gaming_products.size() << " rows):" << std::endl;
                for (const auto& row : gaming_products) {
                    for (const auto& [key, value] : row) {
                        std::cout << "  " << key << ": ";
                        std::visit([](const auto& v) { std::cout << v; }, value);
                        std::cout << " ";
                    }
                    std::cout << std::endl;
                }
            } else {
                std::cout << "No gaming products found" << std::endl;
            }

            // JSONB operations
            std::cout << "\nQuerying products with JSONB operations:" << std::endl;
            std::string json_query = "SELECT name, metadata->>'brand' as brand FROM products WHERE metadata->>'brand' = 'TechCorp'";
            auto techcorp_products = pg_manager->select_query(json_query);

            if (!techcorp_products.empty()) {
                std::cout << "TechCorp products (" << techcorp_products.size() << " rows):" << std::endl;
                for (const auto& row : techcorp_products) {
                    for (const auto& [key, value] : row) {
                        std::cout << "  " << key << ": ";
                        std::visit([](const auto& v) { std::cout << v; }, value);
                        std::cout << " ";
                    }
                    std::cout << std::endl;
                }
            } else {
                std::cout << "No TechCorp products found" << std::endl;
            }

            // 5. Cleanup
            std::cout << "\n5. Cleanup:" << std::endl;

            // Optionally clean up test data
            // std::string cleanup_sql = "DELETE FROM products WHERE name LIKE '%Gaming%' OR name LIKE '%Office%'";
            // unsigned int deleted = pg_manager->delete_query(cleanup_sql);
            // std::cout << "Cleaned up " << deleted << " test records" << std::endl;

            // Disconnect
            pg_manager->disconnect();
            std::cout << "✓ Disconnected from PostgreSQL database" << std::endl;

        } else {
            std::cout << "✗ Failed to connect to PostgreSQL database" << std::endl;
            std::cout << "Please ensure:" << std::endl;
            std::cout << "  - PostgreSQL server is running" << std::endl;
            std::cout << "  - Database 'testdb' exists" << std::endl;
            std::cout << "  - User 'testuser' has appropriate permissions" << std::endl;
            std::cout << "  - Connection parameters are correct" << std::endl;

            std::cout << "\nTo test with a real database, update the connection string:" << std::endl;
            std::cout << "  host=your_host port=5432 dbname=your_db user=your_user password=your_pass" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n=== PostgreSQL Advanced Features Example completed ===" << std::endl;
    return 0;
}