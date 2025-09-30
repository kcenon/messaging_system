cmake_minimum_required(VERSION 3.16)

# Project configuration
project(messaging_system 
    VERSION 1.0.0.0
    DESCRIPTION "Messaging system with container, database, and network modules"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(USE_UNIT_TEST "Use unit test" ON)
option(BUILD_MESSAGING_SAMPLES "Build messaging system samples" ON)

# Module options
option(BUILD_CONTAINER "Build container module" ON)
# Database module removed - not used
# option(BUILD_DATABASE "Build database module" OFF)
option(BUILD_NETWORK "Build network module" ON)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)  # Handled in services_system

# Feature options
option(USE_LOCKFREE_BY_DEFAULT "Use lock-free implementations by default" OFF)
option(USE_STD_FORMAT "Use std::format instead of fmt library" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10
elseif(APPLE)
    add_definitions(-DAPPLE_PLATFORM)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Find external ThreadSystemCore
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Option to use internal thread_system (for backward compatibility)
option(USE_INTERNAL_THREAD_SYSTEM "Use internal thread_system subdirectory" OFF)

# Option to use external logger system
option(USE_EXTERNAL_LOGGER_SYSTEM "Use external logger system" ON)

# Option to use external monitoring system
option(USE_EXTERNAL_MONITORING_SYSTEM "Use external monitoring system" ON)

# Option to use external common system
option(USE_EXTERNAL_COMMON_SYSTEM "Use external common system" ON)

# Option to use external container system
option(USE_EXTERNAL_CONTAINER_SYSTEM "Use external container system" ON)

# Option to use external database system
option(USE_EXTERNAL_DATABASE_SYSTEM "Use external database system" ON)

# Option to build unified messaging system
option(BUILD_UNIFIED_MESSAGING "Build unified messaging system with all integrations" ON)

if(USE_INTERNAL_THREAD_SYSTEM AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/thread_system)
    # Legacy mode: use internal thread_system
    message(STATUS "Using internal thread_system subdirectory")
    set(BUILD_THREADSYSTEM_AS_SUBMODULE ON CACHE BOOL "Build ThreadSystem as submodule" FORCE)
    
    # Pass lock-free option to thread_system
    if(USE_LOCKFREE_BY_DEFAULT)
        set(USE_LOCKFREE_THREAD_POOL ON CACHE BOOL "Use lock-free thread pool by default" FORCE)
        set(USE_LOCKFREE_LOGGER ON CACHE BOOL "Use lock-free logger by default" FORCE)
    endif()
    
    add_subdirectory(libraries/thread_system)
    
    # Include directories for thread_system
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/thread_system/sources
    )
else()
    # Modern mode: use external ThreadSystemCore
    message(STATUS "Attempting to find external ThreadSystemCore")
    find_package(ThreadSystemCore QUIET)
    
    # Create compatibility aliases for internal targets
    if(TARGET ThreadSystem::Core)
        # Create aliases for backward compatibility
        add_library(thread_pool ALIAS ThreadSystem::Core)
        add_library(thread_base ALIAS ThreadSystem::Core)
        add_library(utilities ALIAS ThreadSystem::Core)
        add_library(typed_thread_pool ALIAS ThreadSystem::Core)
    endif()
endif()

# Logger System Integration
option(USE_INTERNAL_LOGGER_SYSTEM "Use internal logger_system subdirectory" ON)

if(USE_INTERNAL_LOGGER_SYSTEM AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/logger_system)
    # Use internal logger_system
    message(STATUS "Using internal logger_system subdirectory")
    set(BUILD_LOGGER_AS_SUBMODULE ON CACHE BOOL "Build Logger as submodule" FORCE)
    add_subdirectory(libraries/logger_system)

    # Include directories for logger_system
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/logger_system/sources
    )
elseif(USE_EXTERNAL_LOGGER_SYSTEM)
    message(STATUS "Attempting to find external LoggerSystem")
    find_package(LoggerSystem QUIET)

    # Make logger available to all modules
    if(TARGET LoggerSystem::logger)
        add_library(logger ALIAS LoggerSystem::logger)
    endif()
else()
    message(STATUS "Logger system integration disabled")
endif()

# Monitoring System Integration
option(USE_INTERNAL_MONITORING_SYSTEM "Use internal monitoring_system subdirectory" OFF)

