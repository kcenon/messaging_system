# Logger System Architecture

## Overview

The Logger System is designed as a high-performance, modular logging framework that implements the `thread_module::logger_interface` from the Thread System. It provides both synchronous and asynchronous logging capabilities with comprehensive error handling through the result pattern.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      Client Application                      │
├─────────────────────────────────────────────────────────────┤
│                    logger_interface (API)                    │
├─────────────────────────────────────────────────────────────┤
│                         logger                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    Sync Mode                         │    │
│  │  ┌─────────────┐  ┌─────────────┐  ┌────────────┐  │    │
│  │  │   Format    │→ │   Write     │→ │   Flush    │  │    │
│  │  └─────────────┘  └─────────────┘  └────────────┘  │    │
│  └─────────────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                   Async Mode                         │    │
│  │  ┌────────────┐  ┌──────────────┐  ┌────────────┐  │    │
│  │  │   Queue    │→ │ log_collector │→ │   Batch    │  │    │
│  │  └────────────┘  └──────────────┘  └────────────┘  │    │
│  └─────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│            Configuration & Builder Pattern Layer            │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐  │
│  │ logger_builder  │  │ logger_config   │  │ Templates   │  │  
│  │                 │  │                 │  │             │  │
│  │ • Fluent API    │  │ • Validation    │  │ • Production│  │
│  │ • Result Types  │  │ • Defaults      │  │ • Debug     │  │
│  │ • Error Checks  │  │ • Strategies    │  │ • High Perf │  │
│  └─────────────────┘  └─────────────────┘  └─────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                Interface Segregation Layer                   │
│  ┌────────────────┐ ┌─────────────────┐ ┌─────────────────┐ │
│  │ log_writer_    │ │ log_filter_     │ │ log_formatter_ │ │
│  │ interface      │ │ interface       │ │ interface      │ │
│  └────────────────┘ └─────────────────┘ └─────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                         Writers                              │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────┐     │
│  │ConsoleWriter │  │ FileWriter   │  │ CustomWriter  │     │
│  └──────────────┘  └──────────────┘  └───────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. Logger Interface Implementation

The system implements `thread_module::logger_interface` with enhanced error handling:

```cpp
class logger : public thread_module::logger_interface {
    // Result pattern for comprehensive error handling
    result_void log(log_level level, const std::string& message) override;
    result_void log(log_level level, const std::string& message,
                    const std::string& file, int line, 
                    const std::string& function) override;
    
    bool is_enabled(log_level level) const override;
    result_void flush() override;
};
```

### 2. Configuration Management

#### Logger Configuration with Validation

```cpp
struct logger_config {
    // Core settings
    bool async = true;
    std::size_t buffer_size = 8192;
    thread_module::log_level min_level = thread_module::log_level::info;
    
    // Performance tuning
    std::size_t batch_size = 100;
    std::chrono::milliseconds flush_interval{1000};
    overflow_policy queue_overflow_policy = overflow_policy::drop_newest;
    
    // Feature flags
    bool enable_metrics = false;
    bool enable_crash_handler = false;
    bool enable_structured_logging = false;
    
    // Comprehensive validation
    result_void validate() const;
};
```

#### Configuration Templates

Predefined configurations for common scenarios:

- **Production**: Optimized for production environments with async logging
- **Debug**: Immediate synchronous output for development  
- **High Performance**: Maximum throughput configuration
- **Low Latency**: Minimal latency for real-time systems

### 3. Builder Pattern with Validation

```cpp
class logger_builder {
    // Fluent interface with validation at each step
    logger_builder& use_template(const std::string& template_name);
    logger_builder& with_async(bool async = true);
    logger_builder& with_buffer_size(std::size_t size);
    logger_builder& add_writer(const std::string& name, 
                              std::unique_ptr<log_writer_interface> writer);
    
    // Validation before building
    result_void validate() const;
    result<std::unique_ptr<logger>> build();
};
```

### 4. Interface Segregation

Clean separation of concerns through dedicated interfaces:

#### Writer Interface
```cpp
class log_writer_interface {
public:
    virtual result_void write(const log_entry& entry) = 0;
    virtual result_void flush() = 0;
    virtual bool is_healthy() const { return true; }
};
```

#### Filter Interface
```cpp
class log_filter_interface {
public:
    virtual bool should_log(const log_entry& entry) const = 0;
};
```

#### Formatter Interface  
```cpp
class log_formatter_interface {
public:
    virtual std::string format(const log_entry& entry) const = 0;
};
```

### 5. Log Entry Structure

Unified data structure for all logging operations:

```cpp
struct log_entry {
    thread_module::log_level level;
    std::string message;
    std::string file;
    int line;
    std::string function;
    std::chrono::system_clock::time_point timestamp;
    std::thread::id thread_id;
    std::unordered_map<std::string, std::string> context;  // For structured logging
};
```

## Advanced Features

### 1. Asynchronous Pipeline

The async mode uses a sophisticated pipeline:

- **Producer Side**: Non-blocking enqueue with overflow policies
- **Consumer Side**: Background thread with batch processing
- **Queue Management**: Currently mutex-backed (lock-free planned)
- **Overflow Handling**: Configurable policies (drop oldest/newest, block, expand)

