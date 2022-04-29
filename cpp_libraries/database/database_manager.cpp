#include "database_manager.h"

namespace database
{
    database_manager::database_manager() :
        _connected(false), _database_type(database_types::postgres)
    {
    }

    database_manager::~database_manager()
    {

    }

    bool database_manager::set_mode(const database_types& database_type)
    {
        if(_connected)
        {
            return false;
        }
        
        _database_type = database_type;

        return true;
    }

    bool database_manager::connect(const wstring& connect_string)
    {
        return true;
    }

    bool database_manager::query(const wstring& query_string)
    {
        return true;
    }

    bool database_manager::disconnect(void)
    {
        return true;
    }

#pragma region singleton
	unique_ptr<database_manager> database_manager::_handle;
	once_flag database_manager::_once;

	database_manager& database_manager::handle(void)
	{
		call_once(_once, []()
			{
				_handle.reset(new database_manager);
			});

		return *_handle.get();
	}
#pragma endregion
};