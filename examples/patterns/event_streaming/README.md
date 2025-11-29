# Event Streaming Pattern Example

This example demonstrates the **Event Streaming** pattern with event sourcing, replay capabilities, and batch processing.

## Overview

Event Streaming enables storing events in a stream and replaying them to new subscribers. This is essential for event sourcing architectures where the current state is derived from a sequence of events.

## Key Features Demonstrated

- **Event Publishing**: Publish events to a stream
- **Event Replay**: New subscribers can replay historical events
- **Filtered Subscriptions**: Subscribe with filters (e.g., high priority only)
- **Batch Processing**: Process events in batches for efficiency
- **messaging_container_builder**: Type-safe event construction
- **Message Serialization**: JSON serialization support

## Quick Start

```bash
# Build the example
cmake --build build --target example_event_streaming

# Run
./build/examples/patterns/event_streaming/example_event_streaming
```

## Code Highlights

### Creating an Event Stream

```cpp
event_stream_config stream_config;
stream_config.max_buffer_size = 100;
stream_config.enable_replay = true;

event_stream stream(bus, "events.orders", stream_config);
```

### Publishing Events with Container Builder

```cpp
auto container = messaging_container_builder()
    .source("order-service", "stream-publisher")
    .target("event-store", "*")
    .message_type("order_event")
    .add_value("order_id", std::format("order-{}", i))
    .add_value("amount", 100.0 * i)
    .add_value("timestamp", std::chrono::system_clock::now())
    .optimize_for_speed()
    .build();

auto msg = message_builder()
    .topic("events.orders")
    .type(message_type::event)
    .payload(container.value())
    .build();

stream.publish_event(std::move(msg.value()));
```

### Subscribing with Replay

```cpp
stream.subscribe(
    [](const message& event) {
        // Process event
        return common::ok();
    },
    true  // Enable replay of historical events
);
```

### Filtered Subscription

```cpp
stream.subscribe(
    [](const message& event) { /* handler */ },
    [](const message& event) {
        return event.metadata().priority == message_priority::high;
    },
    true  // Replay with filter
);
```

### Batch Processing

```cpp
event_batch_processor processor(
    bus,
    "events.batch.*",
    [](const std::vector<message>& batch) {
        // Process batch of events
        return common::ok();
    },
    5,  // batch size
    std::chrono::milliseconds{500}  // batch timeout
);

processor.start();
```

## Expected Output

```
=== Event Streaming Pattern Example ===

1. Setting up message bus...
Message bus started successfully

2. Creating event stream...
Event stream created for topic: events.orders

3. Publishing events to stream...
  Published event: order-1
  Published event: order-2
  ...

4. Event stream snapshot:
  Total events in buffer: 10
  Retrieved 10 events

5. Subscribing with replay...
  [Replay Subscriber] Received: <event-id>
  ...
  Replayed 10 events

6. Subscribing with filter (high priority only)...
  [Filtered Subscriber] High priority event: <event-id>
  Received 3 high-priority events

8. Setting up batch processor...
  [Batch Processor] Processing batch of 5 events
  [Batch Processor] Processing batch of 5 events
  Batches processed: 2
```

## Serialization

```cpp
message_serializer serializer;

// Serialize to JSON
auto json = serializer.to_json(container.value());
std::cout << "JSON: " << json.value() << std::endl;

// Deserialize from JSON
auto restored = serializer.from_json(json.value());
```

## Related Patterns

- [Pub/Sub](../pub_sub/) - Simple publish-subscribe
- [Request-Reply](../request_reply/) - Synchronous RPC
- [Message Pipeline](../message_pipeline/) - Sequential processing

## API Reference

| Class | Description |
|-------|-------------|
| `event_stream` | Stream with replay support |
| `event_batch_processor` | Batch event processing |
| `event_stream_config` | Stream configuration |
| `message_serializer` | JSON/binary serialization |

## See Also

- [Patterns API Documentation](../../../docs/PATTERNS_API.md)
- [API Reference](../../../docs/API_REFERENCE.md)
