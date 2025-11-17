# Phase 4: Validation and Deployment - Design Document

## Overview
Comprehensive validation, testing, benchmarking, security review, and production deployment preparation.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Validation Pipeline                     │
├─────────────────────────────────────────────────────────────┤
│  Unit Tests → Integration Tests → Performance Tests         │
│       ↓              ↓                    ↓                  │
│  Code Coverage → Security Scan → Benchmark Results          │
│       ↓              ↓                    ↓                  │
│  Quality Gate → Documentation → Deployment Artifacts        │
└─────────────────────────────────────────────────────────────┘
```

## Component Designs

### 1. Unit Test Framework (Task 4.1)

**Directory Structure:**

```
test/
├── unit/
│   ├── core/
│   │   ├── test_messaging_container.cpp
│   │   ├── test_message_bus.cpp
│   │   └── test_topic_router.cpp
│   ├── integration/
│   │   ├── test_network_bridge.cpp
│   │   ├── test_persistent_queue.cpp
│   │   ├── test_trace_context.cpp
│   │   └── test_config_loader.cpp
│   └── test_main.cpp
├── integration/
│   ├── test_end_to_end.cpp
│   ├── test_multi_subscriber.cpp
│   └── test_failover.cpp
├── benchmarks/
│   ├── bench_message_creation.cpp
│   ├── bench_routing.cpp
│   ├── bench_serialization.cpp
│   └── bench_network_throughput.cpp
└── CMakeLists.txt
```

**Test CMakeLists.txt:**

```cmake
# test/CMakeLists.txt

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

# ============================================================================
# Unit Tests
# ============================================================================

add_executable(messaging_tests
    unit/core/test_messaging_container.cpp
    unit/core/test_message_bus.cpp
    unit/core/test_topic_router.cpp
    unit/integration/test_network_bridge.cpp
    unit/integration/test_persistent_queue.cpp
    unit/integration/test_trace_context.cpp
    unit/integration/test_config_loader.cpp
    unit/test_main.cpp
)

target_link_libraries(messaging_tests
    PRIVATE
        messaging_system_core
        GTest::GTest
        GTest::Main
)

target_compile_definitions(messaging_tests PRIVATE
    TESTING_MODE
)

# Enable code coverage
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(messaging_tests PRIVATE
        --coverage
        -fprofile-arcs
        -ftest-coverage
    )
    target_link_options(messaging_tests PRIVATE
        --coverage
    )
endif()

# ============================================================================
# Integration Tests
# ============================================================================

add_executable(messaging_integration_tests
    integration/test_end_to_end.cpp
    integration/test_multi_subscriber.cpp
    integration/test_failover.cpp
)

target_link_libraries(messaging_integration_tests
    PRIVATE
        messaging_system_core
        GTest::GTest
        GTest::Main
)

# ============================================================================
# Benchmarks
# ============================================================================

add_executable(messaging_benchmarks
    benchmarks/bench_message_creation.cpp
    benchmarks/bench_routing.cpp
    benchmarks/bench_serialization.cpp
    benchmarks/bench_network_throughput.cpp
)

target_link_libraries(messaging_benchmarks
    PRIVATE
        messaging_system_core
        benchmark::benchmark
        benchmark::benchmark_main
)

# ============================================================================
# Test Registration
# ============================================================================

include(GoogleTest)
gtest_discover_tests(messaging_tests)
gtest_discover_tests(messaging_integration_tests)

# Add custom target for all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS messaging_tests messaging_integration_tests
    COMMENT "Running all tests"
)

# Add custom target for benchmarks
add_custom_target(run_benchmarks
    COMMAND $<TARGET_FILE:messaging_benchmarks> --benchmark_format=json --benchmark_out=benchmark_results.json
    DEPENDS messaging_benchmarks
    COMMENT "Running benchmarks"
)
```

**Example Test: MessagingContainer**

**File:** `test/unit/core/test_messaging_container.cpp`

```cpp
#include <gtest/gtest.h>
#include "messaging_system/core/messaging_container.h"

using namespace messaging;

class MessagingContainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(MessagingContainerTest, CreateValidMessage) {
    auto result = MessagingContainer::create("source1", "target1", "user.created");

    ASSERT_TRUE(result.is_ok());
    auto msg = result.value();

