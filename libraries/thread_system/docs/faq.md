# Frequently Asked Questions (FAQ)

## General Questions

### Q: What is Thread System?
**A:** Thread System is a C++20 library that provides a comprehensive framework for writing concurrent applications. It includes components for thread management, thread pools, type-based scheduling, and concurrent logging.

### Q: What C++ standard does Thread System require?
**A:** Thread System is designed for C++20, but includes fallbacks for compilers with partial C++20 support. It can detect and adapt to the availability of features like `std::format`, `std::jthread`, and `std::chrono::current_zone`.

### Q: Does Thread System work on all major platforms?
**A:** Yes, Thread System works on Windows, macOS, and Linux. Platform-specific optimizations are included for each platform, and the build system is designed to work across environments.

### Q: Does Thread System require external dependencies?
**A:** Thread System depends on the {fmt} library if `std::format` is not available on your compiler. For unit tests, it uses GoogleTest. These dependencies are managed through vcpkg.

### Q: Is Thread System thread-safe?
**A:** Yes, all public interfaces in Thread System are designed to be thread-safe. Internal data structures use appropriate synchronization mechanisms to ensure thread safety.

## Thread Base Questions

### Q: What is the difference between `thread_base` and `std::thread`?
**A:** `thread_base` is a higher-level abstraction built on top of `std::thread` (or `std::jthread` when available). It provides:
- A structured lifecycle (start, do_work, stop)
- Customization points through virtual methods
- Interval-based wake-up capability
- Unified error handling
- Better manageability through state tracking

