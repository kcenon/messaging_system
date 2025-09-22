##################################################
# Typed Thread Pool Benchmarks CMakeLists.txt
# 
# Build configuration for typed thread pool benchmark applications.
##################################################

# Define the benchmark group name
project(typed_thread_pool_benchmarks)

# Typed scheduling benchmark
add_executable(typed_scheduling_benchmark
    typed_scheduling_benchmark.cpp
)

# Typed lock-free benchmark (disabled - lockfree removed)
#add_executable(typed_lockfree_benchmark
#    typed_lockfree_benchmark.cpp
#)

# Queue comparison benchmark
add_executable(queue_comparison_benchmark
    queue_comparison_benchmark.cpp
)

# Typed scheduling benchmark with Google Benchmark
add_executable(typed_scheduling_benchmark_google
    typed_scheduling_benchmark_google.cpp
)

# Set common properties for all priority benchmarks
set(TYPED_BENCHMARK_TARGETS
    typed_scheduling_benchmark
    queue_comparison_benchmark
    typed_scheduling_benchmark_google
)

foreach(target ${TYPED_BENCHMARK_TARGETS})
    # Link against priority thread pool and dependencies
    if(APPLE)
        target_link_libraries(${target} PRIVATE typed_thread_pool thread_pool utilities "-framework CoreFoundation")
    else()
        target_link_libraries(${target} PRIVATE typed_thread_pool thread_pool utilities)
    endif()
    
    # Add Google Benchmark for benchmarks
    if(target STREQUAL "typed_scheduling_benchmark_google" OR 
       target STREQUAL "queue_comparison_benchmark")
        target_link_libraries(${target} PRIVATE benchmark::benchmark)
    endif()
    
    # Set C++20 standard
    target_compile_features(${target} PRIVATE cxx_std_20)
    
    # Set output directory
    set_target_properties(${target}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()

# Create a group target for all priority thread pool benchmarks
add_custom_target(typed_thread_pool_benchmarks_target
    DEPENDS ${TYPED_BENCHMARK_TARGETS}
    COMMENT "Building typed thread pool benchmarks..."
)

# Add run target for priority thread pool benchmarks
add_custom_target(run_typed_thread_pool_benchmarks
    COMMAND typed_scheduling_benchmark
    DEPENDS typed_thread_pool_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running typed thread pool benchmarks..."
)

# Add run target for queue comparison benchmarks
add_custom_target(run_queue_comparison_benchmarks
    COMMAND queue_comparison_benchmark
    DEPENDS typed_thread_pool_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running queue comparison benchmarks..."
)

# Add run target for all typed benchmarks
add_custom_target(run_all_typed_benchmarks
    COMMAND queue_comparison_benchmark
    DEPENDS typed_thread_pool_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running all typed thread pool benchmarks..."
)