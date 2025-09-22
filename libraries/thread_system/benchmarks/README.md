# Thread System Benchmarks

This directory contains comprehensive performance benchmarks for the Thread System framework.

## Available Benchmarks

### Core Benchmarks

#### 1. **thread_pool_benchmark.cpp**
Core performance tests including:
- Thread pool creation overhead
- Job submission latency
- Job throughput with varying workloads
- Scaling efficiency across multiple cores
- Type-based scheduling performance

#### 2. **memory_benchmark.cpp**
Memory usage analysis:
- Base memory footprint
- Per-worker memory overhead
- Job queue memory consumption
- Memory usage with different configurations
- Platform-specific memory measurements

#### 3. **logger_benchmark.cpp**
Logger performance tests:
- Throughput by log level
- Latency analysis (P50, P90, P99, P99.9)
- Concurrent logging performance
- Performance with different output targets

### Advanced Benchmarks

#### 4. **real_world_benchmark.cpp**
Real-world scenario simulations:
- Web server request handling
- Image processing pipeline
- Data analysis workloads (MapReduce style)
- Game engine update loops
- Microservice communication patterns
- Batch file processing

#### 5. **stress_test_benchmark.cpp**
Stress tests and edge cases:
- Maximum thread creation limits
- Queue overflow handling
- Rapid start/stop cycles
- Exception handling under load
- Memory pressure scenarios
- Type starvation tests
- Thundering herd problem
- Cascading failure simulation

#### 6. **comparison_benchmark.cpp**
Comparative analysis with:
- std::async
- Raw std::thread
- Simple thread pool implementations
- OpenMP (if available)
- Memory usage comparisons
- Task creation overhead analysis

#### 7. **throughput_detailed_benchmark.cpp**
In-depth job throughput analysis:
- Impact of job complexity (empty to very heavy)
- Worker count scaling (1 to 128 workers)
- Queue depth effects on performance
- Memory allocation impact
- Job size distribution patterns
- Sustained throughput over time
- Burst pattern handling
- Job dependency impact
- Type-based scheduling overhead
- Mixed workload scenarios

## Building Benchmarks

```bash
# From the thread_system root directory
mkdir build && cd build
cmake .. -DBUILD_BENCHMARKS=ON -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/toolchain.cmake
make benchmarks
```

## Running Benchmarks

```bash
# Run all benchmarks
make run_benchmarks

# Run only core benchmarks (faster)
make run_core_benchmarks

# Run advanced benchmarks (comprehensive)
make run_advanced_benchmarks

# Run individual benchmarks
./bin/thread_pool_benchmark      # Core performance
./bin/memory_benchmark          # Memory analysis
./bin/logger_benchmark          # Logger performance
./bin/real_world_benchmark      # Real-world scenarios
./bin/stress_test_benchmark     # Stress tests
./bin/comparison_benchmark      # Library comparisons
./bin/throughput_detailed_benchmark  # Detailed throughput analysis
```

## Benchmark Categories

### Quick Tests (< 1 minute)
- thread_pool_benchmark
- memory_benchmark
- logger_benchmark

### Comprehensive Tests (5-10 minutes)
- real_world_benchmark
- stress_test_benchmark
- comparison_benchmark
- throughput_detailed_benchmark

## Interpreting Results

### Thread Pool Benchmark
- **Creation Overhead**: Time to create and initialize thread pools
- **Job Latency**: Time to submit a job (not including execution)
- **Throughput**: Jobs processed per second
- **Scaling**: How performance improves with more workers

### Memory Benchmark
- **Base Memory**: Memory used by empty thread pool
- **Per Worker**: Additional memory per worker thread
- **Per Job**: Memory overhead per queued job

### Logger Benchmark
- **Throughput**: Log messages per second
- **Latency**: Time to submit a log message
- **Percentiles**: P50, P90, P99 for latency distribution

## Adding New Benchmarks

1. Create a new `.cpp` file in this directory
2. Include necessary headers and benchmark utilities
3. Add the executable to `CMakeLists.txt`
4. Follow the existing benchmark pattern:
   - Clear console output
   - Multiple test scenarios
   - Statistical analysis
   - Comparative results

## Performance Tips

Based on benchmark results:
- Use `hardware_concurrency()` workers for CPU-bound tasks
- Double that for I/O-bound tasks
- Batch small jobs for better throughput
- Monitor memory usage with large job queues