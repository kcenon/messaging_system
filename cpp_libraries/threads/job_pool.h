#pragma once

#include "job_priorities.h"

#include <map>
#include <mutex>
#include <queue>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

using namespace std;

namespace threads
{
	class job;
	class job_pool : public enable_shared_from_this<job_pool>
	{
	public:
		job_pool(void);
		~job_pool(void);

	public:
		shared_ptr<job_pool> get_ptr(void);

	public:
		void set_push_lock(const bool& lock_condition);
		void push(shared_ptr<job> new_job);
		shared_ptr<job> pop(const priorities& priority, const vector<priorities>& others = {});
		bool contain(const priorities& priority, const vector<priorities>& others = {});

	public:
		void append_notification(const function<void(const priorities&)>& notification);

	private:
		void notification(const priorities& priority);

	private:
		mutex _mutex;
		bool _lock_condition;
		map<priorities, queue<shared_ptr<job>>> _jobs;
		vector<function<void(const priorities&)>> _notifications;
	};
}
