# Logger System API Reference

## Table of Contents
- [Core Classes](#core-classes)
- [Enumerations](#enumerations)
- [Logger Class](#logger-class)
- [Writer Classes](#writer-classes)
- [Log Collector](#log-collector)
- [Macros and Helpers](#macros-and-helpers)

## Core Classes

### logger_module::logger

The main logger class that implements `thread_module::logger_interface`.

```cpp
namespace logger_module {
    class logger : public thread_module::logger_interface;
}
```

## Enumerations

### thread_module::log_level

Log severity levels (defined in thread_system interfaces).

```cpp
enum class log_level {
    trace,    // Most detailed information
    debug,    // Debug information
    info,     // Informational messages
    warning,  // Warning messages
    error,    // Error messages
    critical  // Critical errors
};
```

## Logger Class

### Constructor

```cpp
explicit logger(bool async = true, std::size_t buffer_size = 8192);
```

Creates a new logger instance.

**Parameters:**
- `async` - Enable asynchronous logging (default: true)
- `buffer_size` - Size of the log buffer for async mode (default: 8192)

**Example:**
```cpp
// Asynchronous logger with default buffer
auto async_logger = std::make_shared<logger_module::logger>();

// Synchronous logger
auto sync_logger = std::make_shared<logger_module::logger>(false);

// Async logger with larger buffer
auto large_buffer_logger = std::make_shared<logger_module::logger>(true, 16384);
```

### Logging Methods

#### log (simple)

```cpp
void log(thread_module::log_level level, const std::string& message) override;
```

Logs a message at the specified level.

**Parameters:**
- `level` - Log severity level
- `message` - Message to log

**Example:**
```cpp
logger->log(thread_module::log_level::info, "Application started");
logger->log(thread_module::log_level::error, "Connection failed");
```

#### log (with source location)

```cpp
void log(thread_module::log_level level, const std::string& message,
         const std::string& file, int line, const std::string& function) override;
```

Logs a message with source location information.

**Parameters:**
- `level` - Log severity level
- `message` - Message to log
- `file` - Source file name
- `line` - Line number in source file
- `function` - Function name

**Example:**
```cpp
logger->log(thread_module::log_level::debug, "Debug point reached",
            __FILE__, __LINE__, __func__);
```

### Configuration Methods

#### set_min_level

```cpp
void set_min_level(thread_module::log_level level);
```

Sets the minimum log level. Messages below this level will be ignored.

**Parameters:**
- `level` - Minimum level to log

**Example:**
```cpp
// Only log warnings and above
logger->set_min_level(thread_module::log_level::warning);
```

#### get_min_level

```cpp
thread_module::log_level get_min_level() const;
```

Gets the current minimum log level.

**Returns:** Current minimum log level

#### is_enabled

```cpp
bool is_enabled(thread_module::log_level level) const override;
```

Checks if a log level is enabled.

**Parameters:**
- `level` - Level to check

**Returns:** true if the level will be logged

### Writer Management

#### add_writer

```cpp
void add_writer(std::unique_ptr<base_writer> writer);
```

Adds a writer to output logs.

**Parameters:**
- `writer` - Unique pointer to the writer

**Example:**
```cpp
logger->add_writer(std::make_unique<console_writer>());
```

#### clear_writers

```cpp
void clear_writers();
```

Removes all writers from the logger.

### Lifecycle Methods

#### start

```cpp
void start();
```

Starts the logger (required for async mode). Safe to call multiple times.

**Example:**
```cpp
auto logger = std::make_shared<logger_module::logger>();
logger->start(); // Start background thread
```

#### stop

```cpp
void stop();
```

Stops the logger and flushes all pending messages.

#### is_running

```cpp
bool is_running() const;
```

Checks if the logger is running.

**Returns:** true if the logger is active

#### flush

```cpp
void flush() override;
```

Flushes all pending log messages to writers.

## Writer Classes

### base_writer

Abstract base class for all log writers.

```cpp
class base_writer {
public:
    virtual ~base_writer() = default;
    
    virtual void write(thread_module::log_level level,
                      const std::string& message,
                      const std::string& file,
                      int line,
                      const std::string& function,
                      const std::chrono::system_clock::time_point& timestamp) = 0;
    
    virtual void flush() = 0;
    
    virtual void set_use_color(bool use_color);
    bool use_color() const;
    
protected:
    std::string format_log_entry(...);
    std::string level_to_string(thread_module::log_level level) const;
    std::string level_to_color(thread_module::log_level level) const;
};
```

### console_writer

Writes log messages to console (stdout/stderr).

```cpp
class console_writer : public base_writer {
public:
    explicit console_writer(bool use_stderr = false, 
                           bool auto_detect_color = true);
};
```

**Constructor Parameters:**
- `use_stderr` - If true, all output goes to stderr (default: false)
- `auto_detect_color` - Auto-detect terminal color support (default: true)

**Behavior:**
- Error and critical levels always go to stderr
- Other levels go to stdout (unless use_stderr is true)
- Automatic color detection for supported terminals
- Thread-safe console output

**Example:**
```cpp
// Standard console output with colors
auto console = std::make_unique<console_writer>();

// All output to stderr, no colors
auto stderr_writer = std::make_unique<console_writer>(true, false);
```

## Log Collector

Internal class for asynchronous log collection (not directly exposed).

### Features:
- Background thread for log processing
- Configurable buffer size
- Batch processing for efficiency
- Graceful overflow handling

## Macros and Helpers

While the library doesn't provide macros by default, you can define your own:

```cpp
// Convenience macros (define in your project)
#define LOG_TRACE(logger, msg) \
    (logger)->log(thread_module::log_level::trace, (msg), __FILE__, __LINE__, __func__)

#define LOG_DEBUG(logger, msg) \
    (logger)->log(thread_module::log_level::debug, (msg), __FILE__, __LINE__, __func__)

#define LOG_INFO(logger, msg) \
    (logger)->log(thread_module::log_level::info, (msg), __FILE__, __LINE__, __func__)

#define LOG_WARNING(logger, msg) \
    (logger)->log(thread_module::log_level::warning, (msg), __FILE__, __LINE__, __func__)

#define LOG_ERROR(logger, msg) \
    (logger)->log(thread_module::log_level::error, (msg), __FILE__, __LINE__, __func__)

#define LOG_CRITICAL(logger, msg) \
    (logger)->log(thread_module::log_level::critical, (msg), __FILE__, __LINE__, __func__)
```

## Thread Safety

All public methods of the logger class are thread-safe:
- Multiple threads can log simultaneously
- Writers are protected by mutexes
- Configuration changes are atomic

## Example Usage

### Complete Example

```cpp
#include <logger_system/logger.h>
#include <logger_system/writers/console_writer.h>
#include <thread>
#include <vector>

int main() {
    // Create async logger
    auto logger = std::make_shared<logger_module::logger>(true, 16384);
    
    // Add console writer
    logger->add_writer(std::make_unique<logger_module::console_writer>());
    
    // Configure and start
    logger->set_min_level(thread_module::log_level::debug);
    logger->start();
    
    // Log from multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([logger, i]() {
            for (int j = 0; j < 100; ++j) {
                logger->log(thread_module::log_level::info,
                           "Thread " + std::to_string(i) + 
                           " iteration " + std::to_string(j));
            }
        });
    }
    
    // Wait for threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Stop and flush
    logger->stop();
    
    return 0;
}
```