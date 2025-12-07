# Simple Worker Example

Demonstrates basic usage of the Task module.

## Features

- Creating a `task_system` with default configuration
- Registering handlers with lambda functions
- Submitting tasks and waiting for results
- Displaying statistics

## Usage

```bash
./bin/example_task_simple_worker
```

## Key Concepts

### Handler Registration

```cpp
system.register_handler("greet", [](const task& t, task_context& ctx) {
    auto name = t.payload().get_string("name").value_or("World");

    container_module::value_container result;
    result.set_value("greeting", "Hello, " + name + "!");
    return common::ok(result);
});
```

### Task Submission

```cpp
container_module::value_container payload;
payload.set_value("name", std::string("Task System"));

auto result = system.submit("greet", payload);
auto outcome = result.get(std::chrono::seconds(10));
```

## Expected Output

```
=== Simple Worker Example ===
Task system started with 2 workers

Submitting greeting task...
Result: Hello, Task System!

Submitting addition task (10 + 25)...
Result: 35

=== Statistics ===
Total tasks: 2
Succeeded: 2
Failed: 0

Shutting down...
Done!
```
