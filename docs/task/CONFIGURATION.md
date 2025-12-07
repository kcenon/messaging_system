# Task Module Configuration Guide

This guide covers all configuration options for the Task module.

## Table of Contents

- [Task System Configuration](#task-system-configuration)
- [Task Queue Configuration](#task-queue-configuration)
- [Worker Pool Configuration](#worker-pool-configuration)
- [Task Configuration](#task-configuration)
- [Environment-Specific Settings](#environment-specific-settings)
- [Performance Tuning](#performance-tuning)

---

## Task System Configuration

The `task_system_config` is the main configuration structure for the entire Task system.

### Structure

```cpp
struct task_system_config {
    task_queue_config queue;
    worker_config worker;
    bool enable_scheduler = true;
    bool enable_monitoring = true;
    std::string result_backend_type = "memory";
};
```

### Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `queue` | `task_queue_config` | (see below) | Queue configuration |
| `worker` | `worker_config` | (see below) | Worker pool configuration |
| `enable_scheduler` | `bool` | `true` | Enable task scheduler |
| `enable_monitoring` | `bool` | `true` | Enable monitoring component |
| `result_backend_type` | `std::string` | `"memory"` | Result storage backend type |

### Example

```cpp
task_system_config config;

// Queue settings
config.queue.max_size = 50000;
config.queue.enable_delayed_queue = true;

// Worker settings
config.worker.concurrency = 8;
config.worker.queues = {"default", "high-priority", "background"};

// Enable components
config.enable_scheduler = true;
config.enable_monitoring = true;
config.result_backend_type = "memory";

task_system system(config);
```

---

## Task Queue Configuration

The `task_queue_config` controls the task queue behavior.

### Structure

```cpp
struct task_queue_config {
    size_t max_size = 100000;
    bool enable_persistence = false;
    std::string persistence_path;
    bool enable_delayed_queue = true;
    std::chrono::milliseconds delayed_poll_interval{1000};
};
```

### Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `max_size` | `size_t` | `100000` | Maximum number of tasks in queue |
| `enable_persistence` | `bool` | `false` | Enable queue persistence |
| `persistence_path` | `std::string` | `""` | Path for persistence storage |
| `enable_delayed_queue` | `bool` | `true` | Enable delayed task scheduling |
| `delayed_poll_interval` | `milliseconds` | `1000` | Poll interval for delayed tasks |

### Persistence Configuration

When persistence is enabled, tasks survive process restarts:

```cpp
task_queue_config queue_config;
queue_config.enable_persistence = true;
queue_config.persistence_path = "/var/lib/task_queue/";
queue_config.max_size = 500000;
```

### Delayed Queue Settings

For scheduling tasks to run later:

```cpp
task_queue_config queue_config;
queue_config.enable_delayed_queue = true;
queue_config.delayed_poll_interval = std::chrono::milliseconds(500);
```

---

## Worker Pool Configuration

The `worker_config` controls how tasks are executed.

### Structure

```cpp
struct worker_config {
    size_t concurrency = std::thread::hardware_concurrency();
    std::vector<std::string> queues = {"default"};
    std::chrono::milliseconds poll_interval{100};
    bool prefetch = true;
    size_t prefetch_count = 10;
};
```

### Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `concurrency` | `size_t` | CPU cores | Number of worker threads |
| `queues` | `vector<string>` | `{"default"}` | Queues to process |
| `poll_interval` | `milliseconds` | `100` | Time between polling for tasks |
| `prefetch` | `bool` | `true` | Enable task prefetching |
| `prefetch_count` | `size_t` | `10` | Number of tasks to prefetch |

### Concurrency Settings

Match concurrency to your workload:

```cpp
worker_config worker;

// CPU-bound tasks: use CPU core count
worker.concurrency = std::thread::hardware_concurrency();

// I/O-bound tasks: can use more threads
worker.concurrency = std::thread::hardware_concurrency() * 2;

// Limited resources: constrain threads
worker.concurrency = 4;
```

### Queue Priority

Queues are processed in the order specified:

```cpp
worker_config worker;

// Process critical first, then high, then default
worker.queues = {"critical", "high", "default", "low"};
```

### Prefetch Settings

Prefetching improves throughput for short tasks:

```cpp
worker_config worker;

// Enable prefetching with 20 tasks ahead
worker.prefetch = true;
worker.prefetch_count = 20;

// Disable for long-running tasks
worker.prefetch = false;
```

---

## Task Configuration

Individual task settings via `task_config`.

### Structure

```cpp
struct task_config {
    std::chrono::milliseconds timeout{300000};
    size_t max_retries = 3;
    std::chrono::milliseconds retry_delay{1000};
    double retry_backoff_multiplier = 2.0;
    message_priority priority = message_priority::normal;
    std::optional<std::chrono::system_clock::time_point> eta;
    std::optional<std::chrono::milliseconds> expires;
    std::string queue_name = "default";
    std::vector<std::string> tags;
};
```

### Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `timeout` | `milliseconds` | `300000` (5min) | Task execution timeout |
| `max_retries` | `size_t` | `3` | Maximum retry attempts |
| `retry_delay` | `milliseconds` | `1000` | Initial retry delay |
| `retry_backoff_multiplier` | `double` | `2.0` | Exponential backoff multiplier |
| `priority` | `message_priority` | `normal` | Task priority level |
| `eta` | `optional<time_point>` | `nullopt` | Scheduled execution time |
| `expires` | `optional<milliseconds>` | `nullopt` | Task expiration duration |
| `queue_name` | `string` | `"default"` | Target queue |
| `tags` | `vector<string>` | `{}` | Task tags |

### Using Task Builder

```cpp
auto task = task_builder("process.data")
    .payload(data)
    .timeout(std::chrono::minutes(10))
    .retries(5)
    .retry_delay(std::chrono::seconds(5))
    .backoff(1.5)
    .priority(message_priority::high)
    .queue("processing")
    .tag("batch")
    .tag("important")
    .expires_in(std::chrono::hours(1))
    .build();
```

### Priority Levels

```cpp
enum class message_priority {
    lowest = 1,
    low = 3,
    normal = 5,    // Default
    high = 7,
    highest = 9
};
```

---

## Environment-Specific Settings

### Development Environment

```cpp
task_system_config dev_config;

// Fewer workers for debugging
dev_config.worker.concurrency = 2;
dev_config.worker.queues = {"default"};

// Smaller queue for faster feedback
dev_config.queue.max_size = 1000;

// Short poll interval for responsiveness
dev_config.worker.poll_interval = std::chrono::milliseconds(50);

// Enable all monitoring
dev_config.enable_monitoring = true;
dev_config.enable_scheduler = true;
```

### Staging Environment

```cpp
task_system_config staging_config;

// Moderate resources
staging_config.worker.concurrency = std::thread::hardware_concurrency() / 2;
staging_config.worker.queues = {"critical", "default", "background"};

// Production-like queue settings
staging_config.queue.max_size = 50000;
staging_config.queue.enable_delayed_queue = true;

// Enable persistence for testing
staging_config.queue.enable_persistence = true;
staging_config.queue.persistence_path = "/tmp/task_queue/";
```

### Production Environment

```cpp
task_system_config prod_config;

// Full resources
prod_config.worker.concurrency = std::thread::hardware_concurrency();
prod_config.worker.queues = {"critical", "high", "default", "low", "background"};

// Large queue capacity
prod_config.queue.max_size = 500000;
prod_config.queue.enable_delayed_queue = true;

// Enable persistence
prod_config.queue.enable_persistence = true;
prod_config.queue.persistence_path = "/var/lib/messaging/task_queue/";

// Prefetch for throughput
prod_config.worker.prefetch = true;
prod_config.worker.prefetch_count = 50;

// Enable monitoring
prod_config.enable_monitoring = true;
prod_config.enable_scheduler = true;
```

---

## Performance Tuning

### High Throughput Configuration

For processing many short tasks:

```cpp
task_system_config high_throughput;

// More workers
high_throughput.worker.concurrency = std::thread::hardware_concurrency() * 2;

// Aggressive prefetching
high_throughput.worker.prefetch = true;
high_throughput.worker.prefetch_count = 100;

// Fast polling
high_throughput.worker.poll_interval = std::chrono::milliseconds(10);

// Large queue
high_throughput.queue.max_size = 1000000;

// Fast delayed queue polling
high_throughput.queue.delayed_poll_interval = std::chrono::milliseconds(100);
```

### Low Latency Configuration

For quick task execution with minimal delay:

```cpp
task_system_config low_latency;

// Dedicated workers
low_latency.worker.concurrency = std::thread::hardware_concurrency();

// Fast polling
low_latency.worker.poll_interval = std::chrono::milliseconds(1);

// Disable prefetch for immediate processing
low_latency.worker.prefetch = false;

// Single high-priority queue
low_latency.worker.queues = {"realtime"};
```

### Resource-Constrained Configuration

For limited CPU/memory environments:

```cpp
task_system_config constrained;

// Limited workers
constrained.worker.concurrency = 2;

// Single queue
constrained.worker.queues = {"default"};

// Smaller queue
constrained.queue.max_size = 10000;

// Less aggressive prefetching
constrained.worker.prefetch = true;
constrained.worker.prefetch_count = 5;

// Longer poll interval
constrained.worker.poll_interval = std::chrono::milliseconds(500);

// Disable optional components
constrained.enable_scheduler = false;
constrained.enable_monitoring = false;
```

### Long-Running Tasks Configuration

For tasks that take minutes to hours:

```cpp
task_system_config long_running;

// Fewer but dedicated workers
long_running.worker.concurrency = 4;

// Disable prefetch (one task per worker is enough)
long_running.worker.prefetch = false;

// Longer poll interval (tasks are long anyway)
long_running.worker.poll_interval = std::chrono::seconds(1);

// Task defaults for long operations
// (Use task_builder for individual tasks)
```

### Memory Optimization

```cpp
task_system_config memory_optimized;

// Limit queue size
memory_optimized.queue.max_size = 10000;

// Smaller prefetch buffer
memory_optimized.worker.prefetch_count = 5;

// Enable persistence to offload memory
memory_optimized.queue.enable_persistence = true;
memory_optimized.queue.persistence_path = "/var/lib/task_queue/";
```

---

## Configuration Recommendations

### By Workload Type

| Workload | Concurrency | Prefetch | Poll Interval | Queue Size |
|----------|-------------|----------|---------------|------------|
| CPU-bound | CPU cores | Medium | 100ms | Medium |
| I/O-bound | 2x CPU cores | High | 50ms | Large |
| Mixed | 1.5x CPU cores | Medium | 100ms | Medium |
| Long-running | Low (2-4) | Disabled | 1s | Small |
| Real-time | CPU cores | Disabled | 1-10ms | Small |

### By Scale

| Scale | Workers | Queue Size | Persistence | Monitoring |
|-------|---------|------------|-------------|------------|
| Small (<100 tasks/min) | 2-4 | 10,000 | Optional | Optional |
| Medium (100-1000/min) | 4-8 | 50,000 | Recommended | Recommended |
| Large (1000-10000/min) | 8-16 | 500,000 | Required | Required |
| Very Large (>10000/min) | 16+ | 1,000,000+ | Required | Required |

---

## Related Documentation

- [Quick Start Guide](QUICK_START.md)
- [Architecture Guide](ARCHITECTURE.md)
- [API Reference](API_REFERENCE.md)
- [Patterns Guide](PATTERNS.md)
- [Troubleshooting](TROUBLESHOOTING.md)
