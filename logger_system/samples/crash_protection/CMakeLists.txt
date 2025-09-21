cmake_minimum_required(VERSION 3.16)

# Logger crash protection sample configuration
set(SAMPLE_NAME logger_crash_protection)
set(SAMPLE_DESCRIPTION "Logger Crash Protection Demonstration")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/logger
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/logger/writers
)

# Link libraries
target_link_libraries(${SAMPLE_NAME} PRIVATE
    logger_lib
    ${CMAKE_THREAD_LIBS_INIT}
)

# Platform-specific libraries and definitions
if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE dl)
    target_compile_definitions(${SAMPLE_NAME} PRIVATE 
        LINUX_PLATFORM=1
    )
elseif(APPLE)
    # macOS-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE 
        "-framework CoreFoundation"
    )
    target_compile_definitions(${SAMPLE_NAME} PRIVATE 
        APPLE_PLATFORM=1
    )
elseif(WIN32)
    # Windows-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE
        dbghelp
        psapi
        kernel32
    )
    target_compile_definitions(${SAMPLE_NAME} PRIVATE 
        WIN32_PLATFORM=1
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${SAMPLE_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -fno-omit-frame-pointer  # Better stack traces
        -g                       # Debug symbols
        -fasynchronous-unwind-tables  # Better crash handling
    )
    
    # Enable additional crash safety features for debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${SAMPLE_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
        )
        target_link_options(${SAMPLE_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
        )
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${SAMPLE_NAME} PRIVATE
        /W4
        /Zi          # Debug information
        /EHsc        # Exception handling
        /guard:cf    # Control flow guard
    )
    
    # Enable additional crash safety for debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${SAMPLE_NAME} PRIVATE
            /RTC1    # Runtime checks
            /MDd     # Debug runtime
        )
    endif()
endif()

# Set output directory
set_target_properties(${SAMPLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples
)

# Create necessary directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/samples/logs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/samples/backups)

# Add custom target for running the sample
add_custom_target(run_${SAMPLE_NAME}
    COMMAND ${CMAKE_BINARY_DIR}/samples/${SAMPLE_NAME}
    DEPENDS ${SAMPLE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/samples
    COMMENT "Running ${SAMPLE_DESCRIPTION}"
)

# Add custom target for running with crash simulation
add_custom_target(run_${SAMPLE_NAME}_with_crashes
    COMMAND ${CMAKE_BINARY_DIR}/samples/${SAMPLE_NAME} --enable-crashes
    DEPENDS ${SAMPLE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/samples
    COMMENT "Running ${SAMPLE_DESCRIPTION} with crash simulation"
)

# Add custom target for log analysis
add_custom_target(analyze_${SAMPLE_NAME}_logs
    COMMAND echo "Analyzing log files..." && 
            find ${CMAKE_BINARY_DIR}/samples/logs -name "*.log" -exec echo "File: {}" \; -exec head -10 {} \;
    DEPENDS run_${SAMPLE_NAME}
    COMMENT "Analyzing generated log files"
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

# Install sample configuration files
install(FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    DESTINATION samples/source/${SAMPLE_NAME}
    COMPONENT samples
    OPTIONAL
)