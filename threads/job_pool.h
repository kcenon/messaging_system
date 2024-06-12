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

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <vector>

namespace threads
{
	class job;
	class job_pool : public std::enable_shared_from_this<job_pool>
	{
	public:
		job_pool(const std::string& title);
		~job_pool(void);

	public:
		std::shared_ptr<job_pool> get_ptr(void);

	public:
		bool push(std::shared_ptr<job> new_job);
		std::shared_ptr<job> pop(const priorities& priority,
								 const std::vector<priorities>& others = {});
		bool contain(const priorities& priority,
					 const std::vector<priorities>& others = {});
		void set_push_lock(const bool& lock);

	public:
		bool append_notification(
			const std::string& id,
			const std::function<void(const priorities&)>& notification);
		bool remove_notification(const std::string& id);

	public:
		void check_empty(void);

	private:
		void notification(const priorities& priority);

	private:
		std::mutex mutex_;
		bool push_lock_;
		std::string title_;
		std::map<priorities, std::queue<std::shared_ptr<job>>> jobs_;
		std::map<std::string, std::function<void(const priorities&)>>
			notifications_;
	};
} // namespace threads
