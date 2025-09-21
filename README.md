# Messaging System

A production-ready, high-performance C++20 distributed messaging framework engineered for real-time applications. Built with lock-free data structures, SIMD optimizations, and enterprise-grade reliability, it delivers sub-microsecond latency and millions of messages per second throughput.

## 📁 Project Structure

```
messaging_system/
├── libraries/           # All system libraries
│   ├── container_system/    # Type-safe, SIMD-optimized data containers
│   ├── database_system/     # PostgreSQL integration with connection pooling
│   ├── network_system/      # Asynchronous TCP client/server
│   ├── thread_system/       # Lock-free thread pools
│   ├── logger_system/       # High-performance logging
│   └── monitoring_system/   # System monitoring and metrics
├── services_system/     # Application service layer with Python bindings
├── test/               # Unit and integration tests
├── scripts/            # Build and setup scripts
└── cmake/              # CMake configuration files
```

## ✨ Key Features

### 🚀 Performance
- **2.48M messages/second** throughput with lock-free architecture
- **Sub-microsecond latency** for in-memory operations
- **Linear scalability** up to 32+ CPU cores
- **SIMD optimizations** for ARM NEON and x86 AVX
- **Zero-copy operations** in critical paths

### 🏗️ Architecture
- **Lock-free data structures** with hazard pointer memory management
- **Type-safe containers** with compile-time guarantees
- **Pluggable components** via dependency injection
- **Event-driven design** with async/await patterns
- **Distributed clustering** with automatic failover

### 🔧 Enterprise Features
- **PostgreSQL integration** with connection pooling and transactions
- **SSL/TLS security** with certificate validation
- **Message persistence** and replay capabilities
- **Comprehensive monitoring** with Prometheus metrics
- **Production-ready samples** for common use cases

### 🌐 Integration
- **Python bindings** for rapid development
- **REST API** for HTTP/HTTPS clients
- **Protocol adapters** for RabbitMQ, Kafka, Redis
- **Docker & Kubernetes** deployment ready
- **Cross-platform** support for Linux, macOS, Windows

## 📋 Requirements

### System Requirements
- **Compiler**: C++20 compatible (GCC 10+, Clang 12+, MSVC 2019+)
- **Build System**: CMake 3.16 or later
- **Package Manager**: vcpkg (included as submodule)
- **Platforms**: Linux, macOS 10.15+, Windows 10+

### Runtime Dependencies
- **PostgreSQL**: 12+ (for database module)
- **Python**: 3.8+ (for Python bindings, optional)

### Development Dependencies
- **Testing**: Google Test 1.17+ (included via vcpkg)
- **Benchmarking**: Google Benchmark (included via vcpkg)
- **Documentation**: Doxygen 1.8+ (optional)

## Thread System Integration

This messaging system now includes the latest thread_system with:

- **Lock-free Components**: High-performance lock-free queues and thread pools
- **Typed Thread Pools**: Priority-based job scheduling (RealTime, Batch, Background)
- **Hazard Pointers**: Safe memory reclamation for lock-free data structures
- **Performance**: 2.14x throughput improvement with lock-free implementations

## 🏗️ Quick Start

### 1. Clone and Setup
```bash
git clone <repository-url> messaging_system
cd messaging_system
git submodule update --init --recursive
```

### 2. Build and Run
```bash
# Quick build with all features
./scripts/build.sh

# Run sample application
./build/bin/production_ready_example

# Run tests
./scripts/build.sh --tests
```

### 3. Basic Usage
```cpp
#include <kcenon/messaging/core/message_bus.h>

int main() {
    // Create message bus
    kcenon::messaging::core::message_bus bus;
    bus.initialize();

    // Subscribe to messages
    bus.subscribe("user.created", [](const auto& msg) {
        std::cout << "New user: " << msg.get_payload().get<std::string>("name") << std::endl;
        return kcenon::messaging::core::message_status::processed;
    });

    // Publish message
    kcenon::messaging::core::message_payload payload;
    payload.set("name", "John Doe");
    bus.publish("user.created", payload);

    return 0;
}
```

For detailed setup instructions, see the [Developer Guide](docs/DEVELOPER_GUIDE.md).

## 🎯 Sample Applications

The `application_layer/samples/` directory contains 8 production-ready examples:

