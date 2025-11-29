# Pub/Sub Pattern Example

This example demonstrates the **Publisher-Subscriber** messaging pattern with topic-based routing and message filtering.

## Overview

The Pub/Sub pattern enables loosely-coupled communication where publishers send messages to topics without knowing who will receive them, and subscribers receive messages from topics they're interested in.

## Key Features Demonstrated

- **Topic Subscriptions**: Subscribe to topics using exact matches or wildcards
- **Wildcard Patterns**: Use `*` (single-level) and `#` (multi-level) wildcards
- **Message Filtering**: Apply filters to receive only specific messages
- **messaging_container_builder**: Type-safe message construction
- **Priority Handling**: High-priority message filtering

## Quick Start

```bash
# Build the example
cmake --build build --target example_pub_sub

# Run
./build/examples/patterns/pub_sub/example_pub_sub
```

## Code Highlights

### Creating Publishers and Subscribers

```cpp
// Create subscriber
subscriber sub(bus);

// Subscribe to all user events
sub.subscribe("events.user.*", [](const message& msg) {
    std::cout << "Received: " << msg.metadata().topic << std::endl;
    return common::ok();
});

// Create publisher
publisher user_pub(bus, "events.user");
```

### Using messaging_container_builder

```cpp
auto container = messaging_container_builder()
    .source("user-service", std::format("session_{}", i))
    .target("subscribers", "*")
    .message_type("user_created")
    .add_value("user_id", std::format("USR-{:05d}", i))
    .add_value("timestamp", std::chrono::system_clock::now())
    .optimize_for_speed()
    .build();
```

### Filtering by Priority

```cpp
sub.subscribe(
    "events.#",
    [](const message& msg) { /* handler */ },
    [](const message& msg) {
        return msg.metadata().priority == message_priority::high;
    }
);
```

## Expected Output

```
=== Pub/Sub Pattern Example ===

1. Setting up message bus...
Message bus started successfully

2. Creating subscribers...
Subscribed to user events (events.user.*)
Subscribed to order events (events.order.*)
Subscribed to high-priority events (events.#)

3. Creating publishers...
Publishers created

4. Publishing events...
  Published: events.user.created (event 1)
  [User Subscriber] Received: events.user.created
  ...

6. Statistics:
  User events received: 3
  Order events received: 2
  High-priority events received: 2
```

## Related Patterns

- [Request-Reply](../request_reply/) - Synchronous RPC over async messaging
- [Event Streaming](../event_streaming/) - Event sourcing with replay
- [Message Pipeline](../message_pipeline/) - Sequential message processing

## API Reference

| Class | Description |
|-------|-------------|
| `publisher` | Publishes messages to topics |
| `subscriber` | Subscribes to topics with optional filters |
| `message_bus` | Central pub/sub coordinator |
| `messaging_container_builder` | Type-safe message construction |

## See Also

- [Patterns API Documentation](../../../docs/PATTERNS_API.md)
- [API Reference](../../../docs/API_REFERENCE.md)
