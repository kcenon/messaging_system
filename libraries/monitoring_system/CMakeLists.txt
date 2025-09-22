cmake_minimum_required(VERSION 3.20)
project(monitoring_system VERSION 2.0.0 LANGUAGES CXX)

# C++ standard requirements with graceful degradation
# Default to C++20 but support graceful degradation to C++17
option(FORCE_CPP17 "Force C++17 standard (disable C++20 features)" OFF)

if(FORCE_CPP17)
    set(CMAKE_CXX_STANDARD 17)
    message(STATUS "Monitoring System: Forcing C++17 mode - C++20 features will be disabled")
else()
    set(CMAKE_CXX_STANDARD 20)
    message(STATUS "Monitoring System: Using C++20 mode - Enhanced features enabled")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include compatibility module for C++17/C++20 feature detection
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(MonitoringCompatibility)

# Configure compatibility features based on C++ standard
configure_monitoring_compatibility()
setup_performance_features()
setup_threading_features()

# Options
option(MONITORING_BUILD_TESTS "Build unit tests" ON)
option(MONITORING_BUILD_EXAMPLES "Build example programs" ON)
option(MONITORING_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(MONITORING_USE_THREAD_SYSTEM "Enable thread_system integration" OFF)
option(MONITORING_USE_LOGGER_SYSTEM "Enable logger_system integration" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Export compile commands for tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find dependencies
find_package(Threads REQUIRED)

# Optional dependencies
if(MONITORING_USE_THREAD_SYSTEM)
    find_package(thread_system CONFIG)
    if(thread_system_FOUND)
        message(STATUS "Found thread_system: integration enabled")
        add_compile_definitions(MONITORING_HAS_THREAD_SYSTEM)
    else()
        message(WARNING "thread_system not found: integration disabled")
        set(MONITORING_USE_THREAD_SYSTEM OFF)
    endif()
endif()

if(MONITORING_USE_LOGGER_SYSTEM)
    find_package(logger_system CONFIG)
    if(logger_system_FOUND)
        message(STATUS "Found logger_system: integration enabled")
        add_compile_definitions(MONITORING_HAS_LOGGER_SYSTEM)
    else()
        message(WARNING "logger_system not found: integration disabled")
        set(MONITORING_USE_LOGGER_SYSTEM OFF)
    endif()
endif()

# Create interface library for headers
add_library(monitoring_system_interface INTERFACE)
target_include_directories(monitoring_system_interface
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sources>
        $<INSTALL_INTERFACE:include>
)

# Set compile features
target_compile_features(monitoring_system_interface INTERFACE cxx_std_20)

# Add compile options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(monitoring_system_interface INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Werror=return-type
        -Werror=uninitialized
        -Werror=unused-result
    )
    
    # Add AVX2 support if available (with fallback)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        target_compile_options(monitoring_system_interface INTERFACE -mavx2)
        target_compile_definitions(monitoring_system_interface INTERFACE SIMD_AVX2_AVAILABLE)
    endif()
elseif(MSVC)
    target_compile_options(monitoring_system_interface INTERFACE
        /W4
        /WX
        /permissive-
        /Zc:__cplusplus
    )
    
    # Enable AVX2 on MSVC if supported
    target_compile_options(monitoring_system_interface INTERFACE /arch:AVX2)
    target_compile_definitions(monitoring_system_interface INTERFACE SIMD_AVX2_AVAILABLE)
endif()

# Sanitizers
if(ENABLE_ASAN)
    target_compile_options(monitoring_system_interface INTERFACE -fsanitize=address)
    target_link_options(monitoring_system_interface INTERFACE -fsanitize=address)
endif()

if(ENABLE_TSAN)
    target_compile_options(monitoring_system_interface INTERFACE -fsanitize=thread)
    target_link_options(monitoring_system_interface INTERFACE -fsanitize=thread)
endif()

if(ENABLE_UBSAN)
    target_compile_options(monitoring_system_interface INTERFACE -fsanitize=undefined)
    target_link_options(monitoring_system_interface INTERFACE -fsanitize=undefined)
endif()

# Check for new structure
set(MONITORING_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MONITORING_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(EXISTS ${MONITORING_INCLUDE_DIR}/kcenon/monitoring AND EXISTS ${MONITORING_SOURCE_DIR})
    message(STATUS "Monitoring System: Using new directory structure")

    # Collect source files from new structure
    file(GLOB_RECURSE MONITORING_HEADERS
        ${MONITORING_INCLUDE_DIR}/kcenon/monitoring/*.h
    )

    file(GLOB_RECURSE MONITORING_SOURCES
        ${MONITORING_SOURCE_DIR}/*.cpp
    )

    # Check if new structure has sources
    list(LENGTH MONITORING_SOURCES SOURCE_COUNT)
    if(SOURCE_COUNT GREATER 0)
        # Create library with new structure
        add_library(monitoring_system STATIC
            ${MONITORING_SOURCES}
            ${MONITORING_HEADERS}
        )

        target_include_directories(monitoring_system
            PUBLIC
                $<BUILD_INTERFACE:${MONITORING_INCLUDE_DIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sources>
                $<INSTALL_INTERFACE:include>
        )
    else()
        # Fall back to legacy structure
        message(STATUS "Monitoring System: New structure incomplete, using src sources")
        add_library(monitoring_system STATIC
            src/context/thread_context.cpp
            src/impl/tracing/distributed_tracer.cpp
            src/core/performance_monitor.cpp
            src/impl/adaptive_monitor.cpp
        )

        target_include_directories(monitoring_system
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sources>
                $<BUILD_INTERFACE:${MONITORING_INCLUDE_DIR}>
                $<INSTALL_INTERFACE:include>
        )
    endif()
else()
    # Use legacy structure
    message(STATUS "Monitoring System: Using legacy directory structure")
    add_library(monitoring_system STATIC
        src/context/thread_context.cpp
        src/impl/tracing/distributed_tracer.cpp
        src/core/performance_monitor.cpp
        src/impl/adaptive_monitor.cpp
    )

    target_include_directories(monitoring_system
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sources>
            $<INSTALL_INTERFACE:include>
    )
endif()

target_link_libraries(monitoring_system PUBLIC
    monitoring_system_interface
    Threads::Threads
)

# Setup formatting library (std::format, fmt, or basic fallback)
if(COMMAND setup_monitoring_formatting)
    setup_monitoring_formatting(monitoring_system)
endif()

# Link optional dependencies
if(MONITORING_USE_THREAD_SYSTEM)
    target_link_libraries(monitoring_system INTERFACE thread_system::thread_system)
endif()

if(MONITORING_USE_LOGGER_SYSTEM)
    target_link_libraries(monitoring_system INTERFACE logger_system::logger_system)
endif()

# Tests
if(MONITORING_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(MONITORING_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Benchmarks
if(MONITORING_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS monitoring_system monitoring_system_interface
    EXPORT monitoring_system_targets
)

install(DIRECTORY sources/monitoring
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT monitoring_system_targets
    FILE monitoring_system-targets.cmake
    NAMESPACE monitoring_system::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/monitoring_system
)

# Create package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/monitoring_system-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring_system-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/monitoring_system
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring_system-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring_system-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/monitoring_system-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/monitoring_system
)

# Summary
message(STATUS "")
message(STATUS "monitoring_system configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests: ${MONITORING_BUILD_TESTS}")
message(STATUS "  Build examples: ${MONITORING_BUILD_EXAMPLES}")
message(STATUS "  Build benchmarks: ${MONITORING_BUILD_BENCHMARKS}")
message(STATUS "  Thread system integration: ${MONITORING_USE_THREAD_SYSTEM}")
message(STATUS "  Logger system integration: ${MONITORING_USE_LOGGER_SYSTEM}")
message(STATUS "  AddressSanitizer: ${ENABLE_ASAN}")
message(STATUS "  ThreadSanitizer: ${ENABLE_TSAN}")
message(STATUS "  UBSanitizer: ${ENABLE_UBSAN}")
message(STATUS "")