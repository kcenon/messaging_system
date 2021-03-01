#include "thread_worker.h"

#include "job.h"
#include "job_pool.h"

#include "logging.h"

#include "fmt/format.h"

namespace concurrency
{
	using namespace logging;

	thread_worker::thread_worker(const priorities& priority, const std::vector<priorities>& others)
		: _priority(priority), _others(others)
	{
		job_pool::handle().append_notification(std::bind(&thread_worker::notification, this, std::placeholders::_1));
	}

	thread_worker::~thread_worker(void)
	{

	}

	void thread_worker::start(void)
	{
		stop();

		logger::handle().write(logging::logging_level::sequence, fmt::format(L"attempt to start working thread: priority - {}", _priority));

		_thread = std::thread(&thread_worker::run, this);
	}

	void thread_worker::stop(void)
	{
		_thread_stop.store(true);

		if (_thread.joinable())
		{
			_thread.join();

			logger::handle().write(logging::logging_level::sequence, fmt::format(L"completed to stop working thread: priority - {}", _priority));
		}

		_thread_stop.store(false);
	}

	const priorities thread_worker::priority(void)
	{
		return _priority;
	}

	void thread_worker::run(void)
	{
		while (check_condition(true))
		{
			std::unique_lock<std::mutex> unique(_mutex);
			_condition.wait(unique, [this] { return check_condition(false); });
			unique.unlock();

			working(job_pool::handle().pop(_priority, _others));
		}
	}

	void thread_worker::notification(const priorities& priority)
	{
		_condition.notify_one();
	}

	void thread_worker::working(std::shared_ptr<job> current_job)
	{
		if (current_job == nullptr)
		{
			return;
		}

		current_job->work();
	}

	bool thread_worker::check_condition(const bool& ignore_job)
	{
		if (ignore_job)
		{
			return !_thread_stop.load();
		}

		return !_thread_stop.load() || job_pool::handle().contain(_priority, _others);
	}
}