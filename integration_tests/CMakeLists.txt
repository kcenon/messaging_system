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
