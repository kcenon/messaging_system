# Unit tests for messaging_system core

# Add test executable
add_executable(test_message
    test_message.cpp
)

# Link dependencies
target_link_libraries(test_message
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

# Add test to CTest
add_test(NAME test_message COMMAND test_message)
