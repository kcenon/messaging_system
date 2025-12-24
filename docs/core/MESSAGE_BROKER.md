# Message Broker

**Version**: 0.1.0
**Last Updated**: 2025-12-24
**Language**: [English]

---

## Overview

The `message_broker` is a central message routing component that provides advanced routing capabilities for the messaging system. It builds upon the `topic_router` component to offer a higher-level abstraction for managing message routes with priority ordering, statistics collection, and lifecycle management.

---

## Table of Contents

1. [Features](#features)
2. [Architecture](#architecture)
3. [Quick Start](#quick-start)
4. [Configuration](#configuration)
5. [Route Management](#route-management)
6. [Message Routing](#message-routing)
7. [Statistics](#statistics)
8. [Best Practices](#best-practices)
9. [Migration from topic_router](#migration-from-topic_router)
10. [Planned Features](#planned-features)

---

## Features

### Implemented

| Feature | Description | Status |
|---------|-------------|--------|
| Route Management | Add, remove, enable, disable routes | Implemented |
| Topic Pattern Matching | MQTT-style wildcards (`*`, `#`) | Implemented |
| Priority-based Ordering | Routes processed by priority (higher first) | Implemented |
| Statistics Collection | Track routed, delivered, failed messages | Implemented |
| Thread-safe Operations | Concurrent access with shared_mutex | Implemented |
| PIMPL Pattern | ABI stability and compile-time isolation | Implemented |

### Planned

| Feature | Description | Issue |
|---------|-------------|-------|
| Content-based Routing | Route based on message content/headers | #181 |
| Dead Letter Queue | Handle unroutable/failed messages | #182 |
| Transformation Pipeline | Transform messages during routing | #183 |

---

## Architecture

### Component Diagram

```
                    ┌─────────────────────┐
                    │   message_broker    │
                    │                     │
                    │  ┌───────────────┐  │
                    │  │ Route Manager │  │
                    │  └───────────────┘  │
                    │         │           │
                    │         ▼           │
                    │  ┌───────────────┐  │
                    │  │ topic_router  │  │
                    │  │  (wildcards)  │  │
                    │  └───────────────┘  │
                    │         │           │
                    │         ▼           │
                    │  ┌───────────────┐  │
                    │  │  Statistics   │  │
                    │  └───────────────┘  │
                    └─────────────────────┘
```

### Class Structure

```cpp
// Main class uses PIMPL pattern
class message_broker {
public:
    explicit message_broker(broker_config config = {});
    ~message_broker();

    // Lifecycle
    common::VoidResult start();
    common::VoidResult stop();
    bool is_running() const;

    // Route Management
    common::VoidResult add_route(...);
    common::VoidResult remove_route(...);
    common::VoidResult enable_route(...);
    common::VoidResult disable_route(...);

    // Routing
    common::VoidResult route(const message& msg);

    // Statistics
    broker_statistics get_statistics() const;

private:
    std::unique_ptr<message_broker_impl> impl_;
};
```

---

## Quick Start

### Basic Usage

```cpp
#include <kcenon/messaging/core/message_broker.h>
using namespace kcenon::messaging;

int main() {
    // Create broker with default configuration
    message_broker broker;

    // Start the broker
    auto start_result = broker.start();
    if (!start_result.is_ok()) {
        std::cerr << "Failed to start broker" << std::endl;
        return 1;
    }

    // Add a route for user events
    broker.add_route("user-handler", "user.*", [](const message& msg) {
        std::cout << "Received: " << msg.metadata().topic << std::endl;
        return common::ok();
    });

    // Route a message
    message msg("user.created");
    broker.route(msg);

    // Stop the broker
    broker.stop();
    return 0;
}
```

### With Custom Configuration

```cpp
broker_config config;
config.max_routes = 500;
config.enable_statistics = true;
config.enable_trace_logging = false;

message_broker broker(config);
broker.start();
```

---

## Configuration

### broker_config

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `max_routes` | `size_t` | 1000 | Maximum number of routes allowed |
| `enable_statistics` | `bool` | true | Enable runtime statistics collection |
| `enable_trace_logging` | `bool` | false | Enable detailed trace logging |
| `default_timeout` | `chrono::milliseconds` | 0 | Default timeout for operations (0 = no timeout) |

### Example Configuration

```cpp
broker_config config;
config.max_routes = 100;              // Limit to 100 routes
config.enable_statistics = true;       // Track statistics
config.enable_trace_logging = false;   // Disable verbose logging
config.default_timeout = std::chrono::milliseconds(5000);  // 5 second timeout

message_broker broker(config);
```

---

## Route Management

### Adding Routes

```cpp
// Basic route
broker.add_route("order-handler", "order.*", order_handler);

// Route with priority (0-10, higher = processed first)
broker.add_route("audit-handler", "order.#", audit_handler, 10);  // High priority
broker.add_route("analytics", "order.#", analytics_handler, 1);   // Low priority
```

### Removing Routes

```cpp
// Remove a specific route
auto result = broker.remove_route("order-handler");
if (!result.is_ok()) {
    std::cerr << "Route not found" << std::endl;
}

// Clear all routes
broker.clear_routes();
```

### Enabling/Disabling Routes

```cpp
// Temporarily disable a route
broker.disable_route("analytics");

// Re-enable the route
broker.enable_route("analytics");
```

### Querying Routes

```cpp
// Check if route exists
if (broker.has_route("order-handler")) {
    // Route exists
}

// Get route information
auto route_result = broker.get_route("order-handler");
if (route_result.is_ok()) {
    auto info = route_result.value();
    std::cout << "Route ID: " << info.route_id << std::endl;
    std::cout << "Pattern: " << info.topic_pattern << std::endl;
    std::cout << "Priority: " << info.priority << std::endl;
    std::cout << "Active: " << info.active << std::endl;
    std::cout << "Processed: " << info.messages_processed << std::endl;
}

// Get all routes
auto routes = broker.get_routes();
for (const auto& route : routes) {
    std::cout << route.route_id << ": " << route.topic_pattern << std::endl;
}

// Get route count
std::cout << "Total routes: " << broker.route_count() << std::endl;
```

---

## Message Routing

### How Routing Works

1. Message arrives with a topic
2. Broker matches topic against all active routes
3. Matching routes are sorted by priority (highest first)
4. Message is delivered to each matching handler in order
5. Statistics are updated

### Topic Pattern Matching

The broker uses MQTT-style wildcard patterns:

| Pattern | Description | Example Match |
|---------|-------------|---------------|
| `user.created` | Exact match | `user.created` only |
| `user.*` | Single-level wildcard | `user.created`, `user.updated` |
| `user.#` | Multi-level wildcard | `user.created`, `user.profile.updated` |
| `*.user.#` | Combined wildcards | `app.user.settings.theme` |

### Routing Example

```cpp
// Add routes with different patterns
broker.add_route("specific", "order.created", handler1);      // Exact
broker.add_route("category", "order.*", handler2);            // Single-level
broker.add_route("all-orders", "order.#", handler3, 10);      // Multi-level, high priority

// Route a message
message msg("order.created");
broker.route(msg);  // Matches all three routes

// Execution order (by priority):
// 1. all-orders (priority 10)
// 2. specific (priority 5, added first)
// 3. category (priority 5, added second)
```

### Error Handling

```cpp
auto result = broker.route(msg);
if (!result.is_ok()) {
    auto error = result.error();
    switch (error.code) {
        case error_code::broker_not_running:
            std::cerr << "Broker is not running" << std::endl;
            break;
        case error_code::no_matching_routes:
            std::cerr << "No routes matched topic" << std::endl;
            break;
        default:
            std::cerr << "Routing failed: " << error.message << std::endl;
    }
}
```

---

## Statistics

### Available Statistics

```cpp
struct broker_statistics {
    uint64_t messages_routed;     // Total routing attempts
    uint64_t messages_delivered;  // Successfully delivered
    uint64_t messages_failed;     // Handler returned error
    uint64_t messages_unrouted;   // No matching routes
    uint64_t active_routes;       // Current active routes
    std::chrono::steady_clock::time_point last_reset;
};
```

### Using Statistics

```cpp
auto stats = broker.get_statistics();

std::cout << "Messages routed: " << stats.messages_routed << std::endl;
std::cout << "Delivered: " << stats.messages_delivered << std::endl;
std::cout << "Failed: " << stats.messages_failed << std::endl;
std::cout << "Unrouted: " << stats.messages_unrouted << std::endl;
std::cout << "Active routes: " << stats.active_routes << std::endl;

// Calculate success rate
if (stats.messages_routed > 0) {
    double success_rate = 100.0 * stats.messages_delivered / stats.messages_routed;
    std::cout << "Success rate: " << success_rate << "%" << std::endl;
}

// Reset statistics
broker.reset_statistics();
```

---

## Best Practices

### 1. Use Meaningful Route IDs

```cpp
// Good: Descriptive and unique
broker.add_route("order-notification-handler", "order.created", handler);
broker.add_route("user-audit-logger", "user.#", logger);

// Bad: Generic or duplicate-prone
broker.add_route("handler1", "order.created", handler);
broker.add_route("handler", "user.#", logger);
```

### 2. Set Appropriate Priorities

```cpp
// Critical handlers first
broker.add_route("validation", "order.*", validate, 10);
broker.add_route("processing", "order.*", process, 5);
broker.add_route("analytics", "order.*", track, 1);
```

### 3. Handle Errors in Handlers

```cpp
broker.add_route("safe-handler", "data.*", [](const message& msg) {
    try {
        process(msg);
        return common::ok();
    } catch (const std::exception& e) {
        return common::error(error_code::handler_failed, e.what());
    }
});
```

### 4. Monitor Statistics

```cpp
// Periodic health check
void check_broker_health(const message_broker& broker) {
    auto stats = broker.get_statistics();

    if (stats.messages_unrouted > 100) {
        log_warning("High number of unrouted messages");
    }

    if (stats.messages_failed > stats.messages_delivered * 0.1) {
        log_error("More than 10% of messages failing");
    }
}
```

### 5. Graceful Shutdown

```cpp
// Stop accepting new routes
// Wait for pending routes to complete
broker.stop();

// Then clear routes if needed
broker.clear_routes();
```

---

## Migration from topic_router

If you're currently using `topic_router` directly, here's how to migrate to `message_broker`:

### Before (topic_router)

```cpp
topic_router router;
router.add_pattern("user.*", [](const message& msg) {
    // Handle
});
router.route(msg);
```

### After (message_broker)

```cpp
message_broker broker;
broker.start();

broker.add_route("user-handler", "user.*", [](const message& msg) {
    // Handle
    return common::ok();  // Note: Return Result now
});

broker.route(msg);
broker.stop();
```

### Key Differences

| Aspect | topic_router | message_broker |
|--------|--------------|----------------|
| Lifecycle | None | start()/stop() required |
| Return type | void | common::VoidResult |
| Route ID | Not required | Required |
| Priority | Not supported | Supported |
| Statistics | Not available | Available |
| Enable/Disable | Not supported | Supported |

---

## Planned Features

The following features are planned for future releases:

### Content-based Routing (#181)

Route messages based on payload content or headers.

```cpp
// Planned API
broker.add_content_route("large-orders", "order.*",
    [](const message& msg) {
        return msg.payload().get<int>("amount") > 1000;
    },
    large_order_handler);
```

### Dead Letter Queue (#182)

Handle messages that fail routing or delivery.

```cpp
// Planned API
broker.set_dead_letter_handler([](const message& msg, const error& err) {
    log_failed_message(msg, err);
});
```

### Transformation Pipeline (#183)

Transform messages during routing.

```cpp
// Planned API
broker.add_route("enriched", "order.*", handler)
    .transform([](message& msg) {
        msg.add_header("processed_at", timestamp());
        return msg;
    });
```

---

## See Also

- [API Reference](../API_REFERENCE.md#message-broker-api)
- [Topic Router](../API_REFERENCE.md#topic-routing)
- [Message Bus](../API_REFERENCE.md#message-bus-api)
- [FEATURES](../FEATURES.md#message-broker)

---

**Last Updated**: 2025-12-24
**Version**: 0.1.0
