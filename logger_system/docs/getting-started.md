# Getting Started with Logger System

This guide will help you get started with the Logger System quickly.

## Table of Contents
- [Requirements](#requirements)
- [Installation](#installation)
- [Basic Usage](#basic-usage)
- [Configuration](#configuration)
- [Integration with Thread System](#integration-with-thread-system)

## Requirements

- C++20 compatible compiler:
  - GCC 10+ 
  - Clang 12+
  - MSVC 2019+ (Visual Studio 2019 version 16.11+)
- CMake 3.16 or higher
- Operating System:
  - Linux (Ubuntu 20.04+, CentOS 8+, etc.)
  - macOS 10.15+
  - Windows 10+

## Installation

### Using CMake FetchContent (Recommended)

Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    LoggerSystem
    GIT_REPOSITORY https://github.com/kcenon/logger_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(LoggerSystem)

# Link to your target
target_link_libraries(your_target PRIVATE LoggerSystem::logger)
```

### Building from Source

```bash
# Clone the repository
git clone https://github.com/kcenon/logger_system.git
cd logger_system

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Install (optional)
sudo cmake --install .
```

### Using as Installed Package

If you've installed the logger system:

```cmake
find_package(LoggerSystem REQUIRED)
target_link_libraries(your_target PRIVATE LoggerSystem::logger)
```

## Basic Usage

### Simple Console Logging

```cpp
#include <logger_system/logger.h>
#include <logger_system/writers/console_writer.h>

int main() {
    // Create logger instance
    auto logger = std::make_shared<logger_module::logger>();
    
    // Add console output
    logger->add_writer(std::make_unique<logger_module::console_writer>());
    
    // Start logger (for async mode)
    logger->start();
    
    // Log messages
    logger->log(thread_module::log_level::info, "Application started");
    logger->log(thread_module::log_level::warning, "This is a warning");
    logger->log(thread_module::log_level::error, "An error occurred!");
    
    // Stop logger
    logger->stop();
    
    return 0;
}
```

### Logging with Source Location

```cpp
// Log with file, line, and function information
logger->log(thread_module::log_level::debug, 
            "Debug information", 
            __FILE__, __LINE__, __func__);

// Convenience macro (if you define one)
#define LOG_DEBUG(logger, msg) \
    logger->log(thread_module::log_level::debug, msg, __FILE__, __LINE__, __func__)

LOG_DEBUG(logger, "This includes source location");
```

### Log Levels

The logger supports six log levels:

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

### Filtering by Level

```cpp
// Set minimum log level
logger->set_min_level(thread_module::log_level::info);

// These won't be logged
logger->log(thread_module::log_level::trace, "Too detailed");
logger->log(thread_module::log_level::debug, "Debug info");

// These will be logged
logger->log(thread_module::log_level::info, "Important info");
logger->log(thread_module::log_level::error, "Error!");
```

## Configuration

### Synchronous vs Asynchronous Logging

```cpp
// Asynchronous logger (default)
auto async_logger = std::make_shared<logger_module::logger>(true, 8192);

// Synchronous logger
auto sync_logger = std::make_shared<logger_module::logger>(false);
```

### Multiple Writers

```cpp
// Console output with color
auto console = std::make_unique<logger_module::console_writer>();
logger->add_writer(std::move(console));

// File output (when implemented)
// auto file = std::make_unique<logger_module::file_writer>("app.log");
// logger->add_writer(std::move(file));

// Custom writer
class my_writer : public logger_module::base_writer {
    void write(thread_module::log_level level,
               const std::string& message,
               const std::string& file,
               int line,
               const std::string& function,
               const std::chrono::system_clock::time_point& timestamp) override {
        // Custom implementation
    }
    
    void flush() override {
        // Flush implementation
    }
};

logger->add_writer(std::make_unique<my_writer>());
```

### Console Writer Options

```cpp
// Output to stderr instead of stdout
auto console = std::make_unique<logger_module::console_writer>(true);

// Disable color output
auto console = std::make_unique<logger_module::console_writer>(false, false);
console->set_use_color(false);
```

## Integration with Thread System

### Using Service Container

```cpp
#include <logger_system/logger.h>
#include <logger_system/writers/console_writer.h>
#include <thread_system/interfaces/service_container.h>
#include <thread_system/interfaces/thread_context.h>
#include <thread_system/thread_pool/core/thread_pool.h>

int main() {
    // Create and configure logger
    auto logger = std::make_shared<logger_module::logger>();
    logger->add_writer(std::make_unique<logger_module::console_writer>());
    logger->start();
    
    // Register in service container
    thread_module::service_container::global()
        .register_singleton<thread_module::logger_interface>(logger);
    
    // Create thread pool - it will use the logger automatically
    thread_module::thread_context context;
    auto pool = std::make_shared<thread_pool_module::thread_pool>("MyPool", context);
    
    // The pool will now log its operations
    pool->start();
    
    // Clean up
    pool->stop();
    logger->stop();
    
    return 0;
}
```

### Direct Integration

```cpp
// Create logger
auto logger = std::make_shared<logger_module::logger>();
logger->add_writer(std::make_unique<logger_module::console_writer>());
logger->start();

// Use directly as logger_interface
std::shared_ptr<thread_module::logger_interface> logger_interface = logger;

// Pass to components that need logging
auto context = thread_module::thread_context(logger_interface);
```

## Next Steps

- Read the [Architecture Overview](architecture.md) to understand the system design
- Check the [API Reference](api-reference.md) for detailed documentation
- See [Performance Guide](performance.md) for optimization tips
- Learn about [Custom Writers](custom-writers.md) for specialized output