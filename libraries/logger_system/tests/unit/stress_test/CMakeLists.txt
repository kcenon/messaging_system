##################################################
# Stress Test CMakeLists.txt
# 
# Builds stress tests for the logger system to validate
# performance and stability under extreme load conditions.
##################################################

# Create stress test executable
add_executable(stress_test
    stress_test.cpp
)

# Link with logger library and dependencies
target_link_libraries(stress_test
    PRIVATE
        logger
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
)

# Include directories
target_include_directories(stress_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sources
        ${CMAKE_SOURCE_DIR}/unittest
)

# Add to CTest
add_test(
    NAME stress_test
    COMMAND stress_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Set test properties
set_tests_properties(stress_test PROPERTIES
    TIMEOUT 300  # 5 minutes timeout for stress tests
    LABELS "stress;performance"
)

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(stress_test PRIVATE LINUX_PLATFORM)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(stress_test PRIVATE MACOS_PLATFORM)
endif()

# Apply sanitizers using the logger module function
if(LOGGER_ENABLE_SANITIZERS)
    logger_add_sanitizers(stress_test)
endif()