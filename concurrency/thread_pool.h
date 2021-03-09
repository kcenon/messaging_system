#pragma once

#include "thread_worker.h"

#include <mutex>
#include <memory>
#include <vector>

namespace concurrency
{
	class thread_pool
	{
	private:
		thread_pool(const std::vector<std::shared_ptr<thread_worker>>& workers = {});

	public:
		~thread_pool(void);

	public:
		void clear(void);
		void append(std::shared_ptr<thread_worker> worker);

	public:
		void start(void);
		void stop(void);

	private:
		std::mutex _mutex;
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

