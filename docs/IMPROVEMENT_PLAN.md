# Messaging System Improvement Plan

**Version**: 2.0
**Date**: 2025-11-16
**Status**: In Progress - Phase 7 Complete
**Last Updated**: 2025-11-17

## Progress Overview

| Phase | Status | Completion Date | Branch |
|-------|--------|-----------------|--------|
| Phase 1: Foundation | ✅ Complete | 2025-11-16 | feature/phase-1-foundation |
| Phase 2: Core Message Types | ✅ Complete | 2025-11-16 | feature/phase-2-core-message-types |
| Phase 3: Message Queue | ✅ Complete | 2025-11-16 | feature/phase-3-message-queue |
| Phase 4: Backend Pattern | ✅ Complete | 2025-11-17 | feature/phase-4-backend-pattern |
| Phase 5: Topic Router | ✅ Complete | 2025-11-17 | feature/phase-5-topic-router |
| Phase 6: Message Bus | ✅ Complete | 2025-11-17 | feature/phase-6-message-bus |
| Phase 7: DI Container | ✅ Complete | 2025-11-17 | feature/phase-7-di-container |
| Phase 8: Messaging Patterns | ⏳ Pending | - | - |
| Phase 9-10: Testing & Docs | ⏳ Pending | - | - |

**Overall Progress**: 7/10 phases complete (70%)

## Executive Summary

This document outlines a comprehensive plan to restructure the messaging_system to align with the architectural patterns established in other system modules (common_system, thread_system, logger_system, monitoring_system). The goal is to create a consistent, maintainable, and production-ready messaging infrastructure.

## Table of Contents

