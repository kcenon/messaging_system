# Migration Guide

**Version**: 0.2.0
**Date**: 2025-11-17
**Target Audience**: Developers migrating from messaging_system v1.x to v2.0

## Table of Contents

1. [Overview](#overview)
2. [Breaking Changes](#breaking-changes)
3. [Step-by-Step Migration](#step-by-step-migration)
4. [API Changes](#api-changes)
5. [Code Examples](#code-examples)
6. [Troubleshooting](#troubleshooting)
7. [FAQ](#faq)

---

## Overview

Messaging System v2.0 introduces significant architectural improvements and new features:

- **Messaging Patterns**: New high-level patterns (Pub/Sub, Request-Reply, Event Streaming, Message Pipeline)
- **Dependency Injection**: Built-in DI container for better modularity
- **Backend Abstraction**: Support for standalone and integrated backends
- **Improved Error Handling**: Consistent use of `common::Result<T>` pattern
- **Enhanced Performance**: Lock-free queue options and optimized routing

### Migration Timeline

- **Phase 1 (Week 1-2)**: Update dependencies and error codes
- **Phase 2 (Week 3-4)**: Migrate core message types and queues
- **Phase 3 (Week 5-6)**: Adopt new messaging patterns
- **Phase 4 (Week 7-8)**: Testing and validation
- **Phase 5 (Week 9-10)**: Production deployment

### Compatibility

- **v2.0** is **not** fully backward compatible with v1.x
- A compatibility layer is provided for gradual migration
- Both versions can coexist during migration period

---

## Breaking Changes

### 1. Namespace Changes

**v1.x**:
```cpp
#include <messaging_system/core/message_bus.h>
using namespace messaging;
```

**v2.0**:
```cpp
#include <kcenon/messaging/core/message_bus.h>
using namespace kcenon::messaging;
```

### 2. Error Code Range

Error codes have been moved from the `-200 to -299` range to `-700 to -799` to avoid conflicts with logger_system.

**v1.x**:
```cpp
constexpr int INVALID_MESSAGE = -200;
constexpr int ROUTING_FAILED = -201;
```

**v2.0**:
```cpp
// Use centralized error codes
using namespace kcenon::common::error::codes::messaging_system;
constexpr int invalid_message = -700;
constexpr int routing_failed = -720;
```

### 3. Message Structure

**v1.x**: Messages used a simple struct-based approach.

**v2.0**: Messages use a rich object model with metadata and builder pattern.

**v1.x**:
```cpp
struct Message {
    std::string topic;
    std::string payload;
    int priority;
};
```

**v2.0**:
```cpp
message msg = message_builder()
    .topic("events.user")
    .type(message_type::event)
    .priority(message_priority::high)
    .payload(container_ptr)
    .build()
    .unwrap();
```

### 4. Message Bus Interface

**v1.x**:
```cpp
message_bus bus;
bus.publish(topic, payload);
bus.subscribe(topic, callback);
```

**v2.0**:
```cpp
auto backend = std::make_shared<standalone_backend>();
auto bus = std::make_shared<message_bus>(backend);
bus->start();

bus->publish(message);
bus->subscribe(topic_pattern, callback);
```

### 5. Topic Router

**v1.x**: Basic exact-match routing.

**v2.0**: Pattern-based routing with wildcards.

**v1.x**:
```cpp
router.subscribe("user.created", callback);
```

**v2.0**:
```cpp
// Exact match
router.subscribe("user.created", callback);

// Single-level wildcard
router.subscribe("user.*", callback);

// Multi-level wildcard
router.subscribe("user.#", callback);
```

---

## Step-by-Step Migration

### Step 1: Update Dependencies

#### Update CMakeLists.txt

```cmake
# OLD
find_package(messaging_system 1.0 REQUIRED)
target_link_libraries(your_target messaging_system::core)

# NEW
find_package(messaging_system 2.0 REQUIRED)
target_link_libraries(your_target
    messaging_system::messaging_system_core
    kcenon::common_system
)
```

#### Update Include Paths

```cmake
# Add to include directories
target_include_directories(your_target PRIVATE
    ${messaging_system_INCLUDE_DIRS}
)
```

### Step 2: Update Includes

Create a migration header to ease the transition:

**migration_compat.h**:
```cpp
#pragma once

// Backward compatibility aliases
#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/topic_router.h>

namespace messaging {
    using namespace kcenon::messaging;

    // Type aliases for compatibility
    using Message = kcenon::messaging::message;
    using MessageBus = kcenon::messaging::message_bus;
    using TopicRouter = kcenon::messaging::topic_router;
}

// Error code compatibility
namespace messaging::error {
    constexpr int INVALID_MESSAGE = -700;
    constexpr int ROUTING_FAILED = -720;
    constexpr int QUEUE_FULL = -740;
    // ... other mappings
}
```

### Step 3: Migrate Message Creation

**Before (v1.x)**:
```cpp
Message msg;
msg.topic = "user.created";
msg.payload = user_data;
msg.priority = 1;
```

**After (v2.0)**:
```cpp
// Option 1: Direct construction
message msg("user.created", message_type::event);
msg.metadata().priority = message_priority::high;
msg.metadata().source = "user-service";

// Option 2: Builder pattern (recommended)
auto msg_result = message_builder()
    .topic("user.created")
    .type(message_type::event)
    .priority(message_priority::high)
    .source("user-service")
    .payload(user_data_container)
    .build();

if (msg_result.is_ok()) {
    auto msg = msg_result.unwrap();
    // Use message
}
```

### Step 4: Migrate Message Bus Usage

**Before (v1.x)**:
```cpp
// Simple standalone bus
messaging::MessageBus bus;

// Publish
bus.publish("user.created", data);

// Subscribe
bus.subscribe("user.created", [](const std::string& payload) {
    // Handle message
});
```

**After (v2.0)**:
```cpp
// Create backend
auto backend = std::make_shared<standalone_backend>(4);  // 4 worker threads

// Create message bus
message_bus_config config;
config.worker_threads = 4;
config.queue_capacity = 10000;
auto bus = std::make_shared<message_bus>(backend, config);

// Start bus
auto start_result = bus->start();
if (!start_result.is_ok()) {
    // Handle error
}

// Publish
message msg("user.created", message_type::event);
auto pub_result = bus->publish(std::move(msg));
if (!pub_result.is_ok()) {
    // Handle error
}

// Subscribe
auto sub_result = bus->subscribe("user.*", [](const message& msg) -> common::VoidResult {
    // Handle message
    return common::ok();
});

if (!sub_result.is_ok()) {
    // Handle error
}

// Cleanup
bus->stop();
```

### Step 5: Migrate to Patterns (Optional but Recommended)

#### Pub/Sub Pattern

**Before (v1.x)**:
```cpp
bus.subscribe("events.*", handler);
bus.publish("events.user_login", data);
```

**After (v2.0 with Patterns)**:
```cpp
#include <kcenon/messaging/patterns/pub_sub.h>

// Publisher
publisher pub(bus, "events.user");
pub.publish(msg);

// Subscriber
subscriber sub(bus);
sub.subscribe("events.*", [](const message& msg) {
    // Handle event
    return common::ok();
});
```

#### Request-Reply Pattern

**Before (v1.x)**: Manual correlation tracking required.

**After (v2.0)**:
```cpp
#include <kcenon/messaging/patterns/request_reply.h>

// Client
request_client client(bus);
auto reply_result = client.request("service.compute", request_msg);

// Server
request_server server(bus, "service.compute");
server.register_handler([](const message& req) -> common::Result<message> {
    message reply("reply", message_type::reply);
    // Process and build reply
    return common::ok(std::move(reply));
});
server.start();
```

### Step 6: Update Error Handling

**Before (v1.x)**:
```cpp
try {
    bus.publish(topic, data);
} catch (const messaging::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

**After (v2.0)**:
```cpp
auto result = bus->publish(msg);
if (result.is_err()) {
    auto error = result.get_error();
    std::cerr << "Error: " << error.message
              << " (code: " << error.code << ")" << std::endl;

    // Handle specific errors
    if (error.code == messaging_system::queue_full) {
        // Retry or backoff
    }
}
```

### Step 7: Integration with Other Systems

If you're using thread_system, logger_system, or monitoring_system:

```cpp
#include <kcenon/messaging/backends/integration_backend.h>

// Get shared services from DI container
auto thread_pool = thread::get_global_thread_pool();
auto logger = logger::get_global_logger();
auto monitor = monitoring::get_global_monitor();

// Create integrated backend
auto backend = std::make_shared<integration_backend>(
    thread_pool,
    logger,
    monitor
);

auto bus = std::make_shared<message_bus>(backend);
bus->start();
```

---

## API Changes

### Message API

| v1.x | v2.0 | Notes |
|------|------|-------|
| `Message` struct | `message` class | Use builder pattern |
| `msg.topic` | `msg.metadata().topic` | Structured metadata |
| `msg.priority` (int) | `msg.metadata().priority` (enum) | Type-safe priority |
| `msg.payload` (string) | `msg.payload()` (container) | Rich payload support |

### Message Bus API

| v1.x | v2.0 | Notes |
|------|------|-------|
| `publish(topic, data)` | `publish(message)` | Result-based error handling |
| `subscribe(topic, callback)` | `subscribe(pattern, callback)` | Returns subscription ID |
| `unsubscribe(topic)` | `unsubscribe(subscription_id)` | ID-based unsubscription |
| - | `start()` / `stop()` | Explicit lifecycle management |

### Topic Router API

| v1.x | v2.0 | Notes |
|------|------|-------|
| Exact match only | Wildcard support | `*` and `#` patterns |
| - | Priority-based routing | Subscription priorities |
| - | Message filters | Filter chain support |

---

## Code Examples

### Example 1: Simple Pub/Sub Migration

**v1.x Code**:
```cpp
#include <messaging_system/message_bus.h>

int main() {
    messaging::MessageBus bus;

    // Subscribe
    bus.subscribe("sensor.temperature", [](const std::string& data) {
        std::cout << "Temperature: " << data << std::endl;
    });

    // Publish
    bus.publish("sensor.temperature", "25.5");

    return 0;
}
```

**v2.0 Code**:
```cpp
#include <kcenon/messaging/patterns/pub_sub.h>
#include <kcenon/messaging/backends/standalone_backend.h>

int main() {
    // Setup
    auto backend = std::make_shared<kcenon::messaging::standalone_backend>(2);
    auto bus = std::make_shared<kcenon::messaging::message_bus>(backend);
    bus->start();

    // Subscribe
    kcenon::messaging::patterns::subscriber sub(bus);
    sub.subscribe("sensor.*", [](const kcenon::messaging::message& msg) {
        std::cout << "Sensor data: " << msg.metadata().topic << std::endl;
        return kcenon::common::ok();
    });

    // Publish
    kcenon::messaging::patterns::publisher pub(bus, "sensor.temperature");
    kcenon::messaging::message msg("sensor.temperature",
        kcenon::messaging::message_type::event);
    pub.publish(std::move(msg));

    // Cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bus->stop();

    return 0;
}
```

### Example 2: Request-Reply Migration

**v1.x Code**: Manual correlation

**v2.0 Code**:
```cpp
#include <kcenon/messaging/patterns/request_reply.h>

// Client
void make_request(std::shared_ptr<kcenon::messaging::message_bus> bus) {
    using namespace kcenon::messaging;
    using namespace kcenon::messaging::patterns;

    request_client client(bus);

    message req("service.data", message_type::query);
    req.metadata().source = "client-app";

    auto reply_result = client.request("service.data", std::move(req),
        std::chrono::seconds{5});

    if (reply_result.is_ok()) {
        auto reply = reply_result.unwrap();
        std::cout << "Got reply!" << std::endl;
    } else {
        std::cerr << "Request failed: " << reply_result.get_error().message << std::endl;
    }
}

// Server
void serve_requests(std::shared_ptr<kcenon::messaging::message_bus> bus) {
    using namespace kcenon::messaging;
    using namespace kcenon::messaging::patterns;

    request_server server(bus, "service.data");

    server.register_handler([](const message& req) -> kcenon::common::Result<message> {
        // Process request
        message reply("reply", message_type::reply);
        reply.metadata().correlation_id = req.metadata().id;
        // ... populate reply data ...
        return kcenon::common::ok(std::move(reply));
    });

    server.start();

    // Keep server running
    std::this_thread::sleep_for(std::chrono::seconds(60));
    server.stop();
}
```

### Example 3: Event Streaming

**v2.0 New Feature**:
```cpp
#include <kcenon/messaging/patterns/event_streaming.h>

void event_sourcing_example(std::shared_ptr<kcenon::messaging::message_bus> bus) {
    using namespace kcenon::messaging;
    using namespace kcenon::messaging::patterns;

    // Configure event stream
    event_stream_config config;
    config.max_buffer_size = 1000;
    config.enable_replay = true;

    event_stream stream(bus, "events.orders", config);

    // Publish events
    for (int i = 0; i < 10; ++i) {
        message event("events.orders", message_type::event);
        event.metadata().source = "order-service";
        stream.publish_event(std::move(event));
    }

    // New subscriber with replay
    stream.subscribe([](const message& event) {
        std::cout << "Event: " << event.metadata().id << std::endl;
        return kcenon::common::ok();
    }, true);  // Enable replay

    // Get event history
    auto events = stream.get_events();
    std::cout << "Total events: " << events.size() << std::endl;
}
```

### Example 4: Message Pipeline

**v2.0 New Feature**:
```cpp
#include <kcenon/messaging/patterns/message_pipeline.h>

void pipeline_example(std::shared_ptr<kcenon::messaging::message_bus> bus) {
    using namespace kcenon::messaging;
    using namespace kcenon::messaging::patterns;

    // Build pipeline
    pipeline_builder builder(bus);

    auto pipeline_result = builder
        .from("input.raw_data")
        .to("output.processed_data")
        .add_filter("valid_only", [](const message& msg) {
            return !msg.metadata().topic.empty();
        })
        .add_stage("validate", [](const message& msg) -> kcenon::common::Result<message> {
            // Validation logic
            return kcenon::common::ok(message(msg));
        })
        .add_transformer("enrich", [](const message& msg) {
            message enriched(msg);
            enriched.metadata().priority = message_priority::high;
            return enriched;
        })
        .build();

    if (pipeline_result.is_ok()) {
        auto pipeline = pipeline_result.unwrap();
        pipeline->start();

        // Pipeline automatically processes messages from input to output
        std::this_thread::sleep_for(std::chrono::seconds(30));

        // Check statistics
        auto stats = pipeline->get_statistics();
        std::cout << "Processed: " << stats.messages_processed << std::endl;

        pipeline->stop();
    }
}
```

---

## Troubleshooting

### Issue: Compilation errors with old includes

**Solution**: Update all includes to use the new namespace:
```cpp
// OLD
#include <messaging_system/core/message_bus.h>

// NEW
#include <kcenon/messaging/core/message_bus.h>
```

### Issue: Error code conflicts

**Symptom**: Build errors when linking with logger_system.

**Solution**: Update error code references to use the new range:
```cpp
// Use centralized error codes
using namespace kcenon::common::error::codes::messaging_system;
```

### Issue: Message bus doesn't start

**Symptom**: `start()` returns an error.

**Solution**: Check backend initialization:
```cpp
auto backend = std::make_shared<standalone_backend>(4);
auto init_result = backend->initialize();
if (!init_result.is_ok()) {
    // Handle backend initialization error
}

auto bus = std::make_shared<message_bus>(backend);
auto start_result = bus->start();
if (!start_result.is_ok()) {
    // Handle bus start error
}
```

### Issue: Messages not being delivered

**Symptom**: Subscribers don't receive messages.

**Solution**: Check topic patterns:
```cpp
// Make sure patterns match
// Publish: "user.created"
// Subscribe: "user.*" or "user.created" (not "users.*")

// Also verify bus is running
if (!bus->is_running()) {
    bus->start();
}
```

### Issue: Build performance degradation

**Symptom**: Increased build times.

**Solution**: Use precompiled headers:
```cmake
target_precompile_headers(your_target PRIVATE
    <kcenon/messaging/core/message.h>
    <kcenon/messaging/core/message_bus.h>
)
```

---

## FAQ

### Q: Can I use v1.x and v2.0 together?

**A**: Yes, during the migration period. Use the compatibility layer:
```cpp
#include <kcenon/messaging/compatibility.h>
```

### Q: Do I need to migrate all code at once?

**A**: No. Migrate incrementally:
1. Start with non-critical components
2. Use compatibility layer for coexistence
3. Migrate high-traffic components last
4. Remove compatibility layer after full migration

### Q: What about performance?

**A**: v2.0 is generally faster due to:
- Lock-free queue options
- Optimized routing
- Better thread pool integration

Benchmark your specific use case.

### Q: Are there any runtime dependencies?

**A**: v2.0 requires:
- common_system >= 2.0
- container_system >= 2.0
- (Optional) thread_system >= 2.0
- (Optional) logger_system >= 2.0
- (Optional) monitoring_system >= 2.0

### Q: How do I enable lock-free queues?

**A**:
```cpp
message_bus_config config;
config.enable_lockfree = true;  // If available
auto bus = std::make_shared<message_bus>(backend, config);
```

### Q: Can I still use simple publish/subscribe?

**A**: Yes, use the Pub/Sub pattern for simplified API:
```cpp
#include <kcenon/messaging/patterns/pub_sub.h>
publisher pub(bus);
subscriber sub(bus);
```

### Q: How do I handle migration testing?

**A**: Use a phased approach:
1. Unit tests first
2. Integration tests with compatibility mode
3. Shadow traffic testing
4. Gradual production rollout

---

## Additional Resources

- [API Reference](./API_REFERENCE.md)
- [Patterns API](./PATTERNS_API.md)
- [Architecture Documentation](./ARCHITECTURE.md)
- [Getting Started Guide](./GETTING_STARTED.md)
- [Troubleshooting Guide](./TROUBLESHOOTING.md)

---

## Support

For migration assistance:
- Check [GitHub Issues](https://github.com/kcenon/messaging_system/issues)
- Review [Examples](../examples/)
- Consult [Integration Tests](../integration_tests/)

---

**Document Version**: 0.1.0
**Last Updated**: 2025-11-17
**Maintainer**: Messaging System Team
