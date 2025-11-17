# Unit tests for messaging_system core

# Test: message
add_executable(test_message
    test_message.cpp
)

target_link_libraries(test_message
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_message COMMAND test_message)

# Test: message_queue
add_executable(test_message_queue
    test_message_queue.cpp
)

target_link_libraries(test_message_queue
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_message_queue COMMAND test_message_queue)

# Test: topic_router
add_executable(test_topic_router
    test_topic_router.cpp
)

target_link_libraries(test_topic_router
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_topic_router COMMAND test_topic_router)

# Test: message_bus
add_executable(test_message_bus
    test_message_bus.cpp
)

target_link_libraries(test_message_bus
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_message_bus COMMAND test_message_bus)