    EXPECT_EQ(msg.source(), "source1");
    EXPECT_EQ(msg.target(), "target1");
    EXPECT_EQ(msg.topic(), "user.created");
    EXPECT_FALSE(msg.trace_id().empty());
}

TEST_F(MessagingContainerTest, CreateInvalidMessage_EmptyTopic) {
    auto result = MessagingContainer::create("source1", "target1", "");

    ASSERT_TRUE(result.is_error());
    EXPECT_EQ(result.error().code, error::INVALID_MESSAGE);
}

TEST_F(MessagingContainerTest, SerializeAndDeserialize) {
    auto create_result = MessagingContainer::create("source1", "target1", "test.topic");
    ASSERT_TRUE(create_result.is_ok());
    auto original = create_result.value();

    // Add some data
    original.container().set_value("key1", "value1");
    original.container().set_value("key2", 42);

    // Serialize
    auto serialize_result = original.serialize();
    ASSERT_TRUE(serialize_result.is_ok());
    auto bytes = serialize_result.value();

    // Deserialize
    auto deserialize_result = MessagingContainer::deserialize(bytes);
    ASSERT_TRUE(deserialize_result.is_ok());
    auto restored = deserialize_result.value();

    EXPECT_EQ(restored.source(), original.source());
    EXPECT_EQ(restored.target(), original.target());
    EXPECT_EQ(restored.topic(), original.topic());
    EXPECT_EQ(restored.trace_id(), original.trace_id());
}

TEST_F(MessagingContainerTest, BuilderPattern) {
    auto result = MessagingContainerBuilder()
        .source("src")
        .target("tgt")
        .topic("user.login")
        .add_value("user_id", "12345")
        .add_value("timestamp", 1234567890)
        .optimize_for_speed()
        .build();

    ASSERT_TRUE(result.is_ok());
    auto msg = result.value();

    EXPECT_EQ(msg.source(), "src");
    EXPECT_EQ(msg.topic(), "user.login");
}

// Performance test
TEST_F(MessagingContainerTest, CreationPerformance) {
    const int iterations = 1000000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto result = MessagingContainer::create("src", "tgt", "topic");
        ASSERT_TRUE(result.is_ok());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    double ops_per_sec = (iterations * 1000000.0) / duration.count();

    // Target: 5M containers/sec
    EXPECT_GT(ops_per_sec, 5000000.0)
        << "Performance below target: " << ops_per_sec << " ops/sec";
}
```

### 2. Integration Tests (Task 4.1)

**File:** `test/integration/test_end_to_end.cpp`

```cpp
#include <gtest/gtest.h>
#include "messaging_system/core/message_bus.h"
#include "messaging_system/core/topic_router.h"
#include "messaging_system/integration/network_bridge.h"
#include <kcenon/thread/core/thread_pool.h>

using namespace messaging;

class EndToEndTest : public ::testing::Test {
protected:
    std::shared_ptr<thread::thread_pool> io_executor_;
    std::shared_ptr<thread::thread_pool> work_executor_;
    std::shared_ptr<TopicRouter> router_;
    std::shared_ptr<MessageBus> message_bus_;

    void SetUp() override {
        io_executor_ = std::make_shared<thread::thread_pool>(2);
        work_executor_ = std::make_shared<thread::thread_pool>(4);
        router_ = std::make_shared<TopicRouter>(work_executor_);
        message_bus_ = std::make_shared<MessageBus>(io_executor_, work_executor_, router_);

        auto start_result = message_bus_->start();
        ASSERT_TRUE(start_result.is_ok());
    }

    void TearDown() override {
        auto stop_result = message_bus_->stop();
        ASSERT_TRUE(stop_result.is_ok());
    }
};

TEST_F(EndToEndTest, PublishAndSubscribe) {
    std::atomic<int> received_count{0};
    std::string received_topic;

    // Subscribe
    auto subscribe_result = message_bus_->subscribe(
        "user.created",
        [&](const MessagingContainer& msg) -> Result<void> {
            received_count++;
            received_topic = msg.topic();
            return VoidResult::ok();
        }
    );

    ASSERT_TRUE(subscribe_result.is_ok());

    // Publish
    auto msg_result = MessagingContainer::create("test_src", "test_tgt", "user.created");
    ASSERT_TRUE(msg_result.is_ok());

    auto publish_result = message_bus_->publish_async(msg_result.value());
    ASSERT_TRUE(publish_result.is_ok());

    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(received_count.load(), 1);
    EXPECT_EQ(received_topic, "user.created");
}

