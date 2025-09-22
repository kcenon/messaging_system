##################################################
# Writers Benchmarks CMakeLists.txt
# 
# Build configuration for writer benchmark applications.
##################################################

# Define the benchmark group name
project(writers_benchmarks)

# Console writer benchmark
add_executable(console_writer_benchmark
    console_writer_benchmark.cpp
)

# Writer comparison benchmark
add_executable(writer_comparison_benchmark
    writer_comparison_benchmark.cpp
)

# Set common properties for all benchmarks
set(WRITERS_BENCHMARK_TARGETS
    console_writer_benchmark
    writer_comparison_benchmark
)

foreach(target ${WRITERS_BENCHMARK_TARGETS})
    # Link against required dependencies
    if(APPLE)
        target_link_libraries(${target} PRIVATE logger benchmark::benchmark "-framework CoreFoundation")
    else()
        target_link_libraries(${target} PRIVATE logger benchmark::benchmark)
    endif()
    
    # Set C++20 standard
    target_compile_features(${target} PRIVATE cxx_std_20)
    
    # Set output directory
    set_target_properties(${target}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()

# Create a group target for all writers benchmarks
add_custom_target(writers_benchmarks_target
    DEPENDS ${WRITERS_BENCHMARK_TARGETS}
    COMMENT "Building writers benchmarks..."
)

# Add run target for writers benchmarks
add_custom_target(run_writers_benchmarks
    COMMAND console_writer_benchmark
    COMMAND writer_comparison_benchmark
    DEPENDS writers_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running writers benchmarks..."
)