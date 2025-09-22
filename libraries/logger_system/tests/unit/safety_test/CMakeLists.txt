add_executable(crash_safety_test crash_safety_test.cpp)

target_link_libraries(crash_safety_test
    PRIVATE
        logger
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(crash_safety_test PRIVATE stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(crash_safety_test PRIVATE c++fs)
endif()

add_test(NAME crash_safety_test COMMAND crash_safety_test)

add_custom_target(crash_safety_test_run
    COMMAND crash_safety_test
    DEPENDS crash_safety_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest/safety_test
    COMMENT "Running crash safety unit tests"
)