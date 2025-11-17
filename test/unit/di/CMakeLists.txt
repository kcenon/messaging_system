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
set_tests_properties(MessagingDIContainer PROPERTIES TIMEOUT 60)

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
set_tests_properties(ServiceRegistry PROPERTIES TIMEOUT 60)
