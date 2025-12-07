# Task Module Workflow Patterns

This guide covers common workflow patterns for the Task module.

## Table of Contents

- [Chain Pattern](#chain-pattern)
- [Chord Pattern](#chord-pattern)
- [Retry Strategies](#retry-strategies)
- [Priority Queues](#priority-queues)
- [Scheduled Tasks](#scheduled-tasks)
- [Progress Tracking](#progress-tracking)
- [Checkpoint and Recovery](#checkpoint-and-recovery)
- [Subtask Spawning](#subtask-spawning)

---

## Chain Pattern

The chain pattern executes tasks sequentially, where the result of one task becomes the input for the next.

### Concept

```
Task A → Task B → Task C
         ↑         ↑
     A's result  B's result
```

### Usage

```cpp
#include <kcenon/messaging/task/task_client.h>

// Define tasks
auto task_a = task_builder("step.one")
    .payload({{"input", "data"}})
    .build().value();

auto task_b = task_builder("step.two").build().value();
auto task_c = task_builder("step.three").build().value();

// Execute chain
auto result = client.chain({task_a, task_b, task_c});

// Wait for final result
auto outcome = result.get(std::chrono::minutes(5));
if (outcome.is_ok()) {
    // This is the result from task_c
    auto final_result = outcome.value();
}
```

### Handler Implementation

```cpp
// Each handler receives the previous task's result
system.register_handler("step.two", [](const task& t, task_context& ctx) {
    // Get result from previous task (step.one)
    auto previous_result = t.payload().get_string("_chain_result");

    // Process and return result for next task
    container_module::value_container result;
    result.add("processed", "data from step two");

    return common::ok(result);
});
```

### Error Handling

If any task in the chain fails, the entire chain stops and the error is propagated:

```cpp
auto result = client.chain({task_a, task_b, task_c});
auto outcome = result.get(timeout);

if (outcome.is_error()) {
    // Chain failed at some point
    std::cerr << "Chain failed: " << result.error_message() << "\n";
}
```

### Use Cases

- Multi-step data processing pipelines
- Sequential workflow execution
- Dependent operations that must run in order

---

## Chord Pattern

The chord pattern executes multiple tasks in parallel, then runs a callback task with the aggregated results.

### Concept

```
Task A ─┐
Task B ─┼→ Callback Task (with all results)
Task C ─┘
```

### Usage

```cpp
// Define parallel tasks
auto task_a = task_builder("process.part1")
    .payload({{"data", "chunk1"}})
    .build().value();

auto task_b = task_builder("process.part2")
    .payload({{"data", "chunk2"}})
    .build().value();

auto task_c = task_builder("process.part3")
    .payload({{"data", "chunk3"}})
    .build().value();

// Define callback task
auto callback = task_builder("aggregate.results").build().value();

// Execute chord
auto result = client.chord({task_a, task_b, task_c}, callback);

// Wait for final result
auto outcome = result.get(std::chrono::minutes(10));
```

### Callback Handler

The callback receives all results as an array:

```cpp
system.register_handler("aggregate.results", [](const task& t, task_context& ctx) {
    // Get all results from parallel tasks
    auto results = t.payload().get_array("_chord_results");

    // Process aggregated results
    int total = 0;
    for (const auto& r : results) {
        total += r.get_int("count").value_or(0);
    }

    container_module::value_container result;
    result.add("total", total);

    return common::ok(result);
});
```

### Error Handling

The chord fails if any parallel task fails:

```cpp
auto result = client.chord(tasks, callback);
auto outcome = result.get(timeout);

if (outcome.is_error()) {
    // One or more parallel tasks failed
    std::cerr << "Chord failed: " << result.error_message() << "\n";
}
```

### Use Cases

- Map-reduce operations
- Parallel data processing
- Aggregating results from multiple sources
- Fan-out/fan-in workflows

---

## Retry Strategies

The Task module supports automatic retry with exponential backoff.

### Default Configuration

```cpp
task_config config;
config.max_retries = 3;
config.retry_delay = std::chrono::milliseconds(1000);
config.retry_backoff_multiplier = 2.0;
```

### Retry Delay Calculation

```
delay = retry_delay * (backoff_multiplier ^ attempt)

Example with defaults:
- Attempt 1 fails → Wait 1000ms
- Attempt 2 fails → Wait 2000ms
- Attempt 3 fails → Wait 4000ms
- Attempt 4 fails → Task marked as failed
```

### Using Task Builder

```cpp
auto task = task_builder("email.send")
    .payload(email_data)
    .retries(5)                              // Max 5 retry attempts
    .retry_delay(std::chrono::seconds(2))    // Start with 2s delay
    .backoff(1.5)                            // 1.5x multiplier
    .build();
```

### Custom Retry Logic

Implement `on_retry` hook for custom behavior:

```cpp
class resilient_handler : public task_handler_interface {
public:
    std::string name() const override { return "resilient.task"; }

    common::Result<value_container> execute(
        const task& t, task_context& ctx) override
    {
        // Task implementation
    }

    void on_retry(const task& t, size_t attempt) override {
        // Log retry attempt
        std::cout << "Retrying task " << t.task_id()
                  << " (attempt " << attempt << ")\n";

        // Could also update metrics, send alerts, etc.
    }
};
```

### Retry vs. No Retry Errors

To prevent retry on certain errors:

```cpp
common::Result<value_container> execute(const task& t, task_context& ctx) override {
    auto validation = validate_input(t.payload());
    if (!validation) {
        // Return error that should not be retried
        // Task will be marked as failed immediately
        return common::error(error_code::validation_failed, validation.error());
    }

    // Transient errors will be retried automatically
    return process_data(t.payload());
}
```

---

## Priority Queues

Tasks can be assigned different priority levels to control execution order.

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

### Setting Priority

```cpp
// Using task builder
auto urgent_task = task_builder("notification.send")
    .payload(notification_data)
    .priority(message_priority::highest)
    .build();

// Using task_config
task t("report.generate", payload);
t.config().priority = message_priority::low;
```

### Priority Queue Setup

```cpp
task_system_config config;
config.worker.queues = {"critical", "high", "default", "low"};
config.worker.concurrency = 8;

task_system system(config);
```

### Submitting to Priority Queues

```cpp
// Critical notifications go to critical queue
auto critical = task_builder("alert.send")
    .queue("critical")
    .priority(message_priority::highest)
    .build();

// Background jobs go to low priority queue
auto background = task_builder("cleanup.logs")
    .queue("low")
    .priority(message_priority::lowest)
    .build();
```

### Best Practices

1. **Don't overuse highest priority** - It defeats the purpose
2. **Match queue to priority** - High priority tasks should go to high priority queues
3. **Monitor queue depths** - Ensure high priority queues aren't starved
4. **Set appropriate timeouts** - Low priority tasks may wait longer

---

## Scheduled Tasks

### Periodic Execution

Execute tasks at fixed intervals:

```cpp
// Every 5 minutes
system.schedule_periodic(
    "cleanup-temp-files",
    task_builder("cleanup.temp").build().value(),
    std::chrono::minutes(5)
);

// Every hour
system.schedule_periodic(
    "sync-data",
    task_builder("sync.external").build().value(),
    std::chrono::hours(1)
);
```

### Cron-Based Scheduling

Use cron expressions for complex schedules:

```cpp
// Daily at 3 AM
system.schedule_cron(
    "daily-report",
    task_builder("report.daily").build().value(),
    "0 3 * * *"
);

// Every Monday at 9 AM
system.schedule_cron(
    "weekly-summary",
    task_builder("report.weekly").build().value(),
    "0 9 * * 1"
);

// Every 15 minutes during business hours (9-17) on weekdays
system.schedule_cron(
    "business-sync",
    task_builder("sync.crm").build().value(),
    "*/15 9-17 * * 1-5"
);
```

### Cron Expression Format

```
* * * * *
│ │ │ │ │
│ │ │ │ └─ Day of week (0-6, 0=Sunday)
│ │ │ └─── Month (1-12)
│ │ └───── Day of month (1-31)
│ └─────── Hour (0-23)
└───────── Minute (0-59)

Supported syntax:
- *     All values
- 5     Specific value
- */15  Every 15 units
- 1-5   Range
- 1,3,5 List
```

### Managing Schedules

```cpp
// Disable a schedule temporarily
system.scheduler()->disable("daily-report");

// Re-enable
system.scheduler()->enable("daily-report");

// Trigger immediately (outside of schedule)
system.scheduler()->trigger_now("daily-report");

// Update interval
system.scheduler()->update_interval("cleanup-temp-files",
    std::chrono::minutes(10));

// Remove schedule
system.scheduler()->remove("obsolete-job");
```

### Delayed Task Execution

Execute a task after a delay:

```cpp
// Execute after 30 seconds
auto result = client.send_later(
    task_builder("reminder.send").payload(data).build().value(),
    std::chrono::seconds(30)
);

// Execute at specific time
auto tomorrow_9am = /* calculate time_point */;
auto result = client.send_at(
    task_builder("meeting.reminder").payload(data).build().value(),
    tomorrow_9am
);
```

---

## Progress Tracking

Report and monitor task progress during execution.

### Reporting Progress

```cpp
system.register_handler("large-file.process", [](const task& t, task_context& ctx) {
    auto file_path = t.payload().get_string("path").value();
    auto total_lines = count_lines(file_path);

    size_t processed = 0;
    for (const auto& line : read_lines(file_path)) {
        process_line(line);
        processed++;

        // Update progress
        double progress = static_cast<double>(processed) / total_lines;
        ctx.update_progress(progress,
            "Processing line " + std::to_string(processed) +
            " of " + std::to_string(total_lines));
    }

    ctx.update_progress(1.0, "Complete");

    container_module::value_container result;
    result.add("lines_processed", processed);
    return common::ok(result);
});
```

### Monitoring Progress

```cpp
auto result = system.submit("large-file.process", payload);

// Poll for progress
while (!result.is_ready()) {
    double progress = result.progress();
    std::string message = result.progress_message();

    std::cout << "\rProgress: " << std::fixed << std::setprecision(1)
              << (progress * 100) << "% - " << message << std::flush;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

std::cout << "\nComplete!\n";
```

### Progress History

```cpp
// Get detailed progress history
auto history = result.progress_history();
for (const auto& entry : history) {
    std::cout << entry.timestamp << ": "
              << (entry.progress * 100) << "% - "
              << entry.message << "\n";
}
```

---

## Checkpoint and Recovery

Save progress for recovery after failures.

### Saving Checkpoints

```cpp
system.register_handler("batch.process", [](const task& t, task_context& ctx) {
    auto items = t.payload().get_array("items");

    // Check if we have a checkpoint from previous attempt
    size_t start_index = 0;
    if (ctx.has_checkpoint()) {
        auto checkpoint = ctx.load_checkpoint();
        start_index = checkpoint.get_size_t("last_processed").value_or(0);
        ctx.log_info("Resuming from checkpoint at index " +
                     std::to_string(start_index));
    }

    // Process items
    for (size_t i = start_index; i < items.size(); ++i) {
        // Check for cancellation
        if (ctx.is_cancelled()) {
            return common::error("Task cancelled");
        }

        process_item(items[i]);

        // Save checkpoint every 10 items
        if ((i + 1) % 10 == 0) {
            container_module::value_container checkpoint;
            checkpoint.add("last_processed", i + 1);
            ctx.save_checkpoint(checkpoint);
        }

        ctx.update_progress(
            static_cast<double>(i + 1) / items.size(),
            "Processed " + std::to_string(i + 1) + " items");
    }

    ctx.clear_checkpoint();

    container_module::value_container result;
    result.add("processed_count", items.size());
    return common::ok(result);
});
```

### Checkpoint Data

Store any data needed for recovery:

```cpp
container_module::value_container checkpoint;
checkpoint.add("current_page", page_number);
checkpoint.add("processed_ids", processed_ids);
checkpoint.add("intermediate_results", partial_results);
ctx.save_checkpoint(checkpoint);
```

---

## Subtask Spawning

Create child tasks from within a handler.

### Spawning Subtasks

```cpp
system.register_handler("order.process", [](const task& t, task_context& ctx) {
    auto order_items = t.payload().get_array("items");

    std::vector<std::string> subtask_ids;

    // Spawn a subtask for each item
    for (const auto& item : order_items) {
        auto subtask = task_builder("item.process")
            .payload(item)
            .build().value();

        auto result = ctx.spawn_subtask(subtask);
        if (result.is_ok()) {
            subtask_ids.push_back(result.value());
        }
    }

    ctx.log_info("Spawned " + std::to_string(subtask_ids.size()) + " subtasks");

    // The parent can continue or wait for subtasks
    container_module::value_container result;
    result.add("subtask_ids", subtask_ids);
    return common::ok(result);
});
```

### Tracking Subtasks

```cpp
auto result = system.submit("order.process", order_data);
auto outcome = result.get(timeout);

// Get child task results
for (const auto& child : result.children()) {
    if (child.is_ready()) {
        auto child_result = child.get(std::chrono::seconds(0));
        // Process child result
    }
}
```

### Use Cases

- Breaking large tasks into smaller units
- Parallel processing within a task
- Dynamic workflow creation

---

## Combining Patterns

### Chain with Retry

```cpp
// Each task in the chain has its own retry policy
auto step1 = task_builder("extract")
    .retries(3)
    .build().value();

auto step2 = task_builder("transform")
    .retries(5)
    .retry_delay(std::chrono::seconds(5))
    .build().value();

auto step3 = task_builder("load")
    .retries(3)
    .build().value();

auto result = client.chain({step1, step2, step3});
```

### Scheduled Chord

```cpp
// Schedule a parallel processing job
auto header_tasks = create_header_tasks();
auto callback = task_builder("aggregate.headers").build().value();

system.schedule_cron(
    "hourly-header-check",
    // Create a chord wrapper task
    task_builder("chord.wrapper")
        .payload({{"header_tasks", header_tasks}, {"callback", callback}})
        .build().value(),
    "0 * * * *"  // Every hour
);
```

---

## Related Documentation

- [Quick Start Guide](QUICK_START.md)
- [Architecture Guide](ARCHITECTURE.md)
- [API Reference](API_REFERENCE.md)
- [Configuration Guide](CONFIGURATION.md)
- [Troubleshooting](TROUBLESHOOTING.md)
