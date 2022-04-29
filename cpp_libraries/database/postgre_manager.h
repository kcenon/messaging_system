#include "database.h"

namespace database
{
    class postgre_manager : public database
    {
    public:
        postgre_manager();
        virtual ~postgre_manager();

    public:
        bool connect() override;
        bool query() override;
        bool disconnect() override;
    };
};