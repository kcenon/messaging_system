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
set_tests_properties(test_message PROPERTIES TIMEOUT 60)

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
set_tests_properties(test_message_queue PROPERTIES TIMEOUT 60)

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
set_tests_properties(test_topic_router PROPERTIES TIMEOUT 60)

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
set_tests_properties(test_message_bus PROPERTIES TIMEOUT 60)

# Test: message_broker
add_executable(test_message_broker
    test_message_broker.cpp
)

target_link_libraries(test_message_broker
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_message_broker COMMAND test_message_broker)
set_tests_properties(test_message_broker PROPERTIES TIMEOUT 60)
