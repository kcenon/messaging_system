# Scheduled Tasks Example

Demonstrates delayed and periodic task execution.

## Features

- Scheduling tasks with countdown delays
- Creating periodic tasks that run at intervals
- Using cron expressions for scheduling

## Usage

```bash
./bin/example_task_scheduled_tasks
```

## Key Concepts

### Delayed Execution

```cpp
system.submit_later(task, std::chrono::seconds(5));
```

### Periodic Scheduling

```cpp
system.schedule_periodic(
    "schedule-name",
    task,
    std::chrono::seconds(2)
);
```

### Cron-based Scheduling

```cpp
system.schedule_cron(
    "schedule-name",
    task,
    "0 * * * * *"  // Every minute at second 0
);
```

## Cron Expression Format

```
* * * * * *
│ │ │ │ │ └── Day of week (0-6)
│ │ │ │ └──── Month (1-12)
│ │ │ └────── Day of month (1-31)
│ │ └──────── Hour (0-23)
│ └────────── Minute (0-59)
└──────────── Second (0-59)
```

## Expected Output

```
=== Scheduled Tasks Example ===
Start time: 12:00:00
Task system started

Scheduling delayed task (3 seconds)...
Setting up periodic heartbeat (every 2 seconds)...
Setting up cron task (for demonstration)...
Scheduling another delayed task (5 seconds)...

Waiting for scheduled tasks to execute...
(Running for 8 seconds)

[12:00:02] Heartbeat #1
[12:00:03] Delayed task executed: This was delayed by 3 seconds
[12:00:04] Heartbeat #2
[12:00:05] Delayed task executed: This was delayed by 5 seconds
[12:00:06] Heartbeat #3
...
```
