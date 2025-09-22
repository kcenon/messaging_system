##################################################
# FetchBenchmark.cmake
# 
# Configuration for fetching Google Benchmark if not found
##################################################

include(FetchContent)

# Try to find benchmark first
find_package(benchmark QUIET)

if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found locally, fetching from GitHub...")
    
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.3
        GIT_SHALLOW    TRUE
    )
    
    # Disable benchmark's tests
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googlebenchmark)
    
    # Create an alias for consistency
    if(NOT TARGET benchmark::benchmark)
        add_library(benchmark::benchmark ALIAS benchmark)
    endif()
    
    message(STATUS "Google Benchmark fetched successfully")
else()
    message(STATUS "Found Google Benchmark: ${benchmark_DIR}")
endif()