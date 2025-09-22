# Database System Samples

This directory contains comprehensive demonstration programs showcasing the capabilities of the Database System, including the advanced Phase 4 features: ORM Framework, Performance Monitoring, Security Framework, and Asynchronous Operations.

## Sample Programs Overview

### Core Database Features

#### `basic_usage_sample.cpp`
**Purpose**: Demonstrates fundamental database operations and multi-backend support
**Features Covered**:
- Basic CRUD operations (Create, Read, Update, Delete)
- Multi-database backend support (PostgreSQL, MySQL, SQLite, MongoDB, Redis)
- Connection management and error handling
- Query result processing

**Key Learning Points**:
- How to configure and connect to different database types
- Basic query execution patterns
- Error handling and resource management

#### `postgresql_advanced_sample.cpp`
**Purpose**: Advanced PostgreSQL-specific features and optimizations
**Features Covered**:
- Advanced PostgreSQL features (JSONB, arrays, custom types)
- Connection pooling configuration
- Transaction management and isolation levels
- Performance optimization techniques

**Key Learning Points**:
- PostgreSQL-specific data types and operations
- Advanced query patterns and optimization
- Production-ready connection management

#### `connection_pool_demo.cpp`
**Purpose**: Enterprise-grade connection pooling demonstration
**Features Covered**:
- Connection pool configuration and management
- Thread-safe connection sharing
- Pool health monitoring and statistics
- Automatic connection lifecycle management

**Key Learning Points**:
- Connection pool sizing and configuration
- Monitoring pool performance and utilization
- Handling connection failures and recovery

### Phase 4: Advanced Enterprise Features

#### `orm_framework_demo.cpp`
**Purpose**: C++20 concepts-based ORM framework demonstration
**Features Covered**:
- Entity definition with ENTITY_FIELD macros
- Compile-time type safety and constraint validation
- Automatic schema generation from entity definitions
- Entity lifecycle management (create, read, update, delete)
- Field metadata and constraint introspection

**Key Learning Points**:
- Modern C++20 concepts for type-safe entity definitions
- Automatic SQL schema generation from C++ classes
- Field constraints and validation patterns
- Entity relationship mapping and metadata management

```cpp
// Example: Entity Definition
class User : public entity_base {
    ENTITY_TABLE("users")

    ENTITY_FIELD(int64_t, id, primary_key() | auto_increment())
    ENTITY_FIELD(std::string, username, not_null() | unique())
    ENTITY_FIELD(std::string, email, not_null() | unique())

    ENTITY_METADATA()
};
```

#### `performance_monitoring_demo.cpp`
**Purpose**: Real-time performance monitoring and analysis
**Features Covered**:
- Real-time metrics collection (CPU, memory, disk, network)
- Query performance tracking and analysis
- Connection pool performance monitoring
- Slow query detection and alerting
- Performance trend analysis and reporting
- Metrics export (Prometheus, JSON, CSV formats)
- Configurable alerting system

**Key Learning Points**:
- Performance metrics collection strategies
- Real-time monitoring and alerting setup
- Performance bottleneck identification
- Integration with external monitoring systems

```cpp
// Example: Performance Monitoring
auto& monitor = performance_monitor::instance();
monitor.configure(monitoring_config);
monitor.record_query_execution(query_metrics);
auto report = monitor.generate_performance_report();
```

#### `security_framework_demo.cpp`
**Purpose**: Comprehensive enterprise security implementation
**Features Covered**:
- TLS/SSL connection encryption configuration
- Secure credential management with master key encryption
- Role-Based Access Control (RBAC) system
- Comprehensive audit logging with tamper-proof storage
- SQL injection prevention and threat detection
- Session management with timeout and validation
- Compliance support (GDPR, SOX, HIPAA, PCI DSS)

**Key Learning Points**:
- Enterprise security best practices
- RBAC implementation patterns
- Audit logging and compliance requirements
- Threat detection and prevention strategies

```cpp
// Example: RBAC Configuration
auto& rbac = rbac_manager::instance();
rbac_role admin_role("administrator");
admin_role.add_permission("user.create");
rbac.create_role(admin_role);
rbac.assign_role_to_user("alice", "administrator");
```

#### `async_operations_demo.cpp`
**Purpose**: Modern asynchronous operations with C++20 coroutines
**Features Covered**:
- std::future-based asynchronous database operations
- C++20 coroutine integration for non-blocking operations
- Asynchronous connection pool management
- Real-time data streams (PostgreSQL NOTIFY, MongoDB Change Streams)
- Distributed transaction coordination with two-phase commit
- Saga pattern for long-running transactions
- Asynchronous batch processing with progress tracking