if(USE_INTERNAL_MONITORING_SYSTEM AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/monitoring_system)
    # Use internal monitoring_system
    message(STATUS "Using internal monitoring_system subdirectory")
    set(BUILD_MONITORING_AS_SUBMODULE ON CACHE BOOL "Build Monitoring as submodule" FORCE)
    add_subdirectory(libraries/monitoring_system)

    # Include directories for monitoring_system
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/monitoring_system/sources
    )
elseif(USE_EXTERNAL_MONITORING_SYSTEM)
    message(STATUS "Attempting to find external MonitoringSystem")
    find_package(MonitoringSystem QUIET)

    # Make monitoring available to all modules
    if(TARGET MonitoringSystem::monitoring)
        add_library(monitoring ALIAS MonitoringSystem::monitoring)
    endif()
else()
    message(STATUS "Monitoring system integration disabled")
endif()

# Messaging system modules
if(BUILD_CONTAINER)
    add_subdirectory(libraries/container_system)
    # # Add container to the export set
    # install(TARGETS container
    #     EXPORT messaging_system_targets
    #     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    # )
endif()

# Database module removed - not used

if(BUILD_NETWORK)
    add_subdirectory(libraries/network_system)
endif()

# Add application layer
add_subdirectory(application_layer)

# Python bindings are now handled within services_system
# The old python directory has been removed to avoid duplication

# Unit tests
if(USE_UNIT_TEST)
    enable_testing()
    add_subdirectory(test/unittest)
endif()

# Samples
if(BUILD_MESSAGING_SAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples)
    add_subdirectory(samples)
endif()

# Find external system modules
set(EXTERNAL_SYSTEMS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Path to external system modules")

# Common System
# First check if we have it in libraries folder
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/common_system)
    message(STATUS "Using internal common_system from libraries folder")
    add_subdirectory(libraries/common_system)
    set(HAS_COMMON_SYSTEM ON)
elseif(USE_EXTERNAL_COMMON_SYSTEM)
    if(EXISTS ${EXTERNAL_SYSTEMS_PATH}/common_system)
        add_subdirectory(${EXTERNAL_SYSTEMS_PATH}/common_system ${CMAKE_CURRENT_BINARY_DIR}/common_system)
        set(HAS_COMMON_SYSTEM ON)
        message(STATUS "Found common_system at ${EXTERNAL_SYSTEMS_PATH}/common_system")
    else()
        message(STATUS "Common system not found at ${EXTERNAL_SYSTEMS_PATH}/common_system")
    endif()
endif()

# Thread System (already handled above)
if(TARGET ThreadSystem::Core OR TARGET thread_pool OR TARGET thread_base)
    set(HAS_THREAD_SYSTEM ON)
    message(STATUS "Thread system available")
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/thread_system)
    # Internal thread_system in libraries
    set(HAS_THREAD_SYSTEM ON)
    message(STATUS "Using internal thread_system from libraries")
elseif(EXISTS ${EXTERNAL_SYSTEMS_PATH}/thread_system)
    # External thread_system
    add_subdirectory(${EXTERNAL_SYSTEMS_PATH}/thread_system ${CMAKE_CURRENT_BINARY_DIR}/thread_system_ext)
    set(HAS_THREAD_SYSTEM ON)
    message(STATUS "Found thread_system at ${EXTERNAL_SYSTEMS_PATH}/thread_system")
endif()

# Logger System (already handled above)
if(TARGET LoggerSystem::logger OR TARGET logger)
    set(HAS_LOGGER_SYSTEM ON)
endif()

# Monitoring System
if(USE_EXTERNAL_MONITORING_SYSTEM)
    if(EXISTS ${EXTERNAL_SYSTEMS_PATH}/monitoring_system)
        add_subdirectory(${EXTERNAL_SYSTEMS_PATH}/monitoring_system ${CMAKE_CURRENT_BINARY_DIR}/monitoring_system)
        set(HAS_MONITORING_SYSTEM ON)
        message(STATUS "Found monitoring_system at ${EXTERNAL_SYSTEMS_PATH}/monitoring_system")
    else()
        message(STATUS "Monitoring system not found at ${EXTERNAL_SYSTEMS_PATH}/monitoring_system")
    endif()
endif()

# Container System
if(USE_EXTERNAL_CONTAINER_SYSTEM)
    if(EXISTS ${EXTERNAL_SYSTEMS_PATH}/container_system)
        add_subdirectory(${EXTERNAL_SYSTEMS_PATH}/container_system ${CMAKE_CURRENT_BINARY_DIR}/container_system_ext)
        set(HAS_CONTAINER_SYSTEM ON)
        message(STATUS "Found container_system at ${EXTERNAL_SYSTEMS_PATH}/container_system")
    else()
        message(STATUS "Container system not found at ${EXTERNAL_SYSTEMS_PATH}/container_system")
    endif()
