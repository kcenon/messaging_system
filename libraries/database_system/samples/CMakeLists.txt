cmake_minimum_required(VERSION 3.16)

##################################################
# Database System Samples CMakeLists.txt
#
# Sample programs demonstrating database system capabilities
##################################################

project(database_system_samples
    VERSION 1.0.0
    DESCRIPTION "Database System Sample Programs"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Threads REQUIRED)

# vcpkg integration for fmt
find_package(fmt CONFIG QUIET)
if(NOT fmt_FOUND)
    message(WARNING "fmt library not found - samples will use basic iostream")
    # Define USE_IOSTREAM when fmt is not available
    add_compile_definitions(USE_IOSTREAM)
endif()

##################################################
# Sample Programs Configuration
##################################################

# List of all sample programs
set(SAMPLE_PROGRAMS
    basic_usage
    postgres_advanced
    connection_pool_demo
    run_all_samples
)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##################################################
# Build Each Sample Program
##################################################

foreach(SAMPLE ${SAMPLE_PROGRAMS})
    # Create executable for each sample with prefixed names to avoid conflicts
    add_executable(database_${SAMPLE} ${SAMPLE}.cpp)

    # Set target properties
    set_target_properties(database_${SAMPLE} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    # Include directories
    target_include_directories(database_${SAMPLE} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )

    # Link libraries
    target_link_libraries(database_${SAMPLE} PRIVATE
        database
        Threads::Threads
    )

    # Link fmt only if available
    if(fmt_FOUND)
        target_link_libraries(database_${SAMPLE} PRIVATE fmt::fmt)
    else()
        target_compile_definitions(database_${SAMPLE} PRIVATE USE_IOSTREAM)
    endif()
    
    # Compiler-specific options
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(database_${SAMPLE} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
        )
    elseif(MSVC)
        target_compile_options(database_${SAMPLE} PRIVATE
            /W4
            /wd4100  # unreferenced formal parameter
        )
    endif()

    # Platform-specific definitions
    if(WIN32)
        target_compile_definitions(database_${SAMPLE} PRIVATE
            _WIN32_WINNT=0x0601
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()

    message(STATUS "Database sample configured: database_${SAMPLE}")
endforeach()

##################################################
# Installation (Optional)
##################################################

# Install sample binaries
# Create a list of prefixed target names
set(PREFIXED_SAMPLE_PROGRAMS)
foreach(SAMPLE ${SAMPLE_PROGRAMS})
    list(APPEND PREFIXED_SAMPLE_PROGRAMS database_${SAMPLE})
endforeach()

install(TARGETS ${PREFIXED_SAMPLE_PROGRAMS}
    RUNTIME DESTINATION bin/samples
    COMPONENT samples
)

# Install sample source files for reference
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/basic_usage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/postgres_advanced.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_pool_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/run_all_samples.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    DESTINATION share/database_system/samples
    COMPONENT samples
)

##################################################
# Summary
##################################################

message(STATUS "Database System Samples configured:")
message(STATUS "  Sample programs: ${SAMPLE_PROGRAMS}")
message(STATUS "  Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Dependencies: database, fmt (optional), Threads")