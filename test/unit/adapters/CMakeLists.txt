# Unit tests for adapters

if(KCENON_WITH_NETWORK_SYSTEM)
    # WebSocket transport tests
    add_executable(test_websocket_transport test_websocket_transport.cpp)

    target_link_libraries(test_websocket_transport PRIVATE
        messaging_system_core
        GTest::GTest
        GTest::Main
    )

    target_include_directories(test_websocket_transport PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    add_test(NAME websocket_transport_tests COMMAND test_websocket_transport)

    set_target_properties(test_websocket_transport PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    # HTTP transport tests
    add_executable(test_http_transport test_http_transport.cpp)

    target_link_libraries(test_http_transport PRIVATE
        messaging_system_core
        GTest::GTest
        GTest::Main
    )

    target_include_directories(test_http_transport PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    add_test(NAME http_transport_tests COMMAND test_http_transport)

    set_target_properties(test_http_transport PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
