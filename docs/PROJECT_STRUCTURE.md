# Messaging System Project Structure

**Version**: 0.1.1
**Last Updated**: 2025-12-10
**Language**: [English] | [한국어](PROJECT_STRUCTURE_KO.md)

---

## Overview

This document describes the complete directory structure and organization of the messaging_system project.

---

## Directory Layout

```
messaging_system/
├── include/kcenon/messaging/          # Public API headers
│   ├── core/                          # Core messaging components
│   │   ├── message.h                  # Message structure and builder
│   │   ├── message_bus.h              # Central message hub
│   │   ├── message_broker.h           # Message brokering logic
│   │   ├── message_queue.h            # Thread-safe queue
│   │   └── topic_router.h             # Topic-based routing
│   │
│   ├── interfaces/                    # Abstract interfaces
│   │   ├── message_handler_interface.h
│   │   ├── message_router_interface.h
│   │   ├── publisher_interface.h
│   │   ├── subscriber_interface.h
│   │   └── queue_interface.h
│   │
│   ├── backends/                      # Execution backends
│   │   ├── backend_interface.h
│   │   ├── standalone_backend.h       # Self-contained execution
│   │   └── integration_backend.h      # External integration
│   │
│   ├── patterns/                      # Messaging patterns
│   │   ├── pub_sub.h                  # Publish-Subscribe
│   │   ├── request_reply.h            # Request-Reply
│   │   ├── event_streaming.h          # Event sourcing
│   │   └── message_pipeline.h         # Message processing pipeline
│   │
│   ├── task/                          # Distributed Task Queue System
│   │   ├── task.h                     # Task definition and builder
│   │   ├── task_handler.h             # Handler interface with C++20 Concepts
│   │   ├── task_context.h             # Execution context
│   │   ├── task_queue.h               # Priority task queue
│   │   ├── worker_pool.h              # Worker thread pool
│   │   ├── result_backend.h           # Result storage interface
│   │   ├── memory_result_backend.h    # In-memory result backend
│   │   ├── async_result.h             # Async result handle
│   │   ├── task_client.h              # Task submission client
│   │   ├── scheduler.h                # Periodic/cron scheduler
│   │   ├── cron_parser.h              # Cron expression parser
│   │   ├── monitor.h                  # Task monitoring
│   │   └── task_system.h              # Unified facade
│   │
│   └── utils/                         # Utilities
│       └── integration_detector.h     # System integration detection
│
├── src/impl/                          # Implementation files
│   ├── core/                          # Core implementations
│   │   ├── message.cpp
│   │   ├── message_bus.cpp
│   │   ├── message_broker.cpp
│   │   ├── message_queue.cpp
│   │   └── topic_router.cpp
│   │
│   ├── backends/                      # Backend implementations
│   │   ├── standalone_backend_impl.cpp
│   │   └── integration_backend_impl.cpp
│   │
│   ├── patterns/                      # Pattern implementations
│   │   ├── pub_sub_impl.cpp
│   │   ├── request_reply_impl.cpp
│   │   ├── event_streaming_impl.cpp
│   │   └── message_pipeline_impl.cpp
│   │
│   ├── task/                          # Task queue implementations
│   │   ├── task.cpp
│   │   ├── task_context.cpp
│   │   ├── task_queue.cpp
│   │   ├── worker_pool.cpp
│   │   ├── memory_result_backend.cpp
│   │   ├── async_result.cpp
│   │   ├── task_client.cpp
│   │   ├── scheduler.cpp
│   │   ├── cron_parser.cpp
│   │   ├── monitor.cpp
│   │   └── task_system.cpp
│   │
│   └── di/                            # Dependency injection
│       └── messaging_di_container.cpp
│
├── test/                              # Test suite
│   ├── unit/                          # Unit tests
│   │   ├── core/                      # Core component tests
│   │   │   ├── test_message.cpp
│   │   │   ├── test_message_bus.cpp
│   │   │   ├── test_message_broker.cpp
│   │   │   ├── test_message_queue.cpp
│   │   │   └── test_topic_router.cpp
│   │   │
│   │   ├── backends/                  # Backend tests
│   │   │   ├── test_standalone_backend.cpp
│   │   │   └── test_integration_backend.cpp
│   │   │
│   │   ├── patterns/                  # Pattern tests
│   │   │   ├── test_pub_sub.cpp
│   │   │   ├── test_request_reply.cpp
│   │   │   ├── test_event_streaming.cpp
│   │   │   └── test_message_pipeline.cpp
│   │   │
│   │   ├── task/                      # Task module tests
│   │   │   ├── test_task.cpp
│   │   │   ├── test_task_context.cpp
│   │   │   ├── test_task_handler.cpp
│   │   │   ├── test_task_queue.cpp
│   │   │   ├── test_worker_pool.cpp
│   │   │   ├── test_result_backend.cpp
│   │   │   ├── test_async_result.cpp
│   │   │   ├── test_task_client.cpp
│   │   │   ├── test_scheduler.cpp
│   │   │   ├── test_cron_parser.cpp
│   │   │   ├── test_monitor.cpp
│   │   │   ├── test_task_system.cpp
│   │   │   └── test_task_event_bridge.cpp
│   │   │
│   │   └── di/                        # DI container tests
│   │       └── test_di_container.cpp
│   │
│   └── benchmarks/                    # Performance benchmarks
│       ├── bench_message_creation.cpp
│       ├── bench_message_queue.cpp
│       ├── bench_topic_router.cpp
│       ├── bench_pub_sub_throughput.cpp
│       ├── bench_request_reply_latency.cpp
│       └── task/                      # Task module benchmarks
│           ├── bench_task_queue.cpp
│           ├── bench_worker_throughput.cpp
│           ├── bench_result_backend.cpp
│           └── bench_scheduler.cpp
│
├── integration_tests/                 # Integration test suites
│   ├── framework/                     # Test framework
│   │   ├── messaging_fixture.h
│   │   └── test_helpers.h
│   │
│   ├── test_message_bus_router.cpp    # Bus + Router integration
│   ├── test_priority_queue.cpp        # Priority queue scenarios
│   ├── test_backend_integration.cpp   # Backend integration
│   ├── test_full_integration.cpp      # End-to-end tests
│   │
│   └── task/                          # Task module integration tests
│       ├── task_fixture.h             # Task testing fixtures
│       ├── test_task_lifecycle.cpp    # Full task lifecycle
│       ├── test_worker_scenarios.cpp  # Worker pool scenarios
│       ├── test_scheduling.cpp        # Scheduler integration
│       ├── test_concurrent_load.cpp   # Concurrent load testing
│       └── test_failure_recovery.cpp  # Failure and recovery
│
├── examples/                          # Example applications
│   ├── basic_pub_sub/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   │
│   ├── request_reply/
│   │   ├── CMakeLists.txt
│   │   ├── server.cpp
│   │   └── client.cpp
│   │
│   ├── event_streaming/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   │
│   ├── message_pipeline/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   │
│   └── task/                          # Task queue examples
│       ├── CMakeLists.txt
│       ├── README.md                  # Task examples guide
│       ├── simple_worker/             # Basic task processing
│       ├── priority_tasks/            # Priority-based execution
│       ├── scheduled_tasks/           # Periodic/cron scheduling
│       ├── chain_workflow/            # Task chaining
│       ├── chord_aggregation/         # Parallel task aggregation
│       ├── progress_tracking/         # Progress monitoring
│       └── monitoring_dashboard/      # Real-time monitoring
│
├── docs/                              # Documentation
│   ├── README.md                      # Documentation index
│   ├── BENCHMARKS.md                  # Performance benchmarks
│   ├── FEATURES.md                    # Feature documentation
│   ├── PRODUCTION_QUALITY.md          # Quality assurance
│   ├── PROJECT_STRUCTURE.md           # This file
│   ├── API_REFERENCE.md               # API documentation
│   ├── MIGRATION_GUIDE.md             # Migration guide
│   ├── PATTERNS_API.md                # Patterns documentation
│   ├── DESIGN_PATTERNS.md             # Architecture patterns
│   └── task/                          # Task module documentation
│       └── ARCHITECTURE.md            # Task system architecture
│
├── .github/                           # GitHub configuration
│   └── workflows/                     # CI/CD workflows
│       ├── ci.yml                     # Main CI pipeline
│       ├── coverage.yml               # Code coverage
│       ├── static-analysis.yml        # Static analysis
│       └── documentation.yml          # Documentation generation
│
├── CMakeLists.txt                     # Root CMake configuration
├── README.md                          # Project README
└── LICENSE                            # BSD 3-Clause License
```

