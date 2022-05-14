#pragma once

#include "thread_worker.h"

#include <mutex>
#include <memory>
#include <vector>

using namespace std;

namespace threads
{
	class thread_pool : public enable_shared_from_this<thread_pool>
	{
	public:
		thread_pool(const vector<shared_ptr<thread_worker>>& workers = {});
		~thread_pool(void);

	public:
		void start(void);
		void append(shared_ptr<thread_worker> worker, const bool& start = false);
		void stop(const bool& ignore_contained_job = true);

	public:
		void push(shared_ptr<job> job);

	protected:
		void notification(const priorities& priority);

	private:
		mutex _mutex;
		shared_ptr<job_pool> _job_pool;
		vector<shared_ptr<thread_worker>> _workers;
	};
}

