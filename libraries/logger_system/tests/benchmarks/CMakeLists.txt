##################################################
# Logger System Benchmarks CMakeLists.txt
# 
# Build configuration for all benchmark applications.
# Benchmarks are organized by component for better structure and maintainability.
##################################################

# Include the FetchBenchmark module
include(${CMAKE_SOURCE_DIR}/cmake/FetchBenchmark.cmake)

# Add subdirectories for each component's benchmarks
add_subdirectory(logger_benchmarks)
add_subdirectory(writers_benchmarks)

# Create a master target that builds all benchmarks
add_custom_target(logger_benchmarks
    COMMENT "Building all logger system benchmarks..."
)

# Add dependencies to the master benchmarks target
add_dependencies(logger_benchmarks
    logger_benchmarks_target
    writers_benchmarks_target
)

# Add convenience target to run all benchmarks
add_custom_target(run_logger_system_benchmarks
    COMMENT "Running all logger system benchmarks..."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add dependencies for running benchmarks
add_dependencies(run_logger_system_benchmarks logger_benchmarks)