### 2. Error Handling with Result Pattern

Comprehensive error handling throughout the system:

```cpp
enum class logger_error_code {
    success = 0,
    // Writer errors (1000-1099)
    writer_not_found = 1000,
    writer_initialization_failed = 1001,
    writer_not_healthy = 1003,
    
    // File errors (1100-1199)  
    file_open_failed = 1100,
    file_write_failed = 1101,
    
    // Queue errors (1300-1399)
    queue_full = 1301,
    queue_stopped = 1302,
    
    // Configuration errors (1400-1499)
    invalid_configuration = 1400
};
```

### 3. Performance Monitoring

Built-in metrics collection:

```cpp
class logger_metrics {
public:
    uint64_t get_messages_per_second() const;
    uint64_t get_avg_enqueue_time_ns() const;
    double get_queue_utilization_percent() const;
    uint64_t get_dropped_messages() const;
};
```

### 4. Configuration Strategies

Flexible configuration management:

- **Template Strategy**: Predefined configurations
- **Environment Strategy**: Environment variable-based configuration  
- **Performance Strategy**: Auto-tuning based on load
- **Composite Strategy**: Combination of multiple strategies

## Threading Model

### Synchronous Mode
- **Execution**: Direct write to all registered writers
- **Blocking**: Waits for I/O completion
- **Use Cases**: Low-frequency logging, critical errors, simple applications

### Asynchronous Mode  
- **Execution**: Non-blocking enqueue, background processing
- **Throughput**: High-volume logging capability
- **Use Cases**: High-frequency logging, performance-critical applications

### Thread Safety Guarantees
- All public methods are thread-safe
- Writers are called sequentially (no concurrent writes to same writer)
- Internal state protected by appropriate synchronization
- Lock-free queue planned for better scalability

## Memory Management

### Buffer Management
- Configurable buffer sizes with validation
- Efficient memory usage with move semantics
- Pre-allocated buffers in async mode
- Smart overflow handling

### Object Lifetime
- RAII principles throughout
- Shared ownership via smart pointers where appropriate
- Clear ownership semantics for writers and filters

## Performance Characteristics

### Benchmarks

| Mode | Operation | Latency | Throughput | Memory |
|------|-----------|---------|------------|---------|
| Sync | Direct write | ~100μs | I/O limited | Minimal |
| Async | Enqueue | ~50ns | >1M msg/sec | Buffer size |

### Optimization Strategies

1. **String Operations**: Minimized allocations, move semantics
2. **Batch Processing**: Efficient I/O operations
3. **Lock Contention**: Minimized through careful design
4. **Cache Performance**: Data structure layout optimization

## Integration Patterns

### Service Container Integration
```cpp
// Register as singleton
service_container::global()
    .register_singleton<logger_interface>(
        logger_builder()
            .use_template("production")
            .build()
            .value()
    );
```

### Direct Integration
```cpp  
// Inject into components
class MyComponent {
    MyComponent(std::shared_ptr<logger_interface> logger) 
        : logger_(logger) {}
private:
    std::shared_ptr<logger_interface> logger_;
};
```

## Extension Points

### Custom Writers
```cpp
class custom_writer : public log_writer_interface {
public:
    result_void write(const log_entry& entry) override {
        // Custom implementation (database, network, etc.)
        return result_void{};
    }
};
```

### Custom Filters
```cpp
class severity_filter : public log_filter_interface {
public:
    bool should_log(const log_entry& entry) const override {
        return entry.level >= min_level_;
    }
};
```

### Custom Formatters
```cpp
class json_formatter : public log_formatter_interface {  
public:
    std::string format(const log_entry& entry) const override {
        // JSON formatting logic
        return json_string;
    }
};
```

## Future Enhancements

### Performance Improvements
1. **Lock-free Queue**: True lock-free MPMC queue implementation
2. **SIMD Optimizations**: Vectorized string operations
3. **Memory Pool**: Custom allocators for frequent allocations

### Feature Additions  
1. **Structured Logging**: Native JSON/binary format support
2. **Log Aggregation**: Built-in sampling and aggregation
3. **Network Writers**: Remote logging with compression
4. **Database Integration**: Direct database logging support

### Platform Extensions
1. **Windows Support**: Full Windows compatibility for network features
2. **Mobile Platforms**: iOS/Android optimizations
3. **Embedded Systems**: Resource-constrained environment support

## Best Practices

### For Library Users
- Use async mode for production systems
- Configure appropriate buffer sizes based on load
- Implement custom writers for specialized needs
- Monitor metrics for performance tuning
- Use result pattern for error handling

### For Contributors  
- Maintain thread safety guarantees
- Follow RAII principles consistently
- Use move semantics for efficiency
- Document performance implications of changes
- Write comprehensive tests for new features

## Platform Notes

- **Linux/macOS**: Full support for all features
- **Windows**: Core features supported, network components require WinSock initialization
- **Cross-platform**: CMake build system with feature detection
- **Dependencies**: Minimal external dependencies, optional integrations

This architecture provides a solid foundation for high-performance logging while maintaining flexibility, extensibility, and ease of use.