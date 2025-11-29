# Messaging System Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Documentation structure standardization
- Korean documentation support

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
- common_system: Result<T>, interfaces
- thread_system: Thread pool, executors
- container_system: Message serialization
- logger_system: Logging infrastructure
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
