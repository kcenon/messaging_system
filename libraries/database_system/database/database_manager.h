/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

#include <memory>
#include <mutex>

#include "database_base.h"
#include "connection_pool.h"
#include "query_builder.h"

namespace database
{
	/**
	 * @class database_manager
	 * @brief Manages database connections and operations in a singleton
	 * pattern.
	 *
	 * The @c database_manager class provides a high-level interface for
	 * controlling database connections and executing queries. It wraps
	 * a @c database_base instance and exposes methods such as @c connect,
	 * @c disconnect, @c create_query, @c insert_query, etc. This class
	 * also follows the singleton design pattern, allowing only one
	 * instance of the manager to exist at a time.
	 */
	class database_manager
	{
	public:
		/**
		 * @brief Default constructor.
		 *
		 * Initializes the manager with no active database connection.
		 * This constructor is private in many singleton implementations,
		 * but can remain public if the design requires it.
		 */
		database_manager();

		/**
		 * @brief Destructor.
		 *
		 * Cleans up resources and disconnects from the database if an
		 * active connection exists.
		 */
		virtual ~database_manager();

		/**
		 * @brief Sets the database mode (type) for the manager.
		 *
		 * @param database_type An enum value of @c database_types that
		 *                      specifies the target database type (e.g.,
		 *                      PostgreSQL, MySQL, SQLite).
		 * @return @c true on success (e.g., if a corresponding database
		 *         implementation is available), @c false otherwise.
		 */
		bool set_mode(const database_types& database_type);

		/**
		 * @brief Retrieves the current database type used by the manager.
		 *
		 * @return An enum value of @c database_types representing the
		 *         current database mode.
		 */
		database_types database_type(void);

		/**
		 * @brief Establishes a connection to the database using the
		 *        currently set mode.
		 *
		 * @param connect_string A string containing connection parameters
		 *                       such as host, port, username, password,
		 *                       and database name.
		 * @return @c true if the connection was established successfully,
		 *         @c false otherwise.
		 */
		bool connect(const std::string& connect_string);

		/**
		 * @brief Creates or prepares a query using the provided SQL statement.
		 *
		 * @param query_string The SQL query to be prepared or created.
		 * @return @c true if the query was prepared successfully,
		 *         @c false otherwise.
		 *
		 * This method may handle prepared statements in some database
		 * implementations.
		 */
		bool create_query(const std::string& query_string);

		/**
		 * @brief Executes an SQL INSERT statement.
		 *
		 * @param query_string The SQL INSERT statement.
		 * @return The number of rows inserted, or an implementation-specific
		 *         value if row counts are not supported.
		 */
		unsigned int insert_query(const std::string& query_string);

		/**
		 * @brief Executes an SQL UPDATE statement.
		 *
		 * @param query_string The SQL UPDATE statement.
		 * @return The number of rows updated, or an implementation-specific
		 *         value if row counts are not supported.
		 */
		unsigned int update_query(const std::string& query_string);

		/**
		 * @brief Executes an SQL DELETE statement.
		 *
		 * @param query_string The SQL DELETE statement.
		 * @return The number of rows deleted, or an implementation-specific
		 *         value if row counts are not supported.
		 */
		unsigned int delete_query(const std::string& query_string);

		/**
		 * @brief Executes an SQL SELECT statement and returns the results.
		 *
		 * @param query_string The SQL SELECT statement.
		 * @return A database_result containing rows of data as key-value pairs.
		 *         Returns empty vector if query fails or returns no results.
		 */
		database_result select_query(const std::string& query_string);

		/**
		 * @brief Disconnects from the currently active database.
		 *
		 * @return @c true if successfully disconnected, @c false otherwise
		 *         (e.g., no active connection).
		 */
		bool disconnect(void);

		/**
		 * @brief Creates a connection pool for the specified database type.
		 *
		 * @param db_type The database type to create a pool for
		 * @param config Connection pool configuration parameters
		 * @return @c true if the pool was created successfully, @c false otherwise
		 */
		bool create_connection_pool(database_types db_type, const connection_pool_config& config);

		/**
		 * @brief Gets the connection pool for the specified database type.
		 *
		 * @param db_type The database type to get a pool for
		 * @return Shared pointer to the connection pool, nullptr if not found
		 */
		std::shared_ptr<connection_pool_base> get_connection_pool(database_types db_type);

		/**
		 * @brief Gets connection pool statistics for all active pools.
		 *
		 * @return Map of database type to connection statistics
		 */
		std::map<database_types, connection_stats> get_pool_stats() const;

		/**
		 * @brief Creates a query builder for the current database type.
		 *
		 * @return A query builder configured for the current database
		 */
		query_builder create_query_builder();

		/**
		 * @brief Creates a query builder for a specific database type.
		 *
		 * @param db_type The database type to create a builder for
		 * @return A query builder configured for the specified database
		 */
		query_builder create_query_builder(database_types db_type);

	private:
		bool connected_; ///< Indicates whether a database connection is active.
		std::unique_ptr<database_base>
			database_;	 ///< The underlying database interface.

#pragma region singleton
	public:
		/**
		 * @brief Provides access to the single @c database_manager instance.
		 *
		 * This static method returns a reference to the singleton instance,
		 * ensuring that only one instance of @c database_manager exists
		 * throughout the application.
		 *
		 * @return A reference to the singleton @c database_manager instance.
		 */
		static database_manager& handle(void);

	private:
		/**
		 * @brief A unique pointer holding the singleton instance of
		 *        @c database_manager.
		 */
		static std::unique_ptr<database_manager> handle_;

		/**
		 * @brief A flag used to ensure the singleton instance is created
		 *        only once in a thread-safe manner.
		 */
		static std::once_flag once_;
#pragma endregion
	};
} // namespace database