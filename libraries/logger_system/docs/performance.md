# Logger System Performance Guide

## Overview

The Logger System is designed for high-performance logging in multi-threaded applications. This guide covers performance characteristics, benchmarks, and optimization strategies.

## Performance Characteristics

### Synchronous Mode

| Operation | Typical Latency | Throughput |
|-----------|----------------|------------|
| Console Write | 50-200 μs | ~5K-20K logs/sec |
| File Write | 20-100 μs | ~10K-50K logs/sec |
| Memory Write | < 1 μs | > 1M logs/sec |

**Characteristics:**
- Direct I/O blocking
- Latency depends on output device
- No additional memory overhead
- Zero background CPU usage

### Asynchronous Mode

| Operation | Typical Latency | Throughput |
|-----------|----------------|------------|
| Log Enqueue | < 1 μs | > 1M logs/sec |
| Background Processing | N/A | Hardware limited |

**Characteristics:**
- Non-blocking enqueue
- Decoupled from I/O latency
- Configurable memory buffer
- One background thread

## Benchmarks

### Test Environment
- CPU: Intel Core i7-9700K @ 3.6GHz
- RAM: 32GB DDR4
- OS: Ubuntu 20.04 LTS
- Compiler: GCC 10.3 with -O3

### Single Thread Performance

```cpp
// Benchmark: 1 million log messages
auto logger = std::make_shared<logger_module::logger>(true, 16384);
logger->add_writer(std::make_unique<null_writer>()); // No I/O

auto start = std::chrono::high_resolution_clock::now();
for (int i = 0; i < 1'000'000; ++i) {
    logger->log(log_level::info, "Test message " + std::to_string(i));
}
auto end = std::chrono::high_resolution_clock::now();
```

**Results:**
- Async mode: ~1.2M logs/second
- Sync mode: ~2.5M logs/second (null writer)
- Sync mode: ~15K logs/second (console writer)

### Multi-threaded Performance

```cpp
// Benchmark: 4 threads, 250K messages each
std::vector<std::thread> threads;
for (int t = 0; t < 4; ++t) {
    threads.emplace_back([&logger, t]() {
        for (int i = 0; i < 250'000; ++i) {
            logger->log(log_level::info, 
                       "Thread " + std::to_string(t) + " msg " + std::to_string(i));
        }
    });
}
```

**Results:**
- Async mode: ~950K logs/second (total)
- Sync mode: ~12K logs/second (console, contention)

### Memory Usage

| Configuration | Memory Usage |
|--------------|--------------|
| Base logger | ~1 KB |
| Async buffer (8K entries) | ~256 KB |
| Async buffer (64K entries) | ~2 MB |
| Per log entry | ~32-128 bytes |

## Optimization Strategies

### 1. Choose the Right Mode

**Use Synchronous Mode When:**
- Logging frequency is low (< 100 logs/sec)
- Latency is not critical
- Memory is extremely constrained
- You need guaranteed immediate output

**Use Asynchronous Mode When:**
- High-frequency logging (> 1000 logs/sec)
- Low latency is critical
- Multiple threads are logging
- I/O performance varies

### 2. Buffer Size Tuning

```cpp
// Calculate optimal buffer size
size_t logs_per_second = 10000;
size_t buffer_time_seconds = 1;
size_t safety_factor = 2;
size_t buffer_size = logs_per_second * buffer_time_seconds * safety_factor;

auto logger = std::make_shared<logger_module::logger>(true, buffer_size);
```

**Guidelines:**
- Start with default (8192)
- Monitor dropped messages
- Increase for burst handling
- Consider memory constraints

### 3. Level Filtering

```cpp
// Filter early to avoid processing
logger->set_min_level(log_level::info);

// Check before expensive operations
if (logger->is_enabled(log_level::debug)) {
    std::string expensive_msg = build_debug_string();
    logger->log(log_level::debug, expensive_msg);
}
```

### 4. Message Construction

```cpp
// Avoid string concatenation in hot paths
// Bad:
logger->log(log_level::info, "Value: " + std::to_string(value) + 
                            " Status: " + status);

// Good:
thread_local char buffer[256];
snprintf(buffer, sizeof(buffer), "Value: %d Status: %s", value, status);
logger->log(log_level::info, buffer);

// Better (when available):
logger->log(log_level::info, std::format("Value: {} Status: {}", value, status));
```

