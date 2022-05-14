#pragma once

#include <mutex>
#include <memory>

#include "database.h"

namespace database
{
    class database_manager
    {
    public:
        database_manager(void);
        virtual ~database_manager(void);

    public:
        bool set_mode(const database_types& database_type);
        database_types database_type(void);

    public:
        bool connect(const wstring& connect_string);
        bool create_query(const wstring& query_string);
        unsigned int insert_query(const wstring& query_string);
        unsigned int update_query(const wstring& query_string);
        unsigned int delete_query(const wstring& query_string);
#ifndef __USE_TYPE_CONTAINER__
        shared_ptr<json::value> select_query(const wstring& query_string);
#else
        shared_ptr<container::value_container> select_query(const wstring& query_string);
#endif
        bool disconnect(void);

    private:
        bool _connected;
        shared_ptr<database> _database;

#pragma region singleton
	public:
		static database_manager& handle(void);

	private:
		static unique_ptr<database_manager> _handle;
		static once_flag _once;
#pragma endregion
    };
};