# Test CMakeLists.txt

message(STATUS "Configuring tests...")

# ============================================================================
# Unit Tests - Core Components
# ============================================================================

# MessagingContainer tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/core/test_messaging_container.cpp)
    add_executable(test_messaging_container
        unit/core/test_messaging_container.cpp
    )

    target_link_libraries(test_messaging_container
        PRIVATE
            messaging_system_core
    )

    add_test(NAME test_messaging_container COMMAND test_messaging_container)
    message(STATUS "  Added test: test_messaging_container")
endif()

# TopicRouter tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/core/test_topic_router.cpp)
    add_executable(test_topic_router
        unit/core/test_topic_router.cpp
    )

    target_link_libraries(test_topic_router
        PRIVATE
            messaging_system_core
    )

    add_test(NAME test_topic_router COMMAND test_topic_router)
    message(STATUS "  Added test: test_topic_router")
endif()

# MessageBus tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/core/test_message_bus.cpp)
    add_executable(test_message_bus
        unit/core/test_message_bus.cpp
    )

    target_link_libraries(test_message_bus
        PRIVATE
            messaging_system_core
    )

    add_test(NAME test_message_bus COMMAND test_message_bus)
    message(STATUS "  Added test: test_message_bus")
endif()

# ============================================================================
# Unit Tests - Integration Components
# ============================================================================

# TraceContext tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/integration/test_trace_context.cpp)
    add_executable(test_trace_context
        unit/integration/test_trace_context.cpp
    )

    target_link_libraries(test_trace_context
        PRIVATE
            messaging_system_core
    )

    add_test(NAME test_trace_context COMMAND test_trace_context)
    message(STATUS "  Added test: test_trace_context")
endif()

# ConfigLoader tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/integration/test_config_loader.cpp)
    add_executable(test_config_loader
        unit/integration/test_config_loader.cpp
    )

    target_link_libraries(test_config_loader
        PRIVATE
            messaging_system_core
    )

    add_test(NAME test_config_loader COMMAND test_config_loader)
    message(STATUS "  Added test: test_config_loader")
endif()

# ============================================================================
# Integration Tests
# ============================================================================

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration/test_end_to_end.cpp)
    add_executable(test_end_to_end
        integration/test_end_to_end.cpp
    )

    target_link_libraries(test_end_to_end
        PRIVATE
            messaging_system_core
    )

    add_test(NAME test_end_to_end COMMAND test_end_to_end)
    message(STATUS "  Added integration test: test_end_to_end")
endif()

# ============================================================================
# Test Summary
# ============================================================================

message(STATUS "")
message(STATUS "=== Test Configuration Summary ===")
message(STATUS "Unit tests configured: 5")
message(STATUS "  - test_messaging_container")
message(STATUS "  - test_topic_router")
message(STATUS "  - test_message_bus")
message(STATUS "  - test_trace_context")
message(STATUS "  - test_config_loader")
message(STATUS "")
message(STATUS "Integration tests configured: 1")
message(STATUS "  - test_end_to_end")
message(STATUS "")
message(STATUS "Run tests with:")
message(STATUS "  ctest --test-dir <build-dir>")
message(STATUS "  or: ctest --preset default")
message(STATUS "==================================")
message(STATUS "")
