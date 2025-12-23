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

## Messaging Event Bridge Integration

The messaging event bridge enables cross-module communication by publishing messaging lifecycle events to the common_system event bus.

### Event Types

| Category | Event | Description |
|----------|-------|-------------|
| **Message Bus** | `message_bus_started_event` | Message bus started |
| | `message_bus_stopped_event` | Message bus stopped |
| **Message** | `message_received_event` | Message was received |
| | `message_published_event` | Message was published |
| | `message_error_event` | Message processing error |
| **Topic/Subscription** | `topic_created_event` | New topic pattern registered |
| | `subscriber_added_event` | Subscriber added to topic |
| | `subscriber_removed_event` | Subscriber removed from topic |

### Basic Usage

```cpp
#include <kcenon/common/patterns/event_bus.h>
#include <kcenon/messaging/integration/event_bridge.h>
#include <kcenon/messaging/core/message_bus.h>

using namespace kcenon::messaging;
using namespace kcenon::messaging::integration;
using namespace kcenon::common;

// Get global event bus
auto& event_bus = get_event_bus();

// Subscribe to topic creation events
auto topic_sub = event_bus.subscribe<topic_created_event>(
    [](const topic_created_event& evt) {
        std::cout << "New topic created: " << evt.topic_pattern << std::endl;
    });

// Subscribe to subscriber events
auto sub_added = event_bus.subscribe<subscriber_added_event>(
    [](const subscriber_added_event& evt) {
        std::cout << "Subscriber " << evt.subscription_id
                  << " added to " << evt.topic_pattern
                  << " with priority " << evt.priority << std::endl;
    });

auto sub_removed = event_bus.subscribe<subscriber_removed_event>(
    [](const subscriber_removed_event& evt) {
        std::cout << "Subscriber " << evt.subscription_id
                  << " removed from " << evt.topic_pattern << std::endl;
    });

// Create message bus with event bridge
auto backend = std::make_shared<standalone_backend>();
auto bus = std::make_shared<message_bus>(backend, message_bus_config{});

messaging_event_bridge bridge(bus);
bridge.start();

// Configure topic_router callbacks to connect with event bridge
bus->get_router().set_callbacks({
    .on_topic_created = [&bridge](const std::string& pattern) {
        bridge.on_topic_created(pattern);
    },
    .on_subscriber_added = [&bridge](uint64_t id, const std::string& pattern, int prio) {
        bridge.on_subscriber_added(id, pattern, prio);
    },
    .on_subscriber_removed = [&bridge](uint64_t id, const std::string& pattern) {
        bridge.on_subscriber_removed(id, pattern);
    }
});

// Now when you subscribe to topics, events will be published
bus->subscribe("user.created", [](const message& msg) {
    // Handle message
    return common::ok();
});
// This triggers: topic_created_event, subscriber_added_event

// Cleanup
bridge.stop();
event_bus.unsubscribe(topic_sub);
event_bus.unsubscribe(sub_added);
event_bus.unsubscribe(sub_removed);
```

### Event Schemas

#### topic_created_event

```cpp
struct topic_created_event {
    std::string topic_pattern;  // The topic pattern that was created
    std::chrono::system_clock::time_point timestamp;
};
```

#### subscriber_added_event

```cpp
struct subscriber_added_event {
    uint64_t subscription_id;   // Unique ID of the subscription
    std::string topic_pattern;  // Topic pattern subscribed to
    int priority;               // Subscription priority (0-10)
    std::chrono::system_clock::time_point timestamp;
};
```

#### subscriber_removed_event

```cpp
struct subscriber_removed_event {
    uint64_t subscription_id;   // ID of the removed subscription
    std::string topic_pattern;  // Topic pattern that was unsubscribed
    std::chrono::system_clock::time_point timestamp;
};
```

### Use Cases

1. **Monitoring Dashboard**: Track topic and subscriber counts in real-time
2. **Audit Logging**: Log all subscription changes for compliance
3. **Auto-scaling**: Trigger scaling based on subscriber patterns
4. **Circuit Breaker**: Monitor subscription health and failures

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

## Monitoring System Integration

The `message_bus_collector` provides integration with monitoring_system for collecting message bus performance metrics.

### Prerequisites

