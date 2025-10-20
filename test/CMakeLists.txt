# Test CMakeLists.txt

message(STATUS "Configuring tests...")

# Simple unit test (no GTest dependency yet)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit/core/test_messaging_container.cpp)
    add_executable(test_messaging_container
        unit/core/test_messaging_container.cpp
    )

    target_link_libraries(test_messaging_container
        PRIVATE
            messaging_system_core
    )

    # Add as a test
    add_test(NAME test_messaging_container COMMAND test_messaging_container)

    message(STATUS "  Added test: test_messaging_container")
endif()

# TODO (Phase 4): Add GTest integration
# TODO (Phase 4): Add benchmark integration
# TODO (Phase 4): Add integration tests
