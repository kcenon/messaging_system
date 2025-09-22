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

#include "redis_manager.h"

#ifdef USE_REDIS
#include <hiredis/hiredis.h>
#endif

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <regex>

namespace database
{
	redis_manager::redis_manager(void) : context_(nullptr), host_("localhost"), port_(6379), database_(0) {}

	redis_manager::~redis_manager(void)
	{
		disconnect();
	}

	database_types redis_manager::database_type(void)
	{
		return database_types::redis;
	}

	bool redis_manager::connect(const std::string& connect_string)
	{
#ifdef USE_REDIS
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			// Parse connection string
			std::string password;
			if (!parse_connection_string(connect_string, host_, port_, password, database_)) {
				std::cerr << "Redis connection string parsing failed" << std::endl;
				return false;
			}

			// Create Redis connection
			redisContext* ctx = redisConnect(host_.c_str(), port_);
			if (ctx == nullptr || ctx->err) {
				if (ctx) {
					std::cerr << "Redis connection error: " << ctx->errstr << std::endl;
					redisFree(ctx);
				} else {
					std::cerr << "Redis connection allocation error" << std::endl;
				}
				return false;
			}

			context_ = ctx;

			// Authenticate if password provided
			if (!password.empty()) {
				redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "AUTH %s", password.c_str()));
				if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
					std::cerr << "Redis authentication failed" << std::endl;
					if (reply) freeReplyObject(reply);
					redisFree(ctx);
					context_ = nullptr;
					return false;
				}
				freeReplyObject(reply);
			}

			// Select database if specified
			if (database_ > 0) {
				redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "SELECT %d", database_));
				if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
					std::cerr << "Redis database selection failed" << std::endl;
					if (reply) freeReplyObject(reply);
					redisFree(ctx);
					context_ = nullptr;
					return false;
				}
				freeReplyObject(reply);
			}

			// Test connection with PING
			redisReply* ping_reply = static_cast<redisReply*>(redisCommand(ctx, "PING"));
			if (ping_reply == nullptr || ping_reply->type == REDIS_REPLY_ERROR) {
				std::cerr << "Redis PING failed" << std::endl;
				if (ping_reply) freeReplyObject(ping_reply);
				redisFree(ctx);
				context_ = nullptr;
				return false;
			}
			freeReplyObject(ping_reply);

			return true;
		} catch (const std::exception& e) {
			std::cerr << "Redis connection error: " << e.what() << std::endl;
		}
#else
		std::cerr << "Redis support not compiled. Connection: " << connect_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	bool redis_manager::create_query(const std::string& query_string)
	{
#ifdef USE_REDIS
		if (!context_) return false;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "%s", query_string.c_str()));

			if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
				if (reply) {
					std::cerr << "Redis command error: " << reply->str << std::endl;
					freeReplyObject(reply);
				}
				return false;
			}

			freeReplyObject(reply);
			return true;
		} catch (const std::exception& e) {
			std::cerr << "Redis create query error: " << e.what() << std::endl;
		}
#else
		std::cerr << "Redis support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	unsigned int redis_manager::insert_query(const std::string& query_string)
	{
#ifdef USE_REDIS
		if (!context_) return 0;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			std::string operation, key, value;
			if (!parse_redis_query(query_string, operation, key, value)) {
				return 0;
			}

			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "SET %s %s", key.c_str(), value.c_str()));

			if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
				if (reply) freeReplyObject(reply);
				return 0;
			}

			bool success = (reply->type == REDIS_REPLY_STATUS &&
							std::string(reply->str) == "OK");
			freeReplyObject(reply);
			return success ? 1 : 0;
		} catch (const std::exception& e) {
			std::cerr << "Redis insert error: " << e.what() << std::endl;
		}
