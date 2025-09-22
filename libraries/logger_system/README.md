# Logger System

[![CI](https://github.com/kcenon/logger_system/actions/workflows/ci.yml/badge.svg)](https://github.com/kcenon/logger_system/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

A high-performance, modular C++20 logging system with lock-free implementation designed for multithreaded applications. Part of the integrated threading ecosystem.

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
- **Thread-aware logging**: Automatic thread ID and context tracking
- **Performance optimized**: Lock-free design for high-throughput applications
- **Unified configuration**: Single point of configuration for entire ecosystem
- **Seamless integration**: Plug-and-play with thread pools and monitoring

> 📖 **[Complete Architecture Guide](../ARCHITECTURE.md)**: Comprehensive documentation of the entire ecosystem architecture, dependency relationships, and integration patterns.

## Features

- **Lock-free Implementation**: High-performance logging without mutex contention
- **Multiple Writers**: Console, file, and custom callback writers
- **Asynchronous Logging**: Non-blocking log operations
- **Thread-safe**: Designed for concurrent environments
- **Modular Design**: Easy integration with any C++ project
- **Low Latency**: Optimized for minimal overhead
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

## Integration with Thread System

This logger is designed to work seamlessly with the [Thread System](https://github.com/kcenon/thread_system) through dependency injection:

```cpp
#include <logger_system/logger.h>
#include <thread_system/interfaces/service_container.h>

// Register logger in the service container
auto logger = std::make_shared<logger_module::logger>();
logger->add_writer(std::make_unique<logger_module::console_writer>());

thread_module::service_container::global()
    .register_singleton<thread_module::logger_interface>(logger);

// Now thread system components will automatically use this logger
auto context = thread_module::thread_context(); // Will resolve logger from container
```

## Quick Start

### Basic Usage

```cpp
#include <logger_system/logger.h>

int main() {
    // Create logger instance
    auto logger = std::make_shared<logger_module::logger>();
    
    // Add console output
    logger->add_writer(std::make_unique<logger_module::console_writer>());
    
    // Add file output
    logger->add_writer(std::make_unique<logger_module::file_writer>("app.log"));
    
    // Log messages
    logger->log(log_level::info, "Application started");
    logger->log(log_level::error, "Something went wrong", __FILE__, __LINE__, __func__);
    
    return 0;
}
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

## Installation

```bash
cmake --build . --target install
```

## CMake Integration

```cmake
find_package(LoggerSystem REQUIRED)
target_link_libraries(your_target PRIVATE LoggerSystem::logger)
```

## License

BSD 3-Clause License - see LICENSE file for details.