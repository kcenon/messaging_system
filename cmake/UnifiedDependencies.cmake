# UnifiedDependencies.cmake
# Unified dependency management for kcenon system libraries
#
# This module provides a unified interface for finding dependencies across
# three modes: LOCAL (sibling directories), FETCHCONTENT, and find_package.
#
# Usage:
#   include(UnifiedDependencies)
#   unified_setup_dependency_mode()  # Call once to configure mode
#   unified_find_dependency(common_system REQUIRED)
#   unified_find_dependency(monitoring_system OPTIONAL)
#
# Options (set before including):
#   UNIFIED_USE_LOCAL       - Use sibling directories (default: OFF)
#   UNIFIED_USE_FETCHCONTENT - Use FetchContent (default: OFF)
#   UNIFIED_ALLOW_FETCHCONTENT_FALLBACK - Allow FetchContent as fallback (default: ON)

cmake_minimum_required(VERSION 3.16)

# Guard against multiple inclusion
if(DEFINED _UNIFIED_DEPENDENCIES_INCLUDED)
    return()
endif()
set(_UNIFIED_DEPENDENCIES_INCLUDED TRUE)

# =============================================================================
# Configuration
# =============================================================================

option(UNIFIED_USE_LOCAL "Use local sibling directories for dependencies" OFF)
option(UNIFIED_USE_FETCHCONTENT "Use FetchContent for all dependencies" OFF)
option(UNIFIED_ALLOW_FETCHCONTENT_FALLBACK "Allow FetchContent when find_package fails" ON)

# GitHub organization for FetchContent
set(UNIFIED_GITHUB_ORG "kcenon" CACHE STRING "GitHub organization for FetchContent")

# =============================================================================
# Target Mapping Table
# =============================================================================
# Maps dependency names to possible target names (checked in order)
# Format: dependency_name -> list of possible target names

set(_UNIFIED_TARGET_MAP_common_system
    "common"
    "common_system"
    "CommonSystem::common"
    "kcenon::common"
    "kcenon::common_system"
)

set(_UNIFIED_TARGET_MAP_thread_system
    "thread_pool"
    "thread_base"
    "interfaces"
    "utilities"
    "typed_thread_pool"
    "ThreadSystem::Core"
    "thread_system"
)

set(_UNIFIED_TARGET_MAP_logger_system
    "logger"
    "LoggerSystem::logger"
    "LoggerSystem"
    "logger_system"
)

set(_UNIFIED_TARGET_MAP_monitoring_system
    "monitoring"
    "monitoring_system"
    "MonitoringSystem::monitoring"
    "MonitoringSystem"
)

set(_UNIFIED_TARGET_MAP_container_system
    "container"
    "container_system"
    "ContainerSystem::container"
    "ContainerSystem"
)

set(_UNIFIED_TARGET_MAP_database_system
    "database"
    "database_system"
    "DatabaseSystem::database"
    "DatabaseSystem"
)

set(_UNIFIED_TARGET_MAP_network_system
    "network"
    "network_system"
    "NetworkSystem::network"
    "NetworkSystem"
)

# =============================================================================
# FetchContent Repository Configuration
# =============================================================================

set(_UNIFIED_REPO_common_system "common_system")
set(_UNIFIED_REPO_thread_system "thread_system")
set(_UNIFIED_REPO_logger_system "logger_system")
set(_UNIFIED_REPO_monitoring_system "monitoring_system")
set(_UNIFIED_REPO_container_system "container_system")
set(_UNIFIED_REPO_database_system "database_system")
set(_UNIFIED_REPO_network_system "network_system")

# FetchContent names (CamelCase for FetchContent compatibility)
set(_UNIFIED_FETCH_NAME_common_system "CommonSystem")
set(_UNIFIED_FETCH_NAME_thread_system "ThreadSystem")
set(_UNIFIED_FETCH_NAME_logger_system "LoggerSystem")
set(_UNIFIED_FETCH_NAME_monitoring_system "MonitoringSystem")
set(_UNIFIED_FETCH_NAME_container_system "ContainerSystem")
set(_UNIFIED_FETCH_NAME_database_system "DatabaseSystem")
set(_UNIFIED_FETCH_NAME_network_system "NetworkSystem")

