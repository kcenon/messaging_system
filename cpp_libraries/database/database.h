#pragma once

#include <string>

#include "database_types.h"

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "container.h"
#endif

namespace database
{
    using namespace std;
    
#ifndef __USE_TYPE_CONTAINER__
	using namespace web;
#endif

    class database
    {
    public:
        database(void) {}
        virtual ~database(void) {}

    public:
        virtual database_types database_type(void) = 0;
        virtual bool connect(const wstring& connect_string) = 0;
        virtual bool create_query(const wstring& query_string) = 0;
        virtual unsigned int insert_query(const wstring& query_string) = 0;
        virtual unsigned int update_query(const wstring& query_string) = 0;
        virtual unsigned int delete_query(const wstring& query_string) = 0;
#ifndef __USE_TYPE_CONTAINER__
        virtual shared_ptr<json::value> select_query(const wstring& query_string) = 0;
#else
        virtual shared_ptr<container::value_container> select_query(const wstring& query_string) = 0;
#endif
        virtual bool disconnect(void) = 0;
    };
};