---

## Component Organization

### Core Components (`include/kcenon/messaging/core/`)

**Purpose**: Fundamental messaging infrastructure

| File | Description | Dependencies |
|------|-------------|--------------|
| `message.h` | Message structure, metadata, builder | container_system |
| `message_bus.h` | Central pub/sub coordinator | All core components |
| `message_broker.h` | Advanced routing and filtering | topic_router, queue |
| `message_queue.h` | Thread-safe priority queue | common_system |
| `topic_router.h` | Pattern-based topic routing | common_system |

### Interfaces (`include/kcenon/messaging/interfaces/`)

**Purpose**: Abstract interfaces for extensibility

| File | Description | Implementations |
|------|-------------|-----------------|
| `message_handler_interface.h` | Message processing abstraction | User-defined |
| `message_router_interface.h` | Routing abstraction | topic_router |
| `publisher_interface.h` | Publishing abstraction | publisher (patterns) |
| `subscriber_interface.h` | Subscription abstraction | subscriber (patterns) |
| `queue_interface.h` | Queue abstraction | message_queue |

### Backends (`include/kcenon/messaging/backends/`)

**Purpose**: Pluggable execution strategies

| File | Description | Use Case |
|------|-------------|----------|
| `backend_interface.h` | Backend abstraction | All backends |
| `standalone_backend.h` | Uses thread_system internally | Testing, simple apps |
| `integration_backend.h` | External thread pool | Production, integration |

