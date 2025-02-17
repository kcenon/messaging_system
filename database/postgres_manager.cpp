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

#include "postgres_manager.h"

#include "libpq-fe.h"

#include "convert_string.h"

namespace database
{
	using namespace utility_module;

	postgres_manager::postgres_manager(void) : connection_(nullptr) {}

	postgres_manager::~postgres_manager(void) {}

	database_types postgres_manager::database_type(void)
	{
		return database_types::postgres;
	}

	bool postgres_manager::connect(const std::string& connect_string)
	{
		auto [converted_string, error_message]
			= convert_string::utf8_to_system(connect_string);
		if (error_message.has_value())
		{
			return false;
		}

		auto converted_connect_string = converted_string.value();

		connection_ = PQconnectdb(converted_connect_string.c_str());
		if (PQstatus((PGconn*)connection_) != CONNECTION_OK)
		{
			PQfinish((PGconn*)connection_);
			connection_ = nullptr;

			return false;
		}

		return true;
	}

	bool postgres_manager::create_query(const std::string& query_string)
	{
		PGresult* result = (PGresult*)query_result(query_string);
		if (PQresultStatus(result) != PGRES_TUPLES_OK)
		{
			PQclear(result);
			result = nullptr;

			PQfinish((PGconn*)connection_);
			connection_ = nullptr;

			return false;
		}

		PQclear(result);
		result = nullptr;

		return true;
	}

	unsigned int postgres_manager::insert_query(const std::string& query_string)
	{
		PGresult* result = (PGresult*)query_result(query_string);
		if (PQresultStatus(result) != PGRES_TUPLES_OK)
		{
			PQclear(result);
			result = nullptr;

			PQfinish((PGconn*)connection_);
			connection_ = nullptr;

			return 0;
		}

		unsigned int result_count = atoi(PQcmdTuples(result));

		PQclear(result);
		result = nullptr;

		return result_count;
	}

	unsigned int postgres_manager::update_query(const std::string& query_string)
	{
		PGresult* result = (PGresult*)query_result(query_string);
		if (PQresultStatus(result) != PGRES_TUPLES_OK)
		{
			PQclear(result);
			result = nullptr;

			PQfinish((PGconn*)connection_);
			connection_ = nullptr;

			return 0;
		}

		unsigned int result_count = atoi(PQcmdTuples(result));

		PQclear(result);
		result = nullptr;

		return result_count;
	}

	unsigned int postgres_manager::delete_query(const std::string& query_string)
	{
		PGresult* result = (PGresult*)query_result(query_string);
		if (PQresultStatus(result) != PGRES_TUPLES_OK)
		{
			PQclear(result);
			result = nullptr;

			PQfinish((PGconn*)connection_);
			connection_ = nullptr;

			return 0;
		}

		unsigned int result_count = atoi(PQcmdTuples(result));

		PQclear(result);
		result = nullptr;

		return result_count;
	}

	std::shared_ptr<container::value_container> postgres_manager::select_query(
		const std::string& query_string)
	{
		std::shared_ptr<container::value_container> container
			= std::make_shared<container::value_container>(
				"query", std::vector<std::shared_ptr<container::value>>{});

		return container;
	}

	bool postgres_manager::disconnect(void)
	{
		if (connection_ == nullptr)
		{
			return false;
		}

		PQfinish((PGconn*)connection_);
		connection_ = nullptr;

		return true;
	}

	void* postgres_manager::query_result(const std::string& query_string)
	{
		if (connection_ == nullptr)
		{
			return nullptr;
		}

		if (PQstatus((PGconn*)connection_) != CONNECTION_OK)
		{
			PQfinish((PGconn*)connection_);
			connection_ = nullptr;

			return nullptr;
		}

		auto [converted_string, error_message]
			= convert_string::utf8_to_system(query_string);
		if (error_message.has_value())
		{
			return nullptr;
		}

		auto converted_query_string = converted_string.value();

		return PQexec((PGconn*)connection_, converted_query_string.c_str());
	}
}; // namespace database