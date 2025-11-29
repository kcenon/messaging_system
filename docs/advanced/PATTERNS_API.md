# Messaging Patterns API Reference

This document provides detailed API documentation for the messaging patterns implemented in the messaging system.

## Table of Contents

1. [Pub/Sub Pattern](#pubsub-pattern)
2. [Request-Reply Pattern](#request-reply-pattern)
3. [Event Streaming Pattern](#event-streaming-pattern)
4. [Message Pipeline Pattern](#message-pipeline-pattern)

---

## Pub/Sub Pattern

### Namespace: `kcenon::messaging::patterns`

The Pub/Sub pattern provides a simplified interface for publish-subscribe messaging.

### Class: `publisher`

High-level publisher for the pub/sub pattern.

#### Constructor
```cpp
publisher(std::shared_ptr<message_bus> bus);
publisher(std::shared_ptr<message_bus> bus, std::string default_topic);
```

#### Methods

##### `publish`
```cpp
common::VoidResult publish(message msg);
common::VoidResult publish(const std::string& topic, message msg);
```
Publishes a message to the default topic or a specified topic.

##### `get_default_topic`
```cpp
const std::string& get_default_topic() const;
```
Returns the default topic.

##### `set_default_topic`
```cpp
void set_default_topic(std::string topic);
```
Sets a new default topic.

##### `is_ready`
```cpp
bool is_ready() const;
```
Checks if the publisher is ready to publish messages.

#### Usage Example
```cpp
#include <kcenon/messaging/patterns/pub_sub.h>
using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;

// Create message bus
auto backend = std::make_shared<standalone_backend>(4);
auto bus = std::make_shared<message_bus>(backend);
bus->start();

// Create publisher with default topic
publisher pub(bus, "events.system");

// Publish message to default topic
message msg("events.system", message_type::event);
msg.metadata().source = "service-a";
pub.publish(std::move(msg));

// Publish to specific topic
message msg2("events.custom", message_type::event);
pub.publish("events.custom", std::move(msg2));
```

### Class: `subscriber`

High-level subscriber for the pub/sub pattern.

#### Constructor
```cpp
explicit subscriber(std::shared_ptr<message_bus> bus);
```

#### Methods

##### `subscribe`
```cpp
common::Result<uint64_t> subscribe(
    const std::string& topic_pattern,
    subscription_callback callback,
    message_filter filter = nullptr
);
```
Subscribes to a topic pattern with an optional filter.

##### `unsubscribe_all`
```cpp
common::VoidResult unsubscribe_all();
```
Unsubscribes from all topics.

#### Usage Example
```cpp
// Create subscriber
subscriber sub(bus);

// Subscribe to events
auto sub_result = sub.subscribe("events.*", [](const message& msg) {
    std::cout << "Received event: " << msg.metadata().topic << std::endl;
    return common::ok();
});

// Subscribe with filter
auto filtered_sub = sub.subscribe(
    "events.*",
    [](const message& msg) {
        std::cout << "High priority event: " << msg.metadata().topic << std::endl;
        return common::ok();
    },
    [](const message& msg) {
        return msg.metadata().priority == message_priority::high;
    }
);
```

---

## Request-Reply Pattern

### Namespace: `kcenon::messaging::patterns`

The Request-Reply pattern enables synchronous request-reply communication over asynchronous messaging.

### Class: `request_reply_handler`

Handles request-reply messaging pattern with correlation IDs.

#### Constructor
```cpp
request_reply_handler(
    std::shared_ptr<message_bus> bus,
    std::string service_topic
);
```

#### Methods

##### `request`
```cpp
common::Result<message> request(
    message req,
    std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
);
```
Sends a request and waits for a reply.

##### `register_handler`
```cpp
common::VoidResult register_handler(
    std::function<common::Result<message>(const message&)> handler
);
```
Registers a handler function for processing requests (server side).

#### Usage Example
```cpp
#include <kcenon/messaging/patterns/request_reply.h>

// Client side
request_reply_handler client(bus, "service.compute");

message request("service.compute", message_type::query);
request.metadata().source = "client-app";

auto reply_result = client.request(std::move(request), std::chrono::seconds{10});
if (reply_result.is_ok()) {
    auto reply = reply_result.unwrap();
    std::cout << "Got reply: " << reply.metadata().topic << std::endl;
}

// Server side
request_reply_handler server(bus, "service.compute");

server.register_handler([](const message& request) -> common::Result<message> {
    // Process request
    message reply("reply.topic", message_type::reply);
    reply.metadata().correlation_id = request.metadata().id;
    return common::ok(std::move(reply));
});
```

### Class: `request_client`

Simplified client for making requests.

#### Constructor
```cpp
request_client(std::shared_ptr<message_bus> bus);
```

#### Methods

##### `request`
```cpp
common::Result<message> request(
    const std::string& service_topic,
    message req,
    std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
);
```

#### Usage Example
```cpp
request_client client(bus);

message req("service.data", message_type::query);
auto reply = client.request("service.data", std::move(req));
```

### Class: `request_server`

Simplified server for handling requests.

#### Constructor
```cpp
request_server(
    std::shared_ptr<message_bus> bus,
    std::string service_topic
);
```

#### Methods

##### `register_handler`
```cpp
common::VoidResult register_handler(
    std::function<common::Result<message>(const message&)> handler
);
```

##### `start` / `stop`
```cpp
common::VoidResult start();
common::VoidResult stop();
```

#### Usage Example
```cpp
request_server server(bus, "service.calculator");

server.register_handler([](const message& req) -> common::Result<message> {
    // Perform calculation
    message reply("reply", message_type::reply);
    // ... set reply data ...
    return common::ok(std::move(reply));
});

server.start();
```

---

## Event Streaming Pattern

### Namespace: `kcenon::messaging::patterns`

The Event Streaming pattern provides event sourcing capabilities with replay, filtering, and batch processing.

### Struct: `event_stream_config`

Configuration for event streams.

```cpp
struct event_stream_config {
    size_t max_buffer_size = 1000;
    bool enable_replay = true;
    bool enable_persistence = false;
    std::chrono::milliseconds batch_timeout{100};
    size_t batch_size = 10;
};
```

### Class: `event_stream`

Event streaming and sourcing with replay capabilities.

#### Constructor
```cpp
event_stream(
    std::shared_ptr<message_bus> bus,
    std::string stream_topic,
    event_stream_config config = {}
);
```

#### Methods

##### `publish_event`
```cpp
common::VoidResult publish_event(message event);
```
Publishes an event to the stream and buffers it for replay.

##### `subscribe`
```cpp
common::Result<uint64_t> subscribe(
    subscription_callback callback,
    bool replay_past_events = false
);

common::Result<uint64_t> subscribe(
    subscription_callback callback,
    message_filter filter,
    bool replay_past_events = false
);
```
Subscribes to the event stream with optional replay and filtering.

##### `unsubscribe`
```cpp
common::VoidResult unsubscribe(uint64_t subscription_id);
```

##### `replay`
```cpp
common::VoidResult replay(
    subscription_callback callback,
    message_filter filter = nullptr
);
```
Manually replays buffered events to a callback.

##### `get_events`
```cpp
std::vector<message> get_events(message_filter filter = nullptr) const;
```
Retrieves buffered events with optional filtering.

##### `event_count`
```cpp
size_t event_count() const;
```
Returns the number of buffered events.

##### `clear_buffer`
```cpp
void clear_buffer();
```
Clears the event buffer.

#### Usage Example
```cpp
#include <kcenon/messaging/patterns/event_streaming.h>

// Configure event stream
event_stream_config config;
config.max_buffer_size = 1000;
config.enable_replay = true;

event_stream stream(bus, "events.orders", config);

// Publish events
message order_created("events.orders", message_type::event);
order_created.metadata().source = "order-service";
stream.publish_event(std::move(order_created));

// Subscribe with replay
stream.subscribe([](const message& event) {
    std::cout << "Event: " << event.metadata().topic << std::endl;
    return common::ok();
}, true);  // Replay past events

// Subscribe with filter
auto high_value_filter = [](const message& msg) {
    // Custom filtering logic
    return msg.metadata().priority == message_priority::high;
};

stream.subscribe(
    [](const message& event) {
        std::cout << "High value order!" << std::endl;
        return common::ok();
    },
    high_value_filter,
    true
);

// Get snapshot of events
auto events = stream.get_events();
std::cout << "Total events in stream: " << events.size() << std::endl;
```

### Class: `event_batch_processor`

Processes events in batches for efficiency.

#### Constructor
```cpp
event_batch_processor(
    std::shared_ptr<message_bus> bus,
    std::string topic_pattern,
    batch_callback callback,
    size_t batch_size = 10,
    std::chrono::milliseconds batch_timeout = std::chrono::milliseconds{100}
);
```

#### Methods

##### `start` / `stop`
```cpp
common::VoidResult start();
common::VoidResult stop();
```

##### `flush`
```cpp
common::VoidResult flush();
```
Immediately processes the current batch.

##### `is_running`
```cpp
bool is_running() const;
```

#### Usage Example
```cpp
// Process events in batches of 100
event_batch_processor processor(
    bus,
    "events.analytics.*",
    [](const std::vector<message>& batch) {
        std::cout << "Processing batch of " << batch.size() << " events" << std::endl;
        // Bulk processing logic
        return common::ok();
    },
    100,  // batch size
    std::chrono::seconds{5}  // max wait time
);

processor.start();

// Events are automatically collected and processed in batches
// ...

processor.stop();
```

---

## Message Pipeline Pattern

### Namespace: `kcenon::messaging::patterns`

The Message Pipeline pattern implements pipes-and-filters for sequential message processing.

### Type Aliases

```cpp
using message_processor = std::function<common::Result<message>(const message&)>;
```

### Class: `message_pipeline`

Sequential message processing pipeline.

#### Struct: `pipeline_stage`

```cpp
struct pipeline_stage {
    std::string name;
    message_processor processor;
    bool optional;  // If true, stage failures won't stop pipeline
};
```

#### Constructor
```cpp
message_pipeline(
    std::shared_ptr<message_bus> bus,
    std::string input_topic,
    std::string output_topic
);
```

#### Methods

##### `add_stage`
```cpp
message_pipeline& add_stage(
    std::string name,
    message_processor processor,
    bool optional = false
);
```
Adds a processing stage to the pipeline. Returns reference for chaining.

##### `remove_stage`
```cpp
common::VoidResult remove_stage(const std::string& name);
```

##### `start` / `stop`
```cpp
common::VoidResult start();
common::VoidResult stop();
```
Starts/stops automatic processing from input to output topic.

##### `process`
```cpp
common::Result<message> process(message msg) const;
```
Manually processes a single message through the pipeline.

##### `stage_count`
```cpp
size_t stage_count() const;
```

##### `get_stage_names`
```cpp
std::vector<std::string> get_stage_names() const;
```

##### `get_statistics`
```cpp
struct statistics_snapshot {
    uint64_t messages_processed;
    uint64_t messages_succeeded;
    uint64_t messages_failed;
    uint64_t stage_failures;
};

statistics_snapshot get_statistics() const;
void reset_statistics();
```

#### Usage Example
```cpp
#include <kcenon/messaging/patterns/message_pipeline.h>

message_pipeline pipeline(bus, "input.raw", "output.processed");

// Add validation stage
pipeline.add_stage("validate", [](const message& msg) -> common::Result<message> {
    if (msg.metadata().topic.empty()) {
        return common::err<message>(
            common::error_info::create(-1, "Invalid message")
        );
    }
    return common::ok(message(msg));
});

// Add transformation stage
pipeline.add_stage("transform", [](const message& msg) -> common::Result<message> {
    message transformed(msg);
    transformed.metadata().priority = message_priority::high;
    return common::ok(std::move(transformed));
});

// Add enrichment stage (optional - failures won't stop pipeline)
pipeline.add_stage("enrich", [](const message& msg) -> common::Result<message> {
    message enriched(msg);
    enriched.metadata().source = "pipeline-processor";
    return common::ok(std::move(enriched));
}, true);  // Mark as optional

// Start automatic processing
pipeline.start();

// Or manually process
message input("test.topic");
auto result = pipeline.process(std::move(input));
if (result.is_ok()) {
    auto output = result.unwrap();
    // Use processed message
}

// Check statistics
auto stats = pipeline.get_statistics();
std::cout << "Processed: " << stats.messages_processed << std::endl;
std::cout << "Succeeded: " << stats.messages_succeeded << std::endl;
std::cout << "Failed: " << stats.messages_failed << std::endl;
```

### Class: `pipeline_builder`

Builder pattern for constructing pipelines.

#### Constructor
```cpp
explicit pipeline_builder(std::shared_ptr<message_bus> bus);
```

#### Methods

##### `from` / `to`
```cpp
pipeline_builder& from(std::string topic);
pipeline_builder& to(std::string topic);
```
Sets input and output topics.

##### `add_stage`
```cpp
pipeline_builder& add_stage(
    std::string name,
    message_processor processor,
    bool optional = false
);
```

##### `add_filter`
```cpp
pipeline_builder& add_filter(
    std::string name,
    std::function<bool(const message&)> filter
);
```
Adds a filter stage (messages that don't match are dropped).

##### `add_transformer`
```cpp
pipeline_builder& add_transformer(
    std::string name,
    std::function<message(const message&)> transformer
);
```
Adds a transformation stage.

##### `build`
```cpp
common::Result<std::unique_ptr<message_pipeline>> build();
```
Builds and returns the configured pipeline.

#### Usage Example
```cpp
pipeline_builder builder(bus);

auto pipeline_result = builder
    .from("input.orders")
    .to("output.validated_orders")
    .add_filter("high_value_only", [](const message& msg) {
        // Custom filter logic
        return true;  // Keep message
    })
    .add_transformer("add_timestamp", [](const message& msg) {
        message transformed(msg);
        transformed.metadata().timestamp = std::chrono::system_clock::now();
        return transformed;
    })
    .add_stage("validate", [](const message& msg) -> common::Result<message> {
        // Validation logic
        return common::ok(message(msg));
    })
    .build();

if (pipeline_result.is_ok()) {
    auto pipeline = pipeline_result.unwrap();
    pipeline->start();
}
```

### Namespace: `pipeline_stages`

Common pipeline stage implementations.

#### `create_logging_stage`
```cpp
message_processor create_logging_stage(const std::string& stage_name);
```
Creates a stage that logs messages passing through.

#### `create_validation_stage`
```cpp
message_processor create_validation_stage(
    std::function<bool(const message&)> validator
);
```
Creates a stage that validates messages.

#### `create_enrichment_stage`
```cpp
message_processor create_enrichment_stage(
    std::function<void(message&)> enricher
);
```
Creates a stage that enriches messages with additional data.

#### `create_retry_stage`
```cpp
message_processor create_retry_stage(
    message_processor processor,
    size_t max_retries = 3,
    std::chrono::milliseconds retry_delay = std::chrono::milliseconds{100}
);
```
Wraps a processor with retry logic.

#### Usage Example
```cpp
using namespace pipeline_stages;

message_pipeline pipeline(bus, "input", "output");

// Add logging stage
pipeline.add_stage("log_input", create_logging_stage("InputLogger"));

// Add validation stage
auto validator = [](const message& msg) {
    return !msg.metadata().topic.empty();
};
pipeline.add_stage("validate", create_validation_stage(validator));

// Add enrichment stage
auto enricher = [](message& msg) {
    msg.metadata().source = "pipeline";
    msg.metadata().timestamp = std::chrono::system_clock::now();
};
pipeline.add_stage("enrich", create_enrichment_stage(enricher));

// Add retry wrapper for flaky operation
auto flaky_operation = [](const message& msg) -> common::Result<message> {
    // Operation that might fail temporarily
    return common::ok(message(msg));
};
pipeline.add_stage("process", create_retry_stage(flaky_operation, 3));

pipeline.start();
```

---

## Best Practices

### Pub/Sub Pattern
- Use topic wildcards (`*`, `#`) for flexible subscription patterns
- Apply message filters to reduce callback invocations
- Set appropriate default topics to simplify publishing

### Request-Reply Pattern
- Always set reasonable timeouts for requests
- Use correlation IDs to track request-reply pairs
- Handle timeout errors gracefully on the client side
- Process requests asynchronously on the server side

### Event Streaming Pattern
- Configure buffer size based on replay requirements
- Use filters to optimize replay performance
- Consider batch processing for high-throughput scenarios
- Enable persistence for critical event streams

### Message Pipeline Pattern
- Keep stages focused on single responsibilities
- Use optional stages for non-critical operations
- Monitor pipeline statistics to identify bottlenecks
- Use the builder pattern for complex pipelines
- Apply retry logic for transient failures

### General Guidelines
- Always check Result return values
- Use appropriate message priorities
- Set message TTL for time-sensitive messages
- Leverage the DI container for dependency management
- Monitor message bus statistics for performance tuning

---

## Error Handling

All pattern APIs use the `common::Result<T>` pattern for error handling:

```cpp
auto result = publisher.publish(msg);
if (result.is_err()) {
    auto error = result.get_error();
    std::cerr << "Publish failed: " << error.message
              << " (code: " << error.code << ")" << std::endl;
}
```

Common error codes are defined in `kcenon::common::error::codes::messaging_system`.

---

## Thread Safety

- All pattern classes are thread-safe unless otherwise noted
- Message bus can be safely accessed from multiple threads
- Callbacks may be invoked from worker threads - ensure thread-safe access to shared state
- Use appropriate synchronization primitives in callback implementations

---

## Performance Considerations

- **Pub/Sub**: Lightweight for simple publish-subscribe scenarios
- **Request-Reply**: Adds overhead for correlation tracking - use only when synchronous semantics are required
- **Event Streaming**: Buffer size affects memory usage - tune based on replay requirements
- **Message Pipeline**: Each stage adds processing overhead - minimize stage count for latency-critical paths

For high-performance scenarios, consider:
- Using lock-free queue implementations
- Tuning worker thread counts
- Adjusting queue capacities
- Enabling batch processing for bulk operations