# find_package names
set(_UNIFIED_PACKAGE_NAME_common_system "CommonSystem")
set(_UNIFIED_PACKAGE_NAME_thread_system "ThreadSystem")
set(_UNIFIED_PACKAGE_NAME_logger_system "LoggerSystem")
set(_UNIFIED_PACKAGE_NAME_monitoring_system "MonitoringSystem")
set(_UNIFIED_PACKAGE_NAME_container_system "ContainerSystem")
set(_UNIFIED_PACKAGE_NAME_database_system "DatabaseSystem")
set(_UNIFIED_PACKAGE_NAME_network_system "NetworkSystem")

# =============================================================================
# Helper Functions
# =============================================================================

# Check if any target from the list exists
# Sets ${OUT_VAR} to the first found target name, or empty if none found
function(_unified_find_existing_target DEP_NAME OUT_VAR)
    set(${OUT_VAR} "" PARENT_SCOPE)

    if(DEFINED _UNIFIED_TARGET_MAP_${DEP_NAME})
        foreach(_target IN LISTS _UNIFIED_TARGET_MAP_${DEP_NAME})
            if(TARGET ${_target})
                set(${OUT_VAR} "${_target}" PARENT_SCOPE)
                return()
            endif()
        endforeach()
    endif()
endfunction()

# Get the primary target name for a dependency
function(_unified_get_primary_target DEP_NAME OUT_VAR)
    if(DEFINED _UNIFIED_TARGET_MAP_${DEP_NAME})
        list(GET _UNIFIED_TARGET_MAP_${DEP_NAME} 0 _primary)
        set(${OUT_VAR} "${_primary}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${DEP_NAME}" PARENT_SCOPE)
    endif()
endfunction()

# =============================================================================
# Setup Mode (call once at the beginning)
# =============================================================================

