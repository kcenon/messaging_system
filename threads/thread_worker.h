#pragma once

#include "job_priorities.h"

#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <condition_variable>

namespace threads
{
	class job;
	class job_pool;
	class thread_worker
	{
	public:
		thread_worker(const priorities& priority, const std::vector<priorities>& others = {});
		~thread_worker(void);

	public:
		void set_job_pool(std::shared_ptr<job_pool> job_pool);

	public:
		void start(void);
		void stop(void);

	public:
		const priorities priority(void);
		void notification(const priorities& priority);

	protected:
		void run(void);

	protected:
		virtual void working(std::shared_ptr<job> current_job);

	protected:
		bool check_condition(void);

	private:
		std::atomic<bool> _thread_stop{ false };

	private:
		priorities _priority;
		std::vector<priorities> _others;
		std::shared_ptr<job_pool> _job_pool;

	private:
		std::mutex _mutex;
		std::thread _thread;
		std::condition_variable _condition;
	};
}

