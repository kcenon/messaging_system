# Task Module Architecture

This document describes the architecture and design of the Task module.

## Overview

The Task module provides a distributed task queue system for asynchronous task execution. It is designed for reliability, scalability, and ease of use.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                          task_system (Facade)                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────────┐  │
│  │ task_client │    │ task_queue  │    │      worker_pool        │  │
│  │             │───▶│             │───▶│                         │  │
│  │ Submit API  │    │ Task Store  │    │ ┌─────────────────────┐ │  │
│  └─────────────┘    └─────────────┘    │ │   task_handler(s)   │ │  │
│         │                   ▲          │ └─────────────────────┘ │  │
│         │                   │          │           │             │  │
│         ▼                   │          │           ▼             │  │
│  ┌─────────────┐            │          │ ┌─────────────────────┐ │  │
│  │async_result │            │          │ │   task_context      │ │  │
│  └─────────────┘            │          │ └─────────────────────┘ │  │
│         │                   │          └─────────────────────────┘  │
│         │                   │                      │                │
│         ▼                   │                      ▼                │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                     result_backend                           │   │
│  │                (memory_result_backend)                       │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
│  ┌──────────────────┐              ┌──────────────────┐            │
│  │  task_scheduler  │              │   task_monitor   │            │
│  │                  │              │                  │            │
│  │ Periodic/Cron    │              │ Stats & Events   │            │
│  └──────────────────┘              └──────────────────┘            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. task_system (Facade)

The `task_system` is the main entry point that integrates all components. It provides:

- Unified lifecycle management (start/stop)
- Convenience methods for common operations
- Access to individual components when needed

```cpp
task_system system(config);
system.register_handler("task.name", handler);
system.start();
auto result = system.submit("task.name", payload);
system.stop();
```

### 2. task

The `task` class represents a unit of work. It extends the `message` class from the messaging infrastructure.

**Key Properties:**
- `task_id`: Unique identifier (UUID)
- `task_name`: Handler name to execute
- `payload`: Input data (value_container)
- `config`: Execution configuration
- `state`: Current lifecycle state

**Task States:**

```
                 ┌──────────────────────────────────────────────────┐
                 │                                                  │
                 ▼                                                  │
┌─────────┐   ┌────────┐   ┌─────────┐   ┌───────────┐   ┌─────────┴───┐
│ PENDING │──▶│ QUEUED │──▶│ RUNNING │──▶│ SUCCEEDED │   │   RETRYING  │
└─────────┘   └────────┘   └────┬────┘   └───────────┘   └─────────────┘
                 │              │                              ▲
                 │              ▼                              │
                 │        ┌──────────┐                         │
                 │        │  FAILED  │─────────────────────────┘
                 │        └────┬─────┘        (if retries left)
                 │             │
                 │             ▼ (no retries)
                 │        ┌──────────┐
                 │        │  FAILED  │ (terminal)
                 │        └──────────┘
                 │
                 ▼
           ┌───────────┐        ┌──────────┐
           │ CANCELLED │        │ EXPIRED  │
           └───────────┘        └──────────┘
```

### 3. task_queue

The `task_queue` manages multiple named queues with priority support.

**Features:**
- Multiple named queues (e.g., "default", "high-priority")
- Delayed task scheduling using ETA
- Task cancellation by ID or tag
- Thread-safe operations

**Data Flow:**

