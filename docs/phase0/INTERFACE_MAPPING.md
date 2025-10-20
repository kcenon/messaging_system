# System Interface Mapping Document

## Overview
This document maps legacy messaging system types to new external system interfaces.

## Common System Interfaces

### Error Handling
| Legacy Type | New Interface | Location |
|------------|---------------|----------|
| `std::exception` | `common::Result<T>` | `include/kcenon/common/patterns/result.h` |
| `throw std::runtime_error` | `common::VoidResult` | `include/kcenon/common/patterns/result.h` |
| N/A | `common::error_info` | `include/kcenon/common/patterns/result.h` |

### Event System
| Legacy Type | New Interface | Location |
|------------|---------------|----------|
| Custom event callbacks | `common::event_bus` | `include/kcenon/common/patterns/event_bus.h` |
| N/A | `common::module_started_event` | `include/kcenon/common/patterns/event_bus.h` |
| N/A | `common::error_event` | `include/kcenon/common/patterns/event_bus.h` |

### Execution Contracts
| Legacy Type | New Interface | Location |
|------------|---------------|----------|
| Internal thread pool | `common::IExecutor` | `include/kcenon/common/interfaces/executor_interface.h` |
| N/A | `common::IJob` | `include/kcenon/common/interfaces/executor_interface.h` |

### Logging Contracts
| Legacy Type | New Interface | Location |
|------------|---------------|----------|
| `std::cout` / `std::cerr` | `common::ILogger` | `include/kcenon/common/interfaces/logger_interface.h` |
| N/A | `common::log_level` | `include/kcenon/common/interfaces/logger_interface.h` |
| N/A | `common::log_entry` | `include/kcenon/common/interfaces/logger_interface.h` |

### Monitoring Contracts
| Legacy Type | New Interface | Location |
|------------|---------------|----------|
| N/A | `common::IMonitor` | `include/kcenon/common/interfaces/monitoring_interface.h` |
| N/A | `common::metric_value` | `include/kcenon/common/interfaces/monitoring_interface.h` |
| N/A | `common::health_status` | `include/kcenon/common/interfaces/monitoring_interface.h` |

### Database Contracts
| Legacy Type | New Interface | Location |
|------------|---------------|----------|
| Direct SQL | `common::IDatabase` | `include/kcenon/common/interfaces/database_interface.h` |
| N/A | `common::database_value` | `include/kcenon/common/interfaces/database_interface.h` |
| N/A | `common::database_result` | `include/kcenon/common/interfaces/database_interface.h` |

## Error Code Ranges

### System Error Code Allocation
```cpp
// Common System: -1 to -99
namespace common::error::codes {
    constexpr int GENERIC_ERROR = -1;
    constexpr int INVALID_ARGUMENT = -2;
    constexpr int OUT_OF_RANGE = -3;
    // ... up to -99
}

// Thread System: -100 to -199
namespace thread::error::codes {
    constexpr int THREAD_CREATION_FAILED = -100;
    constexpr int THREAD_POOL_SHUTDOWN = -101;
    // ... up to -199
}

// Messaging System: -200 to -299 (NEW)
namespace messaging::error {
    constexpr int INVALID_MESSAGE = -200;
    constexpr int ROUTING_FAILED = -201;
    constexpr int SERIALIZATION_ERROR = -202;
    constexpr int NETWORK_ERROR = -203;
    constexpr int DATABASE_ERROR = -204;
    constexpr int QUEUE_FULL = -205;
    constexpr int TIMEOUT = -206;
    constexpr int AUTHENTICATION_FAILED = -207;
    constexpr int AUTHORIZATION_FAILED = -208;
}

// Logger System: -300 to -399
namespace logger::error::codes {
    constexpr int WRITER_ERROR = -300;
    constexpr int FILE_ERROR = -301;
    // ... up to -399
}

// Monitoring System: -400 to -499
namespace monitoring::error::codes {
    constexpr int COLLECTOR_ERROR = -400;
    constexpr int STORAGE_ERROR = -401;
    // ... up to -499
}

// Container System: -500 to -599
namespace container::error::codes {
    constexpr int SERIALIZATION_ERROR = -500;
    constexpr int DESERIALIZATION_ERROR = -501;
    // ... up to -599
}

// Database System: -600 to -699
namespace database::error::codes {
    constexpr int CONNECTION_ERROR = -600;
    constexpr int QUERY_ERROR = -601;
    // ... up to -699
}

// Network System: -700 to -799
namespace network::error::codes {
    constexpr int CONNECTION_FAILED = -700;
    constexpr int SEND_FAILED = -701;
    // ... up to -799
}
```

## Legacy Dependencies to Remove

### Internal Implementations
1. **Internal Container System**
   - Location: `libraries/container_system/` (embedded)
   - Remove: All internal container implementations
   - Replace with: External `container_system` package

2. **Internal Network System**
   - Location: `libraries/network_system/` (embedded)
   - Remove: All internal network implementations
   - Replace with: External `network_system` package

3. **Internal Thread Pool**
   - Location: `libraries/thread_system/` (embedded)
   - Remove: All internal thread pool implementations
   - Replace with: External `thread_system` package

4. **Exception-Based Error Handling**
   - Location: Throughout codebase
   - Remove: `throw` statements, `try-catch` blocks
   - Replace with: `Result<T>` return types

5. **Direct Console Output**
   - Location: Throughout codebase
   - Remove: `std::cout`, `std::cerr` usage
   - Replace with: `ILogger` interface

### External Dependencies
None - all systems are header-only or provide clean interfaces.

## Exception Mapper Utility

### Usage Pattern
```cpp
#include <kcenon/common/patterns/result.h>

// Automatic exception to error_info conversion
auto result = common::try_catch<int>([]() {
    // Code that might throw
    if (condition) {
        throw std::runtime_error("Something failed");
    }
    return 42;
});

if (result.is_error()) {
    // Handle error
    std::cerr << result.error().message << std::endl;
}
```

## Migration Checklist

### Phase 0 Completion Criteria
- [x] Interface mapping document created
- [ ] Error code ranges documented
- [ ] Legacy dependency list finalized
- [ ] Exception mapper patterns documented
- [ ] All stakeholders reviewed

### Next Steps
Proceed to Task 0.2: Build Environment Audit
