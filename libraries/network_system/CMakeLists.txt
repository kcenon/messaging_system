cmake_minimum_required(VERSION 3.16)

# Set policy for FindBoost module (suppress warning)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

##################################################
# Network System CMakeLists.txt
#
# Independent high-performance network system
# Separated from messaging_system for modularity
##################################################

# Project definition
project(NetworkSystem
    DESCRIPTION "Independent High-Performance Network System"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SAMPLES "Build samples" ON)
option(BUILD_WITH_CONTAINER_SYSTEM "Build with container_system integration" ON)
option(BUILD_WITH_THREAD_SYSTEM "Build with thread_system integration" ON)
option(BUILD_WITH_LOGGER_SYSTEM "Build with logger_system integration" ON)
option(BUILD_MESSAGING_BRIDGE "Build messaging_system bridge" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Global compile features
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Coverage settings
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Add atomic updates to prevent negative counts in coverage
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage -fprofile-update=atomic")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage -fprofile-update=atomic")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

        # Only add libatomic on Linux (not needed on macOS or Windows)
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -latomic")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -latomic")
        endif()
    endif()
endif()

# Find packages with flexible detection
# Try modern CMake targets first, then fallback to manual detection
find_package(PkgConfig QUIET)

# ASIO Detection (standalone, not Boost.ASIO)
find_path(ASIO_INCLUDE_DIR
    NAMES asio.hpp
    PATHS
        # vcpkg paths
        ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/include
        # Windows common paths
        C:/vcpkg/installed/x64-windows/include
        C:/tools/vcpkg/installed/x64-windows/include
        # macOS Homebrew
        /opt/homebrew/include
        /usr/local/include
        # Ubuntu/Debian system paths
        /usr/include
        /usr/include/asio
        # MSYS2/MinGW paths
        /mingw64/include
        /mingw32/include
        # Custom installation paths
        ${ASIO_ROOT}/include
        $ENV{ASIO_ROOT}/include
)

# FMT Detection with pkgconfig support
if(PkgConfig_FOUND)
    pkg_check_modules(FMT QUIET IMPORTED_TARGET fmt)
    if(FMT_FOUND)
        message(STATUS "Found fmt via pkgconfig: ${FMT_VERSION}")
    endif()
endif()

if(NOT FMT_FOUND)
    find_path(FMT_INCLUDE_DIR
        NAMES fmt/format.h
        PATHS
            # vcpkg paths
            ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include
            ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/include
            # Windows common paths
            C:/vcpkg/installed/x64-windows/include
            C:/tools/vcpkg/installed/x64-windows/include
            # macOS Homebrew
            /opt/homebrew/include
            /usr/local/include
            # Ubuntu/Debian system paths
            /usr/include
            # MSYS2/MinGW paths
            /mingw64/include
            /mingw32/include
            # Custom installation paths
            ${FMT_ROOT}/include
            $ENV{FMT_ROOT}/include
    )

    if(FMT_INCLUDE_DIR)
        find_library(FMT_LIBRARY
            NAMES fmt
            PATHS
                # vcpkg paths
                ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib
                # macOS Homebrew
                /opt/homebrew/lib
                /usr/local/lib
                # Ubuntu/Debian system paths
                /usr/lib
                /usr/lib/x86_64-linux-gnu
                # MSYS2/MinGW paths
                /mingw64/lib
                /mingw32/lib
        )
    endif()
endif()

