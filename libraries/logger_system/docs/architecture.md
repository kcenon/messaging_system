# Logger System Architecture

## Overview

The Logger System is designed as a high-performance, modular logging framework that implements the `thread_module::logger_interface` from the Thread System. It provides both synchronous and asynchronous logging capabilities with a lock-free implementation for minimal contention in multi-threaded environments.

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
│                         Writers                              │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────┐     │
│  │ConsoleWriter │  │ FileWriter   │  │ CustomWriter  │     │
│  └──────────────┘  └──────────────┘  └───────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. Logger Interface

The system implements `thread_module::logger_interface` providing:

```cpp
class logger_interface {
    virtual void log(log_level level, const std::string& message) = 0;
    virtual void log(log_level level, const std::string& message,
                     const std::string& file, int line, 
                     const std::string& function) = 0;
    virtual bool is_enabled(log_level level) const = 0;
    virtual void flush() = 0;
};
```

### 2. Logger Implementation

The main `logger` class provides:
- **Dual Mode Operation**: Synchronous and asynchronous logging
- **Level Filtering**: Configurable minimum log level
- **Multiple Writers**: Support for multiple output destinations
- **Thread Safety**: Safe for concurrent use

#### Key Features:
- **PIMPL Pattern**: Implementation details hidden for ABI stability
- **Smart Pointer Management**: RAII for resource management
- **Configurable Buffer Size**: For async mode optimization

### 3. Log Collector (Async Mode)

The `log_collector` manages asynchronous logging:
- **Lock-free Queue**: Minimal contention for log producers
- **Background Thread**: Dedicated thread for log processing
- **Batch Processing**: Efficient handling of multiple log entries
- **Overflow Handling**: Graceful handling when buffer is full

#### Design Decisions:
- Simple queue with mutex (can be upgraded to lock-free)
- Batch size of 100 entries for efficiency
- Condition variable for efficient waiting

### 4. Writers

Writers handle the actual output of log messages:

#### Base Writer
```cpp
class base_writer {
    virtual void write(log_level level, const std::string& message,
                      const std::string& file, int line,
                      const std::string& function,
                      const std::chrono::system_clock::time_point& timestamp) = 0;
    virtual void flush() = 0;
};
```

#### Console Writer
- **Color Support**: ANSI color codes for different log levels
- **Platform Detection**: Automatic color support detection
- **Stream Selection**: Error levels to stderr, others to stdout

#### Extensibility
- Easy to add new writers (file, network, database)
- Consistent interface for all writers
- Format customization support

## Threading Model

### Synchronous Mode
- Direct write to all registered writers
- Blocking operation - waits for I/O completion
- Lower latency for individual messages
- Suitable for:
  - Low-frequency logging
  - Critical error messages
  - Simple applications

### Asynchronous Mode
- Non-blocking enqueue operation
- Background thread processes log queue
- Higher throughput for bulk logging
- Suitable for:
  - High-frequency logging
  - Performance-critical paths
  - Multi-threaded applications

### Thread Safety Guarantees
- All public methods are thread-safe
- Writers are called sequentially (no concurrent writes)
- Internal state protected by mutexes
- Lock-free operations where possible

## Memory Management

### Buffer Management
- Configurable buffer size (default: 8192 entries)
- Circular buffer behavior on overflow
- Oldest entries dropped when full

### Object Lifetime
- Shared ownership of logger via shared_ptr
- Writers owned by logger
- Log entries copied/moved efficiently

### Performance Considerations
- String operations minimized
- Move semantics used where possible
- Pre-allocated buffers in async mode

## Integration Points

### Service Container Integration
```cpp
// Register logger
service_container::global()
    .register_singleton<logger_interface>(logger);

// Automatic resolution in thread_context
thread_context context; // Resolves logger from container
```

### Direct Integration
```cpp
// Use as logger_interface
std::shared_ptr<logger_interface> iface = logger;

// Pass to components
auto context = thread_context(iface);
```

## Performance Characteristics

### Synchronous Mode
- **Latency**: Direct I/O latency (typically < 100μs)
- **Throughput**: Limited by I/O speed
- **CPU Usage**: Minimal overhead
- **Memory**: Low memory footprint

### Asynchronous Mode
- **Latency**: Enqueue only (typically < 1μs)
- **Throughput**: > 1M logs/second possible
- **CPU Usage**: One background thread
- **Memory**: Buffer size dependent

## Future Enhancements

### Planned Features
1. **Lock-free Queue**: True lock-free implementation
2. **File Writer**: Rotating file output with compression
3. **Network Writer**: Remote logging capability
4. **Structured Logging**: JSON/Binary format support
5. **Log Aggregation**: Built-in metrics and sampling

### Extension Points
- Custom formatters for specialized output
- Filter chains for advanced filtering
- Async writer batching for better I/O
- Memory-mapped file support

## Design Patterns Used

1. **PIMPL (Pointer to Implementation)**
   - Hides implementation details
   - Provides ABI stability
   - Reduces compilation dependencies

2. **Strategy Pattern**
   - Writers as strategies for output
   - Runtime configuration of behavior

3. **Observer Pattern**
   - Multiple writers observing log events
   - Decoupled output handling

4. **Factory Pattern**
   - Potential for writer factories
   - Configuration-based instantiation

## Best Practices

### For Library Users
- Use async mode for production
- Set appropriate log levels
- Add custom writers for specialized needs
- Monitor buffer usage in high-load scenarios

### For Contributors
- Maintain thread safety invariants
- Follow RAII principles
- Use move semantics for efficiency
- Document performance implications