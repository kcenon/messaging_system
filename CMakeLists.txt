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
option(BUILD_DATABASE "Build database module" ON)
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

if(BUILD_DATABASE)
    add_subdirectory(libraries/database_system)
    # # Add database to the export set
    # if(TARGET database)
    #     install(TARGETS database
    #         EXPORT messaging_system_targets
    #         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    #     )
    # endif()
endif()

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

if(BUILD_DATABASE)
    install(DIRECTORY libraries/database_system/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/database
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
        PATTERN "tests" EXCLUDE
    )
endif()

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