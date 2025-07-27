cmake_minimum_required(VERSION 3.16)

##################################################
# Project configuration
##################################################
project(
    MonitoringSystem
    VERSION 1.0.0
    DESCRIPTION "Real-time performance monitoring system for C++20 applications"
    HOMEPAGE_URL "https://github.com/kcenon/monitoring_system"
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
option(BUILD_MONITORING_AS_SUBMODULE "Build Monitoring as submodule" OFF)

if(NOT BUILD_MONITORING_AS_SUBMODULE)
    option(BUILD_TESTS "Build unit tests" ON)
    option(BUILD_BENCHMARKS "Build benchmarks" OFF)
    option(BUILD_SAMPLES "Build sample programs" ON)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
else()
    # When building as submodule, disable tests and samples by default
    option(BUILD_TESTS "Build unit tests" OFF)
    option(BUILD_BENCHMARKS "Build benchmarks" OFF)
    option(BUILD_SAMPLES "Build sample programs" OFF)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
endif()

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
# Output directories
##################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

##################################################
# Dependencies
##################################################
find_package(Threads REQUIRED)
include(FetchContent)

# Thread System Interface (local copy)
# Using local monitoring_interface.h instead of external dependency

# Google Test (for unit tests)
if (BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

# Google Benchmark (for benchmarks)
if (BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

##################################################
# Source files
##################################################
add_subdirectory(sources)

if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

if (BUILD_TESTS)
    add_subdirectory(unittest)
endif()

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

##################################################
# Installation
##################################################
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS monitoring
    EXPORT MonitoringSystemTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY sources/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/monitoring_system
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(EXPORT MonitoringSystemTargets
    FILE MonitoringSystemTargets.cmake
    NAMESPACE MonitoringSystem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MonitoringSystem
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MonitoringSystemConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MonitoringSystemConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MonitoringSystem
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MonitoringSystemConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MonitoringSystemConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MonitoringSystemConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MonitoringSystem
)