# Phase 1: Build System and Dependency Refactoring - Design Document

## Overview
This document provides detailed design specifications for Phase 1: integrating 7 external systems and removing legacy code from the build system.

## Architecture Diagram

```
messaging_system (v2.0)
│
├── External Systems (via find_package or FetchContent)
│   ├── CommonSystem::common
│   ├── ThreadSystem::Core
│   ├── LoggerSystem::logger
│   ├── MonitoringSystem::monitoring
│   ├── ContainerSystem::container
│   ├── DatabaseSystem::database
│   └── NetworkSystem::network
│
├── Internal Components (messaging-specific)
│   ├── MessageBus
│   ├── TopicRouter
│   ├── MessageProcessor
│   └── ServiceContainer
│
└── Configuration Layer
    ├── CMake Build System
    ├── CMake Presets
    └── Validation Scripts
```

## Task 1.1: External Module Integration

### Updated CMakeLists.txt Structure

**File:** `CMakeLists.txt` (root)

```cmake
cmake_minimum_required(VERSION 3.16)

project(messaging_system
    VERSION 2.0.0
    DESCRIPTION "High-performance distributed messaging system"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Build Options
# ============================================================================

# Feature flags
option(MESSAGING_USE_LOCKFREE "Enable lock-free data structures" OFF)
option(MESSAGING_ENABLE_MONITORING "Enable runtime monitoring" ON)
option(MESSAGING_ENABLE_LOGGING "Enable logging system" ON)
option(MESSAGING_ENABLE_METRICS "Enable metrics collection" ON)
option(MESSAGING_ENABLE_TLS "Enable TLS/SSL support" OFF)

# Dependency strategy
option(MESSAGING_USE_EXTERNAL_SYSTEMS "Use external system packages" ON)
option(MESSAGING_USE_FETCHCONTENT "Use FetchContent for dependencies" OFF)

# Development options
option(MESSAGING_BUILD_TESTS "Build unit tests" ON)
option(MESSAGING_BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(MESSAGING_BUILD_EXAMPLES "Build example applications" ON)
option(MESSAGING_BUILD_DOCS "Build documentation with Doxygen" OFF)

# Build profile
set(MESSAGING_BUILD_PROFILE "Release" CACHE STRING "Build profile")
set_property(CACHE MESSAGING_BUILD_PROFILE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel"
    "ASAN" "TSAN" "UBSAN" "MSAN"
)

# ============================================================================
# Compiler Settings
# ============================================================================

# Warning level
set(MESSAGING_WARNING_LEVEL "High" CACHE STRING "Compiler warning level")

if(MESSAGING_WARNING_LEVEL STREQUAL "High" OR MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wextra -Wpedantic)
        if(MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
            add_compile_options(-Werror -Wconversion -Wsign-conversion)
        endif()
    elseif(MSVC)
        add_compile_options(/W4)
        if(MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
            add_compile_options(/WX)
        endif()
    endif()
endif()

# Build profile flags
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

# ============================================================================
# External System Integration
# ============================================================================

if(MESSAGING_USE_EXTERNAL_SYSTEMS)
    message(STATUS "Using external system packages")

    if(MESSAGING_USE_FETCHCONTENT)
        # FetchContent mode (development)
        message(STATUS "Using FetchContent for external systems")
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

    else()
        # find_package mode (production)
        message(STATUS "Using find_package for external systems")

        list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

        find_package(CommonSystem 1.0 REQUIRED)
        find_package(ThreadSystem 1.0 REQUIRED)
        find_package(LoggerSystem 1.0 REQUIRED)
        find_package(MonitoringSystem 1.0 REQUIRED)
        find_package(ContainerSystem 1.0 REQUIRED)
        find_package(DatabaseSystem 1.0 REQUIRED)
        find_package(NetworkSystem 1.0 REQUIRED)
    endif()

    # Validate dependencies
    include(cmake/validate_dependencies.cmake)
    validate_messaging_dependencies()

    # Create backward-compatible aliases
    if(NOT TARGET common)
        add_library(common ALIAS CommonSystem::common)
    endif()

    if(NOT TARGET thread_pool)
        add_library(thread_pool ALIAS ThreadSystem::Core)
    endif()

    if(NOT TARGET logger)
        add_library(logger ALIAS LoggerSystem::logger)
    endif()

    if(NOT TARGET monitoring)
        add_library(monitoring ALIAS MonitoringSystem::monitoring)
    endif()

    if(NOT TARGET container)
        add_library(container ALIAS ContainerSystem::container)
    endif()

    if(NOT TARGET database)
        add_library(database ALIAS DatabaseSystem::database)
    endif()

    if(NOT TARGET network)
        add_library(network ALIAS NetworkSystem::network)
    endif()

    # Define HAS_* macros for legacy_guard.h
    target_compile_definitions(messaging_system PUBLIC
        HAS_COMMON_SYSTEM
        HAS_THREAD_SYSTEM
        HAS_LOGGER_SYSTEM
        HAS_MONITORING_SYSTEM
        HAS_CONTAINER_SYSTEM
        HAS_DATABASE_SYSTEM
        HAS_NETWORK_SYSTEM
        MESSAGING_USE_EXTERNAL_SYSTEMS
    )

else()
    message(WARNING "Building without external systems (legacy mode)")
    # Fall back to internal implementations (deprecated)
    add_subdirectory(libraries/container_system)
    add_subdirectory(libraries/network_system)
    add_subdirectory(libraries/thread_system)
endif()

# ============================================================================
# Propagate Options to External Systems
# ============================================================================

if(MESSAGING_USE_LOCKFREE)
    set(USE_LOCKFREE_BY_DEFAULT ON CACHE BOOL "" FORCE)
    set(USE_LOCKFREE_THREAD_POOL ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_MONITORING)
    set(ENABLE_MONITORING ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_LOGGING)
    set(ENABLE_LOGGING ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_TLS)
    set(ENABLE_TLS ON CACHE BOOL "" FORCE)
endif()

# ============================================================================
# Messaging System Core Library
# ============================================================================

add_library(messaging_system_core
    src/core/message_bus.cpp
    src/core/topic_router.cpp
    src/core/message_processor.cpp
    src/core/service_container.cpp
)

target_include_directories(messaging_system_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(messaging_system_core
    PUBLIC
        CommonSystem::common
        ThreadSystem::Core
        LoggerSystem::logger
        MonitoringSystem::monitoring
        ContainerSystem::container
        DatabaseSystem::database
        NetworkSystem::network
)

target_compile_features(messaging_system_core PUBLIC cxx_std_20)

# ============================================================================
# Main Executable
# ============================================================================

add_executable(messaging_system
    src/main.cpp
)

target_link_libraries(messaging_system
    PRIVATE
        messaging_system_core
)

# ============================================================================
# Tests
# ============================================================================

if(MESSAGING_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# ============================================================================
# Benchmarks
# ============================================================================

if(MESSAGING_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ============================================================================
# Examples
# ============================================================================

if(MESSAGING_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# Documentation
# ============================================================================

if(MESSAGING_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# ============================================================================
# Installation
# ============================================================================

install(TARGETS messaging_system_core messaging_system
    EXPORT messaging_system_targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

install(EXPORT messaging_system_targets
    FILE messaging_system-targets.cmake
    NAMESPACE MessagingSystem::
    DESTINATION lib/cmake/messaging_system
)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "=== Messaging System Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build profile: ${MESSAGING_BUILD_PROFILE}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  Lock-free: ${MESSAGING_USE_LOCKFREE}")
message(STATUS "  Monitoring: ${MESSAGING_ENABLE_MONITORING}")
message(STATUS "  Logging: ${MESSAGING_ENABLE_LOGGING}")
message(STATUS "  Metrics: ${MESSAGING_ENABLE_METRICS}")
message(STATUS "  TLS: ${MESSAGING_ENABLE_TLS}")
message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  Tests: ${MESSAGING_BUILD_TESTS}")
message(STATUS "  Benchmarks: ${MESSAGING_BUILD_BENCHMARKS}")
message(STATUS "  Examples: ${MESSAGING_BUILD_EXAMPLES}")
message(STATUS "  Documentation: ${MESSAGING_BUILD_DOCS}")
message(STATUS "")
message(STATUS "External systems: ${MESSAGING_USE_EXTERNAL_SYSTEMS}")
if(MESSAGING_USE_EXTERNAL_SYSTEMS)
    message(STATUS "  Using FetchContent: ${MESSAGING_USE_FETCHCONTENT}")
endif()
message(STATUS "======================================")
message(STATUS "")
```

