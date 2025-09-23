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

#include "postgres_manager.h"

#ifdef USE_POSTGRESQL
#include <pqxx/pqxx>
#elif defined(HAVE_LIBPQ)
#include "libpq-fe.h"
#endif

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace database
{
	postgres_manager::postgres_manager(void) : connection_(nullptr) {}

	postgres_manager::~postgres_manager(void)
	{
		disconnect();
	}

	database_types postgres_manager::database_type(void)
	{
		return database_types::postgres;
	}

	bool postgres_manager::connect(const std::string& connect_string)
	{
#ifdef USE_POSTGRESQL
		try {
			auto conn = std::make_unique<pqxx::connection>(connect_string);
			if (conn->is_open()) {
				connection_ = conn.release();
				return true;
			}
		} catch (const std::exception& e) {
			std::cerr << "PostgreSQL connection error: " << e.what() << std::endl;
		}
#elif defined(HAVE_LIBPQ)
		try {
			connection_ = PQconnectdb(connect_string.c_str());
			if (PQstatus(static_cast<PGconn*>(connection_)) == CONNECTION_OK) {
				return true;
			}
			PQfinish(static_cast<PGconn*>(connection_));
			connection_ = nullptr;
		} catch (const std::exception& e) {
			std::cerr << "PostgreSQL connection error: " << e.what() << std::endl;
		}
#else
		std::cerr << "PostgreSQL support not compiled. Connection: " << connect_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	bool postgres_manager::create_query(const std::string& query_string)
	{
#ifdef USE_POSTGRESQL
		if (!connection_) return false;
		try {
			pqxx::connection* conn = static_cast<pqxx::connection*>(connection_);
			pqxx::work txn(*conn);
			txn.exec(query_string);
			txn.commit();
			return true;
		} catch (const std::exception& e) {
			std::cerr << "Query execution error: " << e.what() << std::endl;
		}
#elif defined(HAVE_LIBPQ)
		if (!connection_) return false;
		try {
			PGresult* result = PQexec(static_cast<PGconn*>(connection_), query_string.c_str());
			ExecStatusType status = PQresultStatus(result);
			bool success = (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);
			PQclear(result);
			return success;
		} catch (const std::exception& e) {
			std::cerr << "Query execution error: " << e.what() << std::endl;
		}
#else
		std::cerr << "PostgreSQL support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	unsigned int postgres_manager::execute_modification_query(const std::string& query_string)
	{
#ifdef USE_POSTGRESQL
		if (!connection_) return 0;
		try {
			pqxx::connection* conn = static_cast<pqxx::connection*>(connection_);
			pqxx::work txn(*conn);
			pqxx::result result = txn.exec(query_string);
			txn.commit();
			return static_cast<unsigned int>(result.affected_rows());
		} catch (const std::exception& e) {
			std::cerr << "Modification query error: " << e.what() << std::endl;
		}
#elif defined(HAVE_LIBPQ)
		if (!connection_) return 0;
		try {
			PGresult* result = PQexec(static_cast<PGconn*>(connection_), query_string.c_str());
			if (PQresultStatus(result) != PGRES_COMMAND_OK) {
				PQclear(result);
				return 0;
			}
			const char* affected_rows = PQcmdTuples(result);
			unsigned int count = 0;
			if (affected_rows && *affected_rows) {
				count = static_cast<unsigned int>(std::stoul(affected_rows));
			}
			PQclear(result);
			return count;
		} catch (const std::exception& e) {
			std::cerr << "Modification query error: " << e.what() << std::endl;
		}
#else
		std::cerr << "PostgreSQL support not compiled. Modification query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return 0;
	}

	unsigned int postgres_manager::insert_query(const std::string& query_string)
	{
		return execute_modification_query(query_string);
	}

	unsigned int postgres_manager::update_query(const std::string& query_string)
	{
		return execute_modification_query(query_string);
	}

	unsigned int postgres_manager::delete_query(const std::string& query_string)
	{
		return execute_modification_query(query_string);
	}

	database_result postgres_manager::select_query(const std::string& query_string)
	{
		database_result result;

#ifdef USE_POSTGRESQL
		if (!connection_) return result;
		try {
			pqxx::connection* conn = static_cast<pqxx::connection*>(connection_);
			pqxx::work txn(*conn);
			pqxx::result pqxx_result = txn.exec(query_string);
			txn.commit();

			for (const auto& row : pqxx_result) {
				database_row db_row;
				for (size_t i = 0; i < row.size(); ++i) {
					std::string column_name = pqxx_result.column_name(i);
					if (row[i].is_null()) {
						db_row[column_name] = nullptr;
					} else {
						// Try to convert to appropriate type
						// Using numeric OID values for compatibility across libpqxx versions
						// These are standard PostgreSQL OID constants:
						// 20 = int8, 21 = int2, 23 = int4
						// 700 = float4, 701 = float8
						// 16 = bool
						pqxx::oid type = row[i].type();
						if (type == 20 || type == 21 || type == 23) { // int8, int2, int4
							db_row[column_name] = row[i].as<int64_t>();
						} else if (type == 700 || type == 701) { // float4, float8
							db_row[column_name] = row[i].as<double>();
						} else if (type == 16) { // bool
							db_row[column_name] = row[i].as<bool>();
						} else {
							db_row[column_name] = row[i].as<std::string>();
						}
					}
				}
				result.push_back(std::move(db_row));
			}
		} catch (const std::exception& e) {
			std::cerr << "Select query error: " << e.what() << std::endl;
		}
#elif defined(HAVE_LIBPQ)
		if (!connection_) return result;
		try {
			PGresult* pg_result = PQexec(static_cast<PGconn*>(connection_), query_string.c_str());
			if (PQresultStatus(pg_result) != PGRES_TUPLES_OK) {
				PQclear(pg_result);
				return result;
			}

			int rows = PQntuples(pg_result);
			int cols = PQnfields(pg_result);

			for (int row = 0; row < rows; ++row) {
				database_row db_row;
				for (int col = 0; col < cols; ++col) {
					std::string column_name = PQfname(pg_result, col);
					if (PQgetisnull(pg_result, row, col)) {
						db_row[column_name] = nullptr;
					} else {
						const char* value = PQgetvalue(pg_result, row, col);
						Oid type = PQftype(pg_result, col);

						// Convert based on PostgreSQL type
						if (type == 20 || type == 21 || type == 23) { // int8, int2, int4
							db_row[column_name] = static_cast<int64_t>(std::stoll(value));
						} else if (type == 700 || type == 701) { // float4, float8
							db_row[column_name] = std::stod(value);
						} else if (type == 16) { // bool
							db_row[column_name] = (*value == 't' || *value == '1');
						} else {
							db_row[column_name] = std::string(value);
						}
					}
				}
				result.push_back(std::move(db_row));
			}
			PQclear(pg_result);
		} catch (const std::exception& e) {
			std::cerr << "Select query error: " << e.what() << std::endl;
		}
#else
		std::cerr << "PostgreSQL support not compiled. Select query: " << query_string.substr(0, 20) << "..." << std::endl;
		// Return empty result with mock data for testing
		if (query_string.find("SELECT") != std::string::npos) {
			database_row mock_row;
			mock_row["id"] = int64_t(1);
			mock_row["name"] = std::string("mock_data");
			mock_row["active"] = true;
			result.push_back(mock_row);
		}
#endif
		return result;
	}

	bool postgres_manager::disconnect(void)
	{
		if (!connection_) return false;

#ifdef USE_POSTGRESQL
		try {
			delete static_cast<pqxx::connection*>(connection_);
			connection_ = nullptr;
			return true;
		} catch (const std::exception& e) {
			std::cerr << "Disconnect error: " << e.what() << std::endl;
		}
#elif defined(HAVE_LIBPQ)
		try {
			PQfinish(static_cast<PGconn*>(connection_));
			connection_ = nullptr;
			return true;
		} catch (const std::exception& e) {
			std::cerr << "Disconnect error: " << e.what() << std::endl;
		}
#else
		// Mock disconnect
		connection_ = nullptr;
		std::cerr << "PostgreSQL support not compiled. Mock disconnect." << std::endl;
		return true;
#endif
		return false;
	}

	bool postgres_manager::execute_query(const std::string& query_string)
	{
#ifdef USE_POSTGRESQL
		try {
			if (!connection_) {
				std::cerr << "No active PostgreSQL connection" << std::endl;
				return false;
			}

			pqxx::work txn{*static_cast<pqxx::connection*>(connection_)};
			txn.exec(query_string);
			txn.commit();
			return true;
		} catch (const std::exception& e) {
			std::cerr << "PostgreSQL execute error: " << e.what() << std::endl;
			return false;
		}
#elif defined(HAVE_LIBPQ)
		if (!connection_) {
			std::cerr << "No active PostgreSQL connection" << std::endl;
			return false;
		}

		PGresult* result = PQexec(static_cast<PGconn*>(connection_), query_string.c_str());
		if (!result) {
			std::cerr << "PostgreSQL execute failed" << std::endl;
			return false;
		}

		ExecStatusType status = PQresultStatus(result);
		bool success = (status == PGRES_COMMAND_OK) || (status == PGRES_TUPLES_OK);

		if (!success) {
			std::cerr << "PostgreSQL execute error: " << PQerrorMessage(static_cast<PGconn*>(connection_)) << std::endl;
		}

		PQclear(result);
		return success;
#else
		// Mock execution
		std::cout << "PostgreSQL support not compiled. Mock execute: " << query_string << std::endl;
		return true;
#endif
	}

	void* postgres_manager::query_result(const std::string& query_string)
	{
#ifdef USE_POSTGRESQL
		// For libpqxx, we don't use this method directly
		return nullptr;
#elif defined(HAVE_LIBPQ)
		if (!connection_) return nullptr;
		return PQexec(static_cast<PGconn*>(connection_), query_string.c_str());
#else
		std::cerr << "PostgreSQL support not compiled. Query result: " << query_string.substr(0, 20) << "..." << std::endl;
		return nullptr;
#endif
	}
}; // namespace database