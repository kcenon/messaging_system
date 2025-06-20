# Messaging System Samples

This directory contains sample applications demonstrating various features of the messaging system.

## Sample Applications

### 1. [Simple Client-Server](simple_client_server/)
Basic TCP/IP client-server communication example.

### 2. [Container Demo](container_demo/)
Demonstrates the container module for type-safe data serialization.

### 3. [Database Example](database_example/)
Shows PostgreSQL integration and database operations.

### 4. [Async Messaging](async_messaging/)
Demonstrates asynchronous messaging patterns with coroutines.

### 5. [Performance Test](performance_test/)
Benchmarks for measuring system performance.

### 6. [Lock-Free Network Demo](lockfree_network_demo/)
High-performance lock-free networking patterns.

### 7. [Unified Network Benchmark](unified_network_benchmark/)
Comprehensive networking benchmarks.

## Building All Samples

```bash
cd ..
./build.sh --samples
```

## Building Individual Sample

Each sample can be built as part of the main build process. The executables will be placed in the `build/bin/` directory.