/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
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

#include <string>
#include <memory>

#include "database_types.h"
#include "container/core/container.h"

namespace database
{
	/**
	 * @class database_base
	 * @brief Abstract base class defining common database operations.
	 *
	 * This class serves as an interface for database operations such as
	 * connecting, querying, and disconnecting. It provides pure virtual
	 * methods that derived classes must implement for specific database
	 * systems.
	 */
	class database_base
	{
	public:
		/**
		 * @brief Default constructor.
		 */
		database_base(void) {}

		/**
		 * @brief Virtual destructor.
		 */
		virtual ~database_base(void) {}

		/**
		 * @brief Retrieves the type of the database.
		 * @return An enum value of type @c database_types representing
		 *         the specific database type (e.g., MySQL, SQLite, etc.).
		 */
		virtual database_types database_type(void) = 0;

		/**
		 * @brief Establishes a connection to a database using a given
		 *        connection string.
		 *
		 * @param connect_string A string containing the necessary
		 *                       connection details (host, port, user,
		 *                       password, database name, etc.).
		 * @return @c true if the connection is established successfully,
		 *         @c false otherwise.
		 */
		virtual bool connect(const std::string& connect_string) = 0;

		/**
		 * @brief Creates a database query (e.g., prepares a statement).
		 *
		 * @param query_string The SQL query string to be prepared or created.
		 * @return @c true if the query was created (prepared) successfully,
		 *         @c false otherwise.
		 *
		 * This function is intended to handle the preparation of a query
		 * before execution in some database engines (e.g., prepared
		 * statements).
		 */
		virtual bool create_query(const std::string& query_string) = 0;

		/**
		 * @brief Executes an INSERT query on the database.
		 *
		 * @param query_string The SQL INSERT query string.
		 * @return The number of rows inserted, typically returned by the
		 *         database engine. If the database does not support
		 *         row counts, this may be an implementation-specific value.
		 */
		virtual unsigned int insert_query(const std::string& query_string) = 0;

		/**
		 * @brief Executes an UPDATE query on the database.
		 *
		 * @param query_string The SQL UPDATE query string.
		 * @return The number of rows affected by the update query. If
		 *         the database does not support row counts, this may be
		 *         an implementation-specific value.
		 */
		virtual unsigned int update_query(const std::string& query_string) = 0;

		/**
		 * @brief Executes a DELETE query on the database.
		 *
		 * @param query_string The SQL DELETE query string.
		 * @return The number of rows deleted by the query. If the database
		 *         does not support row counts, this may be an
		 *         implementation-specific value.
		 */
		virtual unsigned int delete_query(const std::string& query_string) = 0;

		/**
		 * @brief Executes a SELECT query on the database and retrieves the
		 * results.
		 *
		 * @param query_string The SQL SELECT query string.
		 * @return A shared pointer to a @c container_module::value_container object
		 *         that holds the result set of the query. If the query
		 *         fails or returns no results, the behavior of this
		 *         container is implementation-specific (it may be empty
		 *         or null).
		 */
		virtual std::unique_ptr<container_module::value_container> select_query(
			const std::string& query_string)
			= 0;

		/**
		 * @brief Terminates the current database connection.
		 *
		 * @return @c true if the disconnection is successful,
		 *         @c false otherwise (e.g., if no active connection
		 *         exists).
		 */
		virtual bool disconnect(void) = 0;
	};
} // namespace database
