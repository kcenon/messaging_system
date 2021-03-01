#pragma once

#include "job_priorities.h"

#include <memory>
#include <vector>
#include <functional>

namespace concurrency
{
	class job : public std::enable_shared_from_this<job>
	{
	public:
		job(const priorities& priority);
		job(const priorities& priority, const std::function<void(void)>& working_callback);
		job(const priorities& priority, const std::vector<char>& data, const std::function<void(const std::vector<char>&)>& working_callback);
		~job(void);

	public:
		std::shared_ptr<job> get_ptr(void);

	public:
		const priorities priority(void);

	public:
		void work(void);

	protected:
		virtual bool working(void);

	private:
		priorities _priority;
		std::vector<char> _data;
		std::function<void(void)> _working_callback;
		std::function<void(const std::vector<char>&)> _working_callback2;
	};
}