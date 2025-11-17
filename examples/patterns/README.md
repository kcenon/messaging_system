# Messaging Patterns Examples

This directory contains example applications demonstrating the messaging patterns available in the messaging system v2.0.

## Overview

The examples showcase the following patterns:

1. **Pub/Sub Pattern** - Simple publish-subscribe messaging
2. **Request-Reply Pattern** - Synchronous request-response communication
3. **Event Streaming Pattern** - Event sourcing with replay and batch processing
4. **Message Pipeline Pattern** - Sequential message processing with validation and transformation

## Building the Examples

### Prerequisites

- C++17 compatible compiler
- CMake 3.20 or later
- messaging_system v2.0
- common_system v2.0
- container_system v2.0

### Build Instructions

```bash
# From the messaging_system root directory
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# Or use the build script
./build.sh dev-local --examples
```

The example binaries will be located in `build/bin/examples/patterns/`.

## Running the Examples

### 1. Pub/Sub Pattern Example

```bash
./build/bin/examples/patterns/pub_sub/example_pub_sub
```

**What it demonstrates:**
- Creating publishers and subscribers
- Topic pattern matching with wildcards (`*`, `#`)
- Message filtering based on priority
- Multiple subscribers to the same topic
- Subscription management

**Expected output:**
- User events being published and received
- Order events being published and received
- High-priority events being filtered and processed
- Statistics showing message counts

### 2. Request-Reply Pattern Example

```bash
./build/bin/examples/patterns/request_reply/example_request_reply
```

**What it demonstrates:**
- Setting up a request-reply service
- Making synchronous requests over async infrastructure
- Correlation ID tracking
- Timeout handling
- Multi-threaded server/client interaction

**Expected output:**
- Server starting and registering handler
- Client making multiple requests
- Replies being correlated to requests
- Statistics showing request/reply pairs

### 3. Event Streaming Pattern Example

```bash
./build/bin/examples/patterns/event_streaming/example_event_streaming
```

**What it demonstrates:**
- Publishing events to a stream
- Event replay for new subscribers
- Event filtering during replay
- Getting event snapshots
- Batch event processing

**Expected output:**
- Events being published to stream
- New subscribers receiving replayed events
- Filtered subscribers receiving only matching events
- Batch processor handling events in groups
- Event buffer statistics

### 4. Message Pipeline Pattern Example

```bash
./build/bin/examples/patterns/message_pipeline/example_message_pipeline
```

**What it demonstrates:**
- Building a multi-stage message pipeline
- Message validation, filtering, and transformation
- Optional vs required stages
- Manual and automatic message processing
- Pipeline statistics and monitoring
- Using common pipeline stages (validation, enrichment, retry)

**Expected output:**
- Messages flowing through pipeline stages
- Stage-by-stage processing logs
- Filter stage dropping low-priority messages
- Transformation and enrichment of messages
- Pipeline statistics (processed, succeeded, failed)

## Example Code Structure

Each example follows a similar structure:

```cpp
// 1. Setup
auto backend = std::make_shared<standalone_backend>(threads);
auto bus = std::make_shared<message_bus>(backend, config);
bus->start();

// 2. Create pattern-specific objects
// (publishers, subscribers, streams, pipelines, etc.)

// 3. Demonstrate core functionality
// (publish messages, process requests, replay events, etc.)

// 4. Display statistics

// 5. Cleanup
bus->stop();
```

## Common Patterns

### Error Handling

All examples demonstrate proper error handling with `common::Result<T>`:

```cpp
auto result = bus->publish(msg);
if (!result.is_ok()) {
    auto error = result.get_error();
    std::cerr << "Error: " << error.message << " (code: " << error.code << ")" << std::endl;
    return 1;
}
```

### Message Creation

Examples show different ways to create messages:

```cpp
// Direct construction
message msg("topic.name", message_type::event);
msg.metadata().priority = message_priority::high;

// Builder pattern
auto msg_result = message_builder()
    .topic("topic.name")
    .type(message_type::event)
    .priority(message_priority::high)
    .build();
```

### Callbacks

All callbacks follow the pattern:

```cpp
auto callback = [](const message& msg) -> common::VoidResult {
    // Process message
    return common::ok();
};
```

## Customizing Examples

### Adjusting Worker Threads

```cpp
message_bus_config config;
config.worker_threads = 4;  // Adjust based on your needs
auto bus = std::make_shared<message_bus>(backend, config);
```

### Changing Queue Capacity

```cpp
message_bus_config config;
config.queue_capacity = 10000;  // Larger capacity for high throughput
auto bus = std::make_shared<message_bus>(backend, config);
```

### Adding Custom Stages to Pipeline

```cpp
pipeline.add_stage("custom_stage", [](const message& msg) -> common::Result<message> {
    // Your custom processing logic
    return common::ok(message(msg));
});
```

## Troubleshooting

### Issue: Examples don't compile

**Solution**: Ensure all dependencies are installed and CMake can find them:
```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/dependencies
```

### Issue: Segmentation fault on startup

**Solution**: Check that the backend is properly initialized:
```cpp
auto backend = std::make_shared<standalone_backend>(2);
auto init_result = backend->initialize();
if (!init_result.is_ok()) {
    std::cerr << "Backend init failed" << std::endl;
    return 1;
}
```

### Issue: Messages not being received

**Solution**: Verify bus is started and topic patterns match:
```cpp
bus->start();  // Must call before publishing

// Topic patterns must match:
// Publish: "events.user.created"
// Subscribe: "events.user.*" or "events.#"
```

### Issue: No output from examples

**Solution**: Add sleep to allow async processing:
```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(100));
```

## Next Steps

After running these examples, you can:

1. **Review the API Documentation**
   - See [PATTERNS_API.md](../../docs/PATTERNS_API.md) for detailed API reference
   - See [API_REFERENCE.md](../../docs/API_REFERENCE.md) for core system APIs

2. **Explore Integration**
   - Check [MIGRATION_GUIDE.md](../../docs/MIGRATION_GUIDE.md) for integration patterns
   - Review integration tests in `integration_tests/`

3. **Build Your Application**
   - Use these examples as templates
   - Combine multiple patterns as needed
   - Add your domain-specific logic

4. **Performance Tuning**
   - Run benchmarks in `benchmarks/`
   - Adjust thread counts and queue sizes
   - Consider lock-free queue options

## Additional Resources

- [Messaging System Architecture](../../docs/SYSTEM_ARCHITECTURE.md)
- [Getting Started Guide](../../docs/GETTING_STARTED.md)
- [Troubleshooting Guide](../../docs/TROUBLESHOOTING.md)
- [Design Patterns](../../docs/DESIGN_PATTERNS.md)

## Contributing

If you have suggestions for improving these examples or want to contribute new examples, please:

1. Follow the existing code style
2. Add comprehensive comments
3. Include expected output in documentation
4. Test on multiple platforms

## License

These examples are part of the messaging_system project and are subject to the same license.
