#include "postgre_manager.h"

namespace database
{
    postgre_manager::postgre_manager(void)
    {

    }

    postgre_manager::~postgre_manager(void)
    {

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