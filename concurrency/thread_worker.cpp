#include "thread_worker.h"

#include "job.h"
#include "job_pool.h"

namespace concurrency
{
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

		_thread = std::thread(&thread_worker::run, this);
	}

	void thread_worker::stop(void)
	{
		_thread_stop.store(true);

		if (_thread.joinable())
		{
			_thread.join();
		}

		_thread_stop.store(false);
	}

	void thread_worker::run(void)
	{
		while (!_thread_stop.load())
		{
			std::unique_lock<std::mutex> unique(_mutex);
			_condition.wait(unique, [this] { return !_thread_stop.load() || job_pool::handle().contain(_priority, _others); });
			unique.unlock();

			auto job = job_pool::handle().pop(_priority, _others);
			if (job == nullptr)
			{
				continue;
			}

			job->work();
		}
	}

	void thread_worker::notification(const priorities& priority)
	{
		_condition.notify_one();
	}

	void thread_worker::working(void)
	{

	}
}