#else
		std::cerr << "Redis support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return 0;
	}

	unsigned int redis_manager::update_query(const std::string& query_string)
	{
		// For Redis, update is the same as insert (SET operation)
		return insert_query(query_string);
	}

	unsigned int redis_manager::delete_query(const std::string& query_string)
	{
#ifdef USE_REDIS
		if (!context_) return 0;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			// For delete, query_string should be just the key
			std::string key = query_string;

			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "DEL %s", key.c_str()));

			if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
				if (reply) freeReplyObject(reply);
				return 0;
			}

			unsigned int deleted_count = (reply->type == REDIS_REPLY_INTEGER) ?
										 static_cast<unsigned int>(reply->integer) : 0;
			freeReplyObject(reply);
			return deleted_count;
		} catch (const std::exception& e) {
			std::cerr << "Redis delete error: " << e.what() << std::endl;
		}
#else
		std::cerr << "Redis support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return 0;
	}

	database_result redis_manager::select_query(const std::string& query_string)
	{
		database_result result;
#ifdef USE_REDIS
		if (!context_) return result;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			// For select, query_string can be a key or a pattern
			std::string key = query_string;

			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "GET %s", key.c_str()));

			if (reply != nullptr && reply->type != REDIS_REPLY_ERROR && reply->type != REDIS_REPLY_NIL) {
				database_row row;
				row["key"] = key;

				if (reply->type == REDIS_REPLY_STRING) {
					row["value"] = std::string(reply->str, reply->len);
				} else if (reply->type == REDIS_REPLY_INTEGER) {
					row["value"] = std::to_string(reply->integer);
				} else {
					row["value"] = std::string(""); // Default empty value
				}

				result.push_back(std::move(row));
			}

			if (reply) freeReplyObject(reply);
		} catch (const std::exception& e) {
			std::cerr << "Redis select error: " << e.what() << std::endl;
		}
#else
		std::cerr << "Redis support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
		// Mock data for testing
		if (!query_string.empty()) {
			database_row mock_row;
			mock_row["key"] = query_string;
			mock_row["value"] = std::string("redis_mock_value");
			result.push_back(mock_row);
		}
#endif
		return result;
	}

	bool redis_manager::execute_query(const std::string& query_string)
	{
#ifdef USE_REDIS
		if (!context_) {
			std::cerr << "No active Redis connection" << std::endl;
			return false;
		}

		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "%s", query_string.c_str()));

			if (reply == nullptr) {
				std::cerr << "Redis command failed: " << ctx->errstr << std::endl;
				return false;
			}

			bool success = true;
			if (reply->type == REDIS_REPLY_ERROR) {
				std::cerr << "Redis execute error: " << reply->str << std::endl;
				success = false;
			}

			freeReplyObject(reply);
			return success;
		} catch (const std::exception& e) {
			std::cerr << "Redis execute error: " << e.what() << std::endl;
		}
#else
		// Mock execution for non-Redis builds
		std::cout << "Redis support not compiled. Mock execute: " << query_string << std::endl;
		return true;
#endif
		return false;
	}

	bool redis_manager::disconnect(void)
	{
#ifdef USE_REDIS
		std::lock_guard<std::mutex> lock(redis_mutex_);
		if (context_) {
			redisFree(static_cast<redisContext*>(context_));
			context_ = nullptr;
			return true;
		}
#endif
		return false;
	}

	bool redis_manager::set_key(const std::string& key, const std::string& value, int ttl_seconds)
	{
		std::string query = key + ":" + value;
		bool result = insert_query(query) > 0;

		if (result && ttl_seconds > 0) {
			return expire_key(key, ttl_seconds);
		}

		return result;
	}

	std::string redis_manager::get_key(const std::string& key)
	{
		auto result = select_query(key);
		if (!result.empty() && result[0].find("value") != result[0].end()) {
			auto value_variant = result[0].at("value");
			if (std::holds_alternative<std::string>(value_variant)) {
				return std::get<std::string>(value_variant);
			}
		}
		return "";
	}

	bool redis_manager::delete_key(const std::string& key)
	{
		return delete_query(key) > 0;
	}

	bool redis_manager::exists_key(const std::string& key)
	{
#ifdef USE_REDIS
		if (!context_) return false;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "EXISTS %s", key.c_str()));

			bool exists = false;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				exists = reply->integer > 0;
			}

			if (reply) freeReplyObject(reply);
			return exists;
		} catch (const std::exception& e) {
			std::cerr << "Redis exists error: " << e.what() << std::endl;
		}
