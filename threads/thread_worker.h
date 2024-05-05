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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace threads
{
	class job;
	class job_pool;
	class thread_worker : public std::enable_shared_from_this<thread_worker>
	{
	public:
		thread_worker(const priorities& priority,
					  const std::vector<priorities>& others = {});
		~thread_worker(void);

	public:
		std::shared_ptr<thread_worker> get_ptr(void);

	public:
		void set_job_pool(std::shared_ptr<job_pool> job_pool);
		void set_worker_notification(
			const std::function<void(const std::wstring&, const bool&)>&
				notification);

	public:
		void start(void);
		void stop(void);

	public:
		auto guid(void) const -> std::wstring;
		auto priority(void) const -> priorities;
		auto priority(const priorities& value) -> void;

	protected:
		auto run(void) -> void;

	protected:
		virtual auto working(std::shared_ptr<job> current_job) -> void;

	protected:
		auto append_notification(const priorities& priority) -> void;
		auto check_condition(void) -> bool;

	private:
		bool _thread_stop;
		std::function<void(const std::wstring&, const bool&)> _worker_condition;

	private:
		priorities _priority;
		std::vector<priorities> _others;
		std::weak_ptr<job_pool> _job_pool;

	private:
		std::mutex _mutex;
		std::wstring _guid;
		std::unique_ptr<std::thread> _thread;
		std::condition_variable _condition;
	};
} // namespace threads
