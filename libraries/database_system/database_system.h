/**
 * @file database_system.h
 * @brief Main include file for the database system library
 *
 * This file includes all the public headers from the database system.
 * Users should include this file to use the database functionality.
 *
 * @author 🍀☀🌕🌥 🌊
 * @version 1.0.0
 */

#pragma once

// Core database components
#include "database/database_base.h"
#include "database/database_manager.h"
#include "database/database_types.h"

// Connection management
#include "database/connection_pool.h"

// Query building
#include "database/query_builder.h"

// Database backends
#ifdef DATABASE_ENABLE_POSTGRES
#include "database/postgres_manager.h"
#endif

#ifdef DATABASE_ENABLE_MYSQL
#include "database/backends/mysql/mysql_manager.h"
#endif

#ifdef DATABASE_ENABLE_SQLITE
#include "database/backends/sqlite/sqlite_manager.h"
#endif

#ifdef DATABASE_ENABLE_REDIS
#include "database/backends/redis/redis_manager.h"
#endif

#ifdef DATABASE_ENABLE_MONGODB
#include "database/backends/mongodb/mongodb_manager.h"
#endif

// Advanced features (optional)
#ifdef DATABASE_ENABLE_ASYNC
#include "database/async/async_operations.h"
#endif

#ifdef DATABASE_ENABLE_SECURITY
#include "database/security/secure_connection.h"
#endif

#ifdef DATABASE_ENABLE_MONITORING
#include "database/monitoring/performance_monitor.h"
#endif

#ifdef DATABASE_ENABLE_ORM
#include "database/orm/entity.h"
#endif

// Namespace aliases for compatibility
namespace database_system = database;

// Version information
namespace database {
    constexpr const char* VERSION = "1.0.0";
    constexpr int VERSION_MAJOR = 1;
    constexpr int VERSION_MINOR = 0;
    constexpr int VERSION_PATCH = 0;
}