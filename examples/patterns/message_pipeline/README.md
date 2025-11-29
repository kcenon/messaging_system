# Message Pipeline Pattern Example

This example demonstrates the **Message Pipeline** (Pipes and Filters) pattern for sequential message processing with validation, transformation, and enrichment stages.

## Overview

The Message Pipeline pattern allows you to compose a series of processing stages that messages flow through. Each stage can validate, transform, filter, or enrich messages before passing them to the next stage.

## Key Features Demonstrated

- **Pipeline Builder**: Fluent API for building pipelines
- **Validation Stages**: Validate messages before processing
- **Filter Stages**: Conditionally pass or reject messages
- **Transformer Stages**: Modify messages (e.g., priority boost)
- **Enrichment Stages**: Add metadata to messages
- **Optional Stages**: Stages that can fail without stopping the pipeline
- **Retry Stages**: Built-in retry logic for flaky operations
- **Pipeline Statistics**: Track success/failure rates

## Quick Start

```bash
# Build the example
cmake --build build --target example_message_pipeline

# Run
./build/examples/patterns/message_pipeline/example_message_pipeline
```

## Code Highlights

### Building a Pipeline

```cpp
pipeline_builder builder(bus);

auto pipeline_result = builder
    .from("input.raw_data")
    .to("output.processed_data")

    // Validation stage
    .add_stage("validate", [](const message& msg) -> common::Result<message> {
        if (msg.metadata().topic.empty()) {
            return common::make_error<message>(-1, "Invalid message");
        }
        return common::ok(message(msg));
    })

    // Filter stage
    .add_filter("high_priority_filter", [](const message& msg) {
        return msg.metadata().priority >= message_priority::normal;
    })

    // Transform stage
    .add_transformer("boost_priority", [](const message& msg) {
        message transformed(msg);
        transformed.metadata().priority = message_priority::high;
        return transformed;
    })

    // Enrichment stage
    .add_stage("enrich", [](const message& msg) -> common::Result<message> {
        message enriched(msg);
        enriched.metadata().source = "pipeline-processor";
        enriched.metadata().timestamp = std::chrono::system_clock::now();
        return common::ok(std::move(enriched));
    })

    // Optional stage (won't fail pipeline)
    .add_stage("log", [](const message& msg) -> common::Result<message> {
        // Logging logic
        return common::ok(message(msg));
    }, true)  // Mark as optional

    .build();
```

### Processing Messages

```cpp
// Manual processing
auto result = pipeline->process(std::move(msg));
if (result.is_ok()) {
    auto processed = result.unwrap();
    // Use processed message
}

// Automatic processing (subscribes to input, publishes to output)
pipeline->start();
```

### Common Pipeline Stages

```cpp
// Validation stage
auto validator = [](const message& msg) {
    return !msg.metadata().id.empty();
};
pipeline.add_stage("validate",
    pipeline_stages::create_validation_stage(validator));

// Enrichment stage
auto enricher = [](message& msg) {
    msg.metadata().source = "custom-pipeline";
};
pipeline.add_stage("enrich",
    pipeline_stages::create_enrichment_stage(enricher));

// Retry stage for flaky operations
auto flaky_op = [](const message& msg) -> common::Result<message> {
    // Operation that might fail
    return common::ok(message(msg));
};
pipeline.add_stage("process_with_retry",
    pipeline_stages::create_retry_stage(flaky_op, 3));  // 3 retries
```

## Pipeline Flow

```
┌─────────────┐     ┌──────────┐     ┌──────────┐     ┌─────────┐
│   Input     │ ──► │ Validate │ ──► │  Filter  │ ──► │Transform│
│   Topic     │     │          │     │          │     │         │
└─────────────┘     └──────────┘     └──────────┘     └─────────┘
                                                            │
┌─────────────┐     ┌──────────┐     ┌──────────┐          │
│   Output    │ ◄── │   Log    │ ◄── │  Enrich  │ ◄────────┘
│   Topic     │     │(optional)│     │          │
└─────────────┘     └──────────┘     └──────────┘
```

## Expected Output

```
=== Message Pipeline Pattern Example ===

1. Setting up message bus...
Message bus started successfully

2. Building message pipeline...
Pipeline built with 5 stages:
  1. validate
  2. high_priority_filter
  3. boost_priority
  4. enrich
  5. log

3. Processing messages manually...

Processing Message 1:
  [Stage: Validate] Processing message: msg-001
  [Stage: Validate] Message validated successfully
  [Stage: Filter] Message passed filter
  [Stage: Transform] Boosting message priority
  [Stage: Enrich] Adding metadata
  [Stage: Log] Message: msg-001 | Source: pipeline-processor | Priority: 2
Message 1 processed successfully

5. Pipeline statistics:
  Messages processed: 5
  Messages succeeded: 4
  Messages failed: 1
  Stage failures: 0
```

## Related Patterns

- [Pub/Sub](../pub_sub/) - Simple publish-subscribe
- [Request-Reply](../request_reply/) - Synchronous RPC
- [Event Streaming](../event_streaming/) - Event sourcing with replay

## API Reference

| Class | Description |
|-------|-------------|
| `message_pipeline` | Pipeline for sequential processing |
| `pipeline_builder` | Fluent API for building pipelines |
| `pipeline_stages` | Factory for common stage types |
| `pipeline_statistics` | Processing statistics |

## See Also

- [Patterns API Documentation](../../../docs/PATTERNS_API.md)
- [API Reference](../../../docs/API_REFERENCE.md)
