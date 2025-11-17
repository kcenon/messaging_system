# DI Container Unit Tests

add_executable(test_messaging_di_container
    test_messaging_di_container.cpp
)

target_link_libraries(test_messaging_di_container
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME MessagingDIContainer COMMAND test_messaging_di_container)

add_executable(test_service_registry
    test_service_registry.cpp
)

target_link_libraries(test_service_registry
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME ServiceRegistry COMMAND test_service_registry)
