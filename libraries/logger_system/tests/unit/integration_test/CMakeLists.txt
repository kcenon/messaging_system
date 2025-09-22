##################################################
# Integration Test CMakeLists.txt
# 
# Builds integration tests for the logger system to validate
# end-to-end functionality with multiple components working together.
##################################################

# Create integration test executable
add_executable(integration_test
    integration_test.cpp
)

# Link with logger library and dependencies
target_link_libraries(integration_test
    PRIVATE
        logger
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
)

# Include directories
target_include_directories(integration_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sources
        ${CMAKE_SOURCE_DIR}/unittest
)

# Add to CTest
add_test(
    NAME integration_test
    COMMAND integration_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Set test properties
set_tests_properties(integration_test PROPERTIES
    TIMEOUT 120  # 2 minutes timeout for integration tests
    LABELS "integration;e2e"
)

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(integration_test PRIVATE LINUX_PLATFORM)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(integration_test PRIVATE MACOS_PLATFORM)
endif()

# Apply sanitizers using the logger module function
if(LOGGER_ENABLE_SANITIZERS)
    logger_add_sanitizers(integration_test)
endif()