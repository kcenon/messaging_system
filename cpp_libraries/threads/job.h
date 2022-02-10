#pragma once

#include "job_priorities.h"

#include <memory>
#include <vector>
#include <functional>

using namespace std;

namespace threads
{
	class job_pool;
	class job : public enable_shared_from_this<job>
	{
	public:
		job(const priorities& priority);
		job(const priorities& priority, const vector<unsigned char>& data, const bool& is_async_callback = false);
		job(const priorities& priority, const function<bool(void)>& working_callback, const bool& is_async_callback = false);
		job(const priorities& priority, const vector<unsigned char>& data, const function<bool(const vector<unsigned char>&)>& working_callback, const bool& is_async_callback = false);
		~job(void);

	public:
		shared_ptr<job> get_ptr(void);

	public:
		const priorities priority(void);

	public:
		void set_job_pool(shared_ptr<job_pool> job_pool);

	public:
		void save(void);
		bool work(const priorities& worker_priority);

	protected:
		virtual bool working(const priorities& worker_priority);

	private:
		wstring do_script(const wstring& script);

	private:
		void load(void);

	protected:
		vector<unsigned char> _data;

	private:
		priorities _priority;
		bool _temporary_stored;
		wstring _temporary_stored_path;
		weak_ptr<job_pool> _job_pool;
		
		bool _is_async_callback;
		function<bool(void)> _working_callback;
		function<bool(const vector<unsigned char>&)> _working_callback2;
	};
}