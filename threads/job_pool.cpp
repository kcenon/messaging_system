/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "job_pool.h"

#include "job.h"
#include "logging.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <future>

namespace threads
{
	using namespace logging;

	job_pool::job_pool(void)
	{
	}

	job_pool::~job_pool(void)
	{
		_jobs.clear();
	}

	shared_ptr<job_pool> job_pool::get_ptr(void)
	{
		return shared_from_this();
	}

	void job_pool::push(shared_ptr<job> new_job)
	{
		if (new_job == nullptr)
		{
			return;
		}

		new_job->set_job_pool(get_ptr());

		scoped_lock<mutex> guard(_mutex);

		auto iterator = _jobs.find(new_job->priority());
		if (iterator != _jobs.end())
		{
			iterator->second.push(new_job);

			logger::handle().write(logging_level::parameter, fmt::format(L"push new job: priority - {}", (int)new_job->priority()));

			notification(new_job->priority());

			return;
		}
		
		queue<shared_ptr<job>> queue;
		queue.push(new_job);

		_jobs.insert({ new_job->priority(), queue });

		logger::handle().write(logging_level::parameter, fmt::format(L"push new job: priority - {}", (int)new_job->priority()));

		notification(new_job->priority());
	}

	shared_ptr<job> job_pool::pop(const priorities& priority, const vector<priorities>& others)
	{
		scoped_lock<mutex> guard(_mutex);

		auto iterator = _jobs.find(priority);
		if (iterator != _jobs.end() && !iterator->second.empty())
		{
			shared_ptr<job> temp = iterator->second.front();
			iterator->second.pop();

			logger::handle().write(logging_level::parameter, fmt::format(L"pop a job: priority - {}", (int)temp->priority()));

			return temp;
		}

		for (auto& other : others)
		{
			auto iterator2 = _jobs.find(other);
			if (iterator2 == _jobs.end() || iterator2->second.empty())
			{
				continue;
			}

			shared_ptr<job> temp = iterator2->second.front();
			iterator2->second.pop();

			logger::handle().write(logging_level::parameter, fmt::format(L"pop a job: priority - {}", (int)temp->priority()));

			return temp;
		}

		size_t count = 0;
		for (auto& target : _jobs)
		{
			count += target.second.size();
		}

		if(count == 0)
		{
			notification(priorities::none);
		}

		return nullptr;
	}

	bool job_pool::contain(const priorities& priority, const vector<priorities>& others)
	{
		scoped_lock<mutex> guard(_mutex);

		auto iterator = _jobs.find(priority);
		if (iterator != _jobs.end() && !iterator->second.empty())
		{
			shared_ptr<job> temp = iterator->second.front();

			return temp != nullptr;
		}

		for (auto& other : others)
		{
			auto iterator2 = _jobs.find(priority);
			if (iterator2 == _jobs.end() || iterator2->second.empty())
			{
				continue;
			}

			shared_ptr<job> temp = iterator2->second.front();

			return temp != nullptr;
		}

		return false;
	}

	bool job_pool::append_notification(const wstring& id, const function<void(const priorities&)>& notification)
	{
		auto target = _notifications.find(id);
		if(target != _notifications.end())
		{
			return false;
		}

		_notifications.insert({ id, notification });

		return true;
	}
	
	bool job_pool::remove_notification(const wstring& id)
	{
		auto target = _notifications.find(id);
		if(target == _notifications.end())
		{
			return false;
		}

		_notifications.erase(target);

		return true;
	}

	void job_pool::notification(const priorities& priority)
	{
		for (auto& notification : _notifications)
		{
			if (notification.second == nullptr)
			{
				continue;
			}

			notification.second(priority);
		}
	}
}