# Integration Tests for Messaging System

# Find GTest
find_package(GTest REQUIRED)

# Test message bus + router
add_executable(test_message_bus_router
    test_message_bus_router.cpp
)

target_link_libraries(test_message_bus_router
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_message_bus_router
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(test_message_bus_router)

# Test priority queue
add_executable(test_priority_queue
    test_priority_queue.cpp
)

target_link_libraries(test_priority_queue
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_priority_queue
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(test_priority_queue)

# Test backend integration
add_executable(test_backend_integration
    test_backend_integration.cpp
)

target_link_libraries(test_backend_integration
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_backend_integration
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(test_backend_integration)

# Test full integration
add_executable(test_full_integration
    test_full_integration.cpp
)

target_link_libraries(test_full_integration
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_full_integration
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(test_full_integration)

# Test event bus integration (TICKET-011)
add_executable(test_event_bus_integration
    test_event_bus_integration.cpp
)

target_link_libraries(test_event_bus_integration
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_event_bus_integration
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(test_event_bus_integration)

# Test container serialization (TICKET-011)
add_executable(test_container_serialization
    test_container_serialization.cpp
)

target_link_libraries(test_container_serialization
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_container_serialization
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(test_container_serialization)

# ============================================================================
# Messaging Patterns E2E Tests (Issue #161)
# ============================================================================

# Test messaging patterns end-to-end
add_executable(test_messaging_patterns_e2e
    test_messaging_patterns_e2e.cpp
)

target_link_libraries(test_messaging_patterns_e2e
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_messaging_patterns_e2e
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(test_messaging_patterns_e2e)

# ============================================================================
# Task Module Integration Tests (Issue #118)
# ============================================================================

# Test task lifecycle
add_executable(test_task_lifecycle
    task/test_task_lifecycle.cpp
)

target_link_libraries(test_task_lifecycle
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_task_lifecycle
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/task
)

gtest_discover_tests(test_task_lifecycle)

# Test worker scenarios
add_executable(test_worker_scenarios
    task/test_worker_scenarios.cpp
)

target_link_libraries(test_worker_scenarios
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_worker_scenarios
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/task
)

gtest_discover_tests(test_worker_scenarios)

# Test failure recovery
add_executable(test_failure_recovery
    task/test_failure_recovery.cpp
)

target_link_libraries(test_failure_recovery
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_failure_recovery
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/task
)

gtest_discover_tests(test_failure_recovery)

# Test scheduling
add_executable(test_scheduling
    task/test_scheduling.cpp
)

target_link_libraries(test_scheduling
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_scheduling
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/task
)

gtest_discover_tests(test_scheduling)

# Test concurrent load
add_executable(test_concurrent_load
    task/test_concurrent_load.cpp
)

target_link_libraries(test_concurrent_load
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_concurrent_load
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/task
)

gtest_discover_tests(test_concurrent_load)

# Test timing accuracy (Issue #161)
add_executable(test_timing_accuracy
    task/test_timing_accuracy.cpp
)

target_link_libraries(test_timing_accuracy
    PRIVATE
        messaging_system_core
        GTest::gtest_main
)

target_include_directories(test_timing_accuracy
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/task
)

gtest_discover_tests(test_timing_accuracy)
