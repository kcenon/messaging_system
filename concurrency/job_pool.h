#pragma once

#include "job_priorities.h"

#include <map>
#include <mutex>
#include <queue>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace concurrency
{
	class job;
	class job_pool
	{
	private:
		job_pool(void);

	public:
		~job_pool(void);

	public:
		void push(std::shared_ptr<job> new_job);
		std::shared_ptr<job> pop(const priorities& priority, const std::vector<priorities>& others = {});
		bool contain(const priorities& priority, const std::vector<priorities>& others = {});

	public:
		void append_notification(const std::function<void(const priorities&)>& notification);

	private:
		void notification(const priorities& priority);

	private:
		std::mutex _mutex;
		std::map<priorities, std::queue<std::shared_ptr<job>>> _jobs;
		std::vector<std::function<void(const priorities&)>> _notifications;

#pragma region singleton
	public:
		static job_pool& handle(void);

	private:
		static std::unique_ptr<job_pool> _handle;
		static std::once_flag _once;
#pragma endregion
	};
}
