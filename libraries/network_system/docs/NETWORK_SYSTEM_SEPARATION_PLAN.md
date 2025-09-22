# Network System Separation Plan
# Detailed Plan for Separating network_system from messaging_system

**Date**: 2025-09-19
**Version**: 1.0.0
**Owner**: kcenon

---

## 📋 Project Overview

### Objective
Separate the network module from messaging_system into an independent network_system to improve modularity, reusability, and maintainability.

### Scope
- Complete separation of `Sources/messaging_system/network/` module
- Integration or replacement with existing `Sources/network_system/`
- Maintain compatibility with messaging_system
- Integration with other separated systems (container_system, database_system, thread_system)

---

## 🏗️ Current State Analysis

### messaging_system/network Structure
```
messaging_system/network/
├── core/                               # Public API layer
│   ├── messaging_client.h/cpp             # TCP client implementation
│   └── messaging_server.h/cpp             # TCP server implementation
├── session/                            # Session management layer
│   └── messaging_session.h/cpp            # Connection session handling
├── internal/                           # Internal implementation layer
│   ├── tcp_socket.h/cpp                   # TCP socket wrapper
│   ├── send_coroutine.h/cpp               # Async send coroutines
│   ├── pipeline.h/cpp                     # Message pipeline
│   └── common_defs.h                      # Common definitions
├── network/                            # Integration interface
│   └── network.h                          # Main header
├── CMakeLists.txt                      # Build configuration
└── README.md                           # Module documentation
```

### Existing network_system Structure
```
network_system/
├── core/                               # Existing network core
├── internal/                           # Existing internal implementation
├── session/                            # Existing session management
├── samples/                            # Sample code
├── tests/                             # Test code
├── CMakeLists.txt                     # Build file
└── README.md                          # Project documentation
```

### Key Dependencies
- **ASIO**: Asynchronous I/O and networking
- **fmt**: String formatting library
- **container_system**: Message serialization/deserialization
- **thread_system**: Asynchronous task scheduling
- **utilities**: System utilities (internal to messaging_system)

---

## 🎯 Separation Strategy

### 1. Basic Principles
- **Gradual Migration**: Ensure existing code compatibility while progressive separation
- **Dependency Inversion**: network_system operates independently but can integrate when needed
- **Follow Existing Patterns**: Adhere to structures and patterns of successfully separated systems
- **Performance Optimization**: Minimize performance overhead from separation

### 2. Namespace Policy
```cpp
// Existing (inside messaging_system)
namespace network_module { ... }

// New (independent network_system)
namespace network_system {
    namespace core { ... }          // Core API
    namespace session { ... }       // Session management
    namespace integration { ... }   // System integration interfaces
}
```

### 3. Compatibility Layer
```cpp
namespace network_system::integration {
    // Bridge for messaging_system compatibility
    class messaging_bridge;

    // container_system integration
    class container_integration;

    // thread_system integration
    class thread_integration;
}
```

---

## 📅 Implementation Roadmap

### Phase 1: Preparation and Analysis (2-3 days)

#### Day 1
- [ ] Complete backup of messaging_system/network
- [ ] Detailed analysis of differences with existing network_system
- [ ] Create dependency graph
- [ ] Review migration scenarios

#### Day 2
- [ ] Design new directory structure
- [ ] Define namespace and module boundaries
- [ ] Design integration interfaces
- [ ] Design build system architecture

#### Day 3
- [ ] Establish testing strategy
- [ ] Set performance benchmark baselines
- [ ] Create rollback plan

### Phase 2: Core System Separation (4-5 days)

#### Day 4-5
- [ ] Create new network_system directory structure
- [ ] Copy and reorganize messaging_system/network code
- [ ] Apply namespace changes
- [ ] Configure basic CMakeLists.txt

#### Day 6-7
- [ ] Reconfigure dependencies (ASIO, fmt, container_system, thread_system)
- [ ] Implement independent build system
- [ ] Configure vcpkg.json and dependency.sh

