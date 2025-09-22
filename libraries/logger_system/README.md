[![CodeFactor](https://www.codefactor.io/repository/github/kcenon/logger_system/badge)](https://www.codefactor.io/repository/github/kcenon/logger_system)

[![Ubuntu-GCC](https://github.com/kcenon/logger_system/actions/workflows/build-ubuntu-gcc.yaml/badge.svg)](https://github.com/kcenon/logger_system/actions/workflows/build-ubuntu-gcc.yaml)
[![Ubuntu-Clang](https://github.com/kcenon/logger_system/actions/workflows/build-ubuntu-clang.yaml/badge.svg)](https://github.com/kcenon/logger_system/actions/workflows/build-ubuntu-clang.yaml)
[![Windows-MSYS2](https://github.com/kcenon/logger_system/actions/workflows/build-windows-msys2.yaml/badge.svg)](https://github.com/kcenon/logger_system/actions/workflows/build-windows-msys2.yaml)
[![Windows-VisualStudio](https://github.com/kcenon/logger_system/actions/workflows/build-windows-vs.yaml/badge.svg)](https://github.com/kcenon/logger_system/actions/workflows/build-windows-vs.yaml)
[![Docs](https://github.com/kcenon/logger_system/actions/workflows/build-Doxygen.yaml/badge.svg)](https://github.com/kcenon/logger_system/actions/workflows/build-Doxygen.yaml)

# Logger System Project

## Project Overview

The Logger System Project is a production-ready, high-performance C++20 asynchronous logging framework designed to provide comprehensive logging capabilities for multithreaded applications. Built with a modular, interface-based architecture and seamless integration with the thread system ecosystem, it delivers enterprise-grade logging performance with minimal overhead and maximum flexibility.

> **🏗️ Modular Architecture**: Streamlined interface-based design with pluggable components for writers, filters, formatters, and monitoring integration.

> **✅ Latest Updates**: Enhanced dependency injection, configuration strategy patterns, comprehensive validation, and extensive CMake modularization. All CI/CD pipelines green across platforms.

## 🚀 Recent Improvements

### Phase 2 - Core Systems (Complete - 100% ✅)
- **Adaptive Dependency Injection** [C1] ✅: Abstract DI interface with lightweight implementation
  - Zero external dependencies by default
  - Optional thread_system integration
  - Runtime component injection with fallback
- **Pluggable Monitoring System** [C2] ✅: Flexible monitoring backend architecture
  - Basic metrics collector (standalone)
  - Health check system (healthy/degraded/unhealthy)
  - Thread-safe metric collection with minimal overhead
- **Configuration Strategy Pattern** [C3] ✅: Flexible configuration management
  - Template, environment, and performance tuning strategies
  - Composite strategies with priority ordering
  - Automatic environment detection from LOG_ENV and LOG_LEVEL
- **CMake Modularization** [C4] ✅: Comprehensive build system with feature flags
  - 15+ configurable feature options
  - Automatic dependency detection
  - Package configuration for find_package() support

### Phase 1 - Foundation (Complete)
- **Result Pattern Error Handling**: Comprehensive error handling using `result<T>` pattern from thread_system
- **Configuration Validation**: Robust validation framework with predefined templates
- **Builder Pattern**: Fluent interface for logger construction with automatic validation
- **Interface Segregation**: Clean separation of concerns with dedicated interfaces for writers, filters, formatters, and sinks
- **Enhanced Type Safety**: Strong typing throughout the API with comprehensive error codes

Implementation note: The current asynchronous pipeline uses a mutex/condition_variable backed queue for batching. A lock-free MPMC queue is planned, and the `USE_LOCKFREE` option is reserved for that future implementation.

## 🔗 Project Ecosystem Integration

This logger system is a component of a comprehensive threading and monitoring ecosystem:

### Project Dependencies
- **[thread_system](https://github.com/kcenon/thread_system)**: Core dependency providing `logger_interface`
  - Implements: `thread_module::logger_interface`
  - Provides: Interface contracts for seamless integration
  - Role: Foundation interfaces for logging subsystem

### Related Projects
- **[monitoring_system](https://github.com/kcenon/monitoring_system)**: Complementary metrics collection
  - Relationship: Both integrate with thread_system
  - Synergy: Combined logging and monitoring for complete observability
  - Integration: Can log monitoring events and metrics

- **[integrated_thread_system](https://github.com/kcenon/integrated_thread_system)**: Complete integration examples
  - Usage: Demonstrates logger_system integration patterns
  - Benefits: Production-ready examples with full ecosystem
  - Reference: Complete application templates

### Integration Architecture
```
┌─────────────────┐
│  thread_system  │ ← Core interfaces (logger_interface)
└─────────┬───────┘
          │ implements
┌─────────▼───────┐     ┌─────────────────┐
│  logger_system  │ ◄──► │monitoring_system│
└─────────────────┘     └─────────────────┘
          │                       │
          └───────┬───────────────┘
                  ▼
    ┌─────────────────────────┐
    │integrated_thread_system │
    └─────────────────────────┘
```

### Integration Benefits
- **Plug-and-play**: Use only the components you need
- **Interface-driven**: Clean abstractions enable easy swapping
- **Performance-optimized**: Asynchronous batching optimized for high throughput
- **Unified ecosystem**: Consistent API design across all projects

> 📖 **[Complete Architecture Guide](docs/ARCHITECTURE.md)**: Comprehensive documentation of the entire ecosystem architecture, dependency relationships, and integration patterns.

## Project Purpose & Mission

This project addresses the fundamental challenge faced by developers worldwide: **making high-performance logging accessible, reliable, and efficient**. Traditional logging approaches often become bottlenecks in high-throughput applications, lack proper error handling, and provide insufficient observability. Our mission is to provide a comprehensive solution that:

- **Eliminates logging bottlenecks** through asynchronous, batched processing
- **Ensures data integrity** with comprehensive error handling and validation
- **Maximizes performance** through optimized algorithms and modern C++ features
- **Promotes maintainability** through clean interfaces and modular architecture
- **Accelerates debugging** by providing rich, structured logging capabilities

## Core Advantages & Benefits

### 🚀 **Performance Excellence**
- **Asynchronous processing**: Background thread handles I/O operations without blocking
- **Batch optimization**: Processes multiple log entries efficiently to maximize throughput
- **Minimal overhead**: Zero-allocation formatting and optimized data structures
- **Adaptive queuing**: Intelligent backoff and batching strategies for optimal resource utilization

### 🛡️ **Production-Grade Reliability**
- **Thread-safe by design**: All components guarantee safe concurrent access
- **Comprehensive error handling**: Result pattern ensures no silent failures
- **Memory safety**: RAII principles and smart pointers prevent leaks and corruption
- **Extensive validation**: Configuration validation prevents runtime errors

### 🔧 **Developer Productivity**
- **Intuitive API design**: Clean, self-documenting interfaces reduce learning curve
- **Rich documentation**: Comprehensive documentation with practical examples
- **Flexible configuration**: Template-based configurations for common scenarios
- **Modular components**: Use only what you need - maximum flexibility

### 🌐 **Cross-Platform Compatibility**
- **Universal support**: Works on Windows, Linux, and macOS
- **Compiler flexibility**: Compatible with GCC, Clang, and MSVC
- **C++ standard adaptation**: Graceful fallback from C++20 to older standards
- **Architecture independence**: Optimized for both x86 and ARM processors

### 📈 **Enterprise-Ready Features**
- **Structured logging**: JSON, logfmt, and custom format support
- **Advanced filtering**: Level-based, regex, and custom function filters
- **Network logging**: TCP/UDP remote logging capabilities
- **Security features**: Encryption, sanitization, and access control

## Real-World Impact & Use Cases

### 🎯 **Ideal Applications**
- **High-frequency trading systems**: Ultra-low latency logging for trade execution monitoring
- **Web servers**: Concurrent request logging with minimal performance impact
- **Microservices**: Distributed logging with structured data and correlation IDs
- **Game engines**: Real-time event logging without affecting frame rates
- **IoT systems**: Efficient logging for resource-constrained devices
- **Database systems**: Query logging and performance monitoring

### 📊 **Performance Benchmarks**

*Benchmarked on Apple M1 (8-core) @ 3.2GHz, 16GB, macOS Sonoma*

> **🚀 Architecture Update**: Latest modular architecture provides seamless integration with thread_system ecosystem. Asynchronous processing delivers exceptional performance without blocking application threads.

#### Core Performance Metrics (Latest Benchmarks)
- **Peak Throughput**: Up to 4.34M messages/second (single thread, async mode)
- **Multi-threaded Performance**:
  - 4 threads: 1.07M messages/s (24% better than standard mode)
  - 8 threads: 412K messages/s (78% improvement with adaptive batching)
  - 16 threads: 390K messages/s (117% boost in high-contention scenarios)
- **Latency**:
  - Average enqueue time: 148 nanoseconds
  - 15.7x lower latency compared to spdlog
- **Memory efficiency**: <2MB baseline with adaptive buffer management
- **Queue utilization**: Automatic optimization maintains high throughput

#### Performance Comparison with Industry Standards
| Logger Type | Single Thread | 4 Threads | 8 Threads | 16 Threads | Best Use Case |
|-------------|---------------|-----------|-----------|------------|---------------|
| 🏆 **Logger System** | **4.34M/s** | **1.07M/s** | **412K/s** | **390K/s** | All scenarios (adaptive) |
| 📦 **spdlog async** | 5.35M/s | 785K/s | 240K/s | - | Single-thread focused |
| 📦 **spdlog sync** | 515K/s | 210K/s | 52K/s | - | Simple applications |
| 🐌 **Console output** | 583K/s | - | - | - | Development only |

#### Key Performance Insights
- 🏃 **Single-thread**: Competitive with industry leaders (4.34M/s)
- 🏋️ **Multi-thread**: Adaptive batching provides consistent scaling
- ⏱️ **Latency**: Industry-leading 148ns average enqueue time
- 📈 **Scalability**: Maintains performance under high contention

## Features

### Core Features
- **Asynchronous Pipeline**: Background thread processes batched log entries
- **Multiple Writers**: Console, file, and custom callback writers
- **Thread-safe**: Designed for concurrent environments
- **Modular Design**: Easy integration with any C++ project
- **Low Latency**: Optimized for minimal overhead

### Enhanced Architecture (New in Phase 1)
- **Result Pattern**: Type-safe error handling with `result<T>` and `result_void`
- **Configuration Validation**: Comprehensive validation with meaningful error messages
- **Builder Pattern**: Fluent API for logger construction
- **Interface Segregation**: Clean separation of writer, filter, formatter, and sink interfaces
- **Predefined Templates**: Production, debug, high_performance, and low_latency configurations

### Advanced Features
- **Performance Metrics**: Built-in metrics collection for monitoring logger performance
- **Structured Logging**: Support for JSON, logfmt, and plain text output formats
- **Advanced Filtering**: Level-based, regex, and custom function filters
- **Flexible Routing**: Route logs to specific writers based on conditions
- **File Writers**: Basic and rotating file writers with size/time-based rotation
- **Network Logging**: Send logs to remote servers via TCP/UDP
- **Log Server**: Receive and process logs from multiple sources
- **Real-time Analysis**: Analyze log patterns and generate statistics
- **Alert System**: Define rules to trigger alerts based on log patterns
- **Security Features**: Log encryption, sensitive data sanitization, and access control
- **Integration Testing**: Comprehensive test suite for all components

> Security note: `encrypted_writer` is a demonstration component using a simple XOR scheme and is not suitable for production use. See SECURITY.md for guidance and recommended alternatives.

## Technology Stack & Architecture

### 🏗️ **Modern C++ Foundation**
- **C++20 features**: `std::format`, concepts, and ranges for enhanced performance
- **Template metaprogramming**: Type-safe, compile-time optimizations
- **Memory management**: Smart pointers and RAII for automatic resource cleanup
- **Exception safety**: Strong exception safety guarantees throughout
- **Result pattern**: Comprehensive error handling without exceptions
- **Interface-based design**: Clean separation between interface and implementation
- **Modular architecture**: Core logging functionality with optional ecosystem integration

### 🔄 **Design Patterns Implementation**
- **Observer Pattern**: Event-driven log processing and filtering
- **Strategy Pattern**: Configurable formatters, filters, and writers
- **Builder Pattern**: Fluent API for logger configuration with validation
- **Template Method Pattern**: Customizable writer and formatter behavior
- **Dependency Injection**: Service container integration for ecosystem components
- **Factory Pattern**: Configurable writer and filter creation

## Project Structure

### 📁 **Directory Organization**

```
logger_system/
├── 📁 include/kcenon/logger/       # Public headers
│   ├── 📁 core/                    # Core components
│   │   ├── logger.h                # Main logger interface
│   │   ├── logger_builder.h        # Builder pattern implementation
│   │   ├── log_entry.h             # Log entry data structure
│   │   └── result_types.h          # Error handling types
│   ├── 📁 interfaces/              # Abstract interfaces
│   │   ├── log_writer_interface.h  # Writer abstraction
│   │   ├── log_filter_interface.h  # Filter abstraction
│   │   ├── log_formatter_interface.h # Formatter abstraction
│   │   └── monitoring_interface.h  # Monitoring integration
│   ├── 📁 writers/                 # Log writers
│   │   ├── console_writer.h        # Console output
│   │   ├── file_writer.h           # File output
│   │   ├── rotating_file_writer.h  # Rotating files
│   │   └── network_writer.h        # Network logging
│   ├── 📁 filters/                 # Log filters
│   │   ├── level_filter.h          # Level-based filtering
│   │   ├── regex_filter.h          # Regex-based filtering
│   │   └── function_filter.h       # Custom function filtering
│   ├── 📁 formatters/              # Log formatters
│   │   ├── plain_formatter.h       # Plain text formatting
│   │   ├── json_formatter.h        # JSON formatting
│   │   └── custom_formatter.h      # Custom formatting
│   └── 📁 config/                  # Configuration
│       ├── config_templates.h      # Predefined configurations
│       ├── config_validator.h      # Configuration validation
│       └── config_strategy.h       # Strategy pattern configs
├── 📁 src/                         # Implementation files
│   ├── 📁 core/                    # Core implementations
│   ├── 📁 writers/                 # Writer implementations
│   ├── 📁 filters/                 # Filter implementations
│   ├── 📁 formatters/              # Formatter implementations
│   └── 📁 config/                  # Configuration implementations
├── 📁 examples/                    # Example applications
│   ├── basic_logging/              # Basic usage examples
│   ├── advanced_features/          # Advanced feature demos
│   ├── performance_test/           # Performance benchmarks
│   └── integration_examples/       # Ecosystem integration
├── 📁 tests/                       # All tests
│   ├── 📁 unit/                    # Unit tests
│   ├── 📁 integration/             # Integration tests
│   └── 📁 benchmarks/              # Performance tests
├── 📁 docs/                        # Documentation
├── 📁 cmake/                       # CMake modules
├── 📄 CMakeLists.txt               # Build configuration
└── 📄 vcpkg.json                   # Dependencies
```

### 📖 **Key Files and Their Purpose**

#### Core Module Files
- **`logger.h/cpp`**: Main logger class with asynchronous processing
- **`logger_builder.h/cpp`**: Builder pattern for logger configuration
- **`log_entry.h`**: Data structure for log entries with metadata
- **`result_types.h`**: Error handling types and utilities

#### Writer Files
- **`console_writer.h/cpp`**: Colored console output with ANSI support
- **`file_writer.h/cpp`**: Basic file writing with buffering
- **`rotating_file_writer.h/cpp`**: Size and time-based file rotation
- **`network_writer.h/cpp`**: TCP/UDP network logging

#### Configuration Files
- **`config_templates.h/cpp`**: Production, debug, high-performance templates
- **`config_validator.h/cpp`**: Comprehensive validation framework
- **`config_strategy.h/cpp`**: Strategy pattern for flexible configuration

### 🔗 **Module Dependencies**

```
config (no dependencies)
    │
    └──> core
            │
            ├──> writers
            │
            ├──> filters
            │
            ├──> formatters
            │
            └──> integration (thread_system, monitoring_system)

Optional External Projects:
- thread_system (provides logger_interface)
- monitoring_system (provides metrics collection)
```

## Build Configuration

### CMake Feature Flags

The logger system provides extensive configuration options through CMake:

```bash
# Core Features
cmake -DLOGGER_USE_DI=ON              # Enable dependency injection (default: ON)
cmake -DLOGGER_USE_MONITORING=ON      # Enable monitoring support (default: ON)
cmake -DLOGGER_ENABLE_ASYNC=ON        # Enable async logging (default: ON)
cmake -DLOGGER_ENABLE_CRASH_HANDLER=ON # Enable crash handler (default: ON)

# Advanced Features
cmake -DLOGGER_USE_LOCK_FREE_QUEUE=ON # Use lock-free queue (default: follows USE_LOCKFREE)
cmake -DLOGGER_ENABLE_STRUCTURED_LOGGING=ON # JSON logging (default: OFF)
cmake -DLOGGER_ENABLE_NETWORK_WRITER=ON # Network log writer (default: OFF)
cmake -DLOGGER_ENABLE_FILE_ROTATION=ON  # File rotation (default: ON)

# Performance Tuning
cmake -DLOGGER_DEFAULT_BUFFER_SIZE=16384 # Buffer size in bytes
cmake -DLOGGER_DEFAULT_BATCH_SIZE=200    # Batch processing size
cmake -DLOGGER_DEFAULT_QUEUE_SIZE=20000  # Maximum queue size

# Build Options
cmake -DLOGGER_FORCE_LIGHTWEIGHT=ON   # Force lightweight implementations (default: ON)

# Quality Assurance Options (New in Phase 5 P5)
cmake -DLOGGER_ENABLE_SANITIZERS=ON   # Enable sanitizers in debug builds
cmake -DLOGGER_SANITIZER_TYPE=address # Sanitizer type (address/thread/undefined/memory)
cmake -DLOGGER_ENABLE_WARNINGS=ON     # Enable comprehensive compiler warnings
cmake -DLOGGER_WARNINGS_AS_ERRORS=ON  # Treat warnings as errors
cmake -DLOGGER_ENABLE_COVERAGE=ON     # Enable code coverage reporting
cmake -DLOGGER_USE_EXTERNAL_DI=OFF    # Use external DI container (default: OFF)
cmake -DLOGGER_ENABLE_SANITIZERS=ON   # Enable sanitizers for debugging
cmake -DLOGGER_ENABLE_COVERAGE=ON     # Enable code coverage
```

### Using as a Package

After installation, use the logger system in your CMake project:

```cmake
find_package(LoggerSystem REQUIRED)

target_link_libraries(your_app 
    PRIVATE 
        LoggerSystem::logger
)

# Optional: Print configuration
LoggerSystem_print_configuration()
```

## Integration with Thread System

This logger is designed to work seamlessly with the [Thread System](https://github.com/kcenon/thread_system) through dependency injection:

```cpp
#include <kcenon/logger/core/logger.h>
#include <kcenon/thread/interfaces/service_container.h>

// Register logger in the service container
auto logger = std::make_shared<kcenon::logger::logger>();
logger->add_writer(std::make_unique<kcenon::logger::console_writer>());

kcenon::thread::service_container::global()
    .register_singleton<kcenon::thread::interfaces::logger_interface>(logger);

// Now thread system components will automatically use this logger
auto context = kcenon::thread::thread_context(); // Will resolve logger from container
```

## Quick Start

### Quick Start with Builder Pattern (Recommended)

```cpp
#include <kcenon/logger/core/logger_builder.h>
#include <kcenon/logger/writers/console_writer.h>
#include <kcenon/logger/writers/file_writer.h>

int main() {
    // Create logger using builder with automatic validation
    auto result = kcenon::logger::logger_builder()
        .use_template("production")  // Use predefined configuration
        .with_min_level(kcenon::logger::log_level::info)
        .add_writer("console", std::make_unique<kcenon::logger::console_writer>())
        .add_writer("file", std::make_unique<kcenon::logger::file_writer>("app.log"))
        .build();

    if (!result) {
        std::cerr << "Failed to create logger: " << result.get_error().message() << "\n";
        return -1;
    }

    auto logger = std::move(result.value());

    // Log messages with error handling
    auto log_result = logger->log(kcenon::logger::log_level::info, "Application started");
    if (!log_result) {
        std::cerr << "Log failed: " << log_result.get_error().message() << "\n";
    }

    return 0;
}
```

### Configuration Templates

```cpp
// Production configuration - optimized for production environments
auto prod_logger = kcenon::logger::logger_builder()
    .use_template("production")
    .build()
    .value();  // Throws on error

// Debug configuration - immediate output for development
auto debug_logger = kcenon::logger::logger_builder()
    .use_template("debug")
    .build()
    .value();

// High-performance - maximized throughput
auto hp_logger = kcenon::logger::logger_builder()
    .use_template("high_performance")
    .build()
    .value();

// Low-latency - minimized latency for real-time systems
auto rt_logger = logger_module::logger_builder()
    .use_template("low_latency")
    .build()
    .value();
```

### Traditional API (Legacy Support)

```cpp
#include <logger/logger.h>

int main() {
    // Create logger instance
    auto logger = std::make_shared<logger_module::logger>();
    
    // Add console output
    logger->add_writer(std::make_unique<logger_module::console_writer>());
    
    // Add file output
    logger->add_writer(std::make_unique<logger_module::file_writer>("app.log"));
    
    // Log messages
    logger->log(thread_module::log_level::info, "Application started");
    logger->log(thread_module::log_level::error, "Something went wrong", __FILE__, __LINE__, __func__);
    
    return 0;
}
```

### Error Handling with Result Pattern

```cpp
// All operations return result types for comprehensive error handling
auto result = logger->log(thread_module::log_level::info, "Message");
if (!result) {
    // Handle error
    auto error = result.get_error();
    std::cerr << "Log failed: " << error.message() << " (code: " 
              << static_cast<int>(error.code()) << ")\n";
    
    // Take appropriate action based on error code
    switch (error.code()) {
        case thread_module::error_code::queue_full:
            // Handle queue overflow
            break;
        case thread_module::error_code::queue_stopped:
            // Logger is shutting down
            break;
        default:
            // Handle other errors
            break;
    }
}

// Builder pattern with validation
auto builder_result = logger_module::logger_builder()
    .with_buffer_size(0)  // Invalid!
    .build();

if (!builder_result) {
    // Configuration validation failed
    std::cerr << "Invalid configuration: " 
              << builder_result.get_error().message() << "\n";
}
```

### Interface Architecture

```cpp
// New clean interface separation
#include <logger/interfaces/log_writer_interface.h>
#include <logger/interfaces/log_filter_interface.h>
#include <logger/interfaces/log_formatter_interface.h>

// Implement custom writer
class custom_writer : public logger_module::log_writer_interface {
public:
    result_void write(const logger_module::log_entry& entry) override {
        // Your custom implementation
        return result_void{};  // Success
    }
    
    result_void flush() override {
        return result_void{};
    }
};

// Implement custom filter
class custom_filter : public logger_module::log_filter_interface {
public:
    bool should_log(const logger_module::log_entry& entry) const override {
        // Filter logic
        return entry.level >= thread_module::log_level::warning;
    }
};

// Implement custom formatter
class custom_formatter : public logger_module::log_formatter_interface {
public:
    std::string format(const logger_module::log_entry& entry) const override {
        // Format log entry
        return fmt::format("[{}] {}", entry.level, entry.message);
    }
};
```

### Performance Metrics

```cpp
// Enable metrics collection
logger->enable_metrics_collection(true);

// Log some messages
for (int i = 0; i < 1000; ++i) {
    logger->log(log_level::info, "Test message");
}

// Get current metrics
auto metrics = logger->get_current_metrics();
std::cout << "Messages per second: " << metrics.get_messages_per_second() << "\n";
std::cout << "Average enqueue time: " << metrics.get_avg_enqueue_time_ns() << " ns\n";
std::cout << "Queue utilization: " << metrics.get_queue_utilization_percent() << "%\n";
```

### Structured Logging

```cpp
#include <logger_system/structured/structured_logger.h>

// Create structured logger wrapper
auto structured = std::make_shared<logger_module::structured_logger>(
    logger, 
    logger_module::structured_logger::output_format::json
);

// Log with structured fields
structured->info("User logged in")
    .field("user_id", 12345)
    .field("ip_address", "192.168.1.1")
    .field("session_duration", 3600)
    .commit();

// Output (JSON format):
// {"@timestamp":"2025-01-27T08:30:00Z","level":"INFO","message":"User logged in","thread_id":"12345","user_id":12345,"ip_address":"192.168.1.1","session_duration":3600}
```

### Advanced Filtering and Routing

```cpp
#include <logger_system/filters/log_filter.h>
#include <logger_system/routing/log_router.h>

// Set up filtering - only log warnings and above
logger->set_filter(std::make_unique<level_filter>(log_level::warning));

// Filter out sensitive information
logger->set_filter(std::make_unique<regex_filter>("password|secret", false));

// Set up routing
auto& router = logger->get_router();

// Route errors to a dedicated error file
router_builder(router)
    .when_level(log_level::error)
    .route_to("error_file", true);  // Stop propagation

// Route debug messages to both debug file and console
router_builder(router)
    .when_level(log_level::debug)
    .route_to(std::vector<std::string>{"debug_file", "console"});

// Custom filter function
auto custom_filter = std::make_unique<function_filter>(
    [](log_level level, const std::string& msg, 
       const std::string& file, int line, const std::string& func) {
        // Only log messages from specific files
        return file.find("critical_module") != std::string::npos;
    }
);
logger->set_filter(std::move(custom_filter));
```

### File Writers

```cpp
#include <logger_system/writers/file_writer.h>
#include <logger_system/writers/rotating_file_writer.h>

// Basic file writer
logger->add_writer("main_log", std::make_unique<file_writer>("logs/app.log"));

// Rotating file writer - size based
logger->add_writer("rotating", std::make_unique<rotating_file_writer>(
    "logs/app.log",
    1024 * 1024 * 10,  // 10MB per file
    5                   // Keep 5 backup files
));

// Rotating file writer - time based (daily)
logger->add_writer("daily", std::make_unique<rotating_file_writer>(
    "logs/daily.log",
    rotating_file_writer::rotation_type::daily,
    30  // Keep 30 days of logs
));
```

### Distributed Logging

```cpp
#include <logger_system/writers/network_writer.h>
#include <logger_system/server/log_server.h>

// Send logs to remote server
logger->add_writer("remote", std::make_unique<network_writer>(
    "log-server.example.com",
    9999,
    network_writer::protocol_type::tcp
));

// Create log server to receive logs
auto server = std::make_unique<log_server>(9999, true);
server->add_handler([](const log_server::network_log_entry& entry) {
    std::cout << "Received log from " << entry.source_address 
              << ": " << entry.raw_data << std::endl;
});
server->start();
```

### Real-time Analysis

```cpp
#include <logger_system/analysis/log_analyzer.h>

// Create analyzer with 60-second windows
auto analyzer = std::make_unique<log_analyzer>(
    std::chrono::seconds(60),
    60  // Keep 1 hour of history
);

// Track patterns
analyzer->add_pattern("errors", "error|fail|exception");
analyzer->add_pattern("slow_queries", "query took \\d{4,} ms");

// Add alert rules
analyzer->add_alert_rule({
    "high_error_rate",
    [](const auto& stats) {
        auto errors = stats.level_counts.count(log_level::error) ? 
                     stats.level_counts.at(log_level::error) : 0;
        return errors > 100;  // Alert if >100 errors per minute
    },
    [](const std::string& rule, const auto& stats) {
        std::cout << "ALERT: High error rate detected!" << std::endl;
    }
});

// Analyze logs
analyzer->analyze(level, message, file, line, function, timestamp);

// Generate report
std::string report = analyzer->generate_report(std::chrono::minutes(10));
```

### Security Features

#### Log Encryption

```cpp
#include <logger_system/writers/encrypted_writer.h>

// Generate encryption key
auto key = encrypted_writer::generate_key(32);  // 32 bytes for AES-256

// Save key securely
encrypted_writer::save_key(key, "logger.key");

// Create encrypted writer
auto file = std::make_unique<file_writer>("secure.log");
auto encrypted = std::make_unique<encrypted_writer>(std::move(file), key);
logger->add_writer("secure", std::move(encrypted));

// Note: Demo uses XOR encryption - use proper crypto library in production
```

#### Sensitive Data Sanitization

```cpp
#include <logger_system/security/log_sanitizer.h>

// Create sanitizer with default rules
auto sanitizer = std::make_shared<log_sanitizer>();

// Sanitize logs before writing
std::string message = "User login: john.doe@example.com, Card: 4532-1234-5678-9012";
std::string sanitized = sanitizer->sanitize(message);
// Result: "User login: j******e@example.com, Card: 4532********9012"

// Add custom sanitization rules
sanitizer->add_rule({
    "jwt_token",
    std::regex("Bearer\\s+[A-Za-z0-9\\-_]+\\.[A-Za-z0-9\\-_]+\\.[A-Za-z0-9\\-_]+"),
    [](const std::smatch& match) { return "Bearer [REDACTED]"; }
});
```

#### Access Control

```cpp
#include <logger_system/security/log_sanitizer.h>

// Create access control filter
auto access_filter = std::make_unique<access_control_filter>(
    access_control_filter::permission_level::write_info
);

// Set file-specific permissions
access_filter->set_file_permission(".*secure.*", 
    access_control_filter::permission_level::admin);

// Set user context
access_filter->set_user_context("current_user", 
    access_control_filter::permission_level::write_info);

logger->set_filter(std::move(access_filter));
```

### Custom Writers

```cpp
class custom_writer : public logger_module::base_writer {
public:
    bool write(thread_module::log_level level,
               const std::string& message,
               const std::string& file,
               int line,
               const std::string& function,
               const std::chrono::system_clock::time_point& timestamp) override {
        // Custom implementation
        return true;
    }
    
    void flush() override {
        // Flush implementation
    }
    
    std::string get_name() const override {
        return "custom";
    }
};
```

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Build Options

- `BUILD_TESTS`: Build unit tests (default: ON)
- `BUILD_BENCHMARKS`: Build performance benchmarks (default: OFF)
- `BUILD_SAMPLES`: Build example programs (default: ON)
- `USE_LOCKFREE`: Use lock-free implementation (default: ON)

## Testing

After building with `BUILD_TESTS=ON` (default), run the integration tests:

```bash
ctest --test-dir build
# or
./build/bin/integration_test
```

## Platform Support

- Linux and macOS fully supported for console/file writers and POSIX networking.
- Windows support is partial; network/server components require WinSock initialization and minor adaptations. Contributions are welcome.

## FAQ

- Is the logger lock-free?
  - The current async queue uses mutex/condition_variable for portability and simplicity. A lock-free MPMC queue is planned; see the `USE_LOCKFREE` placeholder.
- Is `encrypted_writer` production-ready?
  - No. It is a demonstration. Use a vetted crypto library and authenticated encryption (e.g., AES-GCM, ChaCha20-Poly1305) with proper key management.
- How do I route only errors to a dedicated file?
  - Use `router_builder(router).when_level(log_level::error).route_to("error_file", true);` and register a writer under that name.
- How do I get JSON output?
  - Use `structured_logger` with `output_format::json`. For strict JSON compliance at scale, consider integrating a JSON library (e.g., nlohmann/json).

## Further Reading

- [Getting Started Guide](docs/guides/GETTING_STARTED.md) - Step-by-step setup and basic usage
- [Best Practices](docs/guides/BEST_PRACTICES.md) - Production-ready patterns and recommendations  
- [Performance Guide](docs/guides/PERFORMANCE.md) - Performance analysis and optimization
- [Custom Writers](docs/advanced/CUSTOM_WRITERS.md) - Creating custom log writers
- [API Reference](docs/API_REFERENCE.md) - Complete API documentation
- [System Architecture](docs/LOGGER_SYSTEM_ARCHITECTURE.md) - Detailed technical architecture
- [Security Guide](docs/SECURITY.md) - Security considerations and reporting
- [Complete Documentation](docs/README.md) - Full documentation index

## Installation

```bash
cmake --build . --target install
```

## CMake Integration

```cmake
find_package(LoggerSystem REQUIRED)
target_link_libraries(your_target PRIVATE LoggerSystem::logger)
```

## API Documentation

### Core API Reference

- **[API Reference](./docs/API_REFERENCE.md)**: Complete API documentation with interfaces
- **[Architecture Guide](./docs/ARCHITECTURE.md)**: System design and internals
- **[Performance Guide](./docs/PERFORMANCE.md)**: Optimization tips and benchmarks
- **[User Guide](./docs/USER_GUIDE.md)**: Usage guide and examples
- **[FAQ](./docs/FAQ.md)**: Frequently asked questions

### Quick API Overview

```cpp
// Logger Core API
namespace kcenon::logger {
    // Main logger with async processing
    class logger {
        auto log(log_level level, const std::string& message) -> result_void;
        auto add_writer(const std::string& name, std::unique_ptr<log_writer_interface> writer) -> result_void;
        auto set_filter(std::unique_ptr<log_filter_interface> filter) -> result_void;
        auto enable_metrics_collection(bool enabled) -> void;
        auto get_current_metrics() const -> metrics_data;
        auto flush() -> result_void;
    };

    // Builder pattern for configuration
    class logger_builder {
        auto use_template(const std::string& template_name) -> logger_builder&;
        auto with_min_level(log_level level) -> logger_builder&;
        auto with_buffer_size(size_t size) -> logger_builder&;
        auto with_batch_size(size_t size) -> logger_builder&;
        auto add_writer(const std::string& name, std::unique_ptr<log_writer_interface> writer) -> logger_builder&;
        auto build() -> result<std::unique_ptr<logger>>;
    };

    // Structured logging wrapper
    class structured_logger {
        auto info(const std::string& message) -> structured_entry&;
        auto error(const std::string& message) -> structured_entry&;
        auto field(const std::string& key, const auto& value) -> structured_entry&;
        auto commit() -> result_void;
    };
}

// Writer Interfaces
namespace kcenon::logger {
    class log_writer_interface {
        virtual auto write(const log_entry& entry) -> result_void = 0;
        virtual auto flush() -> result_void = 0;
    };

    class console_writer : public log_writer_interface { /* ANSI colored output */ };
    class file_writer : public log_writer_interface { /* Buffered file output */ };
    class rotating_file_writer : public log_writer_interface { /* Size/time rotation */ };
    class network_writer : public log_writer_interface { /* TCP/UDP remote */ };
}

// Integration API (with thread_system)
namespace kcenon::thread::interfaces {
    class logger_interface {
        virtual auto log(log_level level, const std::string& message) -> result_void = 0;
        virtual auto enable_metrics(bool enabled) -> void = 0;
    };
}
```

## Contributing

We welcome contributions! Please see our [Contributing Guide](./docs/CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style

- Follow modern C++ best practices
- Use RAII and smart pointers
- Maintain consistent formatting (clang-format configuration provided)
- Write comprehensive unit tests for new features

## Support

- **Issues**: [GitHub Issues](https://github.com/kcenon/logger_system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/kcenon/logger_system/discussions)
- **Email**: kcenon@naver.com

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to all contributors who have helped improve this project
- Special thanks to the C++ community for continuous feedback and support
- Inspired by modern logging frameworks and best practices

---

<p align="center">
  Made with ❤️ by 🍀☀🌕🌥 🌊
</p>
