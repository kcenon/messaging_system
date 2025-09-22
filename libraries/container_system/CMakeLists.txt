##################################################
# Container Module CMakeLists.txt
# 
# Builds the container library for the messaging system.
# Provides type-safe, serializable data containers.
##################################################

# Project definition
project(container 
    VERSION 2.0.0
    DESCRIPTION "Type-safe serializable container library with advanced features"
    LANGUAGES CXX
)

##################################################
# Source Files Configuration
##################################################

# Main include file
set(MAIN_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/container.h
)

# Core files (public API)
set(CORE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/value_types.cpp
)

# Value files (public API)
file(GLOB VALUE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/values/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/values/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/values/*.tpp
)

# Internal implementation files
set(INTERNAL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/variant_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/thread_safe_container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/thread_safe_container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/simd_processor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/simd_processor.cpp
)

# Combine all files
set(ALL_FILES 
    ${MAIN_HEADER}
    ${CORE_FILES} 
    ${VALUE_FILES} 
    ${INTERNAL_FILES}
)

##################################################
# Library Target Configuration
##################################################

# Create static library
add_library(${PROJECT_NAME} STATIC 
    ${ALL_FILES}
)

# Create alias for consistent naming
add_library(MessagingSystem::container ALIAS ${PROJECT_NAME})

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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<INSTALL_INTERFACE:include>
)

##################################################
# Compile Definitions
##################################################

# Internal features are always enabled but not exposed
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CONTAINER_INTERNAL_FEATURES
)

##################################################
# Dependencies
##################################################

# Utilities library dependency (optional)
if(TARGET utilities)
    target_link_libraries(${PROJECT_NAME} PUBLIC utilities)
    message(STATUS "Container module linked with utilities library")
else()
    message(STATUS "utilities target not found - using self-contained implementation")
    # Define utilities compatibility macros
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_INTERNAL_UTILITIES=1)
endif()

# Local utilities compatibility layer - self-contained implementation
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/libraries    # For includes like "container_system/core/container.h"
    ${CMAKE_CURRENT_SOURCE_DIR}      # For local includes
)

# Export include directories for dependent projects
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libraries>"
    "$<INSTALL_INTERFACE:include>"
)

# fmt library for formatting (only if not using std::format)
if(NOT USE_STD_FORMAT)
    find_package(fmt CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
else()
    message(STATUS "Container module using std::format instead of fmt library")
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_STD_FORMAT)
endif()

##################################################
# Platform-Specific Configuration
##################################################

# Check for SIMD support
include(CheckCXXCompilerFlag)

# AVX2 support
if(NOT APPLE)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        # Add AVX2 flags for all build types when compiling SIMD files
        set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/internal/simd_processor.cpp
            PROPERTIES COMPILE_FLAGS "-mavx2 -msse4.2 -msse3"
        )
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_AVX2)
    endif()
endif()

# SSE support (x86/x64 only)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
    if(COMPILER_SUPPORTS_SSE42)
        # SSE4.2 is already included with AVX2 above for simd_processor.cpp
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_SSE42)
    endif()
endif()

# ARM NEON support
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_ARM_NEON)
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
        -Wno-unused-lambda-capture
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /WX-
        /wd4100  # unreferenced formal parameter
        /wd4996  # deprecated functions
    )
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WIN32_WINNT=0x0601  # Windows 7 or later
        WIN32_LEAN_AND_MEAN
        NOMINMAX
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
    enable_testing()
    # Tests are handled by the main unittest directory
endif()

##################################################
# Documentation
##################################################

# Find Doxygen for documentation generation
find_package(Doxygen)
if(DOXYGEN_FOUND AND NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    
    doxygen_add_docs(${PROJECT_NAME}_docs
        ${CMAKE_CURRENT_SOURCE_DIR}/container
        COMMENT "Generating container documentation"
    )
endif()

##################################################
# Summary
##################################################

message(STATUS "Container library configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Module Directory: ${CMAKE_CURRENT_SOURCE_DIR}/container")
message(STATUS "  Dependencies: utilities, fmt")
message(STATUS "  Features:")
message(STATUS "    - Thread-safe operations: ON")
message(STATUS "    - Variant-based types: ON")
if(COMPILER_SUPPORTS_AVX2)
    message(STATUS "    - AVX2 SIMD: ON")
elseif(COMPILER_SUPPORTS_SSE42)
    message(STATUS "    - SSE4.2 SIMD: ON")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "    - ARM NEON SIMD: ON")
else()
    message(STATUS "    - SIMD: OFF (scalar fallback)")
endif()