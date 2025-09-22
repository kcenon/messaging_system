cmake_minimum_required(VERSION 3.16)

# Monitoring crash protection sample configuration
set(SAMPLE_NAME monitoring_crash_protection)
set(SAMPLE_DESCRIPTION "Monitoring Crash Protection Demonstration")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/monitoring
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/storage
)

# Link libraries
target_link_libraries(${SAMPLE_NAME} PRIVATE
    monitoring_lib
    ${CMAKE_THREAD_LIBS_INIT}
)

# Platform-specific libraries and definitions
if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE 
        dl
        rt  # For shared memory and real-time features
    )
    target_compile_definitions(${SAMPLE_NAME} PRIVATE 
        LINUX_PLATFORM=1
        _GNU_SOURCE=1
    )
elseif(APPLE)
    # macOS-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE 
        "-framework CoreFoundation"
        "-framework IOKit"  # For system metrics
    )
    target_compile_definitions(${SAMPLE_NAME} PRIVATE 
        APPLE_PLATFORM=1
    )
elseif(WIN32)
    # Windows-specific libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE
        dbghelp
        psapi
        pdh      # Performance Data Helper
        kernel32
        advapi32
    )
    target_compile_definitions(${SAMPLE_NAME} PRIVATE 
        WIN32_PLATFORM=1
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${SAMPLE_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -fno-omit-frame-pointer  # Better stack traces
        -g                       # Debug symbols
        -fasynchronous-unwind-tables  # Better crash handling
        -funwind-tables          # Exception handling
    )
    
    # Enable additional monitoring safety features for debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${SAMPLE_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
            -fsanitize=thread    # Thread safety checking
        )
        target_link_options(${SAMPLE_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
            -fsanitize=thread
        )
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${SAMPLE_NAME} PRIVATE
        /W4
        /Zi          # Debug information
        /EHsc        # Exception handling
        /guard:cf    # Control flow guard
        /analyze     # Static analysis
    )
    
    # Enable additional safety for debug builds
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
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/samples/monitoring_data)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/samples/backups)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/samples/alerts)

# Add custom target for running the sample
add_custom_target(run_${SAMPLE_NAME}
    COMMAND ${CMAKE_BINARY_DIR}/samples/${SAMPLE_NAME}
    DEPENDS ${SAMPLE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/samples
    COMMENT "Running ${SAMPLE_DESCRIPTION}"
)

# Add custom target for running with stress testing
add_custom_target(run_${SAMPLE_NAME}_stress
    COMMAND ${CMAKE_BINARY_DIR}/samples/${SAMPLE_NAME} --stress-test
    DEPENDS ${SAMPLE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/samples
    COMMENT "Running ${SAMPLE_DESCRIPTION} with stress testing"
)

# Add custom target for monitoring data analysis
add_custom_target(analyze_${SAMPLE_NAME}_data
    COMMAND echo "Analyzing monitoring data..." && 
            find ${CMAKE_BINARY_DIR}/samples/monitoring_data -name "*.dat" -exec echo "File: {}" \; -exec ls -la {} \; &&
            find ${CMAKE_BINARY_DIR}/samples/backups -name "*.backup" -exec echo "Backup: {}" \; -exec ls -la {} \;
    DEPENDS run_${SAMPLE_NAME}
    COMMENT "Analyzing generated monitoring data"
)

# Add custom target for crash simulation
add_custom_target(run_${SAMPLE_NAME}_crash_sim
    COMMAND echo "Running crash simulation..." &&
            timeout 30 ${CMAKE_BINARY_DIR}/samples/${SAMPLE_NAME} --enable-crashes || echo "Crash simulation completed"
    DEPENDS ${SAMPLE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/samples
    COMMENT "Running crash simulation test"
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

# Install monitoring configuration template
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/monitoring_config.json
    DESTINATION samples/config
    COMPONENT samples
    OPTIONAL
)