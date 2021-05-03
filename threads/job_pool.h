#pragma once

#include "job_priorities.h"

#include <map>
#include <mutex>
#include <queue>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace threads
{
	class job;
	class job_pool : public std::enable_shared_from_this<job_pool>
	{
	public:
		job_pool(void);
		~job_pool(void);

	public:
		std::shared_ptr<job_pool> get_ptr(void);

	public:
		void set_push_lock(const bool& lock_condition);
		void push(std::shared_ptr<job> new_job);
		std::shared_ptr<job> pop(const priorities& priority, const std::vector<priorities>& others = {});
		bool contain(const priorities& priority, const std::vector<priorities>& others = {});

	public:
		void append_notification(const std::function<void(const priorities&)>& notification);

	private:
		void notification(const priorities& priority);

	private:
		std::mutex _mutex;
		bool _lock_condition;
		std::map<priorities, std::queue<std::shared_ptr<job>>> _jobs;
		std::vector<std::function<void(const priorities&)>> _notifications;
	};
}
