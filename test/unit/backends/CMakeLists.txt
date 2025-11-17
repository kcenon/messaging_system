# Unit tests for messaging_system backends

# Test: standalone_backend
add_executable(test_standalone_backend
    test_standalone_backend.cpp
)

target_link_libraries(test_standalone_backend
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_standalone_backend COMMAND test_standalone_backend)
set_tests_properties(test_standalone_backend PROPERTIES TIMEOUT 60)

# Test: integration_backend
add_executable(test_integration_backend
    test_integration_backend.cpp
)

target_link_libraries(test_integration_backend
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_integration_backend COMMAND test_integration_backend)
set_tests_properties(test_integration_backend PROPERTIES TIMEOUT 60)
