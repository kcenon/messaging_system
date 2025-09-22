# Basic integration test - only available in integration mode
if(NOT LOGGER_STANDALONE_MODE)
    add_executable(basic_integration_test integration_test.cpp)

    # Link with GTest
    if(TARGET GTest::gtest_main)
        target_link_libraries(basic_integration_test PRIVATE LoggerSystem GTest::gtest_main)
    else()
        target_link_libraries(basic_integration_test PRIVATE LoggerSystem gtest_main)
    endif()

    add_test(NAME basic_integration_test COMMAND basic_integration_test)
    set_target_properties(basic_integration_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
else()
    message(STATUS "Integration tests skipped in standalone mode")
endif()

# Thread system integration test - only available in integration mode
if(NOT LOGGER_STANDALONE_MODE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration/thread_system_integration_test.cpp")
    add_executable(thread_system_integration_test integration/thread_system_integration_test.cpp)

    if(TARGET GTest::gtest_main)
        target_link_libraries(thread_system_integration_test PRIVATE LoggerSystem GTest::gtest_main)
    else()
        target_link_libraries(thread_system_integration_test PRIVATE LoggerSystem gtest_main)
    endif()

    if(USE_THREAD_SYSTEM)
        target_compile_definitions(thread_system_integration_test PRIVATE USE_THREAD_SYSTEM)
    endif()

    add_test(NAME thread_system_integration_test COMMAND thread_system_integration_test)
    set_target_properties(thread_system_integration_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
else()
    message(STATUS "Thread system integration test skipped in standalone mode")
endif()

# Monitoring system integration test
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration/monitoring_integration_test.cpp")
    add_executable(monitoring_integration_test integration/monitoring_integration_test.cpp)

    if(TARGET GTest::gtest_main)
        target_link_libraries(monitoring_integration_test PRIVATE LoggerSystem GTest::gtest_main)
    else()
        target_link_libraries(monitoring_integration_test PRIVATE LoggerSystem gtest_main)
    endif()

    add_test(NAME monitoring_integration_test COMMAND monitoring_integration_test)
    set_target_properties(monitoring_integration_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# Version compatibility test
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/compatibility/version_compatibility_test.cpp")
    add_executable(version_compatibility_test compatibility/version_compatibility_test.cpp)

    if(TARGET GTest::gtest_main)
        target_link_libraries(version_compatibility_test PRIVATE LoggerSystem GTest::gtest_main)
    else()
        target_link_libraries(version_compatibility_test PRIVATE LoggerSystem gtest_main)
    endif()

    add_test(NAME version_compatibility_test COMMAND version_compatibility_test)
    set_target_properties(version_compatibility_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()