1. **[Basic Usage](application_layer/samples/basic_usage_example.cpp)** - Simple pub/sub messaging
2. **[Chat Server](application_layer/samples/chat_server.cpp)** - Real-time chat with rooms
3. **[IoT Monitoring](application_layer/samples/iot_monitoring.cpp)** - Device monitoring system
4. **[Event Pipeline](application_layer/samples/event_pipeline.cpp)** - Stream processing
5. **[Distributed Worker](application_layer/samples/distributed_worker.cpp)** - Task distribution
6. **[Microservices Orchestrator](application_layer/samples/microservices_orchestrator.cpp)** - Service mesh
7. **[Message Bus Benchmark](application_layer/samples/message_bus_benchmark.cpp)** - Performance testing
8. **[Production Ready Example](application_layer/samples/production_ready_example.cpp)** - Full-featured application

Run any sample:
```bash
./build/bin/chat_server
./build/bin/iot_monitoring
./build/bin/production_ready_example
```

## 📚 Module Overview

### 📦 Container Module (`container/`)
**Type-safe, high-performance data containers**
- **Thread Safety**: Lock-free and mutex-based options
- **Value Types**: Bool, numeric (int8-int64), string, bytes, nested containers
- **SIMD Optimization**: ARM NEON and x86 AVX support for numeric operations
- **Serialization**: Binary and JSON serialization with compression
- **Memory Management**: Efficient variant storage with minimal allocations

### 🗄️ Database Module (`database/`)
**PostgreSQL integration with enterprise features**
- **Connection Pool**: Thread-safe connection management with configurable pool sizes
- **Prepared Statements**: Query optimization and SQL injection protection
- **Async Operations**: Non-blocking database operations with coroutine support
- **Transaction Support**: ACID compliance with nested transaction handling
- **Error Recovery**: Automatic connection recovery and query retry mechanisms

### 🌐 Network Module (`network/`)
**High-performance TCP messaging infrastructure**
- **Asynchronous I/O**: ASIO-based coroutine implementation
- **Client/Server**: Full-duplex messaging with session management
- **Protocol Support**: Binary and text protocols with custom message framing
- **Pipeline Processing**: Message transformation and routing capabilities
- **Load Balancing**: Built-in support for distributed server architectures

### 🧵 Thread System (`thread_system/`)
**Lock-free concurrent processing framework**
- **Lock-free Queues**: Multi-producer, multi-consumer queues with hazard pointers
- **Typed Scheduling**: Priority-based job scheduling (RealTime, Batch, Background)
- **Memory Safety**: Hazard pointer-based memory reclamation
- **Performance Monitoring**: Real-time metrics collection and reporting
- **Cross-platform**: Optimized implementations for different CPU architectures

## 💡 Usage Examples

### Basic Container Operations
```cpp
#include <container/container.h>
using namespace container_module;

int main() {
    // Create a type-safe container
    auto container = std::make_shared<value_container>();
    
    // Set various value types
    container->set_source("client_01", "session_1");
    container->set_target("server", "main");
    container->set_message_type("user_data");
    
    // Serialize and deserialize
    std::string serialized = container->serialize();
    auto restored = std::make_shared<value_container>(serialized);
    
    return 0;
}
```

### Network Client/Server
```cpp
#include <network/messaging_server.h>
#include <network/messaging_client.h>
using namespace network_module;

int main() {
    // Create server
    auto server = std::make_shared<messaging_server>("main_server");
    server->start_server(8080);
    
    // Create client
    auto client = std::make_shared<messaging_client>("client_01");
    client->start_client("127.0.0.1", 8080);
    
    // Server and client automatically handle messaging
    
    // Cleanup
    client->stop_client();
    server->stop_server();
    
    return 0;
}
```

### Database Operations
```cpp
#include <database/database_manager.h>
using namespace database;

int main() {
    // Configure database
    database_manager manager;
    manager.set_mode(database_types::postgres);
    
    // Connect and execute queries
    if (manager.connect("host=localhost dbname=mydb")) {
        auto result = manager.select_query("SELECT * FROM users");
        // Process results...
        manager.disconnect();
    }
    
    return 0;
}
```

### Python Integration
```python
from messaging_system import Container, MessagingClient, Value
import time

# Create and populate a container
container = Container()
container.create(
    target_id="server",
    source_id="python_client", 
    message_type="data_update",
    values=[
        Value("user_id", "9", "12345"),
        Value("message", "f", "Hello from Python!"),
        Value("timestamp", "9", str(int(time.time())))
    ]
)

# Network client
client = MessagingClient(
    source_id="py_client",
    connection_key="secure_key", 
    recv_callback=lambda msg: print(f"Received: {msg}")
)

if client.start("localhost", 8080):
    # Send container as message
    serialized = container.serialize()
    client.send_raw_message(serialized)
    client.stop()
```

