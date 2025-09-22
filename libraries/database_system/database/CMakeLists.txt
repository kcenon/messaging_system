##################################################
# Database Module CMakeLists.txt
#
# Builds the database library for the database system.
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
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_pool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/query_builder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/mysql/mysql_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/sqlite/sqlite_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/mongodb/mongodb_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/redis/redis_manager.h
    # Phase 4 Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/orm/entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/monitoring/performance_monitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/security/secure_connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/async/async_operations.h
)

# Collect all source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/database_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/postgres_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connection_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/query_builder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/mysql/mysql_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/sqlite/sqlite_manager.cpp
    # Phase 4 Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/orm/entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/monitoring/performance_monitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/mongodb/mongodb_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/redis/redis_manager.cpp
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
    $<INSTALL_INTERFACE:include/database_system>
)

##################################################
# Dependencies
##################################################

# Find required packages
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

# PostgreSQL dependency (optional)
if(USE_POSTGRESQL)
    find_package(libpqxx CONFIG QUIET)
    find_package(OpenSSL QUIET)

    if(libpqxx_FOUND AND OPENSSL_FOUND)
        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                libpqxx::pqxx
                OpenSSL::SSL
        )
        target_compile_definitions(${PROJECT_NAME} PUBLIC USE_POSTGRESQL)
        message(STATUS "PostgreSQL support enabled")
        message(STATUS "  libpqxx found: ${libpqxx_FOUND}")
        message(STATUS "  OpenSSL found: ${OPENSSL_FOUND}")
    else()
        message(WARNING "PostgreSQL libraries not found, disabling PostgreSQL support")
        message(STATUS "  libpqxx found: ${libpqxx_FOUND}")
        message(STATUS "  OpenSSL found: ${OPENSSL_FOUND}")
        set(USE_POSTGRESQL OFF CACHE BOOL "PostgreSQL support disabled due to missing libraries" FORCE)
    endif()
endif()

# MySQL dependency (optional)
if(USE_MYSQL)
    find_package(unofficial-libmysql CONFIG QUIET)

    if(unofficial-libmysql_FOUND)
        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                unofficial::libmysql::libmysql
        )
        target_compile_definitions(${PROJECT_NAME} PUBLIC USE_MYSQL)
        message(STATUS "MySQL support enabled")
        message(STATUS "  libmysql found: ${unofficial-libmysql_FOUND}")
    else()
        message(WARNING "MySQL libraries not found, disabling MySQL support")
        message(STATUS "  libmysql found: ${unofficial-libmysql_FOUND}")
        set(USE_MYSQL OFF CACHE BOOL "MySQL support disabled due to missing libraries" FORCE)
    endif()
endif()

# SQLite dependency (optional)
if(USE_SQLITE)
    find_package(unofficial-sqlite3 CONFIG QUIET)

    if(unofficial-sqlite3_FOUND)
        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                unofficial::sqlite3::sqlite3
        )
        target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SQLITE)
        message(STATUS "SQLite support enabled")
        message(STATUS "  sqlite3 found: ${unofficial-sqlite3_FOUND}")
    else()
        message(WARNING "SQLite libraries not found, disabling SQLite support")
        message(STATUS "  sqlite3 found: ${unofficial-sqlite3_FOUND}")
        set(USE_SQLITE OFF CACHE BOOL "SQLite support disabled due to missing libraries" FORCE)
    endif()
endif()

# MongoDB dependency (optional)
if(USE_MONGODB)
    find_package(mongocxx CONFIG QUIET)
    find_package(bsoncxx CONFIG QUIET)

    if(mongocxx_FOUND AND bsoncxx_FOUND)
        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                mongo::mongocxx_shared
                mongo::bsoncxx_shared
        )
        target_compile_definitions(${PROJECT_NAME} PUBLIC USE_MONGODB)
        message(STATUS "MongoDB support enabled")
        message(STATUS "  mongocxx found: ${mongocxx_FOUND}")
        message(STATUS "  bsoncxx found: ${bsoncxx_FOUND}")
    else()
        message(WARNING "MongoDB libraries not found, disabling MongoDB support")
        message(STATUS "  mongocxx found: ${mongocxx_FOUND}")
        message(STATUS "  bsoncxx found: ${bsoncxx_FOUND}")
        set(USE_MONGODB OFF CACHE BOOL "MongoDB support disabled due to missing libraries" FORCE)
    endif()
endif()

# Redis dependency (optional)
if(USE_REDIS)
    find_package(hiredis CONFIG QUIET)

    if(hiredis_FOUND)
        target_link_libraries(${PROJECT_NAME}
            PUBLIC
                hiredis::hiredis
        )
        target_compile_definitions(${PROJECT_NAME} PUBLIC USE_REDIS)
        message(STATUS "Redis support enabled")
        message(STATUS "  hiredis found: ${hiredis_FOUND}")
    else()
        message(WARNING "Redis libraries not found, disabling Redis support")
        message(STATUS "  hiredis found: ${hiredis_FOUND}")
        set(USE_REDIS OFF CACHE BOOL "Redis support disabled due to missing libraries" FORCE)
    endif()
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
# Testing Support
##################################################

# Enable testing features if building tests
if(USE_UNIT_TEST AND NOT BUILD_DATABASE_SYSTEM_AS_SUBMODULE)
    # Add any database-specific test configurations here
    set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)
endif()

##################################################
# Summary
##################################################

message(STATUS "Database library configured:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Headers: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Dependencies: Threads, libpqxx (if enabled), libmysql (if enabled)")