1. [Current State Analysis](#current-state-analysis)
2. [Identified Issues](#identified-issues)
3. [Improvement Objectives](#improvement-objectives)
4. [Target Architecture](#target-architecture)
5. [Implementation Plan](#implementation-plan)
6. [Migration Strategy](#migration-strategy)
7. [Testing Strategy](#testing-strategy)
8. [Timeline and Milestones](#timeline-and-milestones)

---

## Current State Analysis

### Existing Directory Structure

```
messaging_system/
├── include/messaging_system/          # Legacy structure
│   ├── core/
│   │   ├── message_bus.h
│   │   ├── topic_router.h
│   │   └── messaging_container.h
│   ├── integration/
│   │   ├── trace_context.h
│   │   ├── config_loader.h
│   │   └── network_bridge.h
│   └── error_codes.h                  # ❌ Conflicts with centralized codes
│
├── application_layer/include/kcenon/messaging/  # ❌ Duplicate structure
│   ├── core/
│   │   ├── message_bus.h
│   │   └── message_types.h
│   ├── cluster/
│   ├── routing/
│   ├── security/
│   └── services/
│
└── src/
    ├── core/
    └── integration/
```

### Current Capabilities

**Strengths**:
- ✅ Uses `Result<T>` pattern from common_system
- ✅ Integrates with container_system for message payloads
- ✅ Basic pub/sub implementation (TopicRouter)
- ✅ Message bus with async capabilities
- ✅ Topic pattern matching (wildcards)

**Weaknesses**:
- ❌ Error codes conflict with logger_system (-200 to -299)
- ❌ No DI container pattern
- ❌ No backend abstraction (standalone vs integration)
- ❌ Missing integration detection
- ❌ No unified adapter pattern
- ❌ Inconsistent directory structure with other systems
- ❌ Duplicate code between include/ and application_layer/

---

## Identified Issues

### 1. Error Code Range Conflict

**Problem**: messaging_system uses error codes -200 to -299, which conflicts with logger_system.

**Current**:
```cpp
// messaging_system/include/messaging_system/error_codes.h
namespace messaging::error {
    constexpr int INVALID_MESSAGE = -200;  // ❌ Conflicts with logger_system
    constexpr int ROUTING_FAILED = -201;
    // ...
}
```

**Impact**: Build failures when both systems are integrated.

### 2. Directory Structure Inconsistency

**Problem**: Other systems follow `kcenon/{system}/` pattern in include/, but messaging_system has both `messaging_system/` and `kcenon/messaging/`.

**Reference Pattern** (from logger_system, thread_system):
```
include/kcenon/{system}/
├── core/
├── interfaces/
├── adapters/
├── backends/
└── utils/
```

### 3. Missing Architectural Patterns

**Missing from messaging_system** but present in other systems:

| Pattern | logger_system | monitoring_system | messaging_system |
|---------|---------------|-------------------|------------------|
| DI Container | ✅ | ✅ | ❌ |
| Backend Abstraction | ✅ | ✅ | ❌ |
| Integration Detection | ✅ | ✅ | ❌ |
| Adapter Pattern | ✅ | ✅ | ❌ |
| Builder Pattern | ✅ | ❌ | ❌ |
| Interface Segregation | ✅ | ✅ | ⚠️ Partial |

### 4. Lack of Interface Definitions

**Problem**: No dedicated `interfaces/` directory for clean abstraction.

**Needed Interfaces**:
- `message_handler_interface.h` - Message processing abstraction
- `message_router_interface.h` - Routing abstraction
- `publisher_interface.h` - Publishing abstraction
- `subscriber_interface.h` - Subscription abstraction
- `message_broker_interface.h` - Broker abstraction

---

## Improvement Objectives

### Primary Goals

1. **Consistency**: Align with established patterns in common_system, thread_system, logger_system
2. **Modularity**: Clear separation of concerns with well-defined interfaces
3. **Flexibility**: Support standalone, integrated, and distributed modes
4. **Performance**: Lock-free queues, efficient routing, minimal overhead
5. **Reliability**: Comprehensive error handling, Result<T> pattern throughout
6. **Observability**: Deep integration with logger_system and monitoring_system

### Secondary Goals

1. Backward compatibility layer for existing code
2. Comprehensive documentation
3. Rich example suite
4. Performance benchmarks
5. Integration tests with all dependent systems

---

## Target Architecture

### Directory Structure (New)

```
messaging_system/
├── include/kcenon/messaging/
│   ├── core/                          # Core messaging components
│   │   ├── message.h                  # Message structure
│   │   ├── message_bus.h              # Central message hub
│   │   ├── message_broker.h           # Message brokering logic
│   │   ├── message_queue.h            # Queue implementation
│   │   ├── topic_router.h             # Topic-based routing
│   │   ├── subscription_manager.h     # Subscription lifecycle
│   │   ├── message_serializer.h       # Serialization utilities
│   │   └── error_codes.h              # Forward to common_system
│   │
│   ├── interfaces/                    # Abstract interfaces
│   │   ├── message_handler_interface.h
│   │   ├── message_router_interface.h
│   │   ├── publisher_interface.h
│   │   ├── subscriber_interface.h
│   │   ├── message_broker_interface.h
│   │   ├── queue_interface.h
│   │   └── serializer_interface.h
│   │
│   ├── adapters/                      # System integration adapters
│   │   ├── common_system_adapter.h    # common_system types
│   │   ├── thread_pool_adapter.h      # thread_system executor
│   │   ├── logger_adapter.h           # logger_system integration
│   │   ├── monitoring_adapter.h       # monitoring_system integration
│   │   └── container_adapter.h        # container_system messages
│   │
│   ├── backends/                      # Execution backends
│   │   ├── backend_interface.h
│   │   ├── standalone_backend.h       # Independent operation
│   │   ├── integration_backend.h      # Full system integration
│   │   └── distributed_backend.h      # Cluster/distributed mode
│   │
│   ├── patterns/                      # Messaging patterns
│   │   ├── pub_sub.h                  # Publish-Subscribe pattern
│   │   ├── request_reply.h            # Request-Reply pattern
│   │   ├── event_streaming.h          # Event sourcing/streaming
│   │   ├── message_pipeline.h         # Message processing pipeline
│   │   └── saga_coordinator.h         # Distributed transactions
│   │
│   ├── reliability/                   # Reliability features
│   │   ├── retry_policy.h             # Retry strategies
│   │   ├── circuit_breaker.h          # Circuit breaker pattern
│   │   ├── dead_letter_queue.h        # Failed message handling
│   │   └── message_persistence.h      # Durable messaging
│   │
│   ├── routing/                       # Advanced routing
│   │   ├── topic_matcher.h            # Pattern matching
│   │   ├── content_router.h           # Content-based routing
│   │   ├── routing_table.h            # Routing rules
│   │   └── load_balancer.h            # Message distribution
│   │
│   ├── security/                      # Security features
│   │   ├── message_validator.h        # Message validation
│   │   ├── access_control.h           # Authorization
│   │   ├── encryption.h               # Message encryption
│   │   └── signature.h                # Message signing
│   │
│   ├── monitoring/                    # Observability
│   │   ├── message_metrics.h          # Metrics collection
│   │   ├── message_tracer.h           # Distributed tracing
│   │   └── health_monitor.h           # System health
│   │
│   ├── utils/                         # Utilities
│   │   ├── message_id_generator.h
│   │   ├── timestamp_provider.h
│   │   ├── compression.h
│   │   └── thread_integration_detector.h
│   │
│   └── compatibility.h                # Backward compatibility
│
├── src/impl/                          # Implementation details
│   ├── core/
│   │   ├── message_bus_impl.cpp
│   │   ├── message_broker_impl.cpp
│   │   └── message_queue_impl.cpp
│   ├── adapters/
│   ├── backends/
│   ├── patterns/
│   ├── di/                            # Dependency injection
│   │   ├── messaging_di_container.cpp
│   │   └── service_registry.cpp
│   └── lockfree/                      # Lock-free structures
│       ├── spsc_queue.h               # Single-producer single-consumer
│       └── mpmc_queue.h               # Multi-producer multi-consumer
│
├── integration_tests/                 # Integration tests
│   ├── framework/
│   │   ├── messaging_fixture.h
│   │   └── test_helpers.h
│   ├── test_pub_sub.cpp
│   ├── test_request_reply.cpp
│   ├── test_integration_with_logger.cpp
│   └── test_integration_with_monitoring.cpp
│
├── examples/                          # Example applications
│   ├── basic_pub_sub/
│   ├── request_reply/
│   ├── event_streaming/
│   ├── distributed_messaging/
│   └── integration_example/
│
└── docs/
    ├── IMPROVEMENT_PLAN.md            # This document
    ├── ARCHITECTURE.md
    ├── API_REFERENCE.md
    └── MIGRATION_GUIDE.md
```

### Component Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Messaging Patterns                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │ Pub/Sub  │  │ Req/Rep  │  │ Pipeline │  │  Saga    │   │
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
│  │ Persistent Queue    │  │   │  └─────────────────────┘ │
│  └─────────────────────┘  │   └───────────────────────────┘
└───────────────────────────┘
                              │
                ┌─────────────┴─────────────┐
                ▼                           ▼
┌───────────────────────────┐   ┌───────────────────────────┐
│  Integration Layer        │   │   Reliability Layer       │
│  ┌──────────────────┐     │   │  ┌──────────────────┐    │
│  │ Logger Adapter   │     │   │  │ Retry Policy     │    │
│  │ Monitor Adapter  │     │   │  │ Circuit Breaker  │    │
│  │ Thread Adapter   │     │   │  │ Persistence      │    │
│  └──────────────────┘     │   │  └──────────────────┘    │
└───────────────────────────┘   └───────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Backend Selection                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Standalone   │  │ Integration  │  │ Distributed  │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              Foundation (common_system)                      │
│   Result<T>, error_info, event_bus, interfaces              │
└─────────────────────────────────────────────────────────────┘
```

---

## Implementation Plan

### Phase 1: Foundation and Error Codes (Week 1)

#### 1.1 Add messaging_system Error Codes to common_system

**File**: `common_system/include/kcenon/common/error/error_codes.h`

**Changes**:
```cpp
// ============================================================================
// messaging_system Error Codes (-700 to -799)
// ============================================================================
namespace messaging_system {
    constexpr int base = static_cast<int>(category::messaging_system);

    // Message errors (-700 to -719)
    constexpr int invalid_message = base - 0;
    constexpr int message_too_large = base - 1;
    constexpr int message_expired = base - 2;
    constexpr int invalid_payload = base - 3;

    // Routing errors (-720 to -739)
    constexpr int routing_failed = base - 20;
    constexpr int unknown_topic = base - 21;
    constexpr int no_subscribers = base - 22;
    constexpr int invalid_topic_pattern = base - 23;

    // Queue errors (-740 to -759)
    constexpr int queue_full = base - 40;
    constexpr int queue_empty = base - 41;
    constexpr int queue_stopped = base - 42;
    constexpr int enqueue_failed = base - 43;
    constexpr int dequeue_failed = base - 44;

    // Subscription errors (-760 to -779)
    constexpr int subscription_failed = base - 60;
    constexpr int subscription_not_found = base - 61;
    constexpr int duplicate_subscription = base - 62;
    constexpr int unsubscribe_failed = base - 63;

    // Publishing errors (-780 to -799)
    constexpr int publication_failed = base - 80;
    constexpr int no_route_found = base - 81;
    constexpr int message_rejected = base - 82;
    constexpr int broker_unavailable = base - 83;
}
```

**Also add to category enum**:
```cpp
enum class category : int {
    success = 0,
    common = -1,
    thread_system = -100,
    logger_system = -200,
    monitoring_system = -300,
    container_system = -400,
    database_system = -500,
    network_system = -600,
    messaging_system = -700,  // NEW
};
```

**Validation**:
```cpp
// Add to validation namespace
static_assert(codes::messaging_system::base == -700,
              "messaging_system base must be -700");
static_assert(codes::messaging_system::invalid_message >= -799 &&
              codes::messaging_system::invalid_message <= -700,
              "messaging_system error codes must be in range [-799, -700]");
```

#### 1.2 Create Core Interfaces

**File**: `include/kcenon/messaging/interfaces/message_handler_interface.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include <memory>

namespace kcenon::messaging {

// Forward declarations
class message;

/**
 * @interface message_handler_interface
 * @brief Abstract interface for message handlers
 */
class message_handler_interface {
public:
    virtual ~message_handler_interface() = default;

    /**
     * @brief Handle a message
     * @param msg Message to process
     * @return Result indicating success or error
     */
    virtual common::VoidResult handle(const message& msg) = 0;

    /**
     * @brief Check if handler can process this message
     * @param msg Message to check
     * @return true if handler can process, false otherwise
     */
    virtual bool can_handle(const message& msg) const = 0;
};

} // namespace kcenon::messaging
```

**File**: `include/kcenon/messaging/interfaces/publisher_interface.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include <string>
#include <memory>

namespace kcenon::messaging {

class message;

/**
 * @interface publisher_interface
 * @brief Abstract interface for message publishers
 */
class publisher_interface {
public:
    virtual ~publisher_interface() = default;

    /**
     * @brief Publish a message to a topic
     * @param topic Topic to publish to
     * @param msg Message to publish
     * @return Result indicating success or error
     */
    virtual common::VoidResult publish(
        const std::string& topic,
        const message& msg
    ) = 0;

    /**
     * @brief Check if publisher is ready
     * @return true if ready to publish
     */
    virtual bool is_ready() const = 0;
};

} // namespace kcenon::messaging
```

#### 1.3 Integration Detection Utility

**File**: `include/kcenon/messaging/utils/thread_integration_detector.h`

```cpp
#pragma once

namespace kcenon::messaging {

/**
 * @class thread_integration_detector
 * @brief Compile-time detection of thread_system availability
 *
 * Pattern borrowed from logger_system for consistent integration detection
 */
class thread_integration_detector {
public:
    /**
     * @brief Check if thread_system is available at compile time
     * @return true if thread_system headers are included
     */
    static constexpr bool has_thread_system() {
#ifdef KCENON_THREAD_SYSTEM_AVAILABLE
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if logger_system is available
     */
    static constexpr bool has_logger_system() {
#ifdef KCENON_LOGGER_SYSTEM_AVAILABLE
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if monitoring_system is available
     */
    static constexpr bool has_monitoring_system() {
#ifdef KCENON_MONITORING_SYSTEM_AVAILABLE
        return true;
#else
        return false;
#endif
    }
};

} // namespace kcenon::messaging
```

### Phase 2: Core Message Types (Week 2)

#### 2.1 Message Structure

**File**: `include/kcenon/messaging/core/message.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include <core/container.h>
#include <string>
#include <chrono>
#include <memory>
#include <optional>

namespace kcenon::messaging {

/**
 * @enum message_priority
 * @brief Message priority levels
 */
enum class message_priority : uint8_t {
    lowest = 0,
    low = 1,
    normal = 2,
    high = 3,
    highest = 4,
    critical = 5
};

/**
 * @enum message_type
 * @brief Message type classification
 */
enum class message_type {
    command,        // Execute an action
    event,          // Something happened
    query,          // Request information
    reply,          // Response to query/command
    notification    // Informational message
};

/**
 * @struct message_metadata
 * @brief Message metadata and headers
 */
struct message_metadata {
    std::string id;                    // Unique message ID
    std::string topic;                 // Topic/channel
    std::string source;                // Source service/component
    std::string target;                // Target service/component (optional)
    std::string correlation_id;        // For request/reply correlation
    std::string trace_id;              // Distributed tracing ID

    message_type type;
    message_priority priority;

    std::chrono::system_clock::time_point timestamp;
    std::optional<std::chrono::milliseconds> ttl;  // Time-to-live

    // Additional headers (key-value pairs)
    std::unordered_map<std::string, std::string> headers;
};

/**
 * @class message
 * @brief Core message structure using container_system
 */
class message {
    message_metadata metadata_;
    std::shared_ptr<container_module::value_container> payload_;

public:
    message();

    // Constructors
    explicit message(const std::string& topic);
    message(const std::string& topic, message_type type);

    // Metadata access
    const message_metadata& metadata() const { return metadata_; }
    message_metadata& metadata() { return metadata_; }

    // Payload access
    const container_module::value_container& payload() const;
    container_module::value_container& payload();

    // Convenience methods
    bool is_expired() const;
    std::chrono::milliseconds age() const;

    // Serialization
    common::Result<std::vector<uint8_t>> serialize() const;
    static common::Result<message> deserialize(const std::vector<uint8_t>& data);
};

/**
 * @class message_builder
 * @brief Builder pattern for message construction
 */
class message_builder {
    message msg_;

public:
    message_builder();

    message_builder& topic(std::string topic);
    message_builder& source(std::string source);
    message_builder& target(std::string target);
    message_builder& type(message_type type);
    message_builder& priority(message_priority priority);
    message_builder& ttl(std::chrono::milliseconds ttl);
    message_builder& correlation_id(std::string id);
    message_builder& trace_id(std::string id);
    message_builder& header(std::string key, std::string value);

    message_builder& payload(std::shared_ptr<container_module::value_container> payload);

    common::Result<message> build();
};

} // namespace kcenon::messaging
```

### Phase 3: Message Queue and Broker (Week 3)

#### 3.1 Queue Interface

**File**: `include/kcenon/messaging/interfaces/queue_interface.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include "../core/message.h"
#include <chrono>
#include <cstddef>

namespace kcenon::messaging {

/**
 * @interface queue_interface
 * @brief Abstract interface for message queues
 */
class queue_interface {
public:
    virtual ~queue_interface() = default;

    /**
     * @brief Enqueue a message
     * @param msg Message to enqueue
     * @return Result indicating success or error
     */
    virtual common::VoidResult enqueue(message msg) = 0;

    /**
     * @brief Dequeue a message (blocking)
     * @param timeout Maximum time to wait
     * @return Result containing message or error
     */
    virtual common::Result<message> dequeue(
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    ) = 0;

    /**
     * @brief Try to dequeue (non-blocking)
     * @return Result containing message or empty if queue is empty
     */
    virtual common::Result<message> try_dequeue() = 0;

    /**
     * @brief Get current queue size
     */
    virtual size_t size() const = 0;

    /**
     * @brief Check if queue is empty
     */
    virtual bool empty() const = 0;

    /**
     * @brief Clear all messages
     */
    virtual void clear() = 0;
};

} // namespace kcenon::messaging
```

#### 3.2 Message Queue Implementation

**File**: `include/kcenon/messaging/core/message_queue.h`

```cpp
#pragma once

#include "../interfaces/queue_interface.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace kcenon::messaging {

/**
 * @struct queue_config
 * @brief Configuration for message queue
 */
struct queue_config {
    size_t max_size = 10000;
    bool enable_priority = false;
    bool enable_persistence = false;
    bool drop_on_full = false;  // Drop oldest on full vs reject
};

/**
 * @class message_queue
 * @brief Thread-safe message queue implementation
 */
class message_queue : public queue_interface {
    struct priority_comparator {
        bool operator()(const message& a, const message& b) const {
            return static_cast<int>(a.metadata().priority) <
                   static_cast<int>(b.metadata().priority);
        }
    };

    queue_config config_;

    // Use priority_queue if enabled, else regular queue
    std::variant<
        std::queue<message>,
        std::priority_queue<message, std::vector<message>, priority_comparator>
    > queue_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stopped_{false};

public:
    explicit message_queue(queue_config config = {});
    ~message_queue() override;

    // queue_interface implementation
    common::VoidResult enqueue(message msg) override;
    common::Result<message> dequeue(std::chrono::milliseconds timeout) override;
    common::Result<message> try_dequeue() override;

    size_t size() const override;
    bool empty() const override;
    void clear() override;

    // Lifecycle
    void stop();
    bool is_stopped() const { return stopped_.load(); }

private:
    size_t get_queue_size() const;
    void push_to_queue(message msg);
    std::optional<message> pop_from_queue();
};

} // namespace kcenon::messaging
```

### Phase 4: Backend Pattern (Week 4)

#### 4.1 Backend Interface

**File**: `include/kcenon/messaging/backends/backend_interface.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/interfaces/logger_interface.h>
#include <kcenon/common/interfaces/monitoring_interface.h>
#include <memory>

namespace kcenon::messaging {

/**
 * @interface backend_interface
 * @brief Abstract backend for messaging system execution
 *
 * Pattern borrowed from logger_system for consistent backend abstraction
 */
class backend_interface {
public:
    virtual ~backend_interface() = default;

    /**
     * @brief Initialize backend
     */
    virtual common::VoidResult initialize() = 0;

    /**
     * @brief Shutdown backend
     */
    virtual common::VoidResult shutdown() = 0;

    /**
     * @brief Get executor for async operations
     */
    virtual std::shared_ptr<common::interfaces::IExecutor> get_executor() = 0;

    /**
     * @brief Get logger instance (optional)
     */
    virtual std::shared_ptr<common::interfaces::ILogger> get_logger() {
        return nullptr;
    }

    /**
     * @brief Get monitoring instance (optional)
     */
    virtual std::shared_ptr<common::interfaces::IMonitoring> get_monitoring() {
        return nullptr;
    }

    /**
     * @brief Check if backend is ready
     */
    virtual bool is_ready() const = 0;
};

} // namespace kcenon::messaging
```

#### 4.2 Standalone Backend

**File**: `include/kcenon/messaging/backends/standalone_backend.h`

```cpp
#pragma once

#include "backend_interface.h"
#include <thread>
#include <vector>

namespace kcenon::messaging {

/**
 * @class standalone_backend
 * @brief Self-contained backend with internal thread pool
 *
 * Uses std::thread directly without external dependencies
 */
class standalone_backend : public backend_interface {
    struct internal_executor;

    size_t num_threads_;
    std::shared_ptr<internal_executor> executor_;
    bool initialized_{false};

public:
    explicit standalone_backend(size_t num_threads = std::thread::hardware_concurrency());
    ~standalone_backend() override;

    common::VoidResult initialize() override;
    common::VoidResult shutdown() override;

    std::shared_ptr<common::interfaces::IExecutor> get_executor() override;
    bool is_ready() const override;
};

} // namespace kcenon::messaging
```

#### 4.3 Integration Backend

**File**: `include/kcenon/messaging/backends/integration_backend.h`

```cpp
#pragma once

#include "backend_interface.h"

namespace kcenon::messaging {

/**
 * @class integration_backend
 * @brief Backend that uses external system services
 *
 * Integrates with thread_system, logger_system, monitoring_system
 */
class integration_backend : public backend_interface {
    std::shared_ptr<common::interfaces::IExecutor> executor_;
    std::shared_ptr<common::interfaces::ILogger> logger_;
    std::shared_ptr<common::interfaces::IMonitoring> monitoring_;
    bool initialized_{false};

public:
    integration_backend(
        std::shared_ptr<common::interfaces::IExecutor> executor,
        std::shared_ptr<common::interfaces::ILogger> logger = nullptr,
        std::shared_ptr<common::interfaces::IMonitoring> monitoring = nullptr
    );

    common::VoidResult initialize() override;
    common::VoidResult shutdown() override;

    std::shared_ptr<common::interfaces::IExecutor> get_executor() override;
    std::shared_ptr<common::interfaces::ILogger> get_logger() override;
    std::shared_ptr<common::interfaces::IMonitoring> get_monitoring() override;

    bool is_ready() const override;
};

} // namespace kcenon::messaging
```

### Phase 5: Topic Router and Pub/Sub (Week 5)

#### 5.1 Topic Router Implementation

**File**: `include/kcenon/messaging/core/topic_router.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include "../interfaces/message_router_interface.h"
#include "message.h"
#include <functional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <regex>

namespace kcenon::messaging {

/**
 * @brief Subscription callback function type
 */
using subscription_callback = std::function<common::VoidResult(const message&)>;

/**
 * @brief Message filter function type
 */
using message_filter = std::function<bool(const message&)>;

/**
 * @struct subscription
 * @brief Represents a topic subscription
 */
struct subscription {
    uint64_t id;
    std::string topic_pattern;  // Supports wildcards: *, #
    subscription_callback callback;
    message_filter filter;
    int priority;  // Higher = executed first

    bool matches(const std::string& topic) const;
};

/**
 * @class topic_router
 * @brief Routes messages based on topic patterns
 */
class topic_router {
    std::unordered_map<std::string, std::vector<subscription>> subscriptions_;
    mutable std::shared_mutex mutex_;
    std::atomic<uint64_t> next_id_{1};

public:
    topic_router() = default;

    /**
     * @brief Subscribe to a topic pattern
     * @param pattern Topic pattern (supports * and # wildcards)
     * @param callback Callback to invoke for matching messages
     * @param filter Optional message filter
     * @param priority Subscription priority (0-10, higher = first)
     * @return Subscription ID or error
     */
    common::Result<uint64_t> subscribe(
        const std::string& pattern,
        subscription_callback callback,
        message_filter filter = nullptr,
        int priority = 5
    );

    /**
     * @brief Unsubscribe by subscription ID
     */
    common::VoidResult unsubscribe(uint64_t subscription_id);

    /**
     * @brief Route a message to matching subscribers
     */
    common::VoidResult route(const message& msg);

    /**
     * @brief Get number of subscriptions for a topic
     */
    size_t subscriber_count(const std::string& topic) const;

    /**
     * @brief Get all active topic patterns
     */
    std::vector<std::string> get_topics() const;

private:
    std::vector<subscription*> find_matching_subscriptions(const std::string& topic);
    bool match_pattern(const std::string& topic, const std::string& pattern) const;
};

} // namespace kcenon::messaging
```

**Topic Pattern Matching**:
- `user.*` matches `user.created`, `user.updated` (single level)
- `user.#` matches `user.created`, `user.profile.updated` (multi-level)
- `*.created` matches `user.created`, `order.created`

### Phase 6: Message Bus (Week 6)

#### 6.1 Message Bus Implementation

**File**: `include/kcenon/messaging/core/message_bus.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include "message.h"
#include "message_queue.h"
#include "topic_router.h"
#include "../backends/backend_interface.h"
#include <memory>
#include <atomic>

namespace kcenon::messaging {

/**
 * @struct message_bus_config
 * @brief Configuration for message bus
 */
struct message_bus_config {
    size_t queue_capacity = 10000;
    size_t worker_threads = 4;
    bool enable_priority_queue = true;
    bool enable_dead_letter_queue = true;
    bool enable_metrics = true;
    std::chrono::milliseconds processing_timeout{5000};
};

/**
 * @class message_bus
 * @brief Central message hub for publish-subscribe messaging
 */
class message_bus {
    message_bus_config config_;
    std::shared_ptr<backend_interface> backend_;
    std::unique_ptr<message_queue> queue_;
    std::unique_ptr<topic_router> router_;
    std::atomic<bool> running_{false};

    // Statistics
    struct statistics {
        std::atomic<uint64_t> messages_published{0};
        std::atomic<uint64_t> messages_processed{0};
        std::atomic<uint64_t> messages_failed{0};
        std::atomic<uint64_t> messages_dropped{0};
    } stats_;

public:
    explicit message_bus(
        std::shared_ptr<backend_interface> backend,
        message_bus_config config = {}
    );
    ~message_bus();

    // Lifecycle
    common::VoidResult start();
    common::VoidResult stop();
    bool is_running() const { return running_.load(); }

    // Publishing
    common::VoidResult publish(const message& msg);
    common::VoidResult publish(const std::string& topic, message msg);

    // Subscription
    common::Result<uint64_t> subscribe(
        const std::string& topic_pattern,
        subscription_callback callback,
        message_filter filter = nullptr,
        int priority = 5
    );

    common::VoidResult unsubscribe(uint64_t subscription_id);

    // Request-Reply pattern
    common::Result<message> request(
        const message& request,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
    );

    // Statistics
    struct statistics_snapshot {
        uint64_t messages_published;
        uint64_t messages_processed;
        uint64_t messages_failed;
        uint64_t messages_dropped;
    };

    statistics_snapshot get_statistics() const;
    void reset_statistics();

private:
    void process_messages();
    common::VoidResult handle_message(const message& msg);
};

} // namespace kcenon::messaging
```

### Phase 7: DI Container (Week 7)

#### 7.1 DI Container Interface

**File**: `src/impl/di/messaging_di_container.h`

```cpp
#pragma once

#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/interfaces/logger_interface.h>
#include <kcenon/common/interfaces/monitoring_interface.h>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <any>

namespace kcenon::messaging::di {

/**
 * @class messaging_di_container
 * @brief Dependency injection container for messaging system
 *
 * Pattern borrowed from logger_system for consistent DI
 */
class messaging_di_container {
    std::unordered_map<std::type_index, std::any> services_;
    mutable std::mutex mutex_;

public:
    /**
     * @brief Register a service
     */
    template<typename T>
    void register_service(std::shared_ptr<T> service) {
        std::lock_guard lock(mutex_);
        services_[typeid(T)] = service;
    }

    /**
     * @brief Resolve a service
     */
    template<typename T>
    std::shared_ptr<T> resolve() const {
        std::lock_guard lock(mutex_);
        auto it = services_.find(typeid(T));
        if (it != services_.end()) {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Check if service is registered
     */
    template<typename T>
    bool has_service() const {
        std::lock_guard lock(mutex_);
        return services_.find(typeid(T)) != services_.end();
    }

    /**
     * @brief Clear all services
     */
    void clear() {
        std::lock_guard lock(mutex_);
        services_.clear();
    }
};

/**
 * @brief Get global DI container
 */
messaging_di_container& get_global_container();

} // namespace kcenon::messaging::di
```

### Phase 8: Patterns Implementation (Week 8)

#### 8.1 Pub/Sub Pattern

**File**: `include/kcenon/messaging/patterns/pub_sub.h`

```cpp
#pragma once

#include "../core/message_bus.h"
#include <kcenon/common/patterns/result.h>

namespace kcenon::messaging::patterns {

/**
 * @class publisher
 * @brief High-level publisher for pub/sub pattern
 */
class publisher {
    std::shared_ptr<message_bus> bus_;
    std::string default_topic_;

public:
    publisher(std::shared_ptr<message_bus> bus, std::string default_topic);

    /**
     * @brief Publish to default topic
     */
    common::VoidResult publish(message msg);

    /**
     * @brief Publish to specific topic
     */
    common::VoidResult publish(const std::string& topic, message msg);
};

/**
 * @class subscriber
 * @brief High-level subscriber for pub/sub pattern
 */
class subscriber {
    std::shared_ptr<message_bus> bus_;
    std::vector<uint64_t> subscription_ids_;

public:
    explicit subscriber(std::shared_ptr<message_bus> bus);
    ~subscriber();

    /**
     * @brief Subscribe to a topic pattern
     */
    common::Result<uint64_t> subscribe(
        const std::string& topic_pattern,
        subscription_callback callback,
        message_filter filter = nullptr
    );

    /**
     * @brief Unsubscribe from all topics
     */
    common::VoidResult unsubscribe_all();
};

} // namespace kcenon::messaging::patterns
```

#### 8.2 Request-Reply Pattern

**File**: `include/kcenon/messaging/patterns/request_reply.h`

```cpp
#pragma once

#include "../core/message_bus.h"
#include <future>

namespace kcenon::messaging::patterns {

/**
 * @class request_reply_handler
 * @brief Handles request-reply messaging pattern
 */
class request_reply_handler {
    std::shared_ptr<message_bus> bus_;
    std::string service_topic_;
    std::unordered_map<std::string, std::promise<message>> pending_requests_;
    mutable std::mutex mutex_;

public:
    request_reply_handler(std::shared_ptr<message_bus> bus, std::string service_topic);

    /**
     * @brief Send request and wait for reply
     */
    common::Result<message> request(
        message req,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
    );

    /**
     * @brief Register reply handler (for service side)
     */
    common::VoidResult register_handler(
        std::function<common::Result<message>(const message&)> handler
    );

private:
    void handle_reply(const message& reply);
};

} // namespace kcenon::messaging::patterns
```

---

## Migration Strategy

### Backward Compatibility

#### Legacy Include Guard

**File**: `include/kcenon/messaging/compatibility.h`

```cpp
#pragma once

// Provide backward compatibility for existing code

#include "core/message.h"
#include "core/message_bus.h"

namespace messaging {
    // Legacy namespace aliases
    using kcenon::messaging::message;
    using kcenon::messaging::message_bus;
    using kcenon::messaging::message_priority;
    using kcenon::messaging::message_type;
}

// Legacy error codes (forward to centralized codes)
#include <kcenon/common/error/error_codes.h>

namespace messaging::error {
    using namespace kcenon::common::error::codes::messaging_system;

    // Remap old codes to new ones
    constexpr int INVALID_MESSAGE = invalid_message;
    constexpr int ROUTING_FAILED = routing_failed;
    constexpr int QUEUE_FULL = queue_full;
    // ... other mappings
}
```

### Migration Path

**Step 1**: Update common_system with new error codes
**Step 2**: Add compatibility.h for legacy code
**Step 3**: Gradually migrate components to new structure
**Step 4**: Update examples and tests
**Step 5**: Mark old headers as deprecated
**Step 6**: Remove deprecated code after grace period

### Coexistence Period

For 2 releases, support both:
```cpp
// Old way (deprecated)
#include <messaging_system/core/message_bus.h>
using namespace messaging;

// New way (recommended)
#include <kcenon/messaging/core/message_bus.h>
using namespace kcenon::messaging;
```

---

## Testing Strategy

### Unit Tests

**Coverage targets**:
- Core components: 90%+
- Interfaces: 100%
- Adapters: 85%+
- Patterns: 90%+

**Test structure**:
```
tests/
├── unit/
│   ├── core/
│   │   ├── test_message.cpp
│   │   ├── test_message_queue.cpp
│   │   ├── test_topic_router.cpp
│   │   └── test_message_bus.cpp
│   ├── patterns/
│   │   ├── test_pub_sub.cpp
│   │   └── test_request_reply.cpp
│   └── backends/
│       ├── test_standalone_backend.cpp
│       └── test_integration_backend.cpp
```

### Integration Tests

**Scenarios**:
1. **Message Bus + Topic Router**: Publish-subscribe flow
2. **Message Queue + Priority**: Priority message handling
3. **Backend Integration**: Thread pool integration
4. **Logger Integration**: Logging integration
5. **Monitoring Integration**: Metrics collection
6. **Full Stack**: End-to-end messaging

**File**: `integration_tests/test_full_integration.cpp`

```cpp
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/integration_backend.h>
#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/logger/core/logger.h>
#include <kcenon/monitoring/core/performance_monitor.h>

TEST(FullIntegration, MessageBusWithAllSystems) {
    // Setup thread pool
    auto thread_pool = thread::create_thread_pool(4);

    // Setup logger
    auto logger = logger::logger_builder()
        .with_async(true)
        .build();

    // Setup monitoring
    auto monitor = monitoring::performance_monitor::create();

    // Create integration backend
    auto backend = std::make_shared<messaging::integration_backend>(
        thread_pool, logger, monitor
    );

    // Create message bus
    auto bus = std::make_shared<messaging::message_bus>(backend);
    ASSERT_TRUE(bus->start().is_ok());

    // Test pub/sub
    bool received = false;
    auto sub_result = bus->subscribe("test.topic", [&](const auto& msg) {
        received = true;
        return common::ok();
    });
    ASSERT_TRUE(sub_result.is_ok());

    // Publish message
    auto msg = messaging::message_builder()
        .topic("test.topic")
        .build();
    ASSERT_TRUE(msg.is_ok());
    ASSERT_TRUE(bus->publish(msg.unwrap()).is_ok());

    // Wait for delivery
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(received);

    // Check statistics
    auto stats = bus->get_statistics();
    EXPECT_EQ(stats.messages_published, 1);
    EXPECT_EQ(stats.messages_processed, 1);
}
```

### Performance Benchmarks

**Metrics**:
- Message throughput (messages/sec)
- Latency (p50, p95, p99)
- Memory usage
- CPU usage
- Lock contention

**Benchmark suite**:
```
benchmarks/
├── bench_message_creation.cpp
├── bench_message_queue.cpp
├── bench_topic_router.cpp
├── bench_pub_sub_throughput.cpp
└── bench_request_reply_latency.cpp
```

---

## Timeline and Milestones

### Phase 1: Foundation (Week 1) - ✅ COMPLETED
- ✅ Error codes in messaging_system (-700 to -799 range)
- ✅ Core interfaces defined (message_handler, publisher, subscriber)
- ✅ Integration detector utility
- ⏳ Message structure implementation (moved to Phase 2)

**Status**: Completed 2025-11-16
**Branch**: feature/phase-1-foundation
**Deliverable**: Error codes, core interfaces, and integration detection

### Phase 2: Core Message Types (Week 2) - ✅ COMPLETED
- ✅ Message structure implemented
- ✅ Message builder pattern
- ✅ Message serialization/deserialization

**Status**: Completed 2025-11-16
**Branch**: feature/phase-2-core-message-types
**Deliverable**: Core message types and builders

### Phase 3: Message Queue (Week 3) - ✅ COMPLETED
- ✅ Queue interface
- ✅ Message queue implementation
- ✅ Priority queue support
- ✅ Dead letter queue

**Status**: Completed 2025-11-16
**Branch**: feature/phase-3-message-queue
**Deliverable**: Thread-safe message queue

### Phase 4: Backend Pattern (Week 4) - ✅ COMPLETED
- ✅ Backend interface
- ✅ Standalone backend
- ✅ Integration backend
- ✅ Backend auto-detection
- ✅ Unit tests (27 tests passing)

**Status**: Completed 2025-11-17
**Branch**: feature/phase-4-backend-pattern
**Deliverable**: Backend abstraction working with comprehensive tests

### Phase 5: Topic Router (Week 5) - ✅ COMPLETED
- ✅ Topic pattern matching
- ✅ Subscription manager
- ✅ Wildcard support (*, #)
- ✅ Filter chains
- ✅ Unit tests (27 tests passing)

**Status**: Completed 2025-11-17
**Branch**: feature/phase-5-topic-router
**Deliverable**: Topic routing with pattern matching

### Phase 6: Message Bus (Week 6) - ✅ COMPLETED
- ✅ Message bus core
- ✅ Pub/sub basic functionality
- ✅ Worker threads
- ✅ Statistics collection
- ✅ Unit tests (17 tests passing)

**Status**: Completed 2025-11-17
**Branch**: feature/phase-6-message-bus
**Deliverable**: Working message bus with comprehensive tests

### Phase 7: DI Container (Week 7) - ✅ COMPLETED
- ✅ DI container implementation
- ✅ Service registry
- ✅ Lifetime management (singleton, transient)
- ✅ Unit tests (22 tests passing)

**Status**: Completed 2025-11-17
**Branch**: feature/phase-7-di-container
**Deliverable**: Dependency injection system with comprehensive tests

### Phase 8: Messaging Patterns (Week 8) - ⏳ PENDING
- ⏳ Pub/sub pattern helpers
- ⏳ Request-reply pattern
- ⏳ Event streaming
- ⏳ Message pipeline

**Deliverable**: High-level messaging patterns

### Phase 9-10: Testing and Documentation (Weeks 9-10) - ⏳ PENDING
- ⏳ Unit tests (90%+ coverage)
- ⏳ Integration tests
- ⏳ Performance benchmarks
- ⏳ API documentation
- ⏳ Migration guide
- ⏳ Examples

**Deliverable**: Production-ready system

---

## Success Criteria

### Functional
- [ ] All core messaging patterns work (pub/sub, request/reply)
- [ ] Topic routing with wildcards functional
- [ ] Priority queue working correctly
- [ ] Backend abstraction allows standalone and integrated modes
- [ ] DI container manages all dependencies

### Non-Functional
- [ ] Message throughput > 100k messages/sec
- [ ] Pub/sub latency p99 < 10ms
- [ ] Request/reply latency p99 < 50ms
- [ ] Memory overhead < 1KB per message
- [ ] Zero memory leaks under valgrind

### Quality
- [ ] Unit test coverage > 90%
- [ ] Integration tests pass with all systems
- [ ] No compiler warnings (-Wall -Wextra -Wpedantic)
- [ ] Passes ASAN, TSAN, UBSAN
- [ ] Documentation complete and accurate

### Integration
- [ ] Works with thread_system
- [ ] Works with logger_system
- [ ] Works with monitoring_system
- [ ] Works with container_system
- [ ] Standalone mode functional

---

## Risk Assessment

### High Risk
| Risk | Mitigation |
|------|------------|
| Performance degradation vs current impl | Early benchmarking, profiling |
| Breaking changes in dependent systems | Version pinning, compatibility layer |
| Lock contention in router | Lock-free structures, RCU patterns |

### Medium Risk
| Risk | Mitigation |
|------|------------|
| Complexity of pattern matching | Thorough testing, regex caching |
| Memory overhead of message metadata | Pooling, small string optimization |
| Integration test flakiness | Retry logic, timeouts, isolation |

### Low Risk
| Risk | Mitigation |
|------|------------|
| Documentation drift | Auto-generated docs, code comments |
| Example code outdated | CI checks for examples |

---

## Appendix

### A. Error Code Mapping

| Old Code | New Code | Category |
|----------|----------|----------|
| -200 (INVALID_MESSAGE) | -700 | Message errors |
| -201 (ROUTING_FAILED) | -720 | Routing errors |
| -205 (QUEUE_FULL) | -740 | Queue errors |
| -209 (SUBSCRIPTION_FAILED) | -760 | Subscription errors |
| -210 (PUBLICATION_FAILED) | -780 | Publishing errors |

### B. File Relocation Map

| Old Location | New Location |
|--------------|--------------|
| `include/messaging_system/core/` | `include/kcenon/messaging/core/` |
| `include/messaging_system/error_codes.h` | `include/kcenon/messaging/core/error_codes.h` (forward) |
| `application_layer/include/kcenon/messaging/` | Merged into `include/kcenon/messaging/` |

### C. Dependencies

**Required**:
- common_system >= 2.0
- container_system >= 2.0

**Optional**:
- thread_system >= 2.0 (for integration backend)
- logger_system >= 2.0 (for logging)
- monitoring_system >= 2.0 (for metrics)

### D. References

- [logger_system Architecture](../../logger_system/docs/ARCHITECTURE.md)
- [thread_system Documentation](../../thread_system/docs/README.md)
- [common_system Error Codes](../../common_system/include/kcenon/common/error/error_codes.h)
- [Design Patterns](./DESIGN_PATTERNS.md)

---

**Document Control**

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-11-16 | Claude | Initial draft |

**Approval**

- [ ] Architecture Review
- [ ] Security Review
- [ ] Performance Review
- [ ] Implementation Team Sign-off
