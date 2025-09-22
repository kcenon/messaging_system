# API Reference

Complete API documentation for all Messaging System components and their integration.

## Table of Contents

1. [Container System API](#container-system-api)
2. [Network System API](#network-system-api)
3. [Database System API](#database-system-api)
4. [Message Bus API](#message-bus-api)
5. [Service Container API](#service-container-api)
6. [Thread System API](#thread-system-api)
7. [Logger System API](#logger-system-api)
8. [Monitoring System API](#monitoring-system-api)
9. [Error Codes](#error-codes)

## Component Documentation Links

For detailed documentation on individual components:

- **[Thread System API](../libraries/thread_system/docs/api-reference.md)** - Lock-free thread pools and concurrent processing
- **[Logger System API](../libraries/logger_system/docs/api-reference.md)** - High-performance logging framework
- **[Monitoring System API](../libraries/monitoring_system/docs/api-reference.md)** - Real-time system monitoring
- **[Container System](../libraries/container_system/README.md#api-reference)** - Type-safe data containers
- **[Database System](../libraries/database_system/README.md#api-reference)** - PostgreSQL integration
- **[Network System](../libraries/network_system/README.md#api-reference)** - Asynchronous TCP messaging

---

## Container System API

### Namespace: `container_module`

### Class: `value_container`

Thread-safe container for storing typed values with serialization support.

#### Constructor
```cpp
value_container();
value_container(const std::string& serialized_data);
```

#### Core Methods

##### Setting Values
```cpp
void set_source(const std::string& source_id, const std::string& source_sub = "");
void set_target(const std::string& target_id, const std::string& target_sub = "");
void set_message_type(const std::string& type);
void set_message_id(const std::string& id);
```

##### Adding Values
```cpp
void add_value(const std::string& key, value_types type, const std::string& value);
void add_value(const std::string& key, const variant& value);
void add_values(const std::vector<std::pair<std::string, variant>>& values);
```

##### Retrieving Values
```cpp
std::optional<variant> get_value(const std::string& key) const;
template<typename T> std::optional<T> get_value_as(const std::string& key) const;
std::vector<std::string> get_keys() const;
bool has_key(const std::string& key) const;
```

##### Serialization
```cpp
std::string serialize(serialization_format format = serialization_format::binary) const;
bool deserialize(const std::string& data, serialization_format format = serialization_format::binary);
```

#### Usage Example
```cpp
#include <container/container.h>
using namespace container_module;

// Create container
auto container = std::make_shared<value_container>();

// Set metadata
container->set_source("client_01", "session_123");
container->set_target("server", "main");
container->set_message_type("data_update");

// Add typed values
container->add_value("user_id", value_types::int32_type, "12345");
container->add_value("temperature", value_types::float_type, "23.5");
container->add_value("message", value_types::string_type, "Hello World");
container->add_value("binary_data", value_types::bytes_type, binary_string);

// Retrieve values
auto user_id = container->get_value_as<int32_t>("user_id");
if (user_id.has_value()) {
    std::cout << "User ID: " << user_id.value() << std::endl;
}

// Serialize for transmission
std::string serialized = container->serialize();

// Deserialize received data
auto received = std::make_shared<value_container>();
received->deserialize(serialized);
```

### Class: `variant`

Type-safe variant for storing different value types.

#### Supported Types
- `bool`
- `int8_t`, `int16_t`, `int32_t`, `int64_t`
- `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`
- `float`, `double`
- `std::string`
- `std::vector<uint8_t>` (bytes)
- `std::shared_ptr<value_container>` (nested)

#### Methods
```cpp
template<typename T> T get() const;
template<typename T> bool is() const;
value_types type() const;
std::string to_string() const;
```

---

## Network System API

### Namespace: `network_module`

### Class: `messaging_server`

Asynchronous TCP server for handling client connections.

#### Constructor
```cpp
explicit messaging_server(const std::string& server_id);
```

#### Methods
```cpp
bool start_server(uint16_t port, const std::string& address = "0.0.0.0");
void stop_server();
bool is_running() const;

// Session management
std::vector<std::string> get_connected_clients() const;
bool send_to_client(const std::string& client_id, const message_ptr& msg);
bool broadcast(const message_ptr& msg, const std::string& exclude_client = "");

// Callbacks
void set_message_handler(message_callback handler);
void set_connection_handler(connection_callback handler);
void set_disconnection_handler(disconnection_callback handler);
```

#### Usage Example
```cpp
#include <network/messaging_server.h>
using namespace network_module;

// Create server
auto server = std::make_shared<messaging_server>("main_server");

// Set up handlers
server->set_message_handler([](const std::string& client_id, const message_ptr& msg) {
    std::cout << "Received from " << client_id << ": " << msg->serialize() << std::endl;
    return true;
});

server->set_connection_handler([](const std::string& client_id) {
    std::cout << "Client connected: " << client_id << std::endl;
});

// Start server
if (server->start_server(8080)) {
    std::cout << "Server running on port 8080" << std::endl;
}

// Broadcast message
auto msg = std::make_shared<value_container>();
msg->set_message_type("announcement");
msg->add_value("text", value_types::string_type, "Server maintenance in 5 minutes");
server->broadcast(msg);

// Graceful shutdown
server->stop_server();
```

### Class: `messaging_client`

Asynchronous TCP client for connecting to servers.

#### Constructor
```cpp
explicit messaging_client(const std::string& client_id);
```

#### Methods
```cpp
bool start_client(const std::string& host, uint16_t port);
void stop_client();
bool is_connected() const;

// Message operations
bool send_message(const message_ptr& msg);
std::future<message_ptr> request(const message_ptr& request,
                                 std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

// Callbacks
void set_message_handler(message_callback handler);
void set_connection_handler(connection_callback handler);
void set_disconnection_handler(disconnection_callback handler);
```

#### Usage Example
```cpp
#include <network/messaging_client.h>
using namespace network_module;

// Create client
auto client = std::make_shared<messaging_client>("client_01");

// Set up handlers
client->set_message_handler([](const std::string& server_id, const message_ptr& msg) {
    std::cout << "Received: " << msg->serialize() << std::endl;
    return true;
});

// Connect to server
if (client->start_client("localhost", 8080)) {
    std::cout << "Connected to server" << std::endl;
}

// Send message
auto msg = std::make_shared<value_container>();
msg->set_message_type("query");
msg->add_value("query_type", value_types::string_type, "get_status");
client->send_message(msg);

// Request-response pattern
auto request = std::make_shared<value_container>();
request->set_message_type("request");
request->add_value("action", value_types::string_type, "get_user_info");

auto future = client->request(request);
if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
    auto response = future.get();
    std::cout << "Response: " << response->serialize() << std::endl;
}

// Disconnect
client->stop_client();
```

---

## Database System API

### Namespace: `database`

### Class: `database_manager`

Thread-safe database connection manager with pooling support.

#### Constructor
```cpp
database_manager();
```

#### Configuration
```cpp
void set_mode(database_types mode);
void set_pool_size(size_t size);
void set_connection_timeout(std::chrono::milliseconds timeout);
void enable_prepared_statements(bool enable);
```

#### Connection Management
```cpp
bool connect(const std::string& connection_string);
void disconnect();
bool is_connected() const;
bool check_connection();
```

#### Query Execution
```cpp
// SELECT queries
query_result select_query(const std::string& query,
                         const std::vector<query_parameter>& params = {});

// INSERT/UPDATE/DELETE
bool execute_query(const std::string& query,
                  const std::vector<query_parameter>& params = {});

// Batch operations
bool execute_batch(const std::vector<std::string>& queries);

// Prepared statements
prepared_statement prepare(const std::string& query);
query_result execute_prepared(const prepared_statement& stmt,
                             const std::vector<query_parameter>& params);
```

#### Transaction Support
```cpp
bool begin_transaction();
bool commit_transaction();
bool rollback_transaction();

// Transaction with callback
template<typename Func>
bool transaction(Func&& func);
```

#### Usage Example
```cpp
#include <database/database_manager.h>
using namespace database;

// Create and configure manager
database_manager db;
db.set_mode(database_types::postgres);
db.set_pool_size(10);

// Connect to database
if (db.connect("host=localhost dbname=myapp user=admin password=secret")) {
    std::cout << "Connected to database" << std::endl;
}

// Execute SELECT query
auto result = db.select_query("SELECT * FROM users WHERE age > $1", {25});
for (const auto& row : result) {
    std::cout << "User: " << row["name"].as<std::string>()
              << ", Age: " << row["age"].as<int>() << std::endl;
}

// Execute INSERT with prepared statement
auto stmt = db.prepare("INSERT INTO users (name, email, age) VALUES ($1, $2, $3)");
db.execute_prepared(stmt, {"John Doe", "john@example.com", 30});

// Transaction example
bool success = db.transaction([&db]() {
    db.execute_query("UPDATE accounts SET balance = balance - 100 WHERE id = 1");
    db.execute_query("UPDATE accounts SET balance = balance + 100 WHERE id = 2");
    return true;  // Commit
});

// Batch operations
std::vector<std::string> batch = {
    "INSERT INTO logs (message, timestamp) VALUES ('Start', NOW())",
    "UPDATE stats SET count = count + 1",
    "INSERT INTO logs (message, timestamp) VALUES ('End', NOW())"
};
db.execute_batch(batch);

// Disconnect
db.disconnect();
```

---

## Message Bus API

### Namespace: `kcenon::messaging::core`

### Class: `message_bus`

Central message routing and distribution system.

#### Constructor
```cpp
explicit message_bus(const message_bus_config& config = {});
```

#### Configuration Structure
```cpp
struct message_bus_config {
    size_t worker_threads = 4;
    size_t max_queue_size = 10000;
    std::chrono::milliseconds processing_timeout{30000};
    bool enable_priority_queue = true;
    bool enable_message_persistence = false;
    bool enable_metrics = true;
};
```

#### Lifecycle Methods
```cpp
bool initialize();
void shutdown();
bool is_running() const;
```

#### Publishing
```cpp
bool publish(const message& msg);
bool publish(const std::string& topic, const message_payload& payload,
            const std::string& sender = "");
```

#### Subscription
```cpp
void subscribe(const std::string& topic, message_handler handler);
void unsubscribe(const std::string& topic, const message_handler& handler);
void unsubscribe_all(const std::string& topic);
```

#### Request-Response
```cpp
std::future<message> request(const message& request_msg);
void respond(const message& original_msg, const message& response_msg);
```

#### Monitoring
```cpp
struct statistics_snapshot {
    uint64_t messages_published;
    uint64_t messages_processed;
    uint64_t messages_failed;
    uint64_t active_subscriptions;
    uint64_t pending_requests;
};

statistics_snapshot get_statistics() const;
void reset_statistics();
```

#### Usage Example
```cpp
#include <kcenon/messaging/core/message_bus.h>
using namespace kcenon::messaging::core;

// Configure and create message bus
message_bus_config config;
config.worker_threads = 8;
config.max_queue_size = 50000;
config.enable_metrics = true;

message_bus bus(config);
bus.initialize();

// Subscribe to topics
bus.subscribe("user.created", [](const message& msg) {
    auto payload = msg.get_payload();
    std::cout << "New user: " << payload.get<std::string>("username") << std::endl;
    return message_status::processed;
});

bus.subscribe("order.*", [](const message& msg) {
    std::cout << "Order event: " << msg.get_topic() << std::endl;
    return message_status::processed;
});

// Publish messages
message_payload user_payload;
user_payload.set("username", "john_doe");
user_payload.set("email", "john@example.com");
bus.publish("user.created", user_payload);

// Request-response pattern
message request;
request.set_topic("service.query");
request.set_payload({{"query", "get_status"}});

auto future = bus.request(request);
auto response = future.get();
std::cout << "Service status: " << response.get_payload().get<std::string>("status") << std::endl;

// Get statistics
auto stats = bus.get_statistics();
std::cout << "Messages processed: " << stats.messages_processed << std::endl;

// Shutdown
bus.shutdown();
```

---

## Service Container API

### Namespace: `kcenon::messaging::services`

### Class: `service_container`

Dependency injection container for service management.

#### Methods
```cpp
// Service registration
template<typename Interface, typename Implementation>
void register_service(service_lifetime lifetime = service_lifetime::singleton);

template<typename Interface>
void register_factory(std::function<std::shared_ptr<Interface>()> factory,
                     service_lifetime lifetime = service_lifetime::singleton);

// Service resolution
template<typename Interface>
std::shared_ptr<Interface> resolve();

template<typename Interface>
std::vector<std::shared_ptr<Interface>> resolve_all();

// Lifecycle management
void initialize_all();
void shutdown_all();
```

#### Service Lifetimes
```cpp
enum class service_lifetime {
    singleton,   // Single instance for application lifetime
    transient,   // New instance for each request
    scoped       // Single instance per scope
};
```

#### Usage Example
```cpp
#include <kcenon/messaging/services/service_container.h>
using namespace kcenon::messaging::services;

// Define interfaces
class ILogger {
public:
    virtual void log(const std::string& message) = 0;
    virtual ~ILogger() = default;
};

class IDatabase {
public:
    virtual bool connect() = 0;
    virtual ~IDatabase() = default;
};

// Implement services
class ConsoleLogger : public ILogger {
    void log(const std::string& message) override {
        std::cout << "[LOG] " << message << std::endl;
    }
};

class PostgresDatabase : public IDatabase {
    bool connect() override {
        // Database connection logic
        return true;
    }
};

// Configure container
service_container container;

// Register services
container.register_service<ILogger, ConsoleLogger>(service_lifetime::singleton);
container.register_service<IDatabase, PostgresDatabase>(service_lifetime::singleton);

// Register factory
container.register_factory<ILogger>([]() {
    return std::make_shared<ConsoleLogger>();
}, service_lifetime::transient);

// Resolve services
auto logger = container.resolve<ILogger>();
logger->log("Application started");

auto db = container.resolve<IDatabase>();
if (db->connect()) {
    logger->log("Database connected");
}

// Initialize all services
container.initialize_all();

// Shutdown when done
container.shutdown_all();
```

---

## Thread System API

### Namespace: `thread_system`

### Class: `thread_pool`

Lock-free thread pool with priority scheduling.

#### Constructor
```cpp
explicit thread_pool(size_t num_threads = std::thread::hardware_concurrency());
thread_pool(size_t num_threads, pool_priority priority);
```

#### Job Submission
```cpp
template<typename Func>
auto submit(Func&& func) -> std::future<decltype(func())>;

template<typename Func>
auto submit_with_priority(Func&& func, job_priority priority)
    -> std::future<decltype(func())>;

template<typename Func>
void submit_detached(Func&& func);
```

#### Pool Management
```cpp
void start();
void stop();
void wait_for_all();
size_t pending_jobs() const;
size_t active_threads() const;
```

#### Priority Levels
```cpp
enum class job_priority {
    real_time = 0,
    high = 1,
    normal = 2,
    low = 3,
    background = 4
};

enum class pool_priority {
    real_time,
    batch,
    background
};
```

#### Usage Example
```cpp
#include <thread_system/thread_pool.h>
using namespace thread_system;

// Create thread pool
thread_pool pool(8, pool_priority::batch);
pool.start();

// Submit jobs with futures
auto future1 = pool.submit([]() {
    // Compute intensive task
    return calculate_result();
});

auto future2 = pool.submit_with_priority([]() {
    // High priority task
    return process_urgent_data();
}, job_priority::high);

// Submit fire-and-forget jobs
pool.submit_detached([]() {
    // Background task
    cleanup_temporary_files();
});

// Batch processing
std::vector<std::future<int>> futures;
for (int i = 0; i < 100; ++i) {
    futures.push_back(pool.submit([i]() {
        return process_item(i);
    }));
}

// Wait for results
for (auto& f : futures) {
    int result = f.get();
    std::cout << "Result: " << result << std::endl;
}

// Wait for all jobs to complete
pool.wait_for_all();

// Shutdown pool
pool.stop();
```

### Class: `lock_free_queue`

Multi-producer, multi-consumer lock-free queue.

#### Constructor
```cpp
explicit lock_free_queue(size_t capacity = 1024);
```

#### Methods
```cpp
bool push(const T& item);
bool pop(T& item);
bool try_pop(T& item);
size_t size() const;
bool empty() const;
```

#### Usage Example
```cpp
#include <thread_system/lock_free_queue.h>
using namespace thread_system;

// Create queue
lock_free_queue<int> queue(10000);

// Producer thread
std::thread producer([&queue]() {
    for (int i = 0; i < 1000; ++i) {
        queue.push(i);
    }
});

// Consumer threads
std::vector<std::thread> consumers;
for (int i = 0; i < 4; ++i) {
    consumers.emplace_back([&queue]() {
        int item;
        while (queue.pop(item)) {
            process_item(item);
        }
    });
}

// Join threads
producer.join();
for (auto& t : consumers) {
    t.join();
}
```

---

## Logger System API

### Namespace: `logger`

### Class: `logger_manager`

High-performance asynchronous logging system.

#### Configuration
```cpp
struct logger_config {
    log_level min_level = log_level::info;
    bool enable_console = true;
    bool enable_file = true;
    std::string log_directory = "./logs";
    size_t max_file_size = 10 * 1024 * 1024;  // 10MB
    size_t max_files = 10;
    bool async_logging = true;
    size_t buffer_size = 8192;
};
```

#### Log Levels
```cpp
enum class log_level {
    trace = 0,
    debug = 1,
    info = 2,
    warning = 3,
    error = 4,
    critical = 5
};
```

#### Methods
```cpp
static logger_manager& instance();
void configure(const logger_config& config);

void trace(const std::string& message);
void debug(const std::string& message);
void info(const std::string& message);
void warning(const std::string& message);
void error(const std::string& message);
void critical(const std::string& message);

template<typename... Args>
void log(log_level level, const std::string& format, Args&&... args);
```

#### Usage Example
```cpp
#include <logger/logger_manager.h>
using namespace logger;

// Configure logger
logger_config config;
config.min_level = log_level::debug;
config.enable_file = true;
config.log_directory = "/var/log/myapp";
config.async_logging = true;

auto& logger = logger_manager::instance();
logger.configure(config);

// Simple logging
logger.info("Application started");
logger.debug("Debug mode enabled");

// Formatted logging
int user_id = 12345;
std::string action = "login";
logger.log(log_level::info, "User {} performed action: {}", user_id, action);

// Error logging with context
try {
    perform_operation();
} catch (const std::exception& e) {
    logger.error("Operation failed: " + std::string(e.what()));
    logger.debug("Stack trace: " + get_stack_trace());
}

// Performance logging
auto start = std::chrono::high_resolution_clock::now();
process_data();
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
logger.info("Processing completed in {} ms", duration.count());
```

---

## Error Codes

### System Error Codes

```cpp
enum class error_code {
    // Success
    success = 0,

    // General errors (1-99)
    unknown_error = 1,
    invalid_parameter = 2,
    null_pointer = 3,
    out_of_memory = 4,
    not_implemented = 5,

    // Container errors (100-199)
    serialization_failed = 100,
    deserialization_failed = 101,
    invalid_type = 102,
    key_not_found = 103,
    container_full = 104,

    // Network errors (200-299)
    connection_failed = 200,
    connection_timeout = 201,
    connection_closed = 202,
    bind_failed = 203,
    send_failed = 204,
    receive_failed = 205,
    protocol_error = 206,
    ssl_error = 207,

    // Database errors (300-399)
    database_connection_failed = 300,
    query_failed = 301,
    transaction_failed = 302,
    deadlock_detected = 303,
    constraint_violation = 304,
    duplicate_key = 305,

    // Thread/concurrency errors (400-499)
    thread_creation_failed = 400,
    lock_timeout = 401,
    deadlock = 402,
    race_condition = 403,

    // Service errors (500-599)
    service_not_found = 500,
    service_unavailable = 501,
    service_timeout = 502,
    circular_dependency = 503,
    initialization_failed = 504,

    // Message bus errors (600-699)
    topic_not_found = 600,
    handler_not_found = 601,
    message_too_large = 602,
    queue_full = 603,
    publish_failed = 604
};
```

### Error Handling

```cpp
class system_error : public std::exception {
public:
    system_error(error_code code, const std::string& message);
    error_code code() const;
    const char* what() const noexcept override;
};

// Usage
try {
    if (!connection.is_connected()) {
        throw system_error(error_code::connection_failed,
                          "Unable to establish connection to server");
    }
} catch (const system_error& e) {
    logger.error("Error {}: {}", static_cast<int>(e.code()), e.what());
    // Handle specific error code
    if (e.code() == error_code::connection_timeout) {
        retry_connection();
    }
}
```

### Error Recovery Strategies

```cpp
// Retry with exponential backoff
template<typename Func>
auto retry_with_backoff(Func&& func, size_t max_retries = 3) {
    size_t delay_ms = 100;
    for (size_t i = 0; i < max_retries; ++i) {
        try {
            return func();
        } catch (const system_error& e) {
            if (i == max_retries - 1) throw;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            delay_ms *= 2;  // Exponential backoff
        }
    }
}

// Circuit breaker pattern
class circuit_breaker {
    enum state { closed, open, half_open };
    state current_state = closed;
    size_t failure_count = 0;
    size_t failure_threshold = 5;
    std::chrono::steady_clock::time_point last_failure_time;

public:
    template<typename Func>
    auto call(Func&& func) {
        if (current_state == open) {
            if (should_attempt_reset()) {
                current_state = half_open;
            } else {
                throw system_error(error_code::service_unavailable,
                                 "Circuit breaker is open");
            }
        }

        try {
            auto result = func();
            on_success();
            return result;
        } catch (...) {
            on_failure();
            throw;
        }
    }

private:
    void on_success() {
        failure_count = 0;
        current_state = closed;
    }

    void on_failure() {
        failure_count++;
        last_failure_time = std::chrono::steady_clock::now();
        if (failure_count >= failure_threshold) {
            current_state = open;
        }
    }

    bool should_attempt_reset() {
        auto now = std::chrono::steady_clock::now();
        auto time_since_failure = now - last_failure_time;
        return time_since_failure > std::chrono::seconds(30);
    }
};
```

## Configuration Reference

### System Configuration File Format

```ini
; messaging_config.ini

[system]
worker_threads = 8
max_memory_mb = 4096
enable_monitoring = true
monitoring_port = 9090

[network]
listen_address = 0.0.0.0
listen_port = 8080
max_connections = 10000
connection_timeout_ms = 5000
keepalive_interval_s = 30
enable_ssl = true
ssl_cert_file = /path/to/cert.pem
ssl_key_file = /path/to/key.pem

[database]
type = postgresql
host = localhost
port = 5432
database = messaging_db
username = app_user
password = ${DB_PASSWORD}  ; Environment variable
pool_size = 20
connection_timeout_ms = 3000

[message_bus]
worker_threads = 4
queue_size = 50000
enable_persistence = true
persistence_path = /var/lib/messaging/queue
enable_metrics = true

[logging]
level = info
console_enabled = true
file_enabled = true
log_directory = /var/log/messaging
max_file_size_mb = 100
max_files = 10
async_logging = true

[security]
enable_authentication = true
auth_type = jwt
jwt_secret = ${JWT_SECRET}
token_expiry_minutes = 60
enable_rate_limiting = true
rate_limit_requests = 1000
rate_limit_window_seconds = 60
```

### Environment Variables

```bash
# Database configuration
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=messaging_db
export DB_USER=app_user
export DB_PASSWORD=secure_password

# Network configuration
export MESSAGING_PORT=8080
export MESSAGING_BIND_ADDRESS=0.0.0.0

# Security
export JWT_SECRET=your_jwt_secret_key
export SSL_CERT_PATH=/path/to/cert.pem
export SSL_KEY_PATH=/path/to/key.pem

# Performance tuning
export WORKER_THREADS=8
export MAX_CONNECTIONS=10000
export QUEUE_SIZE=50000
```