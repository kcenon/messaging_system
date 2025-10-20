# Legacy Code Removal Plan

## Overview
This document identifies all legacy code to be removed and defines the responsibility matrix for replacement with external systems.

## Legacy Code Inventory

### 1. Internal Container System
**Location:** `libraries/container_system/` (embedded in messaging_system)

**Files to Archive:**
- `libraries/container_system/core/`
- `libraries/container_system/include/`
- `libraries/container_system/src/`

**Replacement:** External `container_system` package
**Status:** ⚠️ Currently embedded, needs extraction

### 2. Internal Network System
**Location:** `libraries/network_system/` (embedded in messaging_system)

**Files to Archive:**
- `libraries/network_system/core/`
- `libraries/network_system/include/`
- `libraries/network_system/src/`

**Replacement:** External `network_system` package
**Status:** ⚠️ Currently embedded, needs extraction

### 3. Internal Thread System
**Location:** `libraries/thread_system/` (embedded in messaging_system)

**Files to Archive:**
- `libraries/thread_system/sources/`
- `libraries/thread_system/include/`

**Replacement:** External `thread_system` package
**Status:** ⚠️ Currently embedded, needs extraction

### 4. Exception-Based Error Handling
**Location:** Throughout codebase

**Patterns to Remove:**
```cpp
// Pattern 1: throw statements
throw std::runtime_error("error message");
throw std::invalid_argument("invalid arg");

// Pattern 2: try-catch blocks
try {
    operation();
} catch (const std::exception& e) {
    handle_error(e.what());
}

// Pattern 3: Exception specifications
void function() throw(std::runtime_error);
```

**Replacement:** `Result<T>` return types
**Status:** ⚠️ Needs systematic conversion

### 5. Direct Console Output
**Location:** Throughout codebase

**Patterns to Remove:**
```cpp
std::cout << "message" << std::endl;
std::cerr << "error" << std::endl;
printf("message\n");
fprintf(stderr, "error\n");
```

**Replacement:** `ILogger` interface
**Status:** ⚠️ Needs systematic conversion

## Responsibility Matrix

| Functionality | Legacy Implementation | New Owner | Interface | Status |
|--------------|----------------------|-----------|-----------|--------|
| **Error Handling** | `std::exception` | `common_system` | `Result<T>`, `VoidResult` | Replace |
| **Async Execution** | Internal thread pool | `thread_system` | `IExecutor`, `IJob` | Replace |
| **Message Serialization** | Internal container | `container_system` | `value_container` | Replace |
| **Network I/O** | Internal network | `network_system` | `messaging_client`, `messaging_server` | Replace |
| **Persistence** | Direct SQL calls | `database_system` | `IDatabase`, ORM | Replace |
| **Logging** | `std::cout`/`std::cerr` | `logger_system` | `ILogger` | Replace |
| **Metrics** | None | `monitoring_system` | `IMonitor` | Add |
| **Event Bus** | Custom implementation | `common_system` | `event_bus` | Replace |
| **DI Container** | None | `thread_system` | `service_registry` | Add |

## Detailed Replacement Mapping

### Error Handling Transformation

#### Before (Exception-based)
```cpp
class MessageProcessor {
public:
    void process_message(const Message& msg) {
        if (!validate(msg)) {
            throw std::invalid_argument("Invalid message format");
        }

        try {
            route_message(msg);
        } catch (const NetworkException& e) {
            log_error("Network error: " + std::string(e.what()));
            throw;
        }
    }
};
```

#### After (Result-based)
```cpp
#include <kcenon/common/patterns/result.h>

class MessageProcessor {
public:
    Result<void> process_message(const Message& msg) {
        auto validation = validate(msg);
        RETURN_IF_ERROR(validation);

        auto routing = route_message(msg);
        if (routing.is_error()) {
            logger_->log(log_level::error,
                "Network error: " + routing.error().message);
            return routing;
        }

        return VoidResult::ok();
    }
};
```

### Async Execution Transformation

#### Before (Internal thread pool)
```cpp
class MessageBus {
    std::vector<std::thread> workers_;
    std::queue<Message> queue_;

public:
    void publish_async(Message msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        cv_.notify_one();
    }
};
```

#### After (IExecutor-based)
```cpp
#include <kcenon/common/interfaces/executor_interface.h>

class MessageBus {
    std::shared_ptr<IExecutor> executor_;

public:
    Result<void> publish_async(Message msg) {
        auto job = std::make_unique<MessageProcessJob>(std::move(msg));
        return executor_->execute(std::move(job));
    }
};
```

