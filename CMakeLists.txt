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
option(MESSAGING_USE_LOCAL_SYSTEMS "Use local sibling directories for development" OFF)

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

    if(MESSAGING_USE_LOCAL_SYSTEMS)
        # ================================================================
        # LOCAL DEVELOPMENT MODE (Recommended for local development)
        # ================================================================
        message(STATUS "Using local sibling directories for external systems")
        message(STATUS "Expected directory structure: Sources/{system_name}/")

        # Disable tests/examples in external systems to speed up build
        set(BUILD_TESTS OFF CACHE BOOL "Disable tests in external systems" FORCE)
        set(BUILD_INTEGRATION_TESTS OFF CACHE BOOL "Disable integration tests" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)
        set(BUILD_SAMPLES OFF CACHE BOOL "Disable samples" FORCE)
        set(ENABLE_TESTING OFF CACHE BOOL "Disable testing" FORCE)

        # System-specific build flags
        set(LOGGER_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(LOGGER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(LOGGER_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
        set(DATABASE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(DATABASE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(DATABASE_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
        set(CONTAINER_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(NETWORK_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

        # CommonSystem (required)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../common_system/CMakeLists.txt")
            message(STATUS "  ✓ Found local common_system")
            add_subdirectory(
                "${CMAKE_CURRENT_SOURCE_DIR}/../common_system"
                "${CMAKE_BINARY_DIR}/_local/common_system"
            )
        else()
            message(FATAL_ERROR "common_system not found at ${CMAKE_CURRENT_SOURCE_DIR}/../common_system\n"
                "Please ensure all system projects are in the same parent directory:\n"
                "  Sources/common_system/\n"
                "  Sources/messaging_system/\n"
                "  Sources/thread_system/\n"
                "  etc.")
        endif()

        # ThreadSystem (required)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../thread_system/CMakeLists.txt")
            message(STATUS "  ✓ Found local thread_system")
            set(BUILD_THREADSYSTEM_AS_SUBMODULE ON CACHE BOOL "" FORCE)
            add_subdirectory(
                "${CMAKE_CURRENT_SOURCE_DIR}/../thread_system"
                "${CMAKE_BINARY_DIR}/_local/thread_system"
            )
        else()
            message(FATAL_ERROR "thread_system not found at ${CMAKE_CURRENT_SOURCE_DIR}/../thread_system")
        endif()

        # LoggerSystem (optional but recommended)
        # Note: logger_system will be built in standalone mode to avoid thread_system conflicts
        # The thread_system integration will be provided by our already-loaded thread_system
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../logger_system/CMakeLists.txt")
            message(STATUS "  ✓ Found local logger_system")
            # Use standalone mode to prevent logger_system from adding thread_system again
            set(LOGGER_STANDALONE_MODE ON CACHE BOOL "" FORCE)
            set(BUILD_WITH_COMMON_SYSTEM ON CACHE BOOL "" FORCE)
            add_subdirectory(
                "${CMAKE_CURRENT_SOURCE_DIR}/../logger_system"
                "${CMAKE_BINARY_DIR}/_local/logger_system"
            )
            # Manually link logger with thread_system after it's created
            if(TARGET logger AND TARGET utilities AND TARGET interfaces)
                target_link_libraries(logger PUBLIC utilities interfaces)
            endif()
        else()
            message(WARNING "  ✗ logger_system not found - logging features may be limited")
        endif()

        # MonitoringSystem (optional)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../monitoring_system/CMakeLists.txt")
            message(STATUS "  ✓ Found local monitoring_system")
            add_subdirectory(
                "${CMAKE_CURRENT_SOURCE_DIR}/../monitoring_system"
                "${CMAKE_BINARY_DIR}/_local/monitoring_system"
            )
        else()
            message(WARNING "  ✗ monitoring_system not found - monitoring features disabled")
        endif()

        # ContainerSystem (required)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../container_system/CMakeLists.txt")
            message(STATUS "  ✓ Found local container_system")
            # Force disable samples and set submodule mode before including
            set(BUILD_CONTAINERSYSTEM_AS_SUBMODULE ON CACHE BOOL "Build as submodule" FORCE)
            set(BUILD_CONTAINER_SAMPLES OFF CACHE BOOL "Disable container samples" FORCE)
            set(BUILD_CONTAINER_EXAMPLES OFF CACHE BOOL "Disable container examples" FORCE)
            set(BUILD_SAMPLES OFF CACHE BOOL "Disable samples" FORCE)
            add_subdirectory(
                "${CMAKE_CURRENT_SOURCE_DIR}/../container_system"
                "${CMAKE_BINARY_DIR}/_local/container_system"
            )
        else()
            message(FATAL_ERROR "container_system not found at ${CMAKE_CURRENT_SOURCE_DIR}/../container_system")
        endif()

        # DatabaseSystem (optional)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../database_system/CMakeLists.txt")
            message(STATUS "  ✓ Found local database_system")
            # Force disable samples and tests before including
            set(BUILD_DATABASE_SAMPLES OFF CACHE BOOL "Disable database samples" FORCE)
            set(DATABASE_BUILD_SAMPLES OFF CACHE BOOL "Disable database samples" FORCE)
            set(DATABASE_BUILD_TESTS OFF CACHE BOOL "Disable database tests" FORCE)
            set(USE_UNIT_TEST OFF CACHE BOOL "Disable unit tests" FORCE)
            set(BUILD_SAMPLES OFF CACHE BOOL "Disable samples" FORCE)
            add_subdirectory(
                "${CMAKE_CURRENT_SOURCE_DIR}/../database_system"
                "${CMAKE_BINARY_DIR}/_local/database_system"
            )
        else()
            message(WARNING "  ✗ database_system not found - database features disabled")
        endif()

        # NetworkSystem (optional)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../network_system/CMakeLists.txt")
            message(STATUS "  ✓ Found local network_system")
            # Force disable samples, tests, and install before including
            set(NETWORK_BUILD_SAMPLES OFF CACHE BOOL "Disable network samples" FORCE)
            set(NETWORK_BUILD_TESTS OFF CACHE BOOL "Disable network tests" FORCE)
            set(BUILD_SAMPLES OFF CACHE BOOL "Disable samples" FORCE)
            set(CMAKE_SKIP_INSTALL_RULES ON)
            add_subdirectory(
                "${CMAKE_CURRENT_SOURCE_DIR}/../network_system"
                "${CMAKE_BINARY_DIR}/_local/network_system"
            )
            set(CMAKE_SKIP_INSTALL_RULES OFF)
        else()
            message(WARNING "  ✗ network_system not found - network features disabled")
        endif()

        message(STATUS "Local systems integration complete")

    elseif(MESSAGING_USE_FETCHCONTENT)
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

        # Declare all systems first (before any MakeAvailable calls)
        # This allows us to set FETCHCONTENT_SOURCE_DIR_* variables to prevent duplicate fetches

        # Common System
        # Disable testing for all dependencies BEFORE declaring them
        set(BUILD_TESTING OFF CACHE BOOL "Disable tests" FORCE)

        FetchContent_Declare(
            CommonSystem
            GIT_REPOSITORY https://github.com/kcenon/common_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Thread System
        # Configure ThreadSystem to use std::format instead of fmt library BEFORE declaring it
        set(USE_STD_FORMAT ON CACHE BOOL "Use std::format instead of fmt library in thread_system" FORCE)

        FetchContent_Declare(
            ThreadSystem
            GIT_REPOSITORY https://github.com/kcenon/thread_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
            SOURCE_SUBDIR  "."  # Prevent local directory discovery
            OVERRIDE_FIND_PACKAGE
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
            PATCH_COMMAND sed -i.bak "s/add_subdirectory(samples)/#add_subdirectory(samples)/g" CMakeLists.txt || true
        )

        # Database System - DISABLED (not needed, has compilation issues)
        # FetchContent_Declare(
        #     DatabaseSystem
        #     GIT_REPOSITORY https://github.com/kcenon/database_system.git
        #     GIT_TAG main
        #     GIT_SHALLOW TRUE
        #     PATCH_COMMAND sed -i.bak -e "s/add_subdirectory(samples)/#add_subdirectory(samples)/g" -e "s/add_subdirectory(integration_tests)/#add_subdirectory(integration_tests)/g" CMakeLists.txt || true
        # )

        # Network System
        FetchContent_Declare(
            NetworkSystem
            GIT_REPOSITORY https://github.com/kcenon/network_system.git
            GIT_TAG main
            GIT_SHALLOW TRUE
        )

        # Disable installation for all FetchContent dependencies
        # This prevents export errors when dependencies link to each other
        set(CMAKE_SKIP_INSTALL_RULES_SAVE "${CMAKE_SKIP_INSTALL_RULES}")
        set(CMAKE_SKIP_INSTALL_RULES TRUE)

        # Note: BUILD_TESTING is already set to OFF above before FetchContent_Declare
        # Save the MESSAGING_BUILD_TESTS state to restore later
        if(DEFINED MESSAGING_BUILD_TESTS)
            set(MESSAGING_BUILD_TESTS_SAVE "${MESSAGING_BUILD_TESTS}")
        else()
            set(MESSAGING_BUILD_TESTS_SAVE ON)
        endif()

        # Make CommonSystem available first (required by most others)
        FetchContent_MakeAvailable(CommonSystem)

        # Remove dependency test targets to prevent build failures
        # common_system tests use deprecated error() API
        if(TARGET common_system_tests)
            set_target_properties(common_system_tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif()
        if(TEST common_system_tests)
            set_tests_properties(common_system_tests PROPERTIES DISABLED TRUE)
        endif()

        # Get the source directory for common_system to provide to other systems
        FetchContent_GetProperties(CommonSystem SOURCE_DIR commonsystem_source_dir)
        set(COMMON_SYSTEM_ROOT "${commonsystem_source_dir}" CACHE PATH "Path to common_system" FORCE)
        set(COMMON_SYSTEM_INCLUDE_DIR "${commonsystem_source_dir}/include" CACHE PATH "Path to common_system headers" FORCE)

        # Prevent logger_system from independently fetching common_system
        # Create a temporary Config file so find_package(common_system) succeeds
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/common_system-config.cmake"
            "set(common_system_FOUND TRUE)\n"
            "set(COMMON_SYSTEM_INCLUDE_DIR \"${commonsystem_source_dir}/include\")\n"
            "set(common_system_SOURCE_DIR \"${commonsystem_source_dir}\")\n"
        )
        list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}")
        set(common_system_FOUND TRUE CACHE BOOL "" FORCE)
        set(common_system_SOURCE_DIR "${commonsystem_source_dir}" CACHE STRING "" FORCE)

        # Then ThreadSystem (required by logger, monitoring)
        # Note: USE_STD_FORMAT is set earlier before FetchContent_Declare
        FetchContent_MakeAvailable(ThreadSystem)

        # Get ThreadSystem source directory for reference
        FetchContent_GetProperties(ThreadSystem SOURCE_DIR threadsystem_source_dir)
        set(thread_system_SOURCE_DIR "${threadsystem_source_dir}" CACHE STRING "" FORCE)
        set(thread_system_FOUND TRUE CACHE BOOL "" FORCE)

        # Ensure USE_STD_FORMAT is defined for all thread_system targets IF enabled
        # This is a workaround for thread_system not reading USE_STD_FORMAT option correctly
        # Only add compile definition if USE_STD_FORMAT option is ON
        if(USE_STD_FORMAT)
            # Ensure all thread_system targets propagate the std::format configuration.
            set(_thread_system_format_targets
                thread_base
                thread_pool
                interfaces
                utilities
                typed_thread_pool
            )
            foreach(_ts_target IN LISTS _thread_system_format_targets)
                if(TARGET "${_ts_target}")
                    get_target_property(_aliased "${_ts_target}" ALIASED_TARGET)
                    if(NOT _aliased)
                        target_compile_definitions("${_ts_target}" PUBLIC USE_STD_FORMAT)
                    endif()
                endif()
            endforeach()
        endif()

        # ContainerSystem needs common_system headers to be fully available
        FetchContent_MakeAvailable(ContainerSystem)

        # Disable PostgreSQL in DatabaseSystem due to libpqxx compatibility issues
        set(USE_POSTGRESQL OFF CACHE BOOL "Disable PostgreSQL support in database_system" FORCE)
        set(USE_MYSQL OFF CACHE BOOL "Disable MySQL support in database_system" FORCE)
        set(USE_SQLITE OFF CACHE BOOL "Disable SQLite support in database_system" FORCE)

        # Then remaining systems that depend on common and thread
        FetchContent_MakeAvailable(
            LoggerSystem
            MonitoringSystem
            # DatabaseSystem  # DISABLED - not needed
            NetworkSystem
        )

        # Restore original CMAKE_SKIP_INSTALL_RULES setting
        set(CMAKE_SKIP_INSTALL_RULES "${CMAKE_SKIP_INSTALL_RULES_SAVE}")

        # Re-enable BUILD_TESTING for messaging_system's own tests
        # Use MESSAGING_BUILD_TESTS as the control variable
        if(MESSAGING_BUILD_TESTS)
            set(BUILD_TESTING ON CACHE BOOL "Enable testing for messaging_system" FORCE)
        endif()

        # Create missing aliases that some dependencies expect
        # Note: kcenon::common is already an ALIAS, so we need to alias the real target
        if(TARGET common AND NOT TARGET kcenon::common_system)
            add_library(kcenon::common_system ALIAS common)
        elseif(TARGET common_system AND NOT TARGET kcenon::common_system)
            add_library(kcenon::common_system ALIAS common_system)
        endif()

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

    # Validate dependencies (skip for FetchContent and Local modes - systems are guaranteed to be available)
    if(NOT MESSAGING_USE_FETCHCONTENT AND NOT MESSAGING_USE_LOCAL_SYSTEMS)
        include(cmake/validate_dependencies.cmake)
        validate_messaging_dependencies()
    else()
        if(MESSAGING_USE_LOCAL_SYSTEMS)
            message(STATUS "✓ Using local systems - skipping dependency validation")
        else()
            message(STATUS "✓ Using FetchContent - skipping dependency validation")
        endif()
    endif()

    # Create backward-compatible aliases
    # Note: In Local/FetchContent mode, targets are exported directly without namespace
    #       In find_package mode, targets use namespace (e.g., CommonSystem::common)
    if(NOT MESSAGING_USE_FETCHCONTENT AND NOT MESSAGING_USE_LOCAL_SYSTEMS)
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
    endif()

else()
    message(FATAL_ERROR
        "MESSAGING_USE_EXTERNAL_SYSTEMS is OFF. This mode is no longer supported.\n"
        "Please use one of the following:\n"
        "  - Local development: cmake --preset dev-local (requires sibling directories)\n"
        "  - FetchContent: cmake --preset dev-fetchcontent\n"
        "  - Production: cmake --preset default (requires installed packages)\n"
    )
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

# CommonSystem (check both namespaced and non-namespaced targets)
if(TARGET CommonSystem::common)
    message(STATUS "Linking messaging_system_core with CommonSystem::common")
    list(APPEND MESSAGING_CORE_LIBS CommonSystem::common)
    target_compile_definitions(messaging_system_core PUBLIC HAS_COMMON_SYSTEM)
elseif(TARGET kcenon::common)
    message(STATUS "Linking messaging_system_core with kcenon::common")
    list(APPEND MESSAGING_CORE_LIBS kcenon::common)
    target_compile_definitions(messaging_system_core PUBLIC HAS_COMMON_SYSTEM)
elseif(TARGET common_system)
    message(STATUS "Linking messaging_system_core with common_system")
    list(APPEND MESSAGING_CORE_LIBS common_system)
    target_compile_definitions(messaging_system_core PUBLIC HAS_COMMON_SYSTEM)
else()
    message(STATUS "CommonSystem target not found - header-only mode may still work")
endif()

# ThreadSystem
if(TARGET ThreadSystem::Core)
    list(APPEND MESSAGING_CORE_LIBS ThreadSystem::Core)
    target_compile_definitions(messaging_system_core PUBLIC HAS_THREAD_SYSTEM)
elseif(TARGET thread_pool)
    list(APPEND MESSAGING_CORE_LIBS thread_pool utilities interfaces)
    target_compile_definitions(messaging_system_core PUBLIC HAS_THREAD_SYSTEM)
elseif(TARGET thread_base)
    list(APPEND MESSAGING_CORE_LIBS thread_base utilities interfaces)
    target_compile_definitions(messaging_system_core PUBLIC HAS_THREAD_SYSTEM)
endif()

# LoggerSystem
if(TARGET LoggerSystem::logger)
    list(APPEND MESSAGING_CORE_LIBS LoggerSystem::logger)
    target_compile_definitions(messaging_system_core PUBLIC HAS_LOGGER_SYSTEM)
elseif(TARGET LoggerSystem)
    list(APPEND MESSAGING_CORE_LIBS LoggerSystem)
    target_compile_definitions(messaging_system_core PUBLIC HAS_LOGGER_SYSTEM)
elseif(TARGET logger)
    list(APPEND MESSAGING_CORE_LIBS logger)
    target_compile_definitions(messaging_system_core PUBLIC HAS_LOGGER_SYSTEM)
endif()

# MonitoringSystem
if(TARGET MonitoringSystem::monitoring)
    list(APPEND MESSAGING_CORE_LIBS MonitoringSystem::monitoring)
    target_compile_definitions(messaging_system_core PUBLIC HAS_MONITORING_SYSTEM)
elseif(TARGET monitoring_system)
    list(APPEND MESSAGING_CORE_LIBS monitoring_system)
    target_compile_definitions(messaging_system_core PUBLIC HAS_MONITORING_SYSTEM)
elseif(TARGET monitoring)
    list(APPEND MESSAGING_CORE_LIBS monitoring)
    target_compile_definitions(messaging_system_core PUBLIC HAS_MONITORING_SYSTEM)
endif()

# ContainerSystem
if(TARGET ContainerSystem::container)
    list(APPEND MESSAGING_CORE_LIBS ContainerSystem::container)
    target_compile_definitions(messaging_system_core PUBLIC HAS_CONTAINER_SYSTEM)
elseif(TARGET container_system)
    list(APPEND MESSAGING_CORE_LIBS container_system)
    target_compile_definitions(messaging_system_core PUBLIC HAS_CONTAINER_SYSTEM)
endif()

# DatabaseSystem - DISABLED (not needed)
# if(TARGET DatabaseSystem::database)
#     list(APPEND MESSAGING_CORE_LIBS DatabaseSystem::database)
#     target_compile_definitions(messaging_system_core PUBLIC HAS_DATABASE_SYSTEM)
# elseif(TARGET database)
#     list(APPEND MESSAGING_CORE_LIBS database)
#     target_compile_definitions(messaging_system_core PUBLIC HAS_DATABASE_SYSTEM)
# endif()

# NetworkSystem
if(TARGET NetworkSystem::network)
    list(APPEND MESSAGING_CORE_LIBS NetworkSystem::network)
    target_compile_definitions(messaging_system_core PUBLIC HAS_NETWORK_SYSTEM)
elseif(TARGET NetworkSystem)
    list(APPEND MESSAGING_CORE_LIBS NetworkSystem)
    target_compile_definitions(messaging_system_core PUBLIC HAS_NETWORK_SYSTEM)
endif()

if(MESSAGING_CORE_LIBS)
    target_link_libraries(messaging_system_core PUBLIC ${MESSAGING_CORE_LIBS})
    target_compile_definitions(messaging_system_core PUBLIC MESSAGING_USE_EXTERNAL_SYSTEMS)
endif()

# Treat external dependency headers as system to avoid propagating their warnings.
set(_messaging_external_include_targets
    CommonSystem::common
    kcenon::common
    common_system
    ThreadSystem::Core
    thread_pool
    thread_base
    interfaces
    utilities
    LoggerSystem::logger
    LoggerSystem
    logger
    MonitoringSystem::monitoring
    monitoring_system
    monitoring
    ContainerSystem::container
    container_system
    NetworkSystem::network
    NetworkSystem
)
foreach(_dep_target IN LISTS _messaging_external_include_targets)
    if(TARGET "${_dep_target}")
        get_target_property(_dep_includes "${_dep_target}" INTERFACE_INCLUDE_DIRECTORIES)
        if(_dep_includes)
            target_include_directories(messaging_system_core SYSTEM PUBLIC ${_dep_includes})
        endif()
    endif()
endforeach()

if(USE_STD_FORMAT)
    target_compile_definitions(messaging_system_core PUBLIC USE_STD_FORMAT)
endif()

# Find and link yaml-cpp for configuration
find_package(yaml-cpp QUIET)
if(yaml-cpp_FOUND AND TARGET yaml-cpp::yaml-cpp)
    target_link_libraries(messaging_system_core PUBLIC yaml-cpp::yaml-cpp)
    target_compile_definitions(messaging_system_core PUBLIC HAS_YAML_CPP)
    message(STATUS "  ✓ yaml-cpp found")
elseif(yaml-cpp_FOUND AND TARGET yaml-cpp)
    target_link_libraries(messaging_system_core PUBLIC yaml-cpp)
    target_compile_definitions(messaging_system_core PUBLIC HAS_YAML_CPP)
    message(STATUS "  ✓ yaml-cpp found (non-namespaced)")
else()
    message(STATUS "  ✗ yaml-cpp not found (ConfigLoader disabled)")
endif()

target_compile_features(messaging_system_core PUBLIC cxx_std_20)

# ============================================================================
# Main Executable (Example/Demo)
# ============================================================================

if(MESSAGING_BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
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

# Skip installation in local development mode or FetchContent mode
# (local systems and FetchContent dependencies are not installed, so export would fail)
if(NOT MESSAGING_USE_LOCAL_SYSTEMS AND NOT MESSAGING_USE_FETCHCONTENT)
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
endif()

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
    if(MESSAGING_USE_LOCAL_SYSTEMS)
        message(STATUS "  Using local systems: ON")
        message(STATUS "  Using FetchContent: OFF")
    else()
        message(STATUS "  Using local systems: OFF")
        message(STATUS "  Using FetchContent: ${MESSAGING_USE_FETCHCONTENT}")
    endif()
    message(STATUS "")
    message(STATUS "External systems found:")
    # Check for targets without namespace (local/FetchContent mode) or with namespace (find_package mode)
    if(TARGET CommonSystem::common OR TARGET kcenon::common OR TARGET common_system OR TARGET common)
        message(STATUS "  ✓ CommonSystem")
    else()
        message(STATUS "  ✗ CommonSystem")
    endif()
    if(TARGET ThreadSystem::Core OR TARGET thread_pool OR TARGET thread_base OR TARGET interfaces)
        message(STATUS "  ✓ ThreadSystem")
    else()
        message(STATUS "  ✗ ThreadSystem")
    endif()
    if(TARGET LoggerSystem::logger OR TARGET LoggerSystem OR TARGET logger)
        message(STATUS "  ✓ LoggerSystem")
    else()
        message(STATUS "  ✗ LoggerSystem")
    endif()
    if(TARGET MonitoringSystem::monitoring OR TARGET monitoring_system OR TARGET monitoring)
        message(STATUS "  ✓ MonitoringSystem")
    else()
        message(STATUS "  ✗ MonitoringSystem")
    endif()
    if(TARGET ContainerSystem::container OR TARGET container_system OR TARGET container)
        message(STATUS "  ✓ ContainerSystem")
    else()
        message(STATUS "  ✗ ContainerSystem")
    endif()
    if(TARGET DatabaseSystem::database OR TARGET database)
        message(STATUS "  ✓ DatabaseSystem")
    else()
        message(STATUS "  ✗ DatabaseSystem")
    endif()
    if(TARGET NetworkSystem::network OR TARGET NetworkSystem OR TARGET network)
        message(STATUS "  ✓ NetworkSystem")
    else()
        message(STATUS "  ✗ NetworkSystem")
    endif()
endif()
message(STATUS "======================================")
message(STATUS "")