macro(unified_setup_dependency_mode)
    # Determine the parent directory for sibling projects
    get_filename_component(_UNIFIED_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
    set(_UNIFIED_PARENT_DIR "${_UNIFIED_PARENT_DIR}" CACHE INTERNAL "Parent directory for sibling projects")

    # Display mode
    if(UNIFIED_USE_LOCAL)
        message(STATUS "[UnifiedDependencies] Mode: LOCAL (sibling directories)")
        message(STATUS "[UnifiedDependencies] Parent directory: ${_UNIFIED_PARENT_DIR}")
    elseif(UNIFIED_USE_FETCHCONTENT)
        message(STATUS "[UnifiedDependencies] Mode: FETCHCONTENT")
        include(FetchContent)
    else()
        message(STATUS "[UnifiedDependencies] Mode: find_package")
        if(UNIFIED_ALLOW_FETCHCONTENT_FALLBACK)
            message(STATUS "[UnifiedDependencies] FetchContent fallback: ENABLED")
            include(FetchContent)
        endif()
    endif()

    # Common settings for subdirectory builds
    if(UNIFIED_USE_LOCAL OR UNIFIED_USE_FETCHCONTENT)
        # Disable tests/examples in dependencies to speed up build
        set(BUILD_TESTS OFF CACHE BOOL "Disable tests in dependencies" FORCE)
        set(BUILD_INTEGRATION_TESTS OFF CACHE BOOL "Disable integration tests" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)
        set(BUILD_SAMPLES OFF CACHE BOOL "Disable samples" FORCE)
        set(ENABLE_TESTING OFF CACHE BOOL "Disable testing" FORCE)
        set(BUILD_TESTING OFF CACHE BOOL "Disable testing" FORCE)

        # System-specific build flags
        set(LOGGER_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(LOGGER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(LOGGER_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
        set(DATABASE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(DATABASE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(DATABASE_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
        set(CONTAINER_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(NETWORK_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(COMMON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(COMMON_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)

        # Prevent find_package conflicts in FetchContent mode
        if(UNIFIED_USE_FETCHCONTENT)
            set(CMAKE_DISABLE_FIND_PACKAGE_CommonSystem TRUE)
            set(CMAKE_DISABLE_FIND_PACKAGE_ThreadSystem TRUE)
            set(CMAKE_DISABLE_FIND_PACKAGE_LoggerSystem TRUE)
            set(CMAKE_DISABLE_FIND_PACKAGE_MonitoringSystem TRUE)
            set(CMAKE_DISABLE_FIND_PACKAGE_ContainerSystem TRUE)
            set(CMAKE_DISABLE_FIND_PACKAGE_DatabaseSystem TRUE)
            set(CMAKE_DISABLE_FIND_PACKAGE_NetworkSystem TRUE)
        endif()
    endif()

    # Initialize loaded dependencies list
    set(_UNIFIED_LOADED_DEPS "" CACHE INTERNAL "List of loaded dependencies")
endmacro()

# =============================================================================
# Main Dependency Resolution Macro
# =============================================================================

# Find a dependency using the configured mode
# Usage: unified_find_dependency(NAME [REQUIRED|OPTIONAL] [VERSION ver] [GIT_TAG tag])
macro(unified_find_dependency DEP_NAME)
    cmake_parse_arguments(_UFD "REQUIRED;OPTIONAL" "VERSION;GIT_TAG" "" ${ARGN})

    # Default to main branch if no tag specified
    if(NOT _UFD_GIT_TAG)
        set(_UFD_GIT_TAG "main")
    endif()

    # Check if already loaded
    list(FIND _UNIFIED_LOADED_DEPS "${DEP_NAME}" _dep_index)
    if(NOT _dep_index EQUAL -1)
        message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Already loaded (skipping)")
        # Find and set the target variable anyway
        _unified_find_existing_target(${DEP_NAME} _found_target)
        if(_found_target)
            set(${DEP_NAME}_TARGET "${_found_target}")
        endif()
    else()
        # Step 1: Check if target already exists
        _unified_find_existing_target(${DEP_NAME} _existing_target)

        if(_existing_target)
            message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Found existing target '${_existing_target}'")
            set(${DEP_NAME}_TARGET "${_existing_target}")
            set(${DEP_NAME}_FOUND TRUE)
            list(APPEND _UNIFIED_LOADED_DEPS "${DEP_NAME}")
            set(_UNIFIED_LOADED_DEPS "${_UNIFIED_LOADED_DEPS}" CACHE INTERNAL "List of loaded dependencies")
        else()
            # Step 2: Check environment variable
            if(DEFINED ENV{${DEP_NAME}_ROOT})
                set(_env_root "$ENV{${DEP_NAME}_ROOT}")
                if(EXISTS "${_env_root}/CMakeLists.txt")
                    message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Found via environment variable")
                    add_subdirectory("${_env_root}" "${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}")
                    _unified_find_existing_target(${DEP_NAME} _new_target)
                    set(${DEP_NAME}_TARGET "${_new_target}")
                    set(${DEP_NAME}_FOUND TRUE)
                    list(APPEND _UNIFIED_LOADED_DEPS "${DEP_NAME}")
                    set(_UNIFIED_LOADED_DEPS "${_UNIFIED_LOADED_DEPS}" CACHE INTERNAL "List of loaded dependencies")
                endif()
            endif()

            # Step 3: Mode-specific resolution
            if(NOT ${DEP_NAME}_FOUND)
                if(UNIFIED_USE_LOCAL)
                    # LOCAL mode: check sibling directories
                    _unified_resolve_local(${DEP_NAME} ${_UFD_REQUIRED})
                elseif(UNIFIED_USE_FETCHCONTENT)
                    # FETCHCONTENT mode: fetch from GitHub
                    _unified_resolve_fetchcontent(${DEP_NAME} ${_UFD_GIT_TAG})
                else()
                    # find_package mode with optional FetchContent fallback
                    _unified_resolve_find_package(${DEP_NAME} "${_UFD_VERSION}" ${_UFD_REQUIRED})
                endif()
            endif()
        endif()

        # Validate required dependencies
        if(_UFD_REQUIRED AND NOT ${DEP_NAME}_FOUND)
            message(FATAL_ERROR
                "[UnifiedDependencies] REQUIRED dependency '${DEP_NAME}' not found!\n"
                "Searched locations:\n"
                "  - Existing targets\n"
                "  - Environment variable: ${DEP_NAME}_ROOT\n"
                "  - Sibling directory: ${_UNIFIED_PARENT_DIR}/${DEP_NAME}\n"
                "  - find_package(${_UNIFIED_PACKAGE_NAME_${DEP_NAME}})\n"
            )
        endif()
    endif()
endmacro()

# =============================================================================
# Local Resolution (sibling directories)
# =============================================================================

macro(_unified_resolve_local DEP_NAME IS_REQUIRED)
    set(_local_path "${_UNIFIED_PARENT_DIR}/${DEP_NAME}")

    if(EXISTS "${_local_path}/CMakeLists.txt")
        message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Found local at ${_local_path}")

        # Special handling for specific dependencies
        if(${DEP_NAME} STREQUAL "thread_system")
            set(BUILD_THREADSYSTEM_AS_SUBMODULE ON CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "logger_system")
            if(NOT DEFINED LOGGER_STANDALONE_MODE)
                set(LOGGER_STANDALONE_MODE ON CACHE BOOL "" FORCE)
            endif()
            set(BUILD_WITH_COMMON_SYSTEM ON CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "container_system")
            set(BUILD_CONTAINERSYSTEM_AS_SUBMODULE ON CACHE BOOL "" FORCE)
            set(BUILD_CONTAINER_SAMPLES OFF CACHE BOOL "" FORCE)
            set(BUILD_CONTAINER_EXAMPLES OFF CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "network_system")
            set(NETWORK_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
            set(NETWORK_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "database_system")
            set(BUILD_DATABASE_SAMPLES OFF CACHE BOOL "" FORCE)
            set(DATABASE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
            set(DATABASE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(USE_UNIT_TEST OFF CACHE BOOL "" FORCE)
        endif()

        # Temporarily skip install rules for dependencies
        set(_skip_install_save "${CMAKE_SKIP_INSTALL_RULES}")
        set(CMAKE_SKIP_INSTALL_RULES ON)

        add_subdirectory("${_local_path}" "${CMAKE_BINARY_DIR}/_local/${DEP_NAME}")

        set(CMAKE_SKIP_INSTALL_RULES "${_skip_install_save}")

        # Post-load handling
        if(${DEP_NAME} STREQUAL "logger_system")
            if(TARGET logger AND TARGET utilities AND TARGET interfaces)
                target_link_libraries(logger PUBLIC utilities interfaces)
            endif()
        endif()

        _unified_find_existing_target(${DEP_NAME} _new_target)
        if(_new_target)
            set(${DEP_NAME}_TARGET "${_new_target}")
            set(${DEP_NAME}_FOUND TRUE)
            list(APPEND _UNIFIED_LOADED_DEPS "${DEP_NAME}")
            set(_UNIFIED_LOADED_DEPS "${_UNIFIED_LOADED_DEPS}" CACHE INTERNAL "List of loaded dependencies")
        endif()
    else()
        if(${IS_REQUIRED})
            message(STATUS "[UnifiedDependencies] ${DEP_NAME}: NOT FOUND at ${_local_path}")
        else()
            message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Not found (optional)")
        endif()
        set(${DEP_NAME}_FOUND FALSE)
    endif()
endmacro()

# =============================================================================
# FetchContent Resolution
# =============================================================================

macro(_unified_resolve_fetchcontent DEP_NAME GIT_TAG)
    set(_fetch_name "${_UNIFIED_FETCH_NAME_${DEP_NAME}}")
    set(_repo_name "${_UNIFIED_REPO_${DEP_NAME}}")

    if(NOT _fetch_name OR NOT _repo_name)
        message(WARNING "[UnifiedDependencies] Unknown dependency for FetchContent: ${DEP_NAME}")
        set(${DEP_NAME}_FOUND FALSE)
    else()
        message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Fetching from GitHub...")

        # Skip install rules during FetchContent
        set(_skip_install_save "${CMAKE_SKIP_INSTALL_RULES}")
        set(CMAKE_SKIP_INSTALL_RULES TRUE)

        # Special pre-fetch configuration
        if(${DEP_NAME} STREQUAL "thread_system")
            set(USE_STD_FORMAT ON CACHE BOOL "Use std::format" FORCE)
            set(BUILD_THREADSYSTEM_AS_SUBMODULE ON CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "logger_system")
            set(LOGGER_STANDALONE_MODE ON CACHE BOOL "" FORCE)
            set(BUILD_WITH_COMMON_SYSTEM ON CACHE BOOL "" FORCE)
            set(LOGGER_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
            set(LOGGER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "container_system")
            set(BUILD_CONTAINERSYSTEM_AS_SUBMODULE ON CACHE BOOL "" FORCE)
            set(BUILD_CONTAINER_SAMPLES OFF CACHE BOOL "" FORCE)
            set(BUILD_CONTAINER_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(CONTAINER_SKIP_DEPENDENCIES ON CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "network_system")
            set(NETWORK_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
            set(NETWORK_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(WITH_MONITORING_SYSTEM OFF CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "monitoring_system")
            set(MONITORING_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(MONITORING_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        elseif(${DEP_NAME} STREQUAL "database_system")
            set(USE_POSTGRESQL OFF CACHE BOOL "" FORCE)
            set(USE_MYSQL OFF CACHE BOOL "" FORCE)
            set(USE_SQLITE OFF CACHE BOOL "" FORCE)
            set(DATABASE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
            set(DATABASE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        endif()

        FetchContent_Declare(
            ${_fetch_name}
            GIT_REPOSITORY https://github.com/${UNIFIED_GITHUB_ORG}/${_repo_name}.git
            GIT_TAG ${GIT_TAG}
            GIT_SHALLOW TRUE
        )

        FetchContent_MakeAvailable(${_fetch_name})

        set(CMAKE_SKIP_INSTALL_RULES "${_skip_install_save}")

        # Post-fetch: Set paths for dependent systems
        if(${DEP_NAME} STREQUAL "common_system")
            # Get the source directory and set it for other systems to find
            FetchContent_GetProperties(${_fetch_name} SOURCE_DIR _cs_source_dir)
            set(COMMON_SYSTEM_DIR "${_cs_source_dir}/include" CACHE PATH "common_system include directory" FORCE)
            set(COMMON_SYSTEM_INCLUDE_DIR "${_cs_source_dir}/include" CACHE PATH "" FORCE)
            set(COMMON_SYSTEM_ROOT "${_cs_source_dir}" CACHE PATH "" FORCE)
            set(common_system_SOURCE_DIR "${_cs_source_dir}" CACHE STRING "" FORCE)
            set(common_system_FOUND TRUE CACHE BOOL "" FORCE)

            # Create a fake config file so find_package(common_system) succeeds in dependent projects
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/common_system-config.cmake"
                "set(common_system_FOUND TRUE)\n"
                "set(COMMON_SYSTEM_INCLUDE_DIR \"${_cs_source_dir}/include\")\n"
                "set(COMMON_SYSTEM_DIR \"${_cs_source_dir}/include\")\n"
                "set(common_system_SOURCE_DIR \"${_cs_source_dir}\")\n"
            )
            list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}")
            set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING "" FORCE)

            message(STATUS "[UnifiedDependencies] common_system: Set COMMON_SYSTEM_DIR=${_cs_source_dir}/include")
        endif()

        # Post-fetch: Set thread_system paths
        if(${DEP_NAME} STREQUAL "thread_system")
            FetchContent_GetProperties(${_fetch_name} SOURCE_DIR _ts_source_dir)
            set(thread_system_SOURCE_DIR "${_ts_source_dir}" CACHE STRING "" FORCE)
            set(thread_system_FOUND TRUE CACHE BOOL "" FORCE)

            # Create a fake config file for dependent projects
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/thread_system-config.cmake"
                "set(thread_system_FOUND TRUE)\n"
                "set(thread_system_SOURCE_DIR \"${_ts_source_dir}\")\n"
            )
        endif()

        # Post-fetch: Set logger_system paths
        if(${DEP_NAME} STREQUAL "logger_system")
            FetchContent_GetProperties(${_fetch_name} SOURCE_DIR _ls_source_dir)
            set(logger_system_SOURCE_DIR "${_ls_source_dir}" CACHE STRING "" FORCE)
            set(logger_system_FOUND TRUE CACHE BOOL "" FORCE)

            # Create a fake config file for dependent projects
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/logger_system-config.cmake"
                "set(logger_system_FOUND TRUE)\n"
                "set(logger_system_SOURCE_DIR \"${_ls_source_dir}\")\n"
            )
        endif()

        # Post-fetch: Set container_system paths
        if(${DEP_NAME} STREQUAL "container_system")
            FetchContent_GetProperties(${_fetch_name} SOURCE_DIR _cts_source_dir)
            set(container_system_SOURCE_DIR "${_cts_source_dir}" CACHE STRING "" FORCE)
            set(container_system_FOUND TRUE CACHE BOOL "" FORCE)

            # Create a fake config file for dependent projects
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/container_system-config.cmake"
                "set(container_system_FOUND TRUE)\n"
                "set(container_system_SOURCE_DIR \"${_cts_source_dir}\")\n"
            )
        endif()

        # Post-fetch configuration for thread_system
        if(${DEP_NAME} STREQUAL "thread_system" AND USE_STD_FORMAT)
            set(_ts_targets thread_base thread_pool interfaces utilities typed_thread_pool)
            foreach(_ts_target IN LISTS _ts_targets)
                if(TARGET "${_ts_target}")
                    get_target_property(_aliased "${_ts_target}" ALIASED_TARGET)
                    if(NOT _aliased)
                        get_target_property(_ts_type "${_ts_target}" TYPE)
                        if(_ts_type STREQUAL "INTERFACE_LIBRARY")
                            target_compile_definitions("${_ts_target}" INTERFACE USE_STD_FORMAT)
                        else()
                            target_compile_definitions("${_ts_target}" PUBLIC USE_STD_FORMAT)
                        endif()
                    endif()
                endif()
            endforeach()
        endif()

        _unified_find_existing_target(${DEP_NAME} _new_target)
        if(_new_target)
            set(${DEP_NAME}_TARGET "${_new_target}")
            set(${DEP_NAME}_FOUND TRUE)
            list(APPEND _UNIFIED_LOADED_DEPS "${DEP_NAME}")
            set(_UNIFIED_LOADED_DEPS "${_UNIFIED_LOADED_DEPS}" CACHE INTERNAL "List of loaded dependencies")
            message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Fetched successfully (target: ${_new_target})")
        else()
            message(WARNING "[UnifiedDependencies] ${DEP_NAME}: Fetched but no target found")
            set(${DEP_NAME}_FOUND FALSE)
        endif()
    endif()
endmacro()

# =============================================================================
# find_package Resolution
# =============================================================================

macro(_unified_resolve_find_package DEP_NAME VERSION IS_REQUIRED)
    set(_pkg_name "${_UNIFIED_PACKAGE_NAME_${DEP_NAME}}")

    if(NOT _pkg_name)
        set(_pkg_name "${DEP_NAME}")
    endif()

    # Try find_package
    if(VERSION)
        find_package(${_pkg_name} ${VERSION} QUIET)
    else()
        find_package(${_pkg_name} QUIET)
    endif()

    if(${_pkg_name}_FOUND)
        message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Found via find_package")
        _unified_find_existing_target(${DEP_NAME} _new_target)
        set(${DEP_NAME}_TARGET "${_new_target}")
        set(${DEP_NAME}_FOUND TRUE)
        list(APPEND _UNIFIED_LOADED_DEPS "${DEP_NAME}")
        set(_UNIFIED_LOADED_DEPS "${_UNIFIED_LOADED_DEPS}" CACHE INTERNAL "List of loaded dependencies")
    elseif(UNIFIED_ALLOW_FETCHCONTENT_FALLBACK)
        # Fallback to FetchContent
        message(STATUS "[UnifiedDependencies] ${DEP_NAME}: Not found, trying FetchContent fallback...")
        _unified_resolve_fetchcontent(${DEP_NAME} "main")
    else()
        set(${DEP_NAME}_FOUND FALSE)
    endif()
endmacro()

# =============================================================================
# Alias Creation Helper
# =============================================================================

# Create standardized aliases for consistent linking
# Call after all dependencies are loaded
macro(unified_create_standard_aliases)
    # Common aliases that consumers might expect
    foreach(_dep common_system thread_system logger_system monitoring_system
                 container_system database_system network_system)
        if(${_dep}_TARGET)
            _unified_get_primary_target(${_dep} _primary)

            # Create alias if primary target doesn't exist but found target does
            if(NOT TARGET ${_primary} AND TARGET ${${_dep}_TARGET})
                # Can't alias an alias, check if it's a real target
                get_target_property(_aliased ${${_dep}_TARGET} ALIASED_TARGET)
                if(_aliased)
                    # Target is already an alias, create alias to the real target
                    if(NOT TARGET ${_primary})
                        add_library(${_primary} ALIAS ${_aliased})
                    endif()
                else()
                    if(NOT TARGET ${_primary})
                        add_library(${_primary} ALIAS ${${_dep}_TARGET})
                    endif()
                endif()
            endif()
        endif()
    endforeach()
endmacro()

# =============================================================================
# Dependency Status Summary
# =============================================================================

macro(unified_print_dependency_summary)
    message(STATUS "")
    message(STATUS "=== Dependency Summary ===")

    foreach(_dep IN LISTS _UNIFIED_LOADED_DEPS)
        if(${_dep}_TARGET)
            message(STATUS "  [OK] ${_dep} -> ${${_dep}_TARGET}")
        else()
            message(STATUS "  [--] ${_dep} (not loaded)")
        endif()
    endforeach()

    message(STATUS "==========================")
    message(STATUS "")
endmacro()