#### Day 8
- [ ] Write basic unit tests
- [ ] Verify independent build

### Phase 3: Integration Interface Implementation (3-4 days)

#### Day 9-10
- [ ] Implement messaging_bridge class
- [ ] container_system integration interface
- [ ] thread_system integration interface

#### Day 11-12
- [ ] Implement compatibility API
- [ ] Write integration tests

### Phase 4: messaging_system Update (2-3 days)

#### Day 13-14
- [ ] Update messaging_system CMakeLists.txt
- [ ] Add external network_system usage option
- [ ] Verify existing code compatibility

#### Day 15
- [ ] Run integration tests
- [ ] Compare performance benchmarks

### Phase 5: Verification and Deployment (2-3 days)

#### Day 16-17
- [ ] Full system integration testing
- [ ] Performance optimization
- [ ] Update documentation

#### Day 18
- [ ] Final verification
- [ ] Prepare for deployment

---

## 🛠️ Technical Implementation Details

### 1. Directory Structure Reorganization

#### Target Structure
```
Sources/network_system/
├── include/network_system/             # Public headers
│   ├── core/
│   │   ├── messaging_client.h
│   │   └── messaging_server.h
│   ├── session/
│   │   └── messaging_session.h
│   ├── integration/
│   │   ├── messaging_bridge.h
│   │   ├── container_integration.h
│   │   └── thread_integration.h
│   └── network_system.h               # Main header
├── src/                               # Implementation files
│   ├── core/
│   ├── session/
│   ├── internal/
│   └── integration/
├── samples/                           # Sample code
│   ├── basic_client_server/
│   ├── container_integration/
│   └── messaging_system_bridge/
├── tests/                             # Tests
│   ├── unit/
│   ├── integration/
│   └── performance/
├── docs/                              # Documentation
│   ├── api/
│   ├── tutorials/
│   └── migration_guide.md
├── cmake/                             # CMake modules
├── scripts/                           # Utility scripts
│   ├── build.sh
│   ├── dependency.sh
│   └── migration/
├── CMakeLists.txt                     # Main build file
├── vcpkg.json                         # Package dependencies
├── README.md                          # Project documentation
└── CHANGELOG.md                       # Change history
```

### 2. CMake Build System

#### Main CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)

project(NetworkSystem
    VERSION 2.0.0
    DESCRIPTION "High-performance modular network system"
    LANGUAGES CXX
)

# Options
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SAMPLES "Build samples" ON)
option(BUILD_WITH_CONTAINER_SYSTEM "Build with container_system integration" ON)
option(BUILD_WITH_THREAD_SYSTEM "Build with thread_system integration" ON)
option(BUILD_MESSAGING_BRIDGE "Build messaging_system bridge" ON)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Conditional dependencies
if(BUILD_WITH_CONTAINER_SYSTEM)
    find_package(ContainerSystem CONFIG REQUIRED)
endif()

if(BUILD_WITH_THREAD_SYSTEM)
    find_package(ThreadSystem CONFIG REQUIRED)
endif()

