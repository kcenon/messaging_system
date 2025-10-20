cmake_minimum_required(VERSION 3.16)

project(messaging_system
    VERSION 2.0.0
    DESCRIPTION "High-performance distributed messaging system"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Build Options
# ============================================================================

# Feature flags
option(MESSAGING_USE_LOCKFREE "Enable lock-free data structures" OFF)
option(MESSAGING_ENABLE_MONITORING "Enable runtime monitoring" ON)
option(MESSAGING_ENABLE_LOGGING "Enable logging system" ON)
option(MESSAGING_ENABLE_METRICS "Enable metrics collection" ON)
option(MESSAGING_ENABLE_TLS "Enable TLS/SSL support" OFF)

# Dependency strategy
option(MESSAGING_USE_EXTERNAL_SYSTEMS "Use external system packages" ON)
option(MESSAGING_USE_FETCHCONTENT "Use FetchContent for dependencies" OFF)

# Development options
option(MESSAGING_BUILD_TESTS "Build unit tests" ON)
option(MESSAGING_BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(MESSAGING_BUILD_EXAMPLES "Build example applications" ON)
option(MESSAGING_BUILD_DOCS "Build documentation with Doxygen" OFF)

# Build profile
set(MESSAGING_BUILD_PROFILE "Release" CACHE STRING "Build profile")
set_property(CACHE MESSAGING_BUILD_PROFILE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel"
    "ASAN" "TSAN" "UBSAN" "MSAN"
)

# ============================================================================
# Compiler Settings
# ============================================================================

# Warning level
set(MESSAGING_WARNING_LEVEL "High" CACHE STRING "Compiler warning level")

if(MESSAGING_WARNING_LEVEL STREQUAL "High" OR MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wextra -Wpedantic)
        if(MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
            add_compile_options(-Werror -Wconversion -Wsign-conversion)
        endif()
    elseif(MSVC)
        add_compile_options(/W4)
        if(MESSAGING_WARNING_LEVEL STREQUAL "Pedantic")
            add_compile_options(/WX)
        endif()
    endif()
endif()

# Build profile flags
if(MESSAGING_BUILD_PROFILE STREQUAL "ASAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "TSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "UBSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=undefined")
elseif(MESSAGING_BUILD_PROFILE STREQUAL "MSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=memory")
endif()

# ============================================================================
# External System Integration
# ============================================================================

if(MESSAGING_USE_EXTERNAL_SYSTEMS)
    message(STATUS "Using external system packages")

    if(MESSAGING_USE_FETCHCONTENT)
        # FetchContent mode (development)
        message(STATUS "Using FetchContent for external systems")
        include(FetchContent)

        # Disable tests in external systems to avoid dependency issues
        set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)

        # Prevent local system conflicts with FetchContent
        set(CMAKE_DISABLE_FIND_PACKAGE_CommonSystem TRUE)
        set(CMAKE_DISABLE_FIND_PACKAGE_ThreadSystem TRUE)
        set(CMAKE_DISABLE_FIND_PACKAGE_LoggerSystem TRUE)
        set(CMAKE_DISABLE_FIND_PACKAGE_MonitoringSystem TRUE)
        set(CMAKE_DISABLE_FIND_PACKAGE_ContainerSystem TRUE)
        set(CMAKE_DISABLE_FIND_PACKAGE_DatabaseSystem TRUE)
        set(CMAKE_DISABLE_FIND_PACKAGE_NetworkSystem TRUE)

        # Common System
        FetchContent_Declare(
            CommonSystem
            GIT_REPOSITORY https://github.com/kcenon/common_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Thread System
        FetchContent_Declare(
            ThreadSystem
            GIT_REPOSITORY https://github.com/kcenon/thread_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Logger System
        FetchContent_Declare(
            LoggerSystem
            GIT_REPOSITORY https://github.com/kcenon/logger_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Monitoring System
        FetchContent_Declare(
            MonitoringSystem
            GIT_REPOSITORY https://github.com/kcenon/monitoring_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Container System
        FetchContent_Declare(
            ContainerSystem
            GIT_REPOSITORY https://github.com/kcenon/container_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Database System
        FetchContent_Declare(
            DatabaseSystem
            GIT_REPOSITORY https://github.com/kcenon/database_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Network System
        FetchContent_Declare(
            NetworkSystem
            GIT_REPOSITORY https://github.com/kcenon/network_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        FetchContent_MakeAvailable(
            CommonSystem
            ThreadSystem
            LoggerSystem
            MonitoringSystem
            ContainerSystem
            DatabaseSystem
            NetworkSystem
        )

    else()
        # find_package mode (production)
        message(STATUS "Using find_package for external systems")

        list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

        find_package(CommonSystem 1.0 QUIET)
        find_package(ThreadSystem 1.0 QUIET)
        find_package(LoggerSystem 1.0 QUIET)
        find_package(MonitoringSystem 1.0 QUIET)
        find_package(ContainerSystem 1.0 QUIET)
        find_package(DatabaseSystem 1.0 QUIET)
        find_package(NetworkSystem 1.0 QUIET)
    endif()

    # Validate dependencies
    include(cmake/validate_dependencies.cmake)
    validate_messaging_dependencies()

    # Create backward-compatible aliases
    if(TARGET CommonSystem::common AND NOT TARGET common)
        add_library(common ALIAS CommonSystem::common)
    endif()

    if(TARGET ThreadSystem::Core AND NOT TARGET thread_pool)
        add_library(thread_pool ALIAS ThreadSystem::Core)
    endif()

    if(TARGET LoggerSystem::logger AND NOT TARGET logger)
        add_library(logger ALIAS LoggerSystem::logger)
    endif()

    if(TARGET MonitoringSystem::monitoring AND NOT TARGET monitoring)
        add_library(monitoring ALIAS MonitoringSystem::monitoring)
    endif()

    if(TARGET ContainerSystem::container AND NOT TARGET container)
        add_library(container ALIAS ContainerSystem::container)
    endif()

    if(TARGET DatabaseSystem::database AND NOT TARGET database)
        add_library(database ALIAS DatabaseSystem::database)
    endif()

    if(TARGET NetworkSystem::network AND NOT TARGET network)
        add_library(network ALIAS NetworkSystem::network)
    endif()

else()
    message(WARNING "Building without external systems (legacy mode)")
    # Fall back to internal implementations (deprecated)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/container_system)
        add_subdirectory(libraries/container_system)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/network_system)
        add_subdirectory(libraries/network_system)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/thread_system)
        add_subdirectory(libraries/thread_system)
    endif()
endif()

# ============================================================================
# Propagate Options to External Systems
# ============================================================================

if(MESSAGING_USE_LOCKFREE)
    set(USE_LOCKFREE_BY_DEFAULT ON CACHE BOOL "" FORCE)
    set(USE_LOCKFREE_THREAD_POOL ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_MONITORING)
    set(ENABLE_MONITORING ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_LOGGING)
    set(ENABLE_LOGGING ON CACHE BOOL "" FORCE)
endif()

if(MESSAGING_ENABLE_TLS)
    set(ENABLE_TLS ON CACHE BOOL "" FORCE)
endif()

# ============================================================================
# Messaging System Core Library
# ============================================================================

add_library(messaging_system_core
    src/core/messaging_container.cpp
    src/core/message_bus.cpp
    src/core/topic_router.cpp
    src/integration/trace_context.cpp
    src/integration/config_loader.cpp
)

target_include_directories(messaging_system_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link external systems if available
set(MESSAGING_CORE_LIBS "")

if(TARGET CommonSystem::common)
    list(APPEND MESSAGING_CORE_LIBS CommonSystem::common)
    target_compile_definitions(messaging_system_core PUBLIC HAS_COMMON_SYSTEM)
endif()

if(TARGET ThreadSystem::Core)
    list(APPEND MESSAGING_CORE_LIBS ThreadSystem::Core)
    target_compile_definitions(messaging_system_core PUBLIC HAS_THREAD_SYSTEM)
endif()

if(TARGET LoggerSystem::logger)
    list(APPEND MESSAGING_CORE_LIBS LoggerSystem::logger)
    target_compile_definitions(messaging_system_core PUBLIC HAS_LOGGER_SYSTEM)
endif()

if(TARGET MonitoringSystem::monitoring)
    list(APPEND MESSAGING_CORE_LIBS MonitoringSystem::monitoring)
    target_compile_definitions(messaging_system_core PUBLIC HAS_MONITORING_SYSTEM)
endif()

if(TARGET ContainerSystem::container)
    list(APPEND MESSAGING_CORE_LIBS ContainerSystem::container)
    target_compile_definitions(messaging_system_core PUBLIC HAS_CONTAINER_SYSTEM)
endif()

if(TARGET DatabaseSystem::database)
    list(APPEND MESSAGING_CORE_LIBS DatabaseSystem::database)
    target_compile_definitions(messaging_system_core PUBLIC HAS_DATABASE_SYSTEM)
endif()

if(TARGET NetworkSystem::network)
    list(APPEND MESSAGING_CORE_LIBS NetworkSystem::network)
    target_compile_definitions(messaging_system_core PUBLIC HAS_NETWORK_SYSTEM)
endif()

if(MESSAGING_CORE_LIBS)
    target_link_libraries(messaging_system_core PUBLIC ${MESSAGING_CORE_LIBS})
    target_compile_definitions(messaging_system_core PUBLIC MESSAGING_USE_EXTERNAL_SYSTEMS)
endif()

# Find and link yaml-cpp for configuration
find_package(yaml-cpp QUIET)
if(yaml-cpp_FOUND)
    target_link_libraries(messaging_system_core PUBLIC yaml-cpp)
    target_compile_definitions(messaging_system_core PUBLIC HAS_YAML_CPP)
    message(STATUS "  ✓ yaml-cpp found")
else()
    message(STATUS "  ✗ yaml-cpp not found (ConfigLoader disabled)")
endif()

target_compile_features(messaging_system_core PUBLIC cxx_std_20)

# ============================================================================
# Main Executable
# ============================================================================

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
    add_executable(messaging_system
        src/main.cpp
    )

    target_link_libraries(messaging_system
        PRIVATE
            messaging_system_core
    )
endif()

# ============================================================================
# Tests
# ============================================================================

if(MESSAGING_BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt)
    enable_testing()
    add_subdirectory(test)
endif()

# ============================================================================
# Benchmarks
# ============================================================================

if(MESSAGING_BUILD_BENCHMARKS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
    add_subdirectory(benchmarks)
endif()

# ============================================================================
# Examples
# ============================================================================

if(MESSAGING_BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()

# ============================================================================
# Documentation
# ============================================================================

if(MESSAGING_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/docs)
        add_subdirectory(docs)
    endif()
endif()

# ============================================================================
# Installation
# ============================================================================

include(GNUInstallDirs)

install(TARGETS messaging_system_core
    EXPORT messaging_system_targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(TARGET messaging_system)
    install(TARGETS messaging_system
        EXPORT messaging_system_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT messaging_system_targets
    FILE messaging_system-targets.cmake
    NAMESPACE MessagingSystem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/messaging_system
)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "=== Messaging System Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build profile: ${MESSAGING_BUILD_PROFILE}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  Lock-free: ${MESSAGING_USE_LOCKFREE}")
message(STATUS "  Monitoring: ${MESSAGING_ENABLE_MONITORING}")
message(STATUS "  Logging: ${MESSAGING_ENABLE_LOGGING}")
message(STATUS "  Metrics: ${MESSAGING_ENABLE_METRICS}")
message(STATUS "  TLS: ${MESSAGING_ENABLE_TLS}")
message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  Tests: ${MESSAGING_BUILD_TESTS}")
message(STATUS "  Benchmarks: ${MESSAGING_BUILD_BENCHMARKS}")
message(STATUS "  Examples: ${MESSAGING_BUILD_EXAMPLES}")
message(STATUS "  Documentation: ${MESSAGING_BUILD_DOCS}")
message(STATUS "")
message(STATUS "External systems: ${MESSAGING_USE_EXTERNAL_SYSTEMS}")
if(MESSAGING_USE_EXTERNAL_SYSTEMS)
    message(STATUS "  Using FetchContent: ${MESSAGING_USE_FETCHCONTENT}")
    message(STATUS "")
    message(STATUS "External systems found:")
    if(TARGET CommonSystem::common)
        message(STATUS "  ✓ CommonSystem")
    else()
        message(STATUS "  ✗ CommonSystem")
    endif()
    if(TARGET ThreadSystem::Core)
        message(STATUS "  ✓ ThreadSystem")
    else()
        message(STATUS "  ✗ ThreadSystem")
    endif()
    if(TARGET LoggerSystem::logger)
        message(STATUS "  ✓ LoggerSystem")
    else()
        message(STATUS "  ✗ LoggerSystem")
    endif()
    if(TARGET MonitoringSystem::monitoring)
        message(STATUS "  ✓ MonitoringSystem")
    else()
        message(STATUS "  ✗ MonitoringSystem")
    endif()
    if(TARGET ContainerSystem::container)
        message(STATUS "  ✓ ContainerSystem")
    else()
        message(STATUS "  ✗ ContainerSystem")
    endif()
    if(TARGET DatabaseSystem::database)
        message(STATUS "  ✓ DatabaseSystem")
    else()
        message(STATUS "  ✗ DatabaseSystem")
    endif()
    if(TARGET NetworkSystem::network)
        message(STATUS "  ✓ NetworkSystem")
    else()
        message(STATUS "  ✗ NetworkSystem")
    endif()
endif()
message(STATUS "======================================")
message(STATUS "")
