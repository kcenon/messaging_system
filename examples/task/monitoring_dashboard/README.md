# Monitoring Dashboard Example

Demonstrates console-based monitoring visualization.

## Features

- Using `task_monitor` for system status
- Displaying queue and worker statistics
- Real-time dashboard updates
- Tracking task events and failures

## Usage

```bash
./bin/example_task_monitoring_dashboard
```

## Key Concepts

### Accessing Monitor

```cpp
task_system_config config;
config.enable_monitoring = true;
task_system system(config);

// Access monitor
auto monitor = system.monitor();
auto events = monitor->recent_events(10);
```

### Worker Statistics

```cpp
auto stats = system.get_statistics();
std::cout << "Total tasks: " << stats.total_tasks << std::endl;
std::cout << "Succeeded: " << stats.succeeded_tasks << std::endl;
std::cout << "Failed: " << stats.failed_tasks << std::endl;
```

### Queue Status

```cpp
std::cout << "Pending: " << system.pending_count() << std::endl;
std::cout << "Active workers: " << system.active_workers() << std::endl;
```

## Dashboard Display

The example shows a live-updating dashboard:

```
╔══════════════════════════════════════════════════════════╗
║           Task System Monitoring Dashboard               ║
║                   12:00:00                               ║
╚══════════════════════════════════════════════════════════╝

┌─ Worker Pool Status ──────────────────────────────────────┐
│ Total Workers:      4                                     │
│ Active Workers:     2                                     │
│ Idle Workers:       2                                     │
└───────────────────────────────────────────────────────────┘

┌─ Queue Status ────────────────────────────────────────────┐
│ Pending Tasks:      5                                     │
└───────────────────────────────────────────────────────────┘

┌─ Task Statistics ─────────────────────────────────────────┐
│ Total Submitted:       50                                 │
│ Succeeded:             45                                 │
│ Failed:                 5                                 │
│ Retried:                3                                 │
│ Success Rate:        90.0%                                │
└───────────────────────────────────────────────────────────┘

Press Ctrl+C to exit
```

## Use Cases

- Development and debugging
- System health monitoring
- Performance analysis
- Capacity planning
