#include "thread_pool.h"

#include "logging.h"

#include "fmt/format.h"

namespace concurrency
{
	thread_pool::thread_pool(const std::vector<std::shared_ptr<thread_worker>>& workers)
		: _workers(workers)
	{

	}

	thread_pool::~thread_pool(void)
	{

	}

	void thread_pool::append(std::shared_ptr<thread_worker> worker)
	{
		std::unique_lock<std::mutex> unique(_mutex);

		_workers.push_back(worker);

		logging::util::handle().write(logging::logging_level::parameter, fmt::format(L"appended new worker: priority - {}", worker->priority()));
	}

	void thread_pool::start(void)
	{
		std::lock_guard<std::mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->start();
		}
	}

	void thread_pool::stop(void)
	{
		std::lock_guard<std::mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->stop();
		}
	}

#pragma region singleton
	std::unique_ptr<thread_pool> thread_pool::_handle;
	std::once_flag thread_pool::_once;

	thread_pool& thread_pool::handle(void)
	{
		std::call_once(_once, []()
			{
				_handle.reset(new thread_pool);
			});

		return *_handle.get();
	}
#pragma endregion
}