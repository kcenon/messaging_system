# Monitoring System API Reference

## Table of Contents
- [Core Classes](#core-classes)
- [Data Structures](#data-structures)
- [Monitoring Class](#monitoring-class)
- [Collector Classes](#collector-classes)
- [Ring Buffer](#ring-buffer)
- [Integration Types](#integration-types)

## Core Classes

### monitoring_module::monitoring

The main monitoring class that implements `monitoring_interface::monitoring_interface`.

```cpp
namespace monitoring_module {
    class monitoring : public monitoring_interface::monitoring_interface;
}
```

## Data Structures

### monitoring_interface::system_metrics

System-level performance metrics.

```cpp
struct system_metrics {
    std::uint64_t cpu_usage_percent{0};      // CPU usage (0-100)
    std::uint64_t memory_usage_bytes{0};     // Memory usage in bytes
    std::uint64_t active_threads{0};         // Number of active threads
    std::uint64_t total_allocations{0};      // Total memory allocations
    std::chrono::steady_clock::time_point timestamp;
};
```

### monitoring_interface::thread_pool_metrics

Thread pool performance metrics.

```cpp
struct thread_pool_metrics {
    std::uint64_t jobs_completed{0};         // Total completed jobs
    std::uint64_t jobs_pending{0};           // Jobs waiting in queue
    std::uint64_t total_execution_time_ns{0}; // Total execution time
    std::uint64_t average_latency_ns{0};     // Average job latency
    std::uint64_t worker_threads{0};         // Total worker threads
    std::uint64_t idle_threads{0};           // Idle worker threads
    std::chrono::steady_clock::time_point timestamp;
};
```

### monitoring_interface::worker_metrics

Individual worker thread metrics.

```cpp
struct worker_metrics {
    std::uint64_t jobs_processed{0};         // Jobs processed by worker
    std::uint64_t total_processing_time_ns{0}; // Time spent processing
    std::uint64_t idle_time_ns{0};           // Time spent idle
    std::uint64_t context_switches{0};       // Context switch count
    std::chrono::steady_clock::time_point timestamp;
};
```

### monitoring_interface::metrics_snapshot

Complete metrics snapshot at a point in time.

```cpp
struct metrics_snapshot {
    system_metrics system;
    thread_pool_metrics thread_pool;
    worker_metrics worker;
    std::chrono::steady_clock::time_point capture_time;
};
```

## Monitoring Class

### Constructor

```cpp
explicit monitoring(std::size_t history_size = 1000,
                   std::uint32_t collection_interval_ms = 1000);
```

Creates a new monitoring instance.

**Parameters:**
- `history_size` - Number of historical snapshots to keep (default: 1000)
- `collection_interval_ms` - Interval between automatic collections in milliseconds (default: 1000)

**Example:**
```cpp
// Default configuration
auto monitor1 = std::make_shared<monitoring_module::monitoring>();

// Custom configuration: 100 snapshots, 500ms interval
auto monitor2 = std::make_shared<monitoring_module::monitoring>(100, 500);
```

### Metric Update Methods

#### update_system_metrics

```cpp
void update_system_metrics(const system_metrics& metrics) override;
```

Updates system-level metrics.

**Parameters:**
- `metrics` - System metrics to record

**Example:**
```cpp
monitoring_interface::system_metrics sys;
sys.cpu_usage_percent = 45;
sys.memory_usage_bytes = 1024 * 1024 * 512; // 512MB
sys.active_threads = 8;
monitor->update_system_metrics(sys);
```

#### update_thread_pool_metrics

```cpp
void update_thread_pool_metrics(const thread_pool_metrics& metrics) override;
```

Updates thread pool metrics.

**Parameters:**
- `metrics` - Thread pool metrics to record

**Example:**
```cpp
monitoring_interface::thread_pool_metrics pool;
pool.jobs_completed = 1000;
pool.jobs_pending = 25;
pool.worker_threads = 4;
pool.idle_threads = 1;
monitor->update_thread_pool_metrics(pool);
```

#### update_worker_metrics

```cpp
void update_worker_metrics(std::size_t worker_id, 
                          const worker_metrics& metrics) override;
```

Updates metrics for a specific worker thread.

**Parameters:**
- `worker_id` - Unique identifier for the worker
- `metrics` - Worker metrics to record

**Example:**
```cpp
monitoring_interface::worker_metrics worker;
worker.jobs_processed = 250;
worker.total_processing_time_ns = 500000000; // 500ms
monitor->update_worker_metrics(0, worker);
```

### Data Retrieval Methods

#### get_current_snapshot

```cpp
metrics_snapshot get_current_snapshot() const override;
```

Retrieves the most recent metrics snapshot.

**Returns:** Current metrics snapshot

**Example:**
```cpp
auto snapshot = monitor->get_current_snapshot();
std::cout << "CPU: " << snapshot.system.cpu_usage_percent << "%" << std::endl;
```

#### get_recent_snapshots

```cpp
std::vector<metrics_snapshot> get_recent_snapshots(std::size_t count) const override;
```

Retrieves multiple recent snapshots.

**Parameters:**
- `count` - Number of snapshots to retrieve

**Returns:** Vector of metrics snapshots, newest first

**Example:**
```cpp
// Get last 10 snapshots
auto history = monitor->get_recent_snapshots(10);
for (const auto& snapshot : history) {
    std::cout << "Jobs: " << snapshot.thread_pool.jobs_completed << std::endl;
}
```

### Lifecycle Methods

#### start

```cpp
void start();
```

Starts the monitoring system and background collection thread.

**Example:**
```cpp
auto monitor = std::make_shared<monitoring_module::monitoring>();
monitor->start(); // Begin collecting metrics
```

#### stop

```cpp
void stop();
```

Stops the monitoring system and background thread.

#### is_active

```cpp
bool is_active() const override;
```

Checks if monitoring is currently active.

**Returns:** true if monitoring is active

### Configuration Methods

#### set_collection_interval

```cpp
void set_collection_interval(std::uint32_t interval_ms);
```

Changes the automatic collection interval.

**Parameters:**
- `interval_ms` - New interval in milliseconds

**Example:**
```cpp
// Change to 2-second interval
monitor->set_collection_interval(2000);
```

#### get_collection_interval

```cpp
std::uint32_t get_collection_interval() const;
```

Gets the current collection interval.

**Returns:** Current interval in milliseconds

### Collector Management

#### add_collector

```cpp
void add_collector(std::unique_ptr<metrics_collector> collector);
```

Adds a custom metrics collector.

**Parameters:**
- `collector` - Unique pointer to the collector

**Example:**
```cpp
class my_collector : public monitoring_module::metrics_collector {
    void collect(metrics_snapshot& snapshot) override {
        // Custom collection logic
    }
    std::string name() const override { return "MyCollector"; }
};

monitor->add_collector(std::make_unique<my_collector>());
```

#### clear_collectors

```cpp
void clear_collectors();
```

Removes all custom collectors.

### Utility Methods

#### collect_now

```cpp
void collect_now();
```

Forces an immediate metrics collection cycle.

#### clear_history

```cpp
void clear_history();
```

Clears all historical snapshot data.

#### get_stats

```cpp
struct monitoring_stats {
    std::uint64_t total_collections;
    std::uint64_t dropped_snapshots;
    std::uint64_t collector_errors;
    std::chrono::steady_clock::time_point start_time;
};

monitoring_stats get_stats() const;
```

Gets statistics about the monitoring system itself.

**Returns:** Monitoring system statistics

**Example:**
```cpp
auto stats = monitor->get_stats();
std::cout << "Total collections: " << stats.total_collections << std::endl;
std::cout << "Dropped snapshots: " << stats.dropped_snapshots << std::endl;
```

## Collector Classes

### metrics_collector

Base class for custom metrics collectors.

```cpp
class metrics_collector {
public:
    virtual ~metrics_collector() = default;
    
    virtual void collect(metrics_snapshot& snapshot) = 0;
    virtual std::string name() const = 0;
};
```

### Example Custom Collector

```cpp
class disk_io_collector : public monitoring_module::metrics_collector {
private:
    std::uint64_t last_read_bytes_ = 0;
    std::uint64_t last_write_bytes_ = 0;
    
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        // Read disk I/O statistics
        auto [read_bytes, write_bytes] = get_disk_io_stats();
        
        // Calculate rates
        auto read_rate = read_bytes - last_read_bytes_;
        auto write_rate = write_bytes - last_write_bytes_;
        
        // Store in custom field (would need extended metrics)
        // snapshot.system.disk_read_rate = read_rate;
        // snapshot.system.disk_write_rate = write_rate;
        
        last_read_bytes_ = read_bytes;
        last_write_bytes_ = write_bytes;
    }
    
    std::string name() const override {
        return "DiskIOCollector";
    }
};
```

## Ring Buffer

### Template Class

```cpp
template<typename T>
class ring_buffer {
public:
    explicit ring_buffer(std::size_t capacity);
    
    bool push(const T& value);
    std::vector<T> get_recent(std::size_t count) const;
    std::vector<T> get_all() const;
    void clear();
    
    std::size_t size() const;
    std::size_t capacity() const;
    bool empty() const;
    bool full() const;
};
```

### Usage Example

```cpp
// Create buffer for 1000 entries
ring_buffer<metrics_snapshot> buffer(1000);

// Add snapshots
metrics_snapshot snapshot;
buffer.push(snapshot);

// Get recent entries
auto recent = buffer.get_recent(10);
```

## Integration Types

### Service Container Integration

```cpp
// Register monitoring service
monitoring_module::service_container::global()
    .register_singleton<monitoring_interface::monitoring_interface>(monitor);

// Components will automatically use it
thread_context context; // Resolves monitoring from container
```

### Direct Usage

```cpp
// Create monitoring
auto monitor = std::make_shared<monitoring_module::monitoring>();

// Use as interface
std::shared_ptr<monitoring_interface::monitoring_interface> iface = monitor;

// Pass to components
auto pool = std::make_shared<thread_pool>("Pool", thread_context(nullptr, iface));
```

## Thread Safety

All public methods are thread-safe:
- Concurrent updates from multiple threads supported
- Reading while updating is safe
- Collectors run sequentially in background thread

## Complete Example

```cpp
#include <monitoring_system/monitoring.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // Create monitoring with custom settings
    auto monitor = std::make_shared<monitoring_module::monitoring>(
        500,    // Keep 500 snapshots
        250     // Collect every 250ms
    );
    
    // Add custom collector
    class load_collector : public monitoring_module::metrics_collector {
    public:
        void collect(monitoring_interface::metrics_snapshot& snapshot) override {
            // Simulate load calculation
            snapshot.system.cpu_usage_percent = rand() % 100;
        }
        std::string name() const override { return "LoadCollector"; }
    };
    
    monitor->add_collector(std::make_unique<load_collector>());
    
    // Start monitoring
    monitor->start();
    
    // Simulate application activity
    for (int i = 0; i < 100; ++i) {
        // Update metrics
        monitoring_interface::thread_pool_metrics pool;
        pool.jobs_completed = i * 10;
        pool.jobs_pending = rand() % 50;
        monitor->update_thread_pool_metrics(pool);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Get statistics
    auto stats = monitor->get_stats();
    std::cout << "Collections: " << stats.total_collections << std::endl;
    
    // Get history
    auto history = monitor->get_recent_snapshots(20);
    std::cout << "History size: " << history.size() << std::endl;
    
    // Stop monitoring
    monitor->stop();
    
    return 0;
}
```