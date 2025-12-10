# ADR-001: Logging Architecture - ILogger Interface vs logger_system

## Status

**Accepted** - Current implementation is correct

## Context

The messaging_system project uses logging throughout its codebase. This ADR documents the architectural decision regarding how logging is integrated and the relationship between `common_system::ILogger` and `logger_system`.

### Background

The project initially had a direct dependency on `logger_system` for logging functionality. This created tight coupling and potential circular dependency issues, particularly between `thread_system` and `logger_system`.

### Problem Statement

1. Direct dependency on `logger_system` created tight coupling
2. Circular dependencies were possible between core systems
3. Testing required full logger_system initialization
4. Applications without logging needs still pulled in the entire logger implementation

## Decision

Use `common_system::ILogger` interface with runtime binding via `GlobalLoggerRegistry` instead of direct `logger_system` dependency.

### Architecture Overview

```
messaging_system
    |
    v
common::logging::log_info/error/debug (convenience functions)
    |
    v
GlobalLoggerRegistry::instance() (thread-safe singleton)
    |
    v
common::interfaces::ILogger (abstract interface)
    |
    v (optional runtime binding)
logger_system::logger (concrete implementation)
    |
    v
logger_adapter (ILogger adapter)
```

### Implementation Details

#### 1. ILogger Interface (`common_system`)

Location: `common_system/include/kcenon/common/interfaces/logger_interface.h`

```cpp
class ILogger {
public:
    virtual void log(log_level level, std::string_view message,
                     const source_location& loc = source_location::current()) = 0;
    virtual void log(const log_entry& entry) = 0;
    virtual bool is_enabled(log_level level) const = 0;
    virtual void set_level(log_level level) = 0;
    virtual log_level get_level() const = 0;
    virtual void flush() = 0;
};
```

#### 2. GlobalLoggerRegistry (`common_system`)

Location: `common_system/include/kcenon/common/interfaces/global_logger_registry.h`

Features:
- Thread-safe singleton (Meyer's singleton pattern)
- Named logger management via `register_logger()`, `get_logger()`
- Default logger management via `set_default_logger()`, `get_default_logger()`
- Factory-based lazy initialization with `register_factory()`
- NullLogger fallback for safe operation without logger

#### 3. Convenience Functions (`common_system`)

Location: `common_system/include/kcenon/common/logging/log_functions.h`

```cpp
namespace common::logging {
    void log_info(const std::string& message);
    void log_error(const std::string& message);
    void log_debug(const std::string& message);
    void log_warning(const std::string& message);
    void log_trace(const std::string& message);
    void log_critical(const std::string& message);
}
```

#### 4. CMake Dependency Configuration

Location: `messaging_system/CMakeLists.txt` (lines 126-128)

```cmake
# NOTE: logger_system is no longer a direct dependency (Issue #94).
# Logging is now provided through common_system's ILogger interface
# with runtime binding via GlobalLoggerRegistry.
```

## Consequences

### Positive

1. **Zero Coupling**: messaging_system has no compile-time dependency on logger_system
2. **Optional Integration**: Works with or without logger_system installed
3. **Safe Defaults**: NullLogger prevents null pointer crashes when no logger is configured
4. **Flexible Binding**: Different ILogger implementations can be bound at runtime
5. **Circular Dependency Resolution**: Breaks the thread_system <-> logger_system cycle
6. **Testability**: Unit tests can run without full logger initialization
7. **Reduced Binary Size**: Applications not using logging don't include logger_system

### Negative

1. **Runtime Overhead**: Virtual function calls instead of direct calls (minimal impact)
2. **Configuration Complexity**: Requires explicit logger registration at application startup
3. **Feature Limitation**: Some logger_system-specific features require direct dependency

### Neutral

1. **Documentation Requirement**: Architecture needs clear documentation (this ADR)
2. **Migration Effort**: Existing code using logger_system directly needs refactoring

## Alternatives Considered

### Alternative A: Direct logger_system Dependency

**Rejected** because:
- Creates tight coupling
- Prevents optional logging
- Causes circular dependency issues

### Alternative B: Compile-time Logger Selection via Templates

**Rejected** because:
- Increases binary size (template instantiation)
- Reduces flexibility for runtime configuration
- More complex API

### Alternative C: Macro-based Logging

**Rejected** because:
- Less type-safe
- Harder to maintain
- Doesn't integrate well with C++20 features

## Compliance

### Current Implementation Status

| Requirement | Status | Notes |
|-------------|--------|-------|
| ILogger interface usage | Complete | All logging via convenience functions |
| No direct logger_system dependency | Complete | CMakeLists.txt verified |
| GlobalLoggerRegistry integration | Complete | Thread-safe singleton |
| NullLogger fallback | Complete | Safe default behavior |
| Source location support | Complete | C++20 source_location |

### Usage Examples in messaging_system

```cpp
// src/impl/core/message_bus.cpp
#include <kcenon/common/logging/log_functions.h>

void message_bus::start() {
    common::logging::log_info("Starting message bus with " +
                               std::to_string(worker_count) + " workers");
}

void message_bus::on_error(const std::string& error) {
    common::logging::log_error("Failed to process message: " + error);
}
```

## Related Decisions

- **Issue #94**: Migration from direct logger_system dependency
- **Issue #139**: Thread pool architecture decision (similar pattern)

## References

- `common_system/include/kcenon/common/interfaces/logger_interface.h`
- `common_system/include/kcenon/common/interfaces/global_logger_registry.h`
- `common_system/include/kcenon/common/logging/log_functions.h`
- `messaging_system/CMakeLists.txt`
- `messaging_system/include/kcenon/messaging/utils/integration_detector.h`

## Revision History

| Date | Version | Author | Changes |
|------|---------|--------|---------|
| 2025-12-10 | 1.0 | - | Initial ADR creation |
