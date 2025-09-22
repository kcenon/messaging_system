##################################################
# Container System Samples CMakeLists.txt
##################################################

cmake_minimum_required(VERSION 3.16)

# Sample executables
set(SAMPLE_PROGRAMS
    basic_usage
    thread_safe_example
    performance_benchmark
)

# Build each sample program with prefixed names to avoid conflicts
foreach(SAMPLE ${SAMPLE_PROGRAMS})
    add_executable(container_${SAMPLE} ${SAMPLE}.cpp)

    # Link with container library
    target_link_libraries(container_${SAMPLE}
        PRIVATE
            container_system
    )

    # Set C++20 standard
    set_target_properties(container_${SAMPLE} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    # Add compile definitions
    target_compile_definitions(container_${SAMPLE} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )

    # Platform-specific settings
    if(WIN32)
        target_compile_definitions(container_${SAMPLE} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()

    # Install sample executables
    install(TARGETS container_${SAMPLE}
        RUNTIME DESTINATION bin/samples
    )
endforeach()

# Create a combined sample runner
add_executable(container_run_all_samples run_all_samples.cpp)
target_link_libraries(container_run_all_samples PRIVATE container_system)
set_target_properties(container_run_all_samples PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

install(TARGETS container_run_all_samples
    RUNTIME DESTINATION bin/samples
)

# Sample data files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_BINARY_DIR}/README.md
    COPYONLY
)

install(FILES README.md
    DESTINATION share/samples
)