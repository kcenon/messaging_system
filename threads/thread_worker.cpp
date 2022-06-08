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

#include "thread_worker.h"

#include "job.h"
#include "job_pool.h"

#include "logging.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace threads
{
	using namespace logging;

	thread_worker::thread_worker(const priorities& priority, const vector<priorities>& others)
		: _priority(priority), _others(others)
	{
	}

	thread_worker::~thread_worker(void)
	{
		stop();
	}

	void thread_worker::set_job_pool(shared_ptr<job_pool> job_pool)
	{
		_job_pool = job_pool;
	}

	void thread_worker::start(void)
	{
		stop();

		_thread_stop.store(false);
		_thread = make_shared<thread>(&thread_worker::run, this);
	}

	void thread_worker::stop(void)
	{
		if (_thread == nullptr)
		{
			return;
		}

		_thread_stop.store(true);

		_condition.notify_one();

		if (_thread != nullptr)
		{
			if (_thread->joinable())
			{
				_thread->join();
			}
			_thread.reset();
		}
	}

	const priorities thread_worker::priority(void)
	{
		return _priority;
	}

	void thread_worker::notification(const priorities& priority)
	{
		if (_priority == priority)
		{
			_condition.notify_one();

			return;
		}

		auto target = find(_others.begin(), _others.end(), priority);
		if (target == _others.end())
		{
			return;
		}

		_condition.notify_one();
	}

	void thread_worker::run(void)
	{
		logger::handle().write(logging_level::sequence, fmt::format(L"start working thread: priority - {}", (int)_priority));

		while (!_thread_stop.load())
		{
			unique_lock<mutex> unique(_mutex);
			_condition.wait(unique, [this] { return check_condition(); });

			auto jobs = _job_pool.lock();
			if (jobs == nullptr)
			{
				continue;
			}

			if (_thread_stop.load())
			{
				break;
			}

			shared_ptr<job> current_job = jobs->pop(_priority, _others);
			jobs.reset();
			unique.unlock();

			if (current_job == nullptr && _thread_stop.load())
			{
				break;
			}

			working(current_job);
		}

		logger::handle().write(logging_level::sequence, fmt::format(L"stop working thread: priority - {}", (int)_priority));
	}

	void thread_worker::working(shared_ptr<job> current_job)
	{
		if (current_job == nullptr)
		{
			return;
		}

		current_job->work(_priority);
	}

	bool thread_worker::check_condition(void)
	{
		if (_thread_stop.load())
		{
			return true;
		}

		auto jobs = _job_pool.lock();
		if (jobs == nullptr)
		{
			return false;
		}

		bool result = jobs->contain(_priority, _others);
		jobs.reset();

		return result;
	}
}