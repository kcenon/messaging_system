# Messaging System API Reference

Complete API documentation for all messaging system modules.

## Table of Contents

- [Container Module API](#container-module-api)
- [Database Module API](#database-module-api)
- [Network Module API](#network-module-api)
- [Common Types and Enums](#common-types-and-enums)
- [Error Handling](#error-handling)
- [Thread Safety](#thread-safety)

## Container Module API

### Classes

#### `value_container`

**Header**: `container/container.h`  
**Namespace**: `container_module`

Thread-safe, type-safe data container with SIMD optimization support.

##### Public Methods

###### Construction and Destruction

```cpp
// Default constructor
value_container();

// Construct with message type
value_container(const std::string& message_type, 
               const std::vector<std::shared_ptr<value>>& values);

// Construct with full header
value_container(const std::string& source_id,
               const std::string& source_sub_id,
               const std::string& target_id, 
               const std::string& target_sub_id,
               const std::string& message_type,
               const std::vector<std::shared_ptr<value>>& values);

// Deserialize from string
explicit value_container(const std::string& serialized_data);

// Deserialize from byte array
explicit value_container(const std::vector<uint8_t>& byte_data);

// Copy constructor
value_container(const value_container& other);

// Move constructor
value_container(value_container&& other) noexcept;

// Destructor
virtual ~value_container();
```

###### Header Management

```cpp
// Set source identification
void set_source(const std::string& source_id, 
               const std::string& source_sub_id = "");

// Set target identification  
void set_target(const std::string& target_id,
               const std::string& target_sub_id = "");

// Set message type
void set_message_type(const std::string& message_type);

// Swap source and target
void swap_header();

// Get header information
std::string source_id() const;
std::string source_sub_id() const;
std::string target_id() const;
std::string target_sub_id() const;
std::string message_type() const;
```

###### Value Management

```cpp
// Add a value to the container
void add_value(std::shared_ptr<value> val);

// Set all values (replaces existing values)
void set_values(const std::vector<std::shared_ptr<value>>& values);

// Get value by key
std::shared_ptr<value> get_value(const std::string& key) const;

// Get all values
const std::vector<std::shared_ptr<value>>& values() const;

// Clear all values
void clear_values();

// Get value count
size_t value_count() const;

// Check if container has specific key
bool has_value(const std::string& key) const;
```

###### Serialization

```cpp
// Binary serialization
std::string serialize() const;

// Byte array serialization
std::vector<uint8_t> serialize_array() const;

// JSON serialization
std::string to_json() const;

// XML serialization  
std::string to_xml() const;

// Deserialize from JSON
void from_json(const std::string& json_data);

// Deserialize from XML
void from_xml(const std::string& xml_data);
```

###### Container Operations

```cpp
// Create a copy of the container
std::shared_ptr<value_container> copy(bool deep_copy = true) const;

// Initialize/reset container
void initialize();

// Get container size in bytes
size_t size() const;

// Check if container is empty
bool empty() const;
```

#### `value`

**Header**: `container/core/value.h`  
**Namespace**: `container_module`

Base class for all value types in the container system.

##### Public Methods

```cpp
// Virtual destructor
virtual ~value() = default;

// Get value key
virtual std::string key() const = 0;

// Get value type
virtual value_types value_type() const = 0;

// Get string representation
virtual std::string to_string() const = 0;

// Get byte representation
virtual std::vector<uint8_t> to_bytes() const = 0;

// Get data size
virtual size_t size() const = 0;

// Check if value is null
virtual bool is_null() const = 0;

// Clone the value
virtual std::shared_ptr<value> clone() const = 0;
```

#### `value_factory`

**Header**: `container/core/value.h`  
**Namespace**: `container_module`

Factory class for creating typed values.

##### Static Methods

```cpp
// Generic value creation
static std::shared_ptr<value> create(const std::string& key,
                                   value_types type,
                                   const std::string& data);

// Type-specific creators
static std::shared_ptr<bool_value> create_bool(const std::string& key, bool val);
static std::shared_ptr<char_value> create_char(const std::string& key, char val);
static std::shared_ptr<int8_value> create_int8(const std::string& key, int8_t val);
static std::shared_ptr<uint8_value> create_uint8(const std::string& key, uint8_t val);
static std::shared_ptr<int16_value> create_int16(const std::string& key, int16_t val);
static std::shared_ptr<uint16_value> create_uint16(const std::string& key, uint16_t val);
static std::shared_ptr<int32_value> create_int32(const std::string& key, int32_t val);
static std::shared_ptr<uint32_value> create_uint32(const std::string& key, uint32_t val);
static std::shared_ptr<int64_value> create_int64(const std::string& key, int64_t val);
static std::shared_ptr<uint64_value> create_uint64(const std::string& key, uint64_t val);
static std::shared_ptr<float_value> create_float(const std::string& key, float val);
static std::shared_ptr<double_value> create_double(const std::string& key, double val);
static std::shared_ptr<string_value> create_string(const std::string& key, const std::string& val);
static std::shared_ptr<bytes_value> create_bytes(const std::string& key, const std::vector<uint8_t>& val);
static std::shared_ptr<container_value> create_container(const std::string& key, 
                                                        std::shared_ptr<value_container> val);
```

### Thread-Safe Container Classes

#### `thread_safe_container`

**Header**: `container/internal/thread_safe_container.h`  
**Namespace**: `container_module`

Thread-safe wrapper for value_container with mutex-based synchronization.

##### Public Methods

```cpp
// Constructor
explicit thread_safe_container(std::shared_ptr<value_container> container);

// Thread-safe value operations
void set_value(const std::string& key, value_types type, const std::string& data);
std::shared_ptr<value> get_value(const std::string& key) const;
void remove_value(const std::string& key);

// Thread-safe header operations
void set_source(const std::string& source_id, const std::string& source_sub_id = "");
void set_target(const std::string& target_id, const std::string& target_sub_id = "");
void set_message_type(const std::string& message_type);

// Thread-safe serialization
std::string serialize() const;
std::vector<uint8_t> serialize_array() const;

// Get underlying container (read-only)
std::shared_ptr<const value_container> get_container() const;
```

### Enums and Types

#### `value_types`

```cpp
enum class value_types : char {
    null_value = '0',     // Null/empty value
    bool_value = '1',     // Boolean value
    char_value = '2',     // Character value
    int8_value = '3',     // 8-bit signed integer
    uint8_value = '4',    // 8-bit unsigned integer
    int16_value = '5',    // 16-bit signed integer
    uint16_value = '6',   // 16-bit unsigned integer
    int32_value = '7',    // 32-bit signed integer
    uint32_value = '8',   // 32-bit unsigned integer
    int64_value = '9',    // 64-bit signed integer
    uint64_value = 'a',   // 64-bit unsigned integer
    float_value = 'b',    // 32-bit floating point
    double_value = 'c',   // 64-bit floating point
    bytes_value = 'd',    // Raw byte array
    container_value = 'e', // Nested container
    string_value = 'f'    // UTF-8 string
};
```

## Database Module API

### Classes

#### `database_manager`

**Header**: `database/database_manager.h`  
**Namespace**: `database`

Singleton database manager providing unified database operations.

##### Public Methods

###### Singleton Access

```cpp
// Get singleton instance
static database_manager& handle();
```

###### Configuration

```cpp
// Set database type
bool set_mode(const database_types& database_type);

// Get current database type
database_types database_type() const;
```

###### Connection Management

```cpp
// Connect to database
bool connect(const std::string& connect_string);

// Disconnect from database
bool disconnect();

// Check connection status
bool is_connected() const;
```

###### Query Operations

```cpp
// Execute DDL queries (CREATE, DROP, ALTER)
bool create_query(const std::string& query_string);

// Execute INSERT queries
unsigned int insert_query(const std::string& query_string);

// Execute UPDATE queries  
unsigned int update_query(const std::string& query_string);

// Execute DELETE queries
unsigned int delete_query(const std::string& query_string);

// Execute SELECT queries
std::unique_ptr<container_module::value_container> 
    select_query(const std::string& query_string);
```

#### `postgres_manager`

**Header**: `database/postgres_manager.h`  
**Namespace**: `database`

PostgreSQL-specific database implementation.

##### Public Methods

```cpp
// Database type identification
database_types database_type() override;

// Connection lifecycle
bool connect(const std::string& connect_string) override;
bool disconnect() override;

// Query execution (inherited from database_base)
bool create_query(const std::string& query_string) override;
unsigned int insert_query(const std::string& query_string) override;
unsigned int update_query(const std::string& query_string) override;
unsigned int delete_query(const std::string& query_string) override;
std::unique_ptr<container_module::value_container> 
    select_query(const std::string& query_string) override;
```

### Enums and Types

#### `database_types`

```cpp
enum class database_types {
    none,        // No database configured
    postgres     // PostgreSQL database
    // Future: mysql, sqlite, etc.
};
```

## Network Module API

### Classes

#### `messaging_server`

**Header**: `network/core/messaging_server.h`  
**Namespace**: `network_module`

High-performance TCP server for handling multiple client connections.

##### Public Methods

###### Construction

```cpp
// Constructor with server ID
explicit messaging_server(const std::string& server_id);

// Destructor
virtual ~messaging_server();
```

###### Server Lifecycle

```cpp
// Start server on specified port
bool start_server(unsigned short port);

// Stop the server
void stop_server();

// Wait for server to stop
void wait_for_stop();

// Check if server is running
bool is_running() const;
```

###### Configuration

```cpp
// Set maximum concurrent connections
void set_max_connections(size_t max_connections);

// Set message size limit
void set_message_size_limit(size_t max_size_bytes);

// Set connection timeout
void set_timeout_seconds(int timeout_seconds);

// Get server configuration
size_t max_connections() const;
size_t message_size_limit() const;
int timeout_seconds() const;
```

###### Event Handlers

```cpp
// Set message received handler
void set_message_handler(
    std::function<void(std::shared_ptr<messaging_session>, const std::string&)> handler);

// Set client connected handler
void set_connect_handler(
    std::function<void(std::shared_ptr<messaging_session>)> handler);

// Set client disconnected handler  
void set_disconnect_handler(
    std::function<void(std::shared_ptr<messaging_session>, const std::string&)> handler);
```

###### Server Information

```cpp
// Get server ID
std::string server_id() const;

// Get current connection count
size_t connection_count() const;

// Get server port
unsigned short port() const;
```

#### `messaging_client`

**Header**: `network/core/messaging_client.h`  
**Namespace**: `network_module`

TCP client for connecting to messaging servers.

##### Public Methods

###### Construction

```cpp
// Constructor with client ID
explicit messaging_client(const std::string& client_id);

// Destructor
virtual ~messaging_client();
```

###### Connection Lifecycle

```cpp
// Connect to server
bool start_client(const std::string& host, unsigned short port);

// Disconnect from server
void stop_client();

// Check connection status
bool is_connected() const;
```

###### Message Operations

```cpp
// Send text message
void send_message(const std::string& message);

// Send raw binary data
void send_raw_message(const std::vector<uint8_t>& data);

// Send container message
void send_container(std::shared_ptr<container_module::value_container> container);
```

###### Event Handlers

```cpp
// Set message received handler
void set_message_handler(std::function<void(const std::string&)> handler);

// Set connection established handler
void set_connect_handler(std::function<void()> handler);

// Set disconnection handler
void set_disconnect_handler(std::function<void(const std::string&)> handler);
```

###### Client Information

```cpp
// Get client ID
std::string client_id() const;

// Get connected server information
std::string server_host() const;
unsigned short server_port() const;
```

#### `messaging_session`

**Header**: `network/session/messaging_session.h`  
**Namespace**: `network_module`

Represents a client connection session on the server side.

##### Public Methods

###### Message Operations

```cpp
// Send message to client
void send_message(const std::string& message);

// Send raw binary data to client
void send_raw_data(const std::vector<uint8_t>& data);

// Send container message
void send_container(std::shared_ptr<container_module::value_container> container);
```

###### Session Information

```cpp
// Get unique session ID
std::string session_id() const;

// Get client ID (if provided during handshake)
std::string client_id() const;

// Get remote client address
std::string remote_address() const;

// Get remote client port
unsigned short remote_port() const;

// Get connection timestamp
std::chrono::steady_clock::time_point connect_time() const;

// Get last activity timestamp
std::chrono::steady_clock::time_point last_activity() const;
```

###### Session Management

```cpp
// Check if session is connected
bool is_connected() const;

// Forcibly disconnect session
void disconnect();

// Set session-specific data
void set_session_data(const std::string& key, const std::string& value);

// Get session-specific data
std::string get_session_data(const std::string& key) const;

// Remove session data
void remove_session_data(const std::string& key);
```

### Protocol Classes

#### `tcp_socket`

**Header**: `network/internal/tcp_socket.h`  
**Namespace**: `network_module::internal`

Low-level TCP socket wrapper (internal use).

#### `pipeline`

**Header**: `network/internal/pipeline.h`  
**Namespace**: `network_module::internal`

Message processing pipeline (internal use).

#### `send_coroutine`

**Header**: `network/internal/send_coroutine.h`  
**Namespace**: `network_module::internal`

Coroutine-based message sending (internal use).

## Common Types and Enums

### Message Protocol Types

#### `message_frame`

```cpp
struct message_frame {
    uint32_t magic;      // Magic number (0x4D534749 - "MSGI")
    uint32_t length;     // Total message length including header
    uint16_t type;       // Message type identifier
    std::vector<uint8_t> payload; // Message content
};
```

#### `message_type`

```cpp
enum class message_type : uint16_t {
    PING = 0x0001,           // Keep-alive ping
    PONG = 0x0002,           // Ping response
    DATA = 0x0010,           // Data message with container
    CONTROL = 0x0020,        // Control message
    ERROR = 0x0030,          // Error notification
    HEARTBEAT = 0x0040,      // Connection heartbeat
    DISCONNECT = 0x0050      // Graceful disconnect
};
```

### Utility Functions

#### String Processing

```cpp
namespace utility_module {
    // UTF-8 string conversion utilities
    std::optional<std::string> utf8_to_system(const std::string& utf8_str);
    std::optional<std::string> system_to_utf8(const std::string& system_str);
    
    // String manipulation
    std::vector<std::string> split_string(const std::string& str, char delimiter);
    std::string trim_string(const std::string& str);
    std::string to_lower(const std::string& str);
    std::string to_upper(const std::string& str);
}
```

## Error Handling

### Exception Types

#### `container_exception`

```cpp
class container_exception : public std::runtime_error {
public:
    explicit container_exception(const std::string& message);
};
```

#### `database_exception`

```cpp
class database_exception : public std::runtime_error {
public:
    explicit database_exception(const std::string& message);
    const std::string& sql_state() const;
    
private:
    std::string sql_state_;
};
```

#### `network_exception`

```cpp
class network_exception : public std::runtime_error {
public:
    explicit network_exception(const std::string& message);
    int error_code() const;
    
private:
    int error_code_;
};
```

### Error Result Types

#### `result<T>`

```cpp
template<typename T>
class result {
public:
    // Success constructor
    result(T&& value);
    
    // Error constructor
    result(const std::string& error_message);
    
    // Check if result contains value
    bool has_value() const;
    bool has_error() const;
    
    // Get value (throws if error)
    const T& value() const;
    T& value();
    
    // Get error message
    const std::string& error() const;
    
    // Value access operator
    T& operator*();
    const T& operator*() const;
    
    // Pointer access operator
    T* operator->();
    const T* operator->() const;
    
    // Boolean conversion
    explicit operator bool() const;
};
```

## Thread Safety

### Thread Safety Guarantees

#### Container Module
- **value_container**: Read operations are thread-safe, writes require external synchronization
- **thread_safe_container**: All operations are thread-safe with internal mutex
- **value objects**: Immutable after creation, thread-safe for reads

#### Database Module
- **database_manager**: Thread-safe singleton with internal synchronization
- **Connection pooling**: Thread-safe connection acquisition and release
- **Query operations**: Thread-safe when using separate connections

#### Network Module
- **messaging_server**: Thread-safe for all operations
- **messaging_client**: Thread-safe for all operations  
- **messaging_session**: Thread-safe for all operations
- **Asynchronous operations**: All async operations are thread-safe

### Thread Safety Levels

| Level | Description | Modules |
|-------|-------------|---------|
| **Thread-Safe** | All operations safe from multiple threads | Network, Database |
| **Read-Safe** | Concurrent reads safe, writes need synchronization | Container (basic) |
| **Synchronized** | Thread-safe with explicit synchronization wrapper | Container (thread_safe_container) |
| **Immutable** | Objects never change after creation | Value objects |

### Best Practices

```cpp
// For read-heavy container workloads
auto container = std::make_shared<value_container>();
// Multiple threads can safely read concurrently

// For write-heavy container workloads
auto safe_container = std::make_shared<thread_safe_container>(container);
// All operations are thread-safe

// For mixed workloads with custom synchronization
std::shared_mutex container_mutex;
std::shared_lock<std::shared_mutex> read_lock(container_mutex);  // For reads
std::unique_lock<std::shared_mutex> write_lock(container_mutex); // For writes

// Network operations are always thread-safe
auto server = std::make_shared<messaging_server>("server_id");
auto client = std::make_shared<messaging_client>("client_id");
// No additional synchronization needed

// Database operations are thread-safe at manager level
database_manager& db = database_manager::handle();
// Can be safely used from multiple threads
```

## Performance Considerations

### Optimization Guidelines

#### Container Module
- Use move semantics for large strings and byte arrays
- Reserve space in containers when size is known
- Prefer binary serialization over JSON/XML for performance
- Use SIMD-optimized operations for numeric data

#### Database Module
- Use connection pooling for high-throughput applications
- Batch operations when possible
- Use prepared statements for repeated queries
- Implement proper connection timeout handling

#### Network Module
- Use connection pooling for clients
- Batch small messages when possible
- Configure appropriate buffer sizes
- Use TCP_NODELAY for low-latency applications

### Memory Management

- All classes use RAII for automatic resource management
- Smart pointers (shared_ptr, unique_ptr) are used throughout
- Memory pools are used internally for high-frequency allocations
- Hazard pointers provide lock-free memory reclamation in thread system

## Version Compatibility

### API Versioning

- **Major version**: Breaking API changes
- **Minor version**: New features, backward compatible
- **Patch version**: Bug fixes, fully compatible

### Compatibility Matrix

| Version | Container API | Database API | Network API | Thread System |
|---------|---------------|--------------|-------------|---------------|
| 2.0.x   | Stable | Stable | Stable | Lock-free |
| 1.x.x   | Legacy | Legacy | Legacy | Mutex-based |

### Migration Guide

See [MIGRATION.md](MIGRATION.md) for detailed migration instructions between versions.

## Examples and Samples

Complete usage examples are available in the following locations:

- **Container Examples**: [container/examples/](../container/examples/)
- **Database Examples**: [database/examples/](../database/examples/)  
- **Network Examples**: [network/examples/](../network/examples/)
- **Integration Examples**: [examples/](../examples/)
- **Python Examples**: [python/messaging_system/samples/](../python/messaging_system/samples/)

## License

BSD 3-Clause License - see main project LICENSE file.