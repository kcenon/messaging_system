# Messaging System

> **Language:** **English** | [한국어](README_KO.md)

<div align="center">

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![CI](https://github.com/kcenon/messaging_system/actions/workflows/ci.yml/badge.svg)](https://github.com/kcenon/messaging_system/actions/workflows/ci.yml)
[![Coverage](https://github.com/kcenon/messaging_system/actions/workflows/coverage.yml/badge.svg)](https://github.com/kcenon/messaging_system/actions/workflows/coverage.yml)
[![Static Analysis](https://github.com/kcenon/messaging_system/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/kcenon/messaging_system/actions/workflows/static-analysis.yml)

**Production-Ready Messaging Infrastructure with Advanced Patterns**

Complete pub/sub, request/reply, event streaming, and message pipeline support

[📚 Documentation](#-documentation) | [🔗 Quick Start](#-quick-start) | [📖 Examples](#-usage-examples) | [📊 Architecture](#-architecture) | [🔄 Contributing](#-contributing)

</div>

---

## ✨ Overview

A **production-ready messaging system** built on modern C++20 with comprehensive support for enterprise messaging patterns. The system provides a complete messaging infrastructure with pluggable backends, advanced routing capabilities, and robust reliability features.

### 🎯 Key Features

<table>
<tr>
<td width="50%">

**Core Messaging**
- **Message Bus** - Central pub/sub coordinator
- **Topic Router** - Wildcard pattern matching (*, #)
- **Message Queue** - Thread-safe priority queues
- **Message Serialization** - Container-based payloads
- **Trace Context** - Distributed tracing support

</td>
<td width="50%">

**Advanced Patterns**
- **Pub/Sub** - Publisher and subscriber helpers
- **Request/Reply** - Synchronous RPC over async messaging
- **Event Streaming** - Event sourcing with replay
- **Message Pipeline** - Pipes-and-filters processing
- **DI Container** - Dependency injection support

</td>
</tr>
<tr>
<td width="50%">

**Backend Support**
- **Standalone** - Self-contained execution
- **Integration** - Thread pool integration
- **Auto-detection** - Runtime backend selection
- **Pluggable Executors** - IExecutor abstraction
- **Mock Support** - Testing and development

</td>
<td width="50%">

**Production Quality**
- **Thread-safe** - Lock-based and atomic operations
- **Type-safe** - Result<T> error handling
- **Well-tested** - 100+ unit/integration tests
- **Benchmarked** - Performance validated
- **Documented** - Comprehensive API docs

</td>
</tr>
</table>

---

## 📦 What's Included

The messaging system is fully implemented and production-ready:

### ✅ Core Components (Complete)

- **Message Types** - Structured messages with metadata, priority, TTL
- **Message Builder** - Fluent API for message construction
- **Message Queue** - Thread-safe FIFO and priority queues
- **Topic Router** - Pattern-based routing with wildcards
- **Message Bus** - Async/sync pub/sub coordination
- **Message Broker** - Advanced routing and filtering

### ✅ Patterns (Complete)

- **Pub/Sub Pattern** - Publisher and Subscriber classes
- **Request/Reply Pattern** - Request client and server
- **Event Streaming** - Event sourcing with replay capability
- **Message Pipeline** - Stage-based message processing

### ✅ Infrastructure (Complete)

- **Backend Interface** - Pluggable execution backends
- **Standalone Backend** - Self-contained thread pool
- **Integration Backend** - External thread pool integration
- **DI Container** - Service registration and resolution
- **Error Codes** - Centralized error handling (-700 to -799)

### ✅ Testing & Benchmarks (Complete)

- **Unit Tests** - 100+ tests across all components
- **Integration Tests** - End-to-end messaging scenarios
- **Pattern Tests** - Comprehensive pattern validation
- **Performance Benchmarks** - Throughput and latency testing

### ✅ Documentation (Complete)

- **API Reference** - Complete API documentation
- **Migration Guide** - Upgrade instructions
- **Patterns API** - Pattern usage guide
- **Design Patterns** - Architecture documentation

---

## 🏗️ Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Messaging Patterns                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │ Pub/Sub  │  │ Req/Rep  │  │ Pipeline │  │ Stream   │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Message Bus                             │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │ Message Broker   │  │ Topic Router     │                │
│  └──────────────────┘  └──────────────────┘                │
└─────────────────────────────────────────────────────────────┘
                              │
                ┌─────────────┴─────────────┐
                ▼                           ▼
┌───────────────────────────┐   ┌───────────────────────────┐
│   Message Queue           │   │  Subscription Manager     │
│  ┌─────────────────────┐  │   │  ┌─────────────────────┐ │
│  │ Priority Queue      │  │   │  │ Subscriber Registry │ │
│  │ Dead Letter Queue   │  │   │  │ Filter Chain        │ │
│  └─────────────────────┘  │   │  └─────────────────────┘ │
└───────────────────────────┘   └───────────────────────────┘
                              │
                ┌─────────────┴─────────────┐
                ▼                           ▼
┌───────────────────────────┐   ┌───────────────────────────┐
│  Integration Layer        │   │   Backend Selection       │
│  ┌──────────────────┐     │   │  ┌──────────────────┐    │
│  │ Logger Adapter   │     │   │  │ Standalone       │    │
│  │ Monitor Adapter  │     │   │  │ Integration      │    │
│  │ Thread Adapter   │     │   │  │ Auto-detect      │    │
│  └──────────────────┘     │   │  └──────────────────┘    │
└───────────────────────────┘   └───────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              Foundation (common_system)                      │
│   Result<T>, error_info, event_bus, interfaces              │
└─────────────────────────────────────────────────────────────┘
```

### Message Flow

```
Publisher → Message Bus → Topic Router → Subscribers
    ↓           ↓              ↓             ↓
  Message   Priority      Pattern      Callbacks
  Builder    Queue        Matching     (async)
```

---

## 🚀 Quick Start

### Prerequisites

- **C++20** compiler (GCC 10+, Clang 12+, MSVC 2019+)
- **CMake** 3.16+
- **vcpkg** (for dependency management)

### Installation

```bash
git clone https://github.com/kcenon/messaging_system.git
cd messaging_system

# Build with vcpkg (recommended)
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build -j

# Or use local systems
cmake -B build -DMESSAGING_USE_LOCAL_SYSTEMS=ON
cmake --build build -j
```

### Basic Example

```cpp
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/backends/standalone_backend.h>

using namespace kcenon::messaging;

int main() {
    // Create standalone backend
    auto backend = std::make_shared<standalone_backend>(4);
    backend->initialize();

    // Create message bus
    message_bus_config config;
    config.worker_threads = 4;
    auto bus = std::make_shared<message_bus>(backend, config);
    bus->start();

    // Subscribe to topic
    bus->subscribe("user.created", [](const message& msg) {
        std::cout << "User created: " << msg.metadata().id << std::endl;
        return common::VoidResult::ok();
    });

    // Publish message
    auto msg = message_builder()
        .topic("user.created")
        .source("app")
        .type(message_type::event)
        .build();

    if (msg.is_ok()) {
        bus->publish(msg.value());
    }

    // Cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bus->stop();
    backend->shutdown();

    return 0;
}
```

---

## 📖 Usage Examples

### Pub/Sub Pattern

```cpp
#include <kcenon/messaging/patterns/pub_sub.h>

using namespace kcenon::messaging::patterns;

// Create publisher
auto publisher = std::make_shared<patterns::publisher>(bus, "events");

// Publish events
auto msg = message_builder()
    .topic("events.user.login")
    .type(message_type::event)
    .build();
publisher->publish(msg.value());

// Create subscriber
auto subscriber = std::make_shared<patterns::subscriber>(bus);
subscriber->subscribe("events.user.*", [](const message& msg) {
    // Handle user events
    return common::VoidResult::ok();
});
```

### Request/Reply Pattern

```cpp
#include <kcenon/messaging/patterns/request_reply.h>

using namespace kcenon::messaging::patterns;

// Server side
auto server = std::make_shared<request_server>(bus, "calculator");
server->register_handler([](const message& request) -> common::Result<message> {
    auto a = request.payload().get_value("a").value<int>();
    auto b = request.payload().get_value("b").value<int>();

    auto response = message_builder()
        .topic("calculator.response")
        .type(message_type::reply)
        .build();
    response.value().payload().set_value("result", a + b);

    return response;
});

// Client side
auto client = std::make_shared<request_client>(bus);
auto request = message_builder()
    .topic("calculator.add")
    .type(message_type::query)
    .build();
request.value().payload().set_value("a", 10);
request.value().payload().set_value("b", 20);

auto response = client->request(request.value(), std::chrono::seconds(5));
if (response.is_ok()) {
    auto result = response.value().payload().get_value("result").value<int>();
    std::cout << "Result: " << result << std::endl;  // 30
}
```

### Event Streaming

```cpp
#include <kcenon/messaging/patterns/event_streaming.h>

using namespace kcenon::messaging::patterns;

// Create event stream
auto stream = std::make_shared<event_stream>(bus, "orders");

// Publish events
stream->publish_event("order.created", order_data);
stream->publish_event("order.paid", payment_data);
stream->publish_event("order.shipped", shipping_data);

// Replay events
stream->replay_from(std::chrono::system_clock::now() - std::chrono::hours(24),
    [](const message& event) {
        // Process historical events
        return common::VoidResult::ok();
    });
```

### Message Pipeline

```cpp
#include <kcenon/messaging/patterns/message_pipeline.h>

using namespace kcenon::messaging::patterns;

// Build processing pipeline
auto pipeline = pipeline_builder()
    .add_stage("validate", [](message& msg) {
        // Validation logic
        return common::VoidResult::ok();
    })
    .add_stage("transform", [](message& msg) {
        // Transformation logic
        return common::VoidResult::ok();
    })
    .add_stage("enrich", [](message& msg) {
        // Enrichment logic
        return common::VoidResult::ok();
    })
    .build();

// Process message through pipeline
auto msg = message_builder().topic("data").build();
auto result = pipeline.process(msg.value());
```

---

## 🌟 Integration with Base Systems

The messaging system integrates seamlessly with specialized base systems:

### Required Systems

- **common_system** - Result<T> pattern, error handling, base interfaces
- **container_system** - Type-safe message payloads and serialization

### Optional Systems

- **thread_system** - High-performance thread pools for message dispatch
- **logger_system** - Structured logging for debugging and audit trails
- **monitoring_system** - Real-time metrics and performance telemetry
- **database_system** - Message persistence and audit logging
- **network_system** - Distributed messaging over TCP/IP

### Integration Example

```cpp
#include <kcenon/messaging/backends/integration_backend.h>
#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/logger/core/logger.h>

// Create thread pool from thread_system
auto thread_pool = std::make_shared<thread::thread_pool>(8);

// Create logger from logger_system
auto logger = logger::create_logger("messaging");

// Create integration backend
auto backend = std::make_shared<integration_backend>(
    thread_pool,
    logger,
    nullptr  // optional monitoring
);

// Create message bus
auto bus = std::make_shared<message_bus>(backend);
bus->start();

// Messages are now dispatched via thread_system
// And logged via logger_system
```

---

## 📊 Performance

### Benchmarks

Performance characteristics based on benchmark results:

**Message Throughput**
- Message creation: ~5M messages/sec
- Queue operations: ~2M operations/sec
- Topic routing: ~500K routes/sec
- Pub/Sub: ~100K messages/sec
- Request/Reply: ~50K requests/sec

**Latency (p99)**
- Message creation: < 1 μs
- Queue enqueue/dequeue: < 2 μs
- Topic matching: < 5 μs
- End-to-end pub/sub: < 100 μs
- Request/reply: < 1 ms

### Running Benchmarks

```bash
# Build with benchmarks
cmake -B build -DMESSAGING_BUILD_BENCHMARKS=ON
cmake --build build -j

# Run benchmarks
./build/test/benchmarks/bench_message_creation
./build/test/benchmarks/bench_message_queue
./build/test/benchmarks/bench_topic_router
./build/test/benchmarks/bench_pub_sub_throughput
./build/test/benchmarks/bench_request_reply_latency
```

---

## 🧪 Testing

The system includes comprehensive test coverage:

### Test Suites

- **Core Tests** - Message, queue, router, bus (40+ tests)
- **Backend Tests** - Standalone and integration backends (27 tests)
- **Pattern Tests** - All messaging patterns (80+ tests)
- **Integration Tests** - End-to-end scenarios (4 test suites)

### Running Tests

```bash
# Build with tests
cmake -B build -DMESSAGING_BUILD_TESTS=ON
cmake --build build -j

# Run all tests
cd build
ctest --output-on-failure

# Run specific test suite
./test/unit/core/test_message_bus
./test/unit/patterns/test_pub_sub
./test/integration_tests/test_full_integration
```

### Coverage Report

```bash
# Generate coverage report
cmake -B build -DCMAKE_BUILD_TYPE=Coverage
cmake --build build -j
cd build
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

---

## 📚 Documentation

### Core Documentation

- **[Benchmarks](docs/BENCHMARKS.md)** - Performance characteristics and measurements
- **[Features](docs/FEATURES.md)** - Complete feature documentation
- **[Production Quality](docs/PRODUCTION_QUALITY.md)** - Quality assurance and reliability
- **[Project Structure](docs/PROJECT_STRUCTURE.md)** - Codebase organization
- [API Reference](docs/API_REFERENCE.md) - Complete API documentation
- [Migration Guide](docs/MIGRATION_GUIDE.md) - Upgrade instructions
- [Patterns API](docs/PATTERNS_API.md) - Messaging patterns guide
- [Design Patterns](docs/DESIGN_PATTERNS.md) - Architecture patterns

### Additional Resources

- [Examples](examples/) - Working code examples
- [Integration Tests](integration_tests/) - End-to-end test scenarios
- [Performance Benchmarks](test/benchmarks/) - Performance benchmark suite

---

## 🔧 Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `MESSAGING_USE_LOCAL_SYSTEMS` | OFF | Use sibling directories for dependencies |
| `MESSAGING_USE_FETCHCONTENT` | OFF | Auto-fetch dependencies from GitHub |
| `MESSAGING_BUILD_TESTS` | ON | Build unit and integration tests |
| `MESSAGING_BUILD_EXAMPLES` | ON | Build example programs |
| `MESSAGING_BUILD_BENCHMARKS` | OFF | Build performance benchmarks |

### Example Builds

```bash
# Development build with local systems
cmake -B build -DMESSAGING_USE_LOCAL_SYSTEMS=ON -DMESSAGING_BUILD_TESTS=ON
cmake --build build -j

# Production build with FetchContent
cmake -B build -DMESSAGING_USE_FETCHCONTENT=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build

# Debug build with all features
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
    -DMESSAGING_BUILD_TESTS=ON \
    -DMESSAGING_BUILD_EXAMPLES=ON \
    -DMESSAGING_BUILD_BENCHMARKS=ON
cmake --build build -j
```

---

## 🌐 Platform Support

| Platform | Compiler | Status |
|----------|----------|--------|
| Ubuntu 22.04 | GCC 11+ | ✅ Tested |
| Ubuntu 22.04 | Clang 14+ | ✅ Tested |
| macOS 13+ | Apple Clang | ✅ Tested |
| Windows 10+ | MSVC 2019+ | ✅ Tested |

---

## 🤝 Contributing

Contributions are welcome!

### How to Contribute

1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Ensure all tests pass
5. Submit a pull request

### Development Workflow

```bash
# Clone repository
git clone https://github.com/kcenon/messaging_system.git
cd messaging_system

# Create feature branch
git checkout -b feature/amazing-feature

# Build and test
cmake -B build -DMESSAGING_BUILD_TESTS=ON
cmake --build build -j
cd build && ctest

# Commit and push
git commit -m "feat: add amazing feature"
git push origin feature/amazing-feature
```

---

## 📄 License

**BSD 3-Clause License**

Copyright (c) 2021-2025, Messaging System Contributors
All rights reserved.

See [LICENSE](LICENSE) file for full license text.

---

## 📬 Contact & Support

- **Author**: kcenon ([@kcenon](https://github.com/kcenon))
- **Email**: kcenon@gmail.com
- **Issues**: [GitHub Issues](https://github.com/kcenon/messaging_system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/kcenon/messaging_system/discussions)

---

<div align="center">

**Built on specialized systems**

[common_system](https://github.com/kcenon/common_system) •
[thread_system](https://github.com/kcenon/thread_system) •
[logger_system](https://github.com/kcenon/logger_system) •
[monitoring_system](https://github.com/kcenon/monitoring_system) •
[container_system](https://github.com/kcenon/container_system) •
[database_system](https://github.com/kcenon/database_system) •
[network_system](https://github.com/kcenon/network_system)

Made with ❤️ by the Open Source Community

</div>