### 5. Writer Optimization

#### Console Writer
```cpp
// Disable colors for better performance
auto console = std::make_unique<console_writer>();
console->set_use_color(false);

// Use stderr for all output (single stream)
auto console = std::make_unique<console_writer>(true);
```

#### Custom High-Performance Writer
```cpp
class memory_writer : public base_writer {
    std::vector<std::string> logs_;
    mutable std::mutex mutex_;
    
public:
    void write(log_level level, const std::string& message,
               const std::string& file, int line,
               const std::string& function,
               const std::chrono::system_clock::time_point& timestamp) override {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.reserve(logs_.size() + 1);
        logs_.emplace_back(format_log_entry(level, message, file, line, 
                                           function, timestamp));
    }
    
    void flush() override {
        // No-op for memory writer
    }
};
```

### 6. Batch Processing

For custom writers, implement batching:

```cpp
class batched_file_writer : public base_writer {
    std::vector<std::string> batch_;
    static constexpr size_t BATCH_SIZE = 100;
    
    void write(...) override {
        batch_.push_back(format_log_entry(...));
        if (batch_.size() >= BATCH_SIZE) {
            flush();
        }
    }
    
    void flush() override {
        // Write entire batch at once
        for (const auto& entry : batch_) {
            file_.write(entry.data(), entry.size());
        }
        file_.flush();
        batch_.clear();
    }
};
```

## Performance Anti-patterns

### 1. Synchronous I/O in Hot Paths

```cpp
// Bad: Blocks on every call
for (auto& item : large_collection) {
    sync_logger->log(log_level::debug, "Processing: " + item.name());
    process(item);
}

// Good: Use async or batch
auto async_logger = std::make_shared<logger>(true);
```

### 2. Excessive String Formatting

```cpp
// Bad: Creates many temporaries
logger->log(level, "A: " + a + " B: " + b + " C: " + c);

// Good: Single allocation
logger->log(level, std::format("A: {} B: {} C: {}", a, b, c));
```

### 3. Logging in Tight Loops

```cpp
// Bad: Logs every iteration
for (int i = 0; i < 1000000; ++i) {
    logger->log(log_level::trace, "Iteration " + std::to_string(i));
    // work
}

// Good: Sample or summarize
for (int i = 0; i < 1000000; ++i) {
    if (i % 10000 == 0) {
        logger->log(log_level::debug, "Progress: " + std::to_string(i));
    }
    // work
}
```

## Profiling and Monitoring

### Built-in Metrics

Monitor logger performance:

```cpp
// Check if buffers are overflowing
class monitoring_logger : public logger {
    std::atomic<size_t> dropped_messages_{0};
    
    void log(...) override {
        if (!try_enqueue(...)) {
            dropped_messages_.fetch_add(1);
        }
    }
    
    size_t get_dropped_count() const {
        return dropped_messages_.load();
    }
};
```

### External Profiling

Use system tools:
- `perf` for CPU profiling
- `valgrind` for memory analysis
- `strace` for system call analysis

```bash
# Profile CPU usage
perf record -g ./your_app
perf report

# Check system calls
strace -c ./your_app

# Memory profiling
valgrind --tool=massif ./your_app
```

## Best Practices Summary

1. **Default to Async Mode** - Better performance for most use cases
2. **Set Appropriate Levels** - Filter early to reduce overhead
3. **Size Buffers Correctly** - Based on expected load
4. **Minimize Allocations** - Reuse buffers, avoid string concatenation
5. **Batch I/O Operations** - Reduce system call overhead
6. **Profile Real Workloads** - Measure actual performance
7. **Monitor Health** - Track dropped messages and latency

## Platform-Specific Optimizations

### Linux
- Use `io_uring` for file writes (future)
- Consider `mmap` for large log files
- Tune kernel buffers for network logging

### Windows
- Use overlapped I/O for async file writes
- Consider ETW for system integration
- Optimize console API usage

### macOS
- Use Grand Central Dispatch for async operations
- Consider os_log for system integration
- Optimize for unified buffer cache