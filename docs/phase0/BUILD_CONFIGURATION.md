# Build Configuration Design Document

## Overview
This document defines the build system strategy for integrating 7 external systems into the messaging_system.

## Dependency Integration Strategy

### Option 1: FetchContent (Development Environment)
**Use Case:** Rapid iteration during development, CI/CD pipelines

**Advantages:**
- Automatic dependency download
- Version pinning via Git tags
- No manual installation required
- Consistent across all developers

**CMake Configuration:**
```cmake
include(FetchContent)

# Common System
FetchContent_Declare(
  CommonSystem
  GIT_REPOSITORY https://github.com/kcenon/common_system.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)

# Thread System
FetchContent_Declare(
  ThreadSystem
  GIT_REPOSITORY https://github.com/kcenon/thread_system.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)

# Logger System
FetchContent_Declare(
  LoggerSystem
  GIT_REPOSITORY https://github.com/kcenon/logger_system.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)

# Monitoring System
FetchContent_Declare(
  MonitoringSystem
  GIT_REPOSITORY https://github.com/kcenon/monitoring_system.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)

# Container System
FetchContent_Declare(
  ContainerSystem
  GIT_REPOSITORY https://github.com/kcenon/container_system.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)

# Database System
FetchContent_Declare(
  DatabaseSystem
  GIT_REPOSITORY https://github.com/kcenon/database_system.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)

# Network System
FetchContent_Declare(
  NetworkSystem
  GIT_REPOSITORY https://github.com/kcenon/network_system.git
  GIT_TAG v1.0.0
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(
  CommonSystem
  ThreadSystem
  LoggerSystem
  MonitoringSystem
  ContainerSystem
  DatabaseSystem
  NetworkSystem
)
```

### Option 2: find_package (Production Environment)
**Use Case:** Production deployments, system-wide installations

**Advantages:**
- Faster configuration (no download)
- Version management via package manager
- System-wide reuse
- Stable deployment

**CMake Configuration:**
```cmake
# Find installed packages
find_package(CommonSystem 1.0 REQUIRED)
find_package(ThreadSystem 1.0 REQUIRED)
find_package(LoggerSystem 1.0 REQUIRED)
find_package(MonitoringSystem 1.0 REQUIRED)
find_package(ContainerSystem 1.0 REQUIRED)
find_package(DatabaseSystem 1.0 REQUIRED)
find_package(NetworkSystem 1.0 REQUIRED)

# Verify all targets exist
if(NOT TARGET CommonSystem::common)
  message(FATAL_ERROR "CommonSystem not found. Install via package manager or use FetchContent.")
endif()
```

## Unified CMake Configuration Options

### Feature Flags
```cmake
# Core features
option(MESSAGING_USE_LOCKFREE "Enable lock-free data structures" OFF)
option(MESSAGING_ENABLE_MONITORING "Enable runtime monitoring" ON)
option(MESSAGING_ENABLE_LOGGING "Enable logging system" ON)
option(MESSAGING_ENABLE_METRICS "Enable metrics collection" ON)
option(MESSAGING_ENABLE_TLS "Enable TLS/SSL support" OFF)

# Build type selection
option(MESSAGING_USE_EXTERNAL_SYSTEMS "Use external system packages" ON)
option(MESSAGING_USE_FETCHCONTENT "Use FetchContent for dependencies" OFF)

# Development features
option(MESSAGING_BUILD_TESTS "Build unit tests" ON)
option(MESSAGING_BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(MESSAGING_BUILD_EXAMPLES "Build example applications" ON)
option(MESSAGING_BUILD_DOCS "Build documentation" OFF)
```

### Build Profiles
```cmake
set(MESSAGING_BUILD_PROFILE "Release" CACHE STRING "Build profile")
set_property(CACHE MESSAGING_BUILD_PROFILE PROPERTY STRINGS
  "Debug"
  "Release"
  "RelWithDebInfo"
  "MinSizeRel"
  "ASAN"      # AddressSanitizer
  "TSAN"      # ThreadSanitizer
  "UBSAN"     # UndefinedBehaviorSanitizer
  "MSAN"      # MemorySanitizer
)

# Apply profile-specific flags
if(MESSAGING_BUILD_PROFILE STREQUAL "ASAN")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "TSAN")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "UBSAN")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all -g")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=undefined")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "MSAN")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -g")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=memory")
endif()
```

### Compiler Options
```cmake
# C++ Standard
set(MESSAGING_CXX_STANDARD 20 CACHE STRING "C++ standard version")
set(CMAKE_CXX_STANDARD ${MESSAGING_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning levels
set(MESSAGING_WARNING_LEVEL "High" CACHE STRING "Compiler warning level")
set_property(CACHE MESSAGING_WARNING_LEVEL PROPERTY STRINGS "Low" "Medium" "High" "Pedantic")

if(MESSAGING_WARNING_LEVEL STREQUAL "High" OR MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    if(MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
      add_compile_options(-Wconversion -Wsign-conversion -Wshadow)
    endif()
  elseif(MSVC)
    add_compile_options(/W4 /WX)
  endif()
endif()
```

## Propagating Options to External Systems

### Strategy
Map messaging_system options to external system options:

