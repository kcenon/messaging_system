# Performance Benchmarks for Messaging System

# Benchmark: Message Creation
add_executable(bench_message_creation
    bench_message_creation.cpp
)

target_link_libraries(bench_message_creation
    PRIVATE
        messaging_system_core
)

target_include_directories(bench_message_creation
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Benchmark: Message Queue
add_executable(bench_message_queue
    bench_message_queue.cpp
)

target_link_libraries(bench_message_queue
    PRIVATE
        messaging_system_core
)

target_include_directories(bench_message_queue
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Benchmark: Topic Router
add_executable(bench_topic_router
    bench_topic_router.cpp
)

target_link_libraries(bench_topic_router
    PRIVATE
        messaging_system_core
)

target_include_directories(bench_topic_router
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Benchmark: Pub/Sub Throughput
add_executable(bench_pub_sub_throughput
    bench_pub_sub_throughput.cpp
)

target_link_libraries(bench_pub_sub_throughput
    PRIVATE
        messaging_system_core
)

target_include_directories(bench_pub_sub_throughput
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Benchmark: Request/Reply Latency
add_executable(bench_request_reply_latency
    bench_request_reply_latency.cpp
)

target_link_libraries(bench_request_reply_latency
    PRIVATE
        messaging_system_core
)

target_include_directories(bench_request_reply_latency
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
