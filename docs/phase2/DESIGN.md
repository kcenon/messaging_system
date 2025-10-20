# Phase 2: Messaging Core Redesign - Design Document

## Overview
Redesign messaging core with Result<T> error handling, DI-based architecture, and high-performance routing.

## Core Components

### 1. Message Container DSL

**Header:** `include/messaging_system/core/messaging_container.h`

```cpp
#pragma once

#include <kcenon/container/core/container.h>
#include <kcenon/common/patterns/result.h>
#include <chrono>
#include <string>

namespace messaging {

using common::Result;
using common::VoidResult;
using container::value_container;

class MessagingContainer {
    value_container container_;

public:
    // Factory method
    static Result<MessagingContainer> create(
        const std::string& source,
        const std::string& target,
        const std::string& topic
    );

    // Accessors
    std::string source() const;
    std::string target() const;
    std::string topic() const;
    std::string trace_id() const;

    // Serialization
    Result<std::vector<uint8_t>> serialize() const;
    static Result<MessagingContainer> deserialize(const std::vector<uint8_t>& data);

    // Underlying container access
    const value_container& container() const { return container_; }
    value_container& container() { return container_; }
};

} // namespace messaging
```

**Builder Pattern:**

```cpp
class MessagingContainerBuilder {
    std::string source_;
    std::string target_;
    std::string topic_;
    std::string trace_id_;
    std::unordered_map<std::string, value> values_;
    bool optimize_speed_{false};

public:
    MessagingContainerBuilder& source(std::string s);
    MessagingContainerBuilder& target(std::string t);
    MessagingContainerBuilder& topic(std::string topic);
    MessagingContainerBuilder& trace_id(std::string id);
    MessagingContainerBuilder& add_value(const std::string& key, value val);
    MessagingContainerBuilder& optimize_for_speed();

    Result<MessagingContainer> build();
};
```

### 2. Error Code Definitions

**Header:** `include/messaging_system/error_codes.h`

```cpp
#pragma once

namespace messaging::error {

// Messaging system error codes: -200 to -299
constexpr int INVALID_MESSAGE = -200;
constexpr int ROUTING_FAILED = -201;
constexpr int SERIALIZATION_ERROR = -202;
constexpr int NETWORK_ERROR = -203;
constexpr int DATABASE_ERROR = -204;
constexpr int QUEUE_FULL = -205;
constexpr int TIMEOUT = -206;
constexpr int AUTHENTICATION_FAILED = -207;
constexpr int AUTHORIZATION_FAILED = -208;
constexpr int SUBSCRIPTION_FAILED = -209;
constexpr int PUBLICATION_FAILED = -210;
constexpr int UNKNOWN_TOPIC = -211;
constexpr int NO_SUBSCRIBERS = -212;

} // namespace messaging::error
```

### 3. Message Bus with DI

**Header:** `include/messaging_system/core/message_bus.h`

```cpp
#pragma once

#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/patterns/result.h>
#include "messaging_container.h"
#include "topic_router.h"

namespace messaging {

class MessageBus {
    std::shared_ptr<common::IExecutor> io_executor_;
    std::shared_ptr<common::IExecutor> work_executor_;
    std::shared_ptr<TopicRouter> router_;

public:
    MessageBus(
        std::shared_ptr<common::IExecutor> io_executor,
        std::shared_ptr<common::IExecutor> work_executor,
        std::shared_ptr<TopicRouter> router
    );

    // Asynchronous publication
    Result<void> publish_async(MessagingContainer msg);

    // Synchronous publication (blocking)
    Result<void> publish_sync(const MessagingContainer& msg);

    // Subscription
    using SubscriberCallback = std::function<Result<void>(const MessagingContainer&)>;
    Result<uint64_t> subscribe(const std::string& topic, SubscriberCallback callback);

    // Unsubscribe
    Result<void> unsubscribe(uint64_t subscription_id);

    // Lifecycle
    Result<void> start();
    Result<void> stop();
    bool is_running() const;
};

} // namespace messaging
```

### 4. Topic Router

**Header:** `include/messaging_system/core/topic_router.h`