```
┌──────────────────────────────────────────────────────────┐
│                       task_queue                          │
├──────────────────────────────────────────────────────────┤
│                                                          │
│   ┌─────────────────────────────────────────────────┐   │
│   │              Immediate Queues                    │   │
│   │  ┌─────────┐  ┌─────────┐  ┌─────────────────┐  │   │
│   │  │ default │  │  high   │  │ custom-queue    │  │   │
│   │  │ queue   │  │priority │  │                 │  │   │
│   │  └─────────┘  └─────────┘  └─────────────────┘  │   │
│   └─────────────────────────────────────────────────┘   │
│                          ▲                               │
│                          │ (when ETA reached)           │
│   ┌─────────────────────────────────────────────────┐   │
│   │              Delayed Queue                       │   │
│   │  Tasks waiting for their scheduled time          │   │
│   └─────────────────────────────────────────────────┘   │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

### 4. worker_pool

The `worker_pool` executes tasks using a thread pool.

**Responsibilities:**
- Manage worker threads
- Fetch tasks from queues
- Match tasks to registered handlers
- Handle retries with exponential backoff
- Collect execution statistics

**Execution Flow:**

```
┌────────────────────────────────────────────────────────────────┐
│                         worker_pool                             │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐          │
│   │Worker 1 │  │Worker 2 │  │Worker 3 │  │Worker N │          │
│   └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘          │
│        │            │            │            │                │
│        └────────────┴─────┬──────┴────────────┘                │
│                           │                                    │
│                           ▼                                    │
│                  ┌────────────────┐                            │
│                  │ Dequeue Task   │◀───── task_queue           │
│                  └───────┬────────┘                            │
│                          │                                     │
│                          ▼                                     │
│                  ┌────────────────┐                            │
│                  │ Find Handler   │                            │
│                  └───────┬────────┘                            │
│                          │                                     │
│                          ▼                                     │
│                  ┌────────────────┐                            │
│                  │ Execute Task   │                            │
│                  └───────┬────────┘                            │
│                          │                                     │
│              ┌───────────┼───────────┐                         │
│              ▼           ▼           ▼                         │
│         ┌────────┐  ┌────────┐  ┌────────┐                    │
│         │Success │  │ Retry  │  │ Failed │                    │
│         └───┬────┘  └───┬────┘  └───┬────┘                    │
│             │           │           │                          │
│             └───────────┴───────────┘                          │
│                         │                                      │
│                         ▼                                      │
│                  result_backend                                │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

### 5. task_handler

The `task_handler_interface` defines how tasks are executed.

**Interface:**

```cpp
class task_handler_interface {
public:
    virtual std::string name() const = 0;

    virtual common::Result<value_container> execute(
        const task& t,
        task_context& ctx) = 0;

    // Optional lifecycle hooks
    virtual void on_retry(const task& t, size_t attempt) { }
    virtual void on_failure(const task& t, const std::string& error) { }
    virtual void on_success(const task& t, const value_container& result) { }
};
```

**Handler Types:**
- Class-based: Inherit from `task_handler_interface`
- Lambda-based: Use `make_handler()` or register directly

### 6. task_context

The `task_context` provides execution context to handlers.

**Features:**
- Progress tracking and reporting
- Checkpoint management for recovery
- Subtask spawning
- Cancellation detection
- Logging

```cpp
common::Result<value_container> execute(const task& t, task_context& ctx) {
    // Update progress
    ctx.update_progress(0.0, "Starting...");

    // Check for cancellation
    if (ctx.is_cancelled()) {
        return common::error("Cancelled");
    }

    // Save checkpoint for recovery
    ctx.save_checkpoint({{"step", current_step}});

    // Spawn subtask
    ctx.spawn_subtask(subtask);

    // Log information
    ctx.log_info("Processing complete");

    return common::ok(result);
}
```

### 7. result_backend

The `result_backend_interface` provides storage for task results.

**Implementations:**
- `memory_result_backend`: In-memory storage (default)
- Custom implementations possible (Redis, database, etc.)

**Stored Data:**
- Task state
- Task result
- Error information
- Progress data

### 8. async_result

The `async_result` provides a handle for tracking task execution.

**Usage Patterns:**

```cpp
// Polling
while (!result.is_ready()) {
    std::cout << result.progress() << "%\n";
}

// Blocking wait
auto outcome = result.get(timeout);

// Callback-based
result.then(
    [](const auto& value) { /* success */ },
    [](const auto& error) { /* failure */ }
);
```

### 9. task_scheduler

The `task_scheduler` manages periodic and cron-based task execution.

**Schedule Types:**
- Periodic: Fixed interval (e.g., every 5 minutes)
- Cron: Cron expression (e.g., "0 3 * * *" for daily at 3 AM)

