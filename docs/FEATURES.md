# Messaging System Features

**Version**: 1.0
**Last Updated**: 2025-11-18
**Language**: [English] | [한국어](FEATURES_KO.md)

---

## Overview

This document provides a comprehensive overview of all features available in the messaging_system, including core messaging capabilities, advanced patterns, integration options, and reliability features.

---

## Table of Contents

1. [Core Messaging](#core-messaging)
2. [Messaging Patterns](#messaging-patterns)
3. [Backend Support](#backend-support)
4. [Message Types](#message-types)
5. [Topic Routing](#topic-routing)
6. [Message Queue](#message-queue)
7. [Dependency Injection](#dependency-injection)
8. [Error Handling](#error-handling)
9. [Integration](#integration)
10. [Production Features](#production-features)

---

## Core Messaging

### Message Bus

**Central pub/sub coordinator for all messaging operations**

Features:
- **Async/Sync Publishing**: Choose between async (fire-and-forget) or sync (wait for delivery)
- **Topic-based Routing**: Hierarchical topic structure with wildcard support
- **Priority Queues**: Optional priority-based message ordering
- **Worker Threads**: Configurable number of worker threads for message dispatch
- **Statistics**: Built-in metrics for messages published, processed, failed, dropped

```cpp
auto bus = std::make_shared<message_bus>(backend, config);
bus->start();

// Async publish
bus->publish(msg);

// Sync subscribe
bus->subscribe("user.*", [](const message& msg) {
    // Handle message
    return common::VoidResult::ok();
});
```

### Message Broker

**Advanced routing and filtering capabilities**

Features:
- **Content-based Routing**: Route messages based on content, not just topic
- **Message Filtering**: Apply filters before delivery
- **Transformation Pipeline**: Transform messages during routing
- **Dead Letter Queue**: Automatic handling of failed messages
- **Retry Policies**: Configurable retry strategies

---

## Messaging Patterns

### Pub/Sub Pattern

**Classic publish-subscribe messaging**

Features:
- **Publisher Helper**: Simplified publishing API
- **Subscriber Helper**: Automatic subscription management
- **Topic Patterns**: Subscribe to multiple topics with wildcards
- **Filter Functions**: Client-side message filtering

```cpp
auto publisher = std::make_shared<patterns::publisher>(bus, "events");
publisher->publish(msg);

auto subscriber = std::make_shared<patterns::subscriber>(bus);
subscriber->subscribe("events.*", callback);
```

### Request/Reply Pattern

**Synchronous RPC over async messaging**

Features:
- **Correlation IDs**: Automatic request/reply matching
- **Timeout Support**: Configurable request timeouts
- **Error Propagation**: Proper error handling
- **Server-side Registration**: Register request handlers
- **Client-side Interface**: Simple request/response API

```cpp
// Server
auto server = std::make_shared<request_server>(bus, "calculator");
server->register_handler(handler);

// Client
auto client = std::make_shared<request_client>(bus);
auto response = client->request(request, timeout);
```

### Event Streaming

**Event sourcing with replay capability**

Features:
- **Event Store**: In-memory event storage
- **Replay Support**: Replay events from specific timestamp
- **Batch Processing**: Process events in batches
- **Event Filtering**: Filter events during replay
- **Stream Snapshots**: Create point-in-time snapshots

```cpp
auto stream = std::make_shared<event_stream>(bus, "orders");

// Publish events
stream->publish_event("order.created", data);

// Replay from timestamp
stream->replay_from(timestamp, callback);
```

### Message Pipeline

**Pipes-and-filters message processing**

Features:
- **Stage Chaining**: Chain multiple processing stages
- **Error Handling**: Per-stage error handling
- **Pipeline Builder**: Fluent API for pipeline construction
- **Stage Reusability**: Share stages across pipelines
- **Conditional Routing**: Route messages based on conditions

```cpp
auto pipeline = pipeline_builder()
    .add_stage("validate", validate_fn)
    .add_stage("transform", transform_fn)
    .add_stage("enrich", enrich_fn)
    .build();

auto result = pipeline.process(msg);
```

---

## Backend Support

### Backend Interface

**Pluggable execution backends**

Features:
- **Abstraction Layer**: Common interface for all backends
- **Executor Access**: Get executor for async operations
- **Logger Access**: Optional logger integration
- **Monitoring Access**: Optional monitoring integration
- **Lifecycle Management**: Initialize and shutdown

### Standalone Backend

**Self-contained execution**

Features:
- **Internal Thread Pool**: Built-in thread pool
- **No External Dependencies**: Works without other systems
- **Simple Configuration**: Easy to set up
- **Testing Support**: Ideal for unit tests

```cpp
auto backend = std::make_shared<standalone_backend>(num_threads);
backend->initialize();
```

### Integration Backend

**External thread pool integration**

Features:
- **Thread System Integration**: Use external thread pools
- **Logger Integration**: Automatic logging
- **Monitoring Integration**: Metrics collection
- **Shared Resources**: Share thread pools across systems

```cpp
auto backend = std::make_shared<integration_backend>(
    thread_pool,
    logger,
    monitoring
);
```

### Auto-Detection

**Automatic backend selection**

Features:
- **Runtime Detection**: Detect available systems at runtime
- **Fallback Support**: Graceful degradation
- **Optimal Selection**: Choose best available backend

---

## Message Types

### Message Structure

**Structured messages with rich metadata**

Fields:
- **ID**: Unique message identifier (UUID)
- **Topic**: Message topic/channel
- **Source**: Source service/component
- **Target**: Target service/component (optional)
- **Correlation ID**: For request/reply correlation
- **Trace ID**: Distributed tracing identifier
- **Type**: Message type (command, event, query, reply, notification)
- **Priority**: Message priority (lowest to critical)
- **Timestamp**: Creation timestamp
- **TTL**: Time-to-live (optional)
- **Headers**: Additional key-value headers
- **Payload**: Message payload (container_system)

### Message Builder

**Fluent API for message construction**

Features:
- **Builder Pattern**: Intuitive message construction
- **Validation**: Automatic validation
- **Defaults**: Sensible default values
- **Type Safety**: Compile-time type checking

```cpp
auto msg = message_builder()
    .topic("user.created")
    .source("auth-service")
    .target("notification-service")
    .type(message_type::event)
    .priority(message_priority::high)
    .ttl(std::chrono::seconds(30))
    .correlation_id(correlation_id)
    .trace_id(trace_id)
    .header("version", "1.0")
    .payload(payload)
    .build();
```

### Message Serialization

**Container-based payload serialization**

Features:
- **Binary Serialization**: Efficient wire format
- **JSON Support**: Human-readable format
- **Type Safety**: Type-safe value storage
- **Nested Structures**: Support for complex data
- **Zero-Copy**: Move semantics support

---

## Topic Routing

### Wildcard Patterns

**Flexible topic matching**

Patterns:
- **Exact Match**: `user.created` matches only `user.created`
- **Single-level Wildcard** (`*`): `user.*` matches `user.created`, `user.updated`
- **Multi-level Wildcard** (`#`): `user.#` matches `user.created`, `user.profile.updated`
- **Combined**: `*.user.#` matches `app.user.profile.updated`

### Subscription Management

**Flexible subscription control**

Features:
- **Dynamic Subscriptions**: Add/remove subscriptions at runtime
- **Subscription ID**: Unique identifier for each subscription
- **Priority**: Subscription priority for execution order
- **Filters**: Message filters for fine-grained control
- **Multiple Subscribers**: Multiple subscribers per topic

```cpp
// Subscribe
auto sub_id = bus->subscribe("user.*", callback, filter, priority);

// Unsubscribe
bus->unsubscribe(sub_id.value());
```

---

## Message Queue

### Queue Types

**Multiple queue implementations**

Types:
- **Standard Queue**: FIFO queue with mutex protection
- **Priority Queue**: Priority-based ordering
- **Dead Letter Queue**: Failed message handling
- **Bounded Queue**: Capacity limits with backpressure

### Queue Configuration

**Flexible queue settings**

Options:
- **Max Size**: Maximum queue capacity (default: 10,000)
- **Enable Priority**: Use priority queue (default: false)
- **Enable Persistence**: Enable message persistence (default: false)
- **Drop on Full**: Drop oldest on full vs reject (default: false)

```cpp
queue_config config;
config.max_size = 10000;
config.enable_priority = true;
config.drop_on_full = false;

auto queue = std::make_shared<message_queue>(config);
```

---

## Dependency Injection

### DI Container

**Lightweight dependency injection**

Features:
- **Service Registration**: Register services by type
- **Service Resolution**: Resolve dependencies
- **Lifetime Management**: Singleton support
- **Type Safety**: Compile-time type checking
- **Global Container**: Optional global container

```cpp
auto container = std::make_shared<messaging_di_container>();

// Register
container->register_service<IExecutor>(executor);

// Resolve
auto executor = container->resolve<IExecutor>();
```

---

## Error Handling

### Error Codes

**Centralized error code system (-700 to -799)**

Categories:
- **Message Errors** (-700 to -719): Invalid message, message too large, expired, invalid payload
- **Routing Errors** (-720 to -739): Routing failed, unknown topic, no subscribers, invalid pattern
- **Queue Errors** (-740 to -759): Queue full, queue empty, queue stopped, enqueue/dequeue failed
- **Subscription Errors** (-760 to -779): Subscription failed, not found, duplicate, unsubscribe failed
- **Publishing Errors** (-780 to -799): Publication failed, no route found, message rejected, broker unavailable

### Result<T> Pattern

**Type-safe error handling**

Features:
- **No Exceptions**: Exception-free error handling
- **Composable**: Chain operations
- **Explicit**: Errors must be handled
- **Type Safe**: Compile-time error checking

```cpp
auto result = bus->subscribe("topic", callback);
if (!result.is_ok()) {
    std::cerr << "Error: " << result.error().message << std::endl;
    return;
}

auto sub_id = result.value();
```

---

## Integration

### Thread System Integration

**High-performance thread pool integration**

Features:
- **Lock-free Queues**: Optional lock-free queue support
- **Priority Scheduling**: Priority-based job scheduling
- **Work Stealing**: Efficient load balancing
- **Hazard Pointers**: Safe memory reclamation

### Logger System Integration

**Structured async logging**

Features:
- **Async Logging**: Non-blocking log operations
- **Structured Logs**: JSON/structured output
- **Multiple Sinks**: File, console, network
- **Log Levels**: Configurable log levels

### Monitoring System Integration

**Real-time metrics and telemetry**

Features:
- **Counters**: Message counts
- **Histograms**: Latency distributions
- **Gauges**: Queue sizes
- **Prometheus Export**: Metrics export

### Container System Integration

**Type-safe message payloads**

Features:
- **Variant Storage**: Multiple value types
- **SIMD Optimization**: Fast operations
- **Binary Serialization**: Efficient wire format
- **Thread Safety**: Safe concurrent access

---

## Production Features

### Reliability

**Enterprise-grade reliability features**

Features:
- **Message Acknowledgment**: Confirm message delivery
- **Dead Letter Queue**: Handle failed messages
- **Retry Policies**: Automatic retry with backoff
- **Circuit Breakers**: Prevent cascade failures
- **Health Checks**: Monitor system health

### Observability

**Production observability**

Features:
- **Distributed Tracing**: Trace messages across services
- **Performance Metrics**: Throughput, latency, errors
- **Queue Metrics**: Size, drops, overflows
- **Subscriber Metrics**: Active subscribers, callback latency

### Testing Support

**Comprehensive testing utilities**

Features:
- **Mock Backend**: Testing without threading
- **Test Fixtures**: Reusable test setup
- **Test Helpers**: Assertion helpers
- **Integration Tests**: End-to-end test scenarios

---

## Feature Matrix

| Feature | Core | Patterns | Backend | Status |
|---------|------|----------|---------|--------|
| **Message Bus** | ✅ | - | - | Complete |
| **Topic Router** | ✅ | - | - | Complete |
| **Message Queue** | ✅ | - | - | Complete |
| **Pub/Sub** | - | ✅ | - | Complete |
| **Request/Reply** | - | ✅ | - | Complete |
| **Event Streaming** | - | ✅ | - | Complete |
| **Message Pipeline** | - | ✅ | - | Complete |
| **Standalone Backend** | - | - | ✅ | Complete |
| **Integration Backend** | - | - | ✅ | Complete |
| **Auto-detection** | - | - | ✅ | Complete |
| **DI Container** | ✅ | - | - | Complete |
| **Error Codes** | ✅ | - | - | Complete |
| **Result<T>** | ✅ | - | - | Complete |
| **Serialization** | ✅ | - | - | Complete |
| **Wildcards** | ✅ | - | - | Complete |
| **Priority Queue** | ✅ | - | - | Complete |
| **Dead Letter Queue** | ✅ | - | - | Complete |
| **Tracing** | ✅ | - | - | Complete |
| **Metrics** | ✅ | - | - | Complete |

---

## Getting Started

For usage examples and getting started guides, see:
- [Quick Start Guide](guides/QUICK_START.md)
- [Pattern Examples](PATTERNS_API.md)
- [API Reference](API_REFERENCE.md)
- [Integration Guide](guides/INTEGRATION.md)

---

**Last Updated**: 2025-11-18
**Version**: 1.0
