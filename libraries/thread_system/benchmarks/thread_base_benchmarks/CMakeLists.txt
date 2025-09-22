##################################################
# Thread Base Benchmarks CMakeLists.txt
# 
# Build configuration for thread_base benchmark applications.
##################################################

# Find Google Benchmark package
find_package(benchmark CONFIG REQUIRED)

# MPMC Performance Benchmark
add_executable(mpmc_performance_benchmark mpmc_performance_test.cpp)
target_link_libraries(mpmc_performance_benchmark PRIVATE
    thread_base
    utilities
    logger
    benchmark::benchmark
)
set_target_properties(mpmc_performance_benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Simple MPMC Benchmark
add_executable(simple_mpmc_benchmark simple_mpmc_benchmark.cpp)
target_link_libraries(simple_mpmc_benchmark PRIVATE
    thread_base
    utilities
    logger
    benchmark::benchmark
)
set_target_properties(simple_mpmc_benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Quick MPMC Test
add_executable(quick_mpmc_test quick_mpmc_test.cpp)
target_link_libraries(quick_mpmc_test PRIVATE
    thread_base
    utilities
    logger
)
set_target_properties(quick_mpmc_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Create a convenience target that builds all thread_base benchmarks
add_custom_target(thread_base_benchmarks_target
    DEPENDS mpmc_performance_benchmark simple_mpmc_benchmark quick_mpmc_test
    COMMENT "Building all thread_base benchmarks..."
)

# Add convenience target to run thread_base benchmarks
add_custom_target(run_thread_base_benchmarks
    COMMAND $<TARGET_FILE:mpmc_performance_benchmark> --benchmark_out_format=json --benchmark_out=mpmc_performance_results.json
    DEPENDS mpmc_performance_benchmark
    COMMENT "Running thread_base benchmarks..."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)