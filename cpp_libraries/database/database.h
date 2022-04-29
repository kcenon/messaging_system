#pragma once

namespace database
{
    class database
    {
    public:
        database() {}
        virtual ~database() {}

    public:
        virtual bool connect() = 0;
        virtual bool query() = 0;
        virtual bool disconnect() = 0;
    };
};