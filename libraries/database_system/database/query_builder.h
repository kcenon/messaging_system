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

#include "database_types.h"
#include "database_base.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <initializer_list>

namespace database
{
	/**
	 * @enum join_type
	 * @brief Types of SQL joins.
	 */
	enum class join_type
	{
		inner,
		left,
		right,
		full_outer,
		cross
	};

	/**
	 * @enum sort_order
	 * @brief Sort order for ORDER BY clauses.
	 */
	enum class sort_order
	{
		asc,
		desc
	};

	/**
	 * @class query_condition
	 * @brief Represents a WHERE condition in a query.
	 */
	class query_condition
	{
	public:
		query_condition(const std::string& field, const std::string& op, const database_value& value);
		query_condition(const std::string& raw_condition);

		std::string to_sql() const;
		std::string to_mongodb() const;
		std::string to_redis() const;

		// Logical operators
		query_condition operator&&(const query_condition& other) const;
		query_condition operator||(const query_condition& other) const;

	private:
		std::string field_;
		std::string operator_;
		database_value value_;
		std::string raw_condition_;
		std::vector<query_condition> sub_conditions_;
		std::string logical_operator_;
	};

	/**
	 * @class sql_query_builder
	 * @brief Builder for SQL queries (PostgreSQL, MySQL, SQLite).
	 */
	class sql_query_builder
	{
	public:
		sql_query_builder();
		~sql_query_builder() = default;

		// SELECT operations
		sql_query_builder& select(const std::vector<std::string>& columns);
		sql_query_builder& select(const std::string& column);
		sql_query_builder& select_raw(const std::string& raw_select);
		sql_query_builder& from(const std::string& table);

		// WHERE conditions
		sql_query_builder& where(const std::string& field, const std::string& op, const database_value& value);
		sql_query_builder& where(const query_condition& condition);
		sql_query_builder& where_raw(const std::string& raw_where);
		sql_query_builder& or_where(const std::string& field, const std::string& op, const database_value& value);

		// JOIN operations
		sql_query_builder& join(const std::string& table, const std::string& condition, join_type type = join_type::inner);
		sql_query_builder& left_join(const std::string& table, const std::string& condition);
		sql_query_builder& right_join(const std::string& table, const std::string& condition);

		// GROUP BY and HAVING
		sql_query_builder& group_by(const std::vector<std::string>& columns);
		sql_query_builder& group_by(const std::string& column);
		sql_query_builder& having(const std::string& condition);

		// ORDER BY
		sql_query_builder& order_by(const std::string& column, sort_order order = sort_order::asc);
		sql_query_builder& order_by_raw(const std::string& raw_order);

		// LIMIT and OFFSET
		sql_query_builder& limit(size_t count);
		sql_query_builder& offset(size_t count);

		// INSERT operations
		sql_query_builder& insert_into(const std::string& table);
		sql_query_builder& values(const std::map<std::string, database_value>& data);
		sql_query_builder& values(const std::vector<std::map<std::string, database_value>>& rows);

		// UPDATE operations
		sql_query_builder& update(const std::string& table);
		sql_query_builder& set(const std::string& field, const database_value& value);
		sql_query_builder& set(const std::map<std::string, database_value>& data);

		// DELETE operations
		sql_query_builder& delete_from(const std::string& table);

		// Build final query
		std::string build() const;
		std::string build_for_database(database_types db_type) const;

		// Reset builder
		void reset();

	private:
		enum class query_type { none, select, insert, update, delete_query };

		query_type type_;
		std::vector<std::string> select_columns_;
		std::string from_table_;
		std::vector<query_condition> where_conditions_;
		std::vector<std::string> joins_;
		std::vector<std::string> group_by_columns_;
		std::string having_clause_;
		std::vector<std::string> order_by_clauses_;
		size_t limit_count_;
		size_t offset_count_;

		// For INSERT/UPDATE
		std::string target_table_;
		std::map<std::string, database_value> set_data_;
		std::vector<std::map<std::string, database_value>> insert_rows_;

		std::string escape_identifier(const std::string& identifier, database_types db_type) const;
		std::string format_value(const database_value& value, database_types db_type) const;
		std::string join_type_to_string(join_type type) const;
	};

	/**
	 * @class mongodb_query_builder
	 * @brief Builder for MongoDB queries.
	 */
	class mongodb_query_builder
	{
	public:
		mongodb_query_builder();
		~mongodb_query_builder() = default;

		// Collection operations
		mongodb_query_builder& collection(const std::string& name);

		// Find operations
		mongodb_query_builder& find(const std::map<std::string, database_value>& filter = {});
		mongodb_query_builder& find_one(const std::map<std::string, database_value>& filter = {});

		// Projection
		mongodb_query_builder& project(const std::vector<std::string>& fields);
		mongodb_query_builder& exclude(const std::vector<std::string>& fields);

		// Sorting
		mongodb_query_builder& sort(const std::map<std::string, int>& sort_spec);
		mongodb_query_builder& sort(const std::string& field, int direction = 1);

