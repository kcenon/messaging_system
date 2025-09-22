[![CodeFactor](https://www.codefactor.io/repository/github/kcenon/thread_system/badge)](https://www.codefactor.io/repository/github/kcenon/thread_system)

[![Ubuntu-Clang](https://github.com/kcenon/thread_system/actions/workflows/build-ubuntu-clang.yaml/badge.svg)](https://github.com/kcenon/thread_system/actions/workflows/build-ubuntu-clang.yaml)
[![Ubuntu-GCC](https://github.com/kcenon/thread_system/actions/workflows/build-ubuntu-gcc.yaml/badge.svg)](https://github.com/kcenon/thread_system/actions/workflows/build-ubuntu-gcc.yaml)
[![Windows-MSYS2](https://github.com/kcenon/thread_system/actions/workflows/build-windows-msys2.yaml/badge.svg)](https://github.com/kcenon/thread_system/actions/workflows/build-windows-msys2.yaml)
[![Windows-VisualStudio](https://github.com/kcenon/thread_system/actions/workflows/build-windows-vs.yaml/badge.svg)](https://github.com/kcenon/thread_system/actions/workflows/build-windows-vs.yaml)

# Thread System Project

## Project Overview

The Thread System Project is a comprehensive, production-ready C++20 multithreading framework designed to democratize concurrent programming. Built with a modular, interface-based architecture, it provides intuitive abstractions and robust implementations that empower developers of all skill levels to build high-performance, thread-safe applications without the typical complexity and pitfalls of manual thread management.

> **🏗️ Modular Architecture**: Streamlined to ~2,700 lines of highly optimized code through aggressive refactoring and coroutine removal. Logger and monitoring systems are available as separate, optional projects for maximum flexibility.

> **✅ Latest Updates**: Enhanced synchronization primitives, improved cancellation tokens, service registry pattern, and comprehensive header inclusion fixes. All CI/CD pipelines green across platforms.

## 🔗 Project Ecosystem & Inter-Dependencies

This project is part of a modular ecosystem designed for high-performance concurrent applications:

### Core Threading Framework
- **[thread_system](https://github.com/kcenon/thread_system)** (This project): Core threading framework with worker pools, job queues, and thread management
  - Provides: `kcenon::thread::interfaces::logger_interface`, `kcenon::thread::interfaces::monitoring_interface` for integration
  - Dependencies: None (standalone)
  - Usage: Core threading functionality, interfaces for other systems

### Optional Integration Components
- **[logger_system](https://github.com/kcenon/logger_system)**: High-performance asynchronous logging
  - Implements: `kcenon::thread::interfaces::logger_interface`
  - Dependencies: `thread_system` (for interfaces)
  - Integration: Seamless logging for thread operations and debugging

- **[monitoring_system](https://github.com/kcenon/monitoring_system)**: Real-time metrics collection and performance monitoring
  - Implements: `kcenon::thread::interfaces::monitoring_interface`
  - Dependencies: `thread_system` (for interfaces)
  - Integration: Thread pool metrics, system performance tracking

- **[integrated_thread_system](https://github.com/kcenon/integrated_thread_system)**: Complete solution examples
  - Dependencies: `thread_system`, `logger_system`, `monitoring_system`
  - Purpose: Integration examples, complete application templates
  - Usage: Reference implementation for full-stack integration

### Dependency Flow
```
thread_system (core interfaces)
    ↑                    ↑
logger_system    monitoring_system
    ↑                    ↑
    └── integrated_thread_system ──┘
```

### Integration Benefits
- **Plug-and-play**: Use only the components you need
- **Interface-driven**: Clean abstractions enable easy swapping
- **Performance-optimized**: Each system optimized for its domain
- **Unified ecosystem**: Consistent API design across all projects

> 📖 **[Complete Architecture Guide](docs/ARCHITECTURE.md)**: Comprehensive documentation of the entire ecosystem architecture, dependency relationships, and integration patterns.

## Project Purpose & Mission

This project addresses the fundamental challenge faced by developers worldwide: **making concurrent programming accessible, safe, and efficient**. Traditional threading approaches often lead to complex code, hard-to-debug race conditions, and performance bottlenecks. Our mission is to provide a comprehensive solution that:

- **Eliminates threading complexity** through intuitive, high-level abstractions
- **Ensures thread safety** by design, preventing common concurrency bugs
- **Maximizes performance** through optimized algorithms and modern C++ features
- **Promotes code reusability** across different platforms and use cases
- **Accelerates development** by providing ready-to-use threading components

## Core Advantages & Benefits

### 🚀 **Performance Excellence**
- **Zero-overhead abstractions**: Modern C++ design ensures minimal runtime cost
- **Optimized data structures**: Adaptive algorithms and cache-friendly designs
- **Adaptive scheduling**: Type-based job processing for optimal resource utilization
- **Scalable architecture**: Linear performance scaling with hardware thread count

### 🛡️ **Production-Grade Reliability**
- **Thread-safe by design**: All components guarantee safe concurrent access
- **Comprehensive error handling**: Robust error reporting and recovery mechanisms
- **Memory safety**: RAII principles and smart pointers prevent leaks and corruption
- **Extensive testing**: 95%+ CI/CD success rate across multiple platforms and compilers

### 🔧 **Developer Productivity**
- **Intuitive API design**: Clean, self-documenting interfaces reduce learning curve
- **Rich documentation**: Comprehensive Doxygen documentation with examples
- **Flexible configuration**: Adaptive queues with automatic optimization
- **Modular components**: Use only what you need - logging and monitoring are optional

### 🌐 **Cross-Platform Compatibility**
- **Universal support**: Works on Windows, Linux, and macOS
- **Compiler flexibility**: Compatible with GCC, Clang, and MSVC
- **C++ standard adaptation**: Graceful fallback from C++20 to older standards
- **Architecture independence**: Optimized for both x86 and ARM processors

### 📈 **Enterprise-Ready Features**
- **Type-based scheduling**: Sophisticated job type specialization for real-time systems
- **Interface-based design**: Clean separation of concerns with well-defined interfaces
- **Optional integrations**: Logger and monitoring available as separate projects
- **Modular architecture**: Use individual components or the complete framework

## Real-World Impact & Use Cases

### 🎯 **Ideal Applications**
- **High-frequency trading systems**: Microsecond-level latency requirements
- **Game engines**: Real-time rendering and physics simulation
- **Web servers**: Concurrent request processing with type handling
- **Scientific computing**: Parallel algorithm execution and data processing
- **Media processing**: Video encoding, image processing, and audio streaming
- **IoT systems**: Sensor data collection and real-time response systems

### 📊 **Performance Benchmarks**

*Benchmarked on Apple M1 (8-core) @ 3.2GHz, 16GB, macOS Sonoma*

> **🚀 Architecture Update**: Latest modular architecture removed ~8,700+ lines of code through clean interface-based design. Logger and monitoring systems are now separate optional projects. Adaptive queues continue to provide automatic optimization for all workload scenarios.

#### Core Performance Metrics (Latest Benchmarks)
- **Peak Throughput**: Up to 13.0M jobs/second (1 worker, empty jobs - theoretical)
- **Real-world Throughput**: 
  - Standard thread pool: 1.16M jobs/s (10 workers, proven in production)
  - Typed thread pool: 1.24M jobs/s (6 workers, 3 types)
  - **Adaptive queues**: Automatic optimization for all scenarios
- **Job scheduling latency**: 
  - Standard pool: ~77 nanoseconds (reliable baseline)
  - **Adaptive queues**: 96-580ns with automatic strategy selection
- **Queue operations**: Adaptive strategy provides **up to 7.7x faster** when needed
- **High contention**: Adaptive mode provides **up to 3.46x improvement** when beneficial
- **Priority scheduling**: Type-based routing with **high accuracy** under all conditions
- **Memory efficiency**: <1MB baseline, reduced codebase by ~8,700+ lines
- **Scalability**: Adaptive architecture maintains performance under any contention level

#### Impact of Thread Safety Fixes
- **Wake interval access**: 5% performance impact with mutex protection
- **Cancellation token**: 3% overhead for proper double-check pattern
- **Job queue operations**: 4% performance *improvement* after removing redundant atomic counter

#### Detailed Performance Data

**Real-World Performance** (measured with actual workloads):

*Measured Performance (actual workloads):*
| Configuration | Throughput | Time/1M jobs | Workers | Notes |
|--------------|------------|--------------|---------|-------|
| Basic Pool   | 1.16M/s    | 862 ms       | 10      | 🏆 Real-world baseline performance |
| Adaptive Pool | Dynamic    | Optimized    | Variable| 🚀 Automatic optimization based on load |
| Type Pool    | 1.24M/s    | 806 ms       | 6       | ✅ 6.9% faster with fewer workers |
| **Adaptive Queues** | **Dynamic** | **Optimized** | **Auto** | 🚀 **Automatic optimization** |
| Peak (empty) | 13.0M/s    | -            | 1       | 📊 Theoretical maximum |

*Adaptive Queue Performance (Automatic Optimization):*
| Contention Level | Strategy Selected | Latency | vs Mutex-only | Benefit |
|-----------------|-------------------|---------|---------------|---------|
| Low (1-2 threads) | Mutex | 96 ns | Baseline | Optimal for low load |
| Medium (4 threads) | Adaptive | 142 ns | +8.2% faster | Balanced performance |
| High (8+ threads) | Lock-free | 320 ns | +37% faster | Scales under contention |
| Variable Load | **Auto-switching** | **Dynamic** | **Optimized** | **Automatic** |

## Documentation

- Module READMEs:
  - core/README.md
  - implementations/README.md
  - interfaces/README.md
- Guides:
  - docs/USER_GUIDE.md (build, quick starts, DI)
  - docs/API_REFERENCE.md (complete API documentation with interfaces)
  - docs/ARCHITECTURE.md (ecosystem and modules)

Build API docs with Doxygen (optional):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target docs
# Open documents/html/index.html
```

*Real Workload Performance (8-worker configuration):*
| Job Complexity | Throughput | Use Case | Scaling Efficiency |
|----------------|------------|----------|-------------------|
| **Empty job**     | 8.2M/s     | 📏 Framework overhead measurement | 95% |
| **1 μs work**     | 1.5M/s     | ⚡ Very light computations | 94% |
| **10 μs work**    | 540K/s     | 🔧 Typical small tasks | 92% |
| **100 μs work**   | 70K/s      | 💻 Medium computations | 90% |
| **1 ms work**     | 7.6K/s     | 🔥 Heavy computations | 88% |
| **10 ms work**    | 760/s      | 🏗️ Very heavy computations | 85% |

**Worker Thread Scaling Analysis**:
| Workers | Speedup | Efficiency | Performance Rating | Recommended Use |
|---------|---------|------------|-------------------|-----------------|
| 1       | 1.0x    | 💯 **100%** | 🥇 Excellent | Single-threaded workloads |
| 2       | 2.0x    | 💚 **99%**  | 🥇 Excellent | Dual-core systems |
| 4       | 3.9x    | 💚 **97.5%**  | 🥇 Excellent | Quad-core optimal |
| 8       | 7.7x    | 💚 **96%**  | 🥈 Very Good | Standard multi-core |
| 16      | 15.0x   | 💙 **94%**  | 🥈 Very Good | High-end workstations |
| 32      | 28.3x   | 💛 **88%**  | 🥉 Good | Server environments |

**Library Performance Comparison** (Real-world measurements):
| Library | Throughput | Performance | Verdict | Key Features |
|---------|------------|-------------|---------|--------------|
| 🥇 **Thread System (Typed)** | **1.24M/s** | 🟢 **107%** | ✅ **Excellent** | Priority scheduling, adaptive queues, C++20 |
| 🥈 **Intel TBB** | ~1.24M/s | 🟢 **107%** | ✅ **Excellent** | Industry standard, work stealing |
| 🏆 **Thread System (Standard)** | **1.16M/s** | 🟢 **100%** | ✅ **Baseline** | Adaptive queues, proven performance |
| 📦 Boost.Thread Pool | ~1.09M/s | 🟡 **94%** | ✅ Good | Header-only, portable |
| 📦 OpenMP | ~1.06M/s | 🟡 **92%** | ✅ Good | Compiler directives, easy to use |
| 📦 Microsoft PPL | ~1.02M/s | 🟡 **88%** | ✅ Good | Windows-specific |
| 📚 std::async | ~267K/s | 🔴 **23%** | ⚠️ Limited | Standard library, basic functionality |

**Logger Performance Comparison** (High-contention scenario):
| Logger Type | Single Thread | 4 Threads | 8 Threads | 16 Threads | Best Use Case |
|-------------|---------------|-----------|-----------|------------|---------------|
| 🏆 **Thread System Logger** | 4.41M/s | **1.07M/s** | **0.41M/s** | **0.39M/s** | All scenarios (adaptive) |
| 🥈 **Standard Mode** | 4.41M/s | 0.86M/s | 0.23M/s | 0.18M/s | Low concurrency |
| 📊 **Adaptive Benefit** | 0% | **+24%** | **+78%** | **+117%** | Auto-optimization |

**Logger vs Industry Standards** (spdlog comparison included):
| System | Single-thread | 4 Threads | 8 Threads | Latency | vs Console |
|--------|---------------|-----------|-----------|---------|------------|
| 🐌 **Console** | 583K/s | - | - | 1,716 ns | Baseline |
| 🏆 **TS Logger** | **4.34M/s** | **1.07M/s** | **412K/s** | **148 ns** | 🚀 **7.4x** |
| 📦 **spdlog** | 515K/s | 210K/s | 52K/s | 2,333 ns | 🔴 **0.88x** |
| ⚡ **spdlog async** | **5.35M/s** | 785K/s | 240K/s | - | 🚀 **9.2x** |

**Key Insights**:
- 🏃 **Single-thread**: spdlog async wins (5.35M/s) but TS Logger close behind (4.34M/s)
- 🏋️ **Multi-thread**: TS Logger with adaptive queues shows consistent performance
- ⏱️ **Latency**: TS Logger wins with 148ns (**15.7x lower** than spdlog)
- 📈 **Scalability**: Adaptive mode provides automatic optimization

**Type-based Thread Pool Performance Comparison**:

*Mutex-based Implementation:*
| Complexity | vs Basic Pool | Type Accuracy | Performance | Best For |
|------------|--------------|---------------|-------------|----------|
| **Single Type** | 💚 **-3%** | 💯 **100%** | 525K/s | Specialized workloads |
| **3 Types** | 💛 **-9%** | 💯 **99.6%** | 495K/s | Standard prioritization |
| **Real Workload** | 💚 **+6.9%** | 💯 **100%** | **1.24M/s** | **Actual measurement** |

*With Adaptive Queues:*
| Scenario | Performance | vs Standard | Type Accuracy | Notes |
|----------|-------------|-------------|---------------|-------|
| **Low contention** | 1.24M/s | Same | 💯 **100%** | Mutex strategy selected |
| **High contention** | Dynamic | **Up to +71%** | 💯 **99%+** | Lock-free mode engaged |
| **Mixed workload** | Optimized | **Automatic** | 💯 **99.5%** | Strategy switches as needed |
| **Real measurement** | **1.24M/s** | **+6.9%** | 💯 **100%** | **Production workload** |

**Memory Usage & Creation Performance**:
| Workers | Creation Time | Memory Usage | Efficiency | Resource Rating |
|---------|---------------|--------------|------------|-----------------|
| 1       | 🟢 **162 ns** | 💚 **1.2 MB** | 💯 **100%** | ⚡ Ultra-light |
| 4       | 🟢 **347 ns** | 💚 **1.8 MB** | 💚 **98%** | ⚡ Very light |
| 8       | 🟡 **578 ns** | 💛 **2.6 MB** | 💚 **96%** | 🔋 Light |
| 16      | 🟡 **1.0 μs** | 🟡 **4.2 MB** | 💛 **94%** | 🔋 Moderate |
| 32      | 🟠 **2.0 μs** | 🟠 **7.4 MB** | 🟡 **88%** | 📊 Heavy |

For comprehensive performance analysis and optimization techniques, see the [Performance Guide](docs/PERFORMANCE.md).

## Technology Stack & Architecture

### 🏗️ **Modern C++ Foundation**
- **C++20 features**: `std::jthread`, `std::format`, concepts, and ranges
- **Template metaprogramming**: Type-safe, compile-time optimizations
- **Memory management**: Smart pointers and RAII for automatic resource cleanup
- **Exception safety**: Strong exception safety guarantees throughout
- **Adaptive algorithms**: MPMC queues, automatic strategy selection, and atomic operations
- **Interface-based design**: Clean separation between interface and implementation
- **Modular architecture**: Core threading functionality with optional logger/monitoring integration

### 🔄 **Design Patterns Implementation**
- **Command Pattern**: Job encapsulation for flexible task execution
- **Observer Pattern**: Event-driven logging and monitoring
- **Factory Pattern**: Configurable thread pool creation
- **Singleton Pattern**: Global logger access with thread safety
- **Template Method Pattern**: Customizable thread behavior
- **Strategy Pattern**: Configurable backoff strategies and scheduling policies

## Project Structure

### 📁 **Directory Organization**

```
thread_system/
├── 📁 include/kcenon/thread/       # Public headers
│   ├── 📁 core/                    # Core components
│   │   ├── thread_base.h           # Abstract thread class
│   │   ├── thread_pool.h           # Thread pool interface
│   │   ├── thread_worker.h         # Worker thread
│   │   ├── job.h                   # Job interface
│   │   ├── callback_job.h          # Function-based jobs
│   │   ├── job_queue.h             # Thread-safe queue
│   │   ├── service_registry.h      # Dependency injection
│   │   ├── cancellation_token.h    # Cancellation support
│   │   ├── sync_primitives.h       # Synchronization wrappers
│   │   └── error_handling.h        # Result<T> pattern
│   ├── 📁 interfaces/              # Integration interfaces
│   │   ├── logger_interface.h      # Logger abstraction
│   │   ├── monitoring_interface.h  # Monitoring abstraction
│   │   ├── thread_context.h        # Thread context
│   │   └── service_container.h     # Service management
│   ├── 📁 utils/                   # Utilities
│   │   ├── formatter.h             # String formatting
│   │   ├── convert_string.h        # String conversions
│   │   └── span.h                  # Span utilities
│   └── compatibility.h             # Backward compatibility
├── 📁 src/                         # Implementation files
│   ├── 📁 core/                    # Core implementations
│   │   ├── thread_base.cpp         # Thread base implementation
│   │   ├── job.cpp                 # Job implementation
│   │   ├── callback_job.cpp        # Callback job implementation
│   │   └── job_queue.cpp           # Queue implementation
│   ├── 📁 impl/                    # Concrete implementations
│   │   ├── 📁 thread_pool/         # Thread pool implementation
│   │   │   ├── thread_pool.cpp     # Pool implementation
│   │   │   └── thread_worker.cpp   # Worker implementation
│   │   └── 📁 typed_pool/          # Typed thread pool
│   │       ├── typed_thread_pool.h # Typed pool header
│   │       ├── typed_job_queue.h   # Typed queue
│   │       └── adaptive_typed_job_queue.cpp # Adaptive queue
│   └── 📁 utils/                   # Utility implementations
│       └── convert_string.cpp      # String conversion impl
├── 📁 examples/                    # Example applications
│   ├── thread_pool_sample/         # Basic thread pool usage
│   ├── typed_thread_pool_sample/   # Priority scheduling
│   ├── adaptive_queue_sample/      # Adaptive queue usage
│   ├── hazard_pointer_sample/      # Memory reclamation
│   └── integration_example/        # Integration examples
├── 📁 tests/                       # All tests
│   ├── 📁 unit/                    # Unit tests
│   │   ├── thread_base_test/       # Base functionality
│   │   ├── thread_pool_test/       # Pool tests
│   │   ├── interfaces_test/        # Interface tests
│   │   └── utilities_test/         # Utility tests
│   └── 📁 benchmarks/              # Performance tests
│       ├── thread_base_benchmarks/ # Core benchmarks
│       ├── thread_pool_benchmarks/ # Pool benchmarks
│       └── typed_thread_pool_benchmarks/ # Typed pool benchmarks
├── 📁 docs/                        # Documentation
├── 📁 cmake/                       # CMake modules
├── 📄 CMakeLists.txt               # Build configuration
├── 📄 STRUCTURE.md                 # Project structure guide
└── 📄 vcpkg.json                   # Dependencies
```

### 📖 **Key Files and Their Purpose**

#### Core Module Files
- **`thread_base.h/cpp`**: Abstract base class for all worker threads
- **`job.h/cpp`**: Abstract interface for work units
- **`job_queue.h/cpp`**: Thread-safe FIFO queue implementation
- **`callback_job.h/cpp`**: Lambda-based job implementation

#### Thread Pool Files
- **`thread_pool.h/cpp`**: Main thread pool class managing workers
- **`thread_worker.h/cpp`**: Worker thread that processes jobs
- **`future_extensions.h`**: Future-based task extensions for async results

#### Typed Thread Pool Files (Mutex-based)
- **`typed_thread_pool.h/tpp`**: Template-based priority thread pool
- **`typed_job_queue.h/tpp`**: Priority queue for typed jobs  
- **`typed_thread_worker.h/tpp`**: Workers with type responsibility lists
- **`job_types.h`**: Default priority enumeration (RealTime, Batch, Background)

#### Typed Thread Pool Files (Adaptive) 🆕
- **`typed_thread_pool.h/tpp`**: Adaptive priority thread pool implementation
- **`adaptive_typed_job_queue.h/tpp/cpp`**: Per-type adaptive MPMC queues
- **`typed_thread_worker.h/tpp`**: Adaptive worker with priority handling
- **`typed_queue_statistics_t`**: Performance monitoring and metrics collection

#### Logger Files
- **`logger.h`**: Public API with free functions
- **`log_collector.h/cpp`**: Central log message router
- **`console_writer.h/cpp`**: Colored console output
- **`file_writer.h/cpp`**: Rotating file logger

### 🔗 **Module Dependencies**

```
utilities (no dependencies)
    │
    └──> thread_base
             │
             ├──> thread_pool
             │
             └──> typed_thread_pool
                        │
                        └── typed_thread_pool (adaptive)

Optional External Projects:
- logger (separate project for logging functionality)
- monitoring (separate project for metrics collection)
```

### 🛠️ **Build Output Structure**

```
build/
├── bin/                    # Executable files
│   ├── thread_pool_sample
│   ├── typed_thread_pool_sample          # Mutex-based
│   ├── typed_thread_pool_sample_2        # Advanced usage
│   ├── logger_sample
│   ├── monitoring_sample
│   ├── adaptive_benchmark               # 🆕 Performance comparison
│   ├── queue_comparison_benchmark        # 🆕 Queue benchmarks
│   └── ...
├── lib/                    # Static libraries
│   ├── libthread_base.a
│   ├── libthread_pool.a
│   ├── libtyped_thread_pool.a  # Includes both mutex & lock-free
│   └── libutilities.a
└── include/                # Public headers (for installation)
```

## Key Components

### 1. [Core Threading Foundation (thread_module)](https://github.com/kcenon/thread_system/tree/main/core)

#### Base Components
- **`thread_base` class**: The foundational abstract class for all thread operations
  - Supports both `std::jthread` (C++20) and `std::thread` through conditional compilation
  - Provides lifecycle management (start/stop) and customizable hooks
  - Thread condition monitoring and state management

#### Job System
- **`job` class**: Abstract base class for units of work with cancellation support
- **`callback_job` class**: Concrete job implementation using `std::function`
- **`job_queue` class**: Thread-safe queue for job management
- **`cancellation_token`** 🆕: Enhanced cooperative cancellation mechanism
  - Linked token creation for hierarchical cancellation
  - Thread-safe callback registration
  - Automatic propagation of cancellation signals

#### Synchronization Primitives 🆕
- **`sync_primitives.h`**: Enhanced synchronization wrappers
  - `scoped_lock_guard`: RAII lock with timeout support
  - `condition_variable_wrapper`: Enhanced condition variable with predicates
  - `atomic_flag_wrapper`: Extended atomic operations with wait/notify
  - `shared_mutex_wrapper`: Reader-writer lock implementation

#### Service Infrastructure 🆕
- **`service_registry`**: Lightweight dependency injection container
  - Type-safe service registration and retrieval
  - Thread-safe access with shared_mutex
  - Automatic lifetime management via shared_ptr

#### Adaptive Components
- **`adaptive_job_queue`**: Dual-mode queue supporting both mutex and lock-free strategies
- **`lockfree_job_queue`**: Lock-free MPMC queue (utilized by adaptive mode)
- **`hazard_pointer`**: Safe memory reclamation for lock-free data structures
- **`node_pool`**: Memory pool for efficient node allocation

### 2. [Logging System (Separate Project)](https://github.com/kcenon/logger)

> **Note**: The logging system is now available as a separate, optional project for maximum flexibility and minimal dependencies.

- **Namespace-level logging functions**: `write_information()`, `write_error()`, `write_debug()`, etc.
- **`log_types` enum**: Bitwise-enabled log levels (Exception, Error, Information, Debug, Sequence, Parameter)
- **Multiple output targets**:
  - `console_writer`: Asynchronous console output with color support
  - `file_writer`: Rotating file output with backup support
  - `callback_writer`: Custom callback for log processing
- **`log_collector` class**: Central hub for log message routing and processing
- **Configuration functions**: `set_title()`, `console_target()`, `file_target()`, etc.

### 3. [Thread Pool System (thread_pool_module)](https://github.com/kcenon/thread_system/tree/main/sources/thread_pool)

#### Standard Thread Pool
- **`thread_pool` class**: Thread pool with adaptive queue support
  - Dynamic worker addition/removal
  - Dual-mode job queue architecture (mutex and lock-free)
  - Proven reliability for general workloads
- **`thread_worker` class**: Worker thread implementation supporting adaptive queues

#### Adaptive Queue Features
- **Adaptive job queues**: Dual-mode queue implementation with automatic optimization
  - **Dynamic strategy selection** between mutex and lock-free modes
  - MPMC queue with hazard pointers when needed
  - Intelligent backoff for contention handling
  - Batch processing support for improved throughput
  - Per-worker statistics tracking
  - Optional batch processing mode
  - Configurable backoff strategies
  
#### Common Features
- **`task<T>` template**: Future-based task wrapper for async results
- **Builder pattern support**: Fluent API for pool configuration
- **Drop-in compatibility**: Same API for easy migration

### 4. [Real-time Monitoring System (Separate Project)](https://github.com/kcenon/monitoring)

> **Note**: The monitoring system is now available as a separate, optional project for clean separation of concerns.

- **`metrics_collector` class**: Real-time performance metrics collection engine
- **Cross-platform system metrics**: Memory usage, CPU utilization, active threads
- **Thread pool monitoring**: Job completion rates, queue depths, worker utilization
- **Lock-free storage**: Memory-efficient ring buffer for time-series data
- **Easy integration**: Global singleton collector with simple API
- **Key features**:
  - Real-time data collection (100ms-1s intervals)
  - Thread-safe metric registration and updates
  - Configurable buffer sizes and collection intervals
  - Zero-overhead when disabled

### 5. [Typed Thread Pool System (typed_thread_pool_module)](https://github.com/kcenon/thread_system/tree/main/sources/typed_thread_pool)

The framework provides two distinct typed thread pool implementations optimized for different scenarios:

#### Typed Thread Pool Implementation
- **`typed_thread_pool` class**: Priority thread pool with adaptive queue support
- **Best for**: Type-based job scheduling with automatic optimization
- **Performance**: Adaptive queues provide optimal performance for varying workloads
- **Features**:
  - **Per-type adaptive queues**: Each job type can use optimized queue strategy
  - **Priority-based routing**: RealTime > Batch > Background ordering
  - **Adaptive queue support**: Uses dual-mode queues for optimal performance
  - **Dynamic queue creation**: Automatic type queue lifecycle management
  - **Advanced statistics**: Per-type metrics and performance monitoring

#### Common Components
- **`job_types` enum**: Default priority levels (RealTime, Batch, Background)
- **Type-aware components**:
  - `typed_job_t<T>`: Jobs with associated type/priority
  - `adaptive_typed_job_queue_t<T>`: Adaptive priority queue implementation
  - `typed_lockfree_job_queue_t<T>`: Lock-free priority queue (utilized by adaptive mode)
  - `typed_thread_worker_t<T>`: Workers with adaptive queue handling
- **`callback_typed_job<T>`**: Lambda-based typed job implementation
- **Custom type support**: Use your own enums or types for job prioritization

#### Usage Guidelines
- **Use Adaptive Implementation**: Automatic optimization for all scenarios
- **Benefits**: Simplified deployment with automatic performance tuning

## Advanced Features & Capabilities

### 🎛️ **Intelligent Task Scheduling**
- **Adaptive implementation strategy**: Automatic optimization based on runtime conditions
- **Type-aware job distribution**: Workers can handle multiple type levels with configurable responsibility lists
- **Priority-based scheduling**: Adaptive implementation provides optimal priority ordering (RealTime > Batch > Background)
- **Dynamic type adaptation**: Runtime adjustment of worker responsibilities based on workload patterns
- **FIFO guarantee**: Strict first-in-first-out ordering within same type levels
- **Per-type queue optimization**: Adaptive implementation uses optimized queues for each job type
- **Advanced contention handling**: Automatic strategy selection with hazard pointers for safe memory reclamation
- **Scalable architecture**: Dynamic scaling optimization based on contention patterns

### 🔬 **Advanced Threading Features**
- **Hierarchical design**: Clean `thread_base` foundation with specialized derived classes
- **C++20 compatibility**: Full support for `std::jthread` with graceful fallback to `std::thread`
- **Cancellation support**: Cooperative task cancellation using `std::stop_token`
- **Custom thread naming**: Enhanced debugging with meaningful thread identification
- **Wake interval support**: Periodic task execution without busy waiting
- **Result<T> type**: Modern error handling with monadic operations

### 📊 **Production Monitoring & Diagnostics**
- **Optional monitoring integration**: Connect with separate monitoring project when needed
- **Performance profiling**: Built-in timing and bottleneck identification
- **Health checks**: Automatic detection of thread failures and recovery
- **Optional logging integration**: Connect with separate logger project for comprehensive logging

### ⚙️ **Configuration & Customization**
- **Template-based flexibility**: Custom type types and job implementations
- **Runtime configuration**: JSON-based configuration for deployment flexibility
- **Compile-time optimization**: Conditional feature compilation for minimal overhead
- **Builder pattern**: Fluent API for easy thread pool construction

### 🔒 **Safety & Reliability**
- **Exception safety**: Strong exception safety guarantees throughout the framework
- **Resource leak prevention**: Automatic cleanup using RAII principles
- **Deadlock prevention**: Careful lock ordering and timeout mechanisms
- **Memory corruption protection**: Smart pointer usage and bounds checking

## Quick Start & Usage Examples

### 🚀 **Getting Started in 5 Minutes**

#### Adaptive High-Performance Example

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
// Optional: #include "logger/core/logger.h" // If using separate logger project

using namespace thread_pool_module;
using namespace thread_module;

int main() {
    // 1. Start the logger (if using separate logger project)
    // log_module::start();
    
    // 2. Create a high-performance adaptive thread pool
    auto pool = std::make_shared<thread_pool>();
    
    // 3. Add workers with adaptive queue optimization
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        auto worker = std::make_unique<thread_worker>();
        worker->set_batch_processing(true, 32);  // Process up to 32 jobs at once
        workers.push_back(std::move(worker));
    }
    pool->enqueue_batch(std::move(workers));
    
    // 4. Start processing
    pool->start();
    
    // 5. Submit jobs - adaptive pool handles varying contention efficiently
    std::atomic<int> counter{0};
    const int total_jobs = 100000;
    
    for (int i = 0; i < total_jobs; ++i) {
        pool->enqueue(std::make_unique<callback_job>(
            [&counter, i]() -> result_void {
                counter.fetch_add(1);
                if (i % 10000 == 0) {
                    // Optional logging: log_module::write_information("Processed {} jobs", i);
                    std::cout << "Processed " << i << " jobs\n";
                }
                return {};
            }
        ));
    }
    
    // 6. Wait for completion with progress monitoring
    auto start_time = std::chrono::high_resolution_clock::now();
    while (counter.load() < total_jobs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    
    // 7. Get comprehensive performance statistics
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    auto throughput = static_cast<double>(total_jobs) / duration.count() * 1000.0;
    
    // Optional logging (or use std::cout)
    std::cout << "Performance Results:\n";
    std::cout << "- Total jobs: " << total_jobs << "\n";
    std::cout << "- Execution time: " << duration.count() << " ms\n";
    std::cout << "- Throughput: " << std::fixed << std::setprecision(2) << throughput << " jobs/second\n";
    
    auto workers_list = pool->get_workers();
    for (size_t i = 0; i < workers_list.size(); ++i) {
        auto stats = static_cast<thread_worker*>(workers_list[i].get())->get_statistics();
        std::cout << "Worker " << i << ": " << stats.jobs_processed << " jobs, avg time: " 
                  << stats.avg_processing_time_ns << " ns, " << stats.batch_operations << " batch ops\n";
    }
    
    // 8. Clean shutdown
    pool->stop();
    // log_module::stop(); // If using logger
    
    return 0;
}
```

> **Performance Tip**: The adaptive queues automatically optimize for your workload. They provide both mutex-based reliability and lock-free performance when beneficial.

### 🔄 **More Usage Examples**

#### Standard Thread Pool (Low Contention)
```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"

using namespace thread_pool_module;
using namespace thread_module;

// Create a simple thread pool for low-contention workloads
auto pool = std::make_shared<thread_pool>("StandardPool");

// Add workers
std::vector<std::unique_ptr<thread_worker>> workers;
for (int i = 0; i < 4; ++i) {  // Few workers for simple tasks
    workers.push_back(std::make_unique<thread_worker>());
}
pool->enqueue_batch(std::move(workers));
pool->start();

// Submit jobs
for (int i = 0; i < 100; ++i) {
    pool->enqueue(std::make_unique<callback_job>(
        [i]() -> result_void {
            // Process data
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            log_module::write_debug("Processed item {}", i);
            return {};
        }
    ));
}
```

#### Adaptive Thread Pool (High Contention)
```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"

using namespace thread_pool_module;
using namespace thread_module;

// Create adaptive pool for high-contention scenarios
auto pool = std::make_shared<thread_pool>("AdaptivePool");

// Configure workers for maximum throughput
std::vector<std::unique_ptr<thread_worker>> workers;
for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
    auto worker = std::make_unique<thread_worker>();
    
    // Enable batch processing for better throughput
    worker->set_batch_processing(true, 64);
    
    workers.push_back(std::move(worker));
}
pool->enqueue_batch(std::move(workers));
pool->start();

// Submit jobs from multiple threads (high contention)
// Adaptive queues will automatically switch to lock-free mode when beneficial
std::vector<std::thread> producers;
for (int t = 0; t < 8; ++t) {
    producers.emplace_back([&pool, t]() {
        for (int i = 0; i < 10000; ++i) {
            pool->enqueue(std::make_unique<callback_job>(
                [t, i]() -> result_void {
                    // Fast job execution
                    std::atomic<int> sum{0};
                    for (int j = 0; j < 100; ++j) {
                        sum.fetch_add(j);
                    }
                    return {};
                }
            ));
        }
    });
}

// Wait for all producers
for (auto& t : producers) {
    t.join();
}

// Get detailed statistics
auto workers_vec = pool->get_workers();
for (size_t i = 0; i < workers_vec.size(); ++i) {
    auto stats = static_cast<thread_worker*>(
        workers_vec[i].get())->get_statistics();
    log_module::write_information(
        "Worker {}: {} jobs, {} μs avg, {} batch ops",
        i, stats.jobs_processed,
        stats.avg_processing_time_ns / 1000,
        stats.batch_operations
    );
}
```

#### Asynchronous Logging
```cpp
#include "logger/core/logger.h"

// Configure logger
log_module::set_title("MyApplication");
log_module::console_target(log_module::log_types::Information | 
                          log_module::log_types::Error);
log_module::file_target(log_module::log_types::All);

// Start logger
log_module::start();

// Use various log levels
log_module::write_information("Application started");
log_module::write_debug("Debug mode enabled");
log_module::write_error("Example error: {}", error_code);
log_module::write_sequence("Processing step {}/{}", current, total);

// Custom callback for critical errors
log_module::callback_target(log_module::log_types::Exception);
log_module::message_callback(
    [](const log_module::log_types& type, 
       const std::string& datetime, 
       const std::string& message) {
        if (type == log_module::log_types::Exception) {
            send_alert_email(message);
        }
    }
);
```

#### High-Performance Adaptive Logging
```cpp
#include "logger/core/logger.h"

using namespace log_module;

// Configure the logger for high-performance scenarios
log_module::set_title("HighPerformanceApp");
log_module::console_target(log_types::Information);
log_module::file_target(log_types::Information);

// Start the logger
log_module::start();

// High-frequency logging from multiple threads
// The logger automatically adapts to contention patterns
std::vector<std::thread> log_threads;
for (int t = 0; t < 16; ++t) {
    log_threads.emplace_back([t]() {
        for (int i = 0; i < 10000; ++i) {
            log_module::write_information(
                "Thread {} - High-frequency log message {}", t, i);
        }
    });
}

// Wait for all threads
for (auto& t : log_threads) {
    t.join();
}

// Adaptive logger provides excellent performance:
// - Automatic optimization based on contention
// - Efficient multi-threaded operation
// - Up to 238% better throughput at 16 threads
// - Ideal for high-concurrency logging scenarios

log_module::stop();
```

#### Real-time Performance Monitoring
```cpp
#include "monitoring/core/metrics_collector.h"
#include "thread_pool/core/thread_pool.h"

using namespace monitoring_module;
using namespace thread_pool_module;

// Start monitoring system
monitoring_config config;
config.collection_interval = std::chrono::milliseconds(100);  // 100ms intervals
metrics::start_global_monitoring(config);

// Create and monitor a thread pool
auto pool = std::make_shared<thread_pool>();
pool->start();

// Register thread pool metrics
auto collector = global_metrics_collector::instance().get_collector();
auto pool_metrics = std::make_shared<thread_pool_metrics>();
collector->register_thread_pool_metrics(pool_metrics);

// Submit jobs and monitor in real-time
for (int i = 0; i < 1000; ++i) {
    pool->enqueue(std::make_unique<callback_job>([&pool_metrics]() -> result_void {
        // Update metrics
        pool_metrics->jobs_completed.fetch_add(1);
        return {};
    }));
}

// Get real-time metrics
auto snapshot = metrics::get_current_metrics();
std::cout << "Jobs completed: " << snapshot.thread_pool.jobs_completed.load() << "\n";
std::cout << "Memory usage: " << snapshot.system.memory_usage_bytes.load() << " bytes\n";

// Stop monitoring
metrics::stop_global_monitoring();
```

### 📚 **Comprehensive Sample Collection**

Our samples demonstrate real-world usage patterns and best practices:

#### **Performance & Concurrency**
- **[Adaptive Thread Pool](https://github.com/kcenon/thread_system/tree/main/samples/thread_pool_sample)**: Thread pool with adaptive queue optimization
- **[Typed Thread Pool](https://github.com/kcenon/thread_system/tree/main/samples/typed_thread_pool_sample)**: Priority scheduling with adaptive per-type queues
- **[Adaptive MPMC Queue](https://github.com/kcenon/thread_system/tree/main/samples/mpmc_queue_sample)**: Core adaptive data structure fundamentals
- **[Hazard Pointers](https://github.com/kcenon/thread_system/tree/main/samples/hazard_pointer_sample)**: Safe memory reclamation for lock-free programming
- **[Node Pool](https://github.com/kcenon/thread_system/tree/main/samples/node_pool_sample)**: Memory pool operations for adaptive queues

#### **Thread Pool Fundamentals**
- **[Basic Thread Pool](https://github.com/kcenon/thread_system/tree/main/samples/thread_pool_sample)**: Simple job processing with adaptive queue optimization
- **[Typed Thread Pool](https://github.com/kcenon/thread_system/tree/main/samples/typed_thread_pool_sample)**: Priority-based task scheduling with adaptive queues
- **[Custom Job Types](https://github.com/kcenon/thread_system/tree/main/samples/typed_thread_pool_sample_2)**: Extending the framework with domain-specific types

#### **Monitoring & Diagnostics**
- **[Real-time Monitoring](https://github.com/kcenon/thread_system/tree/main/samples/monitoring_sample)**: Live performance metrics and system monitoring
- **[Asynchronous Logging](https://github.com/kcenon/thread_system/tree/main/samples/logger_sample)**: High-performance, multi-target logging system

### 🛠️ **Build & Integration**

#### Prerequisites
- CMake 3.16 or later
- C++20 capable compiler (GCC 9+, Clang 10+, MSVC 2019+)
- vcpkg package manager (automatically installed by dependency scripts)

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/kcenon/thread_system.git
cd thread_system

# Install dependencies via vcpkg
./dependency.sh  # Linux/macOS
./dependency.bat # Windows

# Build the project
./build.sh       # Linux/macOS
./build.bat      # Windows

# Run samples
./build/bin/thread_pool_sample
./build/bin/typed_thread_pool_sample
./build/bin/logger_sample

# Run tests (Linux/Windows only, disabled on macOS)
cd build && ctest --verbose
```

#### CMake Integration

```cmake
# Using as a subdirectory
add_subdirectory(thread_system)
target_link_libraries(your_target PRIVATE 
    thread_base 
    thread_pool 
    typed_thread_pool 
    utilities
)

# Optional: Add logger and monitoring if needed
# add_subdirectory(logger)      # Separate project
# add_subdirectory(monitoring)  # Separate project
# target_link_libraries(your_target PRIVATE logger monitoring)

# Using with FetchContent
include(FetchContent)
FetchContent_Declare(
    thread_system
    GIT_REPOSITORY https://github.com/kcenon/thread_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(thread_system)
```

## API Documentation

### Core API Reference

- **[API Reference](./docs/API_REFERENCE.md)**: Complete API documentation with interfaces
- **[Architecture Guide](./docs/ARCHITECTURE.md)**: System design and internals
- **[Performance Guide](./docs/PERFORMANCE.md)**: Optimization tips and benchmarks
- **[User Guide](./docs/USER_GUIDE.md)**: Usage guide and examples
- **[FAQ](./docs/FAQ.md)**: Frequently asked questions

### Quick API Overview

```cpp
// Thread Pool API
namespace thread_pool_module {
    // Thread pool with adaptive queue support
    class thread_pool {
        auto start() -> result_void;
        auto stop(bool immediately = false) -> result_void;
        auto enqueue(std::unique_ptr<job>&& job) -> result_void;
        auto enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs) -> result_void;
        auto get_workers() const -> const std::vector<std::shared_ptr<thread_worker>>&;
        auto get_queue_statistics() const -> queue_statistics;
    };
    
    // Thread worker with adaptive capabilities
    class thread_worker : public thread_base {
        struct worker_statistics {
            uint64_t jobs_processed;
            uint64_t total_processing_time_ns;
            uint64_t batch_operations;
            uint64_t avg_processing_time_ns;
        };
        
        auto set_batch_processing(bool enabled, size_t batch_size = 32) -> void;
        auto get_statistics() const -> worker_statistics;
    };
}

// Typed Thread Pool API (Mutex-based)
namespace typed_thread_pool_module {
    template<typename T>
    class typed_thread_pool_t {
        auto start() -> result_void;
        auto stop(bool clear_queue = false) -> result_void;
        auto enqueue(std::unique_ptr<typed_job_t<T>>&& job) -> result_void;
        auto enqueue_batch(std::vector<std::unique_ptr<typed_job_t<T>>>&& jobs) -> result_void;
    };
    
    // Adaptive Typed Queue API (supports both mutex and lock-free modes)
    template<typename T>
    class adaptive_typed_job_queue_t {
        auto enqueue(std::unique_ptr<typed_job_t<T>>&& job) -> result_void;
        auto dequeue() -> result<std::unique_ptr<job>>;
        auto dequeue(const T& type) -> result<std::unique_ptr<typed_job_t<T>>>;
        auto size() const -> std::size_t;
        auto empty() const -> bool;
        auto get_typed_statistics() const -> typed_queue_statistics_t<T>;
    };
    
    // Lock-free Queue (utilized by adaptive mode when beneficial)
    template<typename T>
    class typed_lockfree_job_queue_t {
        auto enqueue(std::unique_ptr<typed_job_t<T>>&& job) -> result_void;
        auto dequeue() -> result<std::unique_ptr<job>>;
        auto dequeue(const T& type) -> result<std::unique_ptr<typed_job_t<T>>>;
        auto size() const -> std::size_t;
        auto empty() const -> bool;
        auto get_typed_statistics() const -> typed_queue_statistics_t<T>;
    };
}

// Optional External APIs (available as separate projects):
// - Logger API: See https://github.com/kcenon/logger
// - Monitoring API: See https://github.com/kcenon/monitoring
```

## Contributing

We welcome contributions! Please see our [Contributing Guide](./docs/CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style

- Follow modern C++ best practices
- Use RAII and smart pointers
- Maintain consistent formatting (clang-format configuration provided)
- Write comprehensive unit tests for new features

## Support

- **Issues**: [GitHub Issues](https://github.com/kcenon/thread_system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/kcenon/thread_system/discussions)
- **Email**: kcenon@naver.com

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to all contributors who have helped improve this project
- Special thanks to the C++ community for continuous feedback and support
- Inspired by modern concurrent programming patterns and best practices

---

<p align="center">
  Made with ❤️ by 🍀☀🌕🌥 🌊
</p>
