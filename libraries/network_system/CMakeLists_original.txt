cmake_minimum_required(VERSION 3.16)

##################################################
# Network System CMakeLists.txt
# 
# Independent network system with high-performance messaging
# Provides TCP/UDP communication and client/server implementation.
##################################################

# Project definition
project(network_system 
    VERSION 1.0.0
    DESCRIPTION "Advanced C++20 Network System with High-Performance Messaging"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(USE_UNIT_TEST "Use unit test" ON)
option(BUILD_NETWORK_SAMPLES "Build network system samples" ON)
option(USE_SSL "Enable SSL/TLS support" OFF)
option(USE_EPOLL "Enable epoll support (Linux)" ON)
option(USE_KQUEUE "Enable kqueue support (macOS/BSD)" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10
elseif(APPLE)
    add_definitions(-DAPPLE_PLATFORM)
elseif(UNIX)
    add_definitions(-DLINUX_PLATFORM)
endif()

# Find required packages
find_package(Threads REQUIRED)

##################################################
# Source Files Configuration
##################################################

# Main include file
set(MAIN_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/network.h
)

# Core files (public API)
set(CORE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/messaging_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/messaging_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/messaging_server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/messaging_server.cpp
)

# Session files (public API)
set(SESSION_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/session/messaging_session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/session/messaging_session.cpp
)

# Internal implementation files
set(INTERNAL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/common_defs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/tcp_socket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/tcp_socket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/send_coroutine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/send_coroutine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/pipeline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/pipeline.cpp
)

# Combine all files
set(ALL_FILES
    ${MAIN_HEADER}
    ${CORE_FILES}
    ${SESSION_FILES}
    ${INTERNAL_FILES}
)

##################################################
# Library Target Configuration
##################################################

# Create static library
add_library(network_system STATIC 
    ${ALL_FILES}
)

# Create alias for consistent naming
add_library(NetworkSystem::network ALIAS network_system)

# Set target properties
set_target_properties(network_system PROPERTIES
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

##################################################
# Include Directories
##################################################

# Public include directories (for consumers of this library)
target_include_directories(network_system PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<INSTALL_INTERFACE:include>
)

##################################################
# Dependencies
##################################################

# vcpkg integration
find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(network_system
    PUBLIC
        Threads::Threads
        asio::asio
        fmt::fmt
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(network_system PUBLIC ws2_32 wsock32)
elseif(UNIX)
    # Unix systems typically don't need additional socket libraries
endif()

if(USE_SSL)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(network_system
        PUBLIC
            OpenSSL::SSL
            OpenSSL::Crypto
    )
    target_compile_definitions(network_system PUBLIC USE_SSL)
endif()

##################################################
# Platform-Specific Configuration
##################################################

# Linux epoll support
if(UNIX AND NOT APPLE AND USE_EPOLL)
    target_compile_definitions(network_system PRIVATE USE_EPOLL)
endif()

# macOS/BSD kqueue support
if((APPLE OR CMAKE_SYSTEM_NAME MATCHES "BSD") AND USE_KQUEUE)
    target_compile_definitions(network_system PRIVATE USE_KQUEUE)
endif()

# ASIO configuration
target_compile_definitions(network_system PUBLIC
    ASIO_STANDALONE
    ASIO_NO_DEPRECATED
)

# Enable coroutines support if available
if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    target_compile_definitions(network_system PUBLIC
        ASIO_HAS_CO_AWAIT
        ASIO_HAS_STD_COROUTINE
    )
endif()

##################################################
# Compiler Options
##################################################

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(network_system PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-missing-field-initializers
    )
elseif(MSVC)
    target_compile_options(network_system PRIVATE
        /W4
        /WX-
        /wd4100  # unreferenced formal parameter
        /wd4458  # declaration hides class member
        /wd4996  # deprecated functions
    )
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(network_system PRIVATE
        _WIN32_WINNT=0x0601  # Windows 7 or later
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    )
endif()

##################################################
# Performance Optimizations
##################################################

# Release mode optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(network_system PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-DNDEBUG>
        $<$<CONFIG:Release>:-march=native>
    )
elseif(MSVC)
    target_compile_options(network_system PRIVATE
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/DNDEBUG>
    )
endif()

##################################################
# Installation Rules (when not building as submodule)
##################################################

# Installation is handled by parent CMakeLists.txt

# Export configuration is handled by parent CMakeLists.txt

##################################################
# Testing Support
##################################################

# Enable testing features if building tests
if(BUILD_UNIT_TESTS AND NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
    # Add any network-specific test configurations here
    set_property(TARGET network_system PROPERTY ENABLE_EXPORTS ON)
endif()

##################################################
# Samples
##################################################

if(BUILD_NETWORK_SAMPLES)
    add_subdirectory(samples)
    message(STATUS "Network samples will be built")
endif()

##################################################
# Summary
##################################################

message(STATUS "Network library configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Headers: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Dependencies: asio, fmt, Threads")
message(STATUS "  Features:")
message(STATUS "    - SSL/TLS support: ${USE_SSL}")
if(UNIX AND NOT APPLE)
    message(STATUS "    - epoll support: ${USE_EPOLL}")
elseif(APPLE)
    message(STATUS "    - kqueue support: ${USE_KQUEUE}")
endif()
message(STATUS "    - High-performance networking: ON")
message(STATUS "    - Cross-platform compatibility: ON")
message(STATUS "    - Build samples: ${BUILD_NETWORK_SAMPLES}")