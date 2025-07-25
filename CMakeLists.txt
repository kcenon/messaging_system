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
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

# Feature options
option(USE_LOCKFREE_BY_DEFAULT "Use lock-free implementations by default" OFF)

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

# Add the thread_system as a subdirectory
# Set it to build as a submodule (no samples/tests)
set(BUILD_THREADSYSTEM_AS_SUBMODULE ON CACHE BOOL "Build ThreadSystem as submodule" FORCE)

# Pass lock-free option to thread_system
if(USE_LOCKFREE_BY_DEFAULT)
    set(USE_LOCKFREE_THREAD_POOL ON CACHE BOOL "Use lock-free thread pool by default" FORCE)
    set(USE_LOCKFREE_LOGGER ON CACHE BOOL "Use lock-free logger by default" FORCE)
endif()

add_subdirectory(thread_system)

# Include directories for thread_system
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_system/sources
)

# Thread system is built as an internal dependency, not exported

# Messaging system modules
if(BUILD_CONTAINER)
    add_subdirectory(container)
    # # Add container to the export set
    # install(TARGETS container
    #     EXPORT messaging_system_targets
    #     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    # )
endif()

if(BUILD_DATABASE)
    add_subdirectory(database)
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
    add_subdirectory(network)
    # # Add network to the export set
    # if(TARGET network)
    #     install(TARGETS network
    #         EXPORT messaging_system_targets
    #         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    #     )
    # endif()
endif()

# Python bindings
if(BUILD_PYTHON_BINDINGS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/python)
    add_subdirectory(python)
endif()

# Unit tests
if(USE_UNIT_TEST)
    enable_testing()
    add_subdirectory(unittest)
endif()

# Samples
if(BUILD_MESSAGING_SAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples)
    add_subdirectory(samples)
endif()

# Installation configuration
include(GNUInstallDirs)

# Install header files
if(BUILD_CONTAINER)
    install(DIRECTORY container/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/container
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.tpp"
        PATTERN "tests" EXCLUDE
        PATTERN "internal" EXCLUDE
    )
endif()

if(BUILD_DATABASE)
    install(DIRECTORY database/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/messaging_system/database
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
        PATTERN "tests" EXCLUDE
    )
endif()

if(BUILD_NETWORK)
    install(DIRECTORY network/
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