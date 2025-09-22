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
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <vector>

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
		 * @return A shared pointer to a @c container_module::value_container object
		 *         that contains the query results. May be null or empty if
		 *         no results are returned or if an error occurs.
		 */
		std::unique_ptr<container_module::value_container> select_query(
			const std::string& query_string) override;

		/**
		 * @brief Closes the connection to the PostgreSQL database.
		 *
		 * @return @c true if the disconnection is successful,
		 *         @c false otherwise (e.g., if no active connection exists).
		 */
		bool disconnect(void) override;

		// Enhanced database operations

		/**
		 * @brief Begin a database transaction
		 * @return true if transaction started successfully
		 */
		bool begin_transaction();

		/**
		 * @brief Commit the current transaction
		 * @return true if transaction committed successfully
		 */
		bool commit_transaction();

		/**
		 * @brief Rollback the current transaction
		 * @return true if transaction rolled back successfully
		 */
		bool rollback_transaction();

		/**
		 * @brief Check if currently in a transaction
		 * @return true if in transaction
		 */
		bool in_transaction() const;

		/**
		 * @brief Execute batch operations in a single transaction
		 * @param queries Vector of SQL queries to execute
		 * @return Number of successfully executed queries
		 */
		unsigned int execute_batch(const std::vector<std::string>& queries);

		/**
		 * @brief Execute parameterized query with prepared statements
		 * @param query_string SQL query with parameter placeholders
		 * @param parameters Vector of parameter values
		 * @return Query result container
		 */
		std::unique_ptr<container_module::value_container> execute_prepared(
			const std::string& query_string,
			const std::vector<std::string>& parameters);

		/**
		 * @brief Get connection health status
		 * @return Connection health information
		 */
		struct connection_health {
			bool is_connected;
			bool is_transaction_active;
			std::chrono::milliseconds last_query_duration{0};
			size_t total_queries_executed{0};
			size_t failed_queries{0};
			double success_rate() const {
				return total_queries_executed > 0 ?
					static_cast<double>(total_queries_executed - failed_queries) / total_queries_executed : 0.0;
			}
		};
		connection_health get_connection_health() const;

		/**
		 * @brief Test database connection with ping
		 * @return true if connection is alive
		 */
		bool ping();

		/**
		 * @brief Reset connection if it's in a bad state
		 * @return true if reset was successful
		 */
		bool reset_connection();

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

		/**
		 * @brief Common implementation for INSERT, UPDATE, and DELETE queries.
		 * 
		 * @param query_string The SQL query to be executed.
		 * @return The number of affected rows, or 0 if an error occurs.
		 */
		unsigned int execute_modification_query(const std::string& query_string);

	private:
		void* connection_; ///< Pointer to the underlying PostgreSQL connection object.

		// Enhanced state tracking
		std::atomic<bool> transaction_active_{false}; ///< Transaction state
		std::string connection_string_; ///< Stored connection string for reconnection

		// Performance and health monitoring
		mutable std::atomic<size_t> query_count_{0};
		mutable std::atomic<size_t> failed_query_count_{0};
		mutable std::atomic<std::chrono::milliseconds::rep> last_query_duration_ms_{0};
		mutable std::chrono::steady_clock::time_point last_activity_;

		// Prepared statements cache
		std::unordered_map<std::string, void*> prepared_statements_;
		std::mutex prepared_statements_mutex_;

		// Connection management
		std::mutex connection_mutex_;
		std::atomic<bool> connection_validated_{false};
	};
} // namespace database
