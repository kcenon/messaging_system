# Priority Tasks Example

Demonstrates priority-based task processing.

## Features

- Creating tasks with different priority levels
- Observing priority-based processing order
- Using `task_builder` to configure task priorities

## Usage

```bash
./bin/example_task_priority_tasks
```

## Key Concepts

### Priority Levels

Lower numbers indicate higher priority:
- Priority 1: High (processed first)
- Priority 5: Medium
- Priority 10: Low (processed last)

### Setting Priority

```cpp
auto task_result = task_builder("process")
    .priority(1)  // High priority
    .build();
```

## Expected Behavior

When multiple tasks are queued:
1. High priority tasks (priority 1) are processed first
2. Medium priority tasks (priority 5) are processed next
3. Low priority tasks (priority 10) are processed last

Note: The first task may start immediately before all tasks are queued, so the exact order may vary slightly.

## Expected Output

```
=== Priority Tasks Example ===
Task system started

Submitting tasks with different priorities...
(Lower number = higher priority)

  Submitted: Low-0 (priority 10)
  Submitted: Low-1 (priority 10)
  Submitted: Low-2 (priority 10)
  Submitted: Medium-0 (priority 5)
  Submitted: Medium-1 (priority 5)
  Submitted: High-0 (priority 1)

Processing order (observe priority handling):
  Completed: High-0 (priority 1)
  Completed: Medium-0 (priority 5)
  Completed: Medium-1 (priority 5)
  ...
```
