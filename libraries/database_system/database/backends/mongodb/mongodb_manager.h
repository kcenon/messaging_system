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
	 * @class mongodb_manager
	 * @brief Manages MongoDB database operations.
	 *
	 * This class provides an implementation of the @c database_base interface
	 * for MongoDB databases. It defines methods for connecting, document operations,
	 * and disconnecting from a MongoDB database using the MongoDB C++ driver.
	 */
	class mongodb_manager : public database_base
	{
	public:
		/**
		 * @brief Default constructor.
		 */
		mongodb_manager(void);

		/**
		 * @brief Destructor.
		 */
		virtual ~mongodb_manager(void);

		/**
		 * @brief Returns the specific type of the database.
		 *
		 * @return An enum value of type @c database_types indicating that
		 *         this is a MongoDB database.
		 */
		database_types database_type(void) override;

		/**
		 * @brief Establishes a connection to a MongoDB database using
		 *        the provided connection URI.
		 *
		 * @param connect_string A MongoDB connection URI string
		 *                       Format: "mongodb://username:password@host:port/database"
		 * @return @c true if the connection is successfully established,
		 *         @c false otherwise.
		 */
		bool connect(const std::string& connect_string) override;

		/**
		 * @brief Creates a query (e.g., creates collection or index) using
		 *        the provided command string.
		 *
		 * @param query_string The MongoDB command to create or execute.
		 * @return @c true if the command is successfully executed,
		 *         @c false otherwise.
		 */
		bool create_query(const std::string& query_string) override;

		/**
		 * @brief Executes an INSERT operation on the connected MongoDB database.
		 *        For MongoDB, this translates to inserting a document.
		 *
		 * @param query_string JSON document to insert (format: "collection_name:document_json")
		 * @return The number of documents inserted, or 0 if failed.
		 */
		unsigned int insert_query(const std::string& query_string) override;

		/**
		 * @brief Executes an UPDATE operation on the connected MongoDB database.
		 *        For MongoDB, this translates to updating documents.
		 *
		 * @param query_string Update operation (format: "collection_name:filter_json:update_json")
		 * @return The number of documents updated, or 0 if failed.
		 */
		unsigned int update_query(const std::string& query_string) override;

		/**
		 * @brief Executes a DELETE operation on the connected MongoDB database.
		 *        For MongoDB, this translates to deleting documents.
		 *
		 * @param query_string Delete operation (format: "collection_name:filter_json")
		 * @return The number of documents deleted, or 0 if failed.
		 */
		unsigned int delete_query(const std::string& query_string) override;

		/**
		 * @brief Executes a FIND operation on the connected MongoDB database
		 *        and returns the resulting documents.
		 *
		 * @param query_string Find operation (format: "collection_name:filter_json")
		 * @return A database_result containing documents as key-value pairs.
		 */
		database_result select_query(const std::string& query_string) override;

		/**
		 * @brief Executes a general MongoDB command (DDL, DML, admin operations).
		 *
		 * @param query_string The MongoDB command string to execute.
		 * @return @c true if the command executed successfully,
		 *         @c false otherwise.
		 */
		bool execute_query(const std::string& query_string) override;

		/**
		 * @brief Closes the connection to the MongoDB database.
		 *
		 * @return @c true if the disconnection is successful,
		 *         @c false otherwise.
		 */
		bool disconnect(void) override;

		// MongoDB-specific document operations

		/**
		 * @brief Inserts a document into the specified collection.
		 *
		 * @param collection_name Name of the collection
		 * @param document_json JSON string representation of the document
		 * @return @c true if insertion successful, @c false otherwise
		 */
		bool insert_document(const std::string& collection_name, const std::string& document_json);

		/**
		 * @brief Finds documents in the specified collection matching the query.
		 *
		 * @param collection_name Name of the collection
		 * @param query_json JSON string representation of the query filter
		 * @return database_result containing matching documents
		 */
		database_result find_documents(const std::string& collection_name, const std::string& query_json);

		/**
		 * @brief Updates documents in the specified collection.
		 *
		 * @param collection_name Name of the collection
		 * @param filter_json JSON string representation of the filter
		 * @param update_json JSON string representation of the update operation
		 * @return Number of documents updated
		 */
		unsigned int update_documents(const std::string& collection_name,
									  const std::string& filter_json,
									  const std::string& update_json);

		/**
		 * @brief Deletes documents from the specified collection.
		 *
		 * @param collection_name Name of the collection
		 * @param filter_json JSON string representation of the filter
		 * @return Number of documents deleted
		 */
		unsigned int delete_documents(const std::string& collection_name, const std::string& filter_json);

	private:
		/**
		 * @brief Parses MongoDB connection string format.
		 *
		 * @param connect_string MongoDB URI
		 * @param host Output parameter for database host
		 * @param port Output parameter for database port
		 * @param database Output parameter for database name
		 * @param username Output parameter for username
		 * @param password Output parameter for password
		 * @return @c true if parsing successful, @c false otherwise
		 */
		bool parse_connection_string(const std::string& connect_string,
									 std::string& host, int& port,
									 std::string& database, std::string& username,
									 std::string& password);

		/**
		 * @brief Converts BSON document to database_value.
		 *
		 * @param bson_doc BSON document pointer
		 * @return database_value representation of the document
		 */
		database_value bson_to_database_value(void* bson_doc);

		/**
		 * @brief Converts JSON string to BSON document.
		 *
		 * @param json_string JSON string to convert
		 * @return Pointer to BSON document, nullptr on failure
		 */
		void* json_to_bson(const std::string& json_string);

		/**
		 * @brief Parses query string into components.
		 *
		 * @param query_string Input query string
		 * @param collection Output collection name
		 * @param filter Output filter JSON
		 * @param update Output update JSON (optional)
		 * @return @c true if parsing successful
		 */
		bool parse_query_string(const std::string& query_string,
								std::string& collection,
								std::string& filter,
								std::string& update);

	private:
		void* client_;          ///< Pointer to MongoDB client
		void* database_;        ///< Pointer to MongoDB database
		std::string db_name_;   ///< Database name
		std::mutex mongo_mutex_; ///< Mutex for thread safety
	};
} // namespace database