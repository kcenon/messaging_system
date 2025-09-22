cmake_minimum_required(VERSION 3.16)

##################################################
# Network System Samples CMakeLists.txt
#
# Sample programs demonstrating network system capabilities
##################################################

project(network_system_samples
    VERSION 1.0.0
    DESCRIPTION "Network System Sample Programs"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Threads REQUIRED)

# vcpkg integration for fmt
find_package(fmt CONFIG REQUIRED)

##################################################
# Sample Programs Configuration
##################################################

# List of all sample programs
set(SAMPLE_PROGRAMS
    basic_usage
    tcp_server_client
    http_client_demo
    run_all_samples
)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##################################################
# Build Each Sample Program
##################################################

foreach(SAMPLE ${SAMPLE_PROGRAMS})
    # Create executable for each sample
    add_executable(${SAMPLE} ${SAMPLE}.cpp)
    
    # Set target properties
    set_target_properties(${SAMPLE} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    
    # Include directories (link to NetworkSystem target provides proper includes)
    target_include_directories(${SAMPLE} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    # Link libraries
    target_link_libraries(${SAMPLE} PRIVATE
        NetworkSystem
        fmt::fmt
        Threads::Threads
    )
    
    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(${SAMPLE} PRIVATE
            ws2_32
            wsock32
        )
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(${SAMPLE} PRIVATE
            pthread
        )
    endif()
    
    # Compiler-specific options
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${SAMPLE} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
        )
    elseif(MSVC)
        target_compile_options(${SAMPLE} PRIVATE
            /W4
            /wd4100  # unreferenced formal parameter
        )
    endif()
    
    # Platform-specific definitions
    if(WIN32)
        target_compile_definitions(${SAMPLE} PRIVATE
            _WIN32_WINNT=0x0601
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()
    
    message(STATUS "Network sample configured: ${SAMPLE}")
endforeach()

##################################################
# HTTP Client Dependencies (Optional)
##################################################

# If HTTP functionality requires additional libraries (e.g., curl)
find_package(CURL QUIET)
if(CURL_FOUND)
    target_link_libraries(http_client_demo PRIVATE ${CURL_LIBRARIES})
    target_include_directories(http_client_demo PRIVATE ${CURL_INCLUDE_DIRS})
    target_compile_definitions(http_client_demo PRIVATE HAVE_CURL)
    message(STATUS "HTTP client demo will use libcurl")
else()
    message(STATUS "HTTP client demo will use built-in HTTP implementation")
endif()

##################################################
# Installation (Optional)
##################################################

# Install sample binaries
install(TARGETS ${SAMPLE_PROGRAMS}
    RUNTIME DESTINATION bin/samples
    COMPONENT samples
)

# Install sample source files for reference
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/basic_usage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp_server_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/http_client_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/run_all_samples.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    DESTINATION share/network_system/samples
    COMPONENT samples
)

##################################################
# Summary
##################################################

message(STATUS "Network System Samples configured:")
message(STATUS "  Sample programs: ${SAMPLE_PROGRAMS}")
message(STATUS "  Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Dependencies: network_system, fmt, Threads")
if(CURL_FOUND)
    message(STATUS "  HTTP library: libcurl")
else()
    message(STATUS "  HTTP library: built-in")
endif()