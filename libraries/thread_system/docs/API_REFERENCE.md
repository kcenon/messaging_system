# Thread System API Reference

Complete API documentation for the Thread System framework.

## Table of Contents

1. [Overview](#overview)
2. [Core Module](#core-module-thread_module)
   - [thread_base Class](#thread_base-class)
   - [job Class](#job-class)
   - [job_queue Class](#job_queue-class)
   - [result Template](#resultt-template)
3. [Synchronization Module](#synchronization-module)
   - [sync_primitives](#sync_primitives)
   - [cancellation_token](#cancellation_token)
4. [Dependency Injection](#dependency-injection)
   - [service_container](#service_container)
   - [service_registry](#service_registry)
5. [Thread Pool Module](#thread-pool-module-thread_pool_module)
   - [thread_pool Class](#thread_pool-class)
   - [thread_worker Class](#thread_worker-class)
6. [Typed Thread Pool Module](#typed-thread-pool-module-typed_thread_pool_module)
   - [typed_thread_pool_t Template](#typed_thread_pool_t-template)
   - [typed_thread_worker_t Template](#typed_thread_worker_t-template)
   - [job_types Enumeration](#job_types-enumeration)
7. [Interfaces](#interfaces)
   - [Overview](#interfaces-overview)
   - [Executor Interface](#executor-interface)
   - [Scheduler Interface](#scheduler-interface)
   - [Logging Interface](#logging-interface-and-registry)
   - [Monitoring Interface](#monitoring-interface)
   - [Monitorable Interface](#monitorable-interface)
   - [Thread Context and Service Container](#thread-context-and-service-container)
   - [Error Handling](#error-handling)
   - [Crash Handling](#crash-handling)
   - [Typed Thread Pool Interfaces](#typed-thread-pool-interfaces)
8. [External Modules](#external-modules)
   - [Logger System](#logger-system)
   - [Monitoring System](#monitoring-system)
9. [Utilities Module](#utilities-module-utility_module)
   - [formatter_macros](#formatter-macros)
   - [convert_string](#convert_string)
10. [Quick Reference](#quick-reference)

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
- **Enhanced Synchronization**: Comprehensive synchronization wrappers with timeout support
- **Cancellation Support**: Cooperative cancellation with linked token hierarchies
- **Dependency Injection**: Prefer `service_container` (thread-safe DI). `service_registry` is a lightweight header-only alternative.
- **Error Handling**: Modern result<T> pattern for robust error management
- **Interface-Based Design**: Clean separation of concerns through well-defined interfaces

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

## Synchronization Module

### sync_primitives

Comprehensive synchronization wrapper utilities providing enhanced functionality over standard library primitives.

```cpp
#include "core/sync/include/sync_primitives.h"

// Scoped lock guard with timeout support
class scoped_lock_guard {
public:
    template<typename Mutex>
    explicit scoped_lock_guard(Mutex& mutex, 
                              std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    
    [[nodiscard]] bool owns_lock() const noexcept;
    explicit operator bool() const noexcept;
};

// Enhanced condition variable wrapper
class condition_variable_wrapper {
public:
    template<typename Predicate>
    void wait(std::unique_lock<std::mutex>& lock, Predicate pred);
    
    template<typename Rep, typename Period, typename Predicate>
    bool wait_for(std::unique_lock<std::mutex>& lock,
                  const std::chrono::duration<Rep, Period>& rel_time,
                  Predicate pred);
    
    void notify_one() noexcept;
    void notify_all() noexcept;
};

// Atomic flag with wait/notify support
class atomic_flag_wrapper {
public:
    void test_and_set(std::memory_order order = std::memory_order_seq_cst) noexcept;
    void clear(std::memory_order order = std::memory_order_seq_cst) noexcept;
    void wait(bool old, std::memory_order order = std::memory_order_seq_cst) const noexcept;
    void notify_one() noexcept;
    void notify_all() noexcept;
};

// Shared mutex wrapper for reader-writer locks
class shared_mutex_wrapper {
public:
    void lock();
    bool try_lock();
    void unlock();
    void lock_shared();
    bool try_lock_shared();
    void unlock_shared();
};
```

### cancellation_token

Provides cooperative cancellation mechanism for long-running operations.

```cpp
#include "core/sync/include/cancellation_token.h"

class cancellation_token {
public:
    // Create a new cancellation token
    static cancellation_token create();
    
    // Create a linked token (cancelled when any parent is cancelled)
    static cancellation_token create_linked(
        std::initializer_list<cancellation_token> tokens);
    
    // Cancel the operation
    void cancel();
    
    // Check cancellation status
    [[nodiscard]] bool is_cancelled() const;
    
    // Throw if cancelled
    void throw_if_cancelled() const;
    
    // Register callback for cancellation
    void register_callback(std::function<void()> callback);
};

// Usage example
auto token = cancellation_token::create();
auto linked = cancellation_token::create_linked({token, other_token});

token.register_callback([]() {
    std::cout << "Operation cancelled" << std::endl;
});

// In worker thread
while (!token.is_cancelled()) {
    // Do work
    token.throw_if_cancelled(); // Throws if cancelled
}
```

## Dependency Injection

### service_container

Preferred DI for Thread System: thread-safe container with singleton/factory lifetimes.

```cpp
#include "interfaces/service_container.h"
#include "interfaces/thread_context.h"

// Register services globally
auto& container = thread_module::service_container::global();
container.register_singleton<thread_module::logger_interface>(my_logger);
container.register_singleton<monitoring_interface::monitoring_interface>(my_monitoring);

// Resolve via thread_context (pools/workers use this context)
thread_module::thread_context ctx; // resolves from global container
```

### service_registry

Lightweight header-only alternative for simple scenarios.

```cpp
#include "core/base/include/service_registry.h"

// Register a service instance
thread_module::service_registry::register_service<MyInterface>(std::make_shared<MyImpl>());

// Retrieve a service instance
auto svc = thread_module::service_registry::get_service<MyInterface>();
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

## Interfaces

This section provides a comprehensive overview of the public interfaces that decouple components in the Thread System and enable dependency injection.

### Interfaces Overview

The thread system uses interface-based design for clean separation of concerns and easy extensibility. These interfaces enable dependency injection and allow components to be easily replaced or extended.

### Executor Interface

Header: `interfaces/executor_interface.h`

Base interface for all executor implementations (thread pools).

```cpp
#include "interfaces/executor_interface.h"

class executor_interface {
public:
    virtual ~executor_interface() = default;
    
    // Execute a job
    virtual auto execute(std::unique_ptr<thread_module::job>&& work) -> thread_module::result_void = 0;
    
    // Execute multiple jobs
    virtual auto execute_batch(std::vector<std::unique_ptr<job>> jobs) -> result_void = 0;
    
    // Shutdown the executor
    virtual auto shutdown() -> thread_module::result_void = 0;
    
    // Check if executor is running
    [[nodiscard]] virtual auto is_running() const -> bool = 0;
};
```

Implemented by: `implementations/thread_pool/thread_pool`

Example:
```cpp
auto pool = std::make_shared<thread_pool_module::thread_pool>("pool");
pool->enqueue_batch({std::make_unique<thread_pool_module::thread_worker>(false)});
pool->start();
pool->execute(std::make_unique<thread_module::callback_job>([](){ return thread_module::result_void(); }));
pool->shutdown();
```

### Scheduler Interface

Header: `interfaces/scheduler_interface.h`

Interface for job scheduling strategies.

```cpp
#include "interfaces/scheduler_interface.h"

class scheduler_interface {
public:
    virtual ~scheduler_interface() = default;
    
    // Schedule a job
    virtual auto schedule(std::unique_ptr<thread_module::job>&& work) -> thread_module::result_void = 0;
    
    // Get next job to execute
    virtual auto get_next_job() -> thread_module::result<std::unique_ptr<thread_module::job>> = 0;
    
    // Check if there are pending jobs
    [[nodiscard]] virtual auto has_pending() const -> bool = 0;
};
```

Implemented by: `core/jobs/job_queue` and its derivatives (`lockfree_job_queue`, `adaptive_job_queue`).

### Logging Interface and Registry

Header: `interfaces/logger_interface.h`

- `logger_interface::log(level, message[, file, line, function])`
- `logger_registry::set_logger(std::shared_ptr<logger_interface>)`
- Convenience macros: `THREAD_LOG_INFO("message")`, etc.

### Monitoring Interface

Header: `interfaces/monitoring_interface.h`

Data structures:
- `system_metrics`, `thread_pool_metrics` (supports `pool_name`/`pool_instance_id`), `worker_metrics`
- `metrics_snapshot`

Key methods:
- `update_system_metrics(const system_metrics&)`
- `update_thread_pool_metrics(const thread_pool_metrics&)`
- `update_thread_pool_metrics(const std::string& pool_name, std::uint32_t pool_instance_id, const thread_pool_metrics&)`
- `update_worker_metrics(std::size_t worker_id, const worker_metrics&)`
- `get_current_snapshot()`, `get_recent_snapshots(size_t)`

Utility:
- `null_monitoring` — no-op implementation
- `scoped_timer(std::atomic<std::uint64_t>& target)` — RAII measurement helper

### Monitorable Interface

Header: `interfaces/monitorable_interface.h`

- `auto get_metrics() -> monitoring_interface::metrics_snapshot`
- `void reset_metrics()`

Use to expose component metrics uniformly.

### Thread Context and Service Container

Headers: `interfaces/thread_context.h`, `interfaces/service_container.h`

`thread_context` provides access to:
- `std::shared_ptr<logger_interface> logger()`
- `std::shared_ptr<monitoring_interface::monitoring_interface> monitoring()`
- Helper methods to log and update metrics safely when services are available

`service_container` is a thread-safe DI container:
- `register_singleton<Interface>(std::shared_ptr<Interface>)`
- `register_factory<Interface>(std::function<std::shared_ptr<Interface>()>, lifetime)`
- `resolve<Interface>() -> std::shared_ptr<Interface>`

Use `scoped_service<Interface>` to register within a scope.

### Error Handling

Header: `core/sync/include/error_handling.h`, `interfaces/error_handler.h`

- Strongly typed `error_code`, `error`, `result<T>`/`result_void`
- `error_handler` interface and `default_error_handler` implementation

### Crash Handling

Header: `interfaces/crash_handler.h`

- Global `crash_handler::instance()` with registration of crash callbacks and cleanup routines
- Configurable safety level, core dumps, stack trace, and log path
- RAII helper `scoped_crash_callback`

### Typed Thread Pool Interfaces

Headers under `implementations/typed_thread_pool/include`:
- `typed_job_interface`, `job_types`, `type_traits`
- Lock-free/adaptive typed queues and typed workers/pools

These enable per-type routing and priority scheduling for heterogeneous workloads.

## External Modules

The following modules have been moved to separate projects for better modularity and reusability:

### Logger System

**Repository**: https://github.com/kcenon/logger_system

The asynchronous logging functionality is now available as a separate project that implements the `logger_interface`. It provides:

- **High-performance asynchronous logging** with multiple output targets
- **Structured logging** with type-safe formatting
- **Log level filtering** with bitwise-enabled categories
- **File rotation** and compression support
- **Thread-safe operations** with minimal performance impact

### Monitoring System

**Repository**: https://github.com/kcenon/monitoring_system

The performance monitoring and metrics collection system is now a separate project that implements the `monitoring_interface`. It offers:

- **Real-time performance metrics** collection and analysis
- **System resource monitoring** (CPU, memory, thread usage)
- **Thread pool statistics** tracking and reporting
- **Historical data retention** with configurable duration
- **Extensible metrics framework** for custom measurements

### Integration with Thread System

These external modules integrate seamlessly through the interface pattern:

```cpp
// Example: Register external logger and create a pool
#include "interfaces/service_container.h"
#include "implementations/thread_pool/include/thread_pool.h"
#include <logger_system/logger.h>  // External project

// Register logger with the global container
thread_module::service_container::global()
  .register_singleton<thread_module::logger_interface>(
      std::make_shared<logger_module::logger>());

// Create a pool (thread_context resolves logger automatically)
auto pool = std::make_shared<thread_pool_module::thread_pool>("MyPool");
```

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
#include "implementations/thread_pool/include/thread_pool.h"
#include "core/jobs/include/callback_job.h"

// Create and configure pool
auto pool = std::make_shared<thread_pool>("MyPool");

// Add workers
for (int i = 0; i < 4; ++i) {
    pool->enqueue(std::make_unique<thread_worker>());
}

// Start pool
if (auto error = pool->start(); error.has_value()) {
    std::cerr << "Failed to start pool: " << *error << std::endl;
    return;
}

// Submit jobs with cancellation support
auto token = cancellation_token::create();
auto job = std::make_unique<callback_job>([]() -> result_void {
    // Do work
    return {};
});
job->set_cancellation_token(token);
pool->enqueue(std::move(job));

// Cancel if needed
token.cancel();

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

### Using Service Container

```cpp
#include "interfaces/service_container.h"
#include "implementations/thread_pool/include/thread_pool.h"

// Register thread pool as executor service
auto pool = std::make_shared<thread_pool_module::thread_pool>("AppPool");
thread_module::service_container::global()
  .register_singleton<thread_module::executor_interface>(pool);

// Optional: register external logger implementation
// thread_module::service_container::global()
//   .register_singleton<thread_module::logger_interface>(
//       std::make_shared<external_logger>());

// Use services later via thread_context or resolve<>()
```

### Advanced Synchronization Example

```cpp
#include "core/sync/include/sync_primitives.h"

// Using scoped lock with timeout
std::mutex resource_mutex;
{
    scoped_lock_guard lock(resource_mutex, std::chrono::milliseconds(100));
    if (lock.owns_lock()) {
        // Successfully acquired lock within timeout
        // Access protected resource
    } else {
        // Timeout occurred
        // Handle timeout scenario
    }
}

// Using enhanced condition variable
condition_variable_wrapper cv;
std::mutex cv_mutex;
bool ready = false;

// Producer thread
{
    std::unique_lock<std::mutex> lock(cv_mutex);
    ready = true;
    cv.notify_one();
}

// Consumer thread with timeout
{
    std::unique_lock<std::mutex> lock(cv_mutex);
    if (cv.wait_for(lock, std::chrono::seconds(1), []{ return ready; })) {
        // Condition met within timeout
    } else {
        // Timeout occurred
    }
}
```
