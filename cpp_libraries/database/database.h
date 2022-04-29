#pragma once

#include <string>

#include "database_types.h"

namespace database
{
    using namespace std;

    class database
    {
    public:
        database(void) {}
        virtual ~database(void) {}

    public:
        virtual database_types database_type(void) = 0;
        virtual bool connect(const wstring& connect_string) = 0;
        virtual bool query(const wstring& query_string) = 0;
        virtual bool disconnect(void) = 0;
    };
};