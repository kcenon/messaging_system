cmake_minimum_required(VERSION 3.16)
project(ThreadSystemIntegrationTests VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the core module
find_package(ThreadSystemCore REQUIRED 
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../core/build
    NO_DEFAULT_PATH
)

# Find GTest for testing
find_package(GTest REQUIRED)

enable_testing()

##################################################
# Test 1: Basic Thread Pool Usage
##################################################
add_executable(test_basic_thread_pool
    test_basic_thread_pool.cpp
)

target_link_libraries(test_basic_thread_pool
    PRIVATE
        ThreadSystem::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME basic_thread_pool_test COMMAND test_basic_thread_pool)

##################################################
# Test 2: Custom Logger Integration
##################################################
add_executable(test_logger_integration
    test_logger_integration.cpp
)

target_link_libraries(test_logger_integration
    PRIVATE
        ThreadSystem::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME logger_integration_test COMMAND test_logger_integration)

##################################################
# Test 3: Custom Monitoring Integration
##################################################
add_executable(test_monitoring_integration
    test_monitoring_integration.cpp
)

target_link_libraries(test_monitoring_integration
    PRIVATE
        ThreadSystem::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME monitoring_integration_test COMMAND test_monitoring_integration)

##################################################
# Test 4: Typed Thread Pool
##################################################
add_executable(test_typed_thread_pool
    test_typed_thread_pool.cpp
)

target_link_libraries(test_typed_thread_pool
    PRIVATE
        ThreadSystem::Core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME typed_thread_pool_test COMMAND test_typed_thread_pool)

##################################################
# Test 5: Performance Benchmark
##################################################
find_package(benchmark QUIET)

if(benchmark_FOUND)
    add_executable(benchmark_thread_system
        benchmark_thread_system.cpp
    )
    
    target_link_libraries(benchmark_thread_system
        PRIVATE
            ThreadSystem::Core
            benchmark::benchmark
            benchmark::benchmark_main
    )
endif()

##################################################
# All Tests Target
##################################################
add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS 
        test_basic_thread_pool
        test_logger_integration
        test_monitoring_integration
        test_typed_thread_pool
)