project(integration_example)

# Create the example executable
add_executable(${PROJECT_NAME} 
    integration_example.cpp
    mock_logger.h
    mock_monitoring.h
)

# Link against required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    thread_base
    interfaces
    utilities
)

# Include new headers for interfaces, core and thread_pool
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../../implementations/thread_pool/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/base/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/jobs/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/sync/include
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Set C++ standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