# Check for ASIO availability
if(NOT ASIO_INCLUDE_DIR)
    message(STATUS "Standalone ASIO not found. Checking for Boost.ASIO...")

    # Try to find Boost.ASIO as fallback
    # First try modern CMake config mode
    find_package(Boost QUIET CONFIG COMPONENTS system)
    if(NOT Boost_FOUND)
        # Fallback to module mode for older installations
        find_package(Boost QUIET MODULE COMPONENTS system)
    endif()

    if(Boost_FOUND)
        message(STATUS "Using Boost.ASIO as fallback (version: ${Boost_VERSION})")
        set(USE_BOOST_ASIO ON)
    else()
        # Try to find boost headers manually for header-only usage
        find_path(BOOST_INCLUDE_DIR
            NAMES boost/asio.hpp
            PATHS
                # Windows common paths
                ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include
                C:/vcpkg/installed/x64-windows/include
                C:/tools/vcpkg/installed/x64-windows/include
                C:/local/boost_1_82_0
                C:/boost/include
                # MSYS2/MinGW paths
                /mingw64/include
                /mingw32/include
                # macOS Homebrew
                /opt/homebrew/include
                /usr/local/include
                # Ubuntu/Debian system paths
                /usr/include
        )

        if(BOOST_INCLUDE_DIR)
            message(STATUS "Using Boost.ASIO headers-only fallback at: ${BOOST_INCLUDE_DIR}")
            set(USE_BOOST_ASIO ON)
            set(Boost_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
        else()
            message(WARNING "ASIO not found. Network functionality may be limited.")
            message(STATUS "Please install one of:")
            message(STATUS "  - libasio-dev (Ubuntu/Debian): sudo apt install libasio-dev")
            message(STATUS "  - Boost libraries: sudo apt install libboost-all-dev")
            message(STATUS "  - vcpkg: vcpkg install asio boost")
            message(STATUS "  - macOS Homebrew: brew install asio boost")
            # Set a fallback to allow build to continue
            set(USE_BOOST_ASIO ON)
            set(BOOST_INCLUDE_DIR "")
        endif()
    endif()
else()
    message(STATUS "Found ASIO at: ${ASIO_INCLUDE_DIR}")
    set(USE_BOOST_ASIO OFF)
endif()

# FMT library check
if(FMT_FOUND)
    message(STATUS "Found fmt via pkgconfig")
elseif(FMT_INCLUDE_DIR)
    message(STATUS "Found fmt at: ${FMT_INCLUDE_DIR}")
    if(FMT_LIBRARY)
        message(STATUS "Found fmt library at: ${FMT_LIBRARY}")
    endif()
else()
    message(STATUS "FMT not found, will use std::format (C++20) if available")
    set(USE_STD_FORMAT ON)
endif()

# Conditional dependencies
if(BUILD_WITH_CONTAINER_SYSTEM)
    # First try to find container_system as a CMake target (if built as subdirectory)
    if(TARGET ContainerSystem::container)
        message(STATUS "Found container_system CMake target: ContainerSystem::container")
        set(CONTAINER_SYSTEM_TARGET ContainerSystem::container)
        add_compile_definitions(BUILD_WITH_CONTAINER_SYSTEM)
    else()
        # Fallback to path-based detection
        find_path(CONTAINER_SYSTEM_INCLUDE_DIR
            NAMES container.h
            PATHS
                ../container_system
                ${CMAKE_CURRENT_SOURCE_DIR}/../container_system
            NO_DEFAULT_PATH
        )

        if(CONTAINER_SYSTEM_INCLUDE_DIR)
            message(STATUS "Found container_system headers at: ${CONTAINER_SYSTEM_INCLUDE_DIR}")

            # Check if the library exists
            find_library(CONTAINER_SYSTEM_LIBRARY
                NAMES container_system ContainerSystem
                PATHS
                    ../container_system/build
                    ${CMAKE_CURRENT_SOURCE_DIR}/../container_system/build
                PATH_SUFFIXES lib Debug Release
                NO_DEFAULT_PATH
            )

            if(CONTAINER_SYSTEM_LIBRARY)
                message(STATUS "Found container_system library at: ${CONTAINER_SYSTEM_LIBRARY}")
            else()
                message(STATUS "container_system library not found, will build from source if available")
            endif()

            add_compile_definitions(BUILD_WITH_CONTAINER_SYSTEM)
        else()
            message(WARNING "container_system not found, integration disabled")
            message(STATUS "Searched paths: ../container_system, ${CMAKE_CURRENT_SOURCE_DIR}/../container_system")
            set(BUILD_WITH_CONTAINER_SYSTEM OFF)
        endif()
    endif()
endif()

if(BUILD_WITH_THREAD_SYSTEM)
    find_path(THREAD_SYSTEM_INCLUDE_DIR
        NAMES kcenon/thread/core/thread_pool.h
        PATHS ../thread_system/include
        NO_DEFAULT_PATH
    )
    find_library(THREAD_SYSTEM_LIBRARY
        NAMES thread_base
        PATHS
            ../thread_system/build/lib
            ../thread_system/build
            ../thread_system/lib
        NO_DEFAULT_PATH
    )
    if(THREAD_SYSTEM_INCLUDE_DIR)
        message(STATUS "Found thread_system at: ${THREAD_SYSTEM_INCLUDE_DIR}")
        if(THREAD_SYSTEM_LIBRARY)
            message(STATUS "Found thread_system library at: ${THREAD_SYSTEM_LIBRARY}")
        else()
            message(WARNING "thread_system library not found, integration disabled")
            set(BUILD_WITH_THREAD_SYSTEM OFF)
        endif()
        add_compile_definitions(BUILD_WITH_THREAD_SYSTEM)
    else()
        message(WARNING "thread_system not found, integration disabled")
        set(BUILD_WITH_THREAD_SYSTEM OFF)
    endif()
endif()

if(BUILD_WITH_LOGGER_SYSTEM)
    find_path(LOGGER_SYSTEM_INCLUDE_DIR
        NAMES kcenon/logger/core/logger.h
        PATHS ../logger_system/include
        NO_DEFAULT_PATH
    )
    find_library(LOGGER_SYSTEM_LIBRARY
        NAMES LoggerSystem logger_system logger
        PATHS
            ../logger_system/build/lib
            ../logger_system/build_test/lib
            ../logger_system/build
            ../logger_system/lib
        NO_DEFAULT_PATH
    )
    if(LOGGER_SYSTEM_INCLUDE_DIR)
        message(STATUS "Found logger_system at: ${LOGGER_SYSTEM_INCLUDE_DIR}")
        if(LOGGER_SYSTEM_LIBRARY)
            message(STATUS "Found logger_system library at: ${LOGGER_SYSTEM_LIBRARY}")
        else()
            message(WARNING "logger_system library not found, integration disabled")
            set(BUILD_WITH_LOGGER_SYSTEM OFF)
        endif()
        add_compile_definitions(BUILD_WITH_LOGGER_SYSTEM)
    else()
        message(WARNING "logger_system not found, integration disabled")
        set(BUILD_WITH_LOGGER_SYSTEM OFF)
    endif()
endif()

# Create main library
add_library(NetworkSystem
    # Core implementation
    src/core/messaging_client.cpp
    src/core/messaging_server.cpp

    # Session management
    src/session/messaging_session.cpp

    # Internal implementation
    src/internal/tcp_socket.cpp
    src/internal/send_coroutine.cpp
    src/internal/pipeline.cpp
)

# Add logger integration if not using logger_system
# (Always add it to provide basic logging functionality)
target_sources(NetworkSystem PRIVATE
    src/integration/logger_integration.cpp
)

# Set target properties
set_target_properties(NetworkSystem PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(NetworkSystem
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# External system integration
if(BUILD_WITH_CONTAINER_SYSTEM)
    if(CONTAINER_SYSTEM_TARGET)
        # Link against the CMake target
        target_link_libraries(NetworkSystem PRIVATE ${CONTAINER_SYSTEM_TARGET})
        message(STATUS "Linked NetworkSystem with container_system target")
    elseif(CONTAINER_SYSTEM_INCLUDE_DIR)
        # Manual include and library linking
        target_include_directories(NetworkSystem PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        if(CONTAINER_SYSTEM_LIBRARY)
            target_link_libraries(NetworkSystem PRIVATE ${CONTAINER_SYSTEM_LIBRARY})
        endif()
        message(STATUS "Configured NetworkSystem with container_system paths")
    endif()
endif()

if(BUILD_WITH_THREAD_SYSTEM AND THREAD_SYSTEM_INCLUDE_DIR)
    target_include_directories(NetworkSystem PRIVATE ${THREAD_SYSTEM_INCLUDE_DIR})
    if(THREAD_SYSTEM_LIBRARY)
        target_link_libraries(NetworkSystem PUBLIC ${THREAD_SYSTEM_LIBRARY})
    endif()
endif()

if(BUILD_WITH_LOGGER_SYSTEM AND LOGGER_SYSTEM_INCLUDE_DIR)
    target_include_directories(NetworkSystem PRIVATE ${LOGGER_SYSTEM_INCLUDE_DIR})
    if(LOGGER_SYSTEM_LIBRARY)
        target_link_libraries(NetworkSystem PUBLIC ${LOGGER_SYSTEM_LIBRARY})
    endif()
endif()

# Configure ASIO
if(USE_BOOST_ASIO)
    target_include_directories(NetworkSystem PRIVATE ${Boost_INCLUDE_DIRS})
    if(Boost_LIBRARIES)
        target_link_libraries(NetworkSystem PRIVATE ${Boost_LIBRARIES})
        message(STATUS "Configured to use Boost.ASIO with libraries")
    else()
        message(STATUS "Configured to use Boost.ASIO header-only")
    endif()
    target_compile_definitions(NetworkSystem PRIVATE USE_BOOST_ASIO)
    target_compile_definitions(NetworkSystem PRIVATE BOOST_ASIO_STANDALONE)
elseif(ASIO_INCLUDE_DIR)
    target_include_directories(NetworkSystem PRIVATE ${ASIO_INCLUDE_DIR})
    target_compile_definitions(NetworkSystem PRIVATE ASIO_STANDALONE)
    target_compile_definitions(NetworkSystem PRIVATE ASIO_NO_DEPRECATED)
    message(STATUS "Configured to use standalone ASIO")
endif()

# Define Windows version for ASIO
if(WIN32)
    target_compile_definitions(NetworkSystem PRIVATE _WIN32_WINNT=0x0601)
endif()

# Configure FMT
if(FMT_FOUND AND TARGET PkgConfig::FMT)
    target_link_libraries(NetworkSystem PRIVATE PkgConfig::FMT)
    target_compile_definitions(NetworkSystem PRIVATE USE_FMT_LIBRARY)
    message(STATUS "Configured to use fmt library via pkgconfig")
elseif(FMT_INCLUDE_DIR)
    target_include_directories(NetworkSystem PRIVATE ${FMT_INCLUDE_DIR})
    if(FMT_LIBRARY)
        target_link_libraries(NetworkSystem PRIVATE ${FMT_LIBRARY})
        target_compile_definitions(NetworkSystem PRIVATE USE_FMT_LIBRARY)
        message(STATUS "Configured to use fmt library")
    else()
        target_compile_definitions(NetworkSystem PRIVATE FMT_HEADER_ONLY)
        message(STATUS "Configured to use fmt header-only")
    endif()
elseif(USE_STD_FORMAT)
    target_compile_definitions(NetworkSystem PRIVATE USE_STD_FORMAT)
    message(STATUS "Configured to use std::format")
endif()

# Link system libraries
if(NOT WIN32)
    target_link_libraries(NetworkSystem
        PUBLIC
            pthread
    )
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(NetworkSystem PRIVATE ws2_32 mswsock)
endif()

# Integration layer (conditional compilation)
if(BUILD_MESSAGING_BRIDGE)
    target_sources(NetworkSystem PRIVATE
        src/integration/messaging_bridge.cpp
        src/integration/thread_integration.cpp
        src/integration/container_integration.cpp
    )
    add_compile_definitions(BUILD_MESSAGING_BRIDGE)
endif()

# Build verification test
add_executable(verify_build verify_build.cpp)
target_link_libraries(verify_build PRIVATE NetworkSystem)

# Add container_system include path
if(CONTAINER_SYSTEM_INCLUDE_DIR)
    target_include_directories(verify_build PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
    target_compile_definitions(verify_build PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
endif()

# Add thread_system include path
if(THREAD_SYSTEM_INCLUDE_DIR)
    target_include_directories(verify_build PRIVATE ${THREAD_SYSTEM_INCLUDE_DIR})
    target_compile_definitions(verify_build PRIVATE BUILD_WITH_THREAD_SYSTEM)
endif()

# Add logger_system include path
if(LOGGER_SYSTEM_INCLUDE_DIR)
    target_include_directories(verify_build PRIVATE ${LOGGER_SYSTEM_INCLUDE_DIR})
    target_compile_definitions(verify_build PRIVATE BUILD_WITH_LOGGER_SYSTEM)
endif()

# Integration tests (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_integration.cpp)
    add_executable(test_integration tests/integration/test_integration.cpp)
    target_link_libraries(test_integration PRIVATE NetworkSystem)

    # Add container_system include path
    if(CONTAINER_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_integration PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_integration PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
    endif()

    # Add thread_system include path
    if(THREAD_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_integration PRIVATE ${THREAD_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_integration PRIVATE BUILD_WITH_THREAD_SYSTEM)
    endif()

    # Add logger_system include path
    if(LOGGER_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_integration PRIVATE ${LOGGER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_integration PRIVATE BUILD_WITH_LOGGER_SYSTEM)
    endif()
endif()

# Compatibility tests (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_compatibility.cpp)
    add_executable(test_compatibility tests/integration/test_compatibility.cpp)
    target_link_libraries(test_compatibility PRIVATE NetworkSystem)

    # Add container_system include path
    if(CONTAINER_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_compatibility PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_compatibility PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
    endif()

    # Add thread_system include path
    if(THREAD_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_compatibility PRIVATE ${THREAD_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_compatibility PRIVATE BUILD_WITH_THREAD_SYSTEM)
    endif()

    # Add logger_system include path
    if(LOGGER_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_compatibility PRIVATE ${LOGGER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_compatibility PRIVATE BUILD_WITH_LOGGER_SYSTEM)
    endif()

    # Configure same as main library
    if(USE_BOOST_ASIO)
        target_include_directories(test_compatibility PRIVATE ${Boost_INCLUDE_DIRS})
        if(Boost_LIBRARIES)
            target_link_libraries(test_compatibility PRIVATE ${Boost_LIBRARIES})
        endif()
        target_compile_definitions(test_compatibility PRIVATE USE_BOOST_ASIO)
        target_compile_definitions(test_compatibility PRIVATE BOOST_ASIO_STANDALONE)
    elseif(ASIO_INCLUDE_DIR)
        target_include_directories(test_compatibility PRIVATE ${ASIO_INCLUDE_DIR})
        target_compile_definitions(test_compatibility PRIVATE ASIO_STANDALONE)
        target_compile_definitions(test_compatibility PRIVATE ASIO_NO_DEPRECATED)
    endif()

    if(WIN32)
        target_compile_definitions(test_compatibility PRIVATE _WIN32_WINNT=0x0601)
    endif()
endif()

# End-to-end tests (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_e2e.cpp)
    add_executable(test_e2e tests/integration/test_e2e.cpp)
    target_link_libraries(test_e2e PRIVATE NetworkSystem)

    # Add container_system include path
    if(CONTAINER_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_e2e PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_e2e PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
    endif()

    # Add thread_system include path
    if(THREAD_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_e2e PRIVATE ${THREAD_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_e2e PRIVATE BUILD_WITH_THREAD_SYSTEM)
    endif()

    # Add logger_system include path
    if(LOGGER_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_e2e PRIVATE ${LOGGER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_e2e PRIVATE BUILD_WITH_LOGGER_SYSTEM)
    endif()

    # Configure same as main library
    if(USE_BOOST_ASIO)
        target_include_directories(test_e2e PRIVATE ${Boost_INCLUDE_DIRS})
        if(Boost_LIBRARIES)
            target_link_libraries(test_e2e PRIVATE ${Boost_LIBRARIES})
        endif()
        target_compile_definitions(test_e2e PRIVATE USE_BOOST_ASIO)
        target_compile_definitions(test_e2e PRIVATE BOOST_ASIO_STANDALONE)
    elseif(ASIO_INCLUDE_DIR)
        target_include_directories(test_e2e PRIVATE ${ASIO_INCLUDE_DIR})
        target_compile_definitions(test_e2e PRIVATE ASIO_STANDALONE)
        target_compile_definitions(test_e2e PRIVATE ASIO_NO_DEPRECATED)
    endif()

    if(WIN32)
        target_compile_definitions(test_e2e PRIVATE _WIN32_WINNT=0x0601)
    endif()
endif()

# Stress tests (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/stress/stress_test.cpp)
    add_executable(stress_test tests/stress/stress_test.cpp)
    target_link_libraries(stress_test PRIVATE NetworkSystem)

    # Add container_system include path
    if(CONTAINER_SYSTEM_INCLUDE_DIR)
        target_include_directories(stress_test PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(stress_test PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
    endif()

    # Add thread_system include path
    if(THREAD_SYSTEM_INCLUDE_DIR)
        target_include_directories(stress_test PRIVATE ${THREAD_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(stress_test PRIVATE BUILD_WITH_THREAD_SYSTEM)
    endif()

    # Add logger_system include path
    if(LOGGER_SYSTEM_INCLUDE_DIR)
        target_include_directories(stress_test PRIVATE ${LOGGER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(stress_test PRIVATE BUILD_WITH_LOGGER_SYSTEM)
    endif()

    # Configure same as main library
    if(USE_BOOST_ASIO)
        target_include_directories(stress_test PRIVATE ${Boost_INCLUDE_DIRS})
        if(Boost_LIBRARIES)
            target_link_libraries(stress_test PRIVATE ${Boost_LIBRARIES})
        endif()
        target_compile_definitions(stress_test PRIVATE USE_BOOST_ASIO)
        target_compile_definitions(stress_test PRIVATE BOOST_ASIO_STANDALONE)
    elseif(ASIO_INCLUDE_DIR)
        target_include_directories(stress_test PRIVATE ${ASIO_INCLUDE_DIR})
        target_compile_definitions(stress_test PRIVATE ASIO_STANDALONE)
        target_compile_definitions(stress_test PRIVATE ASIO_NO_DEPRECATED)
    endif()

    if(WIN32)
        target_compile_definitions(stress_test PRIVATE _WIN32_WINNT=0x0601)
    endif()
endif()

# Performance benchmark (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/performance/benchmark.cpp)
    add_executable(benchmark tests/performance/benchmark.cpp)
    target_link_libraries(benchmark PRIVATE NetworkSystem)

    # Add container_system include path
    if(CONTAINER_SYSTEM_INCLUDE_DIR)
        target_include_directories(benchmark PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(benchmark PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
    endif()

    # Add thread_system include path
    if(THREAD_SYSTEM_INCLUDE_DIR)
        target_include_directories(benchmark PRIVATE ${THREAD_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(benchmark PRIVATE BUILD_WITH_THREAD_SYSTEM)
    endif()

    # Add logger_system include path
    if(LOGGER_SYSTEM_INCLUDE_DIR)
        target_include_directories(benchmark PRIVATE ${LOGGER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(benchmark PRIVATE BUILD_WITH_LOGGER_SYSTEM)
    endif()

    # Configure same as main library
    if(USE_BOOST_ASIO)
        target_include_directories(benchmark PRIVATE ${Boost_INCLUDE_DIRS})
        if(Boost_LIBRARIES)
            target_link_libraries(benchmark PRIVATE ${Boost_LIBRARIES})
        endif()
        target_compile_definitions(benchmark PRIVATE USE_BOOST_ASIO)
        target_compile_definitions(benchmark PRIVATE BOOST_ASIO_STANDALONE)
    elseif(ASIO_INCLUDE_DIR)
        target_include_directories(benchmark PRIVATE ${ASIO_INCLUDE_DIR})
        target_compile_definitions(benchmark PRIVATE ASIO_STANDALONE)
        target_compile_definitions(benchmark PRIVATE ASIO_NO_DEPRECATED)
    endif()

    if(WIN32)
        target_compile_definitions(benchmark PRIVATE _WIN32_WINNT=0x0601)
    endif()
endif()

# Update test_integration configuration to use proper structure
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_integration.cpp)
    # Add container_system include path for test_integration too
    if(CONTAINER_SYSTEM_INCLUDE_DIR)
        target_include_directories(test_integration PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        target_compile_definitions(test_integration PRIVATE BUILD_WITH_CONTAINER_SYSTEM)
    endif()

    # Configure same as main library
    if(USE_BOOST_ASIO)
        target_include_directories(test_integration PRIVATE ${Boost_INCLUDE_DIRS})
        if(Boost_LIBRARIES)
            target_link_libraries(test_integration PRIVATE ${Boost_LIBRARIES})
        endif()
        target_compile_definitions(test_integration PRIVATE USE_BOOST_ASIO)
        target_compile_definitions(test_integration PRIVATE BOOST_ASIO_STANDALONE)
    elseif(ASIO_INCLUDE_DIR)
        target_include_directories(test_integration PRIVATE ${ASIO_INCLUDE_DIR})
        target_compile_definitions(test_integration PRIVATE ASIO_STANDALONE)
        target_compile_definitions(test_integration PRIVATE ASIO_NO_DEPRECATED)
    endif()

    if(WIN32)
        target_compile_definitions(test_integration PRIVATE _WIN32_WINNT=0x0601)
    endif()
endif()

# Configure ASIO for verification test (same as main library)
if(USE_BOOST_ASIO)
    target_include_directories(verify_build PRIVATE ${Boost_INCLUDE_DIRS})
    if(Boost_LIBRARIES)
        target_link_libraries(verify_build PRIVATE ${Boost_LIBRARIES})
    endif()
    target_compile_definitions(verify_build PRIVATE USE_BOOST_ASIO)
    target_compile_definitions(verify_build PRIVATE BOOST_ASIO_STANDALONE)
elseif(ASIO_INCLUDE_DIR)
    target_include_directories(verify_build PRIVATE ${ASIO_INCLUDE_DIR})
    target_compile_definitions(verify_build PRIVATE ASIO_STANDALONE)
    target_compile_definitions(verify_build PRIVATE ASIO_NO_DEPRECATED)
endif()

# Define Windows version for ASIO
if(WIN32)
    target_compile_definitions(verify_build PRIVATE _WIN32_WINNT=0x0601)
endif()

# Configure FMT for verification test
if(FMT_FOUND AND TARGET PkgConfig::FMT)
    target_link_libraries(verify_build PRIVATE PkgConfig::FMT)
elseif(FMT_INCLUDE_DIR)
    target_include_directories(verify_build PRIVATE ${FMT_INCLUDE_DIR})
    if(FMT_LIBRARY)
        target_link_libraries(verify_build PRIVATE ${FMT_LIBRARY})
    endif()
endif()

# Configure container_system for verification test
if(BUILD_WITH_CONTAINER_SYSTEM)
    if(CONTAINER_SYSTEM_TARGET)
        target_link_libraries(verify_build PRIVATE ${CONTAINER_SYSTEM_TARGET})
    elseif(CONTAINER_SYSTEM_INCLUDE_DIR)
        target_include_directories(verify_build PRIVATE ${CONTAINER_SYSTEM_INCLUDE_DIR})
        if(CONTAINER_SYSTEM_LIBRARY)
            target_link_libraries(verify_build PRIVATE ${CONTAINER_SYSTEM_LIBRARY})
        endif()
    endif()
endif()

set_target_properties(verify_build PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Tests - temporarily disabled until implementation is complete
if(BUILD_TESTS)
    enable_testing()
    message(STATUS "Tests temporarily disabled - core implementation in progress")
    # add_subdirectory(tests)

    # Add our simple verification test
    add_test(NAME verify_build_test COMMAND verify_build)
endif()

# Samples - temporarily disabled until implementation is complete
if(BUILD_SAMPLES)
    message(STATUS "Samples temporarily disabled - core implementation in progress")
    # add_subdirectory(samples)
endif()

# Installation rules
include(GNUInstallDirs)

# Install library
install(TARGETS NetworkSystem
    EXPORT NetworkSystemTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Config file generation
include(CMakePackageConfigHelpers)

# Generate config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/NetworkSystemConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/NetworkSystemConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NetworkSystem
)

# Generate version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/NetworkSystemConfigVersion.cmake"
    VERSION 0.0.0
    COMPATIBILITY AnyNewerVersion
)

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/NetworkSystemConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/NetworkSystemConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NetworkSystem
)

# Install export targets
install(EXPORT NetworkSystemTargets
    FILE NetworkSystemTargets.cmake
    NAMESPACE NetworkSystem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NetworkSystem
)

# Print configuration summary
message(STATUS "NetworkSystem Configuration Summary:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build samples: ${BUILD_SAMPLES}")
message(STATUS "  Container system integration: ${BUILD_WITH_CONTAINER_SYSTEM}")
message(STATUS "  Thread system integration: ${BUILD_WITH_THREAD_SYSTEM}")
message(STATUS "  Messaging bridge: ${BUILD_MESSAGING_BRIDGE}")

# Cleanup old files
set(OLD_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/session
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/network
    ${CMAKE_CURRENT_SOURCE_DIR}/network.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists_old.txt
)

# Note: We'll clean these manually after successful build

# Include CPack for distribution packages
include(cmake/CPack.cmake)