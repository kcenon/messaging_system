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

## Task Event Bridge Integration

The task event bridge enables cross-module communication by publishing task lifecycle events to the common_system event bus.

### Event Types

| Category | Event | Description |
|----------|-------|-------------|
| **Task Lifecycle** | `task_queued_event` | Task added to queue |
| | `task_started_event` | Task execution began |
| | `task_progress_event` | Task progress updated |
| | `task_succeeded_event` | Task completed successfully |
| | `task_failed_event` | Task execution failed |
| | `task_retrying_event` | Task scheduled for retry |
| | `task_cancelled_event` | Task was cancelled |
| **Worker** | `worker_online_event` | Worker came online |
| | `worker_offline_event` | Worker went offline |
| | `worker_heartbeat_event` | Periodic worker status |
| **Queue** | `queue_high_watermark_event` | Queue size threshold reached |
| | `queue_empty_event` | Queue became empty |

### Basic Usage

```cpp
#include <kcenon/common/patterns/event_bus.h>
#include <kcenon/messaging/integration/task_event_bridge.h>
#include <kcenon/messaging/integration/task_events.h>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

// Get global event bus and start it
auto& event_bus = get_event_bus();
event_bus.start();

// Subscribe to task events
auto sub = event_bus.subscribe<task_succeeded_event>(
    [](const task_succeeded_event& evt) {
        std::cout << "Task " << evt.task_id << " completed in "
                  << evt.duration.count() << "ms" << std::endl;
    });

// Create and start the bridge
task_event_bridge_config config;
config.enable_progress_events = true;

task_event_bridge bridge(config);
bridge.start();

// Publish events from your worker implementation
bridge.on_task_started("task-123", "email.send", "default", "worker-1");
// ... task executes ...
bridge.on_task_succeeded("task-123", "email.send", "default", "worker-1",
                         std::chrono::milliseconds(150));

// Cleanup
bridge.stop();
event_bus.unsubscribe(sub);
event_bus.stop();
```

### Configuration Options

```cpp
task_event_bridge_config config;

// Threshold for queue high watermark events (default: 1000)
config.queue_high_watermark_threshold = 500;

// Enable/disable progress events (default: true)
config.enable_progress_events = true;

// Enable/disable heartbeat events (default: true)
config.enable_heartbeat_events = true;

// Heartbeat interval (default: 30 seconds)
config.heartbeat_interval = std::chrono::milliseconds(30000);
```

### Event Type Constants

String constants are available for event identification:

```cpp
namespace task_event_types {
    // Task lifecycle events
    static constexpr auto task_queued = "task.queued";
    static constexpr auto task_started = "task.started";
    static constexpr auto task_progress = "task.progress";
    static constexpr auto task_succeeded = "task.succeeded";
    static constexpr auto task_failed = "task.failed";
    static constexpr auto task_retrying = "task.retrying";
    static constexpr auto task_cancelled = "task.cancelled";

    // Worker events
    static constexpr auto worker_online = "worker.online";
    static constexpr auto worker_offline = "worker.offline";
    static constexpr auto worker_heartbeat = "worker.heartbeat";

    // Queue events
    static constexpr auto queue_high_watermark = "queue.high_watermark";
    static constexpr auto queue_empty = "queue.empty";
}
```

---

## Related Documentation

- [Quick Start](../guides/QUICK_START.md)
- [API Reference](../API_REFERENCE.md)
- [Architecture](../ARCHITECTURE.md)
