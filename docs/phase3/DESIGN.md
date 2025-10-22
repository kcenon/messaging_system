# Phase 3: Infrastructure Integration - Design Document

## Overview
Integrate network, database, logging, and monitoring systems with messaging core.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Client Applications                     │
└───────────────────────────┬─────────────────────────────────┘
                            │ TCP/TLS
┌───────────────────────────┴─────────────────────────────────┐
│              MessagingNetworkBridge (Phase 3.1)             │
│  ┌────────────┐    ┌─────────────┐    ┌─────────────────┐  │
│  │ I/O Thread │───▶│ Deserialize │───▶│  Worker Threads │  │
│  └────────────┘    └─────────────┘    └────────┬────────┘  │
└──────────────────────────────────────────────────┼──────────┘
                                                   │
┌──────────────────────────────────────────────────┼──────────┐
│                   MessageBus & TopicRouter       │          │
│                         (Phase 2)                │          │
└──────────────────────────────────────────────────┼──────────┘
                                                   │
                    ┌──────────────┬───────────────┴───────┐
                    │              │                       │
┌───────────────────▼───┐  ┌───────▼────────┐  ┌──────────▼─────┐
│ PersistentMessageQueue │  │ TraceContext   │  │ ConfigLoader   │
│    (Phase 3.2)         │  │ + Logger       │  │  (Phase 3.4)   │
│                        │  │ + Monitor      │  │                │
│  - enqueue()           │  │ (Phase 3.3)    │  │  - YAML parse  │
│  - dequeue()           │  │                │  │  - Watch       │
│  - reprocess()         │  │                │  │                │
└────────────────────────┘  └────────────────┘  └────────────────┘
```

## Component Designs

### 1. Network Bridge (Task 3.1)

**Header:** `include/messaging_system/integration/network_bridge.h`

```cpp
#pragma once

#include <kcenon/network/core/messaging_server.h>
#include <kcenon/network/core/messaging_client.h>
#include <kcenon/common/interfaces/executor_interface.h>
#include "core/message_bus.h"

namespace messaging {

class MessagingNetworkBridge {
    std::shared_ptr<network::messaging_server> server_;
    std::shared_ptr<common::IExecutor> io_executor_;
    std::shared_ptr<common::IExecutor> work_executor_;
    std::shared_ptr<MessageBus> message_bus_;
    uint16_t port_;
    std::atomic<bool> running_{false};

public:
    MessagingNetworkBridge(
        uint16_t port,
        std::shared_ptr<common::IExecutor> io_executor,
        std::shared_ptr<common::IExecutor> work_executor,
        std::shared_ptr<MessageBus> message_bus
    );

    Result<void> start();
    Result<void> stop();
    bool is_running() const { return running_.load(); }

private:
    // Called on I/O thread
    Result<void> on_message_received(
        std::shared_ptr<network::messaging_session> session,
        std::vector<uint8_t> data
    );

    // Executed on worker thread
    Result<void> process_message(
        std::shared_ptr<network::messaging_session> session,
        const MessagingContainer& msg
    );

    Result<void> send_response(
        std::shared_ptr<network::messaging_session> session,
        const MessagingContainer& response
    );
};

} // namespace messaging
```

**Pipeline Flow:**
1. **Receive** (I/O thread): TCP data → buffer
2. **Deserialize** (I/O thread): buffer → MessagingContainer
3. **Offload** (I/O thread): Submit to work_executor
4. **Process** (Worker thread): Route message via MessageBus
5. **Serialize** (Worker thread): Response → buffer
6. **Send** (I/O thread): buffer → TCP

**Performance Target:** 100K+ msg/s end-to-end

### 2. Persistent Message Queue (Task 3.2)

**Header:** `include/messaging_system/integration/persistent_queue.h`

```cpp
#pragma once

#include <kcenon/database/database_manager.h>
#include <kcenon/database/orm/entity_manager.h>
#include <kcenon/common/patterns/result.h>
#include "core/messaging_container.h"

