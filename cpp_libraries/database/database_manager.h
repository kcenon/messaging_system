#pragma once

#include <mutex>
#include <memory>

#include "database.h"
#include "database_types.h"

namespace database
{
    class database_manager
    {
    public:
        database_manager(void);
        virtual ~database_manager(void);

    public:
        bool set_mode(const database_types& database_type);
        bool connect(const wstring& connect_string);
        bool query(const wstring& query_string);
        bool disconnect(void);

    private:
        bool _connected;
        database_types _database_type;
        shared_ptr<database> _databse;

#pragma region singleton
	public:
		static database_manager& handle(void);

	private:
		static unique_ptr<database_manager> _handle;
		static once_flag _once;
#pragma endregion
    };
};