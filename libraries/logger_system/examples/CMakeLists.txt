# Basic usage sample - available when samples are enabled
if(BUILD_SAMPLES)
    add_executable(logger_basic_usage basic_usage.cpp)
    target_link_libraries(logger_basic_usage PRIVATE LoggerSystem)
    set_target_properties(logger_basic_usage PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# Advanced examples - only available in integration mode and when samples are enabled
if(NOT LOGGER_STANDALONE_MODE AND BUILD_SAMPLES)
    # Metrics demo sample
    add_executable(logger_metrics_demo metrics_demo.cpp)
    target_link_libraries(logger_metrics_demo PRIVATE LoggerSystem)
    set_target_properties(logger_metrics_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Advanced features demo sample
    add_executable(logger_advanced_features_demo advanced_features_demo.cpp)
    target_link_libraries(logger_advanced_features_demo PRIVATE LoggerSystem)
    set_target_properties(logger_advanced_features_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Distributed logging demo sample
    add_executable(logger_distributed_logging_demo distributed_logging_demo.cpp)
    target_link_libraries(logger_distributed_logging_demo PRIVATE LoggerSystem)
    set_target_properties(logger_distributed_logging_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Security demo sample - disabled due to missing dependencies
    # add_executable(logger_security_demo security_demo.cpp)
    # target_link_libraries(logger_security_demo PRIVATE LoggerSystem)
    # set_target_properties(logger_security_demo PROPERTIES
    #     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    # )

    # Migration example sample - disabled due to missing dependencies
    # add_executable(logger_migration_example migration_example.cpp)
    # target_link_libraries(logger_migration_example PRIVATE LoggerSystem)
    # set_target_properties(logger_migration_example PROPERTIES
    #     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    # )
else()
    message(STATUS "Advanced examples skipped in standalone mode")
endif()