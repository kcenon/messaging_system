#pragma once

#include "job_priorities.h"

#include <memory>
#include <vector>
#include <functional>

namespace threads
{
	class job : public std::enable_shared_from_this<job>
	{
	public:
		job(const priorities& priority);
		job(const priorities& priority, const std::function<bool(void)>& working_callback);
		job(const priorities& priority, const std::vector<unsigned char>& data, const std::function<bool(const std::vector<unsigned char>&)>& working_callback);
		~job(void);

	public:
		std::shared_ptr<job> get_ptr(void);

	public:
		const priorities priority(void);

	public:
		bool work(const priorities& priority);

	protected:
		virtual bool working(const priorities& priority);

	private:
		priorities _priority;
		std::vector<unsigned char> _data;
		std::function<bool(void)> _working_callback;
		std::function<bool(const std::vector<unsigned char>&)> _working_callback2;
	};
}