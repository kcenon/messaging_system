# Examples directory CMakeLists.txt

# Include directories for examples
include_directories(
    ${CMAKE_SOURCE_DIR}/sources
)

# Basic monitoring example
add_executable(monitoring_basic_monitoring_example
    basic_monitoring_example.cpp
)
target_link_libraries(monitoring_basic_monitoring_example
    PRIVATE monitoring_system
    Threads::Threads
)

# Distributed tracing example
add_executable(monitoring_distributed_tracing_example
    distributed_tracing_example.cpp
)
target_link_libraries(monitoring_distributed_tracing_example
    PRIVATE monitoring_system
    Threads::Threads
)

# Health monitoring and reliability example
add_executable(monitoring_health_reliability_example
    health_reliability_example.cpp
)
target_link_libraries(monitoring_health_reliability_example
    PRIVATE monitoring_system
    Threads::Threads
)

# Result pattern example (existing)
add_executable(monitoring_result_pattern_example
    result_pattern_example.cpp
)
target_link_libraries(monitoring_result_pattern_example
    PRIVATE monitoring_system
    Threads::Threads
)