### Dependency Validation Script

**File:** `cmake/validate_dependencies.cmake`

```cmake
function(validate_messaging_dependencies)
    set(REQUIRED_TARGETS
        CommonSystem::common
        ThreadSystem::Core
        LoggerSystem::logger
        MonitoringSystem::monitoring
        ContainerSystem::container
        DatabaseSystem::database
        NetworkSystem::network
    )

    set(MISSING_TARGETS "")

    foreach(target ${REQUIRED_TARGETS})
        if(NOT TARGET ${target})
            list(APPEND MISSING_TARGETS ${target})
        endif()
    endforeach()

    if(MISSING_TARGETS)
        message(FATAL_ERROR
            "Required dependencies not found: ${MISSING_TARGETS}\n"
            "Please install missing packages or enable FetchContent:\n"
            "  cmake -DMESSAGING_USE_FETCHCONTENT=ON ..\n"
            "\n"
            "Or install packages:\n"
            "  Ubuntu/Debian: apt-get install lib<system>-dev\n"
            "  macOS: brew install <system>\n"
        )
    endif()

    message(STATUS "✓ All messaging_system dependencies validated")
endfunction()
```

## Task 1.2: Compiler Options and Feature Flags

### Flag Propagation Strategy

**Unified Configuration Table:**

| Messaging Flag | Target System | Target Flag | Default |
|---------------|---------------|-------------|---------|
| `MESSAGING_USE_LOCKFREE` | thread_system | `USE_LOCKFREE_THREAD_POOL` | OFF |
| `MESSAGING_USE_LOCKFREE` | logger_system | `USE_LOCKFREE_LOGGER` | OFF |
| `MESSAGING_ENABLE_MONITORING` | All systems | `ENABLE_MONITORING` | ON |
| `MESSAGING_ENABLE_LOGGING` | All systems | `ENABLE_LOGGING` | ON |
| `MESSAGING_ENABLE_TLS` | network_system | `ENABLE_TLS` | OFF |

