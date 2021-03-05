#include "job.h"

#include "logging.h"

#include "fmt/format.h"

namespace concurrency
{
	using namespace logging;

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

	bool job::work(const priorities& priority)
	{
		if (_working_callback != nullptr)
		{
			bool result = _working_callback();

			logger::handle().write(logging::logging_level::sequence, fmt::format(L"completed working callback function without values on job: job priority[{}], worker priority[{}]", _priority, priority));

			return result;
		}

		if (_working_callback2 != nullptr)
		{
			bool result = _working_callback2(_data);

			logger::handle().write(logging::logging_level::sequence, fmt::format(L"completed working callback function with values on job: job priority[{}], worker priority[{}]", _priority, priority));

			return result;
		}

		if (!working(priority))
		{
			logger::handle().write(logging::logging_level::sequence, fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", _priority, priority));

			return false;
		}

		return true;
	}

	bool job::working(const priorities& priority)
	{
		logger::handle().write(logging::logging_level::sequence, fmt::format(L"need to implement working function on job: job priority[{}], worker priority[{}]", _priority, priority));

		return false;
	}
}