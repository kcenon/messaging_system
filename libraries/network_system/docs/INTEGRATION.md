# Integration Guide

This guide explains how to integrate network_system with external systems and libraries.

## Table of Contents
- [Thread System Integration](#thread-system-integration)
- [Container System Integration](#container-system-integration)
- [Logger System Integration](#logger-system-integration)
- [Build Configuration](#build-configuration)

## Thread System Integration

The network_system can optionally integrate with an external thread pool for improved performance.

### Configuration
```cmake
cmake .. -DBUILD_WITH_THREAD_SYSTEM=ON
```

### Usage
When thread_system is available, network operations will automatically utilize the thread pool for:
- Connection handling
- Message processing
- Async operations

### Requirements
- thread_system must be installed in `../thread_system`
- Headers should be available at `../thread_system/include`

## Container System Integration

Enable advanced serialization and deserialization capabilities.

### Configuration
```cmake
cmake .. -DBUILD_WITH_CONTAINER_SYSTEM=ON
```

### Features
- Binary serialization
- JSON serialization
- Protocol buffer support
- Custom container types

### API Example
```cpp
#include <network_system/integration/container_integration.h>

// Serialize custom data
auto serialized = container_adapter::serialize(my_data);
client->send_packet(serialized);
```

## Logger System Integration

Provides structured logging with configurable log levels and output formats.

### Configuration
```cmake
cmake .. -DBUILD_WITH_LOGGER_SYSTEM=ON
```

### Features
- **Log Levels**: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- **Source Location Tracking**: Automatic file, line, and function recording
- **Timestamp Formatting**: Millisecond precision timestamps
- **Conditional Compilation**: Falls back to console output when logger_system is unavailable

### Usage

#### Basic Logging
```cpp
#include <network_system/integration/logger_integration.h>

// Use convenience macros
NETWORK_LOG_INFO("Server started on port " + std::to_string(port));
NETWORK_LOG_ERROR("Connection failed: " + error_message);
NETWORK_LOG_DEBUG("Received " + std::to_string(bytes) + " bytes");
```

#### Advanced Configuration
```cpp
// Get logger instance
auto& logger_mgr = network_system::integration::logger_integration_manager::instance();
auto logger = logger_mgr.get_logger();

// Check if log level is enabled
if (logger->is_enabled(network_system::integration::log_level::debug)) {
    // Perform expensive debug logging
    std::string detailed_state = generate_detailed_state();
    NETWORK_LOG_DEBUG(detailed_state);
}

// Direct logging without macros
logger->log(network_system::integration::log_level::warn,
           "Custom warning message",
           __FILE__, __LINE__, __FUNCTION__);
```

### Implementation Details

The logger integration provides two implementations:

1. **basic_logger**: Standalone console logger
   - Used when BUILD_WITH_LOGGER_SYSTEM is OFF
   - Outputs to std::cout (INFO and below) or std::cerr (WARN and above)
   - Simple timestamp formatting

2. **logger_system_adapter**: External logger integration
   - Used when BUILD_WITH_LOGGER_SYSTEM is ON
   - Wraps kcenon::logger for full-featured logging
   - Supports log file rotation, remote logging, etc.

### Requirements
- When BUILD_WITH_LOGGER_SYSTEM=ON:
  - logger_system must be installed in `../logger_system`
  - Headers should be available at `../logger_system/include`
- C++17 or later for string_view support

## Build Configuration

### Complete Build with All Integrations
```bash
cmake .. -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_WITH_THREAD_SYSTEM=ON \
    -DBUILD_WITH_CONTAINER_SYSTEM=ON \
    -DBUILD_WITH_LOGGER_SYSTEM=ON
```

### Minimal Build (No External Dependencies)
```bash
cmake .. -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_WITH_THREAD_SYSTEM=OFF \
    -DBUILD_WITH_CONTAINER_SYSTEM=OFF \
    -DBUILD_WITH_LOGGER_SYSTEM=OFF
```

### Checking Available Integrations

The system provides macros to check which integrations are available:

```cpp
#ifdef BUILD_WITH_THREAD_SYSTEM
    // Thread system is available
    use_thread_pool();
#endif

#ifdef BUILD_WITH_CONTAINER_SYSTEM
    // Container system is available
    use_advanced_serialization();
#endif

#ifdef BUILD_WITH_LOGGER_SYSTEM
    // Logger system is available
    use_structured_logging();
#else
    // Fallback to basic logging
    use_console_logging();
#endif
```

## Performance Considerations

### With Thread System
- **Benefit**: Up to 40% improvement in concurrent connection handling
- **Trade-off**: Slightly increased memory usage

### With Container System
- **Benefit**: Type-safe serialization with minimal overhead
- **Trade-off**: Additional dependency and build time

### With Logger System
- **Benefit**: Structured logging with filtering reduces I/O overhead
- **Trade-off**: Minimal performance impact (~1-2%)

## Troubleshooting

### Integration Not Found
If CMake cannot find an integration system:
1. Verify the system is installed in the expected location
2. Check that include paths are correct
3. Ensure the system is built with compatible compiler settings

### Link Errors
- Ensure all systems are built with the same C++ standard
- Check ABI compatibility between systems
- Verify all required libraries are linked

### Runtime Issues
- Check that all integrated systems are initialized properly
- Verify thread safety when using multiple integrations
- Review log output for initialization errors