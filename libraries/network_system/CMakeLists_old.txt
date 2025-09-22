##################################################
# Network Module CMakeLists.txt
# 
# Builds the network library for the messaging system.
# Provides high-performance asynchronous networking with ASIO.
##################################################

# Project definition
project(network 
    VERSION 1.0.0
    DESCRIPTION "High-performance asynchronous network messaging library"
    LANGUAGES CXX
)

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
add_library(${PROJECT_NAME} STATIC 
    ${ALL_FILES}
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
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
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include/messaging_system>
)

##################################################
# Dependencies
##################################################

# ASIO dependency
find_package(asio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC asio::asio)

# fmt dependency
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)

# Container library dependency
if(TARGET container)
    target_link_libraries(${PROJECT_NAME} PUBLIC container)
else()
    message(FATAL_ERROR "Container library is required for network module")
endif()

# ThreadSystem dependencies
if(TARGET ThreadSystem::Core)
    # Modern modular thread system
    target_link_libraries(${PROJECT_NAME} PRIVATE ThreadSystem::Core)
else()
    # Legacy thread system (backward compatibility)
    set(THREAD_SYSTEM_LIBS thread_pool thread_base)
    foreach(lib ${THREAD_SYSTEM_LIBS})
        if(TARGET ${lib})
            target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})
        else()
            message(WARNING "${lib} target not found - network may have missing dependencies")
        endif()
    endforeach()
    
    # Utilities library dependency (legacy)
    if(TARGET utilities)
        target_link_libraries(${PROJECT_NAME} PRIVATE utilities)
    else()
        message(WARNING "utilities target not found - network may have missing dependencies")
    endif()
endif()

##################################################
# Platform-Specific Configuration
##################################################

# Windows-specific configuration
if(WIN32)
    # Windows sockets
    target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32 mswsock)
    
    # Windows-specific definitions
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        _WIN32_WINNT=0x0601  # Windows 7 or later
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    )
endif()

# ASIO configuration
target_compile_definitions(${PROJECT_NAME} PUBLIC
    ASIO_STANDALONE
    ASIO_NO_DEPRECATED
)

# Enable coroutines support if available
if(HAS_STD_COROUTINES)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        ASIO_HAS_CO_AWAIT
        ASIO_HAS_STD_COROUTINE
    )
endif()

##################################################
# Compiler Options
##################################################

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-missing-field-initializers
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /WX-
        /wd4100  # unreferenced formal parameter
        /wd4458  # declaration hides class member
        /wd4996  # deprecated functions
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
    set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)
endif()

##################################################
# Summary
##################################################

message(STATUS "Network library configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Headers: ${CMAKE_CURRENT_SOURCE_DIR}")
if(TARGET ThreadSystem::Core)
    message(STATUS "  Dependencies: asio, fmt, container, ThreadSystem::Core")
else()
    message(STATUS "  Dependencies: asio, fmt, container, thread_pool, thread_base, utilities")
endif()
if(WIN32)
    message(STATUS "  Windows Socket Libraries: ws2_32, mswsock")
endif()