### Container API Transformation

#### Before (Internal container)
```cpp
InternalContainer msg;
msg.set_field("user_id", 123);
msg.set_field("action", "login");
msg.set_field("timestamp", time(nullptr));

std::string serialized = msg.to_binary();
```

#### After (value_container)
```cpp
#include <kcenon/container/core/container.h>

auto msg = messaging_container_builder()
    .source("client_123")
    .target("auth_service")
    .add_value("header.user_id", 123)
    .add_value("header.action", "login")
    .add_value("header.timestamp", std::chrono::system_clock::now())
    .optimize_for_speed()
    .build();

auto serialized = msg.serialize();
```

### Logging Transformation

#### Before (Direct output)
```cpp
std::cout << "Processing message from user " << user_id << std::endl;
std::cerr << "Error: Failed to connect to database" << std::endl;
```

#### After (ILogger)
```cpp
#include <kcenon/common/interfaces/logger_interface.h>

logger_->log(log_level::info,
    fmt::format("Processing message from user {}", user_id));

logger_->log(log_level::error,
    "Failed to connect to database",
    source_location{__FILE__, __LINE__, __func__});
```

## Archive Strategy

### Archive Directory Structure
```
_archived/
├── YYYYMMDD_HHMMSS/
│   ├── container_system/
│   ├── network_system/
│   ├── thread_system/
│   └── exception_handlers/
└── README.md (explains what was archived and when)
```

### Archive Script

Create: `scripts/archive_legacy.sh`

```bash
#!/bin/bash

# Archive legacy code before removal
# Usage: ./scripts/archive_legacy.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
ARCHIVE_DIR="${PROJECT_ROOT}/_archived/${TIMESTAMP}"

echo "Creating archive directory: ${ARCHIVE_DIR}"
mkdir -p "${ARCHIVE_DIR}"

# Define legacy directories to archive
LEGACY_DIRS=(
    "libraries/container_system"
    "libraries/network_system"
    "libraries/thread_system"
)

# Archive each directory
for dir in "${LEGACY_DIRS[@]}"; do
    full_path="${PROJECT_ROOT}/${dir}"
    if [ -d "$full_path" ]; then
        echo "Archiving: ${dir}"
        cp -r "$full_path" "${ARCHIVE_DIR}/"
        echo "  ✓ Archived to ${ARCHIVE_DIR}/$(basename ${dir})"
    else
        echo "  ⚠ Directory not found: ${dir}"
    fi
done

# Create archive manifest
cat > "${ARCHIVE_DIR}/MANIFEST.md" <<EOF
# Legacy Code Archive

**Archive Date:** ${TIMESTAMP}
**Created By:** System rebuild process

## Archived Components

$(for dir in "${LEGACY_DIRS[@]}"; do
    if [ -d "${PROJECT_ROOT}/${dir}" ]; then
        echo "- ${dir}"
    fi
done)

## Reason for Archive
These components have been replaced with external system packages as part of the Phase 1 rebuild.

## Restoration (if needed)
To restore archived code:
\`\`\`bash
cp -r _archived/${TIMESTAMP}/<component> libraries/
\`\`\`

## External Replacements
- container_system → External container_system package
- network_system → External network_system package
- thread_system → External thread_system package

## Notes
- Do not restore unless absolutely necessary
- External systems provide improved performance and maintainability
- See docs/REBUILD_PLAN.md for migration details
EOF

echo ""
echo "✓ Archive complete: ${ARCHIVE_DIR}"
echo "  Manifest: ${ARCHIVE_DIR}/MANIFEST.md"
echo ""
echo "To remove archived code from build:"
echo "  Edit CMakeLists.txt and remove add_subdirectory() calls"
echo ""
```

### CMakeLists.txt Cleanup

Create: `scripts/update_cmake_legacy.sh`