# Create library
add_subdirectory(src)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Samples
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# Installation rules
include(GNUInstallDirs)
install(TARGETS NetworkSystem
    EXPORT NetworkSystemTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Config file generation
install(EXPORT NetworkSystemTargets
    FILE NetworkSystemTargets.cmake
    NAMESPACE NetworkSystem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NetworkSystem
)
```

### 3. Integration Interface Design

#### messaging_bridge.h
```cpp
#pragma once

#include "network_system/core/messaging_client.h"
#include "network_system/core/messaging_server.h"

#ifdef BUILD_WITH_CONTAINER_SYSTEM
#include "container_system/container.h"
#endif

namespace network_system::integration {

class messaging_bridge {
public:
    messaging_bridge();
    ~messaging_bridge();

    // messaging_system compatible API
    std::shared_ptr<core::messaging_server> create_server(
        const std::string& server_id
    );

    std::shared_ptr<core::messaging_client> create_client(
        const std::string& client_id
    );

#ifdef BUILD_WITH_CONTAINER_SYSTEM
    // container_system integration
    void set_container_factory(
        std::shared_ptr<container_system::factory> factory
    );

    void set_container_message_handler(
        std::function<void(const container_system::message&)> handler
    );
#endif

#ifdef BUILD_WITH_THREAD_SYSTEM
    // thread_system integration
    void set_thread_pool(
        std::shared_ptr<thread_system::thread_pool> pool
    );
#endif

    // Performance monitoring
    struct performance_metrics {
        uint64_t messages_sent = 0;
        uint64_t messages_received = 0;
        uint64_t bytes_sent = 0;
        uint64_t bytes_received = 0;
        uint64_t connections_active = 0;
        std::chrono::milliseconds avg_latency{0};
    };

    performance_metrics get_metrics() const;
    void reset_metrics();

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace network_system::integration
```

### 4. Dependency Management

#### vcpkg.json
```json
{
    "name": "network-system",
    "version-string": "2.0.0",
    "description": "High-performance modular network system",
    "dependencies": [
        "asio",
        "fmt",
        {
            "name": "gtest",
            "$condition": "BUILD_TESTS"
        },
        {
            "name": "benchmark",
            "$condition": "BUILD_TESTS"
        }
    ],
    "features": {
        "container-integration": {
            "description": "Enable container_system integration",
            "dependencies": [
                "container-system"
            ]
        },
        "thread-integration": {
            "description": "Enable thread_system integration",
            "dependencies": [
                "thread-system"
            ]
        }
    }
}
```

#### dependency.sh
```bash
#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

echo "========================================="
echo "Network System Dependency Setup"
echo "========================================="

# Check vcpkg
if ! command -v vcpkg &> /dev/null; then
    echo "Error: vcpkg not found. Please install vcpkg first."
    exit 1
fi

# Install basic dependencies
echo "Installing basic dependencies..."
vcpkg install asio fmt

# Conditional dependencies
if [[ "${BUILD_TESTS:-ON}" == "ON" ]]; then
    echo "Installing test dependencies..."
    vcpkg install gtest benchmark
fi

# Check external system dependencies
echo "Checking external system dependencies..."

# Check container_system
CONTAINER_SYSTEM_DIR="../container_system"
if [[ -d "$CONTAINER_SYSTEM_DIR" ]]; then
    echo "Found container_system at $CONTAINER_SYSTEM_DIR"
    export BUILD_WITH_CONTAINER_SYSTEM=ON
else
    echo "Warning: container_system not found. Integration will be disabled."
    export BUILD_WITH_CONTAINER_SYSTEM=OFF
fi

# Check thread_system
THREAD_SYSTEM_DIR="../thread_system"
if [[ -d "$THREAD_SYSTEM_DIR" ]]; then
    echo "Found thread_system at $THREAD_SYSTEM_DIR"
    export BUILD_WITH_THREAD_SYSTEM=ON
else
    echo "Warning: thread_system not found. Integration will be disabled."
    export BUILD_WITH_THREAD_SYSTEM=OFF
fi

echo "Dependency setup completed successfully!"
```

---

## 🧪 Testing Strategy

### 1. Unit Tests
- **Core modules**: Individual functionality testing of messaging_client, messaging_server
- **Session modules**: Connection management and session state testing
- **Integration modules**: Bridge and integration interface testing

### 2. Integration Tests
- **Independent operation**: Standalone network_system client-server communication
- **container_system integration**: Message serialization/deserialization integration
- **thread_system integration**: Asynchronous task scheduling integration
- **messaging_system compatibility**: Compatibility verification with existing code

### 3. Performance Tests
- **Throughput**: Messages/second processing performance
- **Latency**: Message transmission latency
- **Memory usage**: Memory usage per connection
- **CPU utilization**: CPU efficiency under load

### 4. Stress Tests
- **High connection load**: 10K+ concurrent connection handling
- **Long-term operation**: 24+ hour stability testing
- **Memory leak detection**: Long-term memory leak inspection

---

## 📊 Performance Requirements

### Baseline (Before Separation)
- **Throughput**: 100K messages/sec (1KB messages, localhost)
- **Latency**: < 1ms (message processing latency)
- **Concurrent connections**: 10K+ connections
- **Memory usage**: ~8KB per connection

### Target (After Separation)
- **Throughput**: >= 100K messages/sec (performance degradation < 5%)
- **Latency**: < 1.2ms (bridge overhead < 20%)
- **Concurrent connections**: 10K+ connections (no change)
- **Memory usage**: ~10KB per connection (increase < 25%)

---

## 🚨 Risk Factors and Mitigation

### 1. Major Risks
| Risk Factor | Impact | Probability | Mitigation |
|-------------|---------|-------------|------------|
| Conflict with existing network_system | High | Medium | Namespace separation, rename existing system |
| Circular dependencies | High | Low | Interface-based dependency inversion |
| Performance degradation | Medium | Medium | Zero-copy bridge, optimized integration layer |
| Compatibility issues | High | Low | Thorough compatibility testing, gradual migration |
| Increased build complexity | Low | High | Automated build scripts, clear documentation |

### 2. Rollback Plan
- **Phase checkpoints**: Set rollback points for each phase
- **Backup retention**: Keep original code backups (minimum 30 days)
- **Feature toggles**: Runtime switching between old/new systems
- **Performance monitoring**: Real-time performance monitoring for early problem detection

---

## 📝 Documentation Plan

### 1. API Documentation
- **Doxygen**: Auto-generated API reference
- **Usage examples**: Practical, usable code examples
- **Migration guide**: Transition methods from messaging_system

### 2. Architecture Documentation
- **System design**: Overall architecture and inter-module relationships
- **Dependency graph**: Visual dependency relationship diagrams
- **Performance characteristics**: Benchmark results and optimization guides

### 3. Operations Documentation
- **Build guide**: Build methods for various environments
- **Deployment guide**: Production environment deployment procedures
- **Troubleshooting**: Common problems and solutions

---

## ✅ Completion Checklist

### Phase 1: Preparation and Analysis
- [ ] messaging_system/network backup completed
- [ ] Analysis of differences with existing network_system completed
- [ ] Dependency graph creation completed
- [ ] New directory structure design completed
- [ ] Namespace definition completed
- [ ] Testing strategy established

### Phase 2: Core System Separation
- [ ] New directory structure creation completed
- [ ] Code copy and reorganization completed
- [ ] Namespace changes applied
- [ ] Basic CMakeLists.txt configuration completed
- [ ] Dependency reconfiguration completed
- [ ] Independent build system implementation completed
- [ ] Basic unit tests written

### Phase 3: Integration Interface Implementation
- [ ] messaging_bridge class implementation completed
- [ ] container_system integration interface completed
- [ ] thread_system integration interface completed
- [ ] Compatibility API implementation completed
- [ ] Integration tests written

### Phase 4: messaging_system Update
- [ ] messaging_system CMakeLists.txt update completed
- [ ] External network_system usage option added
- [ ] Existing code compatibility verification completed
- [ ] Integration tests executed
- [ ] Performance benchmark comparison completed

### Phase 5: Verification and Deployment
- [ ] Full system integration testing completed
- [ ] Performance optimization completed
- [ ] Documentation update completed
- [ ] Final verification completed
- [ ] Deployment preparation completed

---

## 📞 Contact and Support

**Project Owner**: kcenon (kcenon@naver.com)
**Project Duration**: 2025-09-19 ~ 2025-10-07 (estimated 18 days)
**Status**: Plan completed, awaiting implementation

---

*This document is the master plan for network_system separation work. It will be continuously updated according to the progress of each phase.*