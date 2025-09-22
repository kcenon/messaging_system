# Database System API Reference

Complete API reference for the Database System C++20 library with multi-backend support, connection pooling, and query builders.

## Table of Contents

- [Core Classes](#core-classes)
- [Database Manager](#database-manager)
- [Connection Pooling](#connection-pooling)
- [Query Builders](#query-builders)
- [ORM Framework](#orm-framework)
- [Performance Monitoring](#performance-monitoring)
- [Security Framework](#security-framework)
- [Async Operations](#async-operations)
- [Database Types](#database-types)
- [Error Handling](#error-handling)
- [Examples](#examples)

## Core Classes

### database_base

Abstract base class for all database implementations.

```cpp
class database_base
{
public:
    virtual ~database_base() = default;

    // Database identification
    virtual database_types database_type() = 0;

    // Connection management
    virtual bool connect(const std::string& connection_string) = 0;
    virtual bool disconnect() = 0;

    // Query operations
    virtual bool create_query(const std::string& query_string) = 0;
    virtual unsigned int insert_query(const std::string& query_string) = 0;
    virtual unsigned int update_query(const std::string& query_string) = 0;
    virtual unsigned int delete_query(const std::string& query_string) = 0;
    virtual database_result select_query(const std::string& query_string) = 0;
};
```

### database_manager

Singleton class for managing database connections and operations.

```cpp
class database_manager
{
public:
    // Singleton access
    static database_manager& handle();

    // Database configuration
    bool set_mode(const database_types& database_type);
    database_types database_type();

    // Connection management
    bool connect(const std::string& connection_string);
    bool disconnect();

    // Query operations
    bool create_query(const std::string& query_string);
    unsigned int insert_query(const std::string& query_string);
    unsigned int update_query(const std::string& query_string);
    unsigned int delete_query(const std::string& query_string);
    database_result select_query(const std::string& query_string);

    // Phase 3: Advanced Features
    bool create_connection_pool(database_types db_type, const connection_pool_config& config);
    std::shared_ptr<connection_pool_base> get_connection_pool(database_types db_type);
    std::map<database_types, connection_stats> get_pool_stats() const;
    query_builder create_query_builder();
    query_builder create_query_builder(database_types db_type);

    // Phase 4: Enterprise Features
    bool execute_query(const std::string& query_string);
    std::shared_ptr<orm::entity_manager> get_entity_manager();
    std::shared_ptr<monitoring::performance_monitor> get_performance_monitor();
    std::shared_ptr<security::access_control> get_access_control();
    std::shared_ptr<async::async_database> get_async_database();
};
```

## Database Manager

### Basic Usage

```cpp
#include <database/database_manager.h>
using namespace database;

// Get singleton instance
database_manager& db = database_manager::handle();

// Set database type
db.set_mode(database_types::postgres);

// Connect
std::string conn_str = "host=localhost port=5432 dbname=test user=admin password=secret";
if (!db.connect(conn_str)) {
    // Handle connection error
}

// Execute queries
db.create_query("CREATE TABLE users (id SERIAL PRIMARY KEY, name VARCHAR(100))");
unsigned int rows = db.insert_query("INSERT INTO users (name) VALUES ('John')");
database_result result = db.select_query("SELECT * FROM users");
```

### Supported Methods

| Method | Description | Returns |
|--------|-------------|---------|
| `set_mode(database_types)` | Set database backend type | `bool` success |
| `database_type()` | Get current database type | `database_types` |
| `connect(connection_string)` | Connect to database | `bool` success |
| `disconnect()` | Disconnect from database | `bool` success |
| `create_query(query)` | Execute DDL query | `bool` success |
| `insert_query(query)` | Execute INSERT query | `unsigned int` rows affected |
| `update_query(query)` | Execute UPDATE query | `unsigned int` rows affected |
| `delete_query(query)` | Execute DELETE query | `unsigned int` rows affected |
| `select_query(query)` | Execute SELECT query | `database_result` |

## Connection Pooling

### connection_pool_config

Configuration structure for connection pools.

```cpp
struct connection_pool_config
{
    size_t min_connections = 2;                              // Minimum connections to maintain
    size_t max_connections = 20;                             // Maximum connections allowed
    std::chrono::milliseconds acquire_timeout{5000};         // Timeout for acquiring connections
    std::chrono::milliseconds idle_timeout{30000};           // Timeout for idle connections
    std::chrono::milliseconds health_check_interval{60000};   // Health check interval
    bool enable_health_checks = true;                        // Enable periodic health checks
    std::string connection_string;                           // Database connection string
};
```

### connection_stats

Statistics structure for monitoring connection pools.

```cpp
struct connection_stats
{
    size_t total_connections = 0;                             // Total connections created
    size_t active_connections = 0;                            // Currently active connections
    size_t available_connections = 0;                         // Available connections in pool
    size_t failed_acquisitions = 0;                           // Number of failed acquisitions
    size_t successful_acquisitions = 0;                       // Number of successful acquisitions
    std::chrono::steady_clock::time_point last_health_check;  // Last health check time
};
```

### connection_pool_base

Abstract base class for connection pools.

```cpp
class connection_pool_base
{
public:
    virtual ~connection_pool_base() = default;

    virtual std::shared_ptr<connection_wrapper> acquire_connection() = 0;
    virtual void release_connection(std::shared_ptr<connection_wrapper> connection) = 0;
    virtual size_t active_connections() const = 0;
    virtual size_t available_connections() const = 0;
    virtual connection_stats get_stats() const = 0;
    virtual void shutdown() = 0;
};
```

### Usage Example

```cpp
#include <database/database_manager.h>
#include <database/connection_pool.h>

// Configure connection pool
connection_pool_config config;
config.min_connections = 5;
config.max_connections = 20;
config.acquire_timeout = std::chrono::seconds(5);
config.connection_string = "host=localhost port=5432 dbname=test user=admin password=secret";

// Create pool
database_manager& db = database_manager::handle();
if (!db.create_connection_pool(database_types::postgres, config)) {
    // Handle error
}

// Use pool
auto pool = db.get_connection_pool(database_types::postgres);
auto connection = pool->acquire_connection();

if (connection) {
    // Use connection
    auto result = connection->select_query("SELECT * FROM users");

    // Connection automatically returned to pool when destroyed
}

// Monitor statistics
auto stats = db.get_pool_stats();
for (const auto& [db_type, stat] : stats) {
    std::cout << "Active: " << stat.active_connections
              << " Available: " << stat.available_connections << std::endl;
}
```

## Query Builders

### query_builder

Universal query builder that adapts to different database types.

```cpp
class query_builder
{
public:
    explicit query_builder(database_types db_type = database_types::none);

    // Database type selection
    query_builder& for_database(database_types db_type);

    // SQL-style interface (PostgreSQL, MySQL, SQLite)
    query_builder& select(const std::vector<std::string>& columns);
    query_builder& from(const std::string& table);
    query_builder& where(const std::string& field, const std::string& op, const database_value& value);
    query_builder& join(const std::string& table, const std::string& condition);
    query_builder& order_by(const std::string& column, sort_order order = sort_order::asc);
    query_builder& limit(size_t count);

    // NoSQL-style interface
    query_builder& collection(const std::string& name); // MongoDB
    query_builder& key(const std::string& key);         // Redis

    // Universal operations
    query_builder& insert(const std::map<std::string, database_value>& data);
    query_builder& update(const std::map<std::string, database_value>& data);
    query_builder& remove(); // DELETE/DROP

    // Build and execute
    std::string build() const;
    database_result execute(database_base* db) const;

    // Reset builder
    void reset();
};
```

### sql_query_builder

Specialized query builder for SQL databases.

```cpp
class sql_query_builder
{
public:
    // SELECT operations
    sql_query_builder& select(const std::vector<std::string>& columns);
    sql_query_builder& select(const std::string& column);
    sql_query_builder& select_raw(const std::string& raw_select);
    sql_query_builder& from(const std::string& table);

    // WHERE conditions
    sql_query_builder& where(const std::string& field, const std::string& op, const database_value& value);
    sql_query_builder& where(const query_condition& condition);
    sql_query_builder& where_raw(const std::string& raw_where);
    sql_query_builder& or_where(const std::string& field, const std::string& op, const database_value& value);

    // JOIN operations
    sql_query_builder& join(const std::string& table, const std::string& condition, join_type type = join_type::inner);
    sql_query_builder& left_join(const std::string& table, const std::string& condition);
    sql_query_builder& right_join(const std::string& table, const std::string& condition);

    // GROUP BY and HAVING
    sql_query_builder& group_by(const std::vector<std::string>& columns);
    sql_query_builder& group_by(const std::string& column);
    sql_query_builder& having(const std::string& condition);

    // ORDER BY
    sql_query_builder& order_by(const std::string& column, sort_order order = sort_order::asc);
    sql_query_builder& order_by_raw(const std::string& raw_order);

    // LIMIT and OFFSET
    sql_query_builder& limit(size_t count);
    sql_query_builder& offset(size_t count);

    // INSERT operations
    sql_query_builder& insert_into(const std::string& table);
    sql_query_builder& values(const std::map<std::string, database_value>& data);
    sql_query_builder& values(const std::vector<std::map<std::string, database_value>>& rows);

    // UPDATE operations
    sql_query_builder& update(const std::string& table);
    sql_query_builder& set(const std::string& field, const database_value& value);
    sql_query_builder& set(const std::map<std::string, database_value>& data);

    // DELETE operations
    sql_query_builder& delete_from(const std::string& table);

    // Build final query
    std::string build() const;
    std::string build_for_database(database_types db_type) const;

    // Reset builder
    void reset();
};
```

### mongodb_query_builder

Specialized query builder for MongoDB.

```cpp
class mongodb_query_builder
{
public:
    // Collection operations
    mongodb_query_builder& collection(const std::string& name);

    // Find operations
    mongodb_query_builder& find(const std::map<std::string, database_value>& filter = {});
    mongodb_query_builder& find_one(const std::map<std::string, database_value>& filter = {});

    // Projection
    mongodb_query_builder& project(const std::vector<std::string>& fields);
    mongodb_query_builder& exclude(const std::vector<std::string>& fields);

    // Sorting
    mongodb_query_builder& sort(const std::map<std::string, int>& sort_spec);
    mongodb_query_builder& sort(const std::string& field, int direction = 1);

    // Limit and Skip
    mongodb_query_builder& limit(size_t count);
    mongodb_query_builder& skip(size_t count);

    // Insert operations
    mongodb_query_builder& insert_one(const std::map<std::string, database_value>& document);
    mongodb_query_builder& insert_many(const std::vector<std::map<std::string, database_value>>& documents);

    // Update operations
    mongodb_query_builder& update_one(const std::map<std::string, database_value>& filter,
                                     const std::map<std::string, database_value>& update);
    mongodb_query_builder& update_many(const std::map<std::string, database_value>& filter,
                                      const std::map<std::string, database_value>& update);

    // Delete operations
    mongodb_query_builder& delete_one(const std::map<std::string, database_value>& filter);
    mongodb_query_builder& delete_many(const std::map<std::string, database_value>& filter);

    // Aggregation pipeline
    mongodb_query_builder& match(const std::map<std::string, database_value>& conditions);
    mongodb_query_builder& group(const std::map<std::string, database_value>& group_spec);
    mongodb_query_builder& unwind(const std::string& field);

    // Build final query
    std::string build() const;
    std::string build_json() const;

    // Reset builder
    void reset();
};
```

### redis_query_builder

Specialized query builder for Redis.

```cpp
class redis_query_builder
{
public:
    // String operations
    redis_query_builder& set(const std::string& key, const std::string& value);
    redis_query_builder& get(const std::string& key);
    redis_query_builder& del(const std::string& key);
    redis_query_builder& exists(const std::string& key);

    // Hash operations
    redis_query_builder& hset(const std::string& key, const std::string& field, const std::string& value);
    redis_query_builder& hget(const std::string& key, const std::string& field);
    redis_query_builder& hdel(const std::string& key, const std::string& field);
    redis_query_builder& hgetall(const std::string& key);

    // List operations
    redis_query_builder& lpush(const std::string& key, const std::string& value);
    redis_query_builder& rpush(const std::string& key, const std::string& value);
    redis_query_builder& lpop(const std::string& key);
    redis_query_builder& rpop(const std::string& key);
    redis_query_builder& lrange(const std::string& key, int start, int stop);

    // Set operations
    redis_query_builder& sadd(const std::string& key, const std::string& member);
    redis_query_builder& srem(const std::string& key, const std::string& member);
    redis_query_builder& sismember(const std::string& key, const std::string& member);
    redis_query_builder& smembers(const std::string& key);

    // Expiration
    redis_query_builder& expire(const std::string& key, int seconds);
    redis_query_builder& ttl(const std::string& key);

    // Build command
    std::string build() const;
    std::vector<std::string> build_args() const;

    // Reset builder
    void reset();
};
```

### Query Builder Examples

```cpp
// SQL Query Builder
auto sql_query = db.create_query_builder(database_types::postgres)
    .select({"name", "email", "created_at"})
    .from("users")
    .where("age", ">", database_value{int64_t(18)})
    .where("status", "=", database_value{std::string("active")})
    .order_by("created_at", sort_order::desc)
    .limit(10);

std::string query = sql_query.build();
// Output: SELECT "name", "email", "created_at" FROM "users" WHERE age > 18 AND status = 'active' ORDER BY created_at DESC LIMIT 10

// MongoDB Query Builder
auto mongo_query = db.create_query_builder(database_types::mongodb)
    .collection("users")
    .find({{"status", database_value{std::string("active")}}})
    .sort("created_at", -1)
    .limit(10);

std::string mongo_cmd = mongo_query.build();
// Output: db.users.find({ "status": "active" }).sort({"created_at": -1}).limit(10)

// Redis Query Builder
auto redis_query = db.create_query_builder(database_types::redis)
    .hget("user:123", "email");

std::string redis_cmd = redis_query.build();
// Output: HGET user:123 email
```

## Database Types

### database_types

Enumeration of supported database types.

```cpp
enum class database_types : uint8_t
{
    none = 0,           // No database backend
    postgres = 1,       // PostgreSQL backend
    mysql = 2,          // MySQL/MariaDB backend
    sqlite = 3,         // SQLite backend
    oracle = 4,         // Oracle backend (future)
    mongodb = 5,        // MongoDB backend
    redis = 6           // Redis backend
};
```

### database_value

Variant type for database values.

```cpp
using database_value = std::variant<std::string, int64_t, double, bool, std::monostate>;
```

### database_result

Type definitions for database results.

```cpp
using database_row = std::map<std::string, database_value>;
using database_result = std::vector<database_row>;
```

### Working with database_value

```cpp
// Creating values
database_value str_val{std::string("hello")};
database_value int_val{int64_t(42)};
database_value double_val{3.14};
database_value bool_val{true};
database_value null_val{std::monostate{}};

// Visiting values
std::visit([](const auto& value) {
    using T = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "String: " << value << std::endl;
    } else if constexpr (std::is_same_v<T, int64_t>) {
        std::cout << "Integer: " << value << std::endl;
    } else if constexpr (std::is_same_v<T, double>) {
        std::cout << "Double: " << value << std::endl;
    } else if constexpr (std::is_same_v<T, bool>) {
        std::cout << "Boolean: " << (value ? "true" : "false") << std::endl;
    } else if constexpr (std::is_same_v<T, std::monostate>) {
        std::cout << "NULL" << std::endl;
    }
}, str_val);
```

## Error Handling

### Exception Safety

All database operations are exception-safe with RAII resource management.

```cpp
try {
    database_manager& db = database_manager::handle();

    if (!db.set_mode(database_types::postgres)) {
        throw std::runtime_error("Failed to set database mode");
    }

    if (!db.connect(connection_string)) {
        throw std::runtime_error("Failed to connect to database");
    }

    auto result = db.select_query("SELECT * FROM users");
    // Process result

} catch (const std::exception& e) {
    std::cerr << "Database error: " << e.what() << std::endl;
}
```

### Mock Implementations

When database libraries are not available, the system provides mock implementations that return empty results but don't throw exceptions.

```cpp
// Even without PostgreSQL libraries, this won't crash
database_manager& db = database_manager::handle();
db.set_mode(database_types::postgres);

if (!db.connect("mock://connection")) {
    // This will fail gracefully with mock implementation
    std::cout << "Mock connection - no actual database required" << std::endl;
}

auto result = db.select_query("SELECT * FROM users");
// Returns empty result with mock implementation
```

## Examples

### Complete Usage Example

```cpp
#include <database/database_manager.h>
#include <database/connection_pool.h>
#include <database/query_builder.h>
#include <iostream>

int main() {
    try {
        // Initialize database manager
        database_manager& db = database_manager::handle();

        // Configure PostgreSQL
        if (!db.set_mode(database_types::postgres)) {
            std::cerr << "Failed to set database mode" << std::endl;
            return 1;
        }

        // Setup connection pool
        connection_pool_config config;
        config.min_connections = 2;
        config.max_connections = 10;
        config.connection_string = "host=localhost port=5432 dbname=test user=admin password=secret";

        if (!db.create_connection_pool(database_types::postgres, config)) {
            std::cerr << "Failed to create connection pool" << std::endl;
            return 1;
        }

        // Create table using query builder
        auto create_table = db.create_query_builder()
            .create_raw("CREATE TABLE IF NOT EXISTS users ("
                       "id SERIAL PRIMARY KEY, "
                       "name VARCHAR(100) NOT NULL, "
                       "email VARCHAR(100) UNIQUE, "
                       "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");

        // Insert data
        auto insert_query = db.create_query_builder()
            .insert_into("users")
            .values({
                {"name", database_value{std::string("John Doe")}},
                {"email", database_value{std::string("john@example.com")}}
            });

        // Select data with conditions
        auto select_query = db.create_query_builder()
            .select({"id", "name", "email", "created_at"})
            .from("users")
            .where("name", "LIKE", database_value{std::string("%John%")})
            .order_by("created_at", sort_order::desc)
            .limit(10);

        // Execute queries
        auto pool = db.get_connection_pool(database_types::postgres);
        auto connection = pool->acquire_connection();

        if (connection) {
            // Execute through connection
            auto result = select_query.execute(connection.get());

            // Process results
            for (const auto& row : result) {
                for (const auto& [column, value] : row) {
                    std::cout << column << ": ";
                    std::visit([](const auto& v) {
                        using T = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<T, std::monostate>) {
                            std::cout << "NULL";
                        } else {
                            std::cout << v;
                        }
                    }, value);
                    std::cout << " ";
                }
                std::cout << std::endl;
            }
        }

        // Monitor pool statistics
        auto stats = db.get_pool_stats();
        for (const auto& [db_type, stat] : stats) {
            std::cout << "Pool Stats - Active: " << stat.active_connections
                      << " Available: " << stat.available_connections << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### Multi-Database Example

```cpp
#include <database/database_manager.h>
#include <database/query_builder.h>

int main() {
    database_manager& db = database_manager::handle();

    // PostgreSQL operations
    auto pg_query = db.create_query_builder(database_types::postgres)
        .select({"id", "name"})
        .from("users")
        .where("status", "=", database_value{std::string("active")});

    std::cout << "PostgreSQL: " << pg_query.build() << std::endl;

    // MySQL operations (different identifier quoting)
    auto mysql_query = db.create_query_builder(database_types::mysql)
        .select({"id", "name"})
        .from("users")
        .where("status", "=", database_value{std::string("active")});

    std::cout << "MySQL: " << mysql_query.build() << std::endl;

    // MongoDB operations
    auto mongo_query = db.create_query_builder(database_types::mongodb)
        .collection("users")
        .find({{"status", database_value{std::string("active")}}})
        .project({"_id", "name"});

    std::cout << "MongoDB: " << mongo_query.build() << std::endl;

    // Redis operations
    auto redis_query = db.create_query_builder(database_types::redis)
        .hgetall("user:123");

    std::cout << "Redis: " << redis_query.build() << std::endl;

    return 0;
}
```

---

## Phase 4: Enterprise APIs

### ORM Framework

```cpp
#include <database/orm/entity.h>

// Entity definition
class User : public entity_base {
    ENTITY_TABLE("users")
    ENTITY_FIELD(int64_t, id, primary_key() | auto_increment())
    ENTITY_FIELD(std::string, username, not_null() | index("idx_username"))
    ENTITY_FIELD(std::string, email, unique())
    ENTITY_METADATA()
};

// Entity operations
entity_manager::instance().create_tables(db);
auto users = User::query(db).where("age > 18").execute();
```

### Performance Monitoring

```cpp
#include <database/monitoring/performance_monitor.h>

// Performance monitoring
auto& monitor = performance_monitor::instance();
monitor.set_alert_thresholds(0.05, std::chrono::milliseconds(1000));
auto summary = monitor.get_performance_summary();
```

### Security Framework

```cpp
#include <database/security/secure_connection.h>

// Access control
auto& access = access_control::instance();
access.create_role(admin_role);
bool allowed = access.check_permission("user123", "users", "SELECT");

// Audit logging
AUDIT_LOG_ACCESS("user123", "session456", "SELECT", "users", "query_hash", true, "");
```

### Async Operations

```cpp
#include <database/async/async_operations.h>

// Coroutine support
database_awaitable<bool> async_operation() {
    auto result = co_await async_db.execute_coro("SELECT * FROM users");
    co_return result;
}

// Distributed transactions
auto& coordinator = transaction_coordinator::instance();
auto tx_id = coordinator.begin_distributed_transaction({db1, db2});
```

---

## System Requirements

- **C++ Standard**: C++20
- **Supported Compilers**: GCC 10+, Clang 11+, MSVC 2019+
- **Supported Platforms**: Windows, macOS, Linux

For the latest API updates and changes, see the [CHANGELOG](../CHANGELOG.md).