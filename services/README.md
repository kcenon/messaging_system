# Messaging System Services

A modern C++20 messaging system with support for publish-subscribe patterns, service integration, and comprehensive configuration management.

## Features

- **High-Performance Message Bus**: Thread-safe message routing with priority queues
- **Service Integration**: Adapter pattern for seamless service integration
- **Dependency Injection**: Template-based service container with singleton support
- **Configuration Management**: Builder pattern for flexible system configuration
- **Comprehensive Testing**: Unit tests, integration tests, and performance benchmarks
- **Modern C++20**: Utilizes latest C++ features including concepts, ranges, and std::jthread

## Architecture

### Core Components

- **Message Bus** (`message_bus`): Central message routing with worker threads
- **Message Types** (`message_types`): Comprehensive message structures with metadata
- **Configuration** (`config`): Unified configuration system with builder pattern

### Service Architecture

- **Service Interface** (`service_interface`): Base interface with lifecycle management
- **Service Adapter** (`service_adapter`): Adapter pattern for message bus integration
- **Container Service**: Message serialization and compression services
- **Network Service**: Network messaging and broadcasting capabilities

### Integration Layer

- **Service Container** (`service_container`): Dependency injection with factory pattern
- **System Integrator** (`system_integrator`): Main orchestrator and entry point
- **Messaging System Orchestrator**: Service lifecycle coordinator

## Quick Start

### Basic Usage

```cpp
#include <kcenon/messaging/integrations/system_integrator.h>

using namespace kcenon::messaging::integrations;
using namespace kcenon::messaging::config;

// Create and configure the system
config_builder builder;
auto config = builder
    .set_environment("development")
    .set_worker_threads(4)
    .set_queue_size(10000)
    .enable_compression(true)
    .build();

auto integrator = std::make_unique<system_integrator>(config);

// Initialize
integrator->initialize();

// Subscribe to messages
integrator->subscribe("user.login", [](const message& msg) {
    std::cout << "User logged in!" << std::endl;
});

// Publish messages
message_payload payload;
payload.topic = "user.login";
payload.data["username"] = std::string("john_doe");
integrator->publish("user.login", payload, "auth_service");

// Shutdown
integrator->shutdown();
```

### Configuration

The system supports flexible configuration through the builder pattern:

```cpp
config_builder builder;
auto config = builder
    .set_environment("production")        // Environment: development, staging, production
    .set_worker_threads(8)               // Number of worker threads
    .set_queue_size(50000)               // Maximum queue size
    .enable_priority_queue(true)         // Enable priority-based message processing
    .enable_compression(true)            // Enable message compression
    .enable_external_logger(true)       // Use external logger system
    .enable_external_monitoring(true)   // Use external monitoring system
    .set_system_name("my_messaging_system")
    .build();
```

### Service Integration

Services can be easily integrated using the adapter pattern:

```cpp
// Create a custom service
class MyService : public service_interface {
    // Implement service interface methods
};

// Create adapter
auto service = std::make_shared<MyService>();
auto adapter = std::make_shared<MyServiceAdapter>(service);

// Register with system
integrator->get_orchestrator().register_service_adapter("my_service", adapter);
```

## Building

### Prerequisites

- C++20 compatible compiler
- CMake 3.16 or higher
- GTest (for testing)

### Build Instructions

```bash
# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

# Build
cmake --build build

# Run tests
cd build && ctest

# Run benchmarks
./build/bin/messaging_benchmark
```

### Build Options

- `BUILD_TESTING=ON/OFF`: Enable/disable test building (default: OFF)
- `BUILD_NETWORK=ON/OFF`: Enable/disable network module (default: ON)
- `BUILD_CONTAINER=ON/OFF`: Enable/disable container module (default: ON)
- `BUILD_DATABASE=ON/OFF`: Enable/disable database module (default: ON)

## Testing

The system includes comprehensive testing:

### Unit Tests

```bash
# Run all tests
./build/bin/messaging_services_tests

# Run specific test groups
./build/bin/messaging_services_tests --gtest_filter="MessageBus*"
./build/bin/messaging_services_tests --gtest_filter="ServiceContainer*"
./build/bin/messaging_services_tests --gtest_filter="Integration*"
```

### Performance Benchmarks

```bash
# Run performance benchmarks
./build/bin/messaging_benchmark
```

Benchmark results include:
- Message throughput (messages/second)
- Concurrent publishing performance
- Priority queue performance
- Message size impact analysis
- System integration performance

## API Reference

### System Integrator

Main entry point for the messaging system:

```cpp
class system_integrator {
public:
    // System lifecycle
    bool initialize();
    void shutdown();
    bool is_running() const;

    // Message operations
    bool publish(const std::string& topic, const message_payload& payload, const std::string& sender = "");
    void subscribe(const std::string& topic, message_handler handler);

    // Service access
    template<typename T> std::shared_ptr<T> get_service(const std::string& name);
    service_container& get_container();

    // System monitoring
    system_health check_system_health() const;

    // Factory methods
    static std::unique_ptr<system_integrator> create_default();
    static std::unique_ptr<system_integrator> create_for_environment(const std::string& environment);
};
```

### Message Types

Core message structures:

```cpp
struct message_payload {
    std::string topic;
    std::unordered_map<std::string, message_value> data;
    std::vector<uint8_t> binary_data;
};

struct message_metadata {
    std::chrono::system_clock::time_point timestamp;
    message_priority priority = message_priority::normal;
    std::chrono::milliseconds timeout{30000};
    std::unordered_map<std::string, std::string> headers;
    std::string correlation_id;
    std::string reply_to;
};

struct message {
    message_metadata metadata;
    message_payload payload;
    std::string sender;
};
```

### Configuration

Configuration structures and builder:

```cpp
struct messaging_config {
    message_bus_config message_bus;
    container_config container;
    network_config network;
    std::string environment;
    std::string system_name;
    // ... other configuration options
};

class config_builder {
public:
    config_builder& set_worker_threads(size_t threads);
    config_builder& set_queue_size(size_t size);
    config_builder& enable_priority_queue(bool enable = true);
    config_builder& enable_compression(bool enable = true);
    config_builder& set_environment(const std::string& env);
    messaging_config build();
};
```

## Performance

Typical performance characteristics (may vary based on hardware):

- **Message Throughput**: 10,000+ messages/second (single publisher)
- **Concurrent Publishing**: 8,000+ messages/second (8 concurrent publishers)
- **Priority Queue**: 5,000+ messages/second with priority ordering
- **Memory Usage**: Efficient memory management with configurable limits
- **Latency**: Sub-millisecond message routing

## Examples

See the `examples/` directory for comprehensive usage examples:

- `basic_usage_example.cpp`: Complete system usage demonstration
- `benchmarks/message_bus_benchmark.cpp`: Performance testing example

## License

This project is part of the KCENON messaging system framework.

## Contributing

1. Follow the coding standards defined in CLAUDE.md
2. Add comprehensive tests for new features
3. Update documentation for API changes
4. Run performance benchmarks for performance-critical changes