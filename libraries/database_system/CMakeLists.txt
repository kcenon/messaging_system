cmake_minimum_required(VERSION 3.16)

##################################################
# Database System CMakeLists.txt
#
# Independent database system with multi-backend support
# Provides database abstraction and PostgreSQL implementation.
##################################################

# Project definition
project(database_system
    VERSION 1.0.0
    DESCRIPTION "Advanced C++20 Database System with Multi-Backend Support"
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
option(BUILD_DATABASE_SAMPLES "Build database system samples" ON)
option(USE_POSTGRESQL "Enable PostgreSQL support" ON)
option(USE_MYSQL "Enable MySQL support" OFF)
option(USE_SQLITE "Enable SQLite support" OFF)
option(BUILD_DATABASE "Build database module" ON)

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

##################################################
# Modular Architecture
##################################################

# Database module
if(BUILD_DATABASE)
    add_subdirectory(database)
    message(STATUS "Database module will be built")
else()
    message(STATUS "Database module disabled")
endif()

##################################################
# Samples
##################################################

if(BUILD_DATABASE_SAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples)
    add_subdirectory(samples)
    message(STATUS "Database samples will be built")
endif()

##################################################
# Testing
##################################################

# Unit tests
if(USE_UNIT_TEST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Database tests will be built")
endif()

##################################################
# Installation Rules
##################################################

include(GNUInstallDirs)

# Install header files
if(BUILD_DATABASE)
    install(DIRECTORY database/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/database_system/database
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
        PATTERN "tests" EXCLUDE
        PATTERN "samples" EXCLUDE
    )
endif()

##################################################
# Summary
##################################################

message(STATUS "Database System configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Features:")
message(STATUS "    - PostgreSQL support: ${USE_POSTGRESQL}")
message(STATUS "    - MySQL support: ${USE_MYSQL}")
message(STATUS "    - SQLite support: ${USE_SQLITE}")
message(STATUS "    - Build samples: ${BUILD_DATABASE_SAMPLES}")
message(STATUS "    - Build tests: ${USE_UNIT_TEST}")
message(STATUS "  Output directories:")
message(STATUS "    - Runtime: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "    - Library: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "    - Archive: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")