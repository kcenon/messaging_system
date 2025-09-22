# Overflow policy unit tests
add_executable(overflow_policy_test
    overflow_policy_test.cpp
)

target_link_libraries(overflow_policy_test
    PRIVATE
        logger
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
)

target_include_directories(overflow_policy_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/sources
)

# Add test
add_test(
    NAME overflow_policy_test
    COMMAND overflow_policy_test
)

# Set test properties
set_tests_properties(overflow_policy_test PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "GTEST_COLOR=1"
)

# Enable CTest
enable_testing()