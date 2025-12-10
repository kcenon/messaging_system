# Messaging System Project Structure

**Version**: 1.0
**Last Updated**: 2025-11-18
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
│   └── utils/                         # Utilities
│       └── integration_detector.h     # System integration detection
│
├── src/impl/                          # Implementation files
│   ├── core/                          # Core implementations
│   │   ├── message_impl.cpp
│   │   ├── message_bus_impl.cpp
│   │   ├── message_broker_impl.cpp
│   │   ├── message_queue_impl.cpp
│   │   └── topic_router_impl.cpp
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
│   └── di/                            # Dependency injection
│       └── messaging_di_container.cpp
│
├── test/                              # Test suite
│   ├── unit/                          # Unit tests
│   │   ├── core/                      # Core component tests
│   │   │   ├── test_message.cpp
│   │   │   ├── test_message_bus.cpp
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
│   │   └── di/                        # DI container tests
│   │       └── test_di_container.cpp
│   │
│   └── benchmarks/                    # Performance benchmarks
│       ├── bench_message_creation.cpp
│       ├── bench_message_queue.cpp
│       ├── bench_topic_router.cpp
│       ├── bench_pub_sub_throughput.cpp
│       └── bench_request_reply_latency.cpp
│
├── integration_tests/                 # Integration test suites
│   ├── framework/                     # Test framework
│   │   ├── messaging_fixture.h
│   │   └── test_helpers.h
│   │
│   ├── test_message_bus_router.cpp    # Bus + Router integration
│   ├── test_priority_queue.cpp        # Priority queue scenarios
│   ├── test_backend_integration.cpp   # Backend integration
│   └── test_full_integration.cpp      # End-to-end tests
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
│   └── message_pipeline/
│       ├── CMakeLists.txt
│       └── main.cpp
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
│   └── DESIGN_PATTERNS.md             # Architecture patterns
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
- **Interfaces**: Extension points
- **Utilities**: Helper functions

### Dependencies

- **Core** depends on: common_system, container_system
- **Patterns** depend on: core
- **Backends** depend on: core, common_system
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

**Last Updated**: 2025-11-18
**Version**: 1.0
