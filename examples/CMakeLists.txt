# Examples CMakeLists.txt

message(STATUS "Configuring examples...")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/basic_messaging.cpp)
    add_executable(basic_messaging
        basic_messaging.cpp
    )

    target_link_libraries(basic_messaging
        PRIVATE
            messaging_system_core
    )

    message(STATUS "  Added example: basic_messaging")
endif()
