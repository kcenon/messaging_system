#pragma once

#include "thread_worker.h"

#include <mutex>
#include <memory>
#include <vector>

namespace threads
{
	class thread_pool : public std::enable_shared_from_this<thread_pool>
	{
	private:
		thread_pool(const std::vector<std::shared_ptr<thread_worker>>& workers = {});

	public:
		~thread_pool(void);

	public:
		void start(void);
		void append(std::shared_ptr<thread_worker> worker, const bool& start = false);
		void stop(const bool& clear = false);

	public:
		void push(std::shared_ptr<job> job);

	protected:
		void notification(const priorities& priority);

	private:
		std::mutex _mutex;
		std::shared_ptr<job_pool> _job_pool;
		std::vector<std::shared_ptr<thread_worker>> _workers;

#pragma region singleton
	public:
		static thread_pool& handle(void);

	private:
		static std::unique_ptr<thread_pool> _handle;
		static std::once_flag _once;
#pragma endregion
	};
}

