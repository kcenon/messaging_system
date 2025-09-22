# Logger System - Project Structure

## 📁 Directory Layout

```
logger_system/
├── 📁 include/kcenon/logger/    # Public headers & interfaces
│   ├── 📁 core/                 # Core APIs (logger, config, builder)
│   │   ├── logger.h             # Main logger interface
│   │   ├── config.h             # Configuration management
│   │   └── builder.h            # Logger builder pattern
│   ├── 📁 interfaces/           # Interface definitions
│   │   ├── writer_interface.h   # Base writer interface
│   │   ├── formatter_interface.h # Base formatter interface
│   │   └── filter_interface.h   # Base filter interface
│   ├── 📁 writers/              # Writer interfaces
│   │   ├── file_writer.h        # File output writer
│   │   ├── console_writer.h     # Console output writer
│   │   └── network_writer.h     # Network output writer
│   ├── 📁 formatters/           # Formatter interfaces
│   │   ├── text_formatter.h     # Plain text formatting
│   │   ├── json_formatter.h     # JSON formatting
│   │   └── xml_formatter.h      # XML formatting
│   └── 📁 utils/                # Public utilities
│       ├── log_level.h          # Log level definitions
│       ├── timestamp.h          # Timestamp utilities
│       └── thread_safe.h        # Thread safety utilities
├── 📁 src/                      # Implementation files
│   ├── 📁 core/                 # Core implementation
│   │   ├── logger.cpp           # Main logger implementation
│   │   ├── config.cpp           # Configuration implementation
│   │   └── builder.cpp          # Builder implementation
│   ├── 📁 impl/                 # Private implementations
│   │   ├── 📁 writers/          # Writer implementations
│   │   │   ├── file_writer.cpp
│   │   │   ├── console_writer.cpp
│   │   │   └── network_writer.cpp
│   │   ├── 📁 formatters/       # Formatter implementations
│   │   │   ├── text_formatter.cpp
│   │   │   ├── json_formatter.cpp
│   │   │   └── xml_formatter.cpp
│   │   ├── 📁 filters/          # Filter implementations
│   │   │   ├── level_filter.cpp
│   │   │   ├── pattern_filter.cpp
│   │   │   └── rate_limiter.cpp
│   │   ├── 📁 async/            # Async components
│   │   │   ├── async_logger.cpp
│   │   │   ├── buffer_manager.cpp
│   │   │   └── worker_thread.cpp
│   │   ├── 📁 di/               # Dependency injection container
│   │   │   ├── container.cpp
│   │   │   └── factory.cpp
│   │   └── 📁 monitoring/       # Performance monitoring adapters
│   │       ├── metrics_collector.cpp
│   │       └── health_monitor.cpp
│   └── 📁 utils/                # Utility implementations
│       ├── timestamp.cpp        # Timestamp implementation
│       ├── thread_safe.cpp      # Thread safety implementation
│       └── string_utils.cpp     # String utility functions
├── 📁 tests/                    # Comprehensive test suite
│   ├── 📁 unit/                 # Unit tests
│   │   ├── core_tests/          # Core functionality tests
│   │   ├── writer_tests/        # Writer component tests
│   │   ├── formatter_tests/     # Formatter component tests
│   │   └── filter_tests/        # Filter component tests
│   ├── 📁 integration/          # Integration tests
│   │   ├── ecosystem_tests/     # Cross-system integration
│   │   ├── performance_tests/   # Performance integration
│   │   └── stress_tests/        # Stress testing
│   └── 📁 benchmarks/           # Performance benchmarks
│       ├── throughput_bench/    # Throughput measurements
│       ├── latency_bench/       # Latency measurements
│       └── memory_bench/        # Memory usage benchmarks
├── 📁 examples/                 # Usage examples & demos
│   ├── 📁 basic/                # Basic usage examples
│   ├── 📁 advanced/             # Advanced configuration examples
│   └── 📁 integration/          # System integration examples
├── 📁 docs/                     # Comprehensive documentation
│   ├── 📁 api/                  # API documentation
│   ├── 📁 guides/               # User guides & tutorials
│   ├── 📁 architecture/         # Architecture documentation
│   └── 📁 performance/          # Performance guides & benchmarks
├── 📁 scripts/                  # Build & utility scripts
│   ├── build.sh                 # Build automation
│   ├── test.sh                  # Test execution
│   └── benchmark.sh             # Performance testing
├── 📄 CMakeLists.txt            # Build configuration
├── 📄 .clang-format             # Code formatting rules
└── 📄 README.md                 # Project overview & documentation
```

