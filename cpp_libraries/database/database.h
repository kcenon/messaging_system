#pragma once

#include <string>

namespace database
{
    using namespace std;
    
    class database
    {
    public:
        database(void) {}
        virtual ~database(void) {}

    public:
        virtual bool connect(const wstring& connect_string) = 0;
        virtual bool query(const wstring& query_string) = 0;
        virtual bool disconnect(void) = 0;
    };
};