namespace messaging {

struct MessageEntity {
    int64_t id;
    std::string topic;
    std::string payload;
    std::string status;  // PENDING, PROCESSING, COMPLETED, FAILED
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    int retry_count;

    static database::entity_metadata metadata();
};

struct ConsumerOffset {
    std::string consumer_group;
    std::string topic;
    int64_t offset;
    std::chrono::system_clock::time_point updated_at;

    static database::entity_metadata metadata();
};

class PersistentMessageQueue {
    std::shared_ptr<database::connection_pool> pool_;
    std::shared_ptr<database::entity_manager> entity_manager_;

public:
    PersistentMessageQueue(
        std::shared_ptr<database::connection_pool> pool
    );

    // Queue operations
    Result<void> enqueue(const MessagingContainer& msg);
    Result<std::vector<MessageEntity>> dequeue_batch(int limit = 100);

    // Status updates
    Result<void> mark_completed(int64_t message_id);
    Result<void> mark_failed(int64_t message_id);

    // Reprocessing
    Result<void> reprocess_failed(int max_retries = 3);
    Result<size_t> count_pending() const;

    // Consumer offset management
    Result<void> save_offset(const std::string& group, const std::string& topic, int64_t offset);
    Result<int64_t> load_offset(const std::string& group, const std::string& topic);
};

} // namespace messaging
```

**Database Schema:**
```sql
CREATE TABLE messages (
    id BIGSERIAL PRIMARY KEY,
    topic VARCHAR(255) NOT NULL,
    payload TEXT NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'PENDING',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP,
    retry_count INT NOT NULL DEFAULT 0,
    INDEX idx_topic (topic),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at)
);

CREATE TABLE consumer_offsets (
    consumer_group VARCHAR(255) NOT NULL,
    topic VARCHAR(255) NOT NULL,
    offset BIGINT NOT NULL,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (consumer_group, topic)
);
```

**Performance Target:** P95 latency <50ms

### 3. Tracing and Monitoring (Task 3.3)

**Header:** `include/messaging_system/integration/trace_context.h`

```cpp
#pragma once

#include <string>
#include <random>

namespace messaging {

class TraceContext {
    static thread_local std::string current_trace_id_;

public:
    static void set_trace_id(const std::string& id);
    static std::string get_trace_id();
    static std::string generate_trace_id();
    static void clear();
};

// Usage in message handlers
class ScopedTrace {
    std::string previous_trace_id_;

public:
    explicit ScopedTrace(const std::string& trace_id);
    ~ScopedTrace();
};

} // namespace messaging
```

**Logger Integration:**

```cpp
#include <kcenon/logger/core/logger.h>

class MessageLogger {
    std::shared_ptr<logger::logger> logger_;

public:
    void log_message_received(const MessagingContainer& msg) {
        auto trace_id = TraceContext::get_trace_id();
        logger_->log(
            logger::log_level::info,
            fmt::format("[{}] Message received: topic={}, size={}",
                trace_id, msg.topic(), msg.container().serialize().size()
            )
        );
    }

    void log_message_processed(const MessagingContainer& msg, std::chrono::microseconds latency) {
        auto trace_id = TraceContext::get_trace_id();
        logger_->log(
            logger::log_level::info,
            fmt::format("[{}] Message processed: topic={}, latency={}μs",
                trace_id, msg.topic(), latency.count()
            )
        );
    }
};
```

**Monitoring Integration:**

```cpp
#include <kcenon/monitoring/core/performance_monitor.h>

class MessagingMetrics {
    std::shared_ptr<monitoring::performance_monitor> monitor_;

public:
    void record_message_sent(const std::string& topic, size_t bytes) {
        monitor_->record_metric("messages_sent_total", 1.0, {{"topic", topic}});
        monitor_->record_metric("bytes_sent_total", static_cast<double>(bytes), {{"topic", topic}});
    }

    void record_message_latency(const std::string& topic, std::chrono::microseconds latency) {
        monitor_->record_metric("message_latency_us", latency.count(), {{"topic", topic}});
    }

