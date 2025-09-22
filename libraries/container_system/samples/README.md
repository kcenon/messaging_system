# Container System Samples

This directory contains example programs demonstrating the Container System's capabilities.

## Available Samples

### 1. Basic Usage (`basic_usage.cpp`)
Demonstrates fundamental container operations:
- Creating and configuring containers
- Setting and getting values of different types
- Working with nested containers
- Binary data handling
- Serialization and deserialization
- Container iteration

**Usage:**
```bash
./basic_usage
```

### 2. Thread Safety Example (`thread_safe_example.cpp`)
Shows thread-safe container operations:
- Concurrent read/write operations
- Multi-threaded data access patterns
- Performance comparison between regular and thread-safe containers
- Thread safety verification
- Stress testing with multiple threads

**Usage:**
```bash
./thread_safe_example
```

### 3. Performance Benchmark (`performance_benchmark.cpp`)
Comprehensive performance testing:
- Basic operation benchmarks (set/get)
- Serialization performance with various container sizes
- Memory usage analysis
- Concurrent access performance
- SIMD operations with large binary data

**Usage:**
```bash
./performance_benchmark
```

### 4. Run All Samples (`run_all_samples.cpp`)
Utility to run all samples or a specific sample:

**Usage:**
```bash
# Run all samples
./run_all_samples

# Run specific sample
./run_all_samples basic_usage
./run_all_samples thread_safe_example
./run_all_samples performance_benchmark
```

## Building the Samples

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16 or later
- Container System library

### Build Instructions

1. **From the main project directory:**
```bash
mkdir build && cd build
cmake .. -DBUILD_CONTAINER_SAMPLES=ON
make
```

2. **Run samples:**
```bash
cd bin/samples
./basic_usage
./thread_safe_example
./performance_benchmark
./run_all_samples
```

### Alternative Build (samples only)
```bash
cd samples
mkdir build && cd build
cmake ..
make
```

## Sample Output Examples

### Basic Usage Output
```
=== Container System - Basic Usage Example ===

1. Basic Container Operations:
Container message type: user_profile
Container size: 5 values

2. Reading Values:
User ID: 12345
Username: john_doe
Is Active: Yes

3. Nested Containers:
Address: 123 Main St, New York
...
```

### Performance Benchmark Results
```
=== Container System - Performance Benchmark ===

1. Basic Operations Benchmark:
Set operations:
  100000 operations in 1234 μs
  81037.42 ops/sec
  0.012 μs/op
...
```

## Understanding the Results

### Performance Metrics
- **Operations per second**: Higher is better
- **Microseconds per operation**: Lower is better
- **Memory usage**: Actual vs estimated memory consumption
- **Serialization efficiency**: Compression ratio and speed

### Thread Safety Verification
- Multiple threads performing concurrent operations
- Data integrity checks after concurrent access
- Performance overhead of thread-safe operations

### Memory Usage Analysis
- Memory consumption patterns for different data types
- Serialization size vs memory usage
- Memory efficiency across different container sizes

## Customizing the Samples

### Adding New Samples
1. Create a new `.cpp` file in the samples directory
2. Add it to the `SAMPLE_PROGRAMS` list in `CMakeLists.txt`
3. Include it in the `run_all_samples.cpp` samples vector

### Modifying Benchmarks
You can adjust benchmark parameters by modifying the constants in `performance_benchmark.cpp`:
- `iterations`: Number of operations for basic benchmarks
- `sizes`: Container sizes to test for serialization
- `num_threads`: Number of concurrent threads
- `ops_per_thread`: Operations per thread in concurrent tests

## Troubleshooting

### Common Issues

1. **Compilation Errors**
   - Ensure C++20 support is enabled
   - Check that container system library is properly linked
   - Verify all required headers are included

2. **Runtime Errors**
   - Check that the container system library is built
   - Ensure proper library paths are set
   - Verify thread safety requirements for concurrent samples

3. **Performance Variations**
   - Results may vary based on system hardware
   - CPU frequency scaling can affect benchmarks
   - Background processes may impact measurements

### Getting Help
- Check the main project README for detailed build instructions
- Review the API documentation for container system usage
- Examine the sample source code for implementation details

## License
These samples are provided under the same BSD 3-Clause License as the Container System project.