### Patterns (`include/kcenon/messaging/patterns/`)

**Purpose**: High-level messaging patterns

| File | Description | Pattern Type |
|------|-------------|--------------|
| `pub_sub.h` | Publisher and Subscriber | Classic pub/sub |
| `request_reply.h` | Request client and server | Sync RPC |
| `event_streaming.h` | Event sourcing with replay | Event-driven |
| `message_pipeline.h` | Stage-based processing | Pipes-and-filters |

### Task Queue (`include/kcenon/messaging/task/`)

**Purpose**: Distributed task queue system with scheduling and monitoring

| File | Description | Dependencies |
|------|-------------|--------------|
| `task.h` | Task definition and builder | container_system |
| `task_handler.h` | Handler interface with C++20 Concepts | common_system |
| `task_context.h` | Execution context with progress | common_system |
| `task_queue.h` | Priority-based task queue | thread_system |
| `worker_pool.h` | Worker thread pool management | thread_system |
| `result_backend.h` | Result storage interface | container_system |
| `memory_result_backend.h` | In-memory result backend | result_backend |
| `async_result.h` | Async result handle with progress | result_backend |
| `task_client.h` | Task submission client | task_queue, async_result |
| `scheduler.h` | Periodic/cron scheduler | cron_parser |
| `cron_parser.h` | Cron expression parser | - |
| `monitor.h` | Real-time task monitoring | worker_pool |
| `task_system.h` | Unified facade | All task components |

---

## Build System

### CMake Structure

```
CMakeLists.txt                  # Root configuration
├── Library: messaging_system_core
├── Library: messaging_system_patterns
├── Library: messaging_system_backends
├── Executable: test targets
├── Executable: benchmark targets
└── Executable: example targets
```

### Build Targets

