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

#include "thread_pool.h"

#include "logging.h"
#include "job_pool.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace threads
{
	using namespace logging;

	thread_pool::thread_pool(const vector<shared_ptr<thread_worker>>& workers)
		: _workers(workers), _job_pool(make_shared<job_pool>())
	{
		_job_pool->append_notification(L"thread_pool", bind(&thread_pool::notification, this, placeholders::_1));
	}

	thread_pool::~thread_pool(void)
	{
		stop(true);

		_workers.clear();
		_job_pool.reset();
	}

	void thread_pool::start(void)
	{
		scoped_lock<mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->start();
		}
	}

	void thread_pool::append(shared_ptr<thread_worker> worker, const bool& start)
	{
		unique_lock<mutex> unique(_mutex);
		
		worker->set_job_pool(_job_pool);
		_workers.push_back(worker);

		logger::handle().write(logging_level::parameter, fmt::format(L"appended new worker: priority - {}", (int)worker->priority()));

		unique.unlock();

		if (start)
		{
			worker->start();
		}
	}

	void thread_pool::stop(const bool& ignore_contained_job)
	{
		scoped_lock<mutex> guard(_mutex);

		if (_job_pool != nullptr)
		{
			_job_pool->set_push_lock(true);

			if (ignore_contained_job)
			{
				_job_pool.reset();
			}
		}

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->stop();
		}
	}

	void thread_pool::push(shared_ptr<job> job)
	{
		if (_job_pool == nullptr)
		{
			return;
		}

		_job_pool->push(job);
	}

	void thread_pool::notification(const priorities& priority)
	{
		scoped_lock<mutex> guard(_mutex);

		for (auto& worker : _workers)
		{
			if (worker == nullptr)
			{
				continue;
			}

			worker->notification(priority);
		}
	}
}