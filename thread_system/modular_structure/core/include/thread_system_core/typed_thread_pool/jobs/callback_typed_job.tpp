/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

#include "callback_typed_job.h"

#include "thread_system_core/typed_thread_pool/scheduling/typed_job_queue.h"

namespace typed_thread_pool_module
{
	template <typename job_type>
	callback_typed_job_t<job_type>::callback_typed_job_t(
		const std::function<result_void(void)>& callback,
		job_type priority,
		const std::string& name)
		: typed_job_t<job_type>(priority, name), callback_(callback)
	{
	}

	template <typename job_type>
	callback_typed_job_t<job_type>::~callback_typed_job_t(void)
	{
	}

	template <typename job_type>
	auto callback_typed_job_t<job_type>::do_work(void) -> result_void
	{
		if (callback_ == nullptr)
		{
			return error{error_code::job_invalid, "cannot execute job without callback"};
		}

		try
		{
			return callback_();
		}
		catch (const std::exception& e)
		{
			return error{error_code::job_execution_failed, e.what()};
		}
	}
} // namespace typed_thread_pool_module