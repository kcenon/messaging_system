#include "postgres_manager.h"

#include "libpq-fe.h"

#include "converting.h"

namespace database
{
    using namespace converting;

    postgres_manager::postgres_manager(void) : _connection(nullptr)
    {

    }

    postgres_manager::~postgres_manager(void)
    {

    }

    database_types postgres_manager::database_type(void)
    {
        return database_types::postgres;
    }

    bool postgres_manager::connect(const wstring& connect_string)
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

    bool postgres_manager::create_query(const wstring& query_string)
    {
        PGresult *result = (PGresult *)query_result(query_string);
        if(PQresultStatus(result) != PGRES_TUPLES_OK)
        {
            PQclear(result);
            result = nullptr;

            PQfinish((PGconn *)_connection);
            _connection = nullptr;

            return false;
        }

        PQclear(result);
        result = nullptr;

        return true;
    }

    unsigned int postgres_manager::insert_query(const wstring& query_string)
    {
        PGresult *result = (PGresult *)query_result(query_string);
        if(PQresultStatus(result) != PGRES_TUPLES_OK)
        {
            PQclear(result);
            result = nullptr;

            PQfinish((PGconn *)_connection);
            _connection = nullptr;

            return 0;
        }

        unsigned int result_count = atoi(PQcmdTuples(result));

        PQclear(result);
        result = nullptr;

        return result_count;
    }

    unsigned int postgres_manager::update_query(const wstring& query_string)
    {
        PGresult *result = (PGresult *)query_result(query_string);
        if(PQresultStatus(result) != PGRES_TUPLES_OK)
        {
            PQclear(result);
            result = nullptr;

            PQfinish((PGconn *)_connection);
            _connection = nullptr;

            return 0;
        }

        unsigned int result_count = atoi(PQcmdTuples(result));

        PQclear(result);
        result = nullptr;

        return result_count;
    }

    bool postgres_manager::disconnect(void)
    {
        if(_connection == nullptr)
        {
            return false;
        }

        PQfinish((PGconn *)_connection);
        _connection = nullptr;

        return true;
    }

    void* postgres_manager::query_result(const wstring& query_string)
    {
        if(_connection == nullptr)
        {
            return nullptr;
        }

        if(PQstatus((PGconn *)_connection) != CONNECTION_OK)
        {
            PQfinish((PGconn *)_connection);
            _connection = nullptr;

            return nullptr;
        }

        return PQexec((PGconn *)_connection, converter::to_string(query_string).c_str());
    }
};