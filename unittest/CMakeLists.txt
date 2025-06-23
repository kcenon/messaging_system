cmake_minimum_required(VERSION 3.16)
project(messaging_system_tests)

# Find required packages
find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Common test settings
set(TEST_COMMON_LIBS
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Container tests
if(BUILD_CONTAINER AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/container_test.cpp)
    add_executable(container_test container_test.cpp)
    target_link_libraries(container_test
        ${TEST_COMMON_LIBS}
        container
    )
    add_test(NAME container_test COMMAND container_test)
endif()

# Database tests  
if(BUILD_DATABASE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/database_test.cpp)
    add_executable(database_test database_test.cpp)
    target_link_libraries(database_test
        ${TEST_COMMON_LIBS}
        database
    )
    add_test(NAME database_test COMMAND database_test)
endif()

# Network tests
if(BUILD_NETWORK AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/network_test.cpp)
    add_executable(network_test network_test.cpp)
    target_link_libraries(network_test
        ${TEST_COMMON_LIBS}
        network
    )
    add_test(NAME network_test COMMAND network_test)
endif()

# Thread system tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thread_system_test.cpp)
    add_executable(thread_system_test thread_system_test.cpp)
    target_link_libraries(thread_system_test
        ${TEST_COMMON_LIBS}
        thread_base
        thread_pool
        typed_thread_pool
        logger
    )
    add_test(NAME thread_system_test COMMAND thread_system_test)
endif()

# Integration tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration_test.cpp)
    add_executable(integration_test integration_test.cpp)
    target_link_libraries(integration_test
        ${TEST_COMMON_LIBS}
        container
        database
        network
        thread_pool
    )
    add_test(NAME integration_test COMMAND integration_test)
endif()

# Set test properties
set_tests_properties(${TESTS}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${TESTS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
)