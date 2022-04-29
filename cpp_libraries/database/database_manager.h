#pragma once

#include <mutex>
#include <memory>

namespace database
{
    using namespace std;

    class database_manager
    {
    public:
        database_manager(void);
        virtual ~database_manager(void);

    public:
        bool set_mode();
        bool connect();
        bool query();
        bool disconnect();
        
#pragma region singleton
	public:
		static database_manager& handle(void);

	private:
		static unique_ptr<database_manager> _handle;
		static once_flag _once;
#pragma endregion
    }
};