endif()

# Database System removed - not used

# Network System
if(BUILD_NETWORK AND EXISTS ${EXTERNAL_SYSTEMS_PATH}/network_system)
    add_subdirectory(${EXTERNAL_SYSTEMS_PATH}/network_system ${CMAKE_CURRENT_BINARY_DIR}/network_system_ext)
    set(HAS_NETWORK_SYSTEM ON)
    message(STATUS "Found network_system at ${EXTERNAL_SYSTEMS_PATH}/network_system")
endif()

# Build Unified Messaging System
if(BUILD_UNIFIED_MESSAGING)
    message(STATUS "Building Unified Messaging System with available integrations:")
    message(STATUS "  Common System: ${HAS_COMMON_SYSTEM}")
    message(STATUS "  Thread System: ${HAS_THREAD_SYSTEM}")
    message(STATUS "  Logger System: ${HAS_LOGGER_SYSTEM}")
    message(STATUS "  Monitoring System: ${HAS_MONITORING_SYSTEM}")
    message(STATUS "  Container System: ${HAS_CONTAINER_SYSTEM}")
    # Database System: removed - not used
    message(STATUS "  Network System: ${HAS_NETWORK_SYSTEM}")

    # Create unified messaging library
    add_library(unified_messaging STATIC
        src/unified_messaging_system.cpp
    )

    target_include_directories(unified_messaging PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Add include directories for external systems
    if(HAS_THREAD_SYSTEM)
        target_include_directories(unified_messaging PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../thread_system/include
            ${CMAKE_CURRENT_SOURCE_DIR}/libraries/thread_system/sources
        )
    endif()

    if(HAS_LOGGER_SYSTEM)
        target_include_directories(unified_messaging PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../logger_system/include
            ${CMAKE_CURRENT_SOURCE_DIR}/libraries/logger_system/include
        )
        # Logger system may need thread_system headers
        if(HAS_THREAD_SYSTEM)
            target_compile_definitions(unified_messaging PRIVATE USE_THREAD_SYSTEM_INTEGRATION)
        endif()
    endif()

    if(HAS_MONITORING_SYSTEM)
        target_include_directories(unified_messaging PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../monitoring_system/include
        )
    endif()

    if(HAS_CONTAINER_SYSTEM)
        target_include_directories(unified_messaging SYSTEM PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../container_system
            ${CMAKE_CURRENT_SOURCE_DIR}/libraries/container_system
        )
    endif()

    # Database system removed - not used

    if(HAS_NETWORK_SYSTEM)
        target_include_directories(unified_messaging PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../network_system/include
            ${CMAKE_CURRENT_SOURCE_DIR}/libraries/network_system
        )
    endif()

    # Set compilation definitions based on available systems
    if(HAS_COMMON_SYSTEM)
        target_compile_definitions(unified_messaging PUBLIC HAS_COMMON_SYSTEM)
        if(TARGET common_system)
            target_link_libraries(unified_messaging PUBLIC common_system)
        endif()
    endif()

    if(HAS_THREAD_SYSTEM)
        target_compile_definitions(unified_messaging PUBLIC HAS_THREAD_SYSTEM)
        if(TARGET thread_pool)
            target_link_libraries(unified_messaging PUBLIC thread_pool)
        elseif(TARGET ThreadSystem::Core)
            target_link_libraries(unified_messaging PUBLIC ThreadSystem::Core)
        endif()
    endif()

    if(HAS_LOGGER_SYSTEM)
        target_compile_definitions(unified_messaging PUBLIC HAS_LOGGER_SYSTEM)
        if(TARGET logger)
            target_link_libraries(unified_messaging PUBLIC logger)
        elseif(TARGET LoggerSystem::logger)
            target_link_libraries(unified_messaging PUBLIC LoggerSystem::logger)
        endif()
    endif()

    if(HAS_MONITORING_SYSTEM)
        target_compile_definitions(unified_messaging PUBLIC HAS_MONITORING_SYSTEM)
        if(TARGET monitoring_system)
            target_link_libraries(unified_messaging PUBLIC monitoring_system)
        elseif(TARGET MonitoringSystem::monitoring)
            target_link_libraries(unified_messaging PUBLIC MonitoringSystem::monitoring)
        endif()
    endif()

    if(HAS_CONTAINER_SYSTEM)
        target_compile_definitions(unified_messaging PUBLIC HAS_CONTAINER_SYSTEM)
        if(TARGET container_system)
            target_link_libraries(unified_messaging PUBLIC container_system)
        elseif(TARGET container)
            target_link_libraries(unified_messaging PUBLIC container)
        endif()
    endif()

    # Database system removed - not used

    if(HAS_NETWORK_SYSTEM)
        target_compile_definitions(unified_messaging PUBLIC HAS_NETWORK_SYSTEM)
        if(TARGET NetworkSystem)
            target_link_libraries(unified_messaging PUBLIC NetworkSystem)
        elseif(TARGET network_system)
            target_link_libraries(unified_messaging PUBLIC network_system)
        elseif(TARGET network)
            target_link_libraries(unified_messaging PUBLIC network)
        endif()
    endif()

    # Always need threads
    target_link_libraries(unified_messaging PUBLIC Threads::Threads)

    # Set C++ standard
    target_compile_features(unified_messaging PUBLIC cxx_std_20)

    # Install unified messaging
    install(TARGETS unified_messaging
        EXPORT unified_messaging_targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(FILES
        include/kcenon/messaging/unified_messaging_system.h
        include/kcenon/messaging/result_adapter.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kcenon/messaging
    )

    # Build example if requested
    if(BUILD_MESSAGING_SAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/unified_messaging_example.cpp)
        add_executable(unified_messaging_example examples/unified_messaging_example.cpp)
        target_link_libraries(unified_messaging_example PRIVATE unified_messaging)
        target_compile_features(unified_messaging_example PRIVATE cxx_std_20)
    endif()

    # Build test if requested
    if(USE_UNIT_TEST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/unified_messaging_test.cpp)
        find_package(GTest QUIET)
        if(GTest_FOUND)
            add_executable(unified_messaging_test tests/unified_messaging_test.cpp)
            target_link_libraries(unified_messaging_test PRIVATE unified_messaging GTest::gtest_main)
            target_compile_features(unified_messaging_test PRIVATE cxx_std_20)
            add_test(NAME unified_messaging_test COMMAND unified_messaging_test)
        endif()
    endif()
endif()

# Integration example
if(BUILD_MESSAGING_SAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration_example.cpp)
    add_executable(integration_example integration_example.cpp)
    
    # Define macros based on what's available
    if(TARGET ThreadSystem::Core)
        target_compile_definitions(integration_example PRIVATE HAS_THREAD_SYSTEM_CORE)
        target_link_libraries(integration_example PRIVATE ThreadSystem::Core)
    endif()
    
    if(TARGET LoggerSystem::logger)
        target_compile_definitions(integration_example PRIVATE HAS_LOGGER_SYSTEM)
        target_link_libraries(integration_example PRIVATE LoggerSystem::logger)
    endif()
    
    if(TARGET MonitoringSystem::monitoring)
        target_compile_definitions(integration_example PRIVATE HAS_MONITORING_SYSTEM)
        target_link_libraries(integration_example PRIVATE MonitoringSystem::monitoring)
    endif()
    
    # Link messaging system components
    target_link_libraries(integration_example PRIVATE container network)
endif()

# Installation configuration
include(GNUInstallDirs)

# Install header files
if(BUILD_CONTAINER)
    install(DIRECTORY libraries/container_system/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/container
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.tpp"
        PATTERN "tests" EXCLUDE
        PATTERN "internal" EXCLUDE
    )
endif()

# Database system removed

if(BUILD_NETWORK)
    install(DIRECTORY libraries/network_system/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/network
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
        PATTERN "tests" EXCLUDE
    )
endif()

# Export configuration - temporarily commented out
# install(EXPORT messaging_system_targets
#     FILE messaging_system_targets.cmake
#     NAMESPACE messaging_system::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/messaging_system
# )

# # Create config file
# include(CMakePackageConfigHelpers)
# configure_package_config_file(
#     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/messaging_system_config.cmake.in
#     ${CMAKE_CURRENT_BINARY_DIR}/messaging_system_config.cmake
#     INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/messaging_system
# )

# write_basic_package_version_file(
#     ${CMAKE_CURRENT_BINARY_DIR}/messaging_system_config_version.cmake
#     VERSION ${PROJECT_VERSION}
#     COMPATIBILITY SameMajorVersion
# )

# install(FILES
#     ${CMAKE_CURRENT_BINARY_DIR}/messaging_system_config.cmake
#     ${CMAKE_CURRENT_BINARY_DIR}/messaging_system_config_version.cmake
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/messaging_system
# )