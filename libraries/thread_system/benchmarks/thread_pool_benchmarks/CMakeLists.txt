##################################################
# Thread Pool Benchmarks CMakeLists.txt
# 
# Build configuration for thread pool benchmark applications.
##################################################

# Define the benchmark group name
project(thread_pool_benchmarks)

# Google Benchmark based thread pool benchmark
add_executable(gbench_thread_pool gbench_thread_pool.cpp)
target_link_libraries(gbench_thread_pool PRIVATE thread_pool thread_base utilities benchmark::benchmark)
set_target_properties(gbench_thread_pool PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# NEW: Lockfree comparison benchmark using Google Benchmark (disabled - lockfree removed)
#add_executable(lockfree_comparison_benchmark lockfree_comparison_benchmark.cpp)
#target_link_libraries(lockfree_comparison_benchmark PRIVATE thread_pool thread_base utilities benchmark::benchmark)
#set_target_properties(lockfree_comparison_benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# NEW: Lockfree performance benchmark (disabled - lockfree removed)
#add_executable(lockfree_performance_benchmark lockfree_performance_benchmark.cpp)
#target_link_libraries(lockfree_performance_benchmark PRIVATE thread_pool thread_base utilities benchmark::benchmark)
#set_target_properties(lockfree_performance_benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Core thread pool performance benchmark
add_executable(thread_pool_benchmark
    thread_pool_benchmark.cpp
)

# Memory usage benchmark
add_executable(memory_benchmark
    memory_benchmark.cpp
)

# Real-world scenarios benchmark
add_executable(real_world_benchmark
    real_world_benchmark.cpp
)

# Stress test benchmark
add_executable(stress_test_benchmark
    stress_test_benchmark.cpp
)

# Comparison with other libraries
add_executable(comparison_benchmark
    comparison_benchmark.cpp
)

# Detailed throughput analysis
add_executable(throughput_detailed_benchmark
    throughput_detailed_benchmark.cpp
)

# NEW: Scalability benchmark
add_executable(scalability_benchmark
    scalability_benchmark.cpp
)

# NEW: Contention benchmark
add_executable(contention_benchmark
    contention_benchmark.cpp
)

# Set common properties for all benchmarks
set(THREAD_POOL_BENCHMARK_TARGETS
    thread_pool_benchmark
    memory_benchmark
    real_world_benchmark
    stress_test_benchmark
    comparison_benchmark
    throughput_detailed_benchmark
    scalability_benchmark
    contention_benchmark
)

foreach(target ${THREAD_POOL_BENCHMARK_TARGETS})
    # Link against required dependencies
    if(APPLE)
        target_link_libraries(${target} PRIVATE thread_pool typed_thread_pool utilities benchmark::benchmark "-framework CoreFoundation")
    else()
        target_link_libraries(${target} PRIVATE thread_pool typed_thread_pool utilities benchmark::benchmark)
    endif()
    
    # Set C++20 standard
    target_compile_features(${target} PRIVATE cxx_std_20)
    
    # Set output directory
    set_target_properties(${target}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()

# Check for OpenMP for comparison benchmark
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(comparison_benchmark PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(comparison_benchmark PRIVATE _OPENMP)
endif()

# Create a group target for all thread pool benchmarks
add_custom_target(thread_pool_benchmarks_target
    DEPENDS ${THREAD_POOL_BENCHMARK_TARGETS}
    COMMENT "Building thread pool benchmarks..."
)

# Add run target for thread pool benchmarks
add_custom_target(run_thread_pool_benchmarks
    COMMAND thread_pool_benchmark
    COMMAND memory_benchmark
    COMMAND scalability_benchmark
    COMMAND contention_benchmark
    COMMAND real_world_benchmark
    COMMAND stress_test_benchmark
    COMMAND comparison_benchmark
    COMMAND throughput_detailed_benchmark
    DEPENDS thread_pool_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running thread pool benchmarks..."
)