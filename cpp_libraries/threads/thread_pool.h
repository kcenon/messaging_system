#pragma once

#include "thread_worker.h"

#include <mutex>
#include <memory>
#include <vector>

namespace threads
{
	class thread_pool : public std::enable_shared_from_this<thread_pool>
	{
	public:
		thread_pool(const std::vector<std::shared_ptr<thread_worker>>& workers = {});
		~thread_pool(void);

	public:
		void start(void);
		void append(std::shared_ptr<thread_worker> worker, const bool& start = false);
		void stop(const bool& ignore_contained_job = true);

	public:
		void push(std::shared_ptr<job> job);

	protected:
		void notification(const priorities& priority);

	private:
		std::mutex _mutex;
		std::shared_ptr<job_pool> _job_pool;
		std::vector<std::shared_ptr<thread_worker>> _workers;
	};
}

