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

namespace database
{
	/**
	 * @class mysql_manager
	 * @brief Manages MySQL database operations.
	 *
	 * This class provides an implementation of the @c database_base interface
	 * for MySQL databases. It defines methods for connecting, querying,
	 * and disconnecting from a MySQL database using the MySQL C API.
	 */
	class mysql_manager : public database_base
	{
	public:
		/**
		 * @brief Default constructor.
		 */
		mysql_manager(void);

		/**
		 * @brief Destructor.
		 */
		virtual ~mysql_manager(void);

		/**
		 * @brief Returns the specific type of the database.
		 *
		 * @return An enum value of type @c database_types indicating that
		 *         this is a MySQL database.
		 */
		database_types database_type(void) override;

		/**
		 * @brief Establishes a connection to a MySQL database using
		 *        the provided connection string.
		 *
		 * @param connect_string A string containing connection details
		 *                       Format: "host=localhost;port=3306;database=test;user=root;password=pass"
		 * @return @c true if the connection is successfully established,
		 *         @c false otherwise.
		 */
		bool connect(const std::string& connect_string) override;

		/**
		 * @brief Creates a query (e.g., prepares a statement) using
		 *        the provided SQL query string.
		 *
		 * @param query_string The SQL query to create or prepare.
		 * @return @c true if the query is successfully created,
		 *         @c false otherwise.
		 */
		bool create_query(const std::string& query_string) override;

		/**
		 * @brief Executes an INSERT query on the connected MySQL database.
		 *
		 * @param query_string The SQL INSERT query to be executed.
		 * @return The number of rows inserted, or 0 if failed.
		 */
		unsigned int insert_query(const std::string& query_string) override;

		/**
		 * @brief Executes an UPDATE query on the connected MySQL database.
		 *
		 * @param query_string The SQL UPDATE query to be executed.
		 * @return The number of rows updated, or 0 if failed.
		 */
		unsigned int update_query(const std::string& query_string) override;

		/**
		 * @brief Executes a DELETE query on the connected MySQL database.
		 *
		 * @param query_string The SQL DELETE query to be executed.
		 * @return The number of rows deleted, or 0 if failed.
		 */
		unsigned int delete_query(const std::string& query_string) override;

		/**
		 * @brief Executes a SELECT query on the connected MySQL database
		 *        and returns the resulting data.
		 *
		 * @param query_string The SQL SELECT query to be executed.
		 * @return A database_result containing rows of data as key-value pairs.
		 */
		database_result select_query(const std::string& query_string) override;

		/**
		 * @brief Executes a general SQL query (DDL, DML) on MySQL.
		 *
		 * @param query_string The SQL query string to execute.
		 * @return @c true if the query executed successfully,
		 *         @c false otherwise.
		 */
		bool execute_query(const std::string& query_string) override;

		/**
		 * @brief Closes the connection to the MySQL database.
		 *
		 * @return @c true if the disconnection is successful,
		 *         @c false otherwise.
		 */
		bool disconnect(void) override;

	private:
		/**
		 * @brief Executes a generic MySQL query and returns a pointer
		 *        to the raw result.
		 *
		 * @param query_string The SQL query to be executed.
		 * @return A pointer to the underlying MySQL query result structure,
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
		 * @brief Parses MySQL connection string into individual components.
		 *
		 * @param connect_string Connection string in format "host=value;port=value;..."
		 * @param host Output parameter for database host
		 * @param port Output parameter for database port
		 * @param database Output parameter for database name
		 * @param user Output parameter for username
		 * @param password Output parameter for password
		 * @return @c true if parsing successful, @c false otherwise
		 */
		bool parse_connection_string(const std::string& connect_string,
									 std::string& host, unsigned int& port,
									 std::string& database, std::string& user,
									 std::string& password);

	private:
		void* connection_; ///< Pointer to the underlying MySQL connection object (MYSQL*)
	};
} // namespace database