# Task Module Troubleshooting Guide

This guide helps you diagnose and resolve common issues with the Task module.

## Table of Contents

- [Common Issues](#common-issues)
- [Debugging Methods](#debugging-methods)
- [Performance Issues](#performance-issues)
- [FAQ](#faq)

---

## Common Issues

### Tasks Not Being Processed

**Symptoms:**
- Tasks remain in the queue
- `async_result.is_ready()` never returns `true`
- Queue size keeps growing

**Possible Causes and Solutions:**

1. **No handler registered for the task name**

   ```cpp
   // Check if handler exists
   if (!system.workers().has_handler("my.task")) {
       std::cerr << "Handler 'my.task' is not registered!\n";
   }

   // List all registered handlers
   auto handlers = system.workers().list_handlers();
   for (const auto& h : handlers) {
       std::cout << "Registered: " << h << "\n";
   }
   ```

2. **System not started**

   ```cpp
   // Ensure the system is running
   if (!system.is_running()) {
       auto result = system.start();
       if (!result) {
           std::cerr << "Failed to start: " << result.error().message << "\n";
       }
   }
   ```

3. **Wrong queue name**

   ```cpp
   // Task is submitted to a queue that workers don't monitor
   auto task = task_builder("my.task")
       .queue("custom-queue")  // Workers might not be listening to this
       .build();

   // Ensure workers monitor the queue
   worker_config config;
   config.queues = {"default", "custom-queue"};  // Add the queue
   ```

4. **All workers are busy**

   ```cpp
   // Check worker status
   auto stats = system.get_statistics();
   std::cout << "Active: " << system.active_workers()
             << " / " << system.total_workers() << "\n";
   ```

### Tasks Failing Immediately

**Symptoms:**
- `async_result.is_failed()` returns `true`
- Tasks complete too quickly
- Error messages appear

**Possible Causes and Solutions:**

1. **Handler throws exception**

   ```cpp
   // Handlers should return Result, not throw
   system.register_handler("safe.task", [](const task& t, task_context& ctx) {
       try {
           // Your logic
           return common::ok(result);
       } catch (const std::exception& e) {
           return common::error(error_code::execution_failed, e.what());
       }
   });
   ```

2. **Invalid payload data**

   ```cpp
   // Validate payload before processing
   auto required_field = t.payload().get_string("required");
   if (!required_field) {
       return common::error(error_code::validation_failed,
                          "Missing required field");
   }
   ```

3. **Resource unavailable**

   ```cpp
   // Check external dependencies
   if (!database.is_connected()) {
       ctx.log_error("Database connection lost");
       return common::error(error_code::resource_unavailable,
                          "Database not available");
   }
   ```

### Tasks Timing Out

**Symptoms:**
- Tasks run for the full timeout duration then fail
- `error_message()` mentions timeout

**Possible Causes and Solutions:**

1. **Handler takes too long**

   ```cpp
   // Increase timeout for long tasks
   auto task = task_builder("long.task")
       .timeout(std::chrono::minutes(30))
       .build();
   ```

2. **Handler is blocked**

   Check if the handler:
   - Waits on an external service that's unresponsive
   - Has a deadlock in its logic
   - Is stuck in an infinite loop

3. **Check for blocking operations**

   ```cpp
   // Use async operations where possible
   // Check for cancellation in long loops
   for (size_t i = 0; i < items.size(); ++i) {
       if (ctx.is_cancelled()) {
           return common::error("Cancelled");
       }
       process_item(items[i]);
       ctx.update_progress(double(i) / items.size(), "Processing...");
   }
   ```

### Retry Not Working

**Symptoms:**
- Failed tasks are not retried
- Retry count stays at 0

**Possible Causes and Solutions:**

1. **max_retries set to 0**

   ```cpp
   auto task = task_builder("retryable.task")
       .retries(3)  // Enable retries
       .retry_delay(std::chrono::seconds(5))
       .build();
   ```

2. **Non-retryable error returned**

   Some errors may be configured to skip retry. Ensure your handler returns appropriate errors.

3. **Task already at max retries**

   ```cpp
   // Check attempt count
   std::cout << "Attempt: " << t.attempt_count()
             << " / " << t.config().max_retries << "\n";
   ```

### Memory Usage Growing

**Symptoms:**
- Process memory increases over time
- System becomes slow
- Eventually runs out of memory

**Possible Causes and Solutions:**

1. **Results not being cleaned up**

   ```cpp
   // Periodically cleanup old results
   system.results()->cleanup_expired(std::chrono::hours(24));
   ```

2. **Queue growing unboundedly**

   ```cpp
   // Set maximum queue size
   task_queue_config queue_config;
   queue_config.max_size = 100000;
   ```

3. **Progress history accumulating**

   Progress updates are stored in memory. For very long tasks with frequent updates, this can accumulate.

4. **Enable persistence to offload memory**

   ```cpp
   task_queue_config queue_config;
   queue_config.enable_persistence = true;
   queue_config.persistence_path = "/var/lib/task_queue/";
   ```

---

## Debugging Methods

### Enable Logging

Use the task context to log information:

```cpp
system.register_handler("debug.task", [](const task& t, task_context& ctx) {
    ctx.log_info("Starting task: " + t.task_id());
    ctx.log_info("Payload: " + t.payload().to_string());

    // ... processing ...

    ctx.log_info("Task completed successfully");
    return common::ok(result);
});
```

### Monitor Events

Subscribe to task events for debugging:

```cpp
auto monitor = system.monitor();
if (monitor) {
    monitor->on_task_started([](const task& t) {
        std::cout << "[STARTED] " << t.task_name()
                  << " (ID: " << t.task_id() << ")\n";
    });

    monitor->on_task_completed([](const task& t, bool success) {
        std::cout << "[" << (success ? "SUCCESS" : "FAILURE") << "] "
                  << t.task_name() << "\n";
    });

    monitor->on_task_failed([](const task& t, const std::string& error) {
        std::cout << "[FAILED] " << t.task_name()
                  << ": " << error << "\n";
    });
}
```

### Check Queue Status

```cpp
// Get queue statistics
auto monitor = system.monitor();
if (monitor) {
    auto stats = monitor->get_queue_stats();
    for (const auto& q : stats) {
        std::cout << "Queue '" << q.name << "': "
                  << "pending=" << q.pending_count
                  << ", running=" << q.running_count
                  << ", delayed=" << q.delayed_count << "\n";
    }
}
```

### Check Worker Status

```cpp
auto stats = system.get_statistics();
std::cout << "Tasks processed: " << stats.total_tasks_processed << "\n";
std::cout << "Tasks succeeded: " << stats.total_tasks_succeeded << "\n";
std::cout << "Tasks failed: " << stats.total_tasks_failed << "\n";
std::cout << "Tasks retried: " << stats.total_tasks_retried << "\n";
std::cout << "Avg execution time: " << stats.avg_execution_time.count() << "ms\n";
```

### View Task Logs

Access logs from completed tasks:

```cpp
// Logs are stored in task_context
system.register_handler("logging.task", [](const task& t, task_context& ctx) {
    ctx.log_info("Step 1 complete");
    ctx.log_warning("Slow response from service");
    ctx.log_error("Failed to connect, retrying");

    // After execution, logs are available
    auto logs = ctx.logs();
    for (const auto& entry : logs) {
        std::cout << "[" << entry.timestamp << "] "
                  << level_to_string(entry.log_level) << ": "
                  << entry.message << "\n";
    }

    return common::ok(result);
});
```

---

## Performance Issues

### High CPU Usage

**Possible Causes:**

1. **Too many workers**

   ```cpp
   // Reduce concurrency
   worker_config config;
   config.concurrency = std::thread::hardware_concurrency() / 2;
   ```

2. **Poll interval too short**

   ```cpp
   // Increase poll interval
   worker_config config;
   config.poll_interval = std::chrono::milliseconds(100);
   ```

3. **CPU-intensive handlers**

   Consider moving heavy computation to dedicated threads or batching operations.

### High Memory Usage

**Possible Causes:**

1. **Large payloads in queue**

   ```cpp
   // Limit queue size
   task_queue_config config;
   config.max_size = 10000;
   ```

2. **Too many prefetched tasks**

   ```cpp
   // Reduce prefetch count
   worker_config config;
   config.prefetch_count = 5;
   ```

3. **Result accumulation**

   ```cpp
   // Clean up old results
   system.results()->cleanup_expired(std::chrono::hours(1));
   ```

### Low Throughput

**Possible Causes:**

1. **Insufficient workers**

   ```cpp
   // Increase concurrency
   worker_config config;
   config.concurrency = std::thread::hardware_concurrency() * 2;
   ```

2. **I/O bottleneck in handlers**

   Use async I/O or batch operations where possible.

3. **Prefetch disabled**

   ```cpp
   // Enable prefetching
   worker_config config;
   config.prefetch = true;
   config.prefetch_count = 20;
   ```

4. **Single queue bottleneck**

   ```cpp
   // Use multiple queues
   worker_config config;
   config.queues = {"high", "default", "low"};
   ```

### High Latency

**Possible Causes:**

1. **Long poll interval**

   ```cpp
   // Reduce poll interval
   worker_config config;
   config.poll_interval = std::chrono::milliseconds(10);
   ```

2. **Queue depth too high**

   Monitor queue depth and scale workers or reject new tasks when overloaded.

3. **Prefetch holding tasks**

   ```cpp
   // Disable prefetch for latency-sensitive queues
   worker_config config;
   config.prefetch = false;
   ```

---

## FAQ

### Q: How do I cancel a running task?

Tasks can be cancelled, but the handler must check for cancellation:

```cpp
system.register_handler("cancellable.task", [](const task& t, task_context& ctx) {
    while (processing) {
        if (ctx.is_cancelled()) {
            // Cleanup and return
            return common::error("Task cancelled by user");
        }
        // Continue processing
    }
    return common::ok(result);
});

// Request cancellation
auto result = system.submit("cancellable.task", payload);
result.revoke();  // This sets the cancellation flag
```

### Q: How do I get task progress from outside the handler?

```cpp
auto result = system.submit("long.task", payload);

// Poll for progress
while (!result.is_ready()) {
    double progress = result.progress();
    std::string message = result.progress_message();
    std::cout << "Progress: " << (progress * 100) << "% - " << message << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

### Q: How do I handle task dependencies?

Use the chain pattern:

```cpp
auto step1 = task_builder("step1").payload(data).build().value();
auto step2 = task_builder("step2").build().value();
auto step3 = task_builder("step3").build().value();

auto result = client.chain({step1, step2, step3});
// step2 receives step1's result, step3 receives step2's result
```

### Q: How do I process tasks in order?

Set `concurrency = 1` for strict ordering:

```cpp
worker_config config;
config.concurrency = 1;  // Single worker ensures order
config.queues = {"ordered-queue"};
```

### Q: How do I prioritize certain tasks?

Use priority levels and queue names:

```cpp
auto urgent = task_builder("urgent.task")
    .priority(message_priority::highest)
    .queue("critical")
    .build();

auto normal = task_builder("normal.task")
    .priority(message_priority::normal)
    .queue("default")
    .build();

// Configure workers to process critical queue first
worker_config config;
config.queues = {"critical", "default", "background"};
```

### Q: How do I limit retry attempts?

Configure via task_config:

```cpp
auto task = task_builder("limited.retry")
    .retries(1)  // Only retry once
    .build();

// Or disable retries entirely
auto task = task_builder("no.retry")
    .retries(0)  // No retries
    .build();
```

### Q: How do I handle poison messages (tasks that always fail)?

Use the `on_failure` hook to track failures:

```cpp
class monitored_handler : public task_handler_interface {
    std::unordered_map<std::string, size_t> failure_counts_;

public:
    void on_failure(const task& t, const std::string& error) override {
        failure_counts_[t.task_name()]++;

        if (failure_counts_[t.task_name()] > 10) {
            // Move to dead letter queue or alert
            log_alert("Task " + t.task_name() + " failing repeatedly");
        }
    }
};
```

### Q: How do I gracefully shut down the system?

```cpp
// Graceful shutdown waits for current tasks to complete
system.shutdown_graceful(std::chrono::seconds(30));

// This will:
// 1. Stop accepting new tasks
// 2. Wait up to 30 seconds for running tasks to complete
// 3. Force stop if timeout is reached
```

---

## Related Documentation

- [Quick Start Guide](QUICK_START.md)
- [Architecture Guide](ARCHITECTURE.md)
- [API Reference](API_REFERENCE.md)
- [Patterns Guide](PATTERNS.md)
- [Configuration Guide](CONFIGURATION.md)
