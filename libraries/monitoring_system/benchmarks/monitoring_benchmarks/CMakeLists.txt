##################################################
# Monitoring Benchmarks CMakeLists.txt
# 
# Build configuration for monitoring benchmark applications.
##################################################

# Define the benchmark group name
project(monitoring_benchmarks)

# Core monitoring performance benchmark
add_executable(monitoring_performance_benchmark
    monitoring_performance_benchmark.cpp
)

# Metrics update benchmark
add_executable(metrics_update_benchmark
    metrics_update_benchmark.cpp
)

# Collection interval benchmark
add_executable(collection_benchmark
    collection_benchmark.cpp
)

# Multithreaded monitoring benchmark
add_executable(multithreaded_monitoring_benchmark
    multithreaded_monitoring_benchmark.cpp
)

# Set common properties for all benchmarks
set(MONITORING_BENCHMARK_TARGETS
    monitoring_performance_benchmark
    metrics_update_benchmark
    collection_benchmark
    multithreaded_monitoring_benchmark
)

foreach(target ${MONITORING_BENCHMARK_TARGETS})
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

# Create a group target for all monitoring benchmarks
add_custom_target(monitoring_benchmarks_target
    DEPENDS ${MONITORING_BENCHMARK_TARGETS}
    COMMENT "Building monitoring benchmarks..."
)

# Add run target for monitoring benchmarks
add_custom_target(run_monitoring_benchmarks
    COMMAND monitoring_performance_benchmark
    COMMAND metrics_update_benchmark
    COMMAND collection_benchmark
    COMMAND multithreaded_monitoring_benchmark
    DEPENDS monitoring_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running monitoring benchmarks..."
)