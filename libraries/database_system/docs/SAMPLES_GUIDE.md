# Database System Samples Guide

Comprehensive guide to the sample programs demonstrating various features of the Database System.

## Table of Contents

- [Overview](#overview)
- [Basic Usage Sample](#basic-usage-sample)
- [PostgreSQL Advanced Sample](#postgresql-advanced-sample)
- [Connection Pool Demo](#connection-pool-demo)
- [Query Builder Examples](#query-builder-examples)
- [Multi-Database Examples](#multi-database-examples)
- [Best Practices](#best-practices)

## Overview

The Database System includes several sample programs that demonstrate different aspects of the library:

| Sample | File | Description | Dependencies |
|--------|------|-------------|--------------|
| Basic Usage | `basic_usage.cpp` | Core database operations | None (mock fallback) |
| PostgreSQL Advanced | `postgres_advanced.cpp` | Advanced PostgreSQL features | PostgreSQL (optional) |
| Connection Pool Demo | `connection_pool_demo.cpp` | Connection pooling showcase | None (mock fallback) |
| Run All Samples | `run_all_samples.cpp` | Executes all sample programs | None |

### Building Samples

```bash
# Build all samples
mkdir build && cd build
cmake .. -DBUILD_DATABASE_SAMPLES=ON
ninja  # or make

# Run samples
./bin/basic_usage
./bin/postgres_advanced
./bin/connection_pool_demo
./bin/run_all_samples
```

## Basic Usage Sample

**File**: `samples/basic_usage.cpp`

Demonstrates fundamental database operations using the database_manager singleton.

### Key Features Demonstrated

1. **Database Manager Setup**
2. **Connection Management**
3. **CRUD Operations**
4. **Error Handling**
5. **Mock Implementation Fallback**

### Code Walkthrough

```cpp
#include <database/database_manager.h>
#include <iostream>

int main()
{
    std::cout << "=== Database System - Basic Usage Example ===" << std::endl;

    try {
        // 1. Database Manager Setup
        std::cout << "\n1. Database Manager Setup:" << std::endl;
        database::database_manager& db_manager = database::database_manager::handle();

        // Set database type to PostgreSQL
        if (!db_manager.set_mode(database::database_types::postgres)) {
            std::cerr << "Failed to set database mode" << std::endl;
            return 1;
        }
        std::cout << "Database type set to: PostgreSQL" << std::endl;

        // 2. Connection Management
        std::cout << "\n2. Connection Management:" << std::endl;
        std::string connection_string =
            "host=localhost port=5432 dbname=testdb user=testuser password=testpass";
        std::cout << "Connection string configured" << std::endl;

        std::cout << "Attempting to connect to database..." << std::endl;
        if (!db_manager.connect(connection_string)) {
            std::cout << "✗ Failed to connect to database" << std::endl;
            std::cout << "Please ensure:" << std::endl;
            std::cout << "  - PostgreSQL server is running" << std::endl;
            std::cout << "  - Database 'testdb' exists" << std::endl;
            std::cout << "  - User 'testuser' has appropriate permissions" << std::endl;
            std::cout << "  - Connection parameters are correct" << std::endl;

            std::cout << "\nTo test with a real database, update the connection string:" << std::endl;
            std::cout << "  host=your_host port=5432 dbname=your_db user=your_user password=your_pass" << std::endl;

            std::cout << "\n=== Basic Usage Example completed ===" << std::endl;
            return 0;
        }

        std::cout << "✓ Successfully connected to database" << std::endl;

        // 3. Create Table (DDL)
        std::cout << "\n3. Creating Table:" << std::endl;
        bool table_created = db_manager.create_query(
            "CREATE TABLE IF NOT EXISTS users ("
            "id SERIAL PRIMARY KEY, "
            "username VARCHAR(50) NOT NULL, "
            "email VARCHAR(100) UNIQUE, "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ")"
        );

        if (table_created) {
            std::cout << "✓ Table 'users' created successfully" << std::endl;
        } else {
            std::cout << "✗ Failed to create table" << std::endl;
            return 1;
        }

        // 4. Insert Data
        std::cout << "\n4. Inserting Data:" << std::endl;
        unsigned int inserted_rows = db_manager.insert_query(
            "INSERT INTO users (username, email) VALUES "
            "('john_doe', 'john@example.com'), "
            "('jane_smith', 'jane@example.com'), "
            "('bob_wilson', 'bob@example.com')"
        );

        std::cout << "✓ Inserted " << inserted_rows << " rows" << std::endl;

        // 5. Select Data
        std::cout << "\n5. Selecting Data:" << std::endl;
        database::database_result users = db_manager.select_query("SELECT * FROM users ORDER BY id");

        std::cout << "Retrieved " << users.size() << " users:" << std::endl;
        std::cout << "ID | Username   | Email                | Created At" << std::endl;
        std::cout << "---|------------|----------------------|-------------------" << std::endl;

        for (const auto& user : users) {
            for (const auto& [column, value] : user) {
                std::visit([](const auto& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>) {
                        std::cout << "NULL";
                    } else {
                        std::cout << v;
                    }
                }, value);
                std::cout << " | ";
            }
            std::cout << std::endl;
        }

        // 6. Update Data
        std::cout << "\n6. Updating Data:" << std::endl;
        unsigned int updated_rows = db_manager.update_query(
            "UPDATE users SET email = 'john.doe@newdomain.com' WHERE username = 'john_doe'"
        );
        std::cout << "✓ Updated " << updated_rows << " rows" << std::endl;

        // 7. Delete Data
        std::cout << "\n7. Deleting Data:" << std::endl;
        unsigned int deleted_rows = db_manager.delete_query(
            "DELETE FROM users WHERE username = 'bob_wilson'"
        );
        std::cout << "✓ Deleted " << deleted_rows << " rows" << std::endl;

        // 8. Final Select
        std::cout << "\n8. Final State:" << std::endl;
        database::database_result final_users = db_manager.select_query("SELECT * FROM users ORDER BY id");
        std::cout << "Final user count: " << final_users.size() << std::endl;

        // 9. Disconnect
        if (db_manager.disconnect()) {
            std::cout << "\n✓ Successfully disconnected from database" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n=== Basic Usage Example completed ===" << std::endl;
    return 0;
}
```

### Expected Output

```
=== Database System - Basic Usage Example ===

1. Database Manager Setup:
Database type set to: PostgreSQL
Connection string configured
Note: This example demonstrates API usage. Actual database connection requires PostgreSQL server.

2. Connection Management:
Attempting to connect to database...
✗ Failed to connect to database
Please ensure:
  - PostgreSQL server is running
  - Database 'testdb' exists
  - User 'testuser' has appropriate permissions
  - Connection parameters are correct

To test with a real database, update the connection string:
  host=your_host port=5432 dbname=your_db user=your_user password=your_pass

=== Basic Usage Example completed ===
PostgreSQL support not compiled. Connection: host=localhost port=...
```

### Learning Points

1. **Singleton Pattern**: Uses `database_manager::handle()` for global access
2. **Error Handling**: Graceful fallback when database is unavailable
3. **CRUD Operations**: Complete create, read, update, delete cycle
4. **Mock Support**: Works without actual database installation
5. **Connection Management**: Proper connect/disconnect lifecycle

## PostgreSQL Advanced Sample

**File**: `samples/postgres_advanced.cpp`

Demonstrates advanced PostgreSQL-specific features and optimizations.

### Key Features Demonstrated

1. **Prepared Statements**
2. **Transactions**
3. **Batch Operations**
4. **Advanced Data Types**
5. **Connection Pooling Integration**

### Code Highlights

```cpp
// Transaction example
bool transaction_success = db_manager.create_query("BEGIN");

try {
    // Multiple operations in transaction
    db_manager.insert_query("INSERT INTO accounts (name, balance) VALUES ('Alice', 1000)");
    db_manager.insert_query("INSERT INTO accounts (name, balance) VALUES ('Bob', 500)");

    // Transfer money
    db_manager.update_query("UPDATE accounts SET balance = balance - 100 WHERE name = 'Alice'");
    db_manager.update_query("UPDATE accounts SET balance = balance + 100 WHERE name = 'Bob'");

    // Commit transaction
    db_manager.create_query("COMMIT");
    std::cout << "✓ Transaction committed successfully" << std::endl;

} catch (const std::exception& e) {
    // Rollback on error
    db_manager.create_query("ROLLBACK");
    std::cerr << "✗ Transaction rolled back: " << e.what() << std::endl;
}
```

### Advanced Features

```cpp
// JSON data handling
db_manager.create_query(
    "CREATE TABLE products ("
    "id SERIAL PRIMARY KEY, "
    "name VARCHAR(100), "
    "metadata JSONB"
    ")"
);

db_manager.insert_query(
    "INSERT INTO products (name, metadata) VALUES "
    "('Laptop', '{\"brand\": \"TechCorp\", \"specs\": {\"cpu\": \"Intel i7\", \"ram\": \"16GB\"}}'), "
    "('Mouse', '{\"brand\": \"TechCorp\", \"specs\": {\"type\": \"wireless\", \"dpi\": 1600}}')"
);

// Query JSON data
auto products = db_manager.select_query(
    "SELECT name, metadata->>'brand' as brand, metadata->'specs'->>'cpu' as cpu "
    "FROM products WHERE metadata->>'brand' = 'TechCorp'"
);
```

## Connection Pool Demo

**File**: `samples/connection_pool_demo.cpp`

Comprehensive demonstration of connection pooling capabilities.

### Key Features Demonstrated

1. **Pool Configuration**
2. **Concurrent Access**
3. **Health Monitoring**
4. **Statistics Tracking**
5. **Thread Safety**

### Code Walkthrough

```cpp
#include <database/database_manager.h>
#include <database/connection_pool.h>
#include <thread>
#include <vector>
#include <chrono>

int main()
{
    std::cout << "=== Database System - Connection Pool Demo ===" << std::endl;

    // 1. Single Connection Demo
    std::cout << "\n1. Single Connection Demo:" << std::endl;
    single_connection_demo();

    // 2. Multiple Connections Demo
    std::cout << "\n2. Multiple Connections Demo:" << std::endl;
    multiple_connections_demo();

    // 3. Concurrent Operations Demo
    std::cout << "\n3. Concurrent Operations Demo:" << std::endl;
    concurrent_operations_demo();

    std::cout << "\n=== Connection Pool Demo completed ===" << std::endl;
    return 0;
}

void concurrent_operations_demo()
{
    database::database_manager& db_manager = database::database_manager::handle();

    // Configure connection pool
    database::connection_pool_config config;
    config.min_connections = 2;
    config.max_connections = 5;
    config.acquire_timeout = std::chrono::seconds(5);
    config.idle_timeout = std::chrono::seconds(30);
    config.health_check_interval = std::chrono::seconds(60);
    config.connection_string = "host=localhost port=5432 dbname=testdb user=testuser password=testpass";

    // Create connection pool
    if (!db_manager.create_connection_pool(database::database_types::postgres, config)) {
        std::cout << "✗ Failed to create connection pool" << std::endl;
        return;
    }

    // Get pool reference
    auto pool = db_manager.get_connection_pool(database::database_types::postgres);
    if (!pool) {
        std::cout << "✗ Failed to get connection pool" << std::endl;
        return;
    }

    std::cout << "Testing concurrent database operations..." << std::endl;

    // Launch multiple threads
    std::vector<std::thread> threads;
    std::atomic<int> successful_operations{0};
    std::atomic<int> failed_operations{0};

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&pool, &successful_operations, &failed_operations, i]() {
            for (int j = 0; j < 5; ++j) {
                try {
                    // Acquire connection from pool
                    auto connection = pool->acquire_connection();
                    if (connection) {
                        // Simulate database operation
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));

                        // Mock successful operation
                        ++successful_operations;
                        std::cout << "Thread " << i << " operation " << j << " succeeded" << std::endl;

                        // Connection automatically returned to pool when destroyed
                    } else {
                        ++failed_operations;
                        std::cout << "Thread " << i << " operation " << j << " failed to acquire connection" << std::endl;
                    }
                } catch (const std::exception& e) {
                    ++failed_operations;
                    std::cout << "Thread " << i << " operation " << j << " exception: " << e.what() << std::endl;
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

    // Display pool statistics
    auto stats = db_manager.get_pool_stats();
    for (const auto& [db_type, stat] : stats) {
        std::cout << "\nPool Statistics:" << std::endl;
        std::cout << "  Total connections: " << stat.total_connections << std::endl;
        std::cout << "  Active connections: " << stat.active_connections << std::endl;
        std::cout << "  Available connections: " << stat.available_connections << std::endl;
        std::cout << "  Successful acquisitions: " << stat.successful_acquisitions << std::endl;
        std::cout << "  Failed acquisitions: " << stat.failed_acquisitions << std::endl;
    }
}
```

### Performance Insights

```cpp
// Measure connection acquisition time
auto start = std::chrono::high_resolution_clock::now();
auto connection = pool->acquire_connection();
auto end = std::chrono::high_resolution_clock::now();

auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
std::cout << "Connection acquired in " << duration.count() << " microseconds" << std::endl;
```

## Query Builder Examples

### SQL Query Builder Examples

```cpp
#include <database/database_manager.h>
#include <database/query_builder.h>

void sql_query_examples()
{
    database::database_manager& db = database::database_manager::handle();

    // 1. Simple SELECT
    auto simple_select = db.create_query_builder(database::database_types::postgres)
        .select({"id", "name", "email"})
        .from("users")
        .where("status", "=", database::database_value{std::string("active")})
        .order_by("created_at", database::sort_order::desc)
        .limit(10);

    std::cout << "Simple SELECT: " << simple_select.build() << std::endl;
    // Output: SELECT "id", "name", "email" FROM "users" WHERE status = 'active' ORDER BY created_at DESC LIMIT 10

    // 2. Complex JOIN
    auto join_query = db.create_query_builder(database::database_types::postgres)
        .select({"u.name", "p.title", "p.created_at"})
        .from("users u")
        .left_join("posts p", "u.id = p.user_id")
        .where("u.status", "=", database::database_value{std::string("active")})
        .where("p.published", "=", database::database_value{true})
        .order_by("p.created_at", database::sort_order::desc);

    std::cout << "JOIN Query: " << join_query.build() << std::endl;

    // 3. INSERT with multiple values
    auto insert_query = db.create_query_builder(database::database_types::postgres)
        .insert_into("products")
        .values({
            {
                {"name", database::database_value{std::string("Laptop")}},
                {"price", database::database_value{double(999.99)}},
                {"in_stock", database::database_value{true}}
            },
            {
                {"name", database::database_value{std::string("Mouse")}},
                {"price", database::database_value{double(29.99)}},
                {"in_stock", database::database_value{false}}
            }
        });

    std::cout << "INSERT Query: " << insert_query.build() << std::endl;

    // 4. UPDATE with conditions
    auto update_query = db.create_query_builder(database::database_types::postgres)
        .update("products")
        .set("price", database::database_value{double(899.99)})
        .set("updated_at", database::database_value{std::string("NOW()")})
        .where("name", "=", database::database_value{std::string("Laptop")});

    std::cout << "UPDATE Query: " << update_query.build() << std::endl;
}
```

### MongoDB Query Builder Examples

```cpp
void mongodb_query_examples()
{
    database::database_manager& db = database::database_manager::handle();

    // 1. Simple Find
    auto find_query = db.create_query_builder(database::database_types::mongodb)
        .collection("users")
        .find({{"status", database::database_value{std::string("active")}}})
        .project({"name", "email"})
        .sort("created_at", -1)
        .limit(10);

    std::cout << "MongoDB Find: " << find_query.build() << std::endl;
    // Output: db.users.find({ "status": "active" }, { "name": 1, "email": 1 }).sort({"created_at": -1}).limit(10)

    // 2. Aggregation Pipeline
    auto agg_query = db.create_query_builder(database::database_types::mongodb)
        .collection("orders")
        .match({{"status", database::database_value{std::string("completed")}}})
        .group({
            {"_id", database::database_value{std::string("$customer_id")}},
            {"total_amount", database::database_value{std::string("$sum: $amount")}},
            {"order_count", database::database_value{std::string("$sum: 1")}}
        });

    std::cout << "MongoDB Aggregation: " << agg_query.build() << std::endl;

    // 3. Insert Document
    auto insert_doc = db.create_query_builder(database::database_types::mongodb)
        .collection("products")
        .insert_one({
            {"name", database::database_value{std::string("New Product")}},
            {"price", database::database_value{double(49.99)}},
            {"category", database::database_value{std::string("electronics")}},
            {"in_stock", database::database_value{true}}
        });

    std::cout << "MongoDB Insert: " << insert_doc.build() << std::endl;
}
```

### Redis Query Builder Examples

```cpp
void redis_query_examples()
{
    database::database_manager& db = database::database_manager::handle();

    // 1. String Operations
    auto set_cmd = db.create_query_builder(database::database_types::redis)
        .set("user:123:name", "John Doe");
    std::cout << "Redis SET: " << set_cmd.build() << std::endl;
    // Output: SET user:123:name "John Doe"

    auto get_cmd = db.create_query_builder(database::database_types::redis)
        .get("user:123:name");
    std::cout << "Redis GET: " << get_cmd.build() << std::endl;

    // 2. Hash Operations
    auto hset_cmd = db.create_query_builder(database::database_types::redis)
        .hset("user:123", "email", "john@example.com");
    std::cout << "Redis HSET: " << hset_cmd.build() << std::endl;

    auto hgetall_cmd = db.create_query_builder(database::database_types::redis)
        .hgetall("user:123");
    std::cout << "Redis HGETALL: " << hgetall_cmd.build() << std::endl;

    // 3. List Operations
    auto lpush_cmd = db.create_query_builder(database::database_types::redis)
        .lpush("notifications:123", "New message received");
    std::cout << "Redis LPUSH: " << lpush_cmd.build() << std::endl;

    auto lrange_cmd = db.create_query_builder(database::database_types::redis)
        .lrange("notifications:123", 0, 10);
    std::cout << "Redis LRANGE: " << lrange_cmd.build() << std::endl;

    // 4. Set Operations
    auto sadd_cmd = db.create_query_builder(database::database_types::redis)
        .sadd("user:123:tags", "developer");
    std::cout << "Redis SADD: " << sadd_cmd.build() << std::endl;

    auto smembers_cmd = db.create_query_builder(database::database_types::redis)
        .smembers("user:123:tags");
    std::cout << "Redis SMEMBERS: " << smembers_cmd.build() << std::endl;
}
```

## Multi-Database Examples

### Database Abstraction Example

```cpp
void multi_database_example()
{
    database::database_manager& db = database::database_manager::handle();

    // Function to demonstrate same operation across different databases
    auto demonstrate_select = [&db](database::database_types db_type, const std::string& db_name) {
        std::cout << "\n--- " << db_name << " Example ---" << std::endl;

        auto query = db.create_query_builder(db_type);

        switch (db_type) {
            case database::database_types::postgres:
            case database::database_types::mysql:
            case database::database_types::sqlite:
                query.select({"id", "name", "email"})
                     .from("users")
                     .where("status", "=", database::database_value{std::string("active")})
                     .limit(5);
                break;

            case database::database_types::mongodb:
                query.collection("users")
                     .find({{"status", database::database_value{std::string("active")}}})
                     .project({"_id", "name", "email"})
                     .limit(5);
                break;

            case database::database_types::redis:
                query.smembers("active_users");
                break;

            default:
                return;
        }

        std::cout << "Query: " << query.build() << std::endl;
    };

    // Demonstrate across all database types
    demonstrate_select(database::database_types::postgres, "PostgreSQL");
    demonstrate_select(database::database_types::mysql, "MySQL");
    demonstrate_select(database::database_types::sqlite, "SQLite");
    demonstrate_select(database::database_types::mongodb, "MongoDB");
    demonstrate_select(database::database_types::redis, "Redis");
}
```

### Expected Output

```
--- PostgreSQL Example ---
Query: SELECT "id", "name", "email" FROM "users" WHERE status = 'active' LIMIT 5

--- MySQL Example ---
Query: SELECT `id`, `name`, `email` FROM `users` WHERE status = 'active' LIMIT 5

--- SQLite Example ---
Query: SELECT [id], [name], [email] FROM [users] WHERE status = 'active' LIMIT 5

--- MongoDB Example ---
Query: db.users.find({ "status": "active" }, { "_id": 1, "name": 1, "email": 1 }).limit(5)

--- Redis Example ---
Query: SMEMBERS active_users
```

## Best Practices

### 1. Error Handling

```cpp
try {
    database::database_manager& db = database::database_manager::handle();

    // Always check return values
    if (!db.set_mode(database::database_types::postgres)) {
        throw std::runtime_error("Failed to set database mode");
    }

    if (!db.connect(connection_string)) {
        throw std::runtime_error("Failed to connect to database");
    }

    // Use RAII for automatic cleanup
    auto result = db.select_query("SELECT * FROM users");

    // Process results...

} catch (const std::exception& e) {
    std::cerr << "Database error: " << e.what() << std::endl;
    // Handle error appropriately
}
```

### 2. Connection Pool Usage

```cpp
void efficient_pool_usage()
{
    database::database_manager& db = database::database_manager::handle();

    // Configure pool once
    database::connection_pool_config config;
    config.min_connections = 5;
    config.max_connections = 20;
    config.connection_string = "your_connection_string";

    db.create_connection_pool(database::database_types::postgres, config);

    // Use pool for multiple operations
    auto pool = db.get_connection_pool(database::database_types::postgres);

    {
        // Connection automatically returned to pool
        auto conn = pool->acquire_connection();
        if (conn) {
            auto result = conn->select_query("SELECT * FROM users");
            // Process result...
        }
    } // Connection returned here

    // Monitor pool health
    auto stats = db.get_pool_stats();
    if (stats[database::database_types::postgres].failed_acquisitions > 10) {
        // Consider pool reconfiguration
    }
}
```

### 3. Query Builder Best Practices

```cpp
void query_builder_best_practices()
{
    database::database_manager& db = database::database_manager::handle();

    // 1. Use typed values
    auto query = db.create_query_builder(database::database_types::postgres)
        .select({"id", "name", "created_at"})
        .from("users")
        .where("age", ">=", database::database_value{int64_t(18)})  // Typed value
        .where("active", "=", database::database_value{true})       // Boolean
        .where("name", "LIKE", database::database_value{std::string("%john%")});  // String

    // 2. Handle complex conditions
    auto complex_query = db.create_query_builder(database::database_types::postgres)
        .select({"*"})
        .from("orders")
        .where_raw("(status = 'pending' OR status = 'processing') AND amount > 100");

    // 3. Use appropriate database-specific features
    if (db.database_type() == database::database_types::postgres) {
        query.where_raw("metadata @> '{\"premium\": true}'");  // PostgreSQL JSON
    } else if (db.database_type() == database::database_types::mysql) {
        query.where_raw("JSON_EXTRACT(metadata, '$.premium') = true");  // MySQL JSON
    }

    // 4. Reset and reuse builders
    query.reset();
    query.select({"count(*)"})
         .from("users")
         .where("created_at", ">", database::database_value{std::string("2023-01-01")});
}
```

### 4. Performance Optimization

```cpp
void performance_optimization()
{
    database::database_manager& db = database::database_manager::handle();

    // 1. Use connection pooling for concurrent access
    database::connection_pool_config config;
    config.min_connections = 10;
    config.max_connections = 50;
    db.create_connection_pool(database::database_types::postgres, config);

    // 2. Batch operations when possible
    auto batch_insert = db.create_query_builder(database::database_types::postgres)
        .insert_into("logs")
        .values({
            {{"level", database::database_value{std::string("INFO")}},
             {"message", database::database_value{std::string("Operation 1")}}},
            {{"level", database::database_value{std::string("ERROR")}},
             {"message", database::database_value{std::string("Operation 2")}}},
            // ... more rows
        });

    // 3. Use appropriate indexes in DDL
    db.create_query("CREATE INDEX idx_users_email ON users(email)");
    db.create_query("CREATE INDEX idx_users_created_at ON users(created_at)");

    // 4. Monitor and tune based on statistics
    auto stats = db.get_pool_stats();
    for (const auto& [db_type, stat] : stats) {
        double success_rate = static_cast<double>(stat.successful_acquisitions) /
                             (stat.successful_acquisitions + stat.failed_acquisitions);

        if (success_rate < 0.95) {
            std::cout << "Warning: Low success rate (" << success_rate << "), consider increasing pool size" << std::endl;
        }
    }
}
```

---

These samples provide comprehensive examples of using the Database System in various scenarios. For more advanced use cases and specific database features, refer to the [API Reference](API_REFERENCE.md) and individual database documentation.