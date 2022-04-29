#include "database_manager.h"

namespace database
{
    database_manager::database_manager()
    {

    }

    database_manager::~database_manager()
    {

    }

    bool database_manager::set_mode()
    {
        return true;
    }

    bool database_manager::connect()
    {
        return true;
    }

    bool database_manager::query()
    {
        return true;
    }

    bool database_manager::disconnect()
    {
        return true;
    }

#pragma region singleton
	unique_ptr<database_manager> database_manager::_handle;
	once_flag database_manager::_once;

	database_manager& database_manager::handle(void)
	{
		call_once(_once, []()
			{
				_handle.reset(new database_manager);
			});

		return *_handle.get();
	}
#pragma endregion
};