    void record_queue_depth(const std::string& queue, size_t depth) {
        monitor_->record_metric("queue_depth", static_cast<double>(depth), {{"queue", queue}});
    }
};
```

### 4. Configuration System (Task 3.4)

**Header:** `include/messaging_system/integration/config_loader.h`

```cpp
#pragma once

#include <yaml-cpp/yaml.h>
#include <kcenon/common/patterns/result.h>
#include <kcenon/database/connection_pool_config.h>

namespace messaging {

struct NetworkConfig {
    uint16_t port{8080};
    size_t max_connections{10000};
    std::chrono::milliseconds timeout{5000};
    int retry_attempts{3};
};

struct ThreadPoolConfig {
    size_t io_workers{2};
    size_t work_workers{16};
    size_t queue_size{100000};
    bool use_lockfree{false};
};

struct DatabaseConfig {
    std::string type{"postgresql"};
    std::string connection_string;
    database::connection_pool_config pool_config;
};

struct LoggingConfig {
    std::string level{"info"};
    bool async{true};
    std::vector<std::string> writers;
};

struct MonitoringConfig {
    bool enabled{true};
    std::chrono::milliseconds interval{1000};
};

struct MessagingSystemConfig {
    std::string version;
    NetworkConfig network;
    ThreadPoolConfig thread_pools;
    DatabaseConfig database;
    LoggingConfig logging;
    MonitoringConfig monitoring;

    static Result<MessagingSystemConfig> load_from_file(const std::string& path);
    Result<void> validate() const;
};

// File watcher for hot-reload
class ConfigWatcher {
public:
    using Callback = std::function<void(const MessagingSystemConfig&)>;

    Result<void> watch(const std::string& path, Callback callback);
    void stop();
};

} // namespace messaging
```

**YAML Schema Example:**

```yaml
messaging_system:
  version: "2.0"

  network:
    port: 8080
    max_connections: 10000
    timeout_ms: 5000
    retry_attempts: 3

  thread_pools:
    io:
      workers: 2
      queue_size: 100000
    work:
      workers: 16
      queue_size: 100000
      lockfree: true

  database:
    type: postgresql
    connection_string: "host=localhost port=5432 dbname=messaging"
    pool:
      min_connections: 5
      max_connections: 50
      idle_timeout_s: 30

  logging:
    level: info
    async: true
    writers:
      - console
      - rotating_file

  monitoring:
    enabled: true
    interval_ms: 1000
```

## Integration Points

### Startup Sequence

1. **Load Configuration**
   ```cpp
   auto config = MessagingSystemConfig::load_from_file("config.yaml");
   RETURN_IF_ERROR(config);
   ```

2. **Initialize Database**
   ```cpp
   auto pool = create_connection_pool(config.value().database);
   auto persistent_queue = std::make_shared<PersistentMessageQueue>(pool);
   ```

3. **Initialize Thread Pools**
   ```cpp
   auto io_executor = std::make_shared<thread_pool>(config.value().thread_pools.io_workers);
   auto work_executor = std::make_shared<thread_pool>(config.value().thread_pools.work_workers);
   ```

4. **Initialize Logger and Monitor**
   ```cpp
   auto logger = create_logger(config.value().logging);
   auto monitor = create_monitor(config.value().monitoring);
   ```

5. **Create MessageBus**
   ```cpp
   auto router = std::make_shared<TopicRouter>(work_executor);
   auto message_bus = std::make_shared<MessageBus>(io_executor, work_executor, router);
   ```

6. **Start Network Bridge**
   ```cpp
   auto bridge = std::make_shared<MessagingNetworkBridge>(
       config.value().network.port,
       io_executor,
       work_executor,
       message_bus
   );
   RETURN_IF_ERROR(bridge->start());
   ```

## Performance Targets

- **Network throughput:** 100K+ msg/s
- **DB persistence P95:** <50ms
- **Tracing overhead:** <1%
- **Config reload:** <100ms

## Success Criteria

- [ ] Network bridge handles 100K+ msg/s
- [ ] DB queue operations complete in <50ms (P95)
- [ ] Trace IDs propagate correctly
- [ ] Config hot-reload works
- [ ] Integration tests pass
