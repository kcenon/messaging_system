# messaging_system pattern examples
cmake_minimum_required(VERSION 3.16)

# Common settings for all examples
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Request-Reply Pattern Example
add_executable(request_reply_example
    request_reply/example_request_reply.cpp
)
target_link_libraries(request_reply_example PRIVATE
    messaging_system_core
)
target_include_directories(request_reply_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Pub-Sub Pattern Example
add_executable(pub_sub_example
    pub_sub/example_pub_sub.cpp
)
target_link_libraries(pub_sub_example PRIVATE
    messaging_system_core
)
target_include_directories(pub_sub_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Event Streaming Pattern Example
add_executable(event_streaming_example
    event_streaming/example_event_streaming.cpp
)
target_link_libraries(event_streaming_example PRIVATE
    messaging_system_core
)
target_include_directories(event_streaming_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Message Pipeline Pattern Example
add_executable(message_pipeline_example
    message_pipeline/example_message_pipeline.cpp
)
target_link_libraries(message_pipeline_example PRIVATE
    messaging_system_core
)
target_include_directories(message_pipeline_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Install targets
install(TARGETS
    request_reply_example
    pub_sub_example
    event_streaming_example
    message_pipeline_example
    RUNTIME DESTINATION bin/examples/messaging
)
