# Logger System Migration Guide

## Table of Contents
1. [Overview](#overview)
2. [Version Migration](#version-migration)
3. [API Changes](#api-changes)
4. [Configuration Migration](#configuration-migration)
5. [Migration from Other Libraries](#migration-from-other-libraries)
6. [Compatibility Wrappers](#compatibility-wrappers)
7. [Step-by-Step Migration](#step-by-step-migration)
8. [Common Issues and Solutions](#common-issues-and-solutions)

## Overview

This guide helps you migrate to Logger System from:
- Earlier versions of Logger System (1.x to 2.x)
- Other popular logging libraries (spdlog, Boost.Log, glog, log4cpp)
- Custom logging solutions

### Breaking Changes Summary (v2.0+)

| Component | v1.x | v2.x | Impact |
|-----------|------|------|--------|
| Error Handling | Exceptions | Result types | High |
| Memory Management | Raw pointers | Smart pointers | High |
| Configuration | Direct setters | Builder pattern | Medium |
| Async Operations | Manual threads | Built-in async | Low |
| DI Container | Required | Optional | Low |

## Version Migration

### From v1.x to v2.x

#### 1. Error Handling Migration

**Old (v1.x):**
```cpp
try {
    logger->add_writer(new file_writer("app.log"));
} catch (const std::exception& e) {
    std::cerr << "Failed: " << e.what() << std::endl;
}
```

**New (v2.x):**
```cpp
auto result = logger->add_writer(std::make_unique<file_writer>("app.log"));
if (!result) {
    std::cerr << "Failed: " << result.error().message() << std::endl;
}
```

#### 2. Memory Management Migration

**Old (v1.x):**
```cpp
// Manual memory management
base_writer* writer = new file_writer("app.log");
logger->add_writer(writer);  // Logger takes ownership
// Don't delete writer - logger owns it
```

**New (v2.x):**
```cpp
// RAII with unique_ptr
auto writer = std::make_unique<file_writer>("app.log");
logger->add_writer(std::move(writer));  // Explicit ownership transfer
```

#### 3. Configuration Migration

**Old (v1.x):**
```cpp
logger* log = new logger();
log->set_min_level(log_level::info);
log->set_async(true);
log->set_buffer_size(10000);
log->add_console_writer();
log->add_file_writer("app.log");
```

**New (v2.x):**
```cpp
auto log = logger_builder()
    .with_min_level(log_level::info)
    .with_async_mode(true)
    .with_buffer_size(10000)
    .with_console_writer()
    .with_file_writer("app.log")
    .build();
```

#### 4. Logging API Migration

**Old (v1.x):**
```cpp
LOG_INFO(logger, "Message with %s", param);
LOG_DEBUG_F(logger, "Formatted: {}", value);
```

**New (v2.x):**
```cpp
logger->info("Message with param", {{"param", param}});
logger->debug("Message", {{"value", value}});
```

## API Changes

### Core Logger API

| v1.x Method | v2.x Equivalent | Notes |
|-------------|-----------------|-------|
| `log(level, msg)` | `log(level, msg, fields)` | Added structured fields |
| `log_format(level, fmt, ...)` | `log(level, msg, fields)` | Use fields instead of format |
| `set_min_level(level)` | Builder: `with_min_level(level)` | Configure via builder |
| `add_writer(writer*)` | `add_writer(unique_ptr<writer>)` | Smart pointer required |
| `remove_writer(name)` | `remove_writer(name)` | Unchanged |
| `flush()` | `flush()` | Now returns result<void> |
| `shutdown()` | Destructor | Automatic cleanup |

### Writer API

| v1.x Method | v2.x Equivalent | Notes |
|-------------|-----------------|-------|
| `write(string)` | `write(log_entry)` | Structured entry |
| `set_format(string)` | `set_formatter(unique_ptr)` | Formatter object |
| `enable_async()` | Use `async_writer` wrapper | Composition pattern |
| `set_buffer_size(n)` | Constructor parameter | Immutable after creation |

### Configuration API

| v1.x Config | v2.x Builder Method | Notes |
|-------------|---------------------|-------|
| `config.async` | `with_async_mode(bool)` | |
| `config.buffer_size` | `with_buffer_size(size_t)` | |
| `config.min_level` | `with_min_level(log_level)` | |
| `config.pattern` | `with_pattern(string)` | |
| `config.colored` | `with_colored_output(bool)` | |
| `config.rotation_size` | `with_rotation(size_t)` | |
| `config.max_files` | `with_max_files(size_t)` | |

## Configuration Migration

### From INI/XML Configuration

**Old (config.ini):**
```ini
[logger]
level = INFO
async = true
buffer_size = 10000

[console]
enabled = true
colored = true

[file]
enabled = true
path = app.log
rotation = 10485760
max_files = 5
```

**New (code-based):**
```cpp
auto create_logger_from_config() {
    return logger_builder()
        .with_min_level(log_level::info)
        .with_async_mode(true)
        .with_buffer_size(10000)
        .with_console_writer()
        .with_colored_output(true)
        .with_file_writer("app.log")
        .with_rotation(10 * 1024 * 1024)
        .with_max_files(5)
        .build();
}
```

### Environment-based Configuration

```cpp
auto create_logger() {
    const auto env = std::getenv("LOG_ENV");
    const auto level = std::getenv("LOG_LEVEL");
    
    auto builder = logger_builder();
    
    // Apply environment-specific settings
    if (env) {
        if (std::string(env) == "production") {
            builder.with_configuration_template(configuration_template::production);
        } else if (std::string(env) == "development") {
            builder.with_configuration_template(configuration_template::debug);
        }
    }
    
    // Override with specific level if provided
    if (level) {
        builder.with_min_level(parse_level(level));
    }
    
    return builder.build();
}
```

## Migration from Other Libraries

### From spdlog

```cpp
// spdlog
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

auto logger = spdlog::basic_logger_mt("my_logger", "logs/my_log.txt");
logger->set_level(spdlog::level::info);
logger->info("Hello {}", "World");
logger->error("Error code: {}", 404);

// Logger System equivalent
#include "logger_system/logger.h"

auto logger = logger_builder()
    .with_name("my_logger")
    .with_file_writer("logs/my_log.txt")
    .with_min_level(log_level::info)
    .build();
    
logger->info("Hello World");
logger->error("Error code", {{"code", 404}});
```

#### spdlog Pattern Migration

| spdlog Pattern | Logger System Equivalent |
|----------------|-------------------------|
| `%Y-%m-%d %H:%M:%S` | `%time%` with custom format |
| `%l` | `%level%` |
| `%n` | Logger name in context |
| `%v` | `%message%` |
| `%t` | `%thread%` |
| `%P` | `%pid%` |
| `%@` | `%source%` |

### From Boost.Log

```cpp
// Boost.Log
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;

logging::add_file_log("sample.log");
BOOST_LOG_TRIVIAL(info) << "An informational message";
BOOST_LOG_TRIVIAL(error) << "An error message";

// Logger System equivalent
#include "logger_system/logger.h"

auto logger = logger_builder()
    .with_file_writer("sample.log")
    .build();
    
logger->info("An informational message");
logger->error("An error message");
```

### From Google glog

```cpp
// glog
#include <glog/logging.h>

google::InitGoogleLogging(argv[0]);
LOG(INFO) << "Found " << num_cookies << " cookies";
LOG(ERROR) << "Error code: " << error_code;
CHECK(condition) << "Condition failed";

// Logger System equivalent
#include "logger_system/logger.h"

auto logger = create_logger();
logger->info("Found cookies", {{"count", num_cookies}});
logger->error("Error occurred", {{"code", error_code}});
if (!condition) {
    logger->critical("Condition failed");
    std::abort();
}
```

### From log4cpp

```cpp
// log4cpp
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>

log4cpp::Category& root = log4cpp::Category::getRoot();
root.addAppender(new log4cpp::FileAppender("file", "app.log"));
root.setPriority(log4cpp::Priority::INFO);
root.info("Information message");
root.error("Error message");

// Logger System equivalent
#include "logger_system/logger.h"

auto logger = logger_builder()
    .with_file_writer("app.log")
    .with_min_level(log_level::info)
    .build();
    
logger->info("Information message");
logger->error("Error message");
```

## Compatibility Wrappers

### Legacy API Wrapper

Create a compatibility header for gradual migration:

```cpp
// compatibility/logger_v1_compat.h
#pragma once
#include "logger_system/logger.h"

// Legacy macros for backward compatibility
#define LOG_INFO(logger, msg, ...) \
    logger->info(format_string(msg, ##__VA_ARGS__))

#define LOG_ERROR(logger, msg, ...) \
    logger->error(format_string(msg, ##__VA_ARGS__))

#define LOG_DEBUG(logger, msg, ...) \
    logger->debug(format_string(msg, ##__VA_ARGS__))

// Legacy function signatures
namespace logger_v1_compat {
    
    // Old-style logger creation
    [[deprecated("Use logger_builder instead")]]
    inline logger* create_logger() {
        static auto modern_logger = logger_builder().build();
        return modern_logger.get();
    }
    
    // Old-style writer addition
    [[deprecated("Use add_writer with unique_ptr")]]
    inline void add_writer(logger* log, base_writer* writer) {
        if (log && writer) {
            auto result = log->add_writer(
                std::unique_ptr<base_writer>(writer)
            );
            if (!result) {
                throw std::runtime_error(result.error().message());
            }
        }
    }
    
    // Old-style configuration
    [[deprecated("Use logger_builder for configuration")]]
    inline void configure_logger(logger* log, 
                                 log_level level,
                                 bool async,
                                 size_t buffer_size) {
        // Note: This requires recreating the logger
        // Better to use builder pattern from the start
        std::cerr << "Warning: configure_logger is deprecated. "
                  << "Use logger_builder instead.\n";
    }
}
```

### Wrapper for Other Libraries

```cpp
// compatibility/spdlog_compat.h
#pragma once
#include "logger_system/logger.h"

namespace spdlog_compat {
    
    template<typename... Args>
    class logger_wrapper {
        std::shared_ptr<logger_module::logger> impl_;
        
    public:
        explicit logger_wrapper(std::shared_ptr<logger_module::logger> impl)
            : impl_(std::move(impl)) {}
        
        template<typename... T>
        void info(const std::string& fmt, T&&... args) {
            impl_->info(format(fmt, std::forward<T>(args)...));
        }
        
        template<typename... T>
        void error(const std::string& fmt, T&&... args) {
            impl_->error(format(fmt, std::forward<T>(args)...));
        }
        
        void set_level(int level) {
            // Map spdlog levels to logger_system levels
            static const std::map<int, log_level> level_map = {
                {0, log_level::trace},
                {1, log_level::debug},
                {2, log_level::info},
                {3, log_level::warning},
                {4, log_level::error},
                {5, log_level::critical}
            };
            
            if (auto it = level_map.find(level); it != level_map.end()) {
                // Note: Level changes require logger recreation in v2
                std::cerr << "Warning: Dynamic level changes not supported. "
                          << "Recreate logger with desired level.\n";
            }
        }
    };
    
    // Factory function mimicking spdlog API
    inline auto basic_logger_mt(const std::string& name, 
                                const std::string& filename) {
        auto impl = logger_builder()
            .with_name(name)
            .with_file_writer(filename)
            .build();
        return std::make_shared<logger_wrapper>(std::move(impl));
    }
}
```

## Step-by-Step Migration

### Phase 1: Preparation (1-2 days)

1. **Audit Current Usage**
```bash
# Find all logging calls
grep -r "LOG_\|->log\|logger->" --include="*.cpp" --include="*.h" src/

# Count usage by type
grep -r "LOG_INFO\|LOG_ERROR\|LOG_DEBUG" --include="*.cpp" src/ | wc -l
```

2. **Create Migration Branch**
```bash
git checkout -b migration/logger-v2
```

3. **Add Compatibility Headers**
```cpp
// Add to your project
#include "compatibility/logger_v1_compat.h"
using namespace logger_v1_compat;  // Temporary during migration
```

### Phase 2: Core Migration (2-3 days)

1. **Update Logger Creation**
```cpp
// Find and replace patterns
// OLD: logger* log = new logger();
// NEW: auto log = logger_builder().build();

// OLD: logger* log = create_logger();
// NEW: auto log = logger_builder()
//          .with_configuration_template(configuration_template::production)
//          .build();
```

2. **Update Writer Management**
```cpp
// OLD: log->add_writer(new file_writer("app.log"));
// NEW: log->add_writer(std::make_unique<file_writer>("app.log"));
```

3. **Update Logging Calls**
```cpp
// Use automated script for bulk conversion
// OLD: LOG_INFO(log, "User %s logged in", username);
// NEW: log->info("User logged in", {{"user", username}});
```

### Phase 3: Feature Migration (2-3 days)

1. **Migrate Configuration**
```cpp
// Replace configuration files with code
auto create_configured_logger() {
    return logger_builder()
        .with_configuration_template(get_template_for_environment())
        .with_overrides_from_env()
        .build();
}
```

2. **Update Tests**
```cpp
// Update test fixtures
class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // OLD: logger_ = new logger();
        // NEW:
        logger_ = logger_builder()
            .with_min_level(log_level::trace)
            .build();
    }
    
    std::shared_ptr<logger> logger_;
};
```

### Phase 4: Optimization (1-2 days)

1. **Enable New Features**
```cpp
// Take advantage of v2 features
auto logger = logger_builder()
    .with_async_mode(true)
    .with_batch_writing(true)
    .with_structured_logging(true)
    .build();
```

2. **Remove Compatibility Layer**
```cpp
// Remove compatibility headers
// Remove using namespace logger_v1_compat;
// Fix any remaining legacy calls
```

### Phase 5: Validation (1 day)

1. **Run Tests**
```bash
# Run all tests
make test

# Run with sanitizers
make test SANITIZE=address
make test SANITIZE=thread
```

2. **Performance Testing**
```cpp
// Benchmark old vs new
void benchmark_migration() {
    auto start = now();
    for (int i = 0; i < 1000000; ++i) {
        logger->info("Test message", {{"id", i}});
    }
    auto duration = now() - start;
    std::cout << "New version: " << duration << "ms\n";
}
```

## Common Issues and Solutions

### Issue 1: Memory Leaks with Raw Pointers

**Problem:**
```cpp
// Leak: writer not deleted if add_writer throws
base_writer* writer = new file_writer("app.log");
logger->add_writer(writer);  // May throw in v1
```

**Solution:**
```cpp
// RAII ensures cleanup
auto writer = std::make_unique<file_writer>("app.log");
auto result = logger->add_writer(std::move(writer));
if (!result) {
    // Handle error - writer automatically cleaned up
}
```

### Issue 2: Exception Safety

**Problem:**
```cpp
try {
    logger->log(level, message);  // May throw in v1
} catch (...) {
    // Lost log message
}
```

**Solution:**
```cpp
auto result = logger->log(level, message);
if (!result) {
    // Fallback to stderr
    std::cerr << "[FALLBACK] " << message << std::endl;
}
```

### Issue 3: Thread Safety

**Problem:**
```cpp
// v1: Manual synchronization needed
std::mutex log_mutex;
{
    std::lock_guard<std::mutex> lock(log_mutex);
    logger->log(level, message);
}
```

**Solution:**
```cpp
// v2: Thread-safe by default
logger->log(level, message);  // No external locking needed
```

### Issue 4: Configuration Changes

**Problem:**
```cpp
// v1: Runtime configuration changes
logger->set_min_level(log_level::debug);  // Dynamic change
```

**Solution:**
```cpp
// v2: Immutable configuration - recreate logger
logger = logger_builder()
    .copy_from(logger)  // Preserve existing config
    .with_min_level(log_level::debug)  // Override level
    .build();
```

### Issue 5: Custom Formatters

**Problem:**
```cpp
// v1: String-based format
logger->set_format("[%time%] [%level%] %message%");
```

**Solution:**
```cpp
// v2: Formatter objects
class custom_formatter : public base_formatter {
    std::string format(const log_entry& entry) override {
        return format_timestamp(entry.timestamp) + " [" + 
               to_string(entry.level) + "] " + entry.message;
    }
};

logger->set_formatter(std::make_unique<custom_formatter>());
```

## Migration Checklist

- [ ] Backup current codebase
- [ ] Create migration branch
- [ ] Add compatibility headers
- [ ] Update build system (CMake/Make)
- [ ] Migrate logger creation code
- [ ] Update writer management
- [ ] Convert logging calls
- [ ] Update configuration
- [ ] Migrate tests
- [ ] Remove deprecated code
- [ ] Run full test suite
- [ ] Performance benchmarks
- [ ] Update documentation
- [ ] Code review
- [ ] Merge to main branch

## Support and Resources

- [API Documentation](API_DOCUMENTATION.md)
- [Best Practices Guide](BEST_PRACTICES.md)
- [Example Code](../samples/)
- [Issue Tracker](https://github.com/kcenon/logger_system/issues)

For migration assistance, please file an issue with the `migration` label.