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
set_tests_properties(test_pub_sub PROPERTIES TIMEOUT 60)

# Request-Reply pattern tests
add_executable(test_request_reply test_request_reply.cpp)
target_link_libraries(test_request_reply
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)
add_test(NAME test_request_reply COMMAND test_request_reply)
set_tests_properties(test_request_reply PROPERTIES TIMEOUT 60)

# Event Streaming pattern tests
add_executable(test_event_streaming test_event_streaming.cpp)
target_link_libraries(test_event_streaming
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)
add_test(NAME test_event_streaming COMMAND test_event_streaming)
set_tests_properties(test_event_streaming PROPERTIES TIMEOUT 60)

# Message Pipeline pattern tests
add_executable(test_message_pipeline test_message_pipeline.cpp)
target_link_libraries(test_message_pipeline
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)
add_test(NAME test_message_pipeline COMMAND test_message_pipeline)
set_tests_properties(test_message_pipeline PROPERTIES TIMEOUT 60)
