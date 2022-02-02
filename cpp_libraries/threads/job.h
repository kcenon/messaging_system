#pragma once

#include "job_priorities.h"

#include <memory>
#include <vector>
#include <functional>

namespace threads
{
	class job_pool;
	class job : public std::enable_shared_from_this<job>
	{
	public:
		job(const priorities& priority);
		job(const priorities& priority, const std::vector<unsigned char>& data, const bool& is_async_callback = false);
		job(const priorities& priority, const std::function<bool(void)>& working_callback, const bool& is_async_callback = false);
		job(const priorities& priority, const std::vector<unsigned char>& data, const std::function<bool(const std::vector<unsigned char>&)>& working_callback, const bool& is_async_callback = false);
		~job(void);

	public:
		std::shared_ptr<job> get_ptr(void);

	public:
		const priorities priority(void);

	public:
		void set_job_pool(std::shared_ptr<job_pool> job_pool);

	public:
		void save(void);
		bool work(const priorities& worker_priority);

	protected:
		virtual bool working(const priorities& worker_priority);

	private:
		std::wstring do_script(const std::wstring& script);

	private:
		void load(void);

	protected:
		std::vector<unsigned char> _data;

	private:
		priorities _priority;
		bool _temporary_stored;
		std::wstring _temporary_stored_path;
		std::weak_ptr<job_pool> _job_pool;
		
		bool _is_async_callback;
		std::function<bool(void)> _working_callback;
		std::function<bool(const std::vector<unsigned char>&)> _working_callback2;
	};
}