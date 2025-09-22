cmake_minimum_required(VERSION 3.16)

# Create config test executable
add_executable(config_unit
    config_test.cpp
    strategy_test.cpp
)

# Link with logger library and test framework
target_link_libraries(config_unit
    PRIVATE
        logger
        GTest::gtest
        GTest::gtest_main
)

# Add include directories
target_include_directories(config_unit
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sources
)

# Add test
add_test(NAME config_unit COMMAND config_unit)

# Create a custom target that runs the test
add_custom_target(config_unit_test
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS config_unit
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)