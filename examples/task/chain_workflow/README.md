# Chain Workflow Example

Demonstrates sequential task execution (ETL pipeline pattern).

## Features

- Creating a chain of tasks that execute sequentially
- Passing results from one task to the next
- ETL (Extract-Transform-Load) pipeline implementation

## Usage

```bash
./bin/example_task_chain_workflow
```

## Key Concepts

### Chain Pattern

Tasks execute sequentially, with each task receiving the previous task's result:

```
Extract --> Transform --> Load
   │            │           │
   └─ result ──>└─ result ─>└─ final result
```

### Creating a Chain

```cpp
std::vector<task> chain_tasks;
chain_tasks.push_back(extract_task);
chain_tasks.push_back(transform_task);
chain_tasks.push_back(load_task);

auto result = system.client().chain(std::move(chain_tasks));
```

### Result Passing

Each task in the chain receives the previous task's result as its payload:

```cpp
system.register_handler("transform", [](const task& t, task_context& ctx) {
    // Access result from previous task
    auto record_count = t.payload().get_value<int>("record_count").value_or(0);
    // ... transform data ...
    return ok(result);
});
```

## Use Cases

- ETL data pipelines
- Multi-step processing workflows
- Sequential dependency execution

## Expected Output

```
=== Chain Workflow Example ===
Demonstrating ETL (Extract-Transform-Load) pipeline

Task system started

Building ETL chain: Extract -> Transform -> Load

Executing chain...
Waiting for chain to complete...

=== Chain Completed Successfully ===
Records loaded: 10
Destination: data_warehouse
Transformation: normalized

=== Statistics ===
Total tasks: 3
Succeeded: 3
Failed: 0
```
