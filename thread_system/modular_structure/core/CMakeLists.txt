cmake_minimum_required(VERSION 3.16)

##################################################
# Project configuration
##################################################
project(
    ThreadSystemCore
    VERSION 2.0.0
    DESCRIPTION "Core threading framework with minimal dependencies"
    HOMEPAGE_URL "https://github.com/kcenon/thread_system"
    LANGUAGES CXX
)

##################################################
# C++ standard requirements
##################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##################################################
# Global options
##################################################
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BUILD_SAMPLES "Build sample programs" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(USE_LOCKFREE "Use lock-free implementation" ON)

##################################################
# Compiler warnings and optimizations
##################################################
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -march=native)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2)
    endif()
endif()

##################################################
# Dependencies
##################################################
find_package(Threads REQUIRED)

# Optional vcpkg dependencies
find_package(fmt CONFIG QUIET)
if(NOT fmt_FOUND)
    # Try regular find_package
    find_package(fmt QUIET)
endif()

if(NOT fmt_FOUND)
    message(STATUS "fmt library not found, will use std::format if available")
endif()

find_package(GTest CONFIG QUIET)
find_package(benchmark CONFIG QUIET)

##################################################
# Library configuration
##################################################
add_library(thread_system_core)
add_library(ThreadSystem::Core ALIAS thread_system_core)

# Source files
target_sources(thread_system_core
    PRIVATE
        # Interfaces
        include/thread_system_core/interfaces/logger_interface.cpp
        
        # Thread base
        src/thread_base/core/thread_base.cpp
        src/thread_base/jobs/job.cpp
        src/thread_base/jobs/callback_job.cpp
        src/thread_base/jobs/job_queue.cpp
        src/thread_base/lockfree/memory/hazard_pointer.cpp
        src/thread_base/lockfree/queues/adaptive_job_queue.cpp
        src/thread_base/lockfree/queues/lockfree_job_queue.cpp
        
        # Thread pool
        src/thread_pool/core/thread_pool.cpp
        src/thread_pool/workers/thread_worker.cpp
        
        # Typed thread pool
        src/typed_thread_pool/scheduling/adaptive_typed_job_queue.cpp
        src/typed_thread_pool/scheduling/typed_lockfree_job_queue.cpp
        
        # Utilities
        src/utilities/conversion/convert_string.cpp
)

# Include directories
target_include_directories(thread_system_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific configuration
if(APPLE)
    find_library(ICONV_LIBRARY iconv)
    if(ICONV_LIBRARY)
        set(PLATFORM_LIBS ${ICONV_LIBRARY})
    endif()
endif()

# Link libraries
target_link_libraries(thread_system_core
    PUBLIC
        Threads::Threads
    PRIVATE
        $<$<TARGET_EXISTS:fmt::fmt>:fmt::fmt>
        ${PLATFORM_LIBS}
)

# Compile definitions
target_compile_definitions(thread_system_core
    PUBLIC
        $<$<BOOL:${USE_LOCKFREE}>:USE_LOCKFREE>
        $<$<NOT:$<TARGET_EXISTS:fmt::fmt>>:USE_STD_FORMAT>
)

##################################################
# Installation
##################################################
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install targets
install(TARGETS thread_system_core
    EXPORT ThreadSystemCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install export
install(EXPORT ThreadSystemCoreTargets
    FILE ThreadSystemCoreTargets.cmake
    NAMESPACE ThreadSystem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadSystemCore
)

# Create and install package config files
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ThreadSystemCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadSystemCore
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadSystemCore
)

##################################################
# Testing
##################################################
if(BUILD_TESTS AND TARGET GTest::gtest)
    enable_testing()
    add_subdirectory(tests)
endif()

##################################################
# Samples
##################################################
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

##################################################
# Export for build tree
##################################################
export(EXPORT ThreadSystemCoreTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/ThreadSystemCoreTargets.cmake
    NAMESPACE ThreadSystem::
)