### Q: How do I create a custom worker thread?
**A:** Inherit from `thread_base` and override the virtual methods as needed. For detailed patterns and examples, see the [patterns documentation](PATTERNS.md#worker-thread-pattern).

### Q: How do I make a worker thread wake up periodically?
**A:** Use the `set_wake_interval` method. See the [patterns documentation](PATTERNS.md#wake-interval-optimization) for optimization guidelines.

### Q: How do I handle errors in worker thread methods?
**A:** Return the error using the `result_void` type. For comprehensive error handling patterns, see the [patterns documentation](PATTERNS.md#best-practices).

## Thread Pool Questions

### Q: How many worker threads should I create in my thread pool?
**A:** A good rule of thumb is to use the number of hardware threads available on the system. For detailed sizing guidelines for different workload types, see the [patterns documentation](PATTERNS.md#thread-pool-sizing-guidelines).

### Q: Can I reuse a thread pool for multiple tasks?
**A:** Yes, thread pools are designed to be reused. You can keep submitting jobs to the pool, and the worker threads will process them as they become available.

### Q: How do I wait for all jobs in a thread pool to complete?
**A:** Call the `stop()` method on the thread pool, which will wait for all worker threads to finish processing their current jobs before returning.

### Q: What happens if a job throws an exception?
**A:** Jobs should not throw exceptions. Instead, they should return errors using the `std::optional<std::string>` or `result_void` return value. If a job does throw an exception, it will be caught by the worker thread, converted to an error, and logged.

### Q: How do I process the results of jobs submitted to a thread pool?
**A:** There are several approaches:
1. Have the jobs update shared state (with proper synchronization)
2. Use a callback mechanism where jobs report their results to a result collector
3. Implement a future/promise pattern with a result queue

## Type Thread Pool Questions

### Q: What is the difference between `thread_pool` and `typed_thread_pool`?
**A:** `typed_thread_pool` extends `thread_pool` by adding support for job types. Jobs are processed according to their type, with higher-type jobs being processed before lower-type ones.

### Q: How many type levels should I use?
**A:** Most applications work well with 3-5 type levels. More than that can lead to complexity without adding significant benefit. The standard enum provides `High`, `Normal`, and `Low` as a good starting point.

### Q: Can I create custom type types?
**A:** Yes, `typed_thread_pool` is a template class that can work with any type that provides comparison operators. You can define a custom enum or class that represents your application's type system.

### Q: How do I assign workers to specific type levels?
**A:** When creating a worker, specify which type levels it should process. For complete type-based execution patterns, see the [patterns documentation](PATTERNS.md#type-based-job-execution-pattern).

### Q: What happens if there are no workers for a specific type level?
**A:** Jobs with that type level will remain in the queue until a worker that can process them becomes available, or until the thread pool is stopped.

## Logging Questions

### Q: Where is logging implemented now?
**A:** Thread System provides a logging interface (`logger_interface`) only. The concrete implementation lives in the separate project [logger_system](https://github.com/kcenon/logger_system). Integrate it by registering your logger implementation via `service_container` or `logger_registry`.

### Q: Is logging thread-safe?
**A:** Yes for the recommended `logger_system` implementation (thread-safe by design). If you provide your own logger, ensure it is thread-safe because logging may occur from multiple worker threads.

### Q: How do I configure where logs are written?
**A:** Configuration is provided by your logger implementation (e.g., `logger_system`). In Thread System you inject the logger, e.g.:
```cpp
// Register global logger
thread_module::service_container::global()
  .register_singleton<thread_module::logger_interface>(my_logger);

// or
thread_module::logger_registry::set_logger(my_logger);
```
See the logger's own documentation for sinks/targets and levels.

### Q: How do I customize log formatting?
**A:** Use the facilities of your logger implementation (format strings, structured logging). Thread System forwards messages through `logger_interface` unchanged.

### Q: Does logging affect application performance?
**A:** Use an asynchronous logger (like `logger_system`) and avoid heavy logging in tight loops. Choose appropriate log levels and consider sampling or aggregation for high-frequency events.

### Q: How do I handle log rotation?
**A:** Log rotation/retention is handled by the logger implementation. Refer to the `logger_system` documentation (or your chosen logger) for rotation policies and configuration.

## Performance Questions

### Q: How do I measure the performance of Thread System?
**A:** Use standard C++ profiling tools (perf, gprof, Visual Studio Profiler) to measure the performance of your application. Thread System itself doesn't include built-in profiling, but it's designed to work well with external profilers.

### Q: What is the overhead of using Thread System compared to raw threads?
**A:** Thread System adds a small amount of overhead (typically microseconds per operation) compared to raw thread manipulation, but the benefits in terms of code organization, error handling, and maintainability generally outweigh this cost for most applications.

### Q: How does Thread System handle thread contention?
**A:** Thread System uses standard C++ synchronization primitives (mutexes, condition variables) with careful design to minimize contention. The job queue implementation, in particular, is optimized to reduce lock contention during high-throughput scenarios.

### Q: Is there a limit to how many jobs I can submit to a thread pool?
**A:** The practical limit is determined by your system's memory. Jobs are stored in queues that grow dynamically as needed. However, for best performance, consider batching jobs and not queuing extremely large numbers of very small jobs.

## Troubleshooting

For comprehensive troubleshooting guidance, including solutions to common concurrency issues such as:
- Thread pool processing problems
- Deadlocks and race conditions
- Performance issues
- Debugging strategies

Please refer to the [patterns documentation](PATTERNS.md#troubleshooting-common-issues).

## Build and Integration

### Q: How do I include Thread System in my project?
**A:** You can include Thread System either as a standalone project or as a submodule. For CMake projects, use `add_subdirectory()` to include it, and set `BUILD_THREADSYSTEM_AS_SUBMODULE=ON`.

### Q: How do I build Thread System from source?
**A:** Use the provided build script:
```bash
./build.sh         # Basic build
./build.sh --clean # Clean build
./build.sh --docs  # Build with documentation
```

### Q: How do I run the Thread System unit tests?
**A:** After building with the default options, run the tests located in the bin directory:
```bash
./bin/thread_pool_test
./bin/typed_thread_pool_test
./bin/utilities_test
```

### Q: What version of CMake is required to build Thread System?
**A:** Thread System requires CMake 3.16 or higher.

### Q: Can I use Thread System in a non-CMake project?
**A:** Yes, but you'll need to configure your build system to include the appropriate source files and set the required compilation flags (C++20 support, thread library linking, etc.).
