# Task Module Migration Guide

This guide helps you migrate from previous versions of the Task module or from other task queue systems.

## Table of Contents

- [Version Migration](#version-migration)
- [Migration from Custom Solutions](#migration-from-custom-solutions)
- [Migration Checklist](#migration-checklist)

---

## Version Migration

### Migrating to 1.0

If you are upgrading from an earlier development version, follow these steps:

#### API Changes

1. **Result Type Pattern**

   All operations now return `Result<T>` or `VoidResult`:

   ```cpp
   // Before (hypothetical old API)
   task t = create_task("name", payload);  // Could throw

   // After
   auto result = task_builder("name").payload(payload).build();
   if (!result) {
       std::cerr << result.error().message << "\n";
       return;
   }
   task t = result.value();
   ```

2. **Builder Pattern for Tasks**

   ```cpp
   // Before
   task t;
   t.set_name("task.name");
   t.set_timeout(5000);

   // After
   auto t = task_builder("task.name")
       .timeout(std::chrono::milliseconds(5000))
       .build()
       .value();
   ```

3. **Configuration Structure**

   ```cpp
   // Before
   system.set_worker_count(4);
   system.add_queue("default");

   // After
   task_system_config config;
   config.worker.concurrency = 4;
   config.worker.queues = {"default"};
   task_system system(config);
   ```

4. **Handler Registration**

   ```cpp
   // Before
   system.add_handler("name", handler_function);

   // After
   system.register_handler("name", [](const task& t, task_context& ctx) {
       // Implementation
       return common::ok(result);
   });
   ```

#### Configuration Migration

Map your old configuration to the new structure:

| Old Setting | New Setting |
|-------------|-------------|
| `worker_threads` | `config.worker.concurrency` |
| `queue_size` | `config.queue.max_size` |
| `retry_count` | `task_config.max_retries` |
| `timeout_ms` | `task_config.timeout` |

---

## Migration from Custom Solutions

### From Thread Pool Implementation

If you have a custom thread pool, migrate to the Task module:

#### Before (Custom Thread Pool)

```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;

public:
    void enqueue(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
};

ThreadPool pool(4);
pool.enqueue([data]() {
    process_data(data);
});
```

#### After (Task Module)

```cpp
task_system_config config;
config.worker.concurrency = 4;
task_system system(config);

system.register_handler("process.data", [](const task& t, task_context& ctx) {
    auto data = t.payload();
    process_data(data);

    container_module::value_container result;
    result.add("status", "processed");
    return common::ok(result);
});

system.start();

container_module::value_container payload;
payload.add("data", data);
auto result = system.submit("process.data", payload);
```

#### Benefits of Migration

- Automatic retry with exponential backoff
- Progress tracking
- Task persistence
- Monitoring and statistics
- Scheduled execution
- Chain and chord patterns

### From Message Queue Systems

#### Mapping Concepts

| Message Queue Concept | Task Module Equivalent |
|----------------------|------------------------|
| Message | `task` |
| Queue | Named queue in `task_queue` |
| Consumer | `task_handler` |
| Producer | `task_client` |
| Acknowledgment | Automatic on handler completion |
| Dead Letter Queue | Failed tasks in result backend |

#### Migration Example

```cpp
// Before (pseudo-code for message queue)
producer.send("queue-name", message);
consumer.subscribe("queue-name", [](Message& msg) {
    process(msg);
    msg.ack();
});

// After
system.register_handler("process", [](const task& t, task_context& ctx) {
    process(t.payload());
    return common::ok(result);
});

auto task = task_builder("process")
    .payload(message_data)
    .queue("queue-name")
    .build().value();

system.submit(task);
```

### From Cron Jobs

#### Before (System Cron)

```bash
# /etc/crontab
0 3 * * * /usr/bin/my-daily-job
*/15 * * * * /usr/bin/my-sync-job
```

#### After (Task Scheduler)

```cpp
// Daily at 3 AM
system.schedule_cron(
    "daily-job",
    task_builder("daily.task").build().value(),
    "0 3 * * *"
);

// Every 15 minutes
system.schedule_cron(
    "sync-job",
    task_builder("sync.task").build().value(),
    "*/15 * * * *"
);
```

#### Benefits

- Centralized scheduling
- Retry on failure
- Progress monitoring
- No external dependencies
- Programmatic schedule management

---

## Migration Checklist

### Pre-Migration

- [ ] Inventory all existing tasks/jobs
- [ ] Document current retry and timeout behavior
- [ ] Note all queue names and priorities
- [ ] Record current scheduling patterns
- [ ] Backup existing data and configurations

### During Migration

- [ ] Create equivalent handlers for all tasks
- [ ] Configure task_system with appropriate settings
- [ ] Set up equivalent queue structure
- [ ] Migrate scheduling configuration
- [ ] Implement progress tracking where needed
- [ ] Set up monitoring and logging

### Post-Migration

- [ ] Verify all tasks execute correctly
- [ ] Confirm retry behavior matches expectations
- [ ] Check scheduling works as expected
- [ ] Monitor performance and adjust configuration
- [ ] Remove old task infrastructure
- [ ] Update documentation

### Testing Checklist

- [ ] Unit tests for all handlers
- [ ] Integration tests for task flows
- [ ] Retry scenario testing
- [ ] Timeout behavior testing
- [ ] Chain and chord pattern testing
- [ ] Scheduled task testing
- [ ] Performance/load testing

---

## Common Migration Issues

### Handler Throws Instead of Returning Error

```cpp
// Wrong - will cause unexpected behavior
system.register_handler("bad", [](const task& t, task_context& ctx) {
    if (error_condition) {
        throw std::runtime_error("Error!");
    }
    return common::ok(result);
});

// Correct - return error result
system.register_handler("good", [](const task& t, task_context& ctx) {
    if (error_condition) {
        return common::error(error_code::execution_failed, "Error!");
    }
    return common::ok(result);
});
```

### Missing Queue Configuration

```cpp
// Task goes to "special-queue"
auto task = task_builder("task")
    .queue("special-queue")
    .build().value();

// But workers only monitor "default"
worker_config config;
config.queues = {"default"};  // Missing "special-queue"!

// Fix: Add all necessary queues
config.queues = {"default", "special-queue"};
```

### Timeout Too Short

```cpp
// Old system had no timeout, tasks ran forever
// New system defaults to 5 minutes

// For long tasks, increase timeout
auto task = task_builder("long.task")
    .timeout(std::chrono::hours(2))
    .build().value();
```

---

## Related Documentation

- [Quick Start Guide](QUICK_START.md)
- [Architecture Guide](ARCHITECTURE.md)
- [API Reference](API_REFERENCE.md)
- [Configuration Guide](CONFIGURATION.md)
- [Troubleshooting](TROUBLESHOOTING.md)
