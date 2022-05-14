#include "job.h"

#include "logging.h"
#include "job_pool.h"
#include "converting.h"
#include "folder_handler.h"
#include "file_handler.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <chrono>
#include <future>

#include "crossguid/guid.hpp"

namespace threads
{
	using namespace logging;
	using namespace converting;
	using namespace file_handler;
	using namespace folder_handler;

	job::job(const priorities& priority)
		: _priority(priority), _working_callback(nullptr), _working_callback2(nullptr), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const vector<unsigned char>& data)
		: _priority(priority), _data(data), _working_callback(nullptr), _working_callback2(nullptr), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const function<void(void)>& working_callback)
		: _priority(priority), _working_callback(working_callback), _working_callback2(nullptr), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const vector<unsigned char>& data, const function<void(const vector<unsigned char>&)>& working_callback)
		: _priority(priority), _data(data), _working_callback(nullptr), _working_callback2(working_callback), _temporary_stored_path(L"")
	{
	}

	job::~job(void)
	{
	}

	shared_ptr<job> job::get_ptr(void)
	{
		return shared_from_this();
	}

	const priorities job::priority(void)
	{
		return _priority;
	}

	void job::set_job_pool(shared_ptr<job_pool> job_pool)
	{
		_job_pool = job_pool;
	}

	bool job::work(const priorities& worker_priority)
	{
		load();

		if (_working_callback != nullptr)
		{
			try
			{
				_working_callback();
			}
			catch (...)
			{
				logger::handle().write(logging_level::sequence,
					fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

				return false;
			}

			logger::handle().write(logging_level::sequence, 
				fmt::format(L"completed working callback function without value on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

			return true;
		}

		if (_working_callback2 != nullptr)
		{
			try
			{
				_working_callback2(_data);
			}
			catch (...)
			{
				logger::handle().write(logging_level::sequence,
					fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

				return false;
			}

			logger::handle().write(logging_level::sequence, 
				fmt::format(L"completed working callback function with value on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

			return true;
		}

		try
		{
			working(worker_priority);

			logger::handle().write(logging_level::sequence,
				fmt::format(L"completed working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));
		}
		catch (...) 
		{
			logger::handle().write(logging_level::sequence,
				fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

			return false;
		}

		return true;
	}

	void job::save(void)
	{
		if (_data.empty())
		{
			return;
		}

		_temporary_stored_path = fmt::format(L"{}{}.job", folder::get_temporary_folder(), converter::to_wstring(xg::newGuid().str()));

		file::save(_temporary_stored_path, _data);
		_data.clear();
	}

	void job::working(const priorities& worker_priority)
	{
		logger::handle().write(logging_level::error, 
			L"cannot complete job::working because it does not implemented");
	}

	void job::load(void)
	{
		if (_temporary_stored_path.empty())
		{
			return;
		}

		_data = file::load(_temporary_stored_path);
		file::remove(_temporary_stored_path);
		_temporary_stored_path.clear();
	}
}
