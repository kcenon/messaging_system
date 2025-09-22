# Integration test
add_executable(integration_test integration_test.cpp)
target_link_libraries(integration_test PRIVATE logger gtest_main)
add_test(NAME integration_test COMMAND integration_test)
set_target_properties(integration_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)