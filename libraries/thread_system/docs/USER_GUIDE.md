# Thread System User Guide

## Build

### Prerequisites
- CMake 3.16 or later
- C++20 capable compiler (GCC 9+, Clang 10+, MSVC 2019+)
- vcpkg package manager (automatically installed by dependency scripts)

### Quick Build
```bash
# Install dependencies via vcpkg
./dependency.sh       # Linux/macOS
# ./dependency.bat    # Windows

# Build the project
./build.sh           # Linux/macOS  
# ./build.bat        # Windows

# Or manual cmake build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Build Options
```bash
# Build as submodule (libraries only, no samples/tests)
cmake -S . -B build -DBUILD_THREADSYSTEM_AS_SUBMODULE=ON

# Enable documentation build
cmake -S . -B build -DBUILD_DOCUMENTATION=ON

# Debug build with sanitizers
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DENABLE_UBSAN=ON
```

For detailed platform-specific build instructions, see [Platform Build Guide](./PLATFORM_BUILD_GUIDE.md).

## Modules

- core: `thread_base`, `job`, `job_queue`, `sync` (including sync_primitives, cancellation_token, error_handling)
- implementations: `thread_pool`, `typed_thread_pool`, adaptive queues
- interfaces: logging, monitoring, executor/scheduler, service registry
- utilities: formatting, string conversion, span

## Quick Start: Thread Pool

```cpp
using namespace thread_pool_module;
using thread_module::result_void;

auto pool = std::make_shared<thread_pool>("pool");
std::vector<std::unique_ptr<thread_worker>> workers;
workers.emplace_back(std::make_unique<thread_worker>(/*use_time_tag=*/false));
pool->enqueue_batch(std::move(workers));
pool->start();

pool->execute(std::make_unique<thread_module::callback_job>([]() -> result_void {
  // do work
  return result_void();
}));

pool->shutdown(); // or pool->stop(false)
```

## Quick Start: Typed Thread Pool (priority/type routing)

```cpp
using namespace typed_thread_pool_module;

// See samples/typed_thread_pool_sample for a complete example
// Define jobs by type (e.g., job_types::RealTime / Batch / Background)
// and submit to the typed pool to route by type/priority.
```

## Adaptive Job Queue (automatic strategy)

The adaptive queue automatically switches between mutex-based and lock-free 
strategies internally based on contention and latency patterns.

```cpp
thread_module::adaptive_job_queue q{
  thread_module::adaptive_job_queue::queue_strategy::ADAPTIVE
};

q.enqueue(std::make_unique<thread_module::callback_job>([](){ return thread_module::result_void(); }));
auto job = q.dequeue();
```

## Dependency Injection and Context

Use `service_container` to register optional services (logger, monitoring), then
`thread_context` retrieves them automatically for pools/workers.

```cpp
// Register services
thread_module::service_container::global()
  .register_singleton<thread_module::logger_interface>(my_logger);
thread_module::service_container::global()
  .register_singleton<monitoring_interface::monitoring_interface>(my_monitoring);

// Context-aware pool/worker will log and report metrics when services exist
thread_pool_module::thread_worker w{true, thread_module::thread_context{}};
```

## Monitoring and Metrics

- `monitoring_interface` defines `system_metrics`, `thread_pool_metrics` (with
  `pool_name`/`pool_instance_id`), `worker_metrics`, and snapshot APIs.
- `thread_pool` reports pool metrics (workers, idle count, queue size) via context.
- Use `null_monitoring` when monitoring is not configured.

## Error Handling and Cancellation

- Return `result_void` / `result<T>` from operations, with `error_code` on failure.
- Use `cancellation_token` for cooperative cancellation (linkable tokens, callbacks).

## Documentation

If Doxygen is installed:

```bash
cmake --build build --target docs
# Open thread_system/documents/html/index.html
```

See also: `docs/API_REFERENCE.md` for complete API documentation including interface details.

## Samples

### Core Threading
- **minimal_thread_pool**: Minimal thread pool usage
- **thread_pool_sample**: Complete pool lifecycle management
- **typed_thread_pool_sample**: Type-based priority routing
- **typed_thread_pool_sample_2**: Advanced typed pool usage

### Queue Systems
- **adaptive_queue_sample**: Adaptive vs lock-free vs mutex comparison
- **mpmc_queue_sample**: Multi-producer multi-consumer queue usage
- **hierarchical_queue_sample**: Priority-based job queuing
- **typed_job_queue_sample**: Type-specific job queue operations

### Memory Management
- **hazard_pointer_sample**: Safe memory reclamation for lock-free data structures
- **node_pool_sample**: Memory pool operations
- **memory_pooled_jobs_sample**: Job-specific memory pooling

### Integration & Services
- **composition_example**: Dependency injection with interfaces
- **integration_example**: External logger/monitoring integration
- **service_registry_sample**: Service container and registry usage
- **multi_process_monitoring_integration**: Process-aware monitoring

### Advanced Features
- **builder_sample**: Thread pool builder pattern
- **combined_optimizations_sample**: Multiple performance optimizations
- **data_oriented_job_sample**: Data-oriented job processing
- **crash_protection**: Error handling and recovery mechanisms

### External Dependencies (Optional)
- **logger_sample**: High-performance logging (requires separate logger_system)
- **metrics_sample**: Real-time metrics collection (requires separate monitoring_system)
