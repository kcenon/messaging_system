# Unit tests for messaging_system integration

# Test: messaging_health_checks
add_executable(test_messaging_health_checks
    test_messaging_health_checks.cpp
)

target_link_libraries(test_messaging_health_checks
    PRIVATE
        messaging_system_core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME test_messaging_health_checks COMMAND test_messaging_health_checks)
set_tests_properties(test_messaging_health_checks PROPERTIES TIMEOUT 60)
