# Unit tests for messaging_system task module

# Test: task
add_executable(test_task
    test_task.cpp
)

target_link_libraries(test_task
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_task COMMAND test_task)
set_tests_properties(test_task PROPERTIES TIMEOUT 60)
