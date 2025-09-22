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

#include "database/database_manager.h"

#include "database/postgres_manager.h"
#include "database/backends/mysql/mysql_manager.h"
#include "database/backends/sqlite/sqlite_manager.h"
#include "database/backends/mongodb/mongodb_manager.h"
#include "database/backends/redis/redis_manager.h"

namespace database
{
	database_manager::database_manager() : connected_(false), database_(nullptr)
	{
	}

	database_manager::~database_manager() {}

	bool database_manager::set_mode(const database_types& database_type)
	{
		if (connected_)
		{
			return false;
		}

		database_.reset();

		switch (database_type)
		{
		case database_types::postgres:
			database_ = std::make_unique<postgres_manager>();
			break;
		case database_types::mysql:
			database_ = std::make_unique<mysql_manager>();
			break;
		case database_types::sqlite:
			database_ = std::make_unique<sqlite_manager>();
			break;
		case database_types::mongodb:
			database_ = std::make_unique<mongodb_manager>();
			break;
		case database_types::redis:
			database_ = std::make_unique<redis_manager>();
			break;
		default:
			break;
		}

		if (database_ == nullptr)
		{
			return false;
		}

		return true;
	}

	database_types database_manager::database_type(void)
	{
		if (!database_)
		{
			return database_types::none;
		}

		return database_->database_type();
	}

	bool database_manager::connect(const std::string& connect_string)
	{
		if (!database_)
		{
			return false;
		}

		return database_->connect(connect_string);
	}

	bool database_manager::create_query(const std::string& query_string)
	{
		if (!database_)
		{
			return false;
		}

		return database_->create_query(query_string);
	}

	unsigned int database_manager::insert_query(const std::string& query_string)
	{
		if (!database_)
		{
			return 0;
		}

		return database_->insert_query(query_string);
	}

	unsigned int database_manager::update_query(const std::string& query_string)
	{
		if (database_ == nullptr)
		{
			return 0;
		}

		return database_->update_query(query_string);
	}

	unsigned int database_manager::delete_query(const std::string& query_string)
	{
		if (database_ == nullptr)
		{
			return 0;
		}

		return database_->delete_query(query_string);
	}

	database_result database_manager::select_query(const std::string& query_string)
	{
		if (database_ == nullptr)
		{
			return database_result{};
		}

		return database_->select_query(query_string);
	}

	bool database_manager::disconnect(void)
	{
		if (database_ == nullptr)
		{
			return false;
		}

		return database_->disconnect();
	}

	bool database_manager::create_connection_pool(database_types db_type, const connection_pool_config& config)
	{
		return connection_pool_manager::instance().create_pool(db_type, config);
	}

	std::shared_ptr<connection_pool_base> database_manager::get_connection_pool(database_types db_type)
	{
		return connection_pool_manager::instance().get_pool(db_type);
	}

	std::map<database_types, connection_stats> database_manager::get_pool_stats() const
	{
		return connection_pool_manager::instance().get_all_stats();
	}

	query_builder database_manager::create_query_builder()
	{
		return query_builder(database_type());
	}

	query_builder database_manager::create_query_builder(database_types db_type)
	{
		return query_builder(db_type);
	}

#pragma region singleton
	std::unique_ptr<database_manager> database_manager::handle_;
	std::once_flag database_manager::once_;

	database_manager& database_manager::handle(void)
	{
		std::call_once(once_, []() { handle_ = std::make_unique<database_manager>(); });

		return *handle_;
	}
#pragma endregion
}; // namespace database