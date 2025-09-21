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
)

# Chat Server Sample
add_executable(chat_server chat_server.cpp)
target_link_libraries(chat_server PRIVATE
    messaging_services
    container
    database
    network
    Threads::Threads
)

# Distributed Worker Sample
add_executable(distributed_worker distributed_worker.cpp)
target_link_libraries(distributed_worker PRIVATE
    messaging_services
    container
    database
    network
    Threads::Threads
)

# IoT Monitoring System Sample
add_executable(iot_monitoring iot_monitoring.cpp)
target_link_libraries(iot_monitoring PRIVATE
    messaging_services
    container
    database
    network
    Threads::Threads
)

# Microservices Orchestrator Sample
add_executable(microservices_orchestrator microservices_orchestrator.cpp)
target_link_libraries(microservices_orchestrator PRIVATE
    messaging_services
    container
    database
    network
    Threads::Threads
)

# Event-Driven Pipeline Sample
add_executable(event_pipeline event_pipeline.cpp)
target_link_libraries(event_pipeline PRIVATE
    messaging_services
    container
    database
    network
    Threads::Threads
)

# Set output directory for samples
set_target_properties(
    chat_server
    distributed_worker
    iot_monitoring
    microservices_orchestrator
    event_pipeline
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/samples
)

# Add installation rules
install(TARGETS
    chat_server
    distributed_worker
    iot_monitoring
    microservices_orchestrator
    event_pipeline
    RUNTIME DESTINATION bin/samples
)