- `monitoring_system` available (automatically fetched via FetchContent)
- `WITH_MONITORING_SYSTEM` compile definition enabled (automatic when monitoring_system is found)
- The `KCENON_WITH_MONITORING_SYSTEM` macro from `feature_flags.h` gates the monitoring integration

### Collected Metrics

| Metric Name | Type | Description |
|-------------|------|-------------|
| `messaging_messages_published_total` | Counter | Total messages published |
| `messaging_messages_processed_total` | Counter | Total messages successfully processed |
| `messaging_messages_failed_total` | Counter | Total messages that failed processing |
| `messaging_messages_dropped_total` | Counter | Total messages dropped |
| `messaging_queue_depth` | Gauge | Current queue depth |
| `messaging_queue_capacity` | Gauge | Maximum queue capacity |
| `messaging_queue_utilization_percent` | Gauge | Queue utilization percentage |
| `messaging_throughput_per_second` | Gauge | Messages processed per second |
| `messaging_latency_average_ms` | Gauge | Average message processing latency |
| `messaging_latency_max_ms` | Gauge | Maximum message processing latency |
| `messaging_latency_min_ms` | Gauge | Minimum message processing latency |
| `messaging_topic_count` | Gauge | Number of active topics |
| `messaging_total_subscribers` | Gauge | Total number of subscribers |
| `messaging_subscribers_per_topic` | Gauge | Subscribers per topic (with topic label) |
| `messaging_worker_threads` | Gauge | Number of worker threads |
| `messaging_is_running` | Gauge | Message bus running status (1 or 0) |

### Basic Usage

```cpp
#include <kcenon/messaging/collectors/message_bus_collector.h>
#include <kcenon/messaging/core/message_bus.h>

using namespace kcenon::messaging::collectors;

// Create message bus
auto backend = std::make_shared<standalone_backend>();
auto bus = std::make_shared<message_bus>(backend, message_bus_config{});

// Create and configure collector
message_bus_collector collector;
std::unordered_map<std::string, std::string> config{
    {"enable_latency_tracking", "true"},
    {"latency_sample_size", "1000"},
    {"enable_topic_metrics", "true"}
};
collector.initialize(config);

// Register message bus
collector.set_message_bus(bus);

// Collect metrics
auto metrics = collector.collect();

// Process metrics (e.g., send to Prometheus)
for (const auto& metric : metrics) {
    std::cout << metric.name << ": " << std::get<double>(metric.value) << "\n";
}
```

### Health Monitoring

The `message_bus_health_monitor` class provides anomaly detection:

```cpp
#include <kcenon/messaging/collectors/message_bus_collector.h>

using namespace kcenon::messaging::collectors;

// Configure health thresholds
message_bus_health_thresholds thresholds;
thresholds.queue_saturation_warn = 0.7;      // 70%
thresholds.queue_saturation_critical = 0.9;  // 90%
thresholds.failure_rate_warn = 0.05;         // 5%
thresholds.latency_warn_ms = 100.0;          // 100ms

// Create monitor
message_bus_health_monitor monitor(thresholds);

// Analyze health
message_bus_stats stats = get_current_stats();
auto report = monitor.analyze_health(stats, "primary_bus");

switch (report.status) {
    case message_bus_health_status::healthy:
        std::cout << "Bus is healthy\n";
        break;
    case message_bus_health_status::degraded:
        std::cout << "Bus is degraded: " << report.issues.size() << " issues\n";
        break;
    case message_bus_health_status::unhealthy:
        std::cout << "Bus is unhealthy!\n";
        break;
    case message_bus_health_status::critical:
        std::cout << "Bus is critical!\n";
        break;
}

// Print issues
for (const auto& issue : report.issues) {
    std::cout << "  - " << issue << "\n";
}
```

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_latency_tracking` | bool | true | Enable latency sample collection |
| `latency_sample_size` | size_t | 1000 | Maximum latency samples to keep |
| `enable_topic_metrics` | bool | true | Collect per-topic subscriber counts |
| `use_event_bus` | bool | true | Enable event bus integration |

### Prometheus Naming Convention

All metrics follow Prometheus naming conventions:
- Prefix: `messaging_`
- Suffix for totals: `_total`
- Suffix for latency: `_ms`
- Snake_case naming

---

## Related Documentation

- [Quick Start](../guides/QUICK_START.md)
- [API Reference](../API_REFERENCE.md)
- [Architecture](../ARCHITECTURE.md)
