# Logger integration test
add_executable(logger_integration_test integration_test.cpp)
target_link_libraries(logger_integration_test PRIVATE logger gtest_main)
add_test(NAME logger_integration_test COMMAND logger_integration_test)
set_target_properties(logger_integration_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)