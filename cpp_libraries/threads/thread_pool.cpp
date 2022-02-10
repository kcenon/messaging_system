#include "thread_pool.h"

#include "logging.h"
#include "job_pool.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace threads
{
	using namespace logging;

	thread_pool::thread_pool(const vector<shared_ptr<thread_worker>>& workers)
		: _workers(workers), _job_pool(make_shared<job_pool>())
	{
		_job_pool->append_notification(bind(&thread_pool::notification, this, placeholders::_1));
	}

	thread_pool::~thread_pool(void)
	{
		_job_pool.reset();
	}

	void thread_pool::start(void)
	{
		scoped_lock<mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->start();
		}
	}

	void thread_pool::append(shared_ptr<thread_worker> worker, const bool& start)
	{
		unique_lock<mutex> unique(_mutex);
		
		worker->set_job_pool(_job_pool);
		_workers.push_back(worker);

		logger::handle().write(logging_level::parameter, fmt::format(L"appended new worker: priority - {}", (int)worker->priority()));

		unique.unlock();

		if (start)
		{
			worker->start();
		}
	}

	void thread_pool::stop(const bool& ignore_contained_job)
	{
		scoped_lock<mutex> guard(_mutex);

		if (_job_pool != nullptr)
		{
			_job_pool->set_push_lock(!ignore_contained_job);
		}

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->stop(ignore_contained_job);
		}

		_workers.clear();
	}

	void thread_pool::push(shared_ptr<job> job)
	{
		if (_job_pool == nullptr)
		{
			return;
		}

		_job_pool->push(job);
	}

	void thread_pool::notification(const priorities& priority)
	{
		scoped_lock<mutex> guard(_mutex);

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