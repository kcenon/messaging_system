#pragma once

#include "job_priorities.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

using namespace std;

namespace threads
{
	class job_pool;
	class job : public enable_shared_from_this<job>
	{
	public:
		job(const priorities& priority);
		job(const priorities& priority, const vector<unsigned char>& data);
		job(const priorities& priority, const function<void(void)>& working_callback);
		job(const priorities& priority, const vector<unsigned char>& data, const function<void(const vector<unsigned char>&)>& working_callback);
		~job(void);

	public:
		shared_ptr<job> get_ptr(void);

	public:
		const priorities priority(void);

	public:
		void set_job_pool(shared_ptr<job_pool> job_pool);

	public:
		bool work(const priorities& worker_priority);

	protected:
		void save(const wstring& folder_name);
		virtual void working(const priorities& worker_priority);

	private:
		void load(void);

	protected:
		vector<unsigned char> _data;

	private:
		priorities _priority;
		wstring _temporary_stored_path;
		weak_ptr<job_pool> _job_pool;
		
		function<void(void)> _working_callback;
		function<void(const vector<unsigned char>&)> _working_callback2;
	};
}