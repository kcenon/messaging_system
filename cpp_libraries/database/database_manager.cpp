#include "database_manager.h"

#include "postgre_manager.h"

namespace database
{
    database_manager::database_manager() :
        _connected(false), _database(nullptr)
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

        _database.reset();
        
        switch(database_type)
        {
            case database_types::postgres:
                _database = make_shared<postgre_manager>();
                break;
        }

        if(_database == nullptr)
        {
            return false;
        }

        return true;
    }

    database_types database_manager::database_type(void)
    {
        if(_database == nullptr)
        {
            return database_types::none;
        }

        return _database->database_type();
    }

    bool database_manager::connect(const wstring& connect_string)
    {
        if(_database == nullptr)
        {
            return false;
        }

        return true;
    }

    bool database_manager::query(const wstring& query_string)
    {
        if(_database == nullptr)
        {
            return false;
        }

        return true;
    }

    bool database_manager::disconnect(void)
    {
        if(_database == nullptr)
        {
            return false;
        }

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