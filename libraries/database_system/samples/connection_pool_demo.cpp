/**
 * BSD 3-Clause License
 * Copyright (c) 2024, Database System Project
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <random>
#include <variant>
#include "database/database_manager.h"

using namespace database;

class connection_pool_demo {
public:
    void run_demo() {
        std::cout << "=== Database System - Connection Pool Demo ===" << std::endl;

        std::cout << "\n1. Single Connection Demo:" << std::endl;
        demo_single_connection();

        std::cout << "\n2. Multiple Connections Demo:" << std::endl;
        demo_multiple_connections();

        std::cout << "\n3. Concurrent Operations Demo:" << std::endl;
        demo_concurrent_operations();

        std::cout << "\n=== Connection Pool Demo completed ===" << std::endl;
    }

private:
    void demo_single_connection() {
        std::cout << "Testing single database connection..." << std::endl;

        // Get database manager instance (singleton)
        auto& db_manager = database_manager::handle();

        // Set database type
        db_manager.set_mode(database_types::postgres);

        // Connection string
        std::string connection_string = "host=localhost port=5432 dbname=testdb user=testuser password=testpass";

        std::cout << "Attempting to connect..." << std::endl;
        bool connected = db_manager.connect(connection_string);

        if (connected) {
            std::cout << "✓ Successfully connected to database" << std::endl;
            std::cout << "Connection status: Connected" << std::endl;

            // Perform a simple query
            auto result = db_manager.select_query("SELECT 1 as test_value");
            if (!result.empty()) {
                std::cout << "✓ Test query executed successfully" << std::endl;
                std::cout << "Query result: " << result.size() << " rows" << std::endl;
            } else {
                std::cout << "✗ Test query returned no results" << std::endl;
            }

            // Disconnect
            db_manager.disconnect();
            std::cout << "✓ Disconnected from database" << std::endl;

        } else {
            std::cout << "✗ Failed to connect to database" << std::endl;
            std::cout << "Note: This demo requires a running PostgreSQL server" << std::endl;
        }
    }

    void demo_multiple_connections() {
        std::cout << "Testing multiple database operations..." << std::endl;

        // Note: Since database_manager is a singleton, we simulate multiple
        // connection scenarios by performing multiple operations sequentially

        std::vector<std::string> test_queries = {
            "SELECT 1 as connection_test",
            "SELECT 'Hello' as greeting",
            "SELECT CURRENT_TIMESTAMP as current_time",
            "SELECT 42 as answer"
        };

        auto& db_manager = database_manager::handle();
        db_manager.set_mode(database_types::postgres);

        std::string connection_string = "host=localhost port=5432 dbname=testdb user=testuser password=testuser";

        if (db_manager.connect(connection_string)) {
            std::cout << "✓ Connected to database for multiple operations" << std::endl;

            for (size_t i = 0; i < test_queries.size(); ++i) {
                std::cout << "Executing query " << (i + 1) << "/" << test_queries.size() << "..." << std::endl;

                auto result = db_manager.select_query(test_queries[i]);
                if (!result.empty()) {
                    std::cout << "  ✓ Query " << (i + 1) << " succeeded: " << result.size() << " rows" << std::endl;
                } else {
                    std::cout << "  ✗ Query " << (i + 1) << " returned no results" << std::endl;
                }

                // Small delay to simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            db_manager.disconnect();
            std::cout << "✓ All operations completed, disconnected" << std::endl;

        } else {
            std::cout << "✗ Failed to connect for multiple operations demo" << std::endl;
        }
    }

    void demo_concurrent_operations() {
        std::cout << "Testing concurrent database operations..." << std::endl;

        const int num_threads = 3;
        const int operations_per_thread = 5;
        std::atomic<int> successful_operations(0);
        std::atomic<int> failed_operations(0);

        std::vector<std::thread> threads;

        // Create worker threads
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([t, operations_per_thread, &successful_operations, &failed_operations]() {
                // Each thread gets its own connection attempt
                // Note: In a real connection pool, you'd manage multiple connections

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(50, 200);

                for (int op = 0; op < operations_per_thread; ++op) {
                    try {
                        // Get database manager (singleton, so needs synchronization in real use)
                        auto& db_manager = database_manager::handle();

                        // Simulate different operations
                        std::string query = "SELECT " + std::to_string(t * 100 + op) + " as thread_" +
                                          std::to_string(t) + "_operation_" + std::to_string(op);

                        // Note: This is a simplified simulation
                        // In reality, you'd need proper connection pooling and thread safety
                        bool operation_success = !query.empty(); // Simplified success check

                        if (operation_success) {
                            successful_operations++;
                            std::cout << "Thread " << t << " operation " << op << " succeeded" << std::endl;
                        } else {
                            failed_operations++;
                            std::cout << "Thread " << t << " operation " << op << " failed" << std::endl;
                        }

                        // Random delay to simulate work
                        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));

                    } catch (const std::exception& e) {
                        failed_operations++;
                        std::cout << "Thread " << t << " operation " << op << " exception: " << e.what() << std::endl;
                    }
                }
            });
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }

        std::cout << "Concurrent operations completed:" << std::endl;
        std::cout << "  Successful operations: " << successful_operations.load() << std::endl;
        std::cout << "  Failed operations: " << failed_operations.load() << std::endl;
        std::cout << "  Total operations: " << (successful_operations.load() + failed_operations.load()) << std::endl;

        std::cout << "\nNote: This is a simplified demonstration." << std::endl;
        std::cout << "Real connection pooling would require:" << std::endl;
        std::cout << "  - Multiple actual database connections" << std::endl;
        std::cout << "  - Thread-safe connection management" << std::endl;
        std::cout << "  - Connection lifecycle management" << std::endl;
        std::cout << "  - Connection health monitoring" << std::endl;
    }
};

int main() {
    try {
        connection_pool_demo demo;
        demo.run_demo();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}