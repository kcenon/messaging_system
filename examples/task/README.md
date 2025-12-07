# Task Module Examples

This directory contains example applications demonstrating various features of the Task module in the messaging_system.

## Examples Overview

| Example | Description |
|---------|-------------|
| [simple_worker](simple_worker/) | Basic task system usage with handler registration and task submission |
| [priority_tasks](priority_tasks/) | Priority-based task processing and scheduling |
| [scheduled_tasks](scheduled_tasks/) | Delayed and periodic task execution with cron expressions |
| [chain_workflow](chain_workflow/) | Sequential task execution (ETL pipeline pattern) |
| [chord_aggregation](chord_aggregation/) | Parallel execution with result aggregation |
| [progress_tracking](progress_tracking/) | Real-time progress updates for long-running tasks |
| [monitoring_dashboard](monitoring_dashboard/) | Console-based monitoring dashboard |

## Building Examples

From the project root:

```bash
mkdir build && cd build
cmake .. -DBUILD_SAMPLES=ON
cmake --build . --target example_task_simple_worker
cmake --build . --target example_task_priority_tasks
# ... or build all examples
cmake --build .
```

## Running Examples

After building:

```bash
./bin/example_task_simple_worker
./bin/example_task_priority_tasks
./bin/example_task_scheduled_tasks
./bin/example_task_chain_workflow
./bin/example_task_chord_aggregation
./bin/example_task_progress_tracking
./bin/example_task_monitoring_dashboard
```

## Key Concepts

### Task System

The `task_system` class is the main entry point for the Task module. It integrates:
- Task queue for storing pending tasks
- Worker pool for executing tasks
- Task scheduler for periodic/delayed execution
- Task monitor for system statistics

### Handler Registration

Handlers process tasks by name:

```cpp
system.register_handler("task.name", [](const task& t, task_context& ctx) {
    // Process task
    container_module::value_container result;
    result.set_value("status", std::string("done"));
    return common::ok(result);
});
```

### Task Submission

Submit tasks for immediate or delayed execution:

```cpp
// Immediate
auto result = system.submit("task.name", payload);

// Delayed
system.submit_later(task, std::chrono::seconds(5));

// Scheduled (periodic)
system.schedule_periodic("schedule-name", task, std::chrono::minutes(1));
```

### Workflow Patterns

Chain pattern (sequential):
```cpp
auto result = client.chain({task1, task2, task3});
```

Chord pattern (parallel with aggregation):
```cpp
auto result = client.chord({task1, task2, task3}, callback_task);
```

## Common Patterns

### Error Handling

```cpp
auto outcome = result.get(std::chrono::seconds(10));
if (outcome.is_ok()) {
    auto value = outcome.unwrap();
    // Handle success
} else {
    std::cerr << "Error: " << outcome.error().message << std::endl;
}
```

### Progress Tracking

```cpp
system.register_handler("long.task", [](const task& t, task_context& ctx) {
    for (int i = 0; i <= 100; ++i) {
        ctx.update_progress(i / 100.0, "Step " + std::to_string(i));
        // Do work...
    }
    return ok(result);
});
```

### Cancellation Checking

```cpp
system.register_handler("cancellable", [](const task& t, task_context& ctx) {
    while (!done) {
        if (ctx.is_cancelled()) {
            return error("Task cancelled");
        }
        // Continue work...
    }
    return ok(result);
});
```
