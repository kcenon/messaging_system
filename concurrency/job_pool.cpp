#include "job_pool.h"

#include "job.h"
#include "logging.h"

#include "fmt/format.h"

namespace concurrency
{
	job_pool::job_pool(void)
	{

	}

	job_pool::~job_pool(void)
	{

	}

	void job_pool::push(std::shared_ptr<job> new_job)
	{
		if (new_job == nullptr)
		{
			return;
		}

		std::lock_guard<std::mutex> guard(_mutex);

		auto iterator = _jobs.find(new_job->priority());
		if (iterator != _jobs.end())
		{
			iterator->second.push(new_job);

			logging::util::handle().write(logging::logging_level::parameter, fmt::format(L"push new job: priority - {}", new_job->priority()));

			notification(new_job->priority());

			return;
		}

		std::queue<std::shared_ptr<job>> queue;
		queue.push(new_job);

		_jobs.insert({ new_job->priority(), queue });

		logging::util::handle().write(logging::logging_level::parameter, fmt::format(L"push new job: priority - {}", new_job->priority()));

		notification(new_job->priority());
	}

	std::shared_ptr<job> job_pool::pop(const priorities& priority, const std::vector<priorities>& others)
	{
		std::unique_lock<std::mutex> unique(_mutex);

		auto iterator = _jobs.find(priority);
		if (iterator != _jobs.end() && !iterator->second.empty())
		{
			std::shared_ptr<job> temp = iterator->second.front();
			iterator->second.pop();

			logging::util::handle().write(logging::logging_level::parameter, fmt::format(L"pop a job: priority - {}", temp->priority()));

			return temp;
		}

		for (auto& other : others)
		{
			auto iterator = _jobs.find(priority);
			if (iterator != _jobs.end() && !iterator->second.empty())
			{
				std::shared_ptr<job> temp = iterator->second.front();
				iterator->second.pop();

				logging::util::handle().write(logging::logging_level::parameter, fmt::format(L"pop a job: priority - {}", temp->priority()));

				return temp;
			}
		}

		return nullptr;
	}

	bool job_pool::contain(const priorities& priority, const std::vector<priorities>& others)
	{
		std::lock_guard<std::mutex> unique(_mutex);

		auto iterator = _jobs.find(priority);
		if (iterator != _jobs.end() && !iterator->second.empty())
		{
			std::shared_ptr<job> temp = iterator->second.front();

			return temp != nullptr;
		}

		for (auto& other : others)
		{
			auto iterator = _jobs.find(priority);
			if (iterator != _jobs.end() && !iterator->second.empty())
			{
				std::shared_ptr<job> temp = iterator->second.front();

				return temp != nullptr;
			}
		}

		return false;
	}

	void job_pool::append_notification(const std::function<void(const priorities&)>& notification)
	{
		_notifications.push_back(notification);
	}

	void job_pool::notification(const priorities& priority)
	{
		for (auto& notification : _notifications)
		{
			if (notification == nullptr)
			{
				continue;
			}

			notification(priority);
		}
	}

#pragma region singleton
	std::unique_ptr<job_pool> job_pool::_handle;
	std::once_flag job_pool::_once;

	job_pool& job_pool::handle(void)
	{
		std::call_once(_once, []()
			{
				_handle.reset(new job_pool);
			});

		return *_handle.get();
	}
#pragma endregion
}