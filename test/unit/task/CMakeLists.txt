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

# Test: task_handler
add_executable(test_task_handler
    test_task_handler.cpp
)

target_link_libraries(test_task_handler
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_task_handler COMMAND test_task_handler)
set_tests_properties(test_task_handler PROPERTIES TIMEOUT 60)

# Test: task_context
add_executable(test_task_context
    test_task_context.cpp
)

target_link_libraries(test_task_context
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_task_context COMMAND test_task_context)
set_tests_properties(test_task_context PROPERTIES TIMEOUT 60)