### CMakePresets.json (Already created in Phase 0)

Located at project root, defines presets for:
- default
- debug
- release
- asan, tsan, ubsan
- dev-fetchcontent
- ci

## Task 1.3: Configuration Validation

### Validation Checklist

Pre-build validation ensures:
1. All 7 external systems are available
2. Version compatibility (all v1.0+)
3. Required features enabled in dependencies
4. No conflicting options

### Post-build Verification

```bash
# Verify all external libraries linked
ldd build/bin/messaging_system | grep -E "(common|thread|logger|monitoring|container|database|network)"

# Expected output:
# libcommon.so.1 => /usr/lib/libcommon.so.1
# libthread.so.1 => /usr/lib/libthread.so.1
# ...
```

## Task 1.4: Legacy Code Removal

### Execution Steps

1. **Backup current state**
   ```bash
   git checkout -b backup/pre-phase1
   git push origin backup/pre-phase1
   ```

2. **Run archive script**
   ```bash
   ./scripts/archive_legacy.sh
   ```

3. **Verify archive**
   ```bash
   ls -la _archived/$(date +%Y%m%d)*
   ```

4. **Update CMakeLists.txt**
   - Comment out internal add_subdirectory() calls
   - Add external system integration
   - Add validation calls

5. **Test build**
   ```bash
   cmake --preset dev-fetchcontent
   cmake --build --preset debug
   ctest --preset default
   ```

6. **Commit changes**
   ```bash
   git add -A
   git commit -m "Phase 1: External system integration complete"
   ```

## Success Criteria

### Build Success
- [ ] CMake configuration succeeds with find_package
- [ ] CMake configuration succeeds with FetchContent
- [ ] All targets build without errors
- [ ] No warnings with -Wpedantic

### Functionality
- [ ] All unit tests pass
- [ ] Integration tests pass
- [ ] Examples build and run

### Performance
- [ ] No performance regression (benchmark comparison)
- [ ] Binary size within 10% of baseline

### Documentation
- [ ] Updated README with new build instructions
- [ ] Migration guide completed
- [ ] API documentation updated

## Implementation Timeline

- **Day 1:** Update CMakeLists.txt root
- **Day 2:** Create validation scripts, CMake modules
- **Day 3:** Test FetchContent mode
- **Day 4:** Test find_package mode
- **Day 5:** Archive legacy code, final testing

## Rollback Plan

If Phase 1 fails:
1. Restore from `_archived/` directory
2. Revert CMakeLists.txt changes
3. Rebuild with internal systems
4. Document failure reasons
5. Adjust plan and retry

## Next Phase

Upon successful completion, proceed to Phase 2: Messaging Core Redesign.
