#include "postgres_manager.h"

#include "libpq-fe.h"

#include "converting.h"

namespace database
{
    using namespace converting;

    postgre_manager::postgre_manager(void) : _connection(nullptr)
    {

    }

    postgre_manager::~postgre_manager(void)
    {

    }

    database_types postgre_manager::database_type(void)
    {
        return database_types::postgres;
    }

    bool postgre_manager::connect(const wstring& connect_string)
    {
        _connection = PQconnectdb(converter::to_string(connect_string).c_str());
        if(PQstatus((PGconn *)_connection) != CONNECTION_OK)
        {
            PQfinish((PGconn *)_connection);
            _connection = nullptr;

            return false;
        }
        
        return true;
    }

    bool postgre_manager::query(const wstring& query_string)
    {
        if(_connection == nullptr)
        {
            return false;
        }

        if(PQstatus((PGconn *)_connection) != CONNECTION_OK)
        {
            PQfinish((PGconn *)_connection);
            _connection = nullptr;

            return false;
        }

        return true;
    }
    bool postgre_manager::disconnect(void)
    {
        if(_connection == nullptr)
        {
            return false;
        }

        PQfinish((PGconn *)_connection);
        _connection = nullptr;

        return true;
    }
};