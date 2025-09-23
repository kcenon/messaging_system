cmake_minimum_required(VERSION 3.16)
project(messaging_system_tests)

# Find required packages
find_package(GTest)

# Check if GTest is found
if(NOT GTest_FOUND)
    message(WARNING "GTest not found - unit tests will be disabled")
    return()
endif()

# Enable testing
enable_testing()

# Common test settings
set(TEST_COMMON_LIBS
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Create tests list to track all test executables
set(TESTS)

# Container tests
if(BUILD_CONTAINER AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/container_test.cpp)
    add_executable(container_test container_test.cpp)
    target_link_libraries(container_test
        ${TEST_COMMON_LIBS}
        container_system
    )
    add_test(NAME container_test COMMAND container_test)
    list(APPEND TESTS container_test)
endif()

# Database tests  
if(BUILD_DATABASE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/database_test.cpp)
    add_executable(database_test database_test.cpp)
    target_link_libraries(database_test
        ${TEST_COMMON_LIBS}
        database
    )
    add_test(NAME database_test COMMAND database_test)
    list(APPEND TESTS database_test)
endif()

# Network tests
if(BUILD_NETWORK AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/network_test.cpp)
    add_executable(network_test network_test.cpp)
    target_link_libraries(network_test
        ${TEST_COMMON_LIBS}
        NetworkSystem
    )
    target_include_directories(network_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/network_system/include
    )
    add_test(NAME network_test COMMAND network_test)
    list(APPEND TESTS network_test)
endif()


# Integration tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration_test.cpp)
    add_executable(integration_test integration_test.cpp)
    set(INTEGRATION_TEST_LIBS ${TEST_COMMON_LIBS})
    if(BUILD_CONTAINER)
        list(APPEND INTEGRATION_TEST_LIBS container_system)
    endif()
    if(BUILD_DATABASE AND TARGET database)
        list(APPEND INTEGRATION_TEST_LIBS database)
    endif()
    if(BUILD_NETWORK)
        list(APPEND INTEGRATION_TEST_LIBS NetworkSystem)
    endif()
    target_link_libraries(integration_test ${INTEGRATION_TEST_LIBS})
    add_test(NAME integration_test COMMAND integration_test)
    list(APPEND TESTS integration_test)
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