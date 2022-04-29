#include "postgre_manager.h"

namespace database
{
    postgre_manager::postgre_manager(void)
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
        return true;
    }

    bool postgre_manager::query(const wstring& query_string)
    {
        return true;
    }
    bool postgre_manager::disconnect(void)
    {
        return true;
    }
};