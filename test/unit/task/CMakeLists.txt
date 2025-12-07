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

# Test: result_backend
add_executable(test_result_backend
    test_result_backend.cpp
)

target_link_libraries(test_result_backend
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_result_backend COMMAND test_result_backend)
set_tests_properties(test_result_backend PROPERTIES TIMEOUT 60)

# Test: worker_pool
add_executable(test_worker_pool
    test_worker_pool.cpp
)

target_link_libraries(test_worker_pool
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_worker_pool COMMAND test_worker_pool)
set_tests_properties(test_worker_pool PROPERTIES TIMEOUT 120)

# Test: async_result
add_executable(test_async_result
    test_async_result.cpp
)

target_link_libraries(test_async_result
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_async_result COMMAND test_async_result)
set_tests_properties(test_async_result PROPERTIES TIMEOUT 60)

# Test: task_client
add_executable(test_task_client
    test_task_client.cpp
)

target_link_libraries(test_task_client
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_task_client COMMAND test_task_client)
set_tests_properties(test_task_client PROPERTIES TIMEOUT 120)
