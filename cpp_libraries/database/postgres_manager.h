#include "database.h"

namespace database
{
    class postgre_manager : public database
    {
    public:
        postgre_manager();
        virtual ~postgre_manager();

    public:
        database_types database_type(void) override;
        bool connect(const wstring& connect_string) override;
        bool query(const wstring& query_string) override;
        bool disconnect(void) override;

    private:
        void *_connection;
    };
};