##################################################
# Storage Benchmarks CMakeLists.txt
# 
# Build configuration for storage benchmark applications.
##################################################

# Define the benchmark group name
project(storage_benchmarks)

# Ring buffer benchmark
add_executable(ring_buffer_benchmark
    ring_buffer_benchmark.cpp
)

# Set common properties for all benchmarks
set(STORAGE_BENCHMARK_TARGETS
    ring_buffer_benchmark
)

foreach(target ${STORAGE_BENCHMARK_TARGETS})
    # Link against required dependencies
    if(APPLE)
        target_link_libraries(${target} PRIVATE monitoring benchmark::benchmark "-framework CoreFoundation")
    else()
        target_link_libraries(${target} PRIVATE monitoring benchmark::benchmark)
    endif()
    
    # Set C++20 standard
    target_compile_features(${target} PRIVATE cxx_std_20)
    
    # Set output directory
    set_target_properties(${target}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()

# Create a group target for all storage benchmarks
add_custom_target(storage_benchmarks_target
    DEPENDS ${STORAGE_BENCHMARK_TARGETS}
    COMMENT "Building storage benchmarks..."
)

# Add run target for storage benchmarks
add_custom_target(run_storage_benchmarks
    COMMAND ring_buffer_benchmark
    DEPENDS storage_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running storage benchmarks..."
)