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

#include "../../database_base.h"
#include <mutex>

namespace database
{
	/**
	 * @class sqlite_manager
	 * @brief Manages SQLite database operations.
	 *
	 * This class provides an implementation of the @c database_base interface
	 * for SQLite databases. It defines methods for connecting, querying,
	 * and disconnecting from a SQLite database using the SQLite3 C API.
	 */
	class sqlite_manager : public database_base
	{
	public:
		/**
		 * @brief Default constructor.
		 */
		sqlite_manager(void);

		/**
		 * @brief Destructor.
		 */
		virtual ~sqlite_manager(void);

		/**
		 * @brief Returns the specific type of the database.
		 *
		 * @return An enum value of type @c database_types indicating that
		 *         this is a SQLite database.
		 */
		database_types database_type(void) override;

		/**
		 * @brief Establishes a connection to a SQLite database using
		 *        the provided database path.
		 *
		 * @param connect_string A string containing the database path.
		 *                       Examples: "./database.db", ":memory:"
		 * @return @c true if the connection is successfully established,
		 *         @c false otherwise.
		 */
		bool connect(const std::string& connect_string) override;

		/**
		 * @brief Creates a query (e.g., executes DDL) using
		 *        the provided SQL query string.
		 *
		 * @param query_string The SQL query to create or execute.
		 * @return @c true if the query is successfully executed,
		 *         @c false otherwise.
		 */
		bool create_query(const std::string& query_string) override;

		/**
		 * @brief Executes an INSERT query on the connected SQLite database.
		 *
		 * @param query_string The SQL INSERT query to be executed.
		 * @return The number of rows inserted, or 0 if failed.
		 */
		unsigned int insert_query(const std::string& query_string) override;

		/**
		 * @brief Executes an UPDATE query on the connected SQLite database.
		 *
		 * @param query_string The SQL UPDATE query to be executed.
		 * @return The number of rows updated, or 0 if failed.
		 */
		unsigned int update_query(const std::string& query_string) override;

		/**
		 * @brief Executes a DELETE query on the connected SQLite database.
		 *
		 * @param query_string The SQL DELETE query to be executed.
		 * @return The number of rows deleted, or 0 if failed.
		 */
		unsigned int delete_query(const std::string& query_string) override;

		/**
		 * @brief Executes a SELECT query on the connected SQLite database
		 *        and returns the resulting data.
		 *
		 * @param query_string The SQL SELECT query to be executed.
		 * @return A database_result containing rows of data as key-value pairs.
		 */
		database_result select_query(const std::string& query_string) override;

		/**
		 * @brief Executes a general SQL query (DDL, DML) on SQLite.
		 *
		 * @param query_string The SQL query string to execute.
		 * @return @c true if the query executed successfully,
		 *         @c false otherwise.
		 */
		bool execute_query(const std::string& query_string) override;

		/**
		 * @brief Closes the connection to the SQLite database.
		 *
		 * @return @c true if the disconnection is successful,
		 *         @c false otherwise.
		 */
		bool disconnect(void) override;

	private:
		/**
		 * @brief Executes a generic SQLite query and returns a pointer
		 *        to the raw result.
		 *
		 * @param query_string The SQL query to be executed.
		 * @return A pointer to the underlying SQLite statement,
		 *         or @c nullptr if an error occurs.
		 */
		void* query_result(const std::string& query_string);

		/**
		 * @brief Common implementation for INSERT, UPDATE, and DELETE queries.
		 *
		 * @param query_string The SQL query to be executed.
		 * @return The number of affected rows, or 0 if an error occurs.
		 */
		unsigned int execute_modification_query(const std::string& query_string);

		/**
		 * @brief Converts SQLite column type to database_value.
		 *
		 * @param stmt SQLite prepared statement
		 * @param column_index Column index in the result set
		 * @return database_value containing the converted value
		 */
		database_value convert_sqlite_value(void* stmt, int column_index);

	private:
		void* connection_;        ///< Pointer to the underlying SQLite connection (sqlite3*)
		std::mutex sqlite_mutex_; ///< Mutex for thread safety
	};
} // namespace database