#pragma once

#include "job_priorities.h"

#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <condition_variable>

using namespace std;

namespace threads
{
	class job;
	class job_pool;
	class thread_worker
	{
	public:
		thread_worker(const priorities& priority, const vector<priorities>& others = {});
		~thread_worker(void);

	public:
		void set_job_pool(shared_ptr<job_pool> job_pool);

	public:
		void start(void);
		void stop(const bool& ignore_contained_job = true);

	public:
		const priorities priority(void);
		void notification(const priorities& priority);

	protected:
		void run(void);

	protected:
		virtual void working(shared_ptr<job> current_job);

	protected:
		bool check_condition(void);

	private:
		atomic<bool> _thread_stop{ false };
		atomic<bool> _ignore_contained_job{ false };

	private:
		priorities _priority;
		vector<priorities> _others;
		shared_ptr<job_pool> _job_pool;

	private:
		mutex _mutex;
		thread _thread;
		condition_variable _condition;
	};
}

