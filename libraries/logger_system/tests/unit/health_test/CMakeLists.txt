add_executable(health_check_test health_check_test.cpp)

target_link_libraries(health_check_test
    PRIVATE
        logger
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
)

add_test(NAME health_check_test COMMAND health_check_test)

add_custom_target(health_check_test_run
    COMMAND health_check_test
    DEPENDS health_check_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest/health_test
    COMMENT "Running health check unit tests"
)