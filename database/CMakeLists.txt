##################################################
# Database Module CMakeLists.txt
# 
# Builds the database library for the messaging system.
# Provides database abstraction and PostgreSQL implementation.
##################################################

# Project definition
project(database 
    VERSION 1.0.0
    DESCRIPTION "Database abstraction layer with PostgreSQL support"
    LANGUAGES CXX
)

##################################################
# Source Files Configuration
##################################################

# Collect all header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/database_base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/database_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/database_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/postgres_manager.h
)

# Collect all source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/database_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/postgres_manager.cpp
)

##################################################
# Library Target Configuration
##################################################

# Create static library
add_library(${PROJECT_NAME} STATIC 
    ${HEADER_FILES} 
    ${SOURCE_FILES}
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

##################################################
# Include Directories
##################################################

# Public include directories (for consumers of this library)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include/messaging_system>
)

# Include ThreadSystem utilities headers if available
if(TARGET utilities)
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/thread_system/sources/utilities>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/thread_system/sources/utilities/conversion>
    )
endif()

##################################################
# Dependencies
##################################################

# PostgreSQL dependency
find_package(PostgreSQL REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC PostgreSQL::PostgreSQL)

# Container library dependency
if(TARGET container)
    target_link_libraries(${PROJECT_NAME} PUBLIC container)
else()
    message(FATAL_ERROR "Container library is required for database module")
endif()

# Utilities library dependency
if(TARGET utilities)
    target_link_libraries(${PROJECT_NAME} PRIVATE utilities)
else()
    message(WARNING "utilities target not found - database may have missing dependencies")
endif()

##################################################
# Compiler Options
##################################################

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /WX-
    )
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WIN32_WINNT=0x0601  # Windows 7 or later
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

##################################################
# PostgreSQL Configuration
##################################################

# Display PostgreSQL configuration
message(STATUS "PostgreSQL Configuration:")
message(STATUS "  Version: ${PostgreSQL_VERSION_STRING}")
message(STATUS "  Include: ${PostgreSQL_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${PostgreSQL_LIBRARIES}")

# Add PostgreSQL-specific definitions if needed
if(PostgreSQL_VERSION_STRING VERSION_GREATER_EQUAL "14.0")
    target_compile_definitions(${PROJECT_NAME} PRIVATE POSTGRESQL_14_OR_NEWER)
endif()

##################################################
# Installation Rules (when not building as submodule)
##################################################

# Installation is handled by parent CMakeLists.txt

# Export configuration is handled by parent CMakeLists.txt

##################################################
# Testing Support
##################################################

# Enable testing features if building tests
if(BUILD_UNIT_TESTS AND NOT BUILD_MESSAGING_SYSTEM_AS_SUBMODULE)
    # Add any database-specific test configurations here
    set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)
endif()

##################################################
# Summary
##################################################

message(STATUS "Database library configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Headers: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Dependencies: PostgreSQL, container, utilities")