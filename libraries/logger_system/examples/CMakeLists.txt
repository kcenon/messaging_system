# Basic usage sample - available when samples are enabled
if(BUILD_SAMPLES)
    add_executable(basic_usage basic_usage.cpp)
    target_link_libraries(basic_usage PRIVATE LoggerSystem)
    set_target_properties(basic_usage PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# Advanced examples - only available in integration mode and when samples are enabled
if(NOT LOGGER_STANDALONE_MODE AND BUILD_SAMPLES)
    # Metrics demo sample
    add_executable(metrics_demo metrics_demo.cpp)
    target_link_libraries(metrics_demo PRIVATE LoggerSystem)
    set_target_properties(metrics_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Advanced features demo sample
    add_executable(advanced_features_demo advanced_features_demo.cpp)
    target_link_libraries(advanced_features_demo PRIVATE LoggerSystem)
    set_target_properties(advanced_features_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Distributed logging demo sample
    add_executable(distributed_logging_demo distributed_logging_demo.cpp)
    target_link_libraries(distributed_logging_demo PRIVATE LoggerSystem)
    set_target_properties(distributed_logging_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Security demo sample
    add_executable(security_demo security_demo.cpp)
    target_link_libraries(security_demo PRIVATE LoggerSystem)
    set_target_properties(security_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Migration example sample
    add_executable(migration_example migration_example.cpp)
    target_link_libraries(migration_example PRIVATE LoggerSystem)
    set_target_properties(migration_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
else()
    message(STATUS "Advanced examples skipped in standalone mode")
endif()