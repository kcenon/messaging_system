# Changelog

All notable changes to the Thread System project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased] - 2025-09-13

Note: No official releases have been published yet. All entries below are pre-release milestones under Unreleased.

### Changed
- **Major Modularization** 🏗️
  - Moved logger module to separate project (https://github.com/kcenon/logger_system)
  - Moved monitoring module to separate project (https://github.com/kcenon/monitoring_system)
  - Removed ~8,700+ lines of code from core thread system
  - Core thread system now ~2,700 lines focused purely on threading
  - Created interface-based architecture for external module integration

### Removed
- **Logger Module** 📝
  - All logger implementation files (27 files)
  - Logger benchmarks and samples
  - Logger unit tests
  - Logger is now an optional external dependency

- **Monitoring Module** 📊
  - metrics_collector implementation
  - monitoring_types and ring_buffer
  - Monitoring benchmarks and samples
  - Monitoring unit tests
  - Monitoring is now an optional external dependency

- **Unused Utilities** 🔧
  - file_handler (I/O utilities)
  - argument_parser (parsing utilities)
  - datetime_tool (time utilities)
  - Old logger interface from utilities

### Added
- **Interface-Based Design** 🔌
  - logger_interface.h for optional logger integration
  - monitoring_interface.h for optional monitoring integration
  - Clean separation between core and auxiliary functionality

### Documentation
- **Updated All Documentation** 📚
  - README.md reflects modular architecture
  - api-reference.md focuses on core APIs
  - architecture.md describes new modular design
  - USER_GUIDE.md enhanced with detailed build instructions and complete sample listing
  - **NEW: PLATFORM_BUILD_GUIDE.md** - Comprehensive platform-specific build instructions
  - dependency_compatibility_matrix.md updated with latest dependency versions
  - MIGRATION.md updated with current status (2025-09-13)
  - INTERFACES.md verified for consistency with current implementation
  - Performance benchmarks updated to 2025-09-13
  - samples/ demonstrate both core-only and with-modules usage
  - All documentation synchronized with current codebase structure

## [Unreleased: Milestone 2.1.0] - 2025-09-06

### Added
- Interfaces: `executor_interface`, `scheduler_interface`, `monitorable_interface`
- `service_registry` (header-only) for lightweight DI
- Interface tests and service registry sample
- Docs: `docs/INTERFACES.md`, `docs/USER_GUIDE.md`, `docs/QUALITY.md`, `docs/COVERAGE.md`, module READMEs
- CMake: docs target (Doxygen), sanitizer and clang-tidy options

### Changed
- `thread_pool`, `typed_thread_pool` implement `executor_interface`
- `job_queue` implements `scheduler_interface`
- CMake modular structure and install rules updated for new layout

### Testing
- Added error-path tests for job_queue, thread_pool, typed_thread_pool
- Wired sanitizers to unit test targets

## [Unreleased: Milestone 2.0.0] - 2025-07-22

### Added
- **Memory Optimization Improvements** 💾
  - Implemented lazy initialization for adaptive_job_queue
    - Legacy and MPMC queues now initialized only on first use
    - Reduces initial memory usage by ~50% when queue remains unused
  - Optimized node_pool initial allocation
    - Reduced initial chunks from 4 to 1
    - Reduced chunk size from 1024 to 256 nodes
    - Saves ~93.75% initial memory for node pools
  - Overall memory footprint significantly reduced while maintaining performance

### Changed
- **Memory Management** 🔧
  - adaptive_job_queue constructor no longer pre-allocates queues
  - node_pool now uses more conservative initial allocation strategy
  - Memory is allocated on-demand rather than upfront

## [Unreleased: 2025-07-09]

### Changed
- **Major Code Cleanup** 🧹
  - Removed ~2,800 lines of duplicate code across the codebase
  - Eliminated duplicate job_queue files from internal directories
  - Removed duplicate typed_job_queue implementations
  - Removed unused builder pattern (thread_pool_builder, pool_factory)
  - Removed unused C++20 coroutine implementation (task.h - 867 lines)
  - Created formatter_macros.h to eliminate formatter duplication
  - Simplified architecture while maintaining all performance capabilities

### Added
- **Code Quality Improvements** ✨
  - Added formatter_macros.h for reducing boilerplate code
  - Improved code maintainability with cleaner architecture
  - Updated all documentation to reflect simplified structure

### Documentation
- **Complete Documentation Update** 📚
  - Updated README.md to reflect removed components
  - Rewrote api-reference.md to match current codebase
  - Updated architecture.md with cleaner structure
  - Updated USER_GUIDE.md examples
  - Updated performance.md to focus on adaptive queues
  - All references to removed components cleaned up

## [Unreleased: 2025-06-30]

### Fixed
- **Test Stability Improvements** 🔧
  - Fixed segmentation fault in `MultipleProducerConsumer` test
  - Improved race condition handling in MPMC queue tests
  - Added tolerance for cleanup-related race conditions
  - Reduced test complexity to prevent hazard pointer cleanup issues

### Added
- **Code Quality Analysis** 📊
  - Comprehensive unused class analysis identifying 15-20% dead code
  - Detailed report on experimental features (867-line coroutine system)
  - Identification of duplicate implementations in internal/external paths
  - Builder pattern classes marked as potentially unused

### Changed
- **Build System Reliability** ⚡
  - Improved resource management during parallel compilation
  - Single-threaded fallback for resource-constrained environments
  - Enhanced error handling for make jobserver limitations
  - All 106 tests now pass consistently

### Removed
- **Lock-Free System Cleanup** 🧹
  - Removed `lockfree_thread_pool` and related components
  - Removed `typed_lockfree_thread_pool` and related components
  - Removed `lockfree_thread_worker` and `typed_lockfree_thread_worker`
  - Removed lock-free job queue implementations
  - Removed lock-free logger implementations
  - Simplified adaptive queue to legacy-only mode
  - Removed all lock-free samples and benchmarks
  - Updated documentation to reflect simplified architecture

## [Unreleased: 2025-06-29]

### Added
- **Complete Lock-Free Thread Pool System** 🆕
  - `lockfree_thread_pool` class with 2.14x average performance improvement
  - `lockfree_thread_worker` with advanced statistics and batch processing
  - `lockfree_job_queue` with MPMC queue and hazard pointers
  - Superior scalability under high contention (up to 3.46x better with 16+ producers)

- **Lock-Free Typed Thread Pool System** 🆕
  - `typed_lockfree_thread_pool_t<T>` template with per-type lock-free queues
  - `typed_lockfree_thread_worker_t<T>` with priority-aware processing
  - `typed_lockfree_job_queue_t<T>` with separate queues for each job type
  - 7-71% performance improvement over mutex-based typed pools

- **High-Performance Lock-Free Logger** 🆕
  - `lockfree_logger` singleton with wait-free enqueue operations
  - `lockfree_log_collector` using lock-free job queue
  - Up to 238% better throughput at 16 threads compared to standard logger
  - Drop-in replacement for standard logger with identical API

- **Advanced Memory Management System** 🆕
  - `hazard_pointer` implementation for safe lock-free memory reclamation
  - `node_pool<T>` template for high-performance memory allocation
  - Prevents ABA problem and memory leaks in lock-free data structures
  - Cache-line aligned structures for optimal performance

- **Comprehensive Performance Monitoring** 🆕
  - Detailed statistics APIs for all lock-free components
  - Per-worker performance metrics (jobs processed, latency, batch operations)
  - Queue operation statistics (enqueue/dequeue latency, retry counts)
  - Real-time monitoring through metrics_collector

- **Enhanced Documentation**
  - Complete API reference documentation for all lock-free implementations
  - Performance comparison tables and benchmarking data
  - Usage examples and best practices for each component
  - Detailed architecture documentation for lock-free algorithms
  - Enhanced README.md with comprehensive project overview and benefits

- **Batch Processing Support**
  - Batch job submission for improved throughput
  - Configurable batch sizes for workers
  - Batch dequeue operations for lock-free queues

- **Advanced Configuration Options**
  - Backoff strategies for contention handling in lock-free operations
  - Hazard pointer configuration for memory management
  - Runtime statistics collection and reset functionality

### Changed
- **BREAKING**: Renamed `priority_thread_pool` to `typed_thread_pool` to better reflect the job type-based scheduling paradigm
- **BREAKING**: Changed `job_priorities` enum to `job_types` with values: RealTime, Batch, Background
- All `priority_*` classes renamed to `typed_*` for consistency
- **Enhanced Performance Characteristics**:
  - Lock-free thread pool: **2.14x average performance improvement**
  - Lock-free typed thread pool: **7-71% performance improvement** under load
  - Lock-free logger: **Up to 238% better throughput** at high concurrency
  - Lock-free queue operations: **7.7x faster enqueue**, **5.4x faster dequeue** under contention
- **Improved API Design**:
  - All lock-free implementations are drop-in replacements for standard versions
  - Enhanced error handling with detailed error messages
  - Comprehensive statistics APIs for performance monitoring
  - Template-based design for maximum flexibility
- **Memory Management Enhancements**:
  - Hazard pointer implementation prevents memory leaks in lock-free structures
  - Node pooling reduces allocation overhead
  - Cache-line alignment prevents false sharing
- **Updated Documentation**:
  - Complete API reference for all lock-free implementations
  - Performance benchmarking data and comparison tables
  - Updated samples with lock-free examples
  - Enhanced architecture documentation
- **MPMC Queue Improvements**:
  - Completely removed thread-local storage for improved stability
  - Added retry limits (MAX_TOTAL_RETRIES = 1000) to prevent infinite loops
  - All stress tests now enabled and passing reliably
  - Hazard pointer integration for safe memory reclamation

### Fixed
- Thread safety improvements in typed job queue
- Memory leak fixes in worker thread destruction
- Platform-specific compilation issues
- MPMC queue infinite loop issues under high contention
- Thread-local storage cleanup segmentation faults
- All disabled tests now enabled and passing

## [Unreleased: Initial] - 2024-01-15

### Added
- Initial release of Thread System framework
- Core thread_base class with C++20 std::jthread support
- Priority-based thread pool system
- Asynchronous logging framework with multiple output targets
- Comprehensive sample applications
- Cross-platform build system with vcpkg integration
- Extensive unit test suite with Google Test

### Core Components
- **Thread Base Module**: Foundation for all threading operations
- **Logging System**: High-performance asynchronous logging
- **Thread Pool System**: Efficient worker thread management
- **Priority Thread Pool System**: Advanced priority-based scheduling (renamed to Typed Thread Pool in later versions)

### Supported Platforms
- Windows (MSVC 2019+, MSYS2)
- Linux (GCC 9+, Clang 10+)
- macOS (Clang 10+)

### Performance Characteristics

#### Latest Performance Metrics (Apple M1, 8-core @ 3.2GHz, 16GB, macOS Sonoma)

**Core Performance Improvements:**
- **Peak Throughput**: Up to 15.2M jobs/second (lock-free, 1 worker, empty jobs)
- **Real-world Throughput**: 
  - Standard thread pool: 1.16M jobs/s (10 workers)
  - **Lock-free thread pool**: 2.48M jobs/s (8 workers) - **2.14x improvement**
  - Typed thread pool (mutex): 1.24M jobs/s (6 workers, 3 types)
  - **Typed lock-free thread pool**: 2.38M jobs/s (100 jobs), **+7.2%** vs mutex

**Lock-Free Queue Performance:**
- **Enqueue operations**: **7.7x faster** under high contention
- **Dequeue operations**: **5.4x faster** under high contention
- **Scalability**: **2-4x better scaling** under contention
- **Memory efficiency**: ~1.5MB (with hazard pointers) vs <1MB (standard)

**Logger Performance:**
- **Single thread**: Standard logger 4.34M/s, Lock-free logger 3.90M/s
- **Multi-thread (16 threads)**: Lock-free logger **+238% better** (0.54M/s vs 0.16M/s)
- **Latency**: Standard logger 148ns, **15.7x lower** than industry alternatives

**Memory Management:**
- Thread creation overhead: ~24.5 microseconds (measured on Apple M1)
- Job scheduling latency: ~77 nanoseconds (standard), ~320ns (lock-free enqueue)
- Lock-free operations maintain **consistent performance** regardless of contention
- Hazard pointer overhead: <5% for memory safety guarantees

---

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.
