# Messaging System Features

**Version**: 0.1.1
**Last Updated**: 2025-12-10
**Language**: [English] | [한국어](FEATURES_KO.md)

---

## Overview

This document provides a comprehensive overview of all features available in the messaging_system, including core messaging capabilities, advanced patterns, integration options, and reliability features.

---

## Table of Contents

1. [Core Messaging](#core-messaging)
2. [Messaging Patterns](#messaging-patterns)
3. [Task Queue System](#task-queue-system)
4. [C++20 Concepts](#c20-concepts)
5. [Backend Support](#backend-support)
6. [Message Types](#message-types)
7. [Topic Routing](#topic-routing)
8. [Message Queue](#message-queue)
9. [Dependency Injection](#dependency-injection)
10. [Error Handling](#error-handling)
11. [Integration](#integration)
12. [Production Features](#production-features)

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

## Task Queue System

### Overview

**Distributed task queue for background job processing**

The Task Queue System provides a complete solution for distributed task processing with features like priority queues, scheduled execution, retry mechanisms, and real-time monitoring.

### Task System Facade

**Unified interface for all task operations**

Features:
- **Single Entry Point**: Unified facade for task submission and management
- **Component Orchestration**: Coordinates worker pools, schedulers, and monitors
- **Configuration**: Centralized configuration for all components
- **Lifecycle Management**: Start/stop all components together

```cpp
auto system = std::make_shared<task_system>(config);
system->start();

// Submit a task
auto result = system->submit("process.image", payload);

// Wait for result
auto output = result.get();
```

### Task Client

**Task submission with various execution modes**

Features:
- **Immediate Execution**: Submit tasks for immediate processing
- **Delayed Execution**: Schedule tasks for future execution
- **Priority Support**: Submit with different priority levels
- **Async Results**: Get async handles for result tracking

```cpp
auto client = std::make_shared<task_client>(queue, backend);

// Submit with priority
auto result = client->submit_with_priority(task, task_priority::high);

// Submit with delay
auto delayed = client->submit_delayed(task, std::chrono::seconds(30));
```

### Worker Pool

**Configurable worker threads with handler registration**

Features:
- **Thread Pool**: Configurable number of worker threads
- **Handler Registration**: Register handlers by task name
- **Load Balancing**: Automatic work distribution
- **Graceful Shutdown**: Complete pending tasks before shutdown

```cpp
auto pool = std::make_shared<worker_pool>(config);

// Register handlers
pool->register_handler("email.send", email_handler);
pool->register_handler("image.resize", image_handler);

pool->start();
```

### Task Scheduler

**Periodic and cron-based task scheduling**

Features:
- **Periodic Scheduling**: Execute tasks at fixed intervals
- **Cron Expressions**: Standard 5-field cron expression support
- **Schedule Management**: Enable/disable/remove schedules
- **Execution Callbacks**: Hooks for execution events

```cpp
auto scheduler = std::make_shared<scheduler>(client);

// Periodic task (every 5 minutes)
scheduler->schedule_periodic("cleanup", task, std::chrono::minutes(5));

// Cron task (daily at midnight)
scheduler->schedule_cron("daily_report", task, "0 0 * * *");
```

### Async Result

**Async result tracking with progress support**

Features:
- **Status Tracking**: Check task status (pending, running, completed, failed)
- **Progress Updates**: Real-time progress percentage
- **Timeout Support**: Wait with configurable timeout
- **Chaining**: Chain results for workflow orchestration

```cpp
auto result = client->submit(task);

// Check status
if (result.is_pending()) {
    // Still waiting
}

// Wait with timeout
auto output = result.wait_for(std::chrono::seconds(30));

// Get progress
double progress = result.progress();
```

### Result Backend

**Pluggable result storage**

Features:
- **Memory Backend**: In-memory storage for development/testing
- **Interface**: Abstract interface for custom backends
- **TTL Support**: Automatic result expiration
- **Cleanup**: Periodic cleanup of expired results

```cpp
auto backend = std::make_shared<memory_result_backend>();

// Store result
backend->store(task_id, result);

// Retrieve result
auto stored = backend->get(task_id);
```

### Task Monitor

**Real-time task monitoring and statistics**

Features:
- **Queue Statistics**: Pending, running, completed counts
- **Worker Statistics**: Active workers, utilization
- **Performance Metrics**: Throughput, latency
- **Event Callbacks**: Subscribe to task events

```cpp
auto monitor = std::make_shared<monitor>(pool, backend);

// Get statistics
auto stats = monitor->get_statistics();
std::cout << "Pending: " << stats.pending_count << std::endl;
std::cout << "Running: " << stats.running_count << std::endl;

// Subscribe to events
monitor->on_task_completed([](const task& t) {
    std::cout << "Task " << t.id() << " completed" << std::endl;
});
```

### Chain and Chord Patterns

**Workflow orchestration patterns**

Features:
- **Chain**: Sequential task execution
- **Chord**: Parallel execution with final callback
- **Error Handling**: Proper error propagation
- **Result Aggregation**: Combine results from multiple tasks

```cpp
// Chain: task1 -> task2 -> task3
auto chain_result = client->chain({task1, task2, task3});

// Chord: [task1, task2, task3] -> callback
auto chord_result = client->chord({task1, task2, task3}, callback_task);
```

### Retry Mechanism

**Automatic retry with exponential backoff**

Features:
- **Configurable Retries**: Set max retry count per task
- **Exponential Backoff**: Increasing delays between retries
- **Retry Callbacks**: Hook for retry events
- **Final Failure**: Callback when all retries exhausted

```cpp
auto task = task_builder()
    .name("send.email")
    .max_retries(3)
    .retry_delay(std::chrono::seconds(5))
    .build();
```

### Task Timeout

**Timeout handling for long-running tasks**

Features:
- **Per-Task Timeout**: Set timeout per task
- **Cancellation**: Cancel timed-out tasks
- **Timeout Callbacks**: Hook for timeout events

```cpp
auto task = task_builder()
    .name("process.video")
    .timeout(std::chrono::minutes(10))
    .build();
```

---

## C++20 Concepts

### Overview

**Type-safe callback validation with C++20 Concepts**

The messaging system uses C++20 Concepts to provide compile-time type validation for callbacks and handlers. This results in clearer error messages and self-documenting interface requirements.

### TaskHandlerCallable

**Validates task handler signatures**

```cpp
template<typename F>
concept TaskHandlerCallable = std::invocable<F, const task&, task_context&> &&
    std::same_as<std::invoke_result_t<F, const task&, task_context&>,
                 common::Result<container_module::value_container>>;

// Usage
template<TaskHandlerCallable Handler>
void register_handler(const std::string& name, Handler&& handler);
```

### TaskHandlerLike

**Validates task handler interface implementations**

```cpp
template<typename T>
concept TaskHandlerLike = requires(T t, const task& tsk, task_context& ctx) {
    { t.name() } -> std::convertible_to<std::string>;
    { t.execute(tsk, ctx) } -> std::same_as<common::Result<container_module::value_container>>;
};
```

### ScheduleEventCallable

**Validates scheduler event callbacks**

```cpp
template<typename F>
concept ScheduleEventCallable = std::invocable<F, const schedule_entry&>;

// Usage
template<ScheduleEventCallable Callback>
void on_task_executed(Callback&& callback);
```

### MessageProcessorCallable

**Validates message pipeline processors**

```cpp
template<typename F>
concept MessageProcessorCallable = std::invocable<F, const message&> &&
    std::same_as<std::invoke_result_t<F, const message&>,
                 common::Result<message>>;
```

### MessageFilterCallable

**Validates message filtering predicates**

```cpp
template<typename F>
concept MessageFilterCallable = std::invocable<F, const message&> &&
    std::convertible_to<std::invoke_result_t<F, const message&>, bool>;
```

### MessageTransformerCallable

**Validates message transformers**

```cpp
template<typename F>
concept MessageTransformerCallable = std::invocable<F, const message&> &&
    std::same_as<std::invoke_result_t<F, const message&>, message>;
```

### SubscriptionCallable

**Validates topic subscription callbacks**

```cpp
template<typename F>
concept SubscriptionCallable = std::invocable<F, const message&> &&
    std::same_as<std::invoke_result_t<F, const message&>,
                 common::VoidResult>;
```

### Benefits

- **Compile-time Validation**: Catch type mismatches at compile time
- **Clear Error Messages**: Better diagnostics than SFINAE
- **Self-documenting**: Interface requirements are explicit in code
- **IDE Support**: Better autocomplete and type inference

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

| Feature | Core | Patterns | Task | Backend | Status |
|---------|------|----------|------|---------|--------|
| **Message Bus** | ✅ | - | - | - | Complete |
| **Topic Router** | ✅ | - | - | - | Complete |
| **Message Queue** | ✅ | - | - | - | Complete |
| **Pub/Sub** | - | ✅ | - | - | Complete |
| **Request/Reply** | - | ✅ | - | - | Complete |
| **Event Streaming** | - | ✅ | - | - | Complete |
| **Message Pipeline** | - | ✅ | - | - | Complete |
| **Task System (Facade)** | - | - | ✅ | - | Complete |
| **Task Queue** | - | - | ✅ | - | Complete |
| **Worker Pool** | - | - | ✅ | - | Complete |
| **Task Scheduler** | - | - | ✅ | - | Complete |
| **Async Result** | - | - | ✅ | - | Complete |
| **Result Backend** | - | - | ✅ | - | Complete |
| **Task Monitor** | - | - | ✅ | - | Complete |
| **Cron Parser** | - | - | ✅ | - | Complete |
| **Chain/Chord** | - | - | ✅ | - | Complete |
| **Retry Mechanism** | - | - | ✅ | - | Complete |
| **Task Timeout** | - | - | ✅ | - | Complete |
| **C++20 Concepts** | ✅ | ✅ | ✅ | - | Complete |
| **Standalone Backend** | - | - | - | ✅ | Complete |
| **Integration Backend** | - | - | - | ✅ | Complete |
| **Auto-detection** | - | - | - | ✅ | Complete |
| **DI Container** | ✅ | - | - | - | Complete |
| **Error Codes** | ✅ | - | - | - | Complete |
| **Result<T>** | ✅ | - | - | - | Complete |
| **Serialization** | ✅ | - | - | - | Complete |
| **Wildcards** | ✅ | - | - | - | Complete |
| **Priority Queue** | ✅ | - | - | - | Complete |
| **Dead Letter Queue** | ✅ | - | - | - | Complete |
| **Tracing** | ✅ | - | - | - | Complete |
| **Metrics** | ✅ | - | - | - | Complete |

---

## Getting Started

For usage examples and getting started guides, see:
- [Quick Start Guide](guides/QUICK_START.md)
- [Pattern Examples](PATTERNS_API.md)
- [API Reference](API_REFERENCE.md)
- [Integration Guide](guides/INTEGRATION.md)
- [Task System Architecture](task/ARCHITECTURE.md)

---

**Last Updated**: 2025-12-10
**Version**: 0.1.1
