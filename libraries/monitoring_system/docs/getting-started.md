# Getting Started with Monitoring System

This guide will help you get started with the Monitoring System quickly.

## Table of Contents
- [Requirements](#requirements)
- [Installation](#installation)
- [Basic Usage](#basic-usage)
- [Understanding Metrics](#understanding-metrics)
- [Configuration](#configuration)
- [Integration with Thread System](#integration-with-thread-system)

## Requirements

- C++20 compatible compiler:
  - GCC 10+ 
  - Clang 12+
  - MSVC 2019+ (Visual Studio 2019 version 16.11+)
- CMake 3.16 or higher
- Operating System:
  - Linux (Ubuntu 20.04+, CentOS 8+, etc.)
  - macOS 10.15+
  - Windows 10+

## Installation

### Using CMake FetchContent (Recommended)

Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    MonitoringSystem
    GIT_REPOSITORY https://github.com/kcenon/monitoring_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(MonitoringSystem)

# Link to your target
target_link_libraries(your_target PRIVATE MonitoringSystem::monitoring)
```

### Building from Source

```bash
# Clone the repository
git clone https://github.com/kcenon/monitoring_system.git
cd monitoring_system

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Install (optional)
sudo cmake --install .
```

### Using as Installed Package

If you've installed the monitoring system:

```cmake
find_package(MonitoringSystem REQUIRED)
target_link_libraries(your_target PRIVATE MonitoringSystem::monitoring)
```

## Basic Usage

### Simple Monitoring

```cpp
#include <monitoring_system/monitoring.h>

int main() {
    // Create monitoring instance
    auto monitor = std::make_shared<monitoring_module::monitoring>();
    
    // Start monitoring
    monitor->start();
    
    // Update system metrics
    monitoring_interface::system_metrics sys_metrics;
    sys_metrics.cpu_usage_percent = 45;
    sys_metrics.memory_usage_bytes = 1024 * 1024 * 512; // 512MB
    sys_metrics.active_threads = 8;
    monitor->update_system_metrics(sys_metrics);
    
    // Get current snapshot
    auto snapshot = monitor->get_current_snapshot();
    std::cout << "CPU Usage: " << snapshot.system.cpu_usage_percent << "%" << std::endl;
    
    // Stop monitoring
    monitor->stop();
    
    return 0;
}
```

### Continuous Monitoring

```cpp
#include <monitoring_system/monitoring.h>
#include <thread>
#include <chrono>

void monitor_application() {
    // Create monitor with 1-second collection interval
    auto monitor = std::make_shared<monitoring_module::monitoring>(1000, 1000);
    monitor->start();
    
    // Simulate application activity
    for (int i = 0; i < 60; ++i) {
        // Update metrics
        monitoring_interface::thread_pool_metrics pool_metrics;
        pool_metrics.jobs_completed = i * 10;
        pool_metrics.jobs_pending = rand() % 50;
        pool_metrics.worker_threads = 4;
        pool_metrics.idle_threads = rand() % 4;
        monitor->update_thread_pool_metrics(pool_metrics);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Get historical data
    auto history = monitor->get_recent_snapshots(10);
    std::cout << "Collected " << history.size() << " snapshots" << std::endl;
    
    monitor->stop();
}
```

## Understanding Metrics

### System Metrics

Track overall system performance:

```cpp
struct system_metrics {
    std::uint64_t cpu_usage_percent;      // CPU usage (0-100)
    std::uint64_t memory_usage_bytes;     // Memory in bytes
    std::uint64_t active_threads;         // Number of threads
    std::uint64_t total_allocations;      // Memory allocations
    std::chrono::steady_clock::time_point timestamp;
};
```

### Thread Pool Metrics

Monitor thread pool performance:

```cpp
struct thread_pool_metrics {
    std::uint64_t jobs_completed;         // Total completed jobs
    std::uint64_t jobs_pending;           // Current queue size
    std::uint64_t total_execution_time_ns;// Total processing time
    std::uint64_t average_latency_ns;     // Average job latency
    std::uint64_t worker_threads;         // Total workers
    std::uint64_t idle_threads;           // Idle workers
    std::chrono::steady_clock::time_point timestamp;
};
```

### Worker Metrics

Track individual worker performance:

```cpp
struct worker_metrics {
    std::uint64_t jobs_processed;         // Jobs by this worker
    std::uint64_t total_processing_time_ns;// Time spent working
    std::uint64_t idle_time_ns;           // Time spent idle
    std::uint64_t context_switches;       // Thread context switches
    std::chrono::steady_clock::time_point timestamp;
};

// Update worker metrics
monitor->update_worker_metrics(worker_id, metrics);
```

## Configuration

### Construction Options

```cpp
// Default: 1000 snapshots, 1-second interval
auto monitor1 = std::make_shared<monitoring_module::monitoring>();

// Custom: 100 snapshots, 100ms interval
auto monitor2 = std::make_shared<monitoring_module::monitoring>(100, 100);

// Large history: 10000 snapshots, 500ms interval
auto monitor3 = std::make_shared<monitoring_module::monitoring>(10000, 500);
```

### Runtime Configuration

```cpp
// Change collection interval
monitor->set_collection_interval(2000); // 2 seconds

// Force immediate collection
monitor->collect_now();

// Clear history
monitor->clear_history();

// Check monitoring statistics
auto stats = monitor->get_stats();
std::cout << "Total collections: " << stats.total_collections << std::endl;
std::cout << "Dropped snapshots: " << stats.dropped_snapshots << std::endl;
```

### Custom Collectors

```cpp
// Create custom system metrics collector
class cpu_collector : public monitoring_module::metrics_collector {
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        // Linux example
        std::ifstream stat("/proc/stat");
        std::string cpu;
        long user, nice, system, idle;
        stat >> cpu >> user >> nice >> system >> idle;
        
        long total = user + nice + system + idle;
        long active = user + nice + system;
        
        snapshot.system.cpu_usage_percent = (active * 100) / total;
    }
    
    std::string name() const override {
        return "CPU Collector";
    }
};

// Add custom collector
monitor->add_collector(std::make_unique<cpu_collector>());
```

## Integration with Thread System

### Using Service Container

```cpp
#include <monitoring_system/monitoring.h>
#include <thread_system/interfaces/service_container.h>
#include <thread_system/interfaces/thread_context.h>
#include <thread_system/thread_pool/core/thread_pool.h>

int main() {
    // Create and configure monitoring
    auto monitor = std::make_shared<monitoring_module::monitoring>();
    monitor->start();
    
    // Register in service container
    thread_module::service_container::global()
        .register_singleton<monitoring_interface::monitoring_interface>(monitor);
    
    // Create thread pool - it will report metrics automatically
    thread_module::thread_context context;
    auto pool = std::make_shared<thread_pool_module::thread_pool>("MyPool", context);
    
    // The pool will now update monitoring metrics
    pool->start();
    
    // Submit work
    for (int i = 0; i < 100; ++i) {
        pool->enqueue(std::make_unique<some_job>());
    }
    
    // Monitor the pool
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    auto snapshot = monitor->get_current_snapshot();
    std::cout << "Jobs completed: " << snapshot.thread_pool.jobs_completed << std::endl;
    
    // Clean up
    pool->stop();
    monitor->stop();
    
    return 0;
}
```

### Direct Integration

```cpp
// Create monitoring
auto monitor = std::make_shared<monitoring_module::monitoring>();
monitor->start();

// Use directly as monitoring_interface
std::shared_ptr<monitoring_interface::monitoring_interface> monitoring_iface = monitor;

// Pass to components that need monitoring
auto context = thread_module::thread_context(nullptr, monitoring_iface);
```

### Monitoring Multiple Components

```cpp
auto monitor = std::make_shared<monitoring_module::monitoring>();
monitor->start();

// Monitor multiple thread pools
auto pool1 = create_thread_pool("Pool1", monitor);
auto pool2 = create_thread_pool("Pool2", monitor);

// Aggregate metrics
std::thread monitor_thread([monitor]() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        auto snapshot = monitor->get_current_snapshot();
        
        // Log aggregated metrics
        std::cout << "Total jobs: " << snapshot.thread_pool.jobs_completed
                  << ", Pending: " << snapshot.thread_pool.jobs_pending
                  << ", Workers: " << snapshot.thread_pool.worker_threads
                  << std::endl;
    }
});
```

## Next Steps

- Read the [Architecture Overview](architecture.md) to understand the system design
- Check the [API Reference](api-reference.md) for detailed documentation
- Learn about [Custom Collectors](custom-collectors.md) for specialized metrics
- See [Performance Guide](performance.md) for overhead analysis
- Explore [Real-world Examples](examples.md) for production scenarios