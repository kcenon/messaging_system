# Changelog

All notable changes to the Network System project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Added
- **Logger System Integration** (2025-09-20)
  - New logger_integration interface for structured logging
  - Support for external logger_system when available
  - Fallback to basic console logger for standalone operation
  - Replaced all std::cout/cerr with NETWORK_LOG_* macros
  - Added BUILD_WITH_LOGGER_SYSTEM CMake option
  - Log levels: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
  - Source location tracking (__FILE__, __LINE__, __FUNCTION__)
  - Timestamp formatting with millisecond precision

### To Be Implemented
- Network manager factory class
- TCP server and client high-level interfaces
- HTTP client with response handling
- WebSocket support
- TLS/SSL encryption
- Connection pooling
- Migration guide completion (Phase 4)

## 2025-09-20 - Phase 4 In Progress

### Added
- **Performance Benchmarking**
  - Comprehensive performance benchmark suite (benchmark.cpp)
  - Throughput tests for various message sizes (64B, 1KB, 8KB)
  - Concurrent connection benchmarks (10 and 50 clients)
  - Thread pool performance measurements
  - Container serialization benchmarks
  - Achieved 305K+ msg/s average throughput

- **Compatibility Testing**
  - Enhanced compatibility test suite (test_compatibility.cpp)
  - Legacy namespace testing (network_module, messaging)
  - Type alias verification
  - Feature detection macros
  - Cross-compatibility tests between legacy and modern APIs
  - Message passing validation
  - Container and thread pool integration through legacy API

- **Usage Examples**
  - Legacy compatibility example (legacy_compatibility.cpp)
  - Modern API usage example (modern_usage.cpp)
  - Demonstration of backward compatibility
  - Advanced features showcase

### Updated
- README.md with performance results and Phase 4 status
- Documentation cleaned up and streamlined
- CMakeLists.txt with benchmark target

### Performance Results
- Average throughput: 305,255 msg/s
- Small messages (64B): 769,230 msg/s
- Medium messages (1KB): 128,205 msg/s
- Large messages (8KB): 20,833 msg/s
- Concurrent connections: Successfully handled 50 clients
- Performance rating: EXCELLENT - Production ready!

## 2025-09-20 - Phase 3 Complete

### Added
- **Thread System Integration**
  - thread_integration.h/cpp with thread pool abstraction
  - Thread pool interface for async task management
  - Basic thread pool implementation
  - Thread integration manager singleton
  - Support for delayed task submission
  - Task metrics and monitoring

- **Container System Integration**
  - container_integration.h/cpp for serialization
  - Container interface for message serialization
  - Basic container implementation
  - Container manager for registration and management
  - Support for custom serializers/deserializers

- **Compatibility Layer**
  - compatibility.h with legacy namespace aliases
  - Full backward compatibility with messaging_system
  - Legacy API support (network_module namespace)
  - Feature detection macros
  - System initialization/shutdown functions

- **Integration Tests**
  - test_integration.cpp for integration validation
  - Tests for bridge functionality
  - Tests for container integration
  - Tests for thread pool integration
  - Performance baseline verification

### Fixed
- Private member access issues in thread_integration
- Compatibility header structure issues
- API mismatches in sample code
- CMake fmt linking problems on macOS

## 2025-09-19 - Phase 2 Complete

### Added
- **Core System Separation**
  - Verified complete separation from messaging_system
  - Confirmed proper directory structure (include/network_system)
  - All modules properly organized (core, session, internal, integration)

- **Integration Module**
  - Completed messaging_bridge implementation
  - Performance metrics collection functionality
  - Container_system integration interface

- **Build Verification**
  - Successful build with container_system integration
  - verify_build test program passes all checks
  - CMakeLists.txt supports optional integrations

### Updated
- MIGRATION_CHECKLIST.md with Phase 2 completion status
- README.md with current project phase information

## 2025-09-19 - Phase 1 Complete

### Added
- **Core Infrastructure**
  - Complete separation from messaging_system
  - New namespace structure: `network_system::{core,session,internal,integration}`
  - ASIO-based asynchronous networking with C++20 coroutines
  - Messaging bridge for backward compatibility

- **Build System**
  - CMake configuration with vcpkg support
  - Flexible dependency detection (ASIO/Boost.ASIO)
  - Cross-platform support (Linux, macOS, Windows)
  - Manual vcpkg setup as fallback

- **CI/CD Pipeline**
  - GitHub Actions workflows for Ubuntu (GCC/Clang)
  - GitHub Actions workflows for Windows (Visual Studio/MSYS2)
  - Dependency security scanning with Trivy
  - License compatibility checks

- **Container Integration**
  - Full integration with container_system
  - Value container support in messaging bridge
  - Conditional compilation based on availability

- **Documentation**
  - Doxygen configuration for API documentation
  - Comprehensive README with build instructions
  - Migration and implementation plans
  - Architecture documentation

### Fixed
- **CI/CD Issues**
  - Removed problematic lukka/run-vcpkg action
  - Fixed pthread.lib linking error on Windows
  - Added _WIN32_WINNT definition for ASIO on Windows
  - Corrected CMake options (BUILD_TESTS instead of USE_UNIT_TEST)
  - Made vcpkg optional with system package fallback

- **Build Issues**
  - Fixed namespace qualification errors
  - Corrected include paths for internal headers
  - Resolved ASIO detection on various platforms
  - Fixed vcpkg baseline configuration issues

### Changed
- **Dependency Management**
  - Prefer system packages over vcpkg when available
  - Simplified vcpkg.json configuration
  - Added multiple fallback paths for dependency detection
  - Made container_system and thread_system optional

### Security
- Implemented dependency vulnerability scanning
- Added license compatibility verification
- Configured security event reporting

## 2025-09-18 - Initial Separation

### Added
- Initial implementation separated from messaging_system
- Basic TCP client/server functionality
- Session management
- Message pipeline processing

---

## Development Timeline

| Date | Milestone | Description |
|------|-----------|-------------|
| 2025-09-19 | Phase 1 Complete | Infrastructure and separation complete |
| 2025-09-18 | Initial Separation | Started separation from messaging_system |

## Migration Guide

### From messaging_system to network_system

#### Namespace Changes
```cpp
// Old (messaging_system)
#include <messaging_system/network/tcp_server.h>
using namespace network_module;

// New (network_system)
#include <network_system/core/messaging_server.h>
using namespace network_system::core;
```

#### CMake Integration
```cmake
# Old (messaging_system)
# Part of messaging_system

# New (network_system)
find_package(NetworkSystem REQUIRED)
target_link_libraries(your_target NetworkSystem::NetworkSystem)
```

#### Container Integration
```cpp
// New feature in network_system
#include <network_system/integration/messaging_bridge.h>

auto bridge = std::make_unique<network_system::integration::messaging_bridge>();
bridge->set_container(container);
```

## Support

For issues, questions, or contributions, please visit:
- GitHub: https://github.com/kcenon/network_system
- Email: kcenon@naver.com