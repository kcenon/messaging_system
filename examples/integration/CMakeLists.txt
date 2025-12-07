# messaging_system integration examples
cmake_minimum_required(VERSION 3.16)

# Common settings for all examples
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Task Event Bridge Example
add_executable(task_event_bridge_example
    example_task_event_bridge.cpp
)
target_link_libraries(task_event_bridge_example PRIVATE
    messaging_system_core
)
target_include_directories(task_event_bridge_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Install targets
install(TARGETS
    task_event_bridge_example
    RUNTIME DESTINATION bin/examples/messaging
)