#endif
		return false;
	}

	bool redis_manager::expire_key(const std::string& key, int ttl_seconds)
	{
#ifdef USE_REDIS
		if (!context_) return false;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "EXPIRE %s %d", key.c_str(), ttl_seconds));

			bool success = false;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				success = reply->integer == 1;
			}

			if (reply) freeReplyObject(reply);
			return success;
		} catch (const std::exception& e) {
			std::cerr << "Redis expire error: " << e.what() << std::endl;
		}
#endif
		return false;
	}

	int redis_manager::list_push_left(const std::string& key, const std::string& value)
	{
#ifdef USE_REDIS
		if (!context_) return 0;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "LPUSH %s %s", key.c_str(), value.c_str()));

			int length = 0;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				length = static_cast<int>(reply->integer);
			}

			if (reply) freeReplyObject(reply);
			return length;
		} catch (const std::exception& e) {
			std::cerr << "Redis LPUSH error: " << e.what() << std::endl;
		}
#endif
		return 0;
	}

	int redis_manager::list_push_right(const std::string& key, const std::string& value)
	{
#ifdef USE_REDIS
		if (!context_) return 0;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "RPUSH %s %s", key.c_str(), value.c_str()));

			int length = 0;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				length = static_cast<int>(reply->integer);
			}

			if (reply) freeReplyObject(reply);
			return length;
		} catch (const std::exception& e) {
			std::cerr << "Redis RPUSH error: " << e.what() << std::endl;
		}
#endif
		return 0;
	}

	std::string redis_manager::list_pop_left(const std::string& key)
	{
#ifdef USE_REDIS
		if (!context_) return "";
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "LPOP %s", key.c_str()));

			std::string value;
			if (reply && reply->type == REDIS_REPLY_STRING) {
				value = std::string(reply->str, reply->len);
			}

			if (reply) freeReplyObject(reply);
			return value;
		} catch (const std::exception& e) {
			std::cerr << "Redis LPOP error: " << e.what() << std::endl;
		}
#endif
		return "";
	}

	std::string redis_manager::list_pop_right(const std::string& key)
	{
#ifdef USE_REDIS
		if (!context_) return "";
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "RPOP %s", key.c_str()));

			std::string value;
			if (reply && reply->type == REDIS_REPLY_STRING) {
				value = std::string(reply->str, reply->len);
			}

			if (reply) freeReplyObject(reply);
			return value;
		} catch (const std::exception& e) {
			std::cerr << "Redis RPOP error: " << e.what() << std::endl;
		}
#endif
		return "";
	}

	bool redis_manager::hash_set(const std::string& key, const std::string& field, const std::string& value)
	{
#ifdef USE_REDIS
		if (!context_) return false;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str()));

			bool success = false;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				success = true; // HSET returns 1 if new field, 0 if updated existing field
			}

			if (reply) freeReplyObject(reply);
			return success;
		} catch (const std::exception& e) {
			std::cerr << "Redis HSET error: " << e.what() << std::endl;
		}
#endif
		return false;
	}

	std::string redis_manager::hash_get(const std::string& key, const std::string& field)
	{
#ifdef USE_REDIS
		if (!context_) return "";
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "HGET %s %s", key.c_str(), field.c_str()));

			std::string value;
			if (reply && reply->type == REDIS_REPLY_STRING) {
				value = std::string(reply->str, reply->len);
			}

			if (reply) freeReplyObject(reply);
			return value;
		} catch (const std::exception& e) {
			std::cerr << "Redis HGET error: " << e.what() << std::endl;
		}
#endif
		return "";
	}

	bool redis_manager::set_add(const std::string& key, const std::string& member)
	{
#ifdef USE_REDIS
		if (!context_) return false;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "SADD %s %s", key.c_str(), member.c_str()));

			bool added = false;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				added = reply->integer == 1; // 1 if new element, 0 if already exists
			}

			if (reply) freeReplyObject(reply);
			return added;
		} catch (const std::exception& e) {
			std::cerr << "Redis SADD error: " << e.what() << std::endl;
		}