## 📊 Performance Characteristics

### Benchmark Results (AMD Ryzen/Intel Core, 16 threads)

| Component | Metric | Lock-free | Mutex-based | Improvement |
|-----------|--------|-----------|-------------|-------------|
| **Thread Pool** | Throughput | 2.48M jobs/sec | 1.16M jobs/sec | **2.14x** |
| **Job Queue** | Latency | 250ns | 1.2μs | **4.8x** |
| **Container** | Serialization | 15M ops/sec | 12M ops/sec | **1.25x** |
| **Network** | Connections | 10K concurrent | 8K concurrent | **1.25x** |

### Performance Features
- **Sub-microsecond Latency**: Job scheduling and message processing
- **Linear Scalability**: Performance scales with CPU cores up to 32+ threads  
- **Memory Efficiency**: Hazard pointer-based memory reclamation eliminates GC pauses
- **SIMD Optimization**: ARM NEON and x86 AVX for numeric operations
- **Zero-copy Operations**: Minimize memory allocations in hot paths

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                    │
├─────────────────────────────────────────────────────────┤
│  C++ API          │  Python Bindings  │  REST API      │
├─────────────────────────────────────────────────────────┤
│  Container        │  Database         │  Network        │
│  Module           │  Module           │  Module         │
├─────────────────────────────────────────────────────────┤
│              Thread System (Lock-free)                  │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐ │
│  │Thread Pools │ │ Job Queues  │ │ Hazard Pointers     │ │
│  └─────────────┘ └─────────────┘ └─────────────────────┘ │
├─────────────────────────────────────────────────────────┤
│                  Foundation Layer                       │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐ │
│  │   Utilities │ │  Monitoring │ │    Logging          │ │
│  └─────────────┘ └─────────────┘ └─────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## 🧪 Testing

### Test Coverage
- **Container Tests**: 15 unit tests covering serialization, threading, SIMD operations
- **Database Tests**: 14 unit tests covering connections, queries, error handling  
- **Network Tests**: 14 unit tests covering client/server, sessions, protocols
- **Integration Tests**: 8 cross-module integration tests
- **Performance Tests**: Benchmark suite with regression detection

### Running Tests
```bash
# All tests with coverage
./build.sh --tests

# Specific test modules  
cd build/bin
./container_test --gtest_filter="ContainerTest.*"
./database_test --gtest_filter="DatabaseTest.SingletonAccess"
./network_test --gtest_filter="NetworkTest.ServerClientIntegration"

# Performance benchmarks
cd build
./thread_system/bin/lockfree_performance_benchmark
./thread_system/bin/container_benchmark
```

## 🤝 Contributing

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feature/amazing-feature`
3. **Test** your changes: `./build.sh --tests`
4. **Format** code: Follow the project's C++ style guide
5. **Commit** changes: `git commit -m 'feat: add amazing feature'`
6. **Push** to branch: `git push origin feature/amazing-feature`
7. **Submit** a Pull Request

### Development Guidelines
- Follow C++20 best practices and RAII principles
- Maintain thread safety in all public APIs
- Add unit tests for new functionality
- Update documentation for API changes
- Use meaningful commit messages (conventional commits)

## 📚 Documentation

### Core Documentation
- **[System Architecture](docs/SYSTEM_ARCHITECTURE.md)** - Complete system design and components
- **[API Reference](docs/API_REFERENCE.md)** - Comprehensive API documentation
- **[Developer Guide](docs/DEVELOPER_GUIDE.md)** - Getting started and best practices
- **[Deployment Guide](docs/DEPLOYMENT_GUIDE.md)** - Production deployment strategies

### Advanced Topics
- **[Design Patterns](docs/DESIGN_PATTERNS.md)** - Architectural patterns and decisions
- **[Troubleshooting](docs/TROUBLESHOOTING.md)** - Debugging and optimization guide
- **[Performance Tuning](docs/TROUBLESHOOTING.md#performance-optimization)** - Optimization techniques
- **[Sample Applications](application_layer/samples/SAMPLES_README.md)** - Production examples

## 📄 License

**BSD 3-Clause License**

Copyright (c) 2021-2025, Messaging System Contributors  
All rights reserved.

See [LICENSE](LICENSE) file for full license text.