| Target | Type | Description |
|--------|------|-------------|
| `messaging_system_core` | Library | Core messaging |
| `messaging_system_patterns` | Library | Messaging patterns |
| `messaging_system_backends` | Library | Execution backends |
| `messaging_system_task` | Library | Task queue system |
| `test_*` | Executable | Unit tests |
| `bench_*` | Executable | Benchmarks |
| `example_*` | Executable | Examples |

---

## Dependencies

### Required Dependencies

| System | Purpose | Version |
|--------|---------|---------|
| **common_system** | Result<T>, error handling | >= 2.0 |
| **container_system** | Message payloads | >= 2.0 |

### Optional Dependencies

| System | Purpose | Version |
|--------|---------|---------|
| **thread_system** | Thread pools | >= 2.0 |
| **logger_system** | Logging | >= 2.0 |
| **monitoring_system** | Metrics | >= 2.0 |

---

## Include Patterns

### Application Code

```cpp
// Core functionality
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/message.h>

// Patterns
#include <kcenon/messaging/patterns/pub_sub.h>
#include <kcenon/messaging/patterns/request_reply.h>

// Backends
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/backends/integration_backend.h>

// Task Queue System
#include <kcenon/messaging/task/task_system.h>  // Unified facade
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_client.h>
#include <kcenon/messaging/task/scheduler.h>
```

### Internal Implementation

```cpp
// Implementation files should include both public and private headers
#include <kcenon/messaging/core/message_bus.h>
#include "internal_helpers.h"
```

---

## Naming Conventions

### Files

- **Headers**: `snake_case.h`
- **Implementation**: `snake_case_impl.cpp` or `snake_case.cpp`
- **Tests**: `test_snake_case.cpp`
- **Benchmarks**: `bench_snake_case.cpp`
- **Examples**: `snake_case.cpp`

### Classes

- **Interfaces**: `interface_name_interface`
- **Implementations**: `concrete_class_name`
- **Patterns**: `pattern_name` (e.g., `publisher`, `subscriber`)

### Namespaces

```cpp
namespace kcenon::messaging {              // Core namespace
namespace kcenon::messaging::patterns {    // Patterns
namespace kcenon::messaging::backends {    // Backends
namespace kcenon::messaging::task {        // Task queue system
namespace kcenon::messaging::di {          // DI container
}
```

---

## Code Organization Principles

### Separation of Concerns

1. **Public API** (`include/`): User-facing headers
2. **Implementation** (`src/impl/`): Private implementation
3. **Tests** (`test/`): Verification code
4. **Examples** (`examples/`): Usage demonstrations
5. **Documentation** (`docs/`): Reference materials

### Modularity

- **Core**: Foundation components
- **Patterns**: High-level abstractions
- **Backends**: Execution strategies
- **Task**: Distributed task queue system
- **Interfaces**: Extension points
- **Utilities**: Helper functions

### Dependencies

- **Core** depends on: common_system, container_system
- **Patterns** depend on: core
- **Backends** depend on: core, common_system
- **Task** depends on: common_system, container_system, thread_system
- **Everything** depends on: common_system (Result<T>)

---

## Future Structure

### Planned Additions

```
messaging_system/
├── include/kcenon/messaging/
│   ├── reliability/               # Reliability features
│   │   ├── retry_policy.h
│   │   ├── circuit_breaker.h
│   │   ├── dead_letter_queue.h
│   │   └── message_persistence.h
│   │
│   ├── routing/                   # Advanced routing
│   │   ├── content_router.h
│   │   ├── routing_table.h
│   │   └── load_balancer.h
│   │
│   ├── security/                  # Security features
│   │   ├── message_validator.h
│   │   ├── access_control.h
│   │   └── encryption.h
│   │
│   └── monitoring/                # Observability
│       ├── message_metrics.h
│       └── message_tracer.h
│
└── docs/
    └── guides/                    # User guides
        ├── QUICK_START.md
        ├── INTEGRATION.md
        └── TROUBLESHOOTING.md
```

---

**Last Updated**: 2025-12-10
**Version**: 0.1.1