```
┌────────────────────────────────────────┐
│            task_scheduler               │
├────────────────────────────────────────┤
│                                         │
│  ┌───────────────────────────────────┐ │
│  │         Schedule Registry          │ │
│  │                                    │ │
│  │  ┌────────────────────────────┐   │ │
│  │  │ cleanup-job (periodic: 5m) │   │ │
│  │  ├────────────────────────────┤   │ │
│  │  │ daily-report (cron: 0 3 *) │   │ │
│  │  ├────────────────────────────┤   │ │
│  │  │ weekly-summary (cron: ...)  │   │ │
│  │  └────────────────────────────┘   │ │
│  └───────────────────────────────────┘ │
│                    │                    │
│                    ▼                    │
│           ┌────────────────┐           │
│           │ Scheduler Loop │           │
│           │ (check times)  │           │
│           └───────┬────────┘           │
│                   │                    │
│                   ▼                    │
│            task_client.send()          │
│                                         │
└────────────────────────────────────────┘
```

### 10. task_monitor

The `task_monitor` provides observability into the system.

**Features:**
- Queue statistics
- Worker status
- Active/pending/failed task listings
- Event subscriptions

## Data Flow

### Task Submission Flow

```
1. Client creates task with payload
         │
         ▼
2. task_client.send(task)
         │
         ▼
3. task_queue.enqueue(task)
   - Assign task_id if not set
   - Check for delayed execution (ETA)
   - Add to appropriate queue
         │
         ▼
4. async_result returned to client
         │
         ▼
5. Worker dequeues task
         │
         ▼
6. Worker finds matching handler
         │
         ▼
7. Handler executes with task_context
         │
         ▼
8. Result stored in result_backend
         │
         ▼
9. async_result.get() returns result
```

### Retry Flow

```
1. Task execution fails
         │
         ▼
2. Check retry policy
   - attempts < max_retries?
         │
    ┌────┴────┐
    │         │
   Yes        No
    │         │
    ▼         ▼
3a. Calculate   3b. Mark as
    retry delay     FAILED
    │               │
    ▼               ▼
4a. Set state   4b. Store error
    to RETRYING     in backend
    │
    ▼
5a. Re-queue with delay
    │
    ▼
6a. Worker picks up again
```

## Thread Safety

All public APIs are thread-safe:

| Component | Thread Safety Mechanism |
|-----------|------------------------|
| task_queue | Mutex-protected internal state, thread_system integration |
| worker_pool | Concurrent worker execution |
| async_result | Atomic state + condition variables |
| task_context | Atomic progress updates |
| memory_result_backend | shared_mutex for R/W locking |
| task_scheduler | Mutex-protected schedules |

### thread_system Integration

The `task_queue` component uses `thread_system` for managing the delayed task worker thread. Instead of direct `std::thread` usage, it leverages `kcenon::thread::thread_base` which provides:

- Standardized thread lifecycle management (start/stop)
- Proper wake interval handling for periodic tasks
- Consistent thread naming and monitoring
- Integration with the project's threading infrastructure

## Extension Points

### Custom Result Backend

```cpp
class redis_result_backend : public result_backend_interface {
public:
    common::VoidResult store_result(
        const std::string& task_id,
        const value_container& result) override {
        // Store in Redis
    }

    common::Result<value_container> get_result(
        const std::string& task_id) override {
        // Fetch from Redis
    }

    // ... implement other methods
};
```

### Custom Task Handler

```cpp
class custom_handler : public task_handler_interface {
public:
    std::string name() const override { return "custom.task"; }

    common::Result<value_container> execute(
        const task& t,
        task_context& ctx) override {
        // Implementation
    }

    void on_retry(const task& t, size_t attempt) override {
        // Custom retry logic
    }
};
```

## Design Decisions

### 1. Message-Based Tasks

Tasks extend the `message` class to leverage existing messaging infrastructure for serialization and metadata handling.

### 2. Result Type Pattern

All operations return `Result<T>` or `VoidResult` for explicit error handling without exceptions.

### 3. Builder Pattern

The `task_builder` provides a fluent API for constructing tasks with complex configurations.

### 4. Facade Pattern

`task_system` acts as a facade, simplifying the interface while allowing access to individual components.

### 5. Strategy Pattern

`result_backend_interface` allows different storage strategies to be swapped without changing the core logic.

## Related Documentation

- [Quick Start Guide](QUICK_START.md)
- [API Reference](API_REFERENCE.md)
- [Patterns Guide](PATTERNS.md)
- [Configuration Guide](CONFIGURATION.md)
- [Troubleshooting](TROUBLESHOOTING.md)
