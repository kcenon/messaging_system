#include "thread_pool.h"

#include "logging.h"
#include "job_pool.h"

#include "fmt/format.h"

namespace threads
{
	using namespace logging;

	thread_pool::thread_pool(const std::vector<std::shared_ptr<thread_worker>>& workers)
		: _workers(workers), _job_pool(std::make_shared<job_pool>())
	{
		_job_pool->append_notification(std::bind(&thread_pool::notification, this, std::placeholders::_1));
	}

	thread_pool::~thread_pool(void)
	{
		_job_pool.reset();
	}

	void thread_pool::start(void)
	{
		std::scoped_lock<std::mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->start();
		}
	}

	void thread_pool::append(std::shared_ptr<thread_worker> worker, const bool& start)
	{
		std::unique_lock<std::mutex> unique(_mutex);
		
		worker->set_job_pool(_job_pool);
		_workers.push_back(worker);

		logger::handle().write(logging::logging_level::parameter, fmt::format(L"appended new worker: priority - {}", worker->priority()));

		unique.unlock();

		if (start)
		{
			worker->start();
		}
	}

	void thread_pool::stop(const bool& clear)
	{
		std::scoped_lock<std::mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->stop();
		}

		if (clear)
		{
			_workers.clear();
		}
	}

	void thread_pool::push(std::shared_ptr<job> job)
	{
		if (_job_pool == nullptr)
		{
			return;
		}

		_job_pool->push(job);
	}

	void thread_pool::notification(const priorities& priority)
	{
		std::scoped_lock<std::mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->notification(priority);
		}
	}
}