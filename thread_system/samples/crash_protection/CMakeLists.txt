cmake_minimum_required(VERSION 3.16)

# Crash protection sample configuration
set(SAMPLE_NAME crash_protection)
set(SAMPLE_DESCRIPTION "Crash Protection Demonstration")

# Source files
set(SAMPLE_SOURCES
    main.cpp
)

# Create executable
add_executable(${SAMPLE_NAME} ${SAMPLE_SOURCES})

# Set C++ standard
set_target_properties(${SAMPLE_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(${SAMPLE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/thread_pool
)

# Link libraries
target_link_libraries(${SAMPLE_NAME} PRIVATE
    thread_pool
    ${CMAKE_THREAD_LIBS_INIT}
)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE dl)
elseif(APPLE)
    # macOS-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE 
        "-framework CoreFoundation"
    )
elseif(WIN32)
    # Windows-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE
        dbghelp
        psapi
    )
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${SAMPLE_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -fno-omit-frame-pointer  # Better stack traces
        -g                       # Debug symbols for crash analysis
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${SAMPLE_NAME} PRIVATE
        /W4
        /Zi  # Debug information
    )
endif()

# Set output directory
set_target_properties(${SAMPLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples
)

# Create crash logs directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/samples/crash_logs)

# Add custom target for running the sample
add_custom_target(run_${SAMPLE_NAME}
    COMMAND ${CMAKE_BINARY_DIR}/samples/${SAMPLE_NAME}
    DEPENDS ${SAMPLE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/samples
    COMMENT "Running ${SAMPLE_DESCRIPTION}"
)

# Installation
install(TARGETS ${SAMPLE_NAME}
    RUNTIME DESTINATION samples
    COMPONENT samples
)

# Install sample source for reference
install(FILES ${SAMPLE_SOURCES}
    DESTINATION samples/source/${SAMPLE_NAME}
    COMPONENT samples
)