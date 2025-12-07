# Task Module Quick Start Guide

Get up and running with the Task module in 5 minutes.

## Prerequisites

- C++17 or later
- CMake 3.16+
- A C++ compiler (GCC 8+, Clang 10+, or MSVC 2019+)

## Installation

The Task module is included in the messaging_system library. Build with CMake:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Basic Usage

### 1. Include Headers

```cpp
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
```

### 2. Create and Configure the Task System

```cpp
#include <kcenon/messaging/task/task_system.h>

int main() {
    using namespace kcenon::messaging::task;

    // Configure the system
    task_system_config config;
    config.worker.concurrency = 4;  // Use 4 worker threads
    config.worker.queues = {"default", "high-priority"};

    // Create the task system
    task_system system(config);

    // ... register handlers and start
}
```

### 3. Register a Task Handler

```cpp
// Lambda-based handler
system.register_handler("greet", [](const task& t, task_context& ctx) {
    auto name = t.payload().get_string("name").value_or("World");

    container_module::value_container result;
    result.add("message", "Hello, " + name + "!");

    return common::ok(result);
});
```

### 4. Start the System

```cpp
auto start_result = system.start();
if (!start_result) {
    std::cerr << "Failed to start: " << start_result.error().message << "\n";
    return 1;
}
```

### 5. Submit a Task

```cpp
// Create payload
container_module::value_container payload;
payload.add("name", "Alice");

// Submit task and get async result handle
auto result = system.submit("greet", payload);

// Wait for result with timeout
auto outcome = result.get(std::chrono::seconds(10));
if (outcome.is_ok()) {
    std::cout << outcome.value().get_string("message").value() << "\n";
    // Output: Hello, Alice!
} else {
    std::cerr << "Task failed: " << outcome.error().message << "\n";
}
```

### 6. Stop the System

```cpp
system.stop();
```

## Complete Example

```cpp
#include <kcenon/messaging/task/task_system.h>
#include <iostream>

int main() {
    using namespace kcenon::messaging::task;

    // 1. Configure
    task_system_config config;
    config.worker.concurrency = 2;

    // 2. Create system
    task_system system(config);

    // 3. Register handler
    system.register_handler("add", [](const task& t, task_context& ctx) {
        auto a = t.payload().get_int("a").value_or(0);
        auto b = t.payload().get_int("b").value_or(0);

        ctx.update_progress(0.5, "Calculating...");

        container_module::value_container result;
        result.add("sum", a + b);

        ctx.update_progress(1.0, "Done");
        return common::ok(result);
    });

    // 4. Start
    system.start();

    // 5. Submit task
    container_module::value_container payload;
    payload.add("a", 10);
    payload.add("b", 20);

    auto result = system.submit("add", payload);

    // 6. Get result
    auto outcome = result.get(std::chrono::seconds(5));
    if (outcome.is_ok()) {
        std::cout << "Sum: " << outcome.value().get_int("sum").value() << "\n";
        // Output: Sum: 30
    }

    // 7. Stop
    system.stop();

    return 0;
}
```

## Using the Task Builder

For more complex task configurations, use the task builder:

```cpp
auto task = task_builder("email.send")
    .payload(email_payload)
    .priority(message_priority::high)
    .timeout(std::chrono::minutes(2))
    .retries(5)
    .queue("email-queue")
    .tag("notifications")
    .build();

if (task) {
    auto result = system.submit(task.value());
}
```

## Scheduling Tasks

### Periodic Execution

```cpp
// Execute every 5 minutes
system.schedule_periodic(
    "cleanup-job",
    task_builder("cleanup.temp").build().value(),
    std::chrono::minutes(5)
);
```

### Cron-based Execution

```cpp
// Execute daily at 3 AM
system.schedule_cron(
    "daily-report",
    task_builder("report.generate").build().value(),
    "0 3 * * *"
);
```

## Monitoring Progress

```cpp
auto result = system.submit("long-running-task", payload);

// Poll for progress
while (!result.is_ready()) {
    std::cout << "Progress: " << (result.progress() * 100) << "% - "
              << result.progress_message() << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

## Error Handling

```cpp
auto result = system.submit("risky-task", payload);
auto outcome = result.get(std::chrono::seconds(30));

if (outcome.is_ok()) {
    // Handle success
    auto value = outcome.value();
} else {
    // Handle failure
    std::cerr << "Error: " << outcome.error().message << "\n";

    // Get detailed error info from result
    if (result.is_failed()) {
        std::cerr << "Traceback: " << result.error_traceback() << "\n";
    }
}
```

## Next Steps

- [Architecture Guide](ARCHITECTURE.md) - Understand the system design
- [API Reference](API_REFERENCE.md) - Complete API documentation
- [Patterns Guide](PATTERNS.md) - Learn workflow patterns (chain, chord)
- [Configuration Guide](CONFIGURATION.md) - Configure for your environment
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues and solutions