TEST_F(EndToEndTest, MultipleSubscribers) {
    std::atomic<int> subscriber1_count{0};
    std::atomic<int> subscriber2_count{0};
    std::atomic<int> subscriber3_count{0};

    // Subscribe three handlers to same topic
    auto sub1 = message_bus_->subscribe("order.placed",
        [&](const MessagingContainer&) { subscriber1_count++; return VoidResult::ok(); });
    auto sub2 = message_bus_->subscribe("order.placed",
        [&](const MessagingContainer&) { subscriber2_count++; return VoidResult::ok(); });
    auto sub3 = message_bus_->subscribe("order.placed",
        [&](const MessagingContainer&) { subscriber3_count++; return VoidResult::ok(); });

    ASSERT_TRUE(sub1.is_ok() && sub2.is_ok() && sub3.is_ok());

    // Publish one message
    auto msg = MessagingContainer::create("src", "tgt", "order.placed");
    ASSERT_TRUE(msg.is_ok());

    auto publish = message_bus_->publish_async(msg.value());
    ASSERT_TRUE(publish.is_ok());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // All three subscribers should receive it
    EXPECT_EQ(subscriber1_count.load(), 1);
    EXPECT_EQ(subscriber2_count.load(), 1);
    EXPECT_EQ(subscriber3_count.load(), 1);
}

TEST_F(EndToEndTest, HighThroughput) {
    std::atomic<int> total_received{0};
    const int message_count = 100000;

    auto subscribe = message_bus_->subscribe("perf.test",
        [&](const MessagingContainer&) {
            total_received++;
            return VoidResult::ok();
        });
    ASSERT_TRUE(subscribe.is_ok());

    auto start = std::chrono::high_resolution_clock::now();

    // Publish 100K messages
    for (int i = 0; i < message_count; ++i) {
        auto msg = MessagingContainer::create("src", "tgt", "perf.test");
        ASSERT_TRUE(msg.is_ok());

        auto publish = message_bus_->publish_async(msg.value());
        ASSERT_TRUE(publish.is_ok());
    }

    // Wait for all messages to be processed
    while (total_received.load() < message_count) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double msgs_per_sec = (message_count * 1000.0) / duration.count();

    std::cout << "Throughput: " << msgs_per_sec << " msg/s" << std::endl;

    // Target: 100K+ msg/s
    EXPECT_GT(msgs_per_sec, 100000.0);
}
```

### 3. Benchmarks (Task 4.2)

**File:** `test/benchmarks/bench_routing.cpp`

```cpp
#include <benchmark/benchmark.h>
#include "messaging_system/core/topic_router.h"
#include "messaging_system/core/messaging_container.h"
#include <kcenon/thread/core/thread_pool.h>

using namespace messaging;

