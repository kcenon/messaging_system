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
	 * @class redis_manager
	 * @brief Manages Redis database operations.
	 *
	 * This class provides an implementation of the @c database_base interface
	 * for Redis databases. It defines methods for connecting, key-value operations,
	 * and disconnecting from a Redis database using the hiredis library.
	 */
	class redis_manager : public database_base
	{
	public:
		/**
		 * @brief Default constructor.
		 */
		redis_manager(void);

		/**
		 * @brief Destructor.
		 */
		virtual ~redis_manager(void);

		/**
		 * @brief Returns the specific type of the database.
		 *
		 * @return An enum value of type @c database_types indicating that
		 *         this is a Redis database.
		 */
		database_types database_type(void) override;

		/**
		 * @brief Establishes a connection to a Redis database using
		 *        the provided connection string.
		 *
		 * @param connect_string Redis connection string
		 *                       Format: "redis://host:port" or "host:port"
		 * @return @c true if the connection is successfully established,
		 *         @c false otherwise.
		 */
		bool connect(const std::string& connect_string) override;

		/**
		 * @brief Creates/configures Redis settings using the provided command.
		 *
		 * @param query_string Redis configuration command
		 * @return @c true if the command is successfully executed,
		 *         @c false otherwise.
		 */
		bool create_query(const std::string& query_string) override;

		/**
		 * @brief Executes a SET operation on the connected Redis database.
		 *        For Redis, this translates to setting key-value pairs.
		 *
		 * @param query_string SET operation (format: "key:value")
		 * @return The number of keys set, or 0 if failed.
		 */
		unsigned int insert_query(const std::string& query_string) override;

		/**
		 * @brief Executes an UPDATE operation on the connected Redis database.
		 *        For Redis, this is equivalent to SET operation.
		 *
		 * @param query_string UPDATE operation (format: "key:value")
		 * @return The number of keys updated, or 0 if failed.
		 */
		unsigned int update_query(const std::string& query_string) override;

		/**
		 * @brief Executes a DELETE operation on the connected Redis database.
		 *        For Redis, this translates to DEL command.
		 *
		 * @param query_string DELETE operation (format: "key")
		 * @return The number of keys deleted, or 0 if failed.
		 */
		unsigned int delete_query(const std::string& query_string) override;

		/**
		 * @brief Executes a GET operation on the connected Redis database
		 *        and returns the resulting data.
		 *
		 * @param query_string GET operation (format: "key" or "pattern")
		 * @return A database_result containing key-value pairs.
		 */
		database_result select_query(const std::string& query_string) override;

		/**
		 * @brief Executes a general Redis command (raw Redis command).
		 *
		 * @param query_string The Redis command string to execute.
		 * @return @c true if the command executed successfully,
		 *         @c false otherwise.
		 */
		bool execute_query(const std::string& query_string) override;

		/**
		 * @brief Closes the connection to the Redis database.
		 *
		 * @return @c true if the disconnection is successful,
		 *         @c false otherwise.
		 */
		bool disconnect(void) override;

		// Redis-specific key-value operations

		/**
		 * @brief Sets a key-value pair in Redis.
		 *
		 * @param key The key to set
		 * @param value The value to store
		 * @param ttl_seconds Optional TTL in seconds (0 for no expiration)
		 * @return @c true if operation successful, @c false otherwise
		 */
		bool set_key(const std::string& key, const std::string& value, int ttl_seconds = 0);

		/**
		 * @brief Gets the value for a key from Redis.
		 *
		 * @param key The key to retrieve
		 * @return The value associated with the key, empty string if not found
		 */
		std::string get_key(const std::string& key);

		/**
		 * @brief Deletes a key from Redis.
		 *
		 * @param key The key to delete
		 * @return @c true if key was deleted, @c false if key didn't exist
		 */
		bool delete_key(const std::string& key);

		/**
		 * @brief Checks if a key exists in Redis.
		 *
		 * @param key The key to check
		 * @return @c true if key exists, @c false otherwise
		 */
		bool exists_key(const std::string& key);

		/**
		 * @brief Sets expiration time for a key.
		 *
		 * @param key The key to set expiration for
		 * @param ttl_seconds TTL in seconds
		 * @return @c true if expiration was set, @c false otherwise
		 */
		bool expire_key(const std::string& key, int ttl_seconds);

		// Redis data structure operations

		/**
		 * @brief Pushes a value to the left of a Redis list.
		 *
		 * @param key The list key
		 * @param value The value to push
		 * @return Length of the list after operation
		 */
		int list_push_left(const std::string& key, const std::string& value);

		/**
		 * @brief Pushes a value to the right of a Redis list.
		 *
		 * @param key The list key
		 * @param value The value to push
		 * @return Length of the list after operation
		 */
		int list_push_right(const std::string& key, const std::string& value);

		/**
		 * @brief Pops a value from the left of a Redis list.
		 *
		 * @param key The list key
		 * @return The popped value, empty string if list is empty
		 */
		std::string list_pop_left(const std::string& key);

		/**
		 * @brief Pops a value from the right of a Redis list.
		 *
		 * @param key The list key
		 * @return The popped value, empty string if list is empty
		 */
		std::string list_pop_right(const std::string& key);

		/**
		 * @brief Sets a field in a Redis hash.
		 *
		 * @param key The hash key
		 * @param field The field name
		 * @param value The field value
		 * @return @c true if field was set, @c false otherwise
		 */
		bool hash_set(const std::string& key, const std::string& field, const std::string& value);

		/**
		 * @brief Gets a field value from a Redis hash.
		 *
		 * @param key The hash key
		 * @param field The field name
		 * @return The field value, empty string if not found
		 */
		std::string hash_get(const std::string& key, const std::string& field);

		/**
		 * @brief Adds a member to a Redis set.
		 *
		 * @param key The set key
		 * @param member The member to add
		 * @return @c true if member was added, @c false if already exists
		 */
		bool set_add(const std::string& key, const std::string& member);

		/**
		 * @brief Removes a member from a Redis set.
		 *
		 * @param key The set key
		 * @param member The member to remove
		 * @return @c true if member was removed, @c false if didn't exist
		 */
		bool set_remove(const std::string& key, const std::string& member);

		/**
		 * @brief Checks if a member exists in a Redis set.
		 *
		 * @param key The set key
		 * @param member The member to check
		 * @return @c true if member exists, @c false otherwise
		 */
		bool set_is_member(const std::string& key, const std::string& member);

	private:
		/**
		 * @brief Parses Redis connection string.
		 *
		 * @param connect_string Redis connection string
		 * @param host Output parameter for Redis host
		 * @param port Output parameter for Redis port
		 * @param password Output parameter for Redis password
		 * @param database Output parameter for Redis database number
		 * @return @c true if parsing successful, @c false otherwise
		 */
		bool parse_connection_string(const std::string& connect_string,
									 std::string& host, int& port,
									 std::string& password, int& database);

		/**
		 * @brief Executes a Redis command and returns the reply.
		 *
		 * @param command The Redis command to execute
		 * @return Pointer to Redis reply, nullptr on failure
		 */
		void* execute_redis_command(const std::string& command);

		/**
		 * @brief Converts Redis reply to database_value.
		 *
		 * @param reply Redis reply pointer
		 * @return database_value representation of the reply
		 */
		database_value redis_reply_to_database_value(void* reply);

		/**
		 * @brief Parses query string for Redis operations.
		 *
		 * @param query_string Input query string
		 * @param operation Output operation type
		 * @param key Output key
		 * @param value Output value
		 * @return @c true if parsing successful
		 */
		bool parse_redis_query(const std::string& query_string,
								std::string& operation,
								std::string& key,
								std::string& value);

	private:
		void* context_;         ///< Pointer to Redis context
		std::mutex redis_mutex_; ///< Mutex for thread safety
		std::string host_;      ///< Redis host
		int port_;              ///< Redis port
		int database_;          ///< Redis database number
	};
} // namespace database