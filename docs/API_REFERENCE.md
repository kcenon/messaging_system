# API Reference

## Table of Contents

1. [Container System API](#container-system-api)
2. [Network System API](#network-system-api)
3. [Database System API](#database-system-api)
4. [Message Bus API](#message-bus-api)
5. [Message Broker API](#message-broker-api)
6. [Service Container API](#service-container-api)
7. [Thread System API](#thread-system-api)
8. [Logger System API](#logger-system-api)
9. [Error Codes](#error-codes)

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

### Namespace: `kcenon::messaging`

### Enum: `transport_mode`

Defines how message_bus handles message routing.

```cpp
enum class transport_mode {
    local,   // Local-only: messages are routed only to local subscribers
    remote,  // Remote-only: messages are sent only via transport
    hybrid   // Hybrid: messages are routed both locally and remotely
};
```

### Class: `message_bus`

Central message routing and distribution system with optional distributed messaging support.

#### Constructor
```cpp
explicit message_bus(
    std::shared_ptr<backend_interface> backend,
    message_bus_config config = {}
);
```

#### Configuration Structure
```cpp
struct message_bus_config {
    size_t queue_capacity = 10000;
    size_t worker_threads = 4;
    bool enable_priority_queue = true;
    bool enable_dead_letter_queue = true;
    bool enable_metrics = true;
    std::chrono::milliseconds processing_timeout{5000};

    // Transport configuration
    transport_mode mode = transport_mode::local;
    std::shared_ptr<adapters::transport_interface> transport = nullptr;
    std::string local_node_id;  // Unique identifier for distributed routing
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
    uint64_t messages_dropped;
    uint64_t messages_sent_remote;      // Messages sent via transport
    uint64_t messages_received_remote;  // Messages received from transport
};

statistics_snapshot get_statistics() const;
void reset_statistics();
```

#### Transport Accessors
```cpp
transport_mode get_transport_mode() const;
bool has_transport() const;
bool is_transport_connected() const;
```

#### Usage Example (Local Mode)
```cpp
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
using namespace kcenon::messaging;

// Create backend
auto backend = std::make_shared<standalone_backend>(4);

// Configure message bus (local-only mode, default)
message_bus_config config;
config.worker_threads = 4;
config.queue_capacity = 10000;

message_bus bus(backend, config);
bus.start();

// Subscribe to topics
bus.subscribe("user.created", [](const message& msg) {
    std::cout << "New user event received" << std::endl;
    return common::ok();
});

// Publish messages
message msg("user.created");
bus.publish(msg);

// Get statistics
auto stats = bus.get_statistics();
std::cout << "Messages processed: " << stats.messages_processed << std::endl;

// Shutdown
bus.stop();
```

#### Usage Example (Hybrid Mode with Transport)
```cpp
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/adapters/websocket_transport.h>
using namespace kcenon::messaging;

// Create backend and transport
auto backend = std::make_shared<standalone_backend>(4);
auto transport = std::make_shared<adapters::websocket_transport>(
    adapters::websocket_transport_config{"localhost", 8080}
);

// Configure message bus for hybrid mode
message_bus_config config;
config.worker_threads = 4;
config.mode = transport_mode::hybrid;  // Route to both local and remote
config.transport = transport;
config.local_node_id = "node-1";

message_bus bus(backend, config);
bus.start();  // Connects transport automatically

// Messages now route to local subscribers AND remote nodes
message msg("user.created");
bus.publish(msg);

auto stats = bus.get_statistics();
std::cout << "Sent remote: " << stats.messages_sent_remote << std::endl;
std::cout << "Received remote: " << stats.messages_received_remote << std::endl;

bus.stop();  // Disconnects transport automatically
```

---

## Message Broker API

### Namespace: `kcenon::messaging`

### Overview

The `message_broker` provides a central message routing component with advanced routing capabilities. It integrates with `topic_router` for pattern matching while providing a higher-level abstraction for route management.

### Struct: `broker_config`

Configuration for message_broker.

```cpp
struct broker_config {
    size_t max_routes = 1000;           // Maximum number of routes
    bool enable_statistics = true;       // Enable statistics collection
    bool enable_trace_logging = false;   // Enable trace-level logging
    std::chrono::milliseconds default_timeout{0};  // Default timeout (0 = no timeout)
};
```

### Struct: `broker_statistics`

Runtime statistics for message_broker.

```cpp
struct broker_statistics {
    uint64_t messages_routed = 0;     // Total messages routed
    uint64_t messages_delivered = 0;  // Successfully delivered
    uint64_t messages_failed = 0;     // Failed to route
    uint64_t messages_unrouted = 0;   // No matching route
    uint64_t active_routes = 0;       // Number of active routes
    std::chrono::steady_clock::time_point last_reset;  // Last statistics reset
};
```

### Struct: `route_info`

Information about a registered route.

```cpp
struct route_info {
    std::string route_id;        // Unique route identifier
    std::string topic_pattern;   // Topic pattern (supports wildcards)
    int priority = 5;            // Route priority (higher = first)
    bool active = true;          // Whether route is active
    uint64_t messages_processed = 0;  // Messages processed by this route
};
```

### Class: `message_broker`

Central message routing component with route management.

#### Constructor
```cpp
explicit message_broker(broker_config config = {});
```

#### Lifecycle Management
```cpp
common::VoidResult start();
common::VoidResult stop();
bool is_running() const;
```

#### Route Management
```cpp
// Add a new route with handler and optional priority (0-10)
common::VoidResult add_route(
    const std::string& route_id,
    const std::string& topic_pattern,
    message_handler handler,
    int priority = 5
);

// Remove a route
common::VoidResult remove_route(const std::string& route_id);

// Enable/disable routes
common::VoidResult enable_route(const std::string& route_id);
common::VoidResult disable_route(const std::string& route_id);

// Query routes
bool has_route(const std::string& route_id) const;
common::Result<route_info> get_route(const std::string& route_id) const;
std::vector<route_info> get_routes() const;
size_t route_count() const;
void clear_routes();
```

#### Message Routing
```cpp
// Route a message to matching handlers (priority order)
common::VoidResult route(const message& msg);
```

#### Statistics
```cpp
broker_statistics get_statistics() const;
void reset_statistics();
```

#### Usage Example
```cpp
#include <kcenon/messaging/core/message_broker.h>
using namespace kcenon::messaging;

// Create and start broker
message_broker broker;
broker.start();

// Add routes with different priorities
broker.add_route("user-handler", "user.*", [](const message& msg) {
    std::cout << "Processing user event: " << msg.metadata().topic << std::endl;
    return common::ok();
}, 5);

broker.add_route("audit-handler", "user.#", [](const message& msg) {
    std::cout << "Auditing: " << msg.metadata().topic << std::endl;
    return common::ok();
}, 10);  // Higher priority, processed first

// Route messages
message msg("user.created");
auto result = broker.route(msg);
if (!result.is_ok()) {
    std::cerr << "Routing failed: " << result.error().message << std::endl;
}

// Check statistics
auto stats = broker.get_statistics();
std::cout << "Messages routed: " << stats.messages_routed << std::endl;
std::cout << "Messages delivered: " << stats.messages_delivered << std::endl;

// Manage routes
broker.disable_route("audit-handler");
broker.remove_route("user-handler");

broker.stop();
```

#### Topic Pattern Wildcards

The broker supports MQTT-style wildcards via `topic_router`:

| Pattern | Matches | Does Not Match |
|---------|---------|----------------|
| `user.created` | `user.created` | `user.updated` |
| `user.*` | `user.created`, `user.updated` | `user.profile.updated` |
| `user.#` | `user.created`, `user.profile.updated` | `order.created` |
| `*.user.#` | `app.user.profile` | `user.profile` |

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