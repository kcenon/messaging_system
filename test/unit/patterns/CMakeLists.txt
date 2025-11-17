# Pattern tests

# Pub/Sub pattern tests
add_executable(test_pub_sub test_pub_sub.cpp)
target_link_libraries(test_pub_sub
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)
add_test(NAME test_pub_sub COMMAND test_pub_sub)

# Request-Reply pattern tests
add_executable(test_request_reply test_request_reply.cpp)
target_link_libraries(test_request_reply
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)
add_test(NAME test_request_reply COMMAND test_request_reply)

# Event Streaming pattern tests
add_executable(test_event_streaming test_event_streaming.cpp)
target_link_libraries(test_event_streaming
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)
add_test(NAME test_event_streaming COMMAND test_event_streaming)

# Message Pipeline pattern tests
add_executable(test_message_pipeline test_message_pipeline.cpp)
target_link_libraries(test_message_pipeline
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)
add_test(NAME test_message_pipeline COMMAND test_message_pipeline)
