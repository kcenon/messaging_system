# Performance Baseline

**Version**: 0.1.0
**Last Updated**: 2025-11-30

---

## Overview

This document establishes performance baselines for messaging_system to enable regression detection.

---

## Baseline Metrics

### Message Bus Performance

| Metric | Baseline | Acceptable Range |
|--------|----------|------------------|
| Pub/Sub Throughput | 500K msg/s | >400K |
| Routing Latency | 0.8 μs | <1.5 μs |
| Memory per 1K topics | 5 MB | <8 MB |

### Pattern Performance

| Pattern | Baseline | Acceptable Range |
|---------|----------|------------------|
| Request/Reply | 10K req/s | >8K |
| Event Stream | 300K evt/s | >250K |
| Pipeline | 200K msg/s | >150K |

---

## Test Environment

### Reference Platform

- **CPU**: Apple M1 (8-core) @ 3.2GHz
- **RAM**: 16GB
- **OS**: macOS Sonoma
- **Compiler**: Apple Clang 15, -O3

---

## Benchmark Commands

```bash
# Run all benchmarks
./build/benchmarks/messaging_benchmark

# Message bus only
./build/benchmarks/messaging_benchmark --benchmark_filter="MessageBus*"

# With JSON output
./build/benchmarks/messaging_benchmark --benchmark_format=json > results.json
```

---

## Regression Detection

CI will fail if any metric degrades beyond acceptable range.

---

## Historical Data

Performance history is tracked in CI artifacts.
