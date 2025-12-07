# Messaging System Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Distributed Task Queue System - Sprint 2 (Issues #101, #102, #103)**
  - `task_queue`: Task-specific queue extending message_queue
    - Multiple named queues support (queue_name -> independent queue)
    - Delayed task execution with ETA scheduling
    - Bulk enqueue operations (enqueue_bulk)
    - Tag-based task cancellation (cancel_by_tag)
    - Task registry for tracking and lookups
    - Priority-based ordering via message_queue integration
    - Thread-safe implementation with proper lifecycle management
  - `result_backend_interface`: Abstract interface for task result storage
    - Store/retrieve task state, results, errors, and progress
    - Blocking wait for results with timeout
    - Cleanup of expired task data
    - Extensible design for different backends (memory, Redis, database)
  - `memory_result_backend`: In-memory implementation of result backend
    - Thread-safe with shared_mutex for concurrent read access
    - Efficient blocking using condition_variable
    - Automatic cleanup of expired results
    - Suitable for single-process environments

- **Distributed Task Queue System - Sprint 1 (Issues #98, #99, #100)**
  - `task_builder`: Fluent builder pattern for task construction
    - Support for payload, priority, timeout, retries, queue, eta, countdown, expires, tags
    - Validation on build with Result<task> return type
  - `task_handler_interface`: Abstract interface for task execution handlers
    - Lifecycle hooks: on_retry, on_failure, on_success
    - Lambda-based simple_task_handler type
    - lambda_task_handler adapter class
    - make_handler() helper function
  - `task_context`: Execution context for task handlers
    - Progress tracking with history
    - Checkpoint save/restore for long-running task recovery
    - Subtask spawning for workflow patterns
    - Cancellation checking
    - Task-specific logging (info, warning, error)
  - Task-specific error codes (-706 to -712)
  - Comprehensive unit tests for all Sprint 1 components

### Changed
- **Logging Migration (Issue #94)**
  - Migrated from direct logger_system dependency to common_system's ILogger interface
  - Logging now uses runtime binding via GlobalLoggerRegistry
  - Removed logger_system from required CMake dependencies
  - Added comprehensive logging to core components (message_bus, message_queue,
    topic_router, backends)
  - Deprecated `has_logger_system()` in favor of `has_common_system()` in
    integration_detector

- Remove fmt library reference from CMake configuration
  - Project now uses C++20 std::format exclusively through thread_system
  - Simplifies dependency management and build configuration

### Added
- Documentation structure standardization
- Korean documentation support
- `has_common_system()` method in integration_detector for compile-time detection

---

## [1.0.0] - 2025-11-17

### Added
- **Core Messaging Infrastructure**
  - Message Bus: Central pub/sub coordinator
  - Topic Router: Wildcard pattern matching (*, #)
  - Message Queue: Thread-safe priority queues
  - Message Serialization: Container-based payloads
  - Trace Context: Distributed tracing support

- **Advanced Messaging Patterns**
  - Pub/Sub: Publisher and subscriber helpers
  - Request/Reply: Synchronous RPC over async messaging
  - Event Streaming: Event sourcing with replay
  - Message Pipeline: Pipes-and-filters processing
  - DI Container: Dependency injection support

- **Backend Support**
  - Standalone execution
  - Thread pool integration
  - Runtime backend selection
  - IExecutor abstraction
  - Mock support for testing

- **Production Quality**
  - Thread-safe operations
  - Result<T> error handling
  - 100+ unit/integration tests
  - Performance benchmarks
  - Comprehensive documentation

### Dependencies
- common_system: Result<T>, interfaces, ILogger (runtime binding)
- thread_system: Thread pool, executors
- container_system: Message serialization
- monitoring_system: Metrics collection
- network_system: Network transport

---

## [0.9.0] - 2025-10-20

### Added
- Initial message bus implementation
- Basic pub/sub pattern
- Topic routing with wildcards

### Changed
- Migrated to CMake FetchContent

---

*For detailed component changes, refer to the commit history.*
