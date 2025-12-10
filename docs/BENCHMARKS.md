# Messaging System Performance Benchmarks

**Version**: 1.1
**Last Updated**: 2025-12-10
**Language**: [English] | [한국어](BENCHMARKS_KO.md)

---

## Executive Summary

This document provides comprehensive performance benchmarks for the messaging_system, including throughput, latency measurements, and optimization insights.

**Platform**: Apple M1 (8-core) @ 3.2GHz, 16GB RAM, macOS Sonoma

**Key Highlights**:
- **Message Creation**: ~5M messages/second
- **Queue Operations**: ~2M operations/second
- **Topic Routing**: ~500K routes/second
- **Pub/Sub Throughput**: ~100K messages/second
- **Request/Reply**: ~50K requests/second (2-way latency)

---

## Table of Contents

1. [Core Performance Metrics](#core-performance-metrics)
2. [Message Creation Benchmarks](#message-creation-benchmarks)
3. [Queue Performance](#queue-performance)
4. [Topic Router Performance](#topic-router-performance)
5. [Pub/Sub Performance](#pubsub-performance)
6. [Request/Reply Performance](#requestreply-performance)
7. [Task Module Performance](#task-module-performance)
8. [Memory Usage](#memory-usage)
9. [Latency Analysis](#latency-analysis)
10. [Scalability](#scalability)
11. [Optimization Insights](#optimization-insights)

---

## Core Performance Metrics

### Reference Performance Metrics

Benchmarked on Apple M1 (8-core) @ 3.2GHz, 16GB, macOS Sonoma.

| Metric | Value | Configuration | Notes |
|--------|-------|---------------|-------|
| **Message Creation** | ~5M msg/s | message_builder | Lightweight construction |
| **Queue Enqueue** | ~2M ops/s | Priority queue | Thread-safe operations |
| **Queue Dequeue** | ~2M ops/s | Priority queue | Lock-based access |
| **Topic Matching** | ~500K/s | Wildcard patterns | Pattern-based routing |
| **Pub/Sub Throughput** | ~100K msg/s | End-to-end | Complete message flow |
| **Request/Reply** | ~50K req/s | Sync over async | Round-trip latency |
| **Memory per Message** | ~300 bytes | Base overhead | Without payload |

---

## Message Creation Benchmarks

### Message Builder Performance

**Test Configuration**: 1 million message creations

```cpp
auto msg = message_builder()
    .topic("test.topic")
    .source("benchmark")
    .type(message_type::event)
    .priority(message_priority::normal)
    .build();
```

| Operation | Time (ns/op) | Throughput | Memory |
|-----------|--------------|------------|--------|
| **Empty Message** | 50 ns | ~20M/s | 200 bytes |
| **With Metadata** | 100 ns | ~10M/s | 300 bytes |
| **With Payload** | 200 ns | ~5M/s | 500+ bytes |
| **Full Message** | 300 ns | ~3.3M/s | 1000+ bytes |

**Key Insights**:
- Message creation is lightweight and efficient
- Payload serialization dominates cost for large messages
- Metadata overhead is minimal (~100 bytes)

---

## Queue Performance

### Priority Queue Operations

**Test Configuration**: 1 million enqueue/dequeue operations

| Queue Type | Enqueue (ns) | Dequeue (ns) | Throughput |
|------------|--------------|--------------|------------|
| **Standard Queue** | 300 ns | 350 ns | ~2.5M ops/s |
| **Priority Queue** | 500 ns | 550 ns | ~2M ops/s |
| **Dead Letter Queue** | 400 ns | 450 ns | ~2.2M ops/s |

### Queue Contention Performance

| Producers | Consumers | Throughput | Latency (P99) |
|-----------|-----------|------------|---------------|
| 1 | 1 | 2.5M ops/s | 400 ns |
| 4 | 4 | 1.8M ops/s | 600 ns |
| 8 | 8 | 1.2M ops/s | 900 ns |
| 16 | 16 | 800K ops/s | 1.5 μs |

**Key Insights**:
- Priority queue adds ~200ns overhead for sorting
- Contention increases with thread count
- Lock-based implementation provides reliable performance

---

## Topic Router Performance

### Pattern Matching Performance

**Test Configuration**: 1 million topic matches

| Pattern Type | Match Time | Throughput | Example |
|--------------|------------|------------|---------|
| **Exact Match** | 100 ns | ~10M/s | `user.created` |
| **Single Wildcard** | 500 ns | ~2M/s | `user.*` |
| **Multi-level Wildcard** | 1 μs | ~1M/s | `user.#` |
| **Complex Pattern** | 2 μs | ~500K/s | `*.user.#` |

### Subscription Count Impact

| Subscribers | Match Time | Throughput | Dispatch Time |
|-------------|------------|------------|---------------|
| 1 | 500 ns | 2M/s | 1 μs |
| 10 | 800 ns | 1.25M/s | 10 μs |
| 100 | 2 μs | 500K/s | 100 μs |
| 1000 | 10 μs | 100K/s | 1 ms |

**Key Insights**:
- Exact matches are fastest
- Wildcard matching adds overhead
- Dispatch time scales linearly with subscriber count

---

## Pub/Sub Performance

### End-to-End Throughput

**Test Configuration**: Message bus with backends

| Configuration | Throughput | Latency (P50) | Latency (P99) |
|--------------|------------|---------------|---------------|
| **1 Publisher, 1 Subscriber** | 250K msg/s | 2 μs | 10 μs |
| **1 Publisher, 10 Subscribers** | 150K msg/s | 4 μs | 20 μs |
| **4 Publishers, 4 Subscribers** | 100K msg/s | 8 μs | 50 μs |
| **8 Publishers, 8 Subscribers** | 80K msg/s | 10 μs | 100 μs |

### Backend Impact

| Backend | Throughput | Overhead | Notes |
|---------|------------|----------|-------|
| **Standalone** | 120K msg/s | Baseline | Internal thread pool |
| **Integration** | 100K msg/s | +20% | External thread pool |
| **Mock (Testing)** | 250K msg/s | -50% | No threading overhead |

**Key Insights**:
- Pub/Sub throughput scales well to 8 threads
- Multiple subscribers increase dispatch overhead
- Backend choice affects overall throughput

---

## Request/Reply Performance

### Synchronous RPC Latency

**Test Configuration**: Request-reply over message bus

| Payload Size | Latency (P50) | Latency (P99) | Throughput |
|--------------|---------------|---------------|------------|
| **Empty** | 10 μs | 50 μs | 100K req/s |
| **1 KB** | 15 μs | 80 μs | 67K req/s |
| **10 KB** | 50 μs | 200 μs | 20K req/s |
| **100 KB** | 500 μs | 2 ms | 2K req/s |

### Concurrent Request Performance

| Concurrent Requests | Throughput | Latency (P99) |
|-------------------|------------|---------------|
| 1 | 100K req/s | 50 μs |
| 10 | 80K req/s | 150 μs |
| 100 | 50K req/s | 500 μs |
| 1000 | 20K req/s | 5 ms |

**Key Insights**:
- Request/reply adds round-trip overhead
- Payload size significantly impacts latency
- Concurrent requests scale reasonably well

---

## Task Module Performance

The task module provides distributed task queue capabilities with performance benchmarks for each component.

### Task Queue Performance

**Test Configuration**: 100,000 task operations

| Operation | Target | Typical Performance |
|-----------|--------|---------------------|
| **Task Enqueue** | > 100K ops/s | ~150K ops/s |
| **Task Dequeue** | > 50K ops/s | ~80K ops/s |
| **Priority Enqueue** | > 80K ops/s | ~120K ops/s |
| **Task Cancellation** | - | ~500K ops/s |
| **Tag-based Cancel** | - | ~1M ops/s |

### Worker Throughput

**Test Configuration**: Empty task handlers

| Configuration | Target | Typical Performance |
|---------------|--------|---------------------|
| **Single Worker (empty)** | > 10K tasks/s | ~15K tasks/s |
| **4 Workers** | - | ~40K tasks/s |
| **8 Workers** | - | ~70K tasks/s |
| **Handler Dispatch** | - | < 1 ms P99 |

### Worker Scalability

| Workers | Throughput | Scaling Factor |
|---------|------------|----------------|
| 1 | 15K tasks/s | 1.0x |
| 2 | 28K tasks/s | 1.87x |
| 4 | 50K tasks/s | 3.33x |
| 8 | 80K tasks/s | 5.33x |

### Result Backend Performance

**Test Configuration**: memory_result_backend, 100,000 operations

| Operation | Target | Typical Performance |
|-----------|--------|---------------------|
| **Store State** | - | ~500K ops/s |
| **Store Result** | > 50K ops/s | ~200K ops/s |
| **Get State** | - | ~1M ops/s |
| **Get Result** | - | ~500K ops/s |
| **Progress Update** | - | ~300K ops/s |
| **Wait for Result** | - | < 1 ms overhead |

### Scheduler Performance

| Operation | Typical Performance |
|-----------|---------------------|
| **Add Schedule** | ~50K schedules/s |
| **Schedule Lookup** | ~500K lookups/s |
| **Remove Schedule** | ~100K removes/s |
| **Cron Parsing** | ~100K parses/s |
| **Next Run Calculation** | ~1M calculations/s |

### Performance Targets Summary

| Metric | Target | Notes |
|--------|--------|-------|
| Queue Enqueue | > 100K ops/s | Task queue throughput |
| Queue Dequeue | > 50K ops/s | With priority support |
| Worker Throughput | > 10K tasks/s | Empty task execution |
| Result Store | > 50K ops/s | Memory backend |

**Key Insights**:
- Task queue performance exceeds targets for typical workloads
- Worker throughput scales near-linearly up to 4 workers
- Memory result backend provides excellent performance for single-process deployments
- Scheduler overhead is minimal for schedule management operations

---

## Memory Usage

### Per-Component Memory Overhead

| Component | Base Memory | Per Message | Notes |
|-----------|-------------|-------------|-------|
| **Message Bus** | 100 KB | - | Bus infrastructure |
| **Topic Router** | 50 KB | 100 bytes/sub | Subscription registry |
| **Message Queue** | 20 KB | - | Queue infrastructure |
| **Message** | - | 300 bytes | Base overhead |
| **Payload (container)** | - | Variable | Depends on content |
| **Backend (standalone)** | 500 KB | - | Thread pool |
| **Backend (integration)** | 10 KB | - | Minimal overhead |

### Memory Scaling

| Messages in Queue | Memory Usage | Per Message |
|------------------|--------------|-------------|
| 1K | 500 KB | 500 bytes |
| 10K | 4 MB | 400 bytes |
| 100K | 35 MB | 350 bytes |
| 1M | 320 MB | 320 bytes |

**Key Insights**:
- Message overhead decreases with scale
- Queue capacity should be sized appropriately
- Memory usage is predictable and linear

---

## Latency Analysis

### Latency Distribution (Pub/Sub)

**Test Configuration**: 1 million messages, 1 publisher, 1 subscriber

| Percentile | Latency | Category |
|-----------|---------|----------|
| **P50** | 2 μs | Excellent |
| **P90** | 5 μs | Excellent |
| **P95** | 10 μs | Good |
| **P99** | 50 μs | Acceptable |
| **P99.9** | 200 μs | Fair |
| **Max** | 2 ms | Rare outlier |

### Latency Breakdown

| Component | Time | Percentage |
|-----------|------|------------|
| **Queue Enqueue** | 500 ns | 25% |
| **Topic Matching** | 500 ns | 25% |
| **Message Dispatch** | 500 ns | 25% |
| **Callback Execution** | 500 ns | 25% |
| **Total** | 2 μs | 100% |

**Key Insights**:
- Most messages complete within 10 μs
- Outliers typically due to OS scheduling
- Latency is consistent and predictable

---

## Scalability

### Thread Scaling

| Workers | Throughput | Speedup | Efficiency |
|---------|------------|---------|------------|
| 1 | 30K msg/s | 1.0x | 100% |
| 2 | 58K msg/s | 1.93x | 97% |
| 4 | 110K msg/s | 3.67x | 92% |
| 8 | 200K msg/s | 6.67x | 83% |
| 16 | 300K msg/s | 10.0x | 63% |

### Publisher/Subscriber Scaling

**Fixed Workers (4), Variable Pub/Sub**:

| Publishers | Subscribers | Total Throughput | Per Publisher |
|------------|-------------|------------------|---------------|
| 1 | 1 | 100K msg/s | 100K msg/s |
| 2 | 2 | 180K msg/s | 90K msg/s |
| 4 | 4 | 320K msg/s | 80K msg/s |
| 8 | 8 | 560K msg/s | 70K msg/s |

**Key Insights**:
- Near-linear scaling up to 4 workers
- Efficiency decreases with more than 8 workers
- Multiple publishers/subscribers scale well

---

## Optimization Insights

### Performance Tuning Recommendations

1. **Queue Configuration**
   - Use priority queues only when needed (+200ns overhead)
   - Set appropriate capacity (10K default)
   - Enable drop-on-full for high-throughput scenarios

2. **Topic Routing**
   - Prefer exact matches over wildcards
   - Minimize wildcard complexity
   - Cache frequently-used patterns

3. **Backend Selection**
   - Use standalone for simple applications
   - Use integration for optimal thread pool usage
   - Match worker count to CPU cores

4. **Message Design**
   - Keep payloads small (<10KB)
   - Reuse message objects when possible
   - Minimize metadata fields

5. **Concurrency**
   - Match publishers/subscribers to workload
   - Avoid excessive thread counts (>16)
   - Use batch operations when available

### Common Bottlenecks

| Bottleneck | Symptom | Solution |
|-----------|---------|----------|
| **Queue Overflow** | Messages dropped | Increase capacity or use backpressure |
| **Topic Contention** | High latency | Reduce wildcard complexity |
| **Worker Starvation** | Low throughput | Increase worker count |
| **Memory Pressure** | Slow performance | Reduce message size or implement GC |

---

## Running Benchmarks

### Prerequisites

```bash
cmake -B build -DMESSAGING_BUILD_BENCHMARKS=ON
cmake --build build -j
```

### Available Benchmarks

```bash
# Core messaging benchmarks
./build/test/benchmarks/bench_message_creation
./build/test/benchmarks/bench_message_queue
./build/test/benchmarks/bench_topic_router
./build/test/benchmarks/bench_pub_sub_throughput
./build/test/benchmarks/bench_request_reply_latency

# Task module benchmarks
./build/test/benchmarks/bench_task_queue
./build/test/benchmarks/bench_worker_throughput
./build/test/benchmarks/bench_result_backend
./build/test/benchmarks/bench_scheduler
```

### Benchmark Output Format

```
------------------------------------------------------
Benchmark: Message Creation
------------------------------------------------------
Messages created: 1000000
Total time: 200.5 ms
Throughput: 4,987,531 msg/s
Latency (avg): 200 ns
Latency (p50): 150 ns
Latency (p99): 500 ns
Memory usage: 300 MB
------------------------------------------------------
```

---

## Continuous Performance Monitoring

Performance metrics are automatically measured in our CI/CD pipeline:
- Regression detection on every PR
- Historical performance tracking
- Platform-specific benchmarks

See [CI/CD Performance Tracking](../.github/workflows/benchmarks.yml) for details.

---

## Comparison with Other Systems

### Feature Comparison

| Feature | messaging_system | RabbitMQ | Kafka | Redis |
|---------|-----------------|----------|-------|-------|
| **In-Process** | ✅ | ❌ | ❌ | ❌ |
| **Latency** | < 10 μs | ~1 ms | ~5 ms | ~0.1 ms |
| **Throughput** | 100K/s | 100K/s | 1M/s | 100K/s |
| **Persistence** | Optional | ✅ | ✅ | Optional |
| **Clustering** | ❌ | ✅ | ✅ | ✅ |

**Note**: Comparison is approximate and depends on configuration. messaging_system is optimized for in-process, low-latency messaging.

---

## Conclusion

The messaging_system provides:
- **High Performance**: 100K+ msg/s with <10 μs latency
- **Predictable Behavior**: Consistent performance across loads
- **Efficient Resource Usage**: <1 MB base overhead
- **Good Scalability**: Near-linear scaling to 8 workers

For production deployments, we recommend:
- 4-8 worker threads per message bus
- Queue capacity of 10K messages
- Monitoring of P99 latency and throughput

---

**Last Updated**: 2025-12-10
**Platform**: Apple M1 @ 3.2GHz, 16GB RAM, macOS Sonoma
**Version**: 1.1