```bash
#!/bin/bash

# Update CMakeLists.txt to exclude legacy directories
# Usage: ./scripts/update_cmake_legacy.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CMAKE_FILE="${PROJECT_ROOT}/CMakeLists.txt"

echo "Updating ${CMAKE_FILE} to exclude legacy directories..."

# Backup original
cp "${CMAKE_FILE}" "${CMAKE_FILE}.backup"

# Comment out legacy add_subdirectory calls
sed -i.bak \
    -e 's/^\(\s*add_subdirectory(libraries\/container_system)\)/# ARCHIVED: \1/' \
    -e 's/^\(\s*add_subdirectory(libraries\/network_system)\)/# ARCHIVED: \1/' \
    -e 's/^\(\s*add_subdirectory(libraries\/thread_system)\)/# ARCHIVED: \1/' \
    "${CMAKE_FILE}"

echo "✓ CMakeLists.txt updated"
echo "  Backup saved to: ${CMAKE_FILE}.backup"
```

## Build Guard Implementation

### Legacy Guard Header

Create: `include/messaging_system/legacy_guard.h`

```cpp
#pragma once

// This header ensures external systems are properly configured

#ifndef MESSAGING_USE_EXTERNAL_SYSTEMS
  #error "Legacy internal systems are deprecated. Set MESSAGING_USE_EXTERNAL_SYSTEMS=ON in CMake"
#endif

// Verify each required system is available

#if !defined(HAS_COMMON_SYSTEM)
  #error "CommonSystem not found. Install common_system package or enable FetchContent"
  #error "  apt-get install libcommon-system-dev"
  #error "  OR cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_THREAD_SYSTEM)
  #error "ThreadSystem not found. Install thread_system package or enable FetchContent"
  #error "  apt-get install libthread-system-dev"
  #error "  OR cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_LOGGER_SYSTEM)
  #error "LoggerSystem not found. Install logger_system package or enable FetchContent"
  #error "  apt-get install liblogger-system-dev"
  #error "  OR cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_MONITORING_SYSTEM)
  #error "MonitoringSystem not found. Install monitoring_system package or enable FetchContent"
  #error "  apt-get install libmonitoring-system-dev"
  #error "  OR cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_CONTAINER_SYSTEM)
  #error "ContainerSystem not found. Install container_system package or enable FetchContent"
  #error "  apt-get install libcontainer-system-dev"
  #error "  OR cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_DATABASE_SYSTEM)
  #error "DatabaseSystem not found. Install database_system package or enable FetchContent"
  #error "  apt-get install libdatabase-system-dev"
  #error "  OR cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_NETWORK_SYSTEM)
  #error "NetworkSystem not found. Install network_system package or enable FetchContent"
  #error "  apt-get install libnetwork-system-dev"
  #error "  OR cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

// All systems verified - proceed with build
```

### CMake Configuration for Guards

```cmake
# Define HAS_* macros when systems are found
if(TARGET CommonSystem::common)
    target_compile_definitions(messaging_system PUBLIC HAS_COMMON_SYSTEM)
endif()

if(TARGET ThreadSystem::Core)
    target_compile_definitions(messaging_system PUBLIC HAS_THREAD_SYSTEM)
endif()

if(TARGET LoggerSystem::logger)
    target_compile_definitions(messaging_system PUBLIC HAS_LOGGER_SYSTEM)
endif()

if(TARGET MonitoringSystem::monitoring)
    target_compile_definitions(messaging_system PUBLIC HAS_MONITORING_SYSTEM)
endif()

if(TARGET ContainerSystem::container)
    target_compile_definitions(messaging_system PUBLIC HAS_CONTAINER_SYSTEM)
endif()

if(TARGET DatabaseSystem::database)
    target_compile_definitions(messaging_system PUBLIC HAS_DATABASE_SYSTEM)
endif()

if(TARGET NetworkSystem::network)
    target_compile_definitions(messaging_system PUBLIC HAS_NETWORK_SYSTEM)
endif()
```

## Removal Checklist

### Phase 1.4 Execution Checklist
- [ ] Run archive script: `./scripts/archive_legacy.sh`
- [ ] Verify archive created in `_archived/`
- [ ] Run CMake update script: `./scripts/update_cmake_legacy.sh`
- [ ] Add legacy_guard.h to include path
- [ ] Update main CMakeLists.txt with guard definitions
- [ ] Test build with external systems
- [ ] Verify all tests pass
- [ ] Commit changes

### Post-Removal Verification
- [ ] Build succeeds with external systems
- [ ] Build fails without external systems (guards working)
- [ ] No references to archived code in active source
- [ ] All include paths updated
- [ ] Documentation updated

## Success Criteria
- [x] Legacy code inventory complete
- [x] Responsibility matrix defined
- [x] Archive strategy documented
- [x] Archive scripts created
- [x] Build guards implemented
- [x] Removal checklist provided
