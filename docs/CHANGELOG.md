# Messaging System Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed
- **Task-Message Composition Refactoring (Issue #192)**
  - Replaced `task` inheritance from `message` with composition pattern
  - Key architectural changes:
    - `task` no longer inherits from `message` class
    - `task` now directly owns `payload_` and `created_at_` members
    - Added `priority.h` header for shared `message_priority` enum
    - `task_queue` stores tasks directly instead of slicing to messages
    - Removed `task_registry_` (no longer needed for reconstruction)
  - Benefits:
    - Eliminates object slicing issue in `task_queue`
    - ~50% memory reduction (unused message fields removed)
    - Clearer architecture (task HAS-A payload, not IS-A message)
    - Fixes Liskov Substitution Principle violation
  - API changes:
    - `set_task_payload()` renamed to `set_payload()`
    - `get_task()` now returns error (use `dequeue()` instead)
    - New `has_payload()`, `priority()` accessors
  - Serialization updated to format version 3 (backward compatible with v2)

### Added
- **Dead Letter Queue for Message Broker (Issue #182)**
  - Full DLQ implementation in `message_broker`:
    - `dlq_config` structure with configurable max_size, retention_period, and overflow policies
    - `dlq_entry` structure to store failed messages with metadata and retry count
    - `dlq_statistics` for tracking DLQ metrics
    - Overflow policies: drop_oldest, drop_newest, block
  - DLQ operations:
    - `configure_dlq()`: Configure DLQ settings
    - `move_to_dlq()`: Move failed messages to DLQ
    - `get_dlq_messages()`: Query messages in DLQ
    - `get_dlq_size()`: Get current DLQ size
    - `replay_dlq_message()`: Replay a specific message
    - `replay_all_dlq_messages()`: Replay all DLQ messages
    - `purge_dlq()`: Clear all messages
    - `purge_dlq_older_than()`: Purge old messages
  - Event callbacks:
    - `on_dlq_message()`: Called when message enters DLQ
    - `on_dlq_full()`: Called when DLQ is full
  - New error codes in `error_codes.h`:
    - `dlq_full`, `dlq_empty`, `dlq_message_not_found`, `dlq_replay_failed`, `dlq_not_configured`
  - 15 comprehensive unit tests for DLQ functionality
  - Updated `docs/core/MESSAGE_BROKER.md` with DLQ documentation

- **Message Broker Documentation (Issue #184)**
  - New `docs/core/MESSAGE_BROKER.md` with comprehensive usage guide:
    - Architecture overview and component relationships
    - Quick start examples and configuration options
    - Route management and wildcard pattern reference
    - Statistics monitoring and best practices
    - Migration guide from topic_router
  - Updated `docs/API_REFERENCE.md` with Message Broker API section:
    - `broker_config`, `broker_statistics`, `route_info` struct documentation
    - `message_broker` class with all public methods
    - Usage examples and wildcard pattern table
  - Updated `docs/FEATURES_KO.md` with Korean translation of message_broker section

### Fixed
- **HTTP Transport Build Error (PR #186)**
  - Fixed `Result<http_response>` usage in `http_transport.cpp`
  - Replaced invalid `operator!` with `is_err()` method
  - Resolves build failures on macOS/clang, Ubuntu/gcc, and Windows/MSVC

### Added
- **Distributed Messaging E2E Integration Tests (Issue #165)**
  - New `test_distributed_messaging.cpp` with 17 comprehensive integration tests:
    - `TwoNodeMessageExchange`: Basic message delivery between two nodes
    - `BidirectionalCommunication`: Bidirectional message flow verification
    - `RemoteModeOnlyRemoteDelivery`: Remote-only mode behavior validation
    - `HybridModeLocalAndRemote`: Hybrid mode local and remote delivery
    - `WildcardSubscriptionAcrossNodes`: Single-level wildcard (*) subscription routing
    - `MultiLevelWildcardAcrossNodes`: Multi-level wildcard (#) subscription routing
    - `HighVolumeMessageExchange`: 500 message stress test
    - `ConcurrentPublishFromMultipleThreads`: Thread-safety under concurrent publishing
    - `MessagesDuringDisconnection`: Message handling during network failures
    - `TransportStateTransitions`: State machine transition verification
    - `DistributedStatisticsTracking`: Remote message statistics validation
    - `TransportStatisticsReset`: Statistics reset functionality
    - `PubSubPatternAcrossNodes`: Pub/sub pattern integration test
    - `MultipleSubscribersAcrossNodes`: Multiple subscriber delivery
    - `MessageContentPreservedAcrossNodes`: Message integrity across network
    - `GracefulShutdownWithPendingMessages`: Graceful shutdown behavior
    - `TransportDisconnectOnShutdown`: Transport cleanup on bus shutdown
  - `bridged_transport`: Mock transport for simulating distributed communication
    - Bidirectional message forwarding between nodes
    - Network failure simulation (disconnect/reconnect)
    - Transport statistics tracking
  - Completes Epic #165 (Network System Integration) success criteria

- **HTTP Transport Implementation (Issue #168)**
  - `http_transport`: HTTP transport adapter using `network_system::http_client`
    - PIMPL pattern with network_system integration
    - HTTP GET and POST methods for message transmission
    - Custom header management (set/remove headers)
    - Content-Type support (JSON, binary, msgpack)
    - State management with handler callbacks
    - Transport statistics tracking (messages sent/received, bytes, errors)
  - Configuration options in `http_transport_config`:
    - `base_path`: Base URL path for API endpoints
    - `content_type`: Serialization format (json, binary, msgpack)
    - `use_ssl`: Enable HTTPS
    - `default_headers`: Default HTTP headers for all requests
    - `publish_endpoint`, `subscribe_endpoint`, `request_endpoint`: Configurable endpoints
  - Comprehensive unit tests (24 tests)
  - Follows `transport_interface` contract for consistency with `websocket_transport`

### Fixed
- **Build Fix: Enable KCENON_HAS_COMMON_EXECUTOR for thread_pool IExecutor support**
  - Added `KCENON_HAS_COMMON_EXECUTOR=1` compile definition when common_system is linked
  - Fixes standalone_backend::get_executor() compilation error
  - The thread_pool class conditionally inherits from IExecutor interface only when
    this macro is defined, enabling proper shared_ptr<thread_pool> to shared_ptr<IExecutor>
    conversion

### Added
- **Enhanced Integration Tests for Multi-System Messaging (Issue #161)**
  - New `test_messaging_patterns_e2e.cpp` with messaging pattern tests:
    - `PubSubMultipleSubscribers`: Verifies 1 publisher, 3 subscribers, 100 messages with order preservation
    - `PubSubConcurrentPublishers`: Tests 3 concurrent publishers, 1 subscriber
    - `PubSubHighThroughput`: 1000 message throughput verification
    - `RequestReplyWithTimeout`: Timeout handling and cleanup verification
    - `RequestReplySequential`: 10 sequential request/reply operations
    - `RequestReplyConcurrent`: 10 concurrent request/reply operations
    - `SlowConsumerHandling`: Backpressure behavior with slow consumers
    - `TopicWildcardRouting`: Exact, single-level (*), and multi-level (#) wildcard routing
    - `UnsubscribeDuringPublish`: Safe unsubscribe during active message flow
  - New `test_timing_accuracy.cpp` with timing and scheduling tests:
    - `ScheduledTaskTimingAccuracy`: Periodic task execution timing verification
    - `TaskSubmissionLatency`: Submission-to-execution latency measurement (P50, P95, P99)
    - `TaskExecutionOrderConsistency`: FIFO order verification for same-priority tasks
    - `TaskThroughputConsistency`: Consistent throughput measurement across batches
    - `ScheduleNextExecutionTime`: Schedule entry validation
    - `TaskResultWaitTiming`: Async result timeout accuracy

### Fixed
- **MSVC C4100 Unreferenced Parameter Warnings (Issue #155)**
  - Fixed MSVC build failure caused by C4100 (unreferenced parameter) warnings being treated as errors
  - Suppressed warnings using `/* param */` syntax for unused lambda parameters in test files
  - Affected files: test_pub_sub.cpp, test_message_bus.cpp, test_topic_router.cpp, test_event_streaming.cpp, test_scheduler.cpp, bench_worker_throughput.cpp, test_message_pipeline.cpp

### Added
- **System Events Standardization with Event Bus (Issue #156)**
  - New event types in `event_bridge.h`:
    - `topic_created_event`: Published when a new topic pattern is registered
    - `subscriber_added_event`: Published when a subscriber is added to a topic
    - `subscriber_removed_event`: Published when a subscriber is removed
  - New callback mechanism in `topic_router`:
    - `topic_router_callbacks` struct for event notification
    - `set_callbacks()` method to register event listeners
    - Callbacks invoked outside mutex lock to prevent deadlocks
  - New notification methods in `messaging_event_bridge`:
    - `on_topic_created()`: Notify topic creation
    - `on_subscriber_added()`: Notify subscriber addition
    - `on_subscriber_removed()`: Notify subscriber removal
  - Integration documentation with usage examples
  - Use cases: monitoring dashboards, audit logging, auto-scaling, circuit breakers

- **Monitoring System Collector Integration (Issue #155)**
  - `message_bus_collector`: Metric collector plugin for monitoring_system integration
    - Implements `metric_collector_plugin` interface
    - Collects message throughput (messages/sec)
    - Collects message latency statistics (avg, min, max)
    - Collects queue depth and utilization metrics
    - Collects topic-level subscriber counts
    - Prometheus-compatible metric naming convention
  - `message_bus_health_monitor`: Health monitoring and anomaly detection
    - Queue saturation detection
    - Failure rate monitoring
    - Latency spike detection
    - Throughput degradation alerts
    - Configurable health thresholds
  - Collected metrics (Prometheus format):
    - `messaging_messages_published_total`
    - `messaging_messages_processed_total`
    - `messaging_messages_failed_total`
    - `messaging_queue_depth`
    - `messaging_throughput_per_second`
    - `messaging_latency_*_ms`
    - `messaging_subscribers_per_topic`
  - Conditional compilation with `KCENON_WITH_MONITORING_SYSTEM` (via feature_flags.h)
  - CMake workaround for monitoring_system alias issue (#261)
  - Documentation with usage examples

- **UnifiedBootstrapper Integration (Issue #157)**
  - `messaging_bootstrapper`: Integration module for unified system initialization
    - `integrate()`: Register messaging services with UnifiedBootstrapper
    - `remove()`: Unregister messaging services and shutdown hooks
    - Automatic shutdown hook registration for graceful cleanup
    - Auto-start capability for message bus after registration
    - Convenience getters: `get_message_bus()`, `get_event_bridge()`
  - Fluent Builder API for configuration:
    - `with_worker_threads()`: Configure worker thread count
    - `with_queue_capacity()`: Configure message queue capacity
    - `with_event_bridge()`: Enable/disable event bridge
    - `with_auto_start()`: Enable/disable auto-start on registration
    - `with_executor()`: Enable/disable executor integration
    - `with_shutdown_hook_name()`: Customize shutdown hook name
  - Benefits:
    - Unified lifecycle management across all subsystems
    - Automatic graceful shutdown coordination
    - Simplified initialization with single entry point
    - Type-safe DI container integration
  - Comprehensive unit tests (18 tests)
  - Integration documentation with usage examples

- **C++20 Concepts Support (Issue #146)**
  - Added C++20 concept definitions for type-safe callback and handler validation
  - New concepts in task module:
    - `TaskHandlerCallable`: Validates task handler callable signatures
    - `TaskHandlerLike`: Validates task handler interface implementations
    - `ScheduleEventCallable`: Validates scheduler event callbacks
  - New concepts in patterns module:
    - `MessageProcessorCallable`: Validates message pipeline processors
    - `MessageFilterCallable`: Validates message filtering predicates
    - `MessageTransformerCallable`: Validates message transformers
    - `MessageEnricherCallable`: Validates message enrichment callables
  - New concepts in core module:
    - `SubscriptionCallable`: Validates topic subscription callbacks
    - `MessageFilterCallable`: Validates message filter predicates
  - Benefits:
    - Clearer compile-time error messages for incorrect callback types
    - Self-documenting interface requirements through concepts
    - Better IDE support with improved auto-completion
    - Type-safe callback registration without runtime overhead
  - Concept-constrained template overloads added to:
    - `worker_pool::register_handler()`
    - `task_scheduler::on_task_executed()`, `on_task_failed()`
    - `pipeline_builder::add_stage()`, `add_filter()`, `add_transformer()`
    - `topic_router::subscribe()`
    - `make_task_handler()` factory function

- **Distributed Task Queue System - Sprint 7 Unit Tests (Issue #117)**
  - `test_task_queue.cpp`: Comprehensive unit tests for task_queue component
    - Queue lifecycle tests (construction, start/stop, move semantics)
    - Enqueue operations (single, bulk, named queues)
    - Dequeue operations (single queue, multiple queues, timeout handling)
    - Delayed execution tests (ETA scheduling, countdown delay)
    - Cancellation tests (by ID, by tag)
    - Query operations (get task, list queues, queue size)
    - Thread safety tests (concurrent enqueue/dequeue)
    - Priority ordering tests
    - 29 comprehensive tests
  - `test_scheduler.cpp`: Unit tests for task_scheduler component
    - Scheduler lifecycle tests (construction, start/stop, move semantics)
    - Periodic schedule tests (add, duplicate prevention, execution)
    - Cron schedule tests (add, invalid expression, duplicate prevention)
    - Schedule management tests (remove, enable/disable, trigger now)
    - Interval and cron update tests
    - Query operations (list, get, has, count)
    - Callback tests (on_task_executed, run count)
    - Disabled schedule behavior tests
    - Thread safety tests (concurrent operations)
    - 35 comprehensive tests

- **Distributed Task Queue System - Sprint 6 (Issue #113)**
  - `task_system`: Unified facade integrating all task queue components
    - Single entry point for the entire distributed task queue system
    - Lifecycle management: `start()`, `stop()`, `shutdown_graceful()`
    - Component access: `client()`, `workers()`, `scheduler()`, `monitor()`
    - Convenience methods for handler registration and task submission
    - Scheduling convenience methods: `schedule_periodic()`, `schedule_cron()`
    - Statistics and status queries: `get_statistics()`, `pending_count()`, `active_workers()`
    - Thread-safe implementation with lazy initialization
    - Comprehensive unit tests (26 tests)
  - `task_system_config`: Configuration struct combining queue, worker, scheduler, and monitor settings

- **Distributed Task Queue System - Sprint 5 (Issues #110, #111)**
  - `task_scheduler`: Scheduler for periodic and cron-based task execution
    - `add_periodic()`: Register tasks to run at fixed intervals
    - `add_cron()`: Register tasks to run based on cron expressions
    - Schedule management: `remove()`, `enable()`, `disable()`, `trigger_now()`
    - Background scheduler loop with efficient condition variable waiting
    - Event callbacks for task execution and failure notifications
    - Integration with task_client for task submission
    - Thread-safe implementation with proper lifecycle management
  - New error code: `schedule_already_exists` (-715)
  - `cron_parser`: Cron expression parser for scheduled task execution
    - Standard 5-field cron format support (minute hour day month weekday)
    - Syntax support: wildcards (*), specific values, intervals (*/n), ranges (a-b), lists (a,b,c)
    - `parse()`: Parse cron expression string to structured cron_expression
    - `next_run_time()`: Calculate next execution time from given start point
    - `is_valid()`: Validate cron expression syntax
    - `to_string()`: Convert parsed expression back to string
    - Leap year and year rollover handling
    - Thread-safe static methods
    - Comprehensive unit tests (32 tests)
  - New error codes: `task_invalid_argument` (-713), `task_operation_failed` (-714)

- **Distributed Task Queue System - Sprint 4 (Issues #107, #108, #109)**
  - `async_result`: Asynchronous result handle for task execution tracking
    - Status queries (is_ready, is_successful, is_failed, is_cancelled)
    - Progress tracking (progress, progress_message)
    - Blocking result retrieval with timeout
    - Callback-based result retrieval (then)
    - Task cancellation (revoke)
    - Child task management for workflows
    - Thread-safe implementation with comprehensive tests (25 tests)
  - `task_client`: High-level API for task submission
    - Immediate execution (send)
    - Delayed execution (send_later, send_at)
    - Batch submission (send_batch)
    - Chain pattern for sequential task execution
    - Chord pattern for parallel execution with aggregation
    - Result retrieval and cancellation APIs
    - Thread-safe concurrent task submission (24 tests)

- **Distributed Task Queue System - Sprint 3 (Issues #104, #105, #106)**
  - `worker_pool`: Thread pool for distributed task execution
    - Configurable concurrency (number of worker threads)
    - Multi-queue processing with priority ordering
    - Handler registration for task-to-handler matching
    - Graceful shutdown with timeout support
    - Statistics collection (throughput, execution time, success/failure rates)
    - Integration with task_queue, result_backend, and task_handler_interface
    - Retry mechanism with exponential backoff (Issue #105)
    - Task timeout handling with soft cancellation (Issue #106)
      - Uses std::async/std::future for timeout enforcement
      - Requests cancellation via task_context on timeout
      - Handlers can check is_cancelled() to terminate gracefully
      - Tasks transition to failed state with clear timeout error message
      - Added total_tasks_timed_out counter to worker_statistics
  - Comprehensive unit tests for worker_pool (26 tests)

### Fixed
- `task`: Sync metadata.id with task_id for correct task_queue lookup
- **DI Integration Fixes (Issue #157)**
  - Fix `message.type()` API call to use `message.metadata().type` in event_bridge.h
  - Add `worker_count()` public accessor to message_bus class
  - Add virtual destructors to `messaging_event_bridge` and `executor_message_handler`
    for ServiceInterface concept compliance (polymorphic type requirement)

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
- **Standalone Backend Thread System Migration (Issue #139)**
  - Migrated standalone_backend from internal std::thread pool to thread_system
  - Removed ~280 lines of internal thread pool implementation
  - Now uses kcenon::thread::thread_pool directly as IExecutor
  - Maintains same public API for backward compatibility
  - Ensures consistent threading behavior across the codebase

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
