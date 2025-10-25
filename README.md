# 🚀 Messaging System

<div align="center">

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![CI](https://github.com/kcenon/messaging_system/actions/workflows/ci.yml/badge.svg)](https://github.com/kcenon/messaging_system/actions/workflows/ci.yml)
[![Coverage](https://github.com/kcenon/messaging_system/actions/workflows/coverage.yml/badge.svg)](https://github.com/kcenon/messaging_system/actions/workflows/coverage.yml)
[![Static Analysis](https://github.com/kcenon/messaging_system/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/kcenon/messaging_system/actions/workflows/static-analysis.yml)

**Lightweight Pub/Sub Messaging System with Pluggable Components**

Integrates with 7 optional subsystems for extended functionality

[📚 Documentation](#-documentation) | [🔗 Quick Start](#-quick-start) | [📖 Examples](#-real-world-examples) | [📊 Architecture](#-system-architecture) | [🔄 Contributing](#-contributing)

</div>

---

## ✨ What Makes This Special?

This is a **lightweight, extensible messaging system** designed for seamless integration with specialized subsystems. The core provides essential pub/sub messaging, while optional integrations enable advanced features like persistence, networking, and monitoring.

### 🎯 Key Highlights

<table>
<tr>
<td width="50%">

**🚅 Core Features**
- **Pub/Sub messaging** with topic routing
- **Wildcard patterns** (* single-level, # multi-level)
- **Async/Sync** publishing modes
- **Thread-safe** operations
- **Distributed tracing** support (TraceContext)

</td>
<td width="50%">

**🛡️ Production Quality**
- **Type-safe APIs** with Result<T> pattern
- **YAML configuration** loading
- **Pluggable executors** (IExecutor interface)
- **Comprehensive testing** (unit + integration)
- **Clean abstractions** for external systems

</td>
</tr>
<tr>
<td width="50%">

**🏗️ Extensible Design**
- **Optional integrations** (network, database, monitoring)
- **Standalone mode** with MockExecutor
- **C++20** modern features
- **Application layer** for advanced scenarios
- **Header-only** integration points

</td>
<td width="50%">

**🔧 Developer Friendly**
- **Minimal dependencies** for core
- **Multiple build modes** (local/FetchContent)
- **RAII patterns** throughout
- **Clear separation** of concerns
- **Well-documented** codebase

</td>
</tr>
</table>

---

## 📋 Implementation Status

### ✅ Core Features (Fully Implemented)

The following features are complete and production-ready:

- **MessageBus** - Async/sync pub/sub coordination (`src/core/message_bus.cpp`: 106 LOC)
- **TopicRouter** - Wildcard pattern matching with `*` and `#` support (`src/core/topic_router.cpp`: 192 LOC)
- **MessagingContainer** - Type-safe message envelope with serialization (`src/core/messaging_container.cpp`: 212 LOC)
- **TraceContext** - Distributed tracing with thread-local trace ID propagation (`src/integration/trace_context.cpp`: 59 LOC)
- **ConfigLoader** - YAML-based configuration loading (`src/integration/config_loader.cpp`: 249 LOC)
- **MockExecutor** - Standalone executor implementation for testing and standalone mode (`include/messaging_system/support/mock_executor.h`: 188 LOC)

**Thread Safety**: All operations use `std::shared_mutex` for concurrent access.

### 🚧 Integration Features (Headers Only - Planned)

The following components have interface definitions but require implementation:

- **MessagingNetworkBridge** (`include/messaging_system/integration/network_bridge.h`)
  - Purpose: TCP/IP messaging via network_system
  - Status: Interface defined, implementation planned for Phase 1
  - Workaround: Use application_layer network services

- **PersistentMessageQueue** (`include/messaging_system/integration/persistent_queue.h`)
  - Purpose: Database-backed message persistence
  - Status: Interface defined, implementation planned for Phase 2
  - Workaround: Use application_layer database services

### ⚠️ Conditional Features (Requires External Systems)

These features are available when the corresponding system is detected:

- **Logger Integration** - Requires `logger_system` (`#ifdef HAS_LOGGER_SYSTEM`)
- **Monitoring/Metrics** - Requires `monitoring_system` (`#ifdef HAS_MONITORING_SYSTEM`)
- **YAML Support** - Requires `yaml-cpp` library (`#ifdef HAS_YAML_CPP`)
- **Thread Pools** - Requires `thread_system` (`#ifdef HAS_THREAD_SYSTEM`, falls back to MockExecutor)

### ❌ Not Implemented

The following features mentioned in early documentation are not currently implemented:

- **Lock-free data structures** - Current implementation uses `std::shared_mutex`
- **Automatic failover mechanisms** - Manual recovery required
- **Prometheus telemetry export** - Requires monitoring_system integration
- **Circuit breakers** - Not implemented in core
- **Built-in benchmarks** - Performance testing framework pending

---

## 🌟 Ecosystem Integration

The messaging system integrates with 7 specialized base systems through optional dependencies. Core functionality works standalone with minimal dependencies.

### Dependency Architecture

```
                    ╔══════════════════════════════╗
                    ║    messaging_system          ║
                    ║  (High-level Integration)    ║
                    ╚══════════════════════════════╝
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
        ▼                     ▼                     ▼
┌────────────────┐    ┌────────────────┐    ┌────────────────┐
│ network_system │    │container_system│    │database_system │
│ TCP/IP layer   │    │ Data types     │    │ PostgreSQL     │
└────────────────┘    └────────────────┘    └────────────────┘
        │                     │                     │
        └─────────────────────┼─────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
        ▼                     ▼                     ▼
┌────────────────┐    ┌────────────────┐    ┌────────────────┐
│ thread_system  │    │ logger_system  │    │monitoring_system│
│ Lock-free pools│    │ Async logging  │    │   Metrics      │
└────────────────┘    └────────────────┘    └────────────────┘
        │                     │                     │
        └─────────────────────┼─────────────────────┘
                              │
                              ▼
                    ┌────────────────┐
                    │ common_system  │
                    │  Foundation    │
                    └────────────────┘
```

### 🧩 How Each System Contributes

#### 1. **common_system** - The Foundation
```cpp
// Provides core abstractions used throughout
namespace common {
    template<typename T> class Result;  // Rust-style error handling
    class VoidResult;                   // For operations without return value

    namespace interfaces {
        class IExecutor;                // Async execution abstraction
    }

    namespace patterns {
        // Design patterns: Factory, Observer, Strategy, etc.
    }
}
```

**What it provides:**
- `Result<T>` pattern for error handling (no exceptions in hot paths)
- Executor interface for pluggable concurrency
- Common utilities (string, time, filesystem)
- Cross-platform abstractions

**Used for:** Foundation layer that all other systems depend on

#### 2. **thread_system** - Concurrency Engine
```cpp
// High-performance lock-free thread pools
namespace thread {
    class thread_pool : public common::interfaces::IExecutor {
        // 2.48M jobs/sec throughput
        // 250ns enqueue latency
        // Hazard pointer memory reclamation
    };

    class lockfree_thread_pool;  // Lock-free variant
    class priority_thread_pool;   // Priority scheduling
}
```

**What it provides:**
- Lock-free multi-producer/multi-consumer queues
- Work-stealing thread pools
- Priority-based job scheduling (RealTime, Batch, Background)
- Hazard pointer memory management

**Used for:** Message dispatch, async operations, background processing

#### 3. **logger_system** - Observability
```cpp
// Structured async logging with zero allocation fast paths
namespace logger {
    class logger {
        void log(level, format, args...);
        void set_async(bool);  // 15M+ logs/sec in async mode
    };
}
```

**What it provides:**
- Async non-blocking logging
- Structured log output (JSON, text)
- Log rotation and archival
- Multiple sinks (file, console, network)

**Used for:** Debugging, audit trails, performance analysis

#### 4. **monitoring_system** - Telemetry
```cpp
// Real-time metrics collection and export
namespace monitoring {
    class metric_collector {
        void record_counter(name, value);
        void record_histogram(name, value);
        std::string export_prometheus();
    };
}
```

**What it provides:**
- Counters, gauges, histograms
- Prometheus format export
- Health check aggregation
- Performance profiling

**Used for:** Production observability, alerting, dashboards

#### 5. **container_system** - Type-Safe Data
```cpp
// Type-safe containers with serialization
namespace container {
    class value_container {
        void set_value(key, variant_value);
        Result<variant_value> get_value(key);
        std::string serialize();  // Binary or JSON
    };
}
```

**What it provides:**
- Type-safe variant storage (bool, int*, string, bytes, nested)
- SIMD-optimized operations (ARM NEON, x86 AVX)
- Efficient binary serialization
- Thread-safe operations

**Used for:** Message payloads, configuration data, RPC parameters

#### 6. **database_system** - Persistence
```cpp
// PostgreSQL integration with connection pooling
namespace database {
    class database_manager {
        Result<void> connect(connection_string);
        Result<query_result> select_query(sql);
        Result<void> insert_query(sql);
        // Connection pooling, prepared statements, transactions
    };
}
```

**What it provides:**
- Connection pooling with configurable sizes
- Prepared statement caching
- Transaction support (ACID)
- Automatic retry and recovery

**Used for:** Message persistence, audit logs, state storage

#### 7. **network_system** - Communication Layer
```cpp
// Asynchronous TCP client/server with ASIO
namespace network {
    class messaging_server {
        Result<void> start_server(port);
        void on_message_received(callback);
    };

    class messaging_client {
        Result<void> start_client(host, port);
        Result<void> send_message(msg);
    };
}
```

**What it provides:**
- Async I/O with ASIO/coroutines
- Full-duplex messaging
- Session management
- TLS/SSL support (optional)

**Used for:** Distributed messaging, inter-service communication

---

## 🏗️ System Architecture

### High-Level Design

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                       │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│   │  C++ API     │  │Python Binding│  │  REST API    │     │
│   └──────────────┘  └──────────────┘  └──────────────┘     │
├─────────────────────────────────────────────────────────────┤
│                   Messaging System Core                     │
│   ┌──────────────────────────────────────────────────────┐  │
│   │  MessageBus: Central pub/sub coordinator            │  │
│   │  - publish_async() / publish_sync()                 │  │
│   │  - subscribe() / unsubscribe()                      │  │
│   │  - Wildcard topic routing (*, #)                    │  │
│   └──────────────────────────────────────────────────────┘  │
│   ┌──────────────────────────────────────────────────────┐  │
│   │  TopicRouter: Pattern matching and dispatch         │  │
│   │  - Trie-based topic matching                        │  │
│   │  - Wildcard support (* single-level, # multi-level) │  │
│   │  - Subscriber priority handling                     │  │
│   └──────────────────────────────────────────────────────┘  │
│   ┌──────────────────────────────────────────────────────┐  │
│   │  MessagingContainer: Type-safe message envelope     │  │
│   │  - Source/target addressing                         │  │
│   │  - Topic-based routing                              │  │
│   │  - Payload serialization (container_system)         │  │
│   │  - TraceContext integration                         │  │
│   └──────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                    Integration Layer                        │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│   │NetworkBridge │  │PersistentQueue│  │ TraceContext │     │
│   │(network_sys) │  │(database_sys) │  │ (thread-safe)│     │
│   └──────────────┘  └──────────────┘  └──────────────┘     │
├─────────────────────────────────────────────────────────────┤
│                     Base Systems Layer                      │
│   ┌───────────┐ ┌───────────┐ ┌────────────┐               │
│   │ Network   │ │ Container │ │  Database  │               │
│   │  System   │ │  System   │ │   System   │               │
│   └───────────┘ └───────────┘ └────────────┘               │
├─────────────────────────────────────────────────────────────┤
│                  Runtime Services Layer                     │
│   ┌───────────┐ ┌───────────┐ ┌────────────┐               │
│   │  Thread   │ │  Logger   │ │ Monitoring │               │
│   │  System   │ │  System   │ │   System   │               │
│   └───────────┘ └───────────┘ └────────────┘               │
├─────────────────────────────────────────────────────────────┤
│                    Foundation Layer                         │
│   ┌─────────────────────────────────────────────────┐       │
│   │  Common System: Result<T>, IExecutor, Utils     │       │
│   └─────────────────────────────────────────────────┘       │
└─────────────────────────────────────────────────────────────┘
```

### Message Flow

```
1. Publisher                    2. MessageBus               3. TopicRouter
   ┌─────────┐                     ┌─────────┐                 ┌─────────┐
   │ App     │ publish_async()      │ Enqueue │ work_executor   │ Match   │
   │ Code    │ ──────────────────►  │ to I/O  │ ─────────────► │ Topic   │
   └─────────┘                      │ Executor│                │ Pattern │
                                    └─────────┘                └─────────┘
                                                                     │
                                                                     ▼
4. Subscriber Dispatch                               5. Subscriber Execution
   ┌─────────────────┐                                  ┌─────────┐
   │ For each match: │ work_executor                    │Execute  │
   │ - Create trace  │ ──────────────────────────────►  │callback │
   │ - Dispatch async│                                  │in thread│
   └─────────────────┘                                  └─────────┘
```

---

## 🚀 Quick Start

### Prerequisites

This project requires several sibling system projects or uses FetchContent to download them automatically.

**Required Systems:**
- `common_system` - Foundation patterns and Result<T>
- `thread_system` - Executor interface (or use MockExecutor)
- `container_system` - Type-safe data containers

**Optional Systems:**
- `logger_system` - Async logging
- `monitoring_system` - Metrics and telemetry
- `database_system` - Persistence features
- `network_system` - TCP/IP messaging

### Step 1: Choose Your Build Strategy

#### Option A: Local Development (Recommended for Contributors)

Requires all systems in sibling directories:

```bash
# Expected directory structure:
# Sources/
#   ├── common_system/      # Required
#   ├── thread_system/      # Required
#   ├── container_system/   # Required
#   ├── logger_system/      # Optional
#   ├── monitoring_system/  # Optional
#   ├── database_system/    # Optional
#   ├── network_system/     # Optional
#   └── messaging_system/   # This project

cd messaging_system
cmake -B build -DMESSAGING_USE_LOCAL_SYSTEMS=ON -DMESSAGING_BUILD_TESTS=ON
cmake --build build -j
```

#### Option B: Automatic FetchContent (Easiest for New Users)

CMake will automatically download all dependencies:

```bash
git clone https://github.com/kcenon/messaging_system.git
cd messaging_system
cmake -B build -DMESSAGING_USE_FETCHCONTENT=ON -DMESSAGING_BUILD_EXAMPLES=ON
cmake --build build -j
```

#### Option C: System-Installed Packages (Production)

If you have installed the system packages:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build
```

### Step 2: Your First Program (60 seconds)

```cpp
#include "messaging_system/core/message_bus.h"
#include "messaging_system/core/messaging_container.h"

#ifdef HAS_THREAD_SYSTEM
#include <kcenon/thread/core/thread_pool.h>
#endif

using namespace messaging;

int main() {
    // 1. Create thread pools (from thread_system)
    auto io_pool = std::make_shared<thread::thread_pool>(2);
    auto work_pool = std::make_shared<thread::thread_pool>(4);

    // 2. Create router and message bus
    auto router = std::make_shared<TopicRouter>(work_pool);
    auto bus = std::make_shared<MessageBus>(io_pool, work_pool, router);

    // 3. Start the bus
    bus->start();

    // 4. Subscribe to a topic
    bus->subscribe("user.created", [](const MessagingContainer& msg) {
        // logger_system integration (if HAS_LOGGER_SYSTEM)
        std::cout << "New user event: " << msg.topic() << std::endl;
        return common::VoidResult::ok();
    });

    // 5. Publish a message (uses container_system for payload)
    auto msg = MessagingContainer::create(
        "app",          // source
        "subscribers",  // target
        "user.created"  // topic
    );

    if (msg.is_ok()) {
        bus->publish_async(msg.value());  // Async via thread_system
    }

    // 6. Cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bus->stop();

    return 0;
}
```

### Step 3: Run Your Application

```bash
# Build your application
cmake -B build -DMESSAGING_BUILD_EXAMPLES=ON
cmake --build build -j

# Run the example
./build/examples/basic_messaging
```

**Expected Output:**
```
New user event: user.created
```

> 💡 **Next Steps**: Explore more [Real-World Examples](#-real-world-examples) below or dive into the [Complete Documentation](docs/)

---

## 🔥 Real-World Examples

### Example 1: Distributed Task Queue with Persistence

Shows integration of **network_system**, **database_system**, **thread_system**, and **logger_system**

```cpp
#include "messaging_system/core/message_bus.h"
#include "messaging_system/integration/persistent_queue.h"
#include "messaging_system/integration/network_bridge.h"

class DistributedTaskQueue {
    std::shared_ptr<MessageBus> bus_;
    std::shared_ptr<PersistentQueue> queue_;  // database_system
    std::shared_ptr<NetworkBridge> bridge_;   // network_system

public:
    DistributedTaskQueue() {
        // Thread pools for I/O and work
        auto io_pool = std::make_shared<thread::thread_pool>(4);
        auto work_pool = std::make_shared<thread::thread_pool>(8);

        // Create message bus
        auto router = std::make_shared<TopicRouter>(work_pool);
        bus_ = std::make_shared<MessageBus>(io_pool, work_pool, router);

        // Persistent queue (stores tasks in PostgreSQL)
        queue_ = std::make_shared<PersistentQueue>(
            "postgres://localhost/taskdb"
        );

        // Network bridge (distributes tasks across nodes)
        bridge_ = std::make_shared<NetworkBridge>(bus_, 8080);

        bus_->start();
    }

    // Submit task - persisted before dispatch
    common::Result<uint64_t> submit_task(
        const std::string& task_type,
        const container::value_container& payload
    ) {
        // Create message with task data
        auto msg = MessagingContainer::create("client", "workers", task_type);
        if (!msg.is_ok()) return common::make_error<uint64_t>("Failed to create message");

        msg.value().set_payload(payload);  // container_system

        // Persist to database first (database_system)
        auto persist_result = queue_->enqueue(msg.value());
        if (!persist_result.is_ok()) return persist_result;

        // Then publish to workers (thread_system dispatch)
        bus_->publish_async(msg.value());

        return persist_result;
    }

    // Subscribe to task type
    void register_worker(
        const std::string& task_pattern,
        std::function<common::VoidResult(const MessagingContainer&)> handler
    ) {
        bus_->subscribe(task_pattern, [this, handler](const MessagingContainer& msg) {
            // Execute handler in thread pool
            auto result = handler(msg);

            if (result.is_ok()) {
                // Remove from persistent queue on success
                queue_->acknowledge(msg.message_id());
            } else {
                // Log failure (logger_system)
                // Task remains in queue for retry
            }

            return result;
        });
    }
};

// Usage
int main() {
    DistributedTaskQueue queue;

    // Register worker for image processing tasks
    queue.register_worker("task.image.*", [](const MessagingContainer& msg) {
        auto payload = msg.payload();
        std::string image_url = payload.get_value("url").value();

        // Process image...
        process_image(image_url);

        return common::VoidResult::ok();
    });

    // Submit task
    container::value_container task_data;
    task_data.set_value("url", "https://example.com/image.jpg");
    task_data.set_value("resize", true);

    queue.submit_task("task.image.resize", task_data);

    return 0;
}
```

### Example 2: Real-Time Chat Server with Monitoring

Shows integration of **network_system**, **logger_system**, and **monitoring_system**

```cpp
#include "messaging_system/core/message_bus.h"
#include "messaging_system/integration/network_bridge.h"

#ifdef HAS_MONITORING_SYSTEM
#include <kcenon/monitoring/metric_collector.h>
#endif

#ifdef HAS_LOGGER_SYSTEM
#include <kcenon/logger/logger.h>
#endif

class ChatServer {
    std::shared_ptr<MessageBus> bus_;
    std::shared_ptr<NetworkBridge> bridge_;

#ifdef HAS_MONITORING_SYSTEM
    std::shared_ptr<monitoring::metric_collector> metrics_;
#endif

public:
    ChatServer(uint16_t port) {
        auto io_pool = std::make_shared<thread::thread_pool>(4);
        auto work_pool = std::make_shared<thread::thread_pool>(8);

        auto router = std::make_shared<TopicRouter>(work_pool);
        bus_ = std::make_shared<MessageBus>(io_pool, work_pool, router);

        // Network bridge for TCP connections (network_system)
        bridge_ = std::make_shared<NetworkBridge>(bus_, port);

#ifdef HAS_MONITORING_SYSTEM
        metrics_ = std::make_shared<monitoring::metric_collector>();
#endif

        setup_routes();
        bus_->start();
        bridge_->start();
    }

    void setup_routes() {
        // Route 1: User joins room
        bus_->subscribe("chat.room.*.join", [this](const MessagingContainer& msg) {
#ifdef HAS_LOGGER_SYSTEM
            logger::instance().info("User joined: {}", msg.source());
#endif

#ifdef HAS_MONITORING_SYSTEM
            metrics_->record_counter("chat.joins.total", 1);
#endif

            // Broadcast join notification to room
            auto room = extract_room_from_topic(msg.topic());
            broadcast_to_room(room, "user_joined", msg.source());

            return common::VoidResult::ok();
        });

        // Route 2: Chat messages
        bus_->subscribe("chat.room.*.message", [this](const MessagingContainer& msg) {
            auto room = extract_room_from_topic(msg.topic());
            auto text = msg.payload().get_value("text").value();

#ifdef HAS_LOGGER_SYSTEM
            logger::instance().debug("Message in {}: {}", room, text);
#endif

#ifdef HAS_MONITORING_SYSTEM
            metrics_->record_counter("chat.messages.total", 1);
            metrics_->record_histogram("chat.message.size", text.size());
#endif

            // Broadcast to all room members (except sender)
            broadcast_to_room(room, "message", msg.source(), text);

            return common::VoidResult::ok();
        });
    }

    void broadcast_to_room(
        const std::string& room,
        const std::string& event_type,
        const std::string& from_user,
        const std::string& text = ""
    ) {
        auto msg = MessagingContainer::create(
            from_user,
            "room." + room,
            "chat.room." + room + ".broadcast"
        );

        if (msg.is_ok()) {
            msg.value().payload().set_value("event", event_type);
            msg.value().payload().set_value("text", text);

            // network_bridge automatically routes to connected clients
            bus_->publish_async(msg.value());
        }
    }

    std::string extract_room_from_topic(const std::string& topic) {
        // Parse "chat.room.{room_name}.{action}"
        auto parts = split_string(topic, '.');
        return parts.size() > 2 ? parts[2] : "default";
    }
};

int main() {
    ChatServer server(8080);

    std::cout << "Chat server running on port 8080" << std::endl;
    std::cout << "Metrics available at /metrics" << std::endl;

    // Keep server running
    std::this_thread::sleep_for(std::chrono::hours(24));

    return 0;
}
```

### Example 3: Event-Driven Microservice with Tracing

Shows **TraceContext** integration for distributed tracing

```cpp
#include "messaging_system/core/message_bus.h"
#include "messaging_system/integration/trace_context.h"

class OrderService {
    std::shared_ptr<MessageBus> bus_;

public:
    OrderService(std::shared_ptr<MessageBus> bus) : bus_(bus) {
        // Subscribe to order events
        bus_->subscribe("order.created", [this](const MessagingContainer& msg) {
            // Trace context automatically propagated
            ScopedTrace trace(msg.trace_id());

            // All operations within this scope share the same trace ID
            auto order_id = msg.payload().get_value("order_id").value();

            // Publish inventory check event (trace ID propagated)
            auto inv_check = MessagingContainer::create(
                "order_service", "inventory_service", "inventory.check"
            );
            inv_check.value().set_trace_id(TraceContext::get_trace_id());
            inv_check.value().payload().set_value("order_id", order_id);

            bus_->publish_async(inv_check.value());

            return common::VoidResult::ok();
        });

        bus_->subscribe("inventory.checked", [this](const MessagingContainer& msg) {
            ScopedTrace trace(msg.trace_id());  // Continue same trace

            auto available = msg.payload().get_value("available").value<bool>();
            auto order_id = msg.payload().get_value("order_id").value();

            if (available) {
                // Publish payment event
                auto payment = MessagingContainer::create(
                    "order_service", "payment_service", "payment.process"
                );
                payment.value().set_trace_id(TraceContext::get_trace_id());
                payment.value().payload().set_value("order_id", order_id);

                bus_->publish_async(payment.value());
            }

            return common::VoidResult::ok();
        });
    }
};

// Distributed tracing allows you to track a single order
// across multiple services: Order → Inventory → Payment → Shipping
```

[📖 View More Examples](examples/) | [Application Layer Samples](application_layer/samples/)

**Note**: Core messaging_system provides 1 basic example. Application layer provides 8 additional samples including chat server, distributed worker, IoT monitoring, and microservices orchestrator.

---

## 📊 Performance Characteristics

### Expected Performance (Estimates)

Performance depends heavily on the underlying executor implementation and optional system integrations.

**Core Messaging (Local Pub/Sub)**
- **Estimated throughput**: 100K-500K messages/second (depends on executor)
- **Typical latency**: 1-10 μs per message (in-memory routing)
- **Concurrency**: Thread-safe with `std::shared_mutex`

**With External Systems**
- **With serialization** (container_system): 50K-200K msg/s
- **With persistence** (database_system): 1K-10K msg/s (DB-limited)
- **With network** (network_system): 5K-50K msg/s (network-limited)

**Factors Affecting Performance:**
- Executor implementation (MockExecutor vs thread_system)
- Message payload size
- Number of concurrent subscribers
- Wildcard pattern complexity
- External system latencies

### Benchmarking

Formal benchmark suite is planned for future releases. Current test suite includes:
- Concurrent publishing test (4 threads × 10 messages)
- Multiple subscribers test (broadcast to 3 subscribers)
- Wildcard routing test

To add your own benchmarks:
```cpp
#include "messaging_system/core/message_bus.h"
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
// Your messaging code here
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
```

**Contribution Welcome**: We welcome benchmark contributions! See [Contributing](#-contributing) for guidelines.

---

## 🛠️ Building from Source

### Prerequisites

- **Compiler**: C++20 compatible
  - GCC 10+ / Clang 12+ / MSVC 2019+
- **CMake**: 3.16 or later
- **Git**: For submodules
- **Optional**: PostgreSQL 12+ (for database features)

### Build Modes

#### Local Development (Recommended)

Uses sibling directories for all base systems:

```bash
# Directory structure:
# Sources/
#   ├── common_system/
#   ├── thread_system/
#   ├── logger_system/
#   ├── monitoring_system/
#   ├── container_system/
#   ├── database_system/
#   ├── network_system/
#   └── messaging_system/  ← we are here

cmake -B build \
  -DMESSAGING_USE_LOCAL_SYSTEMS=ON \
  -DMESSAGING_BUILD_TESTS=ON \
  -DMESSAGING_BUILD_EXAMPLES=ON

cmake --build build -j
```

#### FetchContent Mode

Automatically fetches all dependencies from GitHub:

```bash
cmake -B build \
  -DMESSAGING_USE_FETCHCONTENT=ON \
  -DMESSAGING_BUILD_EXAMPLES=ON

cmake --build build -j
```

#### Production Mode

Uses system-installed packages:

```bash
# Install dependencies first
sudo apt install \
  libcommon-system-dev \
  libthread-system-dev \
  liblogger-system-dev \
  # ... etc

cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DMESSAGING_BUILD_TESTS=OFF

cmake --build build -j
sudo cmake --install build
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `MESSAGING_USE_LOCAL_SYSTEMS` | OFF | Use sibling directories |
| `MESSAGING_USE_FETCHCONTENT` | OFF | Auto-fetch from GitHub |
| `MESSAGING_BUILD_TESTS` | ON | Build unit tests |
| `MESSAGING_BUILD_EXAMPLES` | ON | Build example programs |
| `MESSAGING_BUILD_BENCHMARKS` | OFF | Build performance benchmarks |
| `MESSAGING_USE_LOCKFREE` | OFF | Enable lock-free mode |
| `MESSAGING_ENABLE_TLS` | OFF | Enable SSL/TLS support |

---

## 🧪 Testing

### Test Framework

**Note**: Tests use assert-based validation, not GTest. Each test is a standalone executable.

### Run Tests

```bash
# Build with tests
cmake -B build -DMESSAGING_BUILD_TESTS=ON -DMESSAGING_USE_FETCHCONTENT=ON
cmake --build build -j

# Run all tests via CTest
cd build
ctest --output-on-failure

# Or run individual tests directly
./test_messaging_container
./test_topic_router
./test_message_bus
./test_trace_context
./test_config_loader
./test_end_to_end
```

### Test Suite Overview

**Unit Tests** (test/unit/core/):
- `test_messaging_container` - Message creation, serialization
- `test_topic_router` - Wildcard matching, subscription management
- `test_message_bus` - Pub/sub, async/sync publishing, 8 test cases

**Integration Tests** (test/integration/):
- `test_trace_context` - Distributed tracing propagation
- `test_config_loader` - YAML configuration loading
- `test_end_to_end` - Full system integration

**Test Example Output**:
```
=== MessageBus Unit Tests ===

Test: MessageBus start/stop...
  ✓ Passed
Test: Synchronous publish/subscribe...
  ✓ Passed
Test: Asynchronous publish/subscribe...
  ✓ Passed
Test: Multiple subscribers on same topic...
  ✓ Passed
Test: Wildcard subscriptions via MessageBus...
  ✓ Passed
Test: Unsubscribe via MessageBus...
  ✓ Passed
Test: Concurrent publishing from multiple threads...
  ✓ Passed
Test: Error handling in subscriber callback...
  ✓ Passed

All tests passed!
```

### Test Coverage

Current coverage (Phase 0):
- Core components: Comprehensive unit tests
- Integration scenarios: Basic end-to-end tests
- Concurrency: Multi-threaded publish/subscribe tests
- Error handling: Callback failure scenarios

Coverage workflow runs on CI:
- lcov-based coverage analysis
- Excludes external dependencies
- Reports uploaded to Codecov

---

## 📦 CMake Integration

### As Subdirectory

```cmake
# Add as subdirectory (local development)
set(MESSAGING_USE_LOCAL_SYSTEMS ON CACHE BOOL "" FORCE)
add_subdirectory(messaging_system)

target_link_libraries(your_app PRIVATE messaging_system_core)
```

### As FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    messaging_system
    GIT_REPOSITORY https://github.com/kcenon/messaging_system.git
    GIT_TAG main
)

set(MESSAGING_USE_FETCHCONTENT ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(messaging_system)

target_link_libraries(your_app PRIVATE messaging_system_core)
```

### As Installed Package

```cmake
find_package(MessagingSystem 2.0 REQUIRED)

target_link_libraries(your_app PRIVATE MessagingSystem::messaging_system_core)
```

---

## 🔧 Configuration

### Runtime Configuration with YAML

```yaml
# config.yaml
messaging:
  network:
    port: 8080
    max_connections: 1000

  thread_pools:
    io_workers: 4
    work_workers: 8

  database:
    connection_string: "postgres://localhost/msgdb"
    pool_size: 10

  monitoring:
    enabled: true
    export_format: "prometheus"
    metrics_port: 9090
```

```cpp
#ifdef HAS_YAML_CPP
auto config = MessagingSystemConfig::load_from_file("config.yaml");
if (config.is_ok()) {
    auto& cfg = config.value();

    // Apply configuration
    auto io_pool = std::make_shared<thread::thread_pool>(cfg.thread_pools.io_workers);
    auto work_pool = std::make_shared<thread::thread_pool>(cfg.thread_pools.work_workers);

    auto router = std::make_shared<TopicRouter>(work_pool);
    auto bus = std::make_shared<MessageBus>(io_pool, work_pool, router);

    bus->start();
}
#endif
```

---

## 📚 Documentation

### Available Documentation

- [Getting Started Guide](docs/GETTING_STARTED.MD) - Initial setup and basic usage
- [Developer Guide](docs/DEVELOPER_GUIDE.md) - Detailed development instructions
- [System Architecture](docs/architecture.md) - How the systems integrate
- [API Reference](docs/API_REFERENCE.md) - Complete API documentation
- [Performance Guide](docs/performance.md) - Optimization and tuning
- [Build Troubleshooting](docs/BUILD_TROUBLESHOOTING.md) - Common build issues

### Phase Documentation

Detailed design documents for each development phase:
- [Phase 0](docs/phase0/) - Legacy removal and interface mapping
- [Phase 1-4](docs/phase1/) - Feature implementation phases
- [Project Summary](docs/PROJECT_COMPLETION_SUMMARY.md) - Overall project status

### External System Documentation

Each base system has its own repository with detailed documentation:
- [common_system](https://github.com/kcenon/common_system) - Foundation patterns and Result<T>
- [thread_system](https://github.com/kcenon/thread_system) - Thread pools and executors
- [logger_system](https://github.com/kcenon/logger_system) - Async logging system
- [monitoring_system](https://github.com/kcenon/monitoring_system) - Metrics and telemetry
- [container_system](https://github.com/kcenon/container_system) - Type-safe containers
- [database_system](https://github.com/kcenon/database_system) - Database integration
- [network_system](https://github.com/kcenon/network_system) - Network messaging

### Application Layer

For advanced usage scenarios, see:
- [Application Layer README](application_layer/README.md) - High-level messaging services
- [Application Layer Samples](application_layer/samples/) - 8 production-ready examples
- [Python Bindings](application_layer/python_bindings/) - Python wrapper (requires application_layer build)

---

## 🤝 Contributing

We welcome contributions!

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes with clear commit messages
4. Ensure all tests pass (`ctest` in build directory)
5. Submit a pull request

### Contribution Areas

**High Priority:**
- Implement `MessagingNetworkBridge` (interface exists, needs implementation)
- Implement `PersistentMessageQueue` (interface exists, needs implementation)
- Add benchmark suite for performance validation
- Improve test coverage

**Documentation:**
- Add more usage examples
- Improve API documentation
- Add troubleshooting guides

**Code Quality:**
- Add GTest support (or remove GTest references)
- Improve error messages
- Add more integration tests

For detailed guidelines, open an issue or discussion on GitHub.

### Development Workflow

```bash
# 1. Clone with all submodules
git clone --recursive https://github.com/kcenon/messaging_system.git
cd messaging_system

# 2. Create feature branch
git checkout -b feature/amazing-feature

# 3. Build and test
cmake -B build -DMESSAGING_USE_LOCAL_SYSTEMS=ON -DMESSAGING_BUILD_TESTS=ON
cmake --build build -j
cd build && ctest

# 4. Commit and push
git commit -m "feat: add amazing feature"
git push origin feature/amazing-feature

# 5. Open Pull Request
```

### Code Style

- Follow C++20 best practices
- Use RAII for resource management
- Prefer `Result<T>` over exceptions
- Thread safety in all public APIs
- Comprehensive unit tests

---

## 📊 Project Statistics

<table>
<tr>
<td>

**Core Implementation**
- 📝 ~820 lines of core code
- 🧪 6 test executables
- 📚 Comprehensive documentation
- 🔍 Static analysis ready

</td>
<td>

**Test Coverage**
- ✅ 8 unit test cases (MessageBus)
- 🧪 5 component tests
- 🔄 1 end-to-end integration test
- 🔀 Concurrency test scenarios

</td>
<td>

**Quality Assurance**
- ✅ Multi-platform support
- 🛡️ Sanitizer-ready code
- 📊 CI/CD workflows
- 🔐 Secure coding practices

</td>
</tr>
</table>

**Code Distribution:**
- Core implementation: 818 LOC (message_bus, topic_router, messaging_container, trace_context, config_loader)
- Support code: 188 LOC (MockExecutor)
- Tests: ~1,500 LOC
- Examples: ~170 LOC (core) + ~2,000 LOC (application_layer)
- Total: ~2,700 LOC core system

---

## 🌐 Platform Support

| Platform | Compiler | Status |
|----------|----------|--------|
| Ubuntu 22.04 | GCC 11+ | ✅ Tested |
| Ubuntu 22.04 | Clang 14+ | ✅ Tested |
| macOS 13+ | Apple Clang | ✅ Tested |
| Windows 10+ | MSVC 2019+ | ✅ Tested |
| Windows 10+ | MSYS2/MinGW | ✅ Tested |

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

**Built on the shoulders of 7 specialized systems**

[common_system](https://github.com/kcenon/common_system) •
[thread_system](https://github.com/kcenon/thread_system) •
[logger_system](https://github.com/kcenon/logger_system) •
[monitoring_system](https://github.com/kcenon/monitoring_system) •
[container_system](https://github.com/kcenon/container_system) •
[database_system](https://github.com/kcenon/database_system) •
[network_system](https://github.com/kcenon/network_system)

**If this project helps you, please consider giving it a ⭐!**

Made with ❤️ by the Open Source Community

</div>

---

## 📋 Appendix

### A. Advanced Build Guide

#### Available Build Presets

The system provides 10 CMake presets for different use cases:

| Preset | Description | Use Case |
|--------|-------------|----------|
| `default` | Production with find_package | Installed external systems |
| `dev-fetchcontent` | Development with FetchContent | Development without installs |
| `debug` | Debug build | Debugging |
| `release` | Optimized release | Production |
| `asan` | AddressSanitizer | Memory error detection |
| `tsan` | ThreadSanitizer | Race condition detection |
| `ubsan` | UndefinedBehaviorSanitizer | Undefined behavior detection |
| `ci` | CI/CD with pedantic warnings | Continuous integration |
| `lockfree` | Lock-free data structures | High performance |
| `minimal` | Minimal feature set | Embedded/constrained |

Usage:
```bash
./build.sh [preset] [options]

# Examples:
./build.sh dev-fetchcontent --tests
./build.sh release --examples
./build.sh asan --clean
```

#### Build Script Options

```bash
./build.sh [preset] [options]

Options:
  --clean           Remove build directory before building
  --tests           Build and run tests
  --examples        Build examples
  --benchmarks      Build benchmarks
  --verbose         Show detailed build output
  --cores N         Use N cores (default: auto-detect)

Feature Options:
  --lockfree        Enable lock-free implementations
  --no-monitoring   Disable monitoring system
  --no-logging      Disable logging system
  --enable-tls      Enable TLS/SSL support
```

#### Troubleshooting Build Issues

**Target Name Conflicts**

Symptom:
```
CMake Error: add_library cannot create target "interfaces" because another
target with the same name already exists.
```

Solutions:
1. Use automated build script: `./scripts/build_with_fetchcontent.sh`
2. Manually move local systems temporarily
3. Install systems and use `default` preset

**Platform-Specific Notes**

macOS:
- Requires Xcode Command Line Tools
- Homebrew recommended for dependencies
- Use `grep -E` (BSD grep doesn't support `-P`)

Linux (Ubuntu/Debian):
- Install build-essential: `sudo apt-get install build-essential cmake`
- GCC 11+ or Clang 14+ for C++20

### B. Code Quality and Linting

#### Linting Tools

We use specialized tools for different file types:

| Tool | Purpose | Config File | File Types |
|------|---------|-------------|------------|
| Cppcheck | C++ analysis | `.cppcheck` | `*.cpp`, `*.h` |
| clang-tidy | C++ linting | `.clang-tidy` | `*.cpp`, `*.h` |
| markdownlint | Doc style | `.markdownlint.json` | `*.md` |
| shellcheck | Script validation | None | `*.sh` |

#### Running Cppcheck

```bash
# Basic usage
cppcheck src/ include/ \
    --enable=warning,style,performance,portability \
    --std=c++20 \
    --suppress=missingIncludeSystem

# With compile commands
cppcheck --project=build/compile_commands.json --enable=all
```

#### Markdown Linting

```bash
# Install markdownlint
npm install -g markdownlint-cli

# Check all markdown files
markdownlint '**/*.md'

# Fix automatically when possible
markdownlint '**/*.md' --fix
```

#### Shell Script Validation

```bash
# Install shellcheck
brew install shellcheck  # macOS
apt-get install shellcheck  # Ubuntu

# Check all shell scripts
shellcheck build.sh scripts/*.sh
```

#### Pre-commit Hooks (Optional)

Create `.git/hooks/pre-commit`:

```bash
#!/bin/bash

echo "Running code quality checks..."

# C++ files
if git diff --cached --name-only | grep -E '\.(cpp|h)$' > /dev/null; then
    echo "Checking C++ files with cppcheck..."
    git diff --cached --name-only | grep -E '\.(cpp|h)$' | xargs cppcheck \
        --enable=warning,style,performance \
        --std=c++20 \
        --suppress=missingIncludeSystem \
        --error-exitcode=1 || exit 1
fi

echo "All checks passed!"
```

Make it executable:
```bash
chmod +x .git/hooks/pre-commit
```

### C. Testing and Benchmarks

#### Test Coverage

Current test suite includes:
- 39+ test cases across 6 test files
- Unit tests for all core components (MessageBus, TopicRouter, MessagingContainer)
- Integration tests for end-to-end scenarios
- Performance benchmarks

#### Running Specific Tests

```bash
# After building with --tests
./build/test/unittest/test_topic_router
./build/test/unittest/test_message_bus
./build/test/unittest/test_end_to_end

# With filtering
./build/test/unittest/messaging_test --gtest_filter="MessageBusTest.*"
```

#### Performance Benchmarks

Build and run benchmarks:

```bash
./build.sh --benchmarks
./build/benchmarks/message_bus_benchmark
```

### D. Development Guidelines

#### Code Style

- Follow C++20 best practices
- Use RAII for resource management
- Prefer `Result<T>` over exceptions in API boundaries
- Thread safety in all public APIs
- Comprehensive unit tests for new features

#### Commit Message Format

We follow conventional commits:

```
<type>(<scope>): <subject>

<body>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `refactor`: Code refactoring
- `test`: Adding tests
- `perf`: Performance improvement
- `ci`: CI/CD changes

Example:
```
feat(message-bus): add priority-based message routing

- Implement priority queue for critical messages
- Add configuration option for priority levels
- Update tests to cover priority scenarios
```

#### VS Code Integration

Recommended extensions (`.vscode/extensions.json`):

```json
{
  "recommendations": [
    "davidanson.vscode-markdownlint",
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools",
    "timonwong.shellcheck"
  ]
}
```

Settings (`.vscode/settings.json`):

```json
{
  "markdownlint.config": {
    "extends": ".markdownlint.json"
  },
  "C_Cpp.codeAnalysis.clangTidy.enabled": true,
  "shellcheck.enable": true,
  "cmake.configureOnOpen": true
}
```

---

**Last Updated**: 2025-10-22
