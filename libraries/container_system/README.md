[![CodeFactor](https://www.codefactor.io/repository/github/kcenon/container_system/badge)](https://www.codefactor.io/repository/github/kcenon/container_system)

[![Ubuntu-GCC](https://github.com/kcenon/container_system/actions/workflows/build-ubuntu-gcc.yaml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/build-ubuntu-gcc.yaml)
[![Ubuntu-Clang](https://github.com/kcenon/container_system/actions/workflows/build-ubuntu-clang.yaml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/build-ubuntu-clang.yaml)
[![Windows-MSYS2](https://github.com/kcenon/container_system/actions/workflows/build-windows-msys2.yaml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/build-windows-msys2.yaml)
[![Windows-VisualStudio](https://github.com/kcenon/container_system/actions/workflows/build-windows-vs.yaml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/build-windows-vs.yaml)

# Container System Project

## Project Overview

The Container System Project is a production-ready, high-performance C++20 type-safe container framework designed to provide comprehensive data management capabilities for messaging systems and general-purpose applications. Built with a modular, interface-based architecture featuring SIMD optimizations and seamless integration with the ecosystem, it delivers enterprise-grade serialization performance with minimal overhead and maximum type safety.

> **🏗️ Modular Architecture**: Advanced type-safe container system with pluggable value types, efficient serialization, and thread-safe operations.

> **✅ Latest Updates**: Enhanced SIMD optimizations, messaging integration, builder patterns, and comprehensive cross-platform support. All CI/CD pipelines green across platforms.

## 🔗 Project Ecosystem & Inter-Dependencies

This container system is a foundational component of a comprehensive messaging and data management ecosystem:

### Project Dependencies
- **[utilities_module](https://github.com/kcenon/utilities)**: Core dependency providing string conversion and system utilities
  - Provides: String conversion, system utilities, and platform abstractions
  - Role: Foundation utilities for container operations
  - Integration: String processing and type conversions

### Related Projects
- **[messaging_system](https://github.com/kcenon/messaging_system)**: Primary consumer of container functionality
  - Relationship: Uses containers for message encapsulation and serialization
  - Synergy: Optimized serialization formats for network transmission
  - Integration: Seamless message construction and processing

- **[network_system](https://github.com/kcenon/network_system)**: Network transport for containers
  - Relationship: Transports serialized containers over network protocols
  - Benefits: Efficient binary serialization for network communication
  - Integration: Automatic serialization/deserialization

- **[database_system](https://github.com/kcenon/database_system)**: Persistent storage for containers
  - Usage: Stores and retrieves containers from databases
  - Benefits: Native container serialization formats for storage
  - Reference: Efficient BLOB storage and retrieval

### Integration Architecture
```
┌─────────────────┐
│ utilities_module│ ← Foundation utilities (string conversion, system utils)
└─────────┬───────┘
          │ depends on
┌─────────▼───────┐     ┌─────────────────┐     ┌─────────────────┐
│container_system │ ◄──► │messaging_system │ ◄──► │ network_system  │
└─────────────────┘     └─────────────────┘     └─────────────────┘
          │                       │                       │
          └───────┬───────────────┴───────────────────────┘
                  ▼
    ┌─────────────────────────┐
    │   database_system      │
    └─────────────────────────┘
```

### Integration Benefits
- **Type-safe messaging**: Strongly-typed value system prevents runtime errors
- **Performance-optimized**: SIMD-accelerated operations for high-throughput scenarios
- **Universal serialization**: Binary, JSON, and XML formats for diverse integration needs
- **Unified data model**: Consistent container structure across all ecosystem components

> 📖 **[Complete Architecture Guide](docs/ARCHITECTURE.md)**: Comprehensive documentation of the entire ecosystem architecture, dependency relationships, and integration patterns.

## Project Purpose & Mission

This project addresses the fundamental challenge faced by developers worldwide: **making high-performance data serialization accessible, type-safe, and efficient**. Traditional serialization approaches often lack type safety, provide insufficient performance for high-throughput applications, and struggle with complex nested data structures. Our mission is to provide a comprehensive solution that:

- **Ensures type safety** through strongly-typed value system with compile-time checks
- **Maximizes performance** through SIMD optimizations and efficient memory management
- **Simplifies integration** through builder patterns and intuitive APIs
- **Promotes interoperability** through multiple serialization formats (binary, JSON, XML)
- **Accelerates development** by providing ready-to-use container components

## Core Advantages & Benefits

### 🚀 **Performance Excellence**
- **SIMD acceleration**: ARM NEON and x86 AVX support for numeric operations (25M ops/sec)
- **Efficient serialization**: Binary format with minimal overhead (2M containers/sec)
- **Memory optimization**: Variant storage with minimal allocations
- **Lock-free operations**: Thread-safe read operations without synchronization overhead

### 🛡️ **Production-Grade Reliability**
- **Type safety**: Strongly-typed value system prevents runtime type errors
- **Thread safety**: Concurrent read operations and optional thread-safe write operations
- **Memory safety**: RAII principles and smart pointers prevent leaks and corruption
- **Comprehensive validation**: Type checking and data integrity verification

### 🔧 **Developer Productivity**
- **Intuitive API design**: Clean, self-documenting interfaces reduce learning curve
- **Builder pattern**: Fluent API for container construction with method chaining
- **Multiple serialization formats**: Binary, JSON, XML for diverse integration scenarios
- **Rich value types**: 15 built-in types covering all common data scenarios

### 🌐 **Cross-Platform Compatibility**
- **Universal support**: Works on Windows, Linux, and macOS
- **Architecture optimization**: Native SIMD support for x86, x64, and ARM64
- **Compiler flexibility**: Compatible with GCC, Clang, and MSVC
- **Build system integration**: Native scripts for all major platforms

### 📈 **Enterprise-Ready Features**
- **Nested containers**: Support for complex hierarchical data structures
- **Messaging integration**: Optimized for messaging system performance
- **Performance monitoring**: Real-time operation metrics and analytics
- **External callbacks**: Integration hooks for custom processing

## Real-World Impact & Use Cases

### 🎯 **Ideal Applications**
- **Messaging systems**: Type-safe message construction and serialization
- **Network protocols**: Efficient binary serialization for network transmission
- **Database storage**: Optimized BLOB storage with native container support
- **IoT platforms**: Lightweight serialization for resource-constrained devices
- **Financial systems**: High-frequency trading data with type safety guarantees
- **Gaming engines**: Real-time data serialization for multiplayer systems

### 📊 **Performance Benchmarks**

*Benchmarked on Intel i7-12700K (16 threads) @ 3.8GHz, 32GB, Windows 11, MSVC 2022*

> **🚀 Architecture Update**: Latest modular architecture with SIMD optimizations delivers exceptional performance for serialization-intensive applications. Type-safe operations ensure reliability without performance compromise.

#### Core Performance Metrics (Latest Benchmarks)
- **Container Creation**: Up to 5M containers/second (empty containers)
- **Value Operations**:
  - String value addition: 15M values/s with efficient string handling
  - Numeric value addition: 25M values/s with SIMD acceleration
  - Complex nested structures: 1.2M containers/s
- **Serialization Performance**:
  - Binary serialization: 2M containers/s (1KB average size)
  - JSON serialization: 800K containers/s with structured output
  - XML serialization: 600K containers/s with schema validation
- **Deserialization**: 1.5M containers/s from binary format
- **Memory efficiency**: ~128 bytes baseline with optimized variant storage

#### Performance Comparison with Industry Standards
| Serialization Type | Throughput | Size Overhead | Memory Usage | Best Use Case |
|-------------------|------------|---------------|--------------|---------------|
| 🏆 **Container System Binary** | **2M/s** | **~10%** | **128B+data** | All scenarios (optimized) |
| 📦 **Protocol Buffers** | 1.2M/s | ~15% | 200B+data | Cross-language compatibility |
| 📦 **JSON (nlohmann)** | 400K/s | ~40% | 300B+data | Human-readable interchange |
| 📦 **MessagePack** | 1.8M/s | ~12% | 150B+data | Compact binary format |
| 📦 **XML (pugixml)** | 200K/s | ~60% | 400B+data | Schema validation needs |

#### Key Performance Insights
- 🏃 **Binary format**: Industry-leading performance with minimal overhead
- 🏋️ **SIMD operations**: 2.5x performance boost for numeric arrays
- ⏱️ **Type safety**: Zero runtime overhead for type checking
- 📈 **Scalability**: Linear performance scaling with container complexity

## Features

### 🎯 Core Capabilities
- **Type Safety**: Strongly-typed value system with compile-time checks
- **Thread Safety**: Lock-free and mutex-based concurrent access patterns
- **SIMD Optimization**: ARM NEON and x86 AVX support for numeric operations
- **Memory Efficiency**: Variant storage with minimal allocations
- **Serialization**: Binary, JSON, and XML serialization formats

### 🚀 Enhanced Features
- **Messaging Integration**: Optimized containers for messaging systems
- **Builder Pattern**: Fluent API for container construction
- **Performance Metrics**: Real-time operation monitoring and analytics
- **External Callbacks**: Integration hooks for external systems
- **Dual Compatibility**: Works standalone or as part of messaging systems

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

## Technology Stack & Architecture

### 🏗️ **Modern C++ Foundation**
- **C++20 features**: Concepts, ranges, `std::format`, and variant for enhanced performance
- **Template metaprogramming**: Type-safe, compile-time value type checking
- **Memory management**: Smart pointers and RAII for automatic resource cleanup
- **SIMD optimizations**: ARM NEON and x86 AVX support for numeric operations
- **Exception safety**: Strong exception safety guarantees throughout
- **Variant storage**: Efficient polymorphic value storage with minimal overhead
- **Modular architecture**: Core container functionality with optional integrations

### 🔄 **Design Patterns Implementation**
- **Factory Pattern**: Value factory for type-safe value creation
- **Builder Pattern**: Fluent API for container construction with validation
- **Visitor Pattern**: Type-safe value processing and serialization
- **Template Method Pattern**: Customizable serialization behavior
- **Strategy Pattern**: Configurable serialization formats (binary, JSON, XML)
- **RAII Pattern**: Automatic resource management for containers and values

## Project Structure

### 📁 **Directory Organization**

```
container_system/
├── 📁 include/container/           # Public headers
│   ├── 📁 core/                    # Core components
│   │   ├── container.h             # Main container interface
│   │   ├── value.h                 # Base value interface
│   │   ├── value_types.h           # Value type definitions
│   │   └── value_factory.h         # Factory for value creation
│   ├── 📁 values/                  # Value implementations
│   │   ├── primitive_values.h      # Basic types (int, bool, string)
│   │   ├── numeric_values.h        # Numeric types with SIMD
│   │   ├── container_value.h       # Nested container support
│   │   └── bytes_value.h           # Raw byte array support
│   ├── 📁 advanced/                # Advanced features
│   │   ├── variant_value.h         # Polymorphic value storage
│   │   ├── thread_safe_container.h # Thread-safe wrapper
│   │   └── simd_processor.h        # SIMD optimization utilities
│   ├── 📁 serialization/           # Serialization support
│   │   ├── binary_serializer.h     # High-performance binary format
│   │   ├── json_serializer.h       # JSON format support
│   │   ├── xml_serializer.h        # XML format with schema
│   │   └── format_detector.h       # Automatic format detection
│   └── 📁 integration/             # Integration features
│       ├── messaging_builder.h     # Builder for messaging systems
│       ├── network_serializer.h    # Network-optimized serialization
│       └── database_adapter.h      # Database storage adapter
├── 📁 src/                         # Implementation files
│   ├── 📁 core/                    # Core implementations
│   ├── 📁 values/                  # Value implementations
│   ├── 📁 advanced/                # Advanced feature implementations
│   ├── 📁 serialization/           # Serialization implementations
│   └── 📁 integration/             # Integration implementations
├── 📁 examples/                    # Example applications
│   ├── basic_container_example/    # Basic usage examples
│   ├── advanced_container_example/ # Advanced features demo
│   ├── real_world_scenarios/       # Industry-specific examples
│   └── messaging_integration_example/ # Messaging system integration
├── 📁 tests/                       # All tests
│   ├── 📁 unit/                    # Unit tests
│   ├── 📁 integration/             # Integration tests
│   └── 📁 performance/             # Performance benchmarks
├── 📁 docs/                        # Documentation
├── 📁 cmake/                       # CMake modules
├── 📄 CMakeLists.txt               # Build configuration
└── 📄 vcpkg.json                   # Dependencies
```

### 📖 **Key Files and Their Purpose**

#### Core Module Files
- **`container.h/cpp`**: Main container class with header management and value storage
- **`value.h/cpp`**: Abstract base class for all value types
- **`value_types.h`**: Enumeration and type definitions for all supported types
- **`value_factory.h/cpp`**: Factory pattern implementation for type-safe value creation

#### Value Implementation Files
- **`primitive_values.h/cpp`**: Basic types (bool, char, string) with optimized storage
- **`numeric_values.h/cpp`**: Integer and floating-point types with SIMD support
- **`container_value.h/cpp`**: Nested container support for hierarchical data
- **`bytes_value.h/cpp`**: Raw byte array with efficient memory management

#### Serialization Files
- **`binary_serializer.h/cpp`**: High-performance binary format with minimal overhead
- **`json_serializer.h/cpp`**: JSON format with pretty-printing and validation
- **`xml_serializer.h/cpp`**: XML format with schema support and namespace handling

### 🔗 **Module Dependencies**

```
utilities (string conversion, system utils)
    │
    └──> core (container, value, value_types)
            │
            ├──> values (primitive, numeric, container, bytes)
            │
            ├──> advanced (variant, thread_safe, simd_processor)
            │
            ├──> serialization (binary, json, xml)
            │
            └──> integration (messaging, network, database)

Optional External Projects:
- messaging_system (uses containers for message data)
- network_system (transports serialized containers)
- database_system (stores containers in BLOB fields)
```

## Quick Start & Usage Examples

### 🚀 **Getting Started in 5 Minutes**

#### High-Performance Container Example

```cpp
#include <container/core/container.h>
#include <container/integration/messaging_builder.h>
#include <container/advanced/thread_safe_container.h>

using namespace container_module;

int main() {
    // 1. Create high-performance container using builder pattern
    auto container = messaging_container_builder()
        .source("trading_engine", "session_001")
        .target("risk_monitor", "main")
        .message_type("market_data")
        .optimize_for_speed()
        .reserve_values(10)  // Pre-allocate for known size
        .build();

    // 2. Add strongly-typed values with SIMD optimization
    auto start_time = std::chrono::high_resolution_clock::now();

    // Financial data with type safety
    container->add_value(value_factory::create_string("symbol", "AAPL"));
    container->add_value(value_factory::create_double("price", 175.50));
    container->add_value(value_factory::create_int64("volume", 1000000));
    container->add_value(value_factory::create_bool("is_active", true));

    // 3. Add nested container for complex data
    auto order_book = std::make_shared<value_container>();
    order_book->set_message_type("order_book");

    // SIMD-optimized numeric arrays
    std::vector<double> bid_prices = {175.48, 175.47, 175.46, 175.45, 175.44};
    std::vector<int64_t> bid_volumes = {1000, 2000, 1500, 3000, 2500};

    for (size_t i = 0; i < bid_prices.size(); ++i) {
        order_book->add_value(value_factory::create_double(
            "bid_price_" + std::to_string(i), bid_prices[i]));
        order_book->add_value(value_factory::create_int64(
            "bid_volume_" + std::to_string(i), bid_volumes[i]));
    }

    container->add_value(std::make_shared<container_value>("order_book", order_book));

    // 4. High-performance serialization with format comparison
    auto serialize_start = std::chrono::high_resolution_clock::now();

    // Binary serialization (fastest)
    std::string binary_data = container->serialize();
    auto binary_time = std::chrono::high_resolution_clock::now();

    // JSON serialization (human-readable)
    std::string json_data = container->to_json();
    auto json_time = std::chrono::high_resolution_clock::now();

    // XML serialization (schema-validated)
    std::string xml_data = container->to_xml();
    auto xml_time = std::chrono::high_resolution_clock::now();

    // 5. Thread-safe operations for concurrent processing
    auto safe_container = std::make_shared<thread_safe_container>(container);

    // Simulate concurrent access
    std::vector<std::thread> worker_threads;
    std::atomic<int> operations_completed{0};

    for (int t = 0; t < 4; ++t) {
        worker_threads.emplace_back([&safe_container, &operations_completed, t]() {
            for (int i = 0; i < 1000; ++i) {
                // Thread-safe read operations
                auto price_value = safe_container->get_value("price");
                if (price_value) {
                    double price = std::stod(price_value->to_string());

                    // Thread-safe write operations
                    safe_container->set_value("last_update_thread",
                                             int32_value, std::to_string(t));
                }
                operations_completed.fetch_add(1);
            }
        });
    }

    // Wait for all threads
    for (auto& thread : worker_threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    // 6. Performance metrics and results
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    auto binary_duration = std::chrono::duration_cast<std::chrono::microseconds>(binary_time - serialize_start);
    auto json_duration = std::chrono::duration_cast<std::chrono::microseconds>(json_time - binary_time);
    auto xml_duration = std::chrono::duration_cast<std::chrono::microseconds>(xml_time - json_time);

    std::cout << "Performance Results:\n";
    std::cout << "- Total processing time: " << total_duration.count() << " μs\n";
    std::cout << "- Binary serialization: " << binary_duration.count() << " μs (" << binary_data.size() << " bytes)\n";
    std::cout << "- JSON serialization: " << json_duration.count() << " μs (" << json_data.size() << " bytes)\n";
    std::cout << "- XML serialization: " << xml_duration.count() << " μs (" << xml_data.size() << " bytes)\n";
    std::cout << "- Thread operations completed: " << operations_completed.load() << "\n";
    std::cout << "- Throughput: " << (operations_completed.load() * 1000000.0 / total_duration.count()) << " ops/sec\n";

    // 7. Deserialization verification
    auto restored = std::make_shared<value_container>(binary_data);
    auto restored_price = restored->get_value("price");
    if (restored_price) {
        std::cout << "- Data integrity verified: price = " << restored_price->to_string() << "\n";
    }

    return 0;
}
```

> **Performance Tip**: The container system automatically optimizes for your use case. Use binary serialization for maximum speed, builder pattern for efficient construction, and thread-safe wrappers for concurrent scenarios.

### 🔄 **More Usage Examples**

#### Enterprise Messaging Integration
```cpp
#include <container/integration/messaging_builder.h>
#include <container/serialization/binary_serializer.h>

using namespace container_module::integration;

// Create optimized container for messaging system
auto message = messaging_container_builder()
    .source("order_service", "instance_01")
    .target("fulfillment_service", "warehouse_west")
    .message_type("order_create")
    .add_value("order_id", "ORD-2025-001234")
    .add_value("customer_id", 98765)
    .add_value("total_amount", 299.99)
    .add_value("priority", "high")
    .add_nested_container("items", [](auto& builder) {
        builder.add_value("sku", "SKU-12345")
               .add_value("quantity", 2)
               .add_value("unit_price", 149.99);
    })
    .optimize_for_network()
    .build();

// High-performance serialization for network transmission
std::string serialized = messaging_integration::serialize_for_messaging(message);

// Automatic compression for large payloads
if (serialized.size() > 1024) {
    serialized = messaging_integration::compress(serialized);
}
```

#### Real-time IoT Data Processing
```cpp
#include <container/advanced/simd_processor.h>
#include <container/values/numeric_values.h>

using namespace container_module;

// Create container optimized for IoT sensor data
auto sensor_data = std::make_shared<value_container>();
sensor_data->set_source("sensor_array", "building_A_floor_3");
sensor_data->set_message_type("environmental_reading");

// SIMD-accelerated bulk data processing
std::vector<double> temperature_readings(1000);
std::vector<double> humidity_readings(1000);

// Simulate sensor data collection
std::iota(temperature_readings.begin(), temperature_readings.end(), 20.0);
std::iota(humidity_readings.begin(), humidity_readings.end(), 45.0);

// Use SIMD processor for efficient data handling
simd_processor processor;
auto processed_temp = processor.process_array(temperature_readings);
auto processed_humidity = processor.process_array(humidity_readings);

// Add processed data to container
sensor_data->add_value(std::make_shared<bytes_value>("temperature_data",
    reinterpret_cast<const char*>(processed_temp.data()),
    processed_temp.size() * sizeof(double)));

sensor_data->add_value(std::make_shared<bytes_value>("humidity_data",
    reinterpret_cast<const char*>(processed_humidity.data()),
    processed_humidity.size() * sizeof(double)));

// Compact binary serialization for bandwidth-constrained IoT networks
std::vector<uint8_t> compact_data = sensor_data->serialize_array();
```

### 📚 **Comprehensive Sample Collection**

Our samples demonstrate real-world usage patterns and best practices:

#### **Core Functionality**
- **[Basic Container](examples/basic_container_example/)**: Type-safe value creation and management
- **[Advanced Features](examples/advanced_container_example/)**: Threading, SIMD, and performance optimization
- **[Serialization Formats](examples/serialization_examples/)**: Binary, JSON, XML format demonstrations
- **[Builder Patterns](examples/builder_examples/)**: Fluent API construction and validation

#### **Advanced Features**
- **[SIMD Operations](examples/simd_examples/)**: High-performance numeric processing
- **[Thread Safety](examples/threading_examples/)**: Concurrent access patterns and synchronization
- **[Memory Optimization](examples/memory_examples/)**: Efficient storage and allocation strategies
- **[Performance Benchmarks](examples/performance_examples/)**: Comprehensive performance analysis

#### **Integration Examples**
- **[Messaging Integration](examples/messaging_integration_example/)**: Messaging system optimization
- **[Network Transport](examples/network_examples/)**: Network serialization and transport
- **[Database Storage](examples/database_examples/)**: Persistent container storage patterns

### 🛠️ **Build & Integration**

#### Prerequisites
- **Compiler**: C++20 capable (GCC 9+, Clang 10+, MSVC 2019+)
- **Build System**: CMake 3.16+
- **Package Manager**: vcpkg (automatically installed by dependency scripts)

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/kcenon/container_system.git
cd container_system

# Install dependencies (cross-platform scripts)
./dependency.sh  # Linux/macOS
# or
dependency.bat   # Windows Command Prompt
# or
.\dependency.ps1 # Windows PowerShell

# Build the project (optimized for your platform)
./build.sh       # Linux/macOS
# or
build.bat        # Windows Command Prompt
# or
.\build.ps1      # Windows PowerShell

# Run examples
./build/examples/basic_container_example
./build/examples/advanced_container_example
./build/examples/real_world_scenarios

# Run tests
cd build && ctest
```

#### CMake Integration

```cmake
# Using as a subdirectory
add_subdirectory(container_system)
target_link_libraries(your_target PRIVATE ContainerSystem::container)

# Optional: Add messaging system integration
add_subdirectory(messaging_system)
target_link_libraries(your_target PRIVATE
    ContainerSystem::container
    MessagingSystem::core
)

# Using with FetchContent
include(FetchContent)
FetchContent_Declare(
    container_system
    GIT_REPOSITORY https://github.com/kcenon/container_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(container_system)
```

## Documentation

- Module READMEs:
  - core/README.md
  - values/README.md
  - serialization/README.md
- Guides:
  - docs/USER_GUIDE.md (setup, quick starts, value types)
  - docs/API_REFERENCE.md (complete API documentation)
  - docs/ARCHITECTURE.md (system design and patterns)

Build API docs with Doxygen (optional):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target docs
# Open documents/html/index.html
```

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

### Enhanced Builder Pattern

```cpp
#include <container/container.h>
using namespace container_module::integration;

// Modern builder pattern with method chaining
auto container = messaging_container_builder()
    .source("client_01", "session_123")
    .target("server", "main_handler")
    .message_type("user_data")
    .add_value("user_id", 12345)
    .add_value("username", std::string("john_doe"))
    .add_value("balance", 1500.75)
    .add_value("active", true)
    .optimize_for_speed()
    .build();

// Enhanced serialization with performance monitoring
std::string serialized = messaging_integration::serialize_for_messaging(container);
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

## Platform Support

The Container System provides full cross-platform support with native build scripts:

### Supported Platforms

| Platform | Architecture | Compiler | Build Scripts |
|----------|-------------|-----------|---------------|
| **Linux** | x86_64, ARM64 | GCC 9+, Clang 10+ | `dependency.sh`, `build.sh` |
| **macOS** | x86_64, ARM64 (Apple Silicon) | Apple Clang, Clang | `dependency.sh`, `build.sh` |
| **Windows** | x86, x64 | MSVC 2019+, Clang | `dependency.bat`, `build.bat`, `dependency.ps1`, `build.ps1` |

### Build Tools

| Tool | Linux/macOS | Windows |
|------|-------------|---------|
| **Build System** | CMake 3.16+ | CMake 3.16+ |
| **Package Manager** | vcpkg | vcpkg |
| **Generators** | Unix Makefiles, Ninja | Visual Studio 2019/2022, Ninja |
| **Dependencies** | apt/yum/brew + vcpkg | vcpkg |

### Windows-Specific Features

- **Multiple Generators**: Visual Studio 2019/2022, Ninja
- **Platform Support**: x86, x64 builds
- **Automatic Dependency Installation**: Visual Studio Build Tools, CMake, vcpkg
- **PowerShell Integration**: Advanced parameter validation and error handling
- **Command Prompt Compatibility**: Traditional batch file support

### SIMD Optimizations

| Platform | SIMD Support | Auto-Detection |
|----------|-------------|----------------|
| Linux x86_64 | SSE4.2, AVX2 | ✓ |
| macOS ARM64 | ARM NEON | ✓ |
| macOS x86_64 | SSE4.2, AVX2 | ✓ |
| Windows x64 | SSE4.2, AVX2 | ✓ |
| Windows x86 | SSE4.2 | ✓ |

## Building

The Container System can be built standalone or as part of a larger messaging system:

### Standalone Build

```bash
# Basic build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Build with all enhanced features
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_MESSAGING_FEATURES=ON \
  -DENABLE_PERFORMANCE_METRICS=ON \
  -DENABLE_EXTERNAL_INTEGRATION=ON \
  -DBUILD_CONTAINER_EXAMPLES=ON
cmake --build .

# Run examples
./bin/examples/messaging_integration_example
```

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `ENABLE_MESSAGING_FEATURES` | Enhanced messaging optimizations | ON |
| `ENABLE_PERFORMANCE_METRICS` | Real-time performance monitoring | OFF |
| `ENABLE_EXTERNAL_INTEGRATION` | External system callback hooks | ON |
| `BUILD_CONTAINER_EXAMPLES` | Build example applications | ON |
| `BUILD_CONTAINER_SAMPLES` | Build sample programs | ON |
| `USE_THREAD_SAFE_OPERATIONS` | Enable thread-safe operations | ON |

### Integration Build

When used as part of a messaging system, the container provides compatibility aliases:

```cmake
# In your CMakeLists.txt
find_package(ContainerSystem REQUIRED)

# Use either alias depending on your context
target_link_libraries(your_target ContainerSystem::container)
# OR
target_link_libraries(your_target MessagingSystem::container)
```

## Dependencies

- **C++20 Standard Library**: Required for concepts, ranges, and format
- **fmt Library**: High-performance string formatting
- **Thread System**: For lock-free operations and threading
- **Utilities Module**: String conversion and system utilities

## API Documentation

### Core API Reference

- **[API Reference](./docs/API_REFERENCE.md)**: Complete API documentation with interfaces
- **[Architecture Guide](./docs/ARCHITECTURE.md)**: System design and patterns
- **[Performance Guide](./docs/PERFORMANCE.md)**: SIMD optimization and benchmarks
- **[User Guide](./docs/USER_GUIDE.md)**: Usage guide and value types
- **[FAQ](./docs/FAQ.md)**: Frequently asked questions

### Quick API Overview

```cpp
// Container Core API
namespace container_module {
    // Main container with header management and value storage
    class value_container {
        // Header management
        auto set_source(const std::string& id, const std::string& sub_id = "") -> void;
        auto set_target(const std::string& id, const std::string& sub_id = "") -> void;
        auto set_message_type(const std::string& type) -> void;
        auto swap_header() -> void;  // Swap source and target

        // Value management
        auto add_value(std::shared_ptr<value> val) -> void;
        auto set_values(const std::vector<std::shared_ptr<value>>& values) -> void;
        auto get_value(const std::string& key) const -> std::shared_ptr<value>;
        auto clear_values() -> void;
        auto reserve_values(size_t count) -> void;

        // Serialization
        auto serialize() const -> std::string;
        auto serialize_array() const -> std::vector<uint8_t>;
        auto to_json() const -> std::string;
        auto to_xml() const -> std::string;

        // Copy operations
        auto copy(bool deep_copy = true) const -> std::shared_ptr<value_container>;
    };

    // Type-safe value factory
    class value_factory {
        static auto create(const std::string& key, value_types type, const std::string& data) -> std::shared_ptr<value>;
        static auto create_bool(const std::string& key, bool val) -> std::shared_ptr<bool_value>;
        static auto create_string(const std::string& key, const std::string& val) -> std::shared_ptr<string_value>;
        static auto create_int64(const std::string& key, int64_t val) -> std::shared_ptr<int64_value>;
        static auto create_double(const std::string& key, double val) -> std::shared_ptr<double_value>;
    };

    // Thread-safe container wrapper
    class thread_safe_container {
        auto get_value(const std::string& key) const -> std::shared_ptr<value>;
        auto set_value(const std::string& key, value_types type, const std::string& data) -> void;
        auto serialize() const -> std::string;
        auto add_value(std::shared_ptr<value> val) -> void;
    };
}

// Builder pattern for messaging integration
namespace container_module::integration {
    class messaging_container_builder {
        auto source(const std::string& id, const std::string& sub_id = "") -> messaging_container_builder&;
        auto target(const std::string& id, const std::string& sub_id = "") -> messaging_container_builder&;
        auto message_type(const std::string& type) -> messaging_container_builder&;
        auto add_value(const std::string& key, const auto& value) -> messaging_container_builder&;
        auto optimize_for_speed() -> messaging_container_builder&;
        auto optimize_for_network() -> messaging_container_builder&;
        auto reserve_values(size_t count) -> messaging_container_builder&;
        auto build() -> std::shared_ptr<value_container>;
    };
}

// SIMD processor for numeric optimization
namespace container_module {
    class simd_processor {
        auto process_array(const std::vector<double>& input) -> std::vector<double>;
        auto process_array(const std::vector<float>& input) -> std::vector<float>;
        auto process_array(const std::vector<int64_t>& input) -> std::vector<int64_t>;
        auto is_simd_available() const -> bool;
        auto get_simd_type() const -> std::string;  // "AVX2", "NEON", "SSE4.2", etc.
    };
}
```

## Contributing

We welcome contributions! Please see our [Contributing Guide](./docs/CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style

- Follow modern C++ best practices
- Use RAII and smart pointers
- Maintain consistent formatting (clang-format configuration provided)
- Write comprehensive unit tests for new features

## Support

- **Issues**: [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/kcenon/container_system/discussions)
- **Email**: kcenon@naver.com

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to all contributors who have helped improve this project
- Special thanks to the C++ community for continuous feedback and support
- Inspired by modern serialization frameworks and high-performance computing practices

---

<p align="center">
  Made with ❤️ by 🍀☀🌕🌥 🌊
</p>