# Chord Aggregation Example

Demonstrates parallel execution with result aggregation.

## Features

- Executing multiple tasks in parallel
- Aggregating results when all tasks complete
- Fan-out/Fan-in pattern for data collection

## Usage

```bash
./bin/example_task_chord_aggregation
```

## Key Concepts

### Chord Pattern

Tasks execute in parallel, then a callback aggregates all results:

```
    ┌─> Task A ─┐
    │           │
Input -> Task B ─┼─> Callback (aggregation)
    │           │
    └─> Task C ─┘
```

### Creating a Chord

```cpp
std::vector<task> parallel_tasks = {task_a, task_b, task_c};
auto callback_task = task_builder("aggregate").build().unwrap();

auto result = system.client().chord(
    std::move(parallel_tasks),
    std::move(callback_task)
);
```

### Aggregation Callback

The callback task receives all results and produces the final output:

```cpp
system.register_handler("aggregate", [](const task& t, task_context& ctx) {
    // Access results from all parallel tasks
    // Aggregate and return final result
    return ok(aggregated_result);
});
```

## Use Cases

- Collecting data from multiple sources
- Parallel processing with final merge
- Distributed map-reduce operations

## Expected Output

```
=== Chord Aggregation Example ===
Collecting data from multiple sources in parallel

Task system started with 4 workers

Setting up chord pattern:
  Parallel tasks: fetch from [database, cache, api, file]
  Callback: aggregate results

  Added fetch task for: database
  Added fetch task for: cache
  Added fetch task for: api
  Added fetch task for: file

Executing chord (parallel tasks + callback)...
Waiting for all parallel tasks and aggregation...

=== Chord Completed Successfully ===
Sources processed: 4
Aggregation type: sum_and_avg
Total value: 250
Average value: 62
```
