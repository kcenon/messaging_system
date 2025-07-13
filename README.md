# Messaging System

A high-performance C++20 messaging framework designed for distributed applications. Built on lock-free thread pools and featuring type-safe containers, PostgreSQL integration, and asynchronous TCP networking.

## 🚀 Key Features

### 🎯 Core Modules
- **Container Module**: Type-safe, SIMD-optimized data containers with thread-safe operations
- **Database Module**: PostgreSQL integration with connection pooling and prepared statements  
- **Network Module**: Asynchronous TCP client/server with coroutine-based I/O
- **Thread System**: Lock-free thread pools with hazard pointer memory management

### 🌟 Advanced Capabilities
- **Lock-free Performance**: Up to 2.48M jobs/second throughput with sub-microsecond latency
- **Type Safety**: Comprehensive type system with variant values and memory safety
- **Python Bindings**: Complete Python API for rapid prototyping and integration
- **Cross-platform**: Linux, macOS, and Windows support with ARM64 optimization

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

### 2. Install Dependencies
```bash
# Automated dependency installation
./dependency.sh

# On macOS with Homebrew
brew install postgresql cmake

# On Ubuntu/Debian  
sudo apt update && sudo apt install postgresql-dev cmake build-essential
```

### 3. Build the Project
```bash
# Quick build (Release mode)
./build.sh

# Build with tests
./build.sh --tests

# Clean build
./build.sh --clean

# Build specific modules only
./build.sh --no-database --no-python
```

### 4. Run Tests
```bash
# Run all tests
./build.sh --tests

# Or run individual test suites
cd build/bin
./container_test
./database_test
./network_test
./integration_test
```

### Manual CMake Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
         -DCMAKE_BUILD_TYPE=Release \
         -DUSE_UNIT_TEST=ON
cmake --build . --parallel
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

- **API Reference**: [Generated documentation](./docs/html/index.html)
- **Architecture Guide**: [Architecture overview](./docs/architecture.md)
- **Performance Guide**: [Optimization tips](./docs/performance.md)
- **Thread System Docs**: [Thread system documentation](./thread_system/docs/)

## 📄 License

**BSD 3-Clause License**

Copyright (c) 2021-2025, Messaging System Contributors  
All rights reserved.

See [LICENSE](LICENSE) file for full license text.