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

#pragma once

#include "job_priorities.h"

#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <condition_variable>

using namespace std;

namespace threads
{
	class job;
	class job_pool;
	class thread_worker
	{
	public:
		thread_worker(const priorities& priority, const vector<priorities>& others = {});
		~thread_worker(void);

	public:
		void set_job_pool(shared_ptr<job_pool> job_pool);

	public:
		void start(void);
		void stop(void);

	public:
		const priorities priority(void);
		void notification(const priorities& priority);

	protected:
		void run(void);

	protected:
		virtual void working(shared_ptr<job> current_job);

	protected:
		bool check_condition(void);

	private:
		bool _thread_stop;

	private:
		priorities _priority;
		vector<priorities> _others;
		weak_ptr<job_pool> _job_pool;

	private:
		mutex _mutex;
		wstring _guid;
		shared_ptr<thread> _thread;
		condition_variable _condition;
	};
}

