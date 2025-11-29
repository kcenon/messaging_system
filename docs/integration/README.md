# Integration Guide

**Version**: 1.0
**Last Updated**: 2025-11-30

---

## Overview

This guide covers integrating messaging_system with other systems and applications.

---

## CMake Integration

### FetchContent (Recommended)

```cmake
include(FetchContent)

FetchContent_Declare(
    messaging_system
    GIT_REPOSITORY https://github.com/kcenon/messaging_system.git
    GIT_TAG main
)

FetchContent_MakeAvailable(messaging_system)

target_link_libraries(your_target PRIVATE messaging_system)
```

### Subdirectory

```cmake
add_subdirectory(external/messaging_system)
target_link_libraries(your_target PRIVATE messaging_system)
```

---

## Integration with Subsystems

### Using with thread_system

```cpp
#include <messaging_system/message_bus.h>
#include <thread_system/thread_pool.h>

auto pool = thread_system::thread_pool::create(4);
auto bus = messaging_system::message_bus::create(pool);
```

### Using with network_system

```cpp
// Remote messaging over network
auto transport = network_system::tcp_transport::create();
auto bus = messaging_system::message_bus::create(transport);
```

---

## Custom Configuration

```cpp
messaging_system::config cfg;
cfg.max_queue_size = 10000;
cfg.enable_tracing = true;
cfg.default_timeout = std::chrono::seconds(30);

auto bus = messaging_system::message_bus::create(cfg);
```

---

## Related Documentation

- [Quick Start](../guides/QUICK_START.md)
- [API Reference](../API_REFERENCE.md)
- [Architecture](../ARCHITECTURE.md)
