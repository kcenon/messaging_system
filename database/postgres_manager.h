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

#include "database_base.h"

namespace database
{
	/**
	 * @class postgres_manager
	 * @brief Manages PostgreSQL database operations.
	 *
	 * This class provides an implementation of the @c database_base interface
	 * for PostgreSQL databases. It defines methods for connecting, querying,
	 * and disconnecting from a PostgreSQL database.
	 */
	class postgres_manager : public database_base
	{
	public:
		/**
		 * @brief Default constructor.
		 */
		postgres_manager(void);

		/**
		 * @brief Destructor.
		 */
		virtual ~postgres_manager(void);

		/**
		 * @brief Returns the specific type of the database.
		 *
		 * @return An enum value of type @c database_types indicating that
		 *         this is a PostgreSQL database.
		 */
		database_types database_type(void) override;

		/**
		 * @brief Establishes a connection to a PostgreSQL database using
		 *        the provided connection string.
		 *
		 * @param connect_string A string containing connection details
		 *                       (e.g., host, port, database name, user,
		 *                       password).
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
		 *
		 * This method may set up the necessary internal structures
		 * for prepared statements.
		 */
		bool create_query(const std::string& query_string) override;

		/**
		 * @brief Executes an INSERT query on the connected PostgreSQL database.
		 *
		 * @param query_string The SQL INSERT query to be executed.
		 * @return The number of rows inserted, or an implementation-specific
		 *         value if row count is not available.
		 */
		unsigned int insert_query(const std::string& query_string) override;

		/**
		 * @brief Executes an UPDATE query on the connected PostgreSQL database.
		 *
		 * @param query_string The SQL UPDATE query to be executed.
		 * @return The number of rows updated, or an implementation-specific
		 *         value if row count is not available.
		 */
		unsigned int update_query(const std::string& query_string) override;

		/**
		 * @brief Executes a DELETE query on the connected PostgreSQL database.
		 *
		 * @param query_string The SQL DELETE query to be executed.
		 * @return The number of rows deleted, or an implementation-specific
		 *         value if row count is not available.
		 */
		unsigned int delete_query(const std::string& query_string) override;

		/**
		 * @brief Executes a SELECT query on the connected PostgreSQL database
		 *        and returns the resulting data.
		 *
		 * @param query_string The SQL SELECT query to be executed.
		 * @return A shared pointer to a @c container::value_container object
		 *         that contains the query results. May be null or empty if
		 *         no results are returned or if an error occurs.
		 */
		std::unique_ptr<container::value_container> select_query(
			const std::string& query_string) override;

		/**
		 * @brief Closes the connection to the PostgreSQL database.
		 *
		 * @return @c true if the disconnection is successful,
		 *         @c false otherwise (e.g., if no active connection exists).
		 */
		bool disconnect(void) override;

	private:
		/**
		 * @brief Executes a generic PostgreSQL query and returns a pointer
		 *        to the raw result.
		 *
		 * @param query_string The SQL query to be executed.
		 * @return A pointer to the underlying query result structure,
		 *         or @c nullptr if an error occurs.
		 */
		void* query_result(const std::string& query_string);

	private:
		void* connection_; ///< Pointer to the underlying PostgreSQL connection
						   ///< object.
	};
} // namespace database