**Key Learning Points**:
- Modern C++20 async programming patterns
- Non-blocking database operation design
- Distributed transaction management
- Event-driven architecture with real-time streams

```cpp
// Example: Async Operations with Coroutines
task async_database_operation() {
    auto result = co_await executor.execute_async([]() {
        // Database operation
        return query_result{};
    });
    co_return;
}
```

### Query Builder Demonstrations

#### `sql_query_builder_examples.cpp`
**Purpose**: Type-safe SQL query construction
**Features Covered**:
- Fluent API for SQL query building
- Type-safe parameter binding
- Complex query construction (JOINs, subqueries, aggregations)
- Query optimization and caching

#### `mongodb_query_builder_examples.cpp`
**Purpose**: MongoDB document operations and aggregation
**Features Covered**:
- Document CRUD operations
- Aggregation pipeline construction
- Index management and optimization
- GridFS for large file operations

#### `redis_query_builder_examples.cpp`
**Purpose**: Redis data structure operations
**Features Covered**:
- All Redis data types (strings, hashes, lists, sets, sorted sets)
- Pub/Sub messaging patterns
- Transaction and pipeline operations
- Lua script execution

### Multi-Database Examples

#### `multi_database_examples.cpp`
**Purpose**: Polyglot persistence patterns
**Features Covered**:
- Using multiple databases in a single application
- Data consistency across different database types
- Cross-database transaction coordination
- Database selection strategies

**Usage:**
```bash
# Run basic usage sample
./basic_usage_sample

# Run advanced samples
./postgresql_advanced_sample
./connection_pool_demo

# Run Phase 4 demonstrations
./orm_framework_demo
./performance_monitoring_demo
./security_framework_demo
./async_operations_demo
```

## Building the Samples

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16 or later
- Database System library
- PostgreSQL client library (libpq)
- PostgreSQL server (for actual database operations)

### Build Instructions

1. **From the main project directory:**
```bash
mkdir build && cd build
cmake .. -DBUILD_DATABASE_SAMPLES=ON
make
```

2. **Run samples:**
```bash
cd bin
./basic_usage
./postgres_advanced
./connection_pool_demo
./run_all_samples
```

### Alternative Build (samples only)
```bash
cd samples
mkdir build && cd build
cmake ..
make
```

## Database Setup

### PostgreSQL Setup for Samples
The samples expect a PostgreSQL database with the following configuration:

```sql
-- Create test database and user
CREATE DATABASE testdb;
CREATE USER testuser WITH PASSWORD 'testpass';
GRANT ALL PRIVILEGES ON DATABASE testdb TO testuser;
```

### Connection Configuration
Update the connection string in the samples if your PostgreSQL setup differs:
```cpp
std::string connection_string = "host=localhost port=5432 dbname=testdb user=testuser password=testpass";
```

### Running Without Database
The samples are designed to gracefully handle connection failures, showing:
- API usage patterns
- Expected behavior descriptions
- Error handling demonstrations

## Sample Output Examples

### Basic Usage Output
```
=== Database System - Basic Usage Example ===

1. Database Manager Setup:
Database type set to: PostgreSQL
Connection string configured

2. Connection Management:
✓ Successfully connected to database
Connection status: Connected

3. Table Operations:
✓ Users table created successfully

4. Data Insertion:
✓ User inserted successfully
✓ User inserted successfully
...
```

### PostgreSQL Advanced Features Output
```
=== Database System - PostgreSQL Advanced Features Example ===

1. PostgreSQL Manager Setup:
PostgreSQL manager created

2. Advanced Table Creation:
✓ Advanced products table created successfully
✓ Index created

3. PostgreSQL Array Operations:
✓ Product with arrays and JSON inserted
Products with 'gaming' tag:
Gaming Laptop | {gaming,laptop,computer}
...
```

### Connection Pool Demo Output
```
=== Database System - Connection Pool Demo ===

1. Single Connection Demo:
✓ Single connection established
✓ Test table ready

2. Multiple Connections Demo:
Creating 5 database connections...
✓ Connection 1 established
✓ Connection 2 established
...

3. Concurrent Access Demo:
Starting 4 concurrent threads...
Each thread will perform 50 operations

Concurrent access results:
  Successful connections: 4/4
  Total operations attempted: 200
  Successful operations: 195
  Success rate: 97.50%
  Total time: 2340 ms
  Operations per second: 83.33
...
```

## Understanding the Results

