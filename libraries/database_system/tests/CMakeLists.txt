##################################################
# Database System Tests Configuration
##################################################

# Enable testing
enable_testing()

# Find test dependencies
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    find_package(gtest QUIET)
endif()
find_package(benchmark QUIET)

##################################################
# Unit Tests
##################################################

if(GTest_FOUND OR gtest_FOUND)
    # Create unit test executable
    add_executable(database_unit_tests
        unit_tests.cpp
    )

    # Link dependencies - handle different target names
    if(GTest_FOUND)
        target_link_libraries(database_unit_tests PRIVATE
            database
            GTest::gtest
            GTest::gtest_main
            Threads::Threads
        )
    else()
        target_link_libraries(database_unit_tests PRIVATE
            database
            gtest
            gtest_main
            Threads::Threads
        )
    endif()
    
    # Set properties
    set_target_properties(database_unit_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Add test
    add_test(NAME DatabaseUnitTests COMMAND database_unit_tests)
    
    # Enable test discovery
    include(GoogleTest)
    gtest_discover_tests(database_unit_tests)
else()
    message(WARNING "GTest not found - unit tests will not be built")
endif()

##################################################
# Benchmark Tests
##################################################

if(benchmark_FOUND)
    # Create benchmark executable
    add_executable(database_benchmark_tests
        benchmark_tests.cpp
    )
    
    # Link dependencies
    target_link_libraries(database_benchmark_tests PRIVATE
        database
        benchmark::benchmark
        benchmark::benchmark_main
        Threads::Threads
    )
    
    # Set properties
    set_target_properties(database_benchmark_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Platform-specific optimizations
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(database_benchmark_tests PRIVATE
            -O3
            -march=native
        )
    elseif(MSVC)
        target_compile_options(database_benchmark_tests PRIVATE
            /O2
        )
    endif()
else()
    message(WARNING "Google Benchmark not found - benchmark tests will not be built")
endif()

##################################################
# Test Data Setup
##################################################

# Copy test data files if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_data)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_data
         DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()