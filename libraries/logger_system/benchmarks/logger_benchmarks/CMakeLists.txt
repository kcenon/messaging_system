##################################################
# Logger Benchmarks CMakeLists.txt
# 
# Build configuration for logger benchmark applications.
##################################################

# Define the benchmark group name
project(logger_benchmarks)

# Core logger performance benchmark
add_executable(logger_performance_benchmark
    logger_performance_benchmark.cpp
)

# Synchronous vs Asynchronous comparison
add_executable(sync_vs_async_benchmark
    sync_vs_async_benchmark.cpp
)

# Memory usage benchmark
add_executable(logger_memory_benchmark
    logger_memory_benchmark.cpp
)

# Multithreaded logging benchmark
add_executable(multithreaded_logging_benchmark
    multithreaded_logging_benchmark.cpp
)

# Log level filtering benchmark
add_executable(log_level_benchmark
    log_level_benchmark.cpp
)

# Throughput benchmark
add_executable(logger_throughput_benchmark
    logger_throughput_benchmark.cpp
)

# Set common properties for all benchmarks
set(LOGGER_BENCHMARK_TARGETS
    logger_performance_benchmark
    sync_vs_async_benchmark
    logger_memory_benchmark
    multithreaded_logging_benchmark
    log_level_benchmark
    logger_throughput_benchmark
)

foreach(target ${LOGGER_BENCHMARK_TARGETS})
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

# Create a group target for all logger benchmarks
add_custom_target(logger_benchmarks_target
    DEPENDS ${LOGGER_BENCHMARK_TARGETS}
    COMMENT "Building logger benchmarks..."
)

# Add run target for logger benchmarks
add_custom_target(run_logger_benchmarks
    COMMAND logger_performance_benchmark
    COMMAND sync_vs_async_benchmark
    COMMAND logger_memory_benchmark
    COMMAND multithreaded_logging_benchmark
    COMMAND log_level_benchmark
    COMMAND logger_throughput_benchmark
    DEPENDS logger_benchmarks_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running logger benchmarks..."
)