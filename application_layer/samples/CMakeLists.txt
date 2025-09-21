# Application Layer Sample Programs
cmake_minimum_required(VERSION 3.16)
project(application_layer_samples)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/application_layer/include
    ${CMAKE_SOURCE_DIR}/libraries
    ${CMAKE_SOURCE_DIR}/libraries/logger_system/sources
)

# Basic Usage Example
add_executable(basic_usage_example basic_usage_example.cpp)
target_link_libraries(basic_usage_example PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# Chat Server Sample
add_executable(chat_server chat_server.cpp)
target_link_libraries(chat_server PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# Distributed Worker Sample
add_executable(distributed_worker distributed_worker.cpp)
target_link_libraries(distributed_worker PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# IoT Monitoring System Sample
add_executable(iot_monitoring iot_monitoring.cpp)
target_link_libraries(iot_monitoring PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# Microservices Orchestrator Sample
add_executable(microservices_orchestrator microservices_orchestrator.cpp)
target_link_libraries(microservices_orchestrator PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# Event-Driven Pipeline Sample
add_executable(event_pipeline event_pipeline.cpp)
target_link_libraries(event_pipeline PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# Message Bus Benchmark
add_executable(message_bus_benchmark message_bus_benchmark.cpp)
target_link_libraries(message_bus_benchmark PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# Production-Ready Example with advanced features
add_executable(production_ready_example production_ready_example.cpp)
target_link_libraries(production_ready_example PRIVATE
    messaging_services
    container
    database
    network
    logger
    Threads::Threads
)

# Set output directory for samples
set_target_properties(
    basic_usage_example
    chat_server
    distributed_worker
    iot_monitoring
    microservices_orchestrator
    event_pipeline
    message_bus_benchmark
    production_ready_example
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/samples
)

# Add installation rules
install(TARGETS
    basic_usage_example
    chat_server
    distributed_worker
    iot_monitoring
    microservices_orchestrator
    event_pipeline
    message_bus_benchmark
    production_ready_example
    RUNTIME DESTINATION bin/samples
)