## 🏗️ Namespace Structure

### Core Namespaces
- **Root**: `kcenon::logger` - Main logger namespace
- **Core functionality**: `kcenon::logger::core` - Essential logger components
- **Interfaces**: `kcenon::logger::interfaces` - Abstract base classes
- **Writers**: `kcenon::logger::writers` - Output destination implementations
- **Formatters**: `kcenon::logger::formatters` - Message formatting implementations
- **Implementation details**: `kcenon::logger::impl` - Internal implementation classes
- **Utilities**: `kcenon::logger::utils` - Helper functions and utilities

### Nested Namespaces
- `kcenon::logger::impl::async` - Asynchronous logging components
- `kcenon::logger::impl::di` - Dependency injection container
- `kcenon::logger::impl::monitoring` - Performance monitoring components

## 🔧 Key Components Overview

### 🎯 Public API Layer (`include/kcenon/logger/`)
| Component | File | Purpose |
|-----------|------|---------|
| **Main Logger** | `core/logger.h` | Primary logging interface |
| **Configuration** | `core/config.h` | Logger configuration management |
| **Builder Pattern** | `core/builder.h` | Fluent logger construction |
| **Writer Interface** | `interfaces/writer_interface.h` | Base class for all writers |
| **Formatter Interface** | `interfaces/formatter_interface.h` | Base class for all formatters |
| **File Writer** | `writers/file_writer.h` | File output implementation |
| **Console Writer** | `writers/console_writer.h` | Console output implementation |
| **JSON Formatter** | `formatters/json_formatter.h` | JSON message formatting |
| **XML Formatter** | `formatters/xml_formatter.h` | XML message formatting |

### ⚙️ Implementation Layer (`src/`)
| Component | Directory | Purpose |
|-----------|-----------|---------|
| **Async Pipeline** | `impl/async/` | Non-blocking logging operations |
| **DI Container** | `impl/di/` | Dependency injection framework |
| **Writer Implementations** | `impl/writers/` | Concrete writer classes |
| **Formatter Implementations** | `impl/formatters/` | Concrete formatter classes |
| **Filter System** | `impl/filters/` | Message filtering logic |
| **Monitoring** | `impl/monitoring/` | Performance metrics collection |

## 📊 Performance Characteristics

- **Throughput**: 4.34M+ messages/second (async mode)
- **Latency**: Sub-microsecond logging calls (async mode)
- **Memory**: Zero-copy message pipeline where possible
- **Thread Safety**: Lock-free queues for high-performance async logging

## 🔄 Migration Guide

### Step 1: Backup Current Setup
```bash
# Automatic backup of old structure
mkdir -p old_structure/
cp -r include/ old_structure/include_backup/
cp -r src/ old_structure/src_backup/
```

### Step 2: Update Include Paths
```cpp
// Old style
#include "logger/logger.h"

// New style
#include "kcenon/logger/core/logger.h"
```

### Step 3: Update Namespace Usage
```cpp
// Old style
using namespace logger;

// New style
using namespace kcenon::logger::core;
```

### Step 4: Run Migration Scripts
```bash
# Automated namespace migration
./scripts/migrate_namespaces.sh
./scripts/update_cmake.sh
```

## 🚀 Quick Start with New Structure

```cpp
#include "kcenon/logger/core/logger.h"
#include "kcenon/logger/core/builder.h"
#include "kcenon/logger/writers/file_writer.h"
#include "kcenon/logger/formatters/json_formatter.h"

int main() {
    using namespace kcenon::logger;

    // Build logger with new structure
    auto logger = core::logger_builder()
        .add_writer(std::make_shared<writers::file_writer>("app.log"))
        .set_formatter(std::make_shared<formatters::json_formatter>())
        .set_level(core::log_level::info)
        .build();

    // Use logger
    logger->info("Application started with new structure");

    return 0;
}
```