		// Limit and Skip
		mongodb_query_builder& limit(size_t count);
		mongodb_query_builder& skip(size_t count);

		// Insert operations
		mongodb_query_builder& insert_one(const std::map<std::string, database_value>& document);
		mongodb_query_builder& insert_many(const std::vector<std::map<std::string, database_value>>& documents);

		// Update operations
		mongodb_query_builder& update_one(const std::map<std::string, database_value>& filter,
										   const std::map<std::string, database_value>& update);
		mongodb_query_builder& update_many(const std::map<std::string, database_value>& filter,
											const std::map<std::string, database_value>& update);

		// Delete operations
		mongodb_query_builder& delete_one(const std::map<std::string, database_value>& filter);
		mongodb_query_builder& delete_many(const std::map<std::string, database_value>& filter);

		// Aggregation pipeline
		mongodb_query_builder& match(const std::map<std::string, database_value>& conditions);
		mongodb_query_builder& group(const std::map<std::string, database_value>& group_spec);
		mongodb_query_builder& unwind(const std::string& field);

		// Build final query
		std::string build() const;
		std::string build_json() const;

		// Reset builder
		void reset();

	private:
		enum class operation_type { none, find, insert, update, delete_op, aggregate };

		operation_type type_;
		std::string collection_name_;
		std::map<std::string, database_value> filter_;
		std::map<std::string, database_value> projection_;
		std::map<std::string, int> sort_spec_;
		size_t limit_count_;
		size_t skip_count_;

		// For operations
		std::map<std::string, database_value> document_;
		std::vector<std::map<std::string, database_value>> documents_;
		std::map<std::string, database_value> update_spec_;

		// For aggregation
		std::vector<std::map<std::string, database_value>> pipeline_;

		std::string to_json(const std::map<std::string, database_value>& data) const;
		std::string value_to_json(const database_value& value) const;
	};

	/**
	 * @class redis_query_builder
	 * @brief Builder for Redis commands.
	 */
	class redis_query_builder
	{
	public:
		redis_query_builder();
		~redis_query_builder() = default;

		// String operations
		redis_query_builder& set(const std::string& key, const std::string& value);
		redis_query_builder& get(const std::string& key);
		redis_query_builder& del(const std::string& key);
		redis_query_builder& exists(const std::string& key);

		// Hash operations
		redis_query_builder& hset(const std::string& key, const std::string& field, const std::string& value);
		redis_query_builder& hget(const std::string& key, const std::string& field);
		redis_query_builder& hdel(const std::string& key, const std::string& field);
		redis_query_builder& hgetall(const std::string& key);

		// List operations
		redis_query_builder& lpush(const std::string& key, const std::string& value);
		redis_query_builder& rpush(const std::string& key, const std::string& value);
		redis_query_builder& lpop(const std::string& key);
		redis_query_builder& rpop(const std::string& key);
		redis_query_builder& lrange(const std::string& key, int start, int stop);

		// Set operations
		redis_query_builder& sadd(const std::string& key, const std::string& member);
		redis_query_builder& srem(const std::string& key, const std::string& member);
		redis_query_builder& sismember(const std::string& key, const std::string& member);
		redis_query_builder& smembers(const std::string& key);

		// Expiration
		redis_query_builder& expire(const std::string& key, int seconds);
		redis_query_builder& ttl(const std::string& key);

		// Build command
		std::string build() const;
		std::vector<std::string> build_args() const;

		// Reset builder
		void reset();

	private:
		std::string command_;
		std::vector<std::string> args_;
	};

	/**
	 * @class query_builder
	 * @brief Universal query builder that adapts to different database types.
	 */
	class query_builder
	{
	public:
		explicit query_builder(database_types db_type = database_types::none);
		~query_builder() = default;

		// Set database type
		query_builder& for_database(database_types db_type);

		// SQL-style interface (works for PostgreSQL, MySQL, SQLite)
		query_builder& select(const std::vector<std::string>& columns);
		query_builder& from(const std::string& table);
		query_builder& where(const std::string& field, const std::string& op, const database_value& value);
		query_builder& join(const std::string& table, const std::string& condition);
		query_builder& order_by(const std::string& column, sort_order order = sort_order::asc);
		query_builder& limit(size_t count);

		// NoSQL-style interface
		query_builder& collection(const std::string& name); // MongoDB
		query_builder& key(const std::string& key); // Redis

		// Universal operations
		query_builder& insert(const std::map<std::string, database_value>& data);
		query_builder& update(const std::map<std::string, database_value>& data);
		query_builder& remove(); // DELETE/DROP

		// Build and execute
		std::string build() const;
		database_result execute(database_base* db) const;

		// Reset builder
		void reset();

	private:
		database_types db_type_;
		std::unique_ptr<sql_query_builder> sql_builder_;
		std::unique_ptr<mongodb_query_builder> mongo_builder_;
		std::unique_ptr<redis_query_builder> redis_builder_;

		void ensure_builder();
	};

} // namespace database