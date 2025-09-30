# Container Module

High-performance, type-safe data containers with SIMD optimization and thread-safe operations.

## Overview

The Container Module provides a comprehensive type-safe container system for the messaging framework. It supports multiple value types, efficient serialization, and thread-safe operations with optional SIMD optimizations.

## Features

### 🎯 Core Capabilities
- **Type Safety**: Strongly-typed value system with compile-time checks
- **Thread Safety**: Lock-free and mutex-based concurrent access patterns
- **SIMD Optimization**: ARM NEON and x86 AVX support for numeric operations
- **Memory Efficiency**: Variant storage with minimal allocations
- **Serialization**: Binary, JSON, and XML serialization formats

### 📦 Value Types

| Type | Code | Description | Size Range |
|------|------|-------------|------------|
| `null_value` | '0' | Null/empty value | 0 bytes |
| `bool_value` | '1' | Boolean true/false | 1 byte |
| `char_value` | '2' | Single character | 1 byte |
| `int8_value` | '3' | 8-bit signed integer | 1 byte |
| `uint8_value` | '4' | 8-bit unsigned integer | 1 byte |
| `int16_value` | '5' | 16-bit signed integer | 2 bytes |
| `uint16_value` | '6' | 16-bit unsigned integer | 2 bytes |
| `int32_value` | '7' | 32-bit signed integer | 4 bytes |
| `uint32_value` | '8' | 32-bit unsigned integer | 4 bytes |
| `int64_value` | '9' | 64-bit signed integer | 8 bytes |
| `uint64_value` | 'a' | 64-bit unsigned integer | 8 bytes |
| `float_value` | 'b' | 32-bit floating point | 4 bytes |
| `double_value` | 'c' | 64-bit floating point | 8 bytes |
| `bytes_value` | 'd' | Raw byte array | Variable |
| `container_value` | 'e' | Nested container | Variable |
| `string_value` | 'f' | UTF-8 string | Variable |

## Usage Examples

### Basic Container Operations

```cpp
#include <container/container.h>
using namespace container_module;

// Create a new container
auto container = std::make_shared<value_container>();

// Set header information
container->set_source("client_01", "session_123");
container->set_target("server", "main_handler");
container->set_message_type("user_data");

// Add values using the value_factory
auto values = std::vector<std::shared_ptr<value>>{
    value_factory::create("user_id", int64_value, "12345"),
    value_factory::create("username", string_value, "john_doe"),
    value_factory::create("balance", double_value, "1500.75"),
    value_factory::create("active", bool_value, "true")
};

container->set_values(values);
```

### Serialization and Deserialization

```cpp
// Binary serialization
std::string binary_data = container->serialize();

// JSON serialization  
std::string json_data = container->to_json();

// XML serialization
std::string xml_data = container->to_xml();

// Byte array serialization
std::vector<uint8_t> byte_data = container->serialize_array();

// Restore from serialized data
auto restored = std::make_shared<value_container>(binary_data);
auto from_json = std::make_shared<value_container>();
from_json->from_json(json_data);
```

### Working with Nested Containers

```cpp
// Create nested container
auto nested = std::make_shared<value_container>();
nested->set_message_type("address_info");

auto address_values = std::vector<std::shared_ptr<value>>{
    value_factory::create("street", string_value, "123 Main St"),
    value_factory::create("city", string_value, "Seattle"),
    value_factory::create("zip", string_value, "98101")
};
nested->set_values(address_values);

// Add nested container to parent
auto container_val = std::make_shared<container_value>("address", nested);
container->add_value(container_val);
```

### Thread-Safe Operations

```cpp
#include <container/internal/thread_safe_container.h>

// Create thread-safe wrapper
auto safe_container = std::make_shared<thread_safe_container>(container);

// Access from multiple threads
std::thread writer([&safe_container]() {
    safe_container->set_value("counter", int32_value, "100");
});

std::thread reader([&safe_container]() {
    auto value = safe_container->get_value("counter");
    if (value) {
        std::cout << "Counter: " << value->to_string() << std::endl;
    }
});

writer.join();
reader.join();
```

## Performance Optimizations

### SIMD Operations

The container module automatically uses SIMD instructions for supported operations:

```cpp
// Numeric operations benefit from SIMD acceleration
auto numeric_container = std::make_shared<value_container>();

// Add large arrays of numeric data
std::vector<double> large_array(1000);
std::iota(large_array.begin(), large_array.end(), 0.0);

auto bytes_val = std::make_shared<bytes_value>("data", 
    reinterpret_cast<const char*>(large_array.data()),
    large_array.size() * sizeof(double));

numeric_container->add_value(bytes_val);

// SIMD-optimized serialization/deserialization
std::string serialized = numeric_container->serialize(); // Uses SIMD
```

### Memory Management

