# Request-Reply Pattern Example

This example demonstrates the **Request-Reply** messaging pattern for synchronous RPC-style communication over asynchronous messaging infrastructure.

## Overview

The Request-Reply pattern enables synchronous communication where a client sends a request and waits for a response. This is useful for implementing services that need to return results to callers.

## Key Features Demonstrated

- **Request Client**: Send requests and wait for replies
- **Request Server**: Handle requests and send replies
- **Correlation IDs**: Automatic request-reply matching
- **Timeout Handling**: Configurable request timeouts
- **Multi-threaded**: Server and client run in separate threads

## Quick Start

```bash
# Build the example
cmake --build build --target example_request_reply

# Run
./build/examples/patterns/request_reply/example_request_reply
```

## Code Highlights

### Creating a Request Server

```cpp
class CalculatorService {
public:
    static common::Result<message> handle_request(const message& request) {
        // Process request
        message reply("reply", message_type::reply);
        reply.metadata().correlation_id = request.metadata().id;
        reply.metadata().source = "calculator-service";

        return common::ok(std::move(reply));
    }
};

// Register handler
request_server server(bus, "service.calculator");
server.register_handler(CalculatorService::handle_request);
```

### Making Requests

```cpp
request_client client(bus, "service.calculator");

message request("service.calculator", message_type::query);
request.metadata().source = "client-app";

auto reply_result = client.request(
    std::move(request),
    std::chrono::milliseconds{2000}  // timeout
);

if (reply_result.is_ok()) {
    const auto& reply = reply_result.value();
    // Process reply
}
```

## Expected Output

```
=== Request-Reply Pattern Example ===

1. Setting up message bus...
Message bus started successfully

2. Starting server and client...

[Server Thread] Starting calculator service...
[Server Thread] Calculator service started

[Client Thread] Starting client...
[Client Thread] Making request #1...
  [Server] Processing request: <request-id>
  [Server] Sending reply for: <request-id>
[Client Thread] Received reply for request #1
  Correlation ID: <request-id>

...

3. Statistics:
  Total messages published: 6
  Total messages processed: 6
```

## Architecture

```
┌─────────────┐     Request      ┌─────────────┐
│   Client    │ ───────────────► │   Server    │
│             │                  │             │
│             │ ◄─────────────── │             │
└─────────────┘      Reply       └─────────────┘
                                        │
                                        ▼
                              ┌─────────────────┐
                              │ Request Handler │
                              │  (Your Logic)   │
                              └─────────────────┘
```

## Error Handling

```cpp
auto reply_result = client.request(request, timeout);

if (reply_result.is_err()) {
    auto& error = reply_result.error();
    // Handle timeout or other errors
    std::cerr << "Request failed: " << error.message << std::endl;
}
```

## Related Patterns

- [Pub/Sub](../pub_sub/) - One-to-many async messaging
- [Event Streaming](../event_streaming/) - Event sourcing with replay
- [Message Pipeline](../message_pipeline/) - Sequential message processing

## API Reference

| Class | Description |
|-------|-------------|
| `request_client` | Sends requests and waits for replies |
| `request_server` | Handles requests and sends replies |
| `message_type::query` | Request message type |
| `message_type::reply` | Reply message type |

## See Also

- [Patterns API Documentation](../../../docs/PATTERNS_API.md)
- [API Reference](../../../docs/API_REFERENCE.md)