#endif
		return false;
	}

	bool redis_manager::set_remove(const std::string& key, const std::string& member)
	{
#ifdef USE_REDIS
		if (!context_) return false;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "SREM %s %s", key.c_str(), member.c_str()));

			bool removed = false;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				removed = reply->integer == 1; // 1 if removed, 0 if didn't exist
			}

			if (reply) freeReplyObject(reply);
			return removed;
		} catch (const std::exception& e) {
			std::cerr << "Redis SREM error: " << e.what() << std::endl;
		}
#endif
		return false;
	}

	bool redis_manager::set_is_member(const std::string& key, const std::string& member)
	{
#ifdef USE_REDIS
		if (!context_) return false;
		std::lock_guard<std::mutex> lock(redis_mutex_);
		try {
			redisContext* ctx = static_cast<redisContext*>(context_);
			redisReply* reply = static_cast<redisReply*>(redisCommand(ctx, "SISMEMBER %s %s", key.c_str(), member.c_str()));

			bool is_member = false;
			if (reply && reply->type == REDIS_REPLY_INTEGER) {
				is_member = reply->integer == 1;
			}

			if (reply) freeReplyObject(reply);
			return is_member;
		} catch (const std::exception& e) {
			std::cerr << "Redis SISMEMBER error: " << e.what() << std::endl;
		}
#endif
		return false;
	}

	bool redis_manager::parse_connection_string(const std::string& connect_string,
												 std::string& host, int& port,
												 std::string& password, int& database)
	{
		// Parse Redis connection formats:
		// redis://password@host:port/database
		// redis://host:port/database
		// host:port
		// host

		std::regex redis_uri_regex(R"(redis://(?:([^@]+)@)?([^:]+):?(\d+)?/?(\d+)?)");
		std::smatch matches;

		if (std::regex_match(connect_string, matches, redis_uri_regex)) {
			password = matches[1].str();
			host = matches[2].str();
			port = matches[3].str().empty() ? 6379 : std::stoi(matches[3].str());
			database = matches[4].str().empty() ? 0 : std::stoi(matches[4].str());
			return true;
		}

		// Simple host:port format
		std::regex simple_regex(R"(([^:]+):?(\d+)?)");
		if (std::regex_match(connect_string, matches, simple_regex)) {
			host = matches[1].str();
			port = matches[2].str().empty() ? 6379 : std::stoi(matches[2].str());
			database = 0;
			password = "";
			return true;
		}

		// Default values
		host = "localhost";
		port = 6379;
		database = 0;
		password = "";
		return true;
	}

	void* redis_manager::execute_redis_command(const std::string& command)
	{
#ifdef USE_REDIS
		if (!context_) return nullptr;
		redisContext* ctx = static_cast<redisContext*>(context_);
		return redisCommand(ctx, "%s", command.c_str());
#endif
		return nullptr;
	}

	database_value redis_manager::redis_reply_to_database_value(void* reply)
	{
#ifdef USE_REDIS
		if (!reply) return nullptr;

		redisReply* redis_reply = static_cast<redisReply*>(reply);
		switch (redis_reply->type) {
			case REDIS_REPLY_STRING:
				return std::string(redis_reply->str, redis_reply->len);
			case REDIS_REPLY_INTEGER:
				return static_cast<int64_t>(redis_reply->integer);
			case REDIS_REPLY_NIL:
				return nullptr;
			case REDIS_REPLY_STATUS:
				return std::string(redis_reply->str);
			case REDIS_REPLY_ERROR:
				std::cerr << "Redis error: " << redis_reply->str << std::endl;
				return nullptr;
			default:
				return std::string("UNKNOWN_TYPE");
		}
#endif
		return nullptr;
	}

	bool redis_manager::parse_redis_query(const std::string& query_string,
										   std::string& operation,
										   std::string& key,
										   std::string& value)
	{
		// Parse format: "key:value" for SET operations
		std::istringstream ss(query_string);
		std::string part;
		std::vector<std::string> parts;

		while (std::getline(ss, part, ':')) {
			parts.push_back(part);
		}

		if (parts.size() >= 1) key = parts[0];
		if (parts.size() >= 2) value = parts[1];

		operation = "SET"; // Default operation
		return !key.empty();
	}
} // namespace database