```cpp
// Efficient value creation with minimal allocations
auto efficient_container = std::make_shared<value_container>();

// Reserve space for known number of values
efficient_container->reserve_values(10);

// Use move semantics for large strings
std::string large_string(1000000, 'x');
auto str_val = std::make_shared<string_value>("big_data", std::move(large_string));
efficient_container->add_value(std::move(str_val));
```

## API Reference

### value_container Class

#### Core Methods
```cpp
// Header management
void set_source(const std::string& id, const std::string& sub_id = "");
void set_target(const std::string& id, const std::string& sub_id = "");
void set_message_type(const std::string& type);
void swap_header(); // Swap source and target

// Value management
void add_value(std::shared_ptr<value> val);
void set_values(const std::vector<std::shared_ptr<value>>& values);
std::shared_ptr<value> get_value(const std::string& key) const;
void clear_values();

// Serialization
std::string serialize() const;
std::vector<uint8_t> serialize_array() const;
std::string to_json() const;
std::string to_xml() const;

// Deserialization constructors
value_container(const std::string& serialized_data);
value_container(const std::vector<uint8_t>& byte_data);

// Copy operations
std::shared_ptr<value_container> copy(bool deep_copy = true) const;
```

### value_factory Class

```cpp
// Create values of different types
static std::shared_ptr<value> create(const std::string& key, 
                                   value_types type, 
                                   const std::string& data);

// Type-specific creators
static std::shared_ptr<bool_value> create_bool(const std::string& key, bool val);
static std::shared_ptr<string_value> create_string(const std::string& key, const std::string& val);
static std::shared_ptr<int64_value> create_int64(const std::string& key, int64_t val);
static std::shared_ptr<double_value> create_double(const std::string& key, double val);
```

## Thread Safety

### Thread-Safe Guarantees

- **read operations**: Always thread-safe without external synchronization
- **write operations**: Thread-safe when using `thread_safe_container` wrapper
- **serialization**: Thread-safe for read-only containers
- **value access**: Concurrent reads are safe, writes require synchronization

### Best Practices

```cpp
// For read-heavy workloads
auto container = std::make_shared<value_container>();
// Multiple threads can safely read simultaneously

// For write-heavy workloads  
auto safe_container = std::make_shared<thread_safe_container>(container);
// Use wrapper for synchronized access

// For mixed workloads
std::shared_mutex container_mutex;
std::shared_lock<std::shared_mutex> read_lock(container_mutex); // For reads
std::unique_lock<std::shared_mutex> write_lock(container_mutex); // For writes
```

## Performance Characteristics

### Benchmarks (Intel i7-12700K, 16 threads)

| Operation | Rate | Notes |
|-----------|------|-------|
| Container Creation | 5M/sec | Empty containers |
| Value Addition | 15M/sec | String values |
| Binary Serialization | 2M/sec | 1KB containers |
| JSON Serialization | 800K/sec | 1KB containers |
| Deserialization | 1.5M/sec | Binary format |
| SIMD Operations | 25M/sec | Numeric arrays |

### Memory Usage

- **Empty Container**: ~128 bytes
- **String Value**: ~64 bytes + string length
- **Numeric Value**: ~48 bytes
- **Nested Container**: Recursive calculation
- **Serialized Overhead**: ~10% for binary, ~40% for JSON

## Error Handling

```cpp
#include <container/core/container.h>

try {
    auto container = std::make_shared<value_container>(invalid_data);
} catch (const std::invalid_argument& e) {
    std::cerr << "Invalid serialization data: " << e.what() << std::endl;
} catch (const std::runtime_error& e) {
    std::cerr << "Container error: " << e.what() << std::endl;
}

// Check for valid values
auto value = container->get_value("key");
if (!value) {
    std::cerr << "Value 'key' not found" << std::endl;
}

// Validate container state
if (container->source_id().empty()) {
    std::cerr << "Container missing source ID" << std::endl;
}
```

## Integration with Other Modules

### With Network Module
```cpp
#include <network/messaging_client.h>

// Send container over network
auto client = std::make_shared<messaging_client>("client_01");
std::string message = container->serialize();
client->send_raw_message(message);
```

### With Database Module
```cpp
#include <database/database_manager.h>

// Store container in database
database_manager db;
db.connect("host=localhost dbname=messages");

std::string data = container->serialize();
db.insert_query("INSERT INTO messages (data) VALUES ('" + data + "')");
```

## Building

The Container module is built as part of the main messaging system:

```bash
# Build with container module
./build.sh

# Build container tests only
cd build
make container_test
./bin/container_test
```

## Dependencies

- **C++20 Standard Library**: Required for concepts, ranges, and format
- **fmt Library**: High-performance string formatting
- **Thread System**: For lock-free operations and threading
- **Utilities Module**: String conversion and system utilities

## License

BSD 3-Clause License - see main project LICENSE file.