### Performance Metrics
- **Operations per second**: Higher is better for throughput
- **Connection success rate**: Should be close to 100%
- **Response time**: Lower is better for latency
- **Concurrent operation success**: Indicates thread safety

### PostgreSQL Features
- **Array Operations**: Demonstrates PostgreSQL's native array support
- **JSONB Queries**: Shows efficient JSON document storage and querying
- **Full-text Search**: PostgreSQL's powerful text search capabilities
- **Window Functions**: Advanced analytics and ranking functions

### Connection Management
- **Connection Pooling**: Efficient resource utilization
- **Health Monitoring**: Proactive connection maintenance
- **Resilience Testing**: Recovery from connection failures
- **Load Testing**: Performance under concurrent access

## Advanced Usage

### Customizing Database Configuration
Modify connection parameters in each sample:
```cpp
// For SSL connections
std::string connection_string = "host=localhost port=5432 dbname=testdb user=testuser password=testpass sslmode=require";

// For different PostgreSQL instance
std::string connection_string = "host=remote-db.example.com port=5432 dbname=proddb user=produser password=securepass";
```

### Performance Tuning
Adjust benchmark parameters in `connection_pool_demo.cpp`:
```cpp
const int num_threads = 8;        // Increase for more concurrency
const int operations_per_thread = 100;  // More operations per thread
const int load_operations = 1000; // Sustained load test size
```

### Adding New Samples
1. Create a new `.cpp` file in the samples directory
2. Add it to the `SAMPLE_PROGRAMS` list in `CMakeLists.txt`
3. Include it in the `run_all_samples.cpp` samples registry

## Troubleshooting

### Common Issues

1. **Connection Failures**
   ```
   ✗ Failed to connect to database
   ```
   - Verify PostgreSQL server is running
   - Check connection parameters (host, port, database, user, password)
   - Ensure database and user exist with proper permissions
   - Check firewall and network connectivity

2. **Permission Errors**
   ```
   ✗ Failed to create users table
   ```
   - Ensure user has CREATE TABLE privileges
   - Check schema permissions
   - Verify database ownership or privileges

3. **Compilation Errors**
   - Ensure C++20 support is enabled
   - Check that database system library is properly linked
   - Verify PostgreSQL client library (libpq) is installed
   - Ensure all required headers are included

4. **Runtime Library Errors**
   - Check that the database system library is built
   - Ensure proper library paths are set (LD_LIBRARY_PATH on Linux, PATH on Windows)
   - Verify PostgreSQL client library is available

### Performance Considerations
- Results may vary based on:
  - Database server performance
  - Network latency (for remote databases)
  - System hardware (CPU, memory, disk I/O)
  - Concurrent database load
  - PostgreSQL configuration and tuning

### Getting Help
- Check the main project README for detailed build instructions
- Review the API documentation for database system usage
- Examine the sample source code for implementation details
- Check PostgreSQL documentation for database-specific features

## PostgreSQL Feature Reference

### Supported PostgreSQL Features
- **Data Types**: SERIAL, VARCHAR, TEXT, INTEGER, DECIMAL, BOOLEAN, TIMESTAMP, ARRAY, JSONB
- **Indexes**: B-tree, GIN (for arrays and JSONB), full-text search indexes
- **Queries**: Complex SELECT with JOINs, CTEs, window functions, subqueries
- **Full-text Search**: tsvector, tsquery, ranking functions
- **JSON Operations**: JSONB storage, operators (->>, ->, @>), indexing
- **Array Operations**: Array construction, containment operators, ANY/ALL
- **Transactions**: BEGIN, COMMIT, ROLLBACK, savepoints
- **Advanced SQL**: Window functions, CTEs, advanced aggregations

### Sample Query Examples
The samples demonstrate various PostgreSQL features:

```sql
-- Array operations
SELECT name FROM products WHERE 'gaming' = ANY(tags);

-- JSONB queries
SELECT name, metadata->>'brand' FROM products WHERE metadata @> '{"warranty": "2 years"}';

-- Window functions
SELECT name, price, ROW_NUMBER() OVER (ORDER BY price DESC) FROM products;

-- Full-text search
SELECT name, ts_rank(search_vector, query) FROM products, plainto_tsquery('laptop') query WHERE search_vector @@ query;

-- Common Table Expressions
WITH expensive_products AS (SELECT * FROM products WHERE price > 100)
SELECT category_id, COUNT(*) FROM expensive_products GROUP BY category_id;
```

## License
These samples are provided under the same BSD 3-Clause License as the Database System project.