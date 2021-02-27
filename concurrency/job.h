#pragma once

#include "job_priorities.h"

#include <memory>
#include <functional>

namespace concurrency
{
	class job : public std::enable_shared_from_this<job>
	{
	public:
		job(const std::function<void(void)>& notification = nullptr);
		~job(void);

	public:
		std::shared_ptr<job> get_ptr(void);

	public:
		void set_priority(const priorities& priority);
		const priorities priority(void);

	public:
		void work(void);

	private:
		priorities _priority;
		std::function<void(void)> _notification;
	};
}