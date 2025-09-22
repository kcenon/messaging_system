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

#include "sqlite_manager.h"

#ifdef USE_SQLITE
#include <sqlite3.h>
#endif

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace database
{
	sqlite_manager::sqlite_manager(void) : connection_(nullptr) {}

	sqlite_manager::~sqlite_manager(void)
	{
		disconnect();
	}

	database_types sqlite_manager::database_type(void)
	{
		return database_types::sqlite;
	}

	bool sqlite_manager::connect(const std::string& connect_string)
	{
#ifdef USE_SQLITE
		std::lock_guard<std::mutex> lock(sqlite_mutex_);
		try {
			sqlite3* db = nullptr;

			// Open or create the database
			int result = sqlite3_open(connect_string.c_str(), &db);

			if (result != SQLITE_OK) {
				std::cerr << "SQLite connection failed: " << sqlite3_errmsg(db) << std::endl;
				if (db) {
					sqlite3_close(db);
				}
				return false;
			}

			connection_ = db;

			// Enable foreign key constraints
			if (!create_query("PRAGMA foreign_keys = ON")) {
				std::cerr << "Warning: Failed to enable foreign key constraints" << std::endl;
			}

			return true;
		} catch (const std::exception& e) {
			std::cerr << "SQLite connection error: " << e.what() << std::endl;
		}
#else
		std::cerr << "SQLite support not compiled. Connection: " << connect_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	bool sqlite_manager::create_query(const std::string& query_string)
	{
#ifdef USE_SQLITE
		if (!connection_) return false;
		std::lock_guard<std::mutex> lock(sqlite_mutex_);
		try {
			sqlite3* db = static_cast<sqlite3*>(connection_);
			char* error_msg = nullptr;

			int result = sqlite3_exec(db, query_string.c_str(), nullptr, nullptr, &error_msg);

			if (result != SQLITE_OK) {
				std::cerr << "SQLite query execution error: " << error_msg << std::endl;
				sqlite3_free(error_msg);
				return false;
			}

			return true;
		} catch (const std::exception& e) {
			std::cerr << "Query execution error: " << e.what() << std::endl;
		}
#else
		std::cerr << "SQLite support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	unsigned int sqlite_manager::insert_query(const std::string& query_string)
	{
		return execute_modification_query(query_string);
	}

	unsigned int sqlite_manager::update_query(const std::string& query_string)
	{
		return execute_modification_query(query_string);
	}

	unsigned int sqlite_manager::delete_query(const std::string& query_string)
	{
		return execute_modification_query(query_string);
	}

	database_result sqlite_manager::select_query(const std::string& query_string)
	{
		database_result result;
#ifdef USE_SQLITE
		if (!connection_) return result;
		std::lock_guard<std::mutex> lock(sqlite_mutex_);
		try {
			sqlite3* db = static_cast<sqlite3*>(connection_);
			sqlite3_stmt* stmt = nullptr;

			// Prepare the statement
			int prepare_result = sqlite3_prepare_v2(db, query_string.c_str(), -1, &stmt, nullptr);
			if (prepare_result != SQLITE_OK) {
				std::cerr << "SQLite prepare failed: " << sqlite3_errmsg(db) << std::endl;
				return result;
			}

			// Get column count and names
			int column_count = sqlite3_column_count(stmt);
			std::vector<std::string> column_names;
			for (int i = 0; i < column_count; i++) {
				column_names.push_back(sqlite3_column_name(stmt, i));
			}

			// Execute and fetch results
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				database_row row;

				for (int i = 0; i < column_count; i++) {
					const std::string& column_name = column_names[i];
					row[column_name] = convert_sqlite_value(stmt, i);
				}

				result.push_back(std::move(row));
			}

			// Clean up
			sqlite3_finalize(stmt);

		} catch (const std::exception& e) {
			std::cerr << "Select query error: " << e.what() << std::endl;
		}
#else
		std::cerr << "SQLite support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
		// Mock data for testing
		if (query_string.find("SELECT") != std::string::npos) {
			database_row mock_row;
			mock_row["id"] = int64_t(1);
			mock_row["name"] = std::string("sqlite_mock_data");
			result.push_back(mock_row);
		}
#endif
		return result;
	}

	bool sqlite_manager::disconnect(void)
	{
#ifdef USE_SQLITE
		std::lock_guard<std::mutex> lock(sqlite_mutex_);
		if (connection_) {
			sqlite3* db = static_cast<sqlite3*>(connection_);
			int result = sqlite3_close(db);
			connection_ = nullptr;
			return result == SQLITE_OK;
		}
#endif
		return false;
	}

	bool sqlite_manager::execute_query(const std::string& query_string)
	{
#ifdef USE_SQLITE
		if (!connection_) {
			std::cerr << "No active SQLite connection" << std::endl;
			return false;
		}

		std::lock_guard<std::mutex> lock(sqlite_mutex_);
		sqlite3* db = static_cast<sqlite3*>(connection_);

		char* error_msg = nullptr;
		int result = sqlite3_exec(db, query_string.c_str(), nullptr, nullptr, &error_msg);

		if (result != SQLITE_OK) {
			std::cerr << "SQLite execute error: " << (error_msg ? error_msg : "Unknown error") << std::endl;
			if (error_msg) {
				sqlite3_free(error_msg);
			}
			return false;
		}

		return true;
#else
		// Mock execution
		std::cout << "SQLite support not compiled. Mock execute: " << query_string << std::endl;
		return true;
#endif
	}

	void* sqlite_manager::query_result(const std::string& query_string)
	{
#ifdef USE_SQLITE
		if (!connection_) return nullptr;
		std::lock_guard<std::mutex> lock(sqlite_mutex_);
		try {
			sqlite3* db = static_cast<sqlite3*>(connection_);
			sqlite3_stmt* stmt = nullptr;

			int result = sqlite3_prepare_v2(db, query_string.c_str(), -1, &stmt, nullptr);
			if (result != SQLITE_OK) {
				return nullptr;
			}

			return stmt;
		} catch (const std::exception& e) {
			std::cerr << "Query result error: " << e.what() << std::endl;
		}
#endif
		return nullptr;
	}

	unsigned int sqlite_manager::execute_modification_query(const std::string& query_string)
	{
#ifdef USE_SQLITE
		if (!connection_) return 0;
		std::lock_guard<std::mutex> lock(sqlite_mutex_);
		try {
			sqlite3* db = static_cast<sqlite3*>(connection_);
			char* error_msg = nullptr;

			int result = sqlite3_exec(db, query_string.c_str(), nullptr, nullptr, &error_msg);

			if (result != SQLITE_OK) {
				std::cerr << "SQLite modification query failed: " << error_msg << std::endl;
				sqlite3_free(error_msg);
				return 0;
			}

			// Return the number of affected rows
			return static_cast<unsigned int>(sqlite3_changes(db));
		} catch (const std::exception& e) {
			std::cerr << "Modification query error: " << e.what() << std::endl;
		}
#else
		std::cerr << "SQLite support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return 0;
	}

	database_value sqlite_manager::convert_sqlite_value(void* stmt, int column_index)
	{
#ifdef USE_SQLITE
		sqlite3_stmt* sqlite_stmt = static_cast<sqlite3_stmt*>(stmt);

		int sqlite_type = sqlite3_column_type(sqlite_stmt, column_index);

		switch (sqlite_type) {
			case SQLITE_INTEGER:
				return static_cast<int64_t>(sqlite3_column_int64(sqlite_stmt, column_index));

			case SQLITE_FLOAT:
				return sqlite3_column_double(sqlite_stmt, column_index);

			case SQLITE_TEXT:
				{
					const char* text = reinterpret_cast<const char*>(sqlite3_column_text(sqlite_stmt, column_index));
					return std::string(text ? text : "");
				}

			case SQLITE_BLOB:
				{
					// For BLOB data, convert to string representation
					const void* blob = sqlite3_column_blob(sqlite_stmt, column_index);
					int blob_size = sqlite3_column_bytes(sqlite_stmt, column_index);
					if (blob && blob_size > 0) {
						const char* blob_chars = static_cast<const char*>(blob);
						return std::string(blob_chars, blob_size);
					}
					return std::string();
				}

			case SQLITE_NULL:
			default:
				return nullptr;
		}
#endif
		return nullptr;
	}
} // namespace database