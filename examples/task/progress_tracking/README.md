# Progress Tracking Example

Demonstrates real-time progress updates for long-running tasks.

## Features

- Updating task progress with `update_progress()`
- Polling for progress from the client side
- Displaying progress bars in the console
- Checkpoint save/restore for resumable tasks

## Usage

```bash
./bin/example_task_progress_tracking
```

## Key Concepts

### Updating Progress

From within a handler:

```cpp
system.register_handler("long.task", [](const task& t, task_context& ctx) {
    for (int step = 0; step <= 100; ++step) {
        double progress = step / 100.0;
        ctx.update_progress(progress, "Step " + std::to_string(step));
        // Do work...
    }
    return ok(result);
});
```

### Reading Progress

From the client side:

```cpp
auto result = system.submit("long.task", payload);

while (result.status() != task_state::succeeded) {
    double progress = result.progress();
    std::string message = result.progress_message();
    display_progress_bar(progress, message);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### Checkpoints

Save and restore progress for resumable tasks:

```cpp
// Save checkpoint
container_module::value_container state;
state.set_value("step", current_step);
ctx.save_checkpoint(state);

// Load checkpoint (on retry)
auto checkpoint = ctx.load_checkpoint();
int start_step = checkpoint.get_value<int>("step").value_or(0);
```

## Expected Output

```
=== Progress Tracking Example ===
Task system started

=== Processing File ===
Submitted task: abc123...
Tracking progress:

[========================================>] 100.0% - Processing step 15/15

File processed successfully!
  Steps completed: 15

=== Multi-Phase Task ===
[========================================>] 100.0% - All phases complete

Multi-phase task completed!
  Phases completed: 5
```
