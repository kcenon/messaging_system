##################################################
# Container System Examples CMakeLists.txt
##################################################

cmake_minimum_required(VERSION 3.16)

# Example executables
set(EXAMPLE_PROGRAMS
    basic_container_example
    messaging_integration_example
    advanced_container_example
    real_world_scenarios
)

# Build each example program
foreach(EXAMPLE ${EXAMPLE_PROGRAMS})
    add_executable(${EXAMPLE} ${EXAMPLE}.cpp)

    # Link with container library
    target_link_libraries(${EXAMPLE}
        PRIVATE
            container_system
            Threads::Threads
    )

    # Set C++20 standard
    set_target_properties(${EXAMPLE} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    # Include directories
    target_include_directories(${EXAMPLE} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    # Add compile definitions based on enabled features
    if(ENABLE_MESSAGING_FEATURES)
        target_compile_definitions(${EXAMPLE} PRIVATE HAS_MESSAGING_FEATURES)
    endif()

    if(ENABLE_PERFORMANCE_METRICS)
        target_compile_definitions(${EXAMPLE} PRIVATE HAS_PERFORMANCE_METRICS)
    endif()

    if(ENABLE_EXTERNAL_INTEGRATION)
        target_compile_definitions(${EXAMPLE} PRIVATE HAS_EXTERNAL_INTEGRATION)
    endif()

    # Debug/Release configurations
    target_compile_definitions(${EXAMPLE} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )

    # Platform-specific settings
    if(WIN32)
        target_compile_definitions(${EXAMPLE} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()

    # Install example executables
    install(TARGETS ${EXAMPLE}
        RUNTIME DESTINATION bin/examples
    )

    message(STATUS "Container Example: ${EXAMPLE} configured")
endforeach()

# Create example README
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_BINARY_DIR}/README.md
    COPYONLY
)

install(FILES README.md
    DESTINATION share/examples
)