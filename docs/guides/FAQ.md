# Frequently Asked Questions

**Version**: 1.0
**Last Updated**: 2025-11-30

---

## General Questions

### Q: What is messaging_system?

A: messaging_system is a production-ready messaging infrastructure providing pub/sub, request/reply, event streaming, and message pipeline patterns.

### Q: What C++ standard is required?

A: C++20 is required.

### Q: Which platforms are supported?

A: Linux (Ubuntu 20.04+), macOS (12+), and Windows (10+).

---

## Installation & Setup

### Q: How do I install the library?

A: Use CMake FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    messaging_system
    GIT_REPOSITORY https://github.com/kcenon/messaging_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(messaging_system)
```

### Q: What dependencies are required?

A: Dependencies are fetched automatically:
- common_system
- thread_system
- container_system
- logger_system
- monitoring_system
- network_system

---

## Usage Questions

### Q: How do I create a message bus?

```cpp
auto bus = messaging_system::message_bus::create();
bus->start();
```

### Q: How do I subscribe to topics?

```cpp
bus->subscribe("events.*", [](const message& msg) {
    // Handle message
});
```

### Q: How do I use wildcards?

- `*` matches one level: `events.*` matches `events.user`
- `#` matches multiple levels: `events.#` matches `events.user.created`

---

## Performance Questions

### Q: What throughput can I expect?

- Pub/Sub: 500K+ messages/second
- Request/Reply: 10K+ requests/second

### Q: What is the latency?

- Topic routing: <1 μs
- Message delivery: <10 μs

---

## More Questions?

- Check [Troubleshooting](TROUBLESHOOTING.md)
- Open an [issue](https://github.com/kcenon/messaging_system/issues)
