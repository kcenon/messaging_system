#include "database_manager.h"

#include "postgres_manager.h"

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
                _database = make_shared<postgres_manager>();
                break;
            default:
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

        return _database->connect(connect_string);
    }

    bool database_manager::create_query(const wstring& query_string)
    {
        if(_database == nullptr)
        {
            return false;
        }

        return _database->create_query(query_string);
    }

    unsigned int database_manager::insert_query(const wstring& query_string)
    {
        if(_database == nullptr)
        {
            return 0;
        }

        return _database->insert_query(query_string);
    }

    unsigned int database_manager::update_query(const wstring& query_string)
    {
        if(_database == nullptr)
        {
            return 0;
        }

        return _database->update_query(query_string);
    }

    unsigned int database_manager::delete_query(const wstring& query_string)
    {
        if(_database == nullptr)
        {
            return 0;
        }

        return _database->update_query(query_string);
    }

#ifndef __USE_TYPE_CONTAINER__
    shared_ptr<json::value> database_manager::select_query(const wstring& query_string)
#else
    shared_ptr<container::value_container> database_manager::select_query(const wstring& query_string)
#endif
    {
        if(_database == nullptr)
        {
            return 0;
        }

        return _database->select_query(query_string);
    }

    bool database_manager::disconnect(void)
    {
        if(_database == nullptr)
        {
            return false;
        }

        return _database->disconnect();
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