```cpp
#pragma once

#include <kcenon/common/patterns/result.h>
#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/thread/core/bounded_job_queue.h>
#include "messaging_container.h"
#include <functional>
#include <regex>

namespace messaging {

class TopicRouter {
public:
    using SubscriberCallback = std::function<Result<void>(const MessagingContainer&)>;
    using Filter = std::function<bool(const MessagingContainer&)>;

    struct Subscription {
        uint64_t id;
        std::string topic_pattern;  // Supports: "user.*.created", "order.#"
        SubscriberCallback callback;
        Filter filter;
        int priority;  // 0-10, higher = more priority
    };

private:
    std::unordered_map<std::string, std::vector<Subscription>> subscriptions_;
    std::shared_ptr<common::IExecutor> executor_;
    std::shared_ptr<thread::bounded_job_queue> queue_;
    std::atomic<uint64_t> next_subscription_id_{1};
    mutable std::shared_mutex mutex_;

public:
    TopicRouter(
        std::shared_ptr<common::IExecutor> executor,
        size_t queue_capacity = 10000
    );

    // Subscription management
    Result<uint64_t> subscribe(
        const std::string& topic_pattern,
        SubscriberCallback callback,
        Filter filter = nullptr,
        int priority = 5
    );

    Result<void> unsubscribe(uint64_t subscription_id);

    // Message routing
    Result<void> route(const MessagingContainer& msg);

    // Statistics
    size_t subscriber_count() const;
    size_t pending_messages() const;

private:
    std::vector<Subscription> find_matching_subscriptions(const std::string& topic);
    bool match_pattern(const std::string& topic, const std::string& pattern);
    std::regex pattern_to_regex(const std::string& pattern);
};

} // namespace messaging
```

## Result<T> Migration Patterns

### Pattern 1: Simple Function Conversion

```cpp
// BEFORE
void validate_message(const Message& msg) {
    if (msg.empty()) {
        throw std::invalid_argument("Empty message");
    }
}

// AFTER
Result<void> validate_message(const MessagingContainer& msg) {
    if (msg.container().empty()) {
        return VoidResult::error(
            error_info{
                error::INVALID_MESSAGE,
                "Empty message",
                "validate_message",
                ""
            }
        );
    }
    return VoidResult::ok();
}
```

### Pattern 2: Error Propagation with RETURN_IF_ERROR

```cpp
Result<void> process_message(const MessagingContainer& msg) {
    // Validate
    auto validation = validate_message(msg);
    RETURN_IF_ERROR(validation);

    // Route
    auto routing = router_->route(msg);
    RETURN_IF_ERROR(routing);

    // Persist
    auto persistence = persist_message(msg);
    RETURN_IF_ERROR(persistence);

    return VoidResult::ok();
}
```

### Pattern 3: Monadic Chaining

```cpp
Result<ProcessedMessage> process_pipeline(const MessagingContainer& msg) {
    return parse_message(msg)
        .and_then([](auto parsed) { return validate(parsed); })
        .and_then([](auto validated) { return transform(validated); })
        .and_then([](auto transformed) { return enrich(transformed); })
        .map([](auto enriched) {
            return ProcessedMessage{enriched};
        });
}
```

## Performance Targets

### Message Container
- **Creation:** 5M containers/sec
- **Serialization:** 2M containers/sec (binary)
- **Memory:** ~128 bytes baseline

### Message Bus
- **Throughput:** 100K+ messages/sec
- **Latency P95:** <10ms
- **CPU:** <80% @ max load

### Topic Router
- **Routing:** 100K+ routes/sec
- **Subscription lookup:** O(1) for exact match, O(n) for patterns
- **Pattern matching:** <1μs per pattern

## Implementation Plan

### Week 1: Message Container
- Day 1-2: Implement MessagingContainer and Builder
- Day 3-4: Unit tests and benchmarks
- Day 5: Integration with value_container

### Week 2: Result<T> Migration
- Day 1-2: Define error codes, implement error_info
- Day 3-4: Convert core APIs to Result<T>
- Day 5: Update tests

### Week 3: DI Architecture
- Day 1-2: Refactor MessageBus with IExecutor injection
- Day 3-4: Service registry integration
- Day 5: Type-based executor routing

### Week 4: Topic Router
- Day 1-3: Implement TopicRouter with pattern matching
- Day 4: Backpressure integration
- Day 5: Performance benchmarks

## Testing Strategy

### Unit Tests
- Message container creation and serialization
- Error code propagation
- Topic pattern matching
- Subscription management

### Integration Tests
- End-to-end message flow
- Multiple subscribers
- High-throughput scenarios
- Error handling paths

### Benchmarks
- Message creation rate
- Serialization throughput
- Routing performance
- Memory usage

## Success Criteria
- [x] All APIs return Result<T>
- [ ] Message container meets performance targets
- [ ] DI architecture implemented
- [ ] Topic router handles 100K+ msg/s
- [ ] Zero exceptions in core code
- [ ] All tests passing
