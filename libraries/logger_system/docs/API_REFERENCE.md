# API Reference

## Table of Contents
- [Core Classes](#core-classes)
- [Configuration](#configuration)
- [Builder Pattern](#builder-pattern)
- [Interfaces](#interfaces)
- [Error Handling](#error-handling)
- [Writers](#writers)
- [Filters](#filters)
- [Formatters](#formatters)

## Core Classes

### `logger_module::logger`

The main logger class that handles all logging operations.

#### Constructor
```cpp
logger(size_t buffer_size = 8192);
```

#### Methods

##### `log`
```cpp
result_void log(thread_module::log_level level, 
                const std::string& message,
                const std::string& file = "",
                int line = 0,
                const std::string& function = "");
```
Logs a message with specified level and optional source location.

**Returns:** `result_void` - Success or error with details

**Error Codes:**
- `queue_full`: The internal queue is full
- `queue_stopped`: Logger is shutting down

##### `add_writer`
```cpp
void add_writer(const std::string& name, std::unique_ptr<base_writer> writer);
result_void add_writer(std::unique_ptr<base_writer> writer);
```
Adds a writer to the logger.

##### `set_min_level`
```cpp
void set_min_level(thread_module::log_level level);
```
Sets the minimum log level. Messages below this level are discarded.

##### `enable_metrics_collection`
```cpp
void enable_metrics_collection(bool enable);
```
Enables or disables performance metrics collection.

##### `flush`
```cpp
result_void flush();
```
Flushes all pending log messages.

## Configuration

### `logger_module::logger_config`

Configuration structure with validation capabilities.

#### Members
```cpp
struct logger_config {
    // Basic settings
    bool async = true;
    std::size_t buffer_size = 8192;
    thread_module::log_level min_level = thread_module::log_level::info;
    
    // Performance settings
    std::size_t batch_size = 100;
    std::chrono::milliseconds flush_interval{1000};
    bool use_lock_free = false;
    
    // Feature flags
    bool enable_metrics = false;
    bool enable_crash_handler = false;
    bool enable_structured_logging = false;
    bool enable_color_output = true;
    
    // Queue settings
    std::size_t max_queue_size = 10000;
    overflow_policy queue_overflow_policy = overflow_policy::drop_newest;
    
    // File settings
    std::size_t max_file_size = 100 * 1024 * 1024;  // 100MB
    std::size_t max_file_count = 5;
    
    // Network settings
    std::string remote_host = "";
    uint16_t remote_port = 0;
    std::chrono::milliseconds network_timeout{5000};
};
```

#### Methods

##### `validate`
```cpp
result_void validate() const;
```
Validates the configuration.

**Returns:** `result_void` - Success or validation error

**Error Codes:**
- `invalid_configuration`: Configuration parameter is invalid

#### Static Factory Methods

##### `default_config`
```cpp
static logger_config default_config();
```
Returns default configuration.

##### `production`
```cpp
static logger_config production();
```
Returns production-optimized configuration.

##### `debug_config`
```cpp
static logger_config debug_config();
```
Returns debug-optimized configuration (synchronous, immediate output).

##### `high_performance`
```cpp
static logger_config high_performance();
```
Returns high-throughput configuration.

##### `low_latency`
```cpp
static logger_config low_latency();
```
Returns low-latency configuration for real-time systems.

## Builder Pattern

### `logger_module::logger_builder`

Fluent interface for constructing logger instances.

#### Methods

##### `with_config`
```cpp
logger_builder& with_config(const logger_config& config);
```
Sets the entire configuration.

##### `use_template`
```cpp
logger_builder& use_template(const std::string& template_name);
```
Uses a predefined configuration template.

**Templates:**
- `"default"`: Default configuration
- `"production"`: Production environment
- `"debug"`: Development/debugging
- `"high_performance"`: Maximum throughput
- `"low_latency"`: Minimum latency

##### `with_async`
```cpp
logger_builder& with_async(bool async = true);
```
Enables or disables asynchronous logging.

##### `with_buffer_size`
```cpp
logger_builder& with_buffer_size(std::size_t size);
```
Sets the buffer size.

##### `with_min_level`
```cpp
logger_builder& with_min_level(thread_module::log_level level);
```
Sets the minimum log level.

##### `with_metrics`
```cpp
logger_builder& with_metrics(bool enable = true);
```
Enables metrics collection.

##### `add_writer`
```cpp
logger_builder& add_writer(const std::string& name, 
                           std::unique_ptr<base_writer> writer);
```
Adds a named writer.

##### `add_filter`
```cpp
logger_builder& add_filter(std::unique_ptr<log_filter> filter);
```
Adds a filter.

##### `validate`
```cpp
result_void validate() const;
```
Validates the current configuration without building.

##### `build`
```cpp
result<std::unique_ptr<logger>> build();
```
Builds the logger with validation.

**Returns:** `result<unique_ptr<logger>>` - Logger instance or error

**Error Codes:**
- `invalid_configuration`: Configuration validation failed

## Interfaces

### `logger_module::log_writer_interface`

Interface for log writers.

```cpp
class log_writer_interface {
public:
    virtual ~log_writer_interface() = default;
    virtual result_void write(const log_entry& entry) = 0;
    virtual result_void flush() = 0;
    virtual bool is_healthy() const { return true; }
};
```

### `logger_module::log_filter_interface`

Interface for log filters.

```cpp
class log_filter_interface {
public:
    virtual ~log_filter_interface() = default;
    virtual bool should_log(const log_entry& entry) const = 0;
};
```

### `logger_module::log_formatter_interface`

Interface for log formatters.

```cpp
class log_formatter_interface {
public:
    virtual ~log_formatter_interface() = default;
    virtual std::string format(const log_entry& entry) const = 0;
};
```

### `logger_module::log_entry`

Unified structure for log data.

```cpp
struct log_entry {
    thread_module::log_level level;
    std::string message;
    std::string file;
    int line;
    std::string function;
    std::chrono::system_clock::time_point timestamp;
    std::thread::id thread_id;
    std::unordered_map<std::string, std::string> context;
};
```

## Error Handling

### Error Codes

```cpp
enum class logger_error_code {
    // General errors
    success = 0,
    unknown_error = 1,
    not_implemented = 2,
    invalid_argument = 3,
    
    // Writer errors (1000-1099)
    writer_not_found = 1000,
    writer_initialization_failed = 1001,
    writer_already_exists = 1002,
    writer_not_healthy = 1003,
    
    // File errors (1100-1199)
    file_open_failed = 1100,
    file_write_failed = 1101,
    file_rotation_failed = 1102,
    file_permission_denied = 1103,
    
    // Network errors (1200-1299)
    network_connection_failed = 1200,
    network_send_failed = 1201,
    network_timeout = 1202,
    
    // Buffer/Queue errors (1300-1399)
    buffer_overflow = 1300,
    queue_full = 1301,
    queue_stopped = 1302,
    
    // Configuration errors (1400-1499)
    invalid_configuration = 1400,
    configuration_missing = 1401,
    configuration_conflict = 1402
};
```

### Helper Functions

```cpp
// Create error result
result_void make_error(logger_error_code code, 
                       const std::string& message = "");

// Convert error code to string
std::string logger_error_to_string(logger_error_code code);
```

## Writers

### `logger_module::console_writer`

Writes logs to console with optional color support.

```cpp
class console_writer : public base_writer {
public:
    console_writer(bool use_color = true);
    void set_use_color(bool use_color);
};
```

### `logger_module::file_writer`

Writes logs to a file.

```cpp
class file_writer : public base_writer {
public:
    file_writer(const std::string& filename);
    result_void open();
    result_void close();
};
```

### `logger_module::rotating_file_writer`

File writer with rotation support.

```cpp
class rotating_file_writer : public base_writer {
public:
    enum class rotation_type {
        size_based,
        time_based,
        daily,
        hourly
    };
    
    rotating_file_writer(const std::string& filename,
                        std::size_t max_size,
                        std::size_t max_files);
};
```

## Filters

### `logger_module::level_filter`

Filters messages by log level.

```cpp
class level_filter : public log_filter {
public:
    level_filter(thread_module::log_level min_level);
};
```

### `logger_module::regex_filter`

Filters messages using regular expressions.

```cpp
class regex_filter : public log_filter {
public:
    regex_filter(const std::string& pattern, bool include = true);
};
```

### `logger_module::function_filter`

Custom filter using a function.

```cpp
class function_filter : public log_filter {
public:
    using filter_func = std::function<bool(const log_entry&)>;
    function_filter(filter_func func);
};
```

## Formatters

### `logger_module::plain_formatter`

Simple plain text formatter.

```cpp
class plain_formatter : public base_formatter {
public:
    std::string format(const log_entry& entry) const override;
};
```

### `logger_module::json_formatter`

JSON output formatter.

```cpp
class json_formatter : public base_formatter {
public:
    json_formatter(bool pretty_print = false);
    std::string format(const log_entry& entry) const override;
};
```

### `logger_module::compact_formatter`

Compact single-line formatter.

```cpp
class compact_formatter : public base_formatter {
public:
    std::string format(const log_entry& entry) const override;
};
```

## Usage Examples

### Basic Usage
```cpp
// Using builder pattern
auto result = logger_module::logger_builder()
    .use_template("production")
    .add_writer("console", std::make_unique<console_writer>())
    .build();

if (result) {
    auto logger = std::move(result.value());
    logger->log(thread_module::log_level::info, "Hello, World!");
}
```

### Error Handling
```cpp
auto log_result = logger->log(thread_module::log_level::info, "Message");
if (!log_result) {
    std::cerr << "Logging failed: " 
              << log_result.get_error().message() << "\n";
}
```

### Custom Writer
```cpp
class custom_writer : public logger_module::log_writer_interface {
public:
    result_void write(const logger_module::log_entry& entry) override {
        // Custom implementation
        return result_void{};  // Success
    }
    
    result_void flush() override {
        return result_void{};
    }
};
```

### Configuration Validation
```cpp
logger_config config;
config.buffer_size = 0;  // Invalid!

auto validation = config.validate();
if (!validation) {
    std::cerr << "Invalid config: " 
              << validation.get_error().message() << "\n";
}
```

## Thread Safety

All public methods of the logger class are thread-safe. Writers should implement their own synchronization if needed.

## Performance Considerations

- Use asynchronous mode for better performance
- Batch size affects latency vs throughput trade-off
- Lock-free queue provides better scalability (when enabled)
- Larger buffer sizes reduce contention but increase memory usage

## Advanced Features

### Structured Logging

```cpp
// Using structured fields with built-in logger methods
logger->info("User login", {
    {"user_id", 12345},
    {"ip_address", "192.168.1.1"},
    {"timestamp", std::chrono::system_clock::now()},
    {"success", true}
});

// Using log_entry directly for complex scenarios
log_entry entry;
entry.level = log_level::info;
entry.message = "Database query";
entry.context["query"] = "SELECT * FROM users";
entry.context["duration_ms"] = "45";
entry.context["rows_returned"] = "100";
// Entry would be processed by structured-aware writers
```

### Contextual Logging

```cpp
// Set thread-local context (implementation dependent)
// Context is included automatically in log entries
logger->log(log_level::info, "Processing request");

// Context is accessible through the log_entry structure
// in custom writers and formatters
```

### Performance Monitoring

```cpp
// Enable performance metrics collection
logger->enable_metrics_collection(true);

// Log some messages
for (int i = 0; i < 1000; ++i) {
    auto result = logger->log(log_level::info, "Test message");
    if (!result) {
        // Handle logging errors
        break;
    }
}

// Get current metrics
auto metrics = logger->get_current_metrics();
std::cout << "Messages per second: " << metrics.get_messages_per_second() << "\n";
std::cout << "Average enqueue time: " << metrics.get_avg_enqueue_time_ns() << " ns\n";
std::cout << "Queue utilization: " << metrics.get_queue_utilization_percent() << "%\n";
```

### Environment-based Configuration

```cpp
// Configuration can adapt based on environment variables
auto logger = logger_module::logger_builder()
    .use_template("production")  // Base template
    .with_config_validation(true) // Enable validation
    .build();

// Templates support environment variable overrides:
// LOG_LEVEL, LOG_ASYNC, LOG_BUFFER_SIZE, etc.
```

## Migration from v1.0

The v1.0 API is still supported for backward compatibility. New code should use the builder pattern and result types for better error handling.

```cpp
// Old way (still works)
auto logger = std::make_shared<logger_module::logger>();
logger->log(log_level::info, "Message");

// New way (recommended)
auto logger = logger_module::logger_builder()
    .use_template("production")
    .build()
    .value();  // or handle error properly
```