# Container System Tests CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# Test configuration
enable_testing()

# Find required packages for testing
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # If GTest is not found, try to use FetchContent to download it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    set(GTEST_LIBRARIES gtest gtest_main)
    set(GTEST_INCLUDE_DIRS ${googletest_SOURCE_DIR}/googletest/include)
else()
    set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main)
    set(GTEST_INCLUDE_DIRS "")
endif()

# Find benchmark library for performance tests
find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    # Try pkg-config as fallback
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(BENCHMARK QUIET benchmark)
        if(BENCHMARK_FOUND)
            set(BENCHMARK_LIBRARIES ${BENCHMARK_LIBRARIES})
            set(BENCHMARK_INCLUDE_DIRS ${BENCHMARK_INCLUDE_DIRS})
        endif()
    endif()
endif()

if(benchmark_FOUND)
    set(BENCHMARK_LIBRARIES benchmark::benchmark)
    set(BENCHMARK_INCLUDE_DIRS "")
elseif(BENCHMARK_FOUND)
    # Use pkg-config results
else()
    # Fallback to system library
    find_library(BENCHMARK_LIBRARIES NAMES benchmark)
    find_path(BENCHMARK_INCLUDE_DIRS benchmark/benchmark.h)
endif()

# Test source files
set(TEST_SOURCES
    unit_tests.cpp
    benchmark_tests.cpp
    test_messaging_integration.cpp
    performance_tests.cpp
)

# Create test executables
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SOURCE})

    # Link with container system library
    target_link_libraries(${TEST_NAME}
        container_system
        ${GTEST_LIBRARIES}
        Threads::Threads
    )

    # Add benchmark library for benchmark tests
    if(${TEST_NAME} STREQUAL "benchmark_tests" AND BENCHMARK_LIBRARIES)
        target_link_libraries(${TEST_NAME} ${BENCHMARK_LIBRARIES})
        if(BENCHMARK_INCLUDE_DIRS)
            target_include_directories(${TEST_NAME} PRIVATE ${BENCHMARK_INCLUDE_DIRS})
        endif()
    endif()

    # Include directories
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${GTEST_INCLUDE_DIRS}
    )

    # Set compile definitions based on features
    if(ENABLE_MESSAGING_FEATURES)
        target_compile_definitions(${TEST_NAME} PRIVATE HAS_MESSAGING_FEATURES)
    endif()

    if(ENABLE_PERFORMANCE_METRICS)
        target_compile_definitions(${TEST_NAME} PRIVATE HAS_PERFORMANCE_METRICS)
    endif()

    if(ENABLE_EXTERNAL_INTEGRATION)
        target_compile_definitions(${TEST_NAME} PRIVATE HAS_EXTERNAL_INTEGRATION)
    endif()

    # Add as CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 300
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()

# Special configuration for performance tests
if(TARGET performance_tests)
    set_tests_properties(performance_tests PROPERTIES
        TIMEOUT 600  # Longer timeout for performance tests
        LABELS "performance"
    )
endif()

# Create individual test runners (thread_system style)
add_executable(container_base_unit
    unit_tests.cpp
)

add_executable(container_messaging_unit
    test_messaging_integration.cpp
)

# Create a combined test runner for compatibility
add_executable(all_tests
    unit_tests.cpp
    test_messaging_integration.cpp
)

# Link individual test runners
target_link_libraries(container_base_unit
    container_system
    ${GTEST_LIBRARIES}
    Threads::Threads
)

target_include_directories(container_base_unit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(container_messaging_unit
    container_system
    ${GTEST_LIBRARIES}
    Threads::Threads
)

target_include_directories(container_messaging_unit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${GTEST_INCLUDE_DIRS}
)

# Link combined test runner
target_link_libraries(all_tests
    container_system
    ${GTEST_LIBRARIES}
    Threads::Threads
)

target_include_directories(all_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${GTEST_INCLUDE_DIRS}
)

# Set compile definitions for individual tests
if(ENABLE_MESSAGING_FEATURES)
    target_compile_definitions(container_base_unit PRIVATE HAS_MESSAGING_FEATURES)
    target_compile_definitions(container_messaging_unit PRIVATE HAS_MESSAGING_FEATURES)
    target_compile_definitions(all_tests PRIVATE HAS_MESSAGING_FEATURES)
endif()

if(ENABLE_PERFORMANCE_METRICS)
    target_compile_definitions(container_base_unit PRIVATE HAS_PERFORMANCE_METRICS)
    target_compile_definitions(container_messaging_unit PRIVATE HAS_PERFORMANCE_METRICS)
    target_compile_definitions(all_tests PRIVATE HAS_PERFORMANCE_METRICS)
endif()

if(ENABLE_EXTERNAL_INTEGRATION)
    target_compile_definitions(container_base_unit PRIVATE HAS_EXTERNAL_INTEGRATION)
    target_compile_definitions(container_messaging_unit PRIVATE HAS_EXTERNAL_INTEGRATION)
    target_compile_definitions(all_tests PRIVATE HAS_EXTERNAL_INTEGRATION)
endif()

# Add individual tests
add_test(NAME container_base_unit COMMAND container_base_unit)
add_test(NAME container_messaging_unit COMMAND container_messaging_unit)
add_test(NAME all_tests COMMAND all_tests)

# Test coverage configuration (if supported)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(all_tests PRIVATE --coverage)
    target_link_options(all_tests PRIVATE --coverage)

    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        if(TARGET ${TEST_NAME})
            target_compile_options(${TEST_NAME} PRIVATE --coverage)
            target_link_options(${TEST_NAME} PRIVATE --coverage)
        endif()
    endforeach()
endif()

# Custom test targets
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L unit -V
    DEPENDS unit_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running unit tests"
)

add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L integration -V
    DEPENDS test_messaging_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running integration tests"
)

add_custom_target(run_performance_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L performance -V
    DEPENDS performance_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running performance tests"
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -V
    DEPENDS ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all tests"
)

# Test configuration summary
message(STATUS "Container System Tests Configuration:")
message(STATUS "  GTest found: ${GTest_FOUND}")
message(STATUS "  Test sources: ${TEST_SOURCES}")
message(STATUS "  Coverage enabled: ${ENABLE_COVERAGE}")
if(ENABLE_MESSAGING_FEATURES)
    message(STATUS "  Messaging features tests: ON")
endif()
if(ENABLE_PERFORMANCE_METRICS)
    message(STATUS "  Performance metrics tests: ON")
endif()
if(ENABLE_EXTERNAL_INTEGRATION)
    message(STATUS "  External integration tests: ON")
endif()