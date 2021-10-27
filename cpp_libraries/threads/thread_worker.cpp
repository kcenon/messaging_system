#include "thread_worker.h"

#include "job.h"
#include "job_pool.h"

#include "logging.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace threads
{
	using namespace logging;

	thread_worker::thread_worker(const priorities& priority, const std::vector<priorities>& others)
		: _priority(priority), _others(others), _job_pool(nullptr)
	{
	}

	thread_worker::~thread_worker(void)
	{
		stop();
	}

	void thread_worker::set_job_pool(std::shared_ptr<job_pool> job_pool)
	{
		_job_pool = job_pool;
	}

	void thread_worker::start(void)
	{
		stop();

		_thread_stop.store(false);
		_thread = std::thread(&thread_worker::run, this);
	}

	void thread_worker::stop(const bool& ignore_contained_job)
	{
		if (!_thread.joinable())
		{
			return;
		}

		_ignore_contained_job.store(ignore_contained_job);

		_thread_stop.store(true);

		_condition.notify_one();

		_thread.join();
	}

	const priorities thread_worker::priority(void)
	{
		return _priority;
	}

	void thread_worker::notification(const priorities& priority)
	{
		if (_priority == priority)
		{
			_condition.notify_one();

			return;
		}

		auto target = std::find(_others.begin(), _others.end(), priority);
		if (target == _others.end())
		{
			return;
		}

		_condition.notify_one();
	}

	void thread_worker::run(void)
	{
		logger::handle().write(logging::logging_level::sequence, fmt::format(L"start working thread: priority - {}", _priority));

		while (!_thread_stop.load() || !_ignore_contained_job.load())
		{
			std::unique_lock<std::mutex> unique(_mutex);
			_condition.wait(unique, [this] { return check_condition(); });
			if (_job_pool == nullptr)
			{
				continue;
			}

			if (_thread_stop.load() && _ignore_contained_job.load())
			{
				break;
			}

			std::shared_ptr<job> current_job = _job_pool->pop(_priority, _others);
			unique.unlock();

			if (current_job == nullptr && _thread_stop.load())
			{
				break;
			}

			working(current_job);
		}

		logger::handle().write(logging::logging_level::sequence, fmt::format(L"stop working thread: priority - {}", _priority));
	}

	void thread_worker::working(std::shared_ptr<job> current_job)
	{
		if (current_job == nullptr)
		{
			return;
		}

		current_job->work(_priority);
	}

	bool thread_worker::check_condition(void)
	{
		if (_thread_stop.load())
		{
			return true;
		}

		if (_job_pool == nullptr)
		{
			return false;
		}

		return _job_pool->contain(_priority, _others);
	}
}