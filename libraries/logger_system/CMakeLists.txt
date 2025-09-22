cmake_minimum_required(VERSION 3.16)

##################################################
# Project configuration
##################################################
project(
    LoggerSystem
    VERSION 1.0.0
    DESCRIPTION "High-performance C++20 logging system with lock-free implementation"
    HOMEPAGE_URL "https://github.com/kcenon/logger_system"
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
option(BUILD_LOGGER_AS_SUBMODULE "Build Logger as submodule" OFF)

if(NOT BUILD_LOGGER_AS_SUBMODULE)
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

option(USE_LOCKFREE "Use lock-free implementation" ON)

##################################################
# Compiler warnings and optimizations
##################################################
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -march=native -DNDEBUG)
        # Additional performance optimizations
        add_compile_options(-flto -ffast-math -funroll-loops)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            add_compile_options(-fvectorize -fslp-vectorize)
        endif()
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /Ob2 /Oi /Ot /GL /DNDEBUG)
        # Link-time optimization
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
        set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
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
# Using local logger_interface.h instead of external dependency

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
    add_subdirectory(tests)
endif()

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

##################################################
# Installation
##################################################
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS logger
    EXPORT LoggerSystemTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY sources/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/logger_system
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(EXPORT LoggerSystemTargets
    FILE LoggerSystemTargets.cmake
    NAMESPACE LoggerSystem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LoggerSystem
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LoggerSystemConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LoggerSystemConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LoggerSystem
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LoggerSystemConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LoggerSystemConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LoggerSystemConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LoggerSystem
)