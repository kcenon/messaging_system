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

#include "typed_thread_worker.h"

#include "thread_system_core/interfaces/logger_interface.h"
#include "thread_system_core/utilities/core/formatter.h"
#include "typed_job_queue.h"

using namespace utility_module;

namespace typed_thread_pool_module
{
	template <typename job_type>
	typed_thread_worker_t<job_type>::typed_thread_worker_t(
		std::vector<job_type> types, const bool& use_time_tag,
		const thread_context& context)
		: thread_base("typed_thread_worker")
		, job_queue_(nullptr)
		, types_(types)
		, use_time_tag_(use_time_tag)
		, context_(context)
	{
	}

	template <typename job_type>
	typed_thread_worker_t<job_type>::~typed_thread_worker_t(void)
	{
	}

	template <typename job_type>
	auto typed_thread_worker_t<job_type>::set_job_queue(
		std::shared_ptr<typed_job_queue_t<job_type>> job_queue) -> void
	{
		job_queue_ = job_queue;
	}

	template <typename job_type>
	auto typed_thread_worker_t<job_type>::types(void) const
		-> std::vector<job_type>
	{
		return types_;
	}

	template <typename job_type>
	auto typed_thread_worker_t<job_type>::set_context(const thread_context& context) -> void
	{
		context_ = context;
	}

	template <typename job_type>
	auto typed_thread_worker_t<job_type>::get_context(void) const -> const thread_context&
	{
		return context_;
	}

	template <typename job_type>
	auto typed_thread_worker_t<job_type>::should_continue_work() const -> bool
	{
		if (job_queue_ == nullptr)
		{
			return false;
		}

		return !job_queue_->empty(types_);
	}

	template <typename job_type>
	auto typed_thread_worker_t<job_type>::do_work() -> result_void
	{
		if (job_queue_ == nullptr)
		{
			return error{error_code::resource_allocation_failed, "there is no job_queue"};
		}

		auto dequeue_result = job_queue_->dequeue(types_);
		if (!dequeue_result.has_value())
		{
			if (!job_queue_->is_stopped())
			{
				return error{error_code::queue_empty, 
					formatter::format("cannot dequeue job: {}", 
						dequeue_result.get_error().message())};
			}

			return {}; // Success - nothing to do
		}

		auto current_job = std::move(dequeue_result.value());
		if (current_job == nullptr)
		{
			return error{error_code::job_invalid, "error executing job: nullptr"};
		}

		std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>>
			started_time_point = std::nullopt;
		if (use_time_tag_)
		{
			started_time_point = std::chrono::high_resolution_clock::now();
		}

		current_job->set_job_queue(job_queue_);
		auto work_result = current_job->do_work();
		if (work_result.has_error())
		{
			return error{error_code::job_execution_failed, 
				formatter::format("error executing job: {}", 
					work_result.get_error().message())};
		}

		if (!started_time_point.has_value())
		{
			context_.log(log_level::debug, formatter::format(
				"job executed successfully: {}[{}] on typed_thread_worker",
				current_job->get_name(), current_job->priority()));

			return {}; // Success
		}

		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
			end_time - started_time_point.value()).count();
		context_.log(log_level::debug, formatter::format(
			"job executed successfully: {}[{}] on typed_thread_worker ({}ns)",
			current_job->get_name(), current_job->priority(), duration));

		return {}; // Success
	}
} // namespace typed_thread_pool_module