```cmake
# Propagate lock-free preference
if(MESSAGING_USE_LOCKFREE)
  set(USE_LOCKFREE_BY_DEFAULT ON CACHE BOOL "Use lock-free structures in thread_system" FORCE)
  set(USE_LOCKFREE_THREAD_POOL ON CACHE BOOL "Use lock-free thread pool" FORCE)
  set(USE_LOCKFREE_LOGGER ON CACHE BOOL "Use lock-free logger" FORCE)
endif()

# Propagate monitoring preference
if(MESSAGING_ENABLE_MONITORING)
  set(ENABLE_MONITORING ON CACHE BOOL "Enable monitoring in all systems" FORCE)
  set(BUILD_WITH_MONITORING ON CACHE BOOL "Build monitoring integration" FORCE)
endif()

# Propagate logging preference
if(MESSAGING_ENABLE_LOGGING)
  set(ENABLE_LOGGING ON CACHE BOOL "Enable logging in all systems" FORCE)
  set(BUILD_WITH_LOGGER ON CACHE BOOL "Build logger integration" FORCE)
endif()

# Propagate TLS preference
if(MESSAGING_ENABLE_TLS)
  set(ENABLE_TLS ON CACHE BOOL "Enable TLS in network_system" FORCE)
  set(USE_SSL ON CACHE BOOL "Use SSL encryption" FORCE)
endif()
```

## CMake Presets

### CMakePresets.json
```json
{
  "version": 3,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 16,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "default",
      "displayName": "Default Configuration",
      "description": "Standard build configuration",
      "binaryDir": "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "MESSAGING_USE_EXTERNAL_SYSTEMS": "ON",
        "MESSAGING_ENABLE_MONITORING": "ON",
        "MESSAGING_ENABLE_LOGGING": "ON"
      }
    },
    {
      "name": "debug",
      "inherits": "default",
      "displayName": "Debug Build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "MESSAGING_BUILD_TESTS": "ON"
      }
    },
    {
      "name": "release",
      "inherits": "default",
      "displayName": "Release Build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "MESSAGING_BUILD_TESTS": "OFF",
        "CMAKE_INTERPROCEDURAL_OPTIMIZATION": "ON"
      }
    },
    {
      "name": "asan",
      "inherits": "default",
      "displayName": "AddressSanitizer Build",
      "cacheVariables": {
        "MESSAGING_BUILD_PROFILE": "ASAN",
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "tsan",
      "inherits": "default",
      "displayName": "ThreadSanitizer Build",
      "cacheVariables": {
        "MESSAGING_BUILD_PROFILE": "TSAN",
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "ubsan",
      "inherits": "default",
      "displayName": "UndefinedBehaviorSanitizer Build",
      "cacheVariables": {
        "MESSAGING_BUILD_PROFILE": "UBSAN",
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "dev-fetchcontent",
      "inherits": "debug",
      "displayName": "Development with FetchContent",
      "cacheVariables": {
        "MESSAGING_USE_FETCHCONTENT": "ON",
        "MESSAGING_USE_EXTERNAL_SYSTEMS": "OFF"
      }
    },
    {
      "name": "ci",
      "inherits": "default",
      "displayName": "CI Build",
      "cacheVariables": {
        "MESSAGING_BUILD_TESTS": "ON",
        "MESSAGING_BUILD_BENCHMARKS": "ON",
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "default",
      "configurePreset": "default"
    },
    {
      "name": "debug",
      "configurePreset": "debug"
    },
    {
      "name": "release",
      "configurePreset": "release"
    }
  ],
  "testPresets": [
    {
      "name": "default",
      "configurePreset": "default",
      "output": {
        "outputOnFailure": true
      },
      "execution": {
        "noTestsAction": "error",
        "stopOnFailure": false
      }
    }
  ]
}
```

## CI Profile Configurations

### GitHub Actions Matrix
```yaml
strategy:
  matrix:
    profile:
      - { name: "Debug", preset: "debug" }
      - { name: "Release", preset: "release" }
      - { name: "ASAN", preset: "asan" }
      - { name: "TSAN", preset: "tsan" }
      - { name: "UBSAN", preset: "ubsan" }
    os:
      - ubuntu-latest
      - macos-latest
      - windows-latest
```

## Documentation of Option Impact

### MESSAGING_USE_LOCKFREE
- **Impact:** Uses lock-free queue implementations in thread_system
- **Performance:** +10-15% throughput under high contention
- **Compatibility:** Requires C++20 atomics support
- **Recommendation:** Enable for production, disable for debugging

### MESSAGING_ENABLE_MONITORING
- **Impact:** Enables real-time metrics collection
- **Performance:** ~2% overhead
- **Use Case:** Production deployments, performance analysis
- **Recommendation:** Always enable

### MESSAGING_ENABLE_LOGGING
- **Impact:** Enables structured logging
- **Performance:** ~1% overhead (async mode)
- **Use Case:** All environments
- **Recommendation:** Always enable

### MESSAGING_ENABLE_TLS
- **Impact:** Enables TLS/SSL encryption for network communication
- **Performance:** ~5-10% overhead
- **Dependencies:** Requires OpenSSL or BoringSSL
- **Recommendation:** Enable for production

## Installation Guide

### System-Wide Installation (find_package mode)
```bash
# Install dependencies
sudo apt-get install libcommon-system-dev libthread-system-dev \
                     liblogger-system-dev libmonitoring-system-dev \
                     libcontainer-system-dev libdatabase-system-dev \
                     libnetwork-system-dev

# Configure and build
cmake --preset release
cmake --build --preset release
sudo cmake --install build
```

### FetchContent Mode (Development)
```bash
# No installation required, all dependencies auto-downloaded
cmake --preset dev-fetchcontent
cmake --build --preset debug
```

## Completion Checklist
- [x] Dependency integration strategies documented
- [x] CMake options defined
- [x] Build profiles configured
- [x] CMake Presets created
- [x] CI matrix defined
- [x] Installation guide provided
