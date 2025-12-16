# Integration Guide

**Version**: 0.1.0
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

### Using UnifiedBootstrapper (Recommended)

The recommended way to integrate messaging_system is through `UnifiedBootstrapper` from common_system. This provides unified lifecycle management and automatic shutdown coordination.

#### Basic Integration

```cpp
#include <kcenon/common/di/unified_bootstrapper.h>
#include <kcenon/messaging/di/messaging_bootstrapper.h>

using namespace kcenon::common::di;
using namespace kcenon::messaging::di;

int main() {
    // Step 1: Initialize unified bootstrapper
    auto init_result = unified_bootstrapper::initialize({
        .enable_logging = true,
        .enable_monitoring = true,
        .register_signal_handlers = true
    });

    if (init_result.is_err()) {
        std::cerr << "Bootstrapper init failed\n";
        return 1;
    }

    // Step 2: Integrate messaging services
    auto msg_result = messaging_bootstrapper::integrate({
        .config = {
            .worker_threads = 8,
            .queue_capacity = 2000,
            .enable_event_bridge = true
        },
        .auto_start = true
    });

    if (msg_result.is_err()) {
        std::cerr << "Messaging integration failed\n";
        unified_bootstrapper::shutdown();
        return 1;
    }

    // Step 3: Use the message bus
    auto bus = messaging_bootstrapper::get_message_bus();
    if (bus) {
        // Publish messages, subscribe to topics, etc.
    }

    // Step 4: Wait for shutdown signal
    while (!unified_bootstrapper::is_shutdown_requested()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Step 5: Graceful shutdown (messaging automatically stops via hooks)
    unified_bootstrapper::shutdown();
    return 0;
}
```

#### Fluent Builder API

For a more concise configuration, use the builder pattern:

```cpp
#include <kcenon/common/di/unified_bootstrapper.h>
#include <kcenon/messaging/di/messaging_bootstrapper.h>

// Initialize bootstrapper
unified_bootstrapper::initialize({
    .register_signal_handlers = true
});

// Configure and integrate messaging with builder
auto result = messaging_bootstrapper::builder()
    .with_worker_threads(8)
    .with_queue_capacity(2000)
    .with_event_bridge(true)
    .with_auto_start(true)
    .with_executor(true)
    .integrate();

// Use messaging services...
auto bus = messaging_bootstrapper::get_message_bus();
auto bridge = messaging_bootstrapper::get_event_bridge();
```

#### Service Resolution from DI Container

You can also resolve services directly from the container:

```cpp
auto& container = unified_bootstrapper::services();

auto bus_result = container.resolve<IMessageBus>();
if (bus_result.is_ok()) {
    auto bus = bus_result.value();
    bus->publish(some_message);
}
```

#### Shutdown Behavior

The messaging_bootstrapper automatically registers a shutdown hook that:
1. Stops the message bus gracefully
2. Stops the event bridge if enabled
3. Cleans up all messaging services

This ensures proper cleanup order and prevents resource leaks.

---

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
