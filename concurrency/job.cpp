#include "job.h"

#include "logging.h"

namespace concurrency
{
	job::job(const priorities& priority)
		: _priority(priority), _working_callback(nullptr), _working_callback2(nullptr)
	{
	}

	job::job(const priorities& priority, const std::function<bool(void)>& working_callback)
		: _priority(priority), _working_callback(working_callback), _working_callback2(nullptr)
	{
	}

	job::job(const priorities& priority, const std::vector<char>& data, const std::function<bool(const std::vector<char>&)>& working_callback)
		: _priority(priority), _data(data), _working_callback(nullptr), _working_callback2(working_callback)
	{
	}

	job::~job(void)
	{
	}

	std::shared_ptr<job> job::get_ptr(void)
	{
		return shared_from_this();
	}

	const priorities job::priority(void)
	{
		return _priority;
	}

	bool job::work(void)
	{
		if (_working_callback != nullptr)
		{
			bool result = _working_callback();

			logging::util::handle().write(logging::logging_level::sequence, L"completed working callback function without values on job");

			return result;
		}

		if (_working_callback2 != nullptr)
		{
			bool result = _working_callback2(_data);

			logging::util::handle().write(logging::logging_level::sequence, L"completed working callback function with values on job");

			return result;
		}

		if (!working())
		{
			logging::util::handle().write(logging::logging_level::error, L"cannot complete working function on job");

			return false;
		}

		return true;
	}

	bool job::working(void)
	{
		logging::util::handle().write(logging::logging_level::error, L"need to implement working function on job");

		return false;
	}
}