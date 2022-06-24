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

#include <map>
#include <mutex>
#include <queue>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

using namespace std;

namespace threads
{
	class job;
	class job_pool : public enable_shared_from_this<job_pool>
	{
	public:
		job_pool(void);
		~job_pool(void);

	public:
		shared_ptr<job_pool> get_ptr(void);

	public:
		bool push(shared_ptr<job> new_job);
		shared_ptr<job> pop(const priorities& priority, const vector<priorities>& others = {});
		bool contain(const priorities& priority, const vector<priorities>& others = {});
		void set_push_lock(const bool& lock);

	public:
		bool append_notification(const wstring& id, const function<void(const priorities&)>& notification);
		bool remove_notification(const wstring& id);

	private:
		size_t count(void);
		void notification(const priorities& priority);

	private:
		mutex _mutex;
		bool _push_lock;
		map<priorities, queue<shared_ptr<job>>> _jobs;
		map<wstring, function<void(const priorities&)>> _notifications;
	};
}