static void BM_TopicRouter_ExactMatch(benchmark::State& state) {
    auto executor = std::make_shared<thread::thread_pool>(4);
    auto router = std::make_shared<TopicRouter>(executor);

    // Add subscriber
    auto subscribe = router->subscribe(
        "user.created",
        [](const MessagingContainer&) { return VoidResult::ok(); }
    );

    auto msg = MessagingContainer::create("src", "tgt", "user.created").value();

    for (auto _ : state) {
        auto result = router->route(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_TopicRouter_ExactMatch);

static void BM_TopicRouter_PatternMatch(benchmark::State& state) {
    auto executor = std::make_shared<thread::thread_pool>(4);
    auto router = std::make_shared<TopicRouter>(executor);

    // Add pattern subscriber
    auto subscribe = router->subscribe(
        "user.*",
        [](const MessagingContainer&) { return VoidResult::ok(); }
    );

    auto msg = MessagingContainer::create("src", "tgt", "user.login").value();

    for (auto _ : state) {
        auto result = router->route(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_TopicRouter_PatternMatch);

static void BM_MessageCreation(benchmark::State& state) {
    for (auto _ : state) {
        auto result = MessagingContainer::create("source", "target", "topic");
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MessageCreation);

static void BM_MessageSerialization(benchmark::State& state) {
    auto msg = MessagingContainer::create("source", "target", "topic").value();
    msg.container().set_value("key1", "value1");
    msg.container().set_value("key2", 42);

    for (auto _ : state) {
        auto result = msg.serialize();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * msg.serialize().value().size());
}
BENCHMARK(BM_MessageSerialization);

BENCHMARK_MAIN();
```

### 4. Security Review (Task 4.4)

**Security Checklist Document:**

**File:** `docs/phase4/SECURITY_REVIEW.md`

```markdown
# Security Review Checklist - Phase 4

## 1. Input Validation

- [ ] All user inputs validated before processing
- [ ] Topic names validated against injection attacks
- [ ] Message size limits enforced (prevent DoS)
- [ ] Rate limiting implemented on incoming connections
- [ ] Malformed message handling tested

**Test Cases:**
```cpp
TEST(SecurityTest, RejectOversizedMessage) {
    std::vector<uint8_t> huge_payload(100 * 1024 * 1024); // 100MB
    auto result = MessagingContainer::deserialize(huge_payload);
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error().code, error::SERIALIZATION_ERROR);
}

TEST(SecurityTest, RejectInvalidTopicName) {
    auto result = MessagingContainer::create("src", "tgt", "../../../etc/passwd");
    EXPECT_TRUE(result.is_error());
}
```

## 2. Memory Safety

- [ ] No buffer overflows in serialization/deserialization
- [ ] RAII used consistently (no manual memory management)
- [ ] All Result<T> errors handled (no ignored errors)
- [ ] Thread-safe data structures verified
- [ ] ASAN/TSAN/UBSAN tests passed

**Sanitizer Tests:**
```bash
# Address Sanitizer
cmake --preset asan
cmake --build --preset asan
./build-asan/test/messaging_tests

# Thread Sanitizer
cmake --preset tsan
cmake --build --preset tsan
./build-tsan/test/messaging_tests

# Undefined Behavior Sanitizer
cmake --preset ubsan
cmake --build --preset ubsan
./build-ubsan/test/messaging_tests
```

## 3. Network Security

- [ ] TLS support tested (if enabled)
- [ ] Certificate validation enforced
- [ ] Connection limits enforced
- [ ] Timeout handling prevents resource exhaustion
- [ ] No plaintext secrets in logs

**TLS Configuration:**
```yaml
network:
  tls:
    enabled: true
    cert_file: /path/to/server.crt
    key_file: /path/to/server.key
    verify_client: true
    ca_file: /path/to/ca.crt
```

## 4. Database Security

- [ ] SQL injection prevention (parameterized queries)
- [ ] Connection string does not expose credentials
- [ ] Least privilege database user configured
- [ ] Database connection pooling limits enforced
- [ ] Prepared statements used exclusively

**ORM Security:**
```cpp
// GOOD: Parameterized query via ORM
auto result = entity_manager_->find<MessageEntity>(
    "topic = ? AND status = ?",
    {"user.created", "PENDING"}
);

// BAD: String concatenation (DO NOT USE)
// std::string query = "SELECT * FROM messages WHERE topic = '" + topic + "'";
```

## 5. Logging and Monitoring

- [ ] No sensitive data in logs (passwords, tokens, PII)
- [ ] Log injection attacks prevented
- [ ] Audit logs for critical operations
- [ ] Monitoring does not leak internal architecture
- [ ] Trace IDs do not contain sensitive data

**Safe Logging:**
```cpp
// GOOD: Safe logging
logger_->log(logger::log_level::info,
    fmt::format("[{}] User action: type={}, user_id={}",
        trace_id, action_type, hash(user_id)));

// BAD: Exposing sensitive data
// logger_->log(logger::log_level::info,
//     fmt::format("Password reset for {}", email_address));
```

## 6. Dependency Security

- [ ] All external systems from trusted sources
- [ ] Dependency versions pinned (not floating)
- [ ] Known CVEs checked (GitHub Security Advisories)
- [ ] License compliance verified
- [ ] SBOM (Software Bill of Materials) generated

**Dependency Verification:**
```cmake
FetchContent_Declare(
    CommonSystem
    GIT_REPOSITORY https://github.com/kcenon/common_system.git
    GIT_TAG v1.0.0  # Pinned version, not 'main'
    GIT_SHALLOW TRUE
)
```

## 7. Code Quality

- [ ] Static analysis passed (clang-tidy, cppcheck)
- [ ] Code coverage >80%
- [ ] All compiler warnings resolved (-Wall -Wextra -Wpedantic)
- [ ] Peer review completed
- [ ] Security-sensitive code double-reviewed

**Static Analysis:**
```bash
# clang-tidy
clang-tidy src/**/*.cpp -- -std=c++20

# cppcheck
cppcheck --enable=all --suppress=missingInclude src/
```

## Security Review Sign-off

| Reviewer | Role | Status | Date |
|----------|------|--------|------|
| [Name] | Security Engineer | [ ] Approved | YYYY-MM-DD |
| [Name] | Lead Developer | [ ] Approved | YYYY-MM-DD |
| [Name] | DevOps | [ ] Approved | YYYY-MM-DD |
```

### 5. Performance Validation (Task 4.2)

**Performance Test Script:**

**File:** `scripts/validate_performance.sh`

```bash
#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-release"
RESULTS_DIR="${PROJECT_ROOT}/benchmark_results"

mkdir -p "${RESULTS_DIR}"

echo "=== Performance Validation ==="
echo ""

# Build in release mode
echo "Building in Release mode..."
cmake --preset release
cmake --build --preset release

# Run benchmarks
echo "Running benchmarks..."
"${BUILD_DIR}/test/messaging_benchmarks" \
    --benchmark_format=json \
    --benchmark_out="${RESULTS_DIR}/benchmarks_$(date +%Y%m%d_%H%M%S).json"

# Parse results and validate against targets
echo ""
echo "=== Performance Targets Validation ==="

python3 - <<EOF
import json
import sys

with open("${RESULTS_DIR}/benchmarks_latest.json", "r") as f:
    data = json.load(f)

targets = {
    "BM_MessageCreation": 5_000_000,  # 5M ops/s
    "BM_MessageSerialization": 2_000_000,  # 2M ops/s
    "BM_TopicRouter_ExactMatch": 100_000,  # 100K ops/s
}

failed = []

for benchmark in data["benchmarks"]:
    name = benchmark["name"]
    if name in targets:
        actual_ops = 1e9 / benchmark["cpu_time"]  # ns to ops/s
        target_ops = targets[name]

        status = "✓ PASS" if actual_ops >= target_ops else "✗ FAIL"
        print(f"{status} {name}: {actual_ops:.0f} ops/s (target: {target_ops:.0f})")

        if actual_ops < target_ops:
            failed.append(name)

if failed:
    print(f"\n❌ {len(failed)} benchmark(s) failed to meet targets")
    sys.exit(1)
else:
    print("\n✅ All benchmarks meet performance targets")
    sys.exit(0)
EOF
```

### 6. Deployment Artifacts (Task 4.5)

**Production Deployment Configuration:**

**File:** `deploy/production/config.yaml`

```yaml
messaging_system:
  version: "2.0"

  network:
    port: 8080
    max_connections: 50000
    timeout_ms: 5000
    retry_attempts: 3
    tls:
      enabled: true
      cert_file: /etc/messaging/certs/server.crt
      key_file: /etc/messaging/certs/server.key
      verify_client: true
      ca_file: /etc/messaging/certs/ca.crt

  thread_pools:
    io:
      workers: 4
      queue_size: 100000
    work:
      workers: 32
      queue_size: 500000
      lockfree: true

  database:
    type: postgresql
    connection_string: "${DB_CONNECTION_STRING}"
    pool:
      min_connections: 10
      max_connections: 100
      idle_timeout_s: 60
      max_lifetime_s: 3600

  logging:
    level: info
    async: true
    writers:
      - type: rotating_file
        path: /var/log/messaging/app.log
        max_size_mb: 100
        max_files: 10
      - type: syslog
        facility: local0

  monitoring:
    enabled: true
    interval_ms: 1000
    exporters:
      - type: prometheus
        port: 9090
      - type: statsd
        host: localhost
        port: 8125
```

**Docker Configuration:**

**File:** `deploy/Dockerfile`

```dockerfile
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN cmake --preset release -DMESSAGING_USE_FETCHCONTENT=ON
RUN cmake --build --preset release
RUN ctest --preset default

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    libpq5 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/build-release/bin/messaging_system /usr/local/bin/
COPY deploy/production/config.yaml /etc/messaging/config.yaml

RUN useradd -r -s /bin/false messaging
USER messaging

EXPOSE 8080 9090

CMD ["/usr/local/bin/messaging_system", "--config", "/etc/messaging/config.yaml"]
```

**Kubernetes Deployment:**

**File:** `deploy/k8s/deployment.yaml`

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: messaging-system
  namespace: messaging
spec:
  replicas: 3
  selector:
    matchLabels:
      app: messaging-system
  template:
    metadata:
      labels:
        app: messaging-system
        version: "2.0"
    spec:
      containers:
      - name: messaging
        image: messaging-system:2.0
        ports:
        - containerPort: 8080
          name: messaging
          protocol: TCP
        - containerPort: 9090
          name: metrics
          protocol: TCP
        env:
        - name: DB_CONNECTION_STRING
          valueFrom:
            secretKeyRef:
              name: messaging-db-secret
              key: connection-string
        resources:
          requests:
            memory: "512Mi"
            cpu: "1000m"
          limits:
            memory: "2Gi"
            cpu: "4000m"
        livenessProbe:
          httpGet:
            path: /health
            port: 9090
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /ready
            port: 9090
          initialDelaySeconds: 10
          periodSeconds: 5
        volumeMounts:
        - name: config
          mountPath: /etc/messaging
          readOnly: true
        - name: certs
          mountPath: /etc/messaging/certs
          readOnly: true
      volumes:
      - name: config
        configMap:
          name: messaging-config
      - name: certs
        secret:
          secretName: messaging-tls-certs
---
apiVersion: v1
kind: Service
metadata:
  name: messaging-service
  namespace: messaging
spec:
  selector:
    app: messaging-system
  ports:
  - name: messaging
    port: 8080
    targetPort: 8080
    protocol: TCP
  - name: metrics
    port: 9090
    targetPort: 9090
    protocol: TCP
  type: LoadBalancer
```

## Testing Strategy

### Validation Phases

**Phase 4.1: Unit Testing (Week 1)**
- Write unit tests for all core components
- Achieve >80% code coverage
- All tests pass with ASAN/TSAN/UBSAN

**Phase 4.2: Integration Testing (Week 2)**
- End-to-end message flow tests
- Multi-subscriber scenarios
- Failover and recovery tests
- Performance validation (100K+ msg/s)

**Phase 4.3: Benchmarking (Week 2)**
- Message creation: 5M+ containers/sec
- Serialization: 2M+ containers/sec
- Routing: 100K+ routes/sec
- Network throughput: 100K+ msg/s end-to-end

**Phase 4.4: Security Review (Week 3)**
- Static analysis (clang-tidy, cppcheck)
- Dynamic analysis (sanitizers)
- Dependency vulnerability scan
- Code review with security focus

**Phase 4.5: Deployment Preparation (Week 3)**
- Docker image creation
- Kubernetes manifests
- Production configuration
- Rollback procedures documented

## Performance Targets

| Metric | Target | Measurement Method |
|--------|--------|--------------------|
| Message creation | 5M/s | Google Benchmark |
| Serialization | 2M/s | Google Benchmark |
| Routing | 100K/s | Google Benchmark |
| End-to-end throughput | 100K+ msg/s | Integration test |
| P95 latency | <10ms | Integration test |
| Memory usage | <500MB @ 10K conn | Integration test |
| CPU usage | <80% @ max load | Integration test |

## Success Criteria

- [ ] All unit tests pass (>80% coverage)
- [ ] All integration tests pass
- [ ] All performance benchmarks meet targets
- [ ] Security review approved
- [ ] Zero critical/high vulnerabilities
- [ ] Docker image builds successfully
- [ ] Kubernetes deployment tested
- [ ] Production configuration validated
- [ ] Rollback procedure documented and tested
- [ ] Documentation complete

## Implementation Timeline

**Week 1: Testing Infrastructure**
- Day 1-2: Set up unit test framework
- Day 3-4: Write core unit tests
- Day 5: Code coverage analysis

**Week 2: Integration and Performance**
- Day 1-2: Integration test suite
- Day 3-4: Performance benchmarks
- Day 5: Performance validation

**Week 3: Security and Deployment**
- Day 1-2: Security review and fixes
- Day 3-4: Deployment artifacts
- Day 5: Final validation and sign-off

## Rollback Plan

If Phase 4 validation fails:
1. Document failing test cases
2. Isolate root cause
3. Determine if Phase 1-3 rework needed
4. Fix issues and re-run validation
5. Do not proceed to production until all criteria met
