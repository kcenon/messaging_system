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
        bool query(const wstring& query_string);
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