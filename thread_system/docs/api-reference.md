# Thread System API Reference

Complete API documentation for the Thread System framework.

## Table of Contents

1. [Overview](#overview)
2. [Core Module](#core-module-thread_module)
   - [thread_base Class](#thread_base-class)
   - [job Class](#job-class)
   - [job_queue Class](#job_queue-class)
   - [result Template](#resultt-template)
3. [Thread Pool Module](#thread-pool-module-thread_pool_module)
   - [thread_pool Class](#thread_pool-class)
   - [thread_worker Class](#thread_worker-class)
4. [Typed Thread Pool Module](#typed-thread-pool-module-typed_thread_pool_module)
   - [typed_thread_pool_t Template](#typed_thread_pool_t-template)
   - [typed_thread_worker_t Template](#typed_thread_worker_t-template)
   - [job_types Enumeration](#job_types-enumeration)
5. [External Modules](#external-modules)
   - [Logger Module](#logger-module)
   - [Monitoring Module](#monitoring-module)
6. [Utilities Module](#utilities-module-utility_module)
   - [formatter_macros](#formatter-macros)
   - [convert_string](#convert_string)
7. [Quick Reference](#quick-reference)

## Overview

The Thread System framework provides a comprehensive set of classes for building multi-threaded applications with adaptive performance optimization.

### Core Components

- **Core Classes** (`thread_module` namespace)
  - `thread_base` - Abstract base class for all workers
  - `job` - Unit of work representation
  - `job_queue` - Thread-safe job queue
  - `result<T>` - Error handling type
  - `callback_job` - Job implementation with lambda support

### Thread Pool Components

- **Standard Thread Pool** (`thread_pool_module` namespace)
  - `thread_pool` - Thread pool with adaptive queue optimization
  - `thread_worker` - Worker thread implementation

### Typed Thread Pool Components

- **Type-based Priority Pool** (`typed_thread_pool_module` namespace)
  - `typed_thread_pool_t<T>` - Template-based priority pool
  - `typed_thread_worker_t<T>` - Priority-aware worker
  - `typed_job_t<T>` - Job with type information
  - `job_types` - Default priority enumeration (RealTime, Batch, Background)

### Advanced Features

- **Adaptive Queue System**: Automatic switching between mutex and lock-free strategies
- **Hazard Pointers**: Safe memory reclamation for lock-free structures
- **Node Pools**: Memory pools for reduced allocation overhead
- **Real-time Monitoring**: Performance metrics collection and analysis

## Core Module (thread_module)

### thread_base Class

Abstract base class for all worker threads in the system.

```cpp
class thread_base {
public:
    thread_base(const std::string& thread_title = "thread_base");
    virtual ~thread_base();
    
    // Thread control
    auto start() -> result_void;
    auto stop() -> result_void;
    
    // Configuration
    auto set_wake_interval(const std::optional<std::chrono::milliseconds>& interval) -> void;
    auto get_wake_interval() const -> std::optional<std::chrono::milliseconds>;
    
    // Status
    auto is_running() const -> bool;
    auto get_thread_title() const -> std::string;
    
protected:
    // Override in derived classes
    virtual auto before_start() -> result_void { return {}; }
    virtual auto do_work() -> result_void = 0;
    virtual auto after_stop() -> result_void { return {}; }
    virtual auto should_continue_work() const -> bool { return false; }
};
```

### job Class

Base class for all work units.

```cpp
class job {
public:
    job(const std::string& name = "job");
    job(const std::vector<uint8_t>& data, const std::string& name = "data_job");
    virtual ~job();
    
    // Core functionality
    virtual auto do_work() -> result_void;
    
    // Cancellation support
    virtual auto set_cancellation_token(const cancellation_token& token) -> void;
    virtual auto get_cancellation_token() const -> cancellation_token;
    
    // Queue association
    virtual auto set_job_queue(const std::shared_ptr<job_queue>& queue) -> void;
    virtual auto get_job_queue() const -> std::shared_ptr<job_queue>;
    
    // Metadata
    auto get_name() const -> std::string;
    virtual auto to_string() const -> std::string;
};
```

### job_queue Class

Thread-safe queue for job management.

```cpp
class job_queue : public std::enable_shared_from_this<job_queue> {
public:
    job_queue();
    virtual ~job_queue();
    
    // Queue operations
    [[nodiscard]] virtual auto enqueue(std::unique_ptr<job>&& value) -> result_void;
    [[nodiscard]] virtual auto enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs) -> result_void;
    [[nodiscard]] virtual auto dequeue() -> result<std::unique_ptr<job>>;
    [[nodiscard]] virtual auto dequeue_batch() -> std::deque<std::unique_ptr<job>>;
    
    // State management
    virtual auto clear() -> void;
    auto stop_waiting_dequeue() -> void;
    
    // Status
    [[nodiscard]] auto empty() const -> bool;
    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto is_stopped() const -> bool;
    
    // Configuration
    auto set_notify(bool notify) -> void;
};
```

### result<T> Template

Error handling type based on `std::expected` pattern.

```cpp
template<typename T>
using result = std::expected<T, error>;

using result_void = result<void>;

struct error {
    error_code code;
    std::string message;
};

enum class error_code {
    success = 0,
    invalid_argument,
    resource_allocation_failed,
    operation_canceled,
    thread_start_failure,
    job_execution_failed,
    queue_full,
    queue_empty
};
```

## Thread Pool Module (thread_pool_module)

### thread_pool Class

Main thread pool implementation with adaptive queue optimization.

```cpp
class thread_pool : public std::enable_shared_from_this<thread_pool> {
public:
    thread_pool(const std::string& thread_title = "thread_pool");
    virtual ~thread_pool();
    
    // Pool control
    auto start() -> std::optional<std::string>;
    auto stop(const bool& immediately_stop = false) -> void;
    
    // Job management
    auto enqueue(std::unique_ptr<job>&& job) -> std::optional<std::string>;
    auto enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs) -> std::optional<std::string>;
    
    // Worker management
    auto enqueue(std::unique_ptr<thread_worker>&& worker) -> std::optional<std::string>;
    auto enqueue_batch(std::vector<std::unique_ptr<thread_worker>>&& workers) -> std::optional<std::string>;
    
    // Access
    auto get_job_queue() -> std::shared_ptr<job_queue>;
    auto get_workers() const -> const std::vector<std::unique_ptr<thread_worker>>&;
};
```

### thread_worker Class

Worker thread implementation with adaptive capabilities.

```cpp
class thread_worker : public thread_base {
public:
    struct worker_statistics {
        uint64_t jobs_processed;
        uint64_t total_processing_time_ns;
        uint64_t batch_operations;
        uint64_t avg_processing_time_ns;
    };
    
    thread_worker(const std::string& name = "thread_worker");
    
    // Configuration
    auto set_batch_processing(bool enabled, size_t batch_size = 32) -> void;
    auto get_statistics() const -> worker_statistics;
    
    // Queue association
    auto set_job_queue(const std::shared_ptr<job_queue>& queue) -> void;
    
protected:
    auto do_work() -> result_void override;
};
```

## Typed Thread Pool Module (typed_thread_pool_module)

### typed_thread_pool_t Template

Priority-based thread pool with per-type queues.

```cpp
template<typename T>
class typed_thread_pool_t : public std::enable_shared_from_this<typed_thread_pool_t<T>> {
public:
    typed_thread_pool_t(const std::string& name = "typed_thread_pool");
    
    // Pool control
    auto start() -> result_void;
    auto stop(bool clear_queue = false) -> result_void;
    
    // Job management
    auto enqueue(std::unique_ptr<typed_job_t<T>>&& job) -> result_void;
    auto enqueue_batch(std::vector<std::unique_ptr<typed_job_t<T>>>&& jobs) -> result_void;
    
    // Worker management
    auto add_worker(std::unique_ptr<typed_thread_worker_t<T>>&& worker) -> result_void;
    auto add_workers(std::vector<std::unique_ptr<typed_thread_worker_t<T>>>&& workers) -> result_void;
    
    // Queue strategy (adaptive mode)
    auto set_queue_strategy(queue_strategy strategy) -> void;
};
```

### typed_thread_worker_t Template

Worker with type-based job handling.

```cpp
template<typename T>
class typed_thread_worker_t : public thread_base {
public:
    typed_thread_worker_t(const std::string& name = "typed_worker");
    
    // Type responsibilities
    auto set_responsibilities(const std::vector<T>& types) -> void;
    auto get_responsibilities() const -> std::vector<T>;
    
    // Queue association
    auto set_job_queue(const std::shared_ptr<typed_job_queue_t<T>>& queue) -> void;
};
```

### job_types Enumeration

Default priority levels for typed pools.

```cpp
enum class job_types : uint8_t {
    Background = 0,  // Lowest priority
    Batch = 1,       // Medium priority
    RealTime = 2     // Highest priority
};
```

## External Modules

The following modules have been moved to separate projects for better modularity and reusability:

### Logger Module

The asynchronous logging functionality is now available as a separate project. It provides:

- **High-performance asynchronous logging** with multiple output targets
- **Structured logging** with type-safe formatting
- **Log level filtering** with bitwise-enabled categories
- **File rotation** and compression support
- **Thread-safe operations** with minimal performance impact

**Repository**: Available as a standalone project for integration into various applications.

### Monitoring Module

The performance monitoring and metrics collection system is now a separate project. It offers:

- **Real-time performance metrics** collection and analysis
- **System resource monitoring** (CPU, memory, thread usage)
- **Thread pool statistics** tracking and reporting
- **Historical data retention** with configurable duration
- **Extensible metrics framework** for custom measurements

**Repository**: Available as a standalone project for comprehensive application monitoring.

### Integration with Thread System

While these modules are now separate projects, they were designed to work seamlessly with the Thread System:

- The logger can be used to track thread pool operations and job execution
- The monitoring module can collect detailed metrics from thread pools and workers
- Both modules maintain the same design principles and performance characteristics

For applications requiring logging or monitoring capabilities, include these modules as separate dependencies in your project.

## Utilities Module (utility_module)

### formatter_macros

Macros for reducing formatter code duplication.

```cpp
#include "utilities/core/formatter_macros.h"

// Generate formatter specializations for custom types
DECLARE_FORMATTER(my_namespace::my_class)
```

### convert_string

String conversion utilities.

```cpp
namespace convert_string {
    auto to_string(const std::wstring& str) -> std::string;
    auto to_wstring(const std::string& str) -> std::wstring;
    auto to_utf8(const std::wstring& str) -> std::string;
    auto from_utf8(const std::string& str) -> std::wstring;
}
```

## Quick Reference

### Creating a Basic Thread Pool

```cpp
// Create and configure pool
auto pool = std::make_shared<thread_pool>("MyPool");

// Add workers
for (int i = 0; i < 4; ++i) {
    pool->enqueue(std::make_unique<thread_worker>());
}

// Start pool
pool->start();

// Submit jobs
pool->enqueue(std::make_unique<callback_job>([]() -> result_void {
    // Do work
    return {};
}));

// Stop pool
pool->stop();
```

### Using Typed Thread Pool

```cpp
// Create typed pool with priorities
auto typed_pool = std::make_shared<typed_thread_pool_t<job_types>>();

// Add specialized workers
auto realtime_worker = std::make_unique<typed_thread_worker_t<job_types>>();
realtime_worker->set_responsibilities({job_types::RealTime});
typed_pool->add_worker(std::move(realtime_worker));

// Start pool
typed_pool->start();

// Submit priority job
auto priority_job = std::make_unique<callback_typed_job<job_types>>(
    job_types::RealTime,
    []() -> result_void {
        // High priority work
        return {};
    }
);
typed_pool->enqueue(std::move(priority_job));
```

### Error Handling Example

```cpp
// Using result type for error handling
auto process_with_result() -> result_void {
    auto pool = std::make_shared<thread_pool>();
    
    // Start pool with error checking
    if (auto error = pool->start(); error.has_value()) {
        return thread_module::error{
            thread_module::error_code::thread_start_failure,
            *error
        };
    }
    
    // Submit job with error handling
    auto job = std::make_unique<callback_job>([]() -> result_void {
        // Simulate potential failure
        if (some_condition_fails()) {
            return thread_module::error{
                thread_module::error_code::job_execution_failed,
                "Operation failed due to invalid state"
            };
        }
        return {}; // Success
    });
    
    if (auto error = pool->enqueue(std::move(job)); error.has_value()) {
        return thread_module::error{
            thread_module::error_code::queue_full,
            *error
        };
    }
    
    return {}; // Success
}
```

### Batch Processing Example

```cpp
// Configure worker for batch processing
auto worker = std::make_unique<thread_worker>("BatchWorker");
worker->set_batch_processing(true, 64); // Enable with batch size 64

// Submit multiple jobs for efficient batch processing
std::vector<std::unique_ptr<job>> batch;
for (int i = 0; i < 1000; ++i) {
    batch.push_back(std::make_unique<callback_job>([i]() -> result_void {
        // Process item i
        return {};
    }));
}

// Enqueue entire batch
pool->enqueue_batch(std::move(batch));
```