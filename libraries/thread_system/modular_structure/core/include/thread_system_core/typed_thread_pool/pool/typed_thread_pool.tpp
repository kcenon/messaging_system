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

#include "typed_thread_pool.h"
#include "thread_system_core/typed_thread_pool/scheduling/adaptive_typed_job_queue.h"
#include "thread_system_core/utilities/core/formatter.h"

using namespace utility_module;

namespace typed_thread_pool_module
{
	// Template instantiations for commonly used types
	template class typed_thread_pool_t<job_types>;
	
	template <typename job_type>
	auto typed_thread_pool_t<job_type>::enqueue_batch(
		std::vector<std::unique_ptr<typed_job_t<job_type>>>&& jobs)
		-> result_void
	{
		if (jobs.empty())
		{
			return error{error_code::invalid_argument, "cannot enqueue empty batch of jobs"};
		}

		if (job_queue_ == nullptr)
		{
			return error{error_code::resource_allocation_failed, "cannot enqueue batch of jobs due to null job queue"};
		}

		for (auto& job : jobs)
		{
			if (job == nullptr)
			{
				return error{error_code::job_invalid, "cannot enqueue null job"};
			}

			auto enqueue_result = job_queue_->enqueue(std::move(job));
			if (enqueue_result.has_error())
			{
				return enqueue_result.get_error();
			}
		}

		return {};
	}
	
	template <typename job_type>
	typed_thread_pool_t<job_type>::typed_thread_pool_t(
		const std::string& thread_title,
		const thread_context& context)
		: thread_title_(thread_title)
		, job_queue_(create_typed_job_queue<job_type>(
			adaptive_typed_job_queue_t<job_type>::queue_strategy::ADAPTIVE))
		, start_pool_(false)
		, context_(context)
	{
	}

	template <typename job_type>
	typed_thread_pool_t<job_type>::~typed_thread_pool_t(void)
	{
		stop();
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::get_ptr(void) -> std::shared_ptr<typed_thread_pool_t<job_type>>
	{
		return this->shared_from_this();
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::start(void) -> result_void
	{
		if (workers_.empty())
		{
			return error{error_code::thread_start_failure, "no workers to start"};
		}

		for (auto& worker : workers_)
		{
			auto start_result = worker->start();
			if (start_result.has_error())
			{
				stop();
				return start_result.get_error();
			}
		}

		start_pool_.store(true);

		return {};
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::get_job_queue(void)
		-> std::shared_ptr<typed_job_queue_t<job_type>>
	{
		return job_queue_;
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::enqueue(
		std::unique_ptr<typed_job_t<job_type>>&& job) -> result_void
	{
		if (job == nullptr)
		{
			return error{error_code::job_invalid, "cannot enqueue null job"};
		}

		if (job_queue_ == nullptr)
		{
			return error{error_code::resource_allocation_failed, "cannot enqueue job to null job queue"};
		}

		auto enqueue_result = job_queue_->enqueue(std::move(job));
		if (enqueue_result.has_error())
		{
			return enqueue_result.get_error();
		}

		return {};
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::enqueue(
		std::unique_ptr<typed_thread_worker_t<job_type>>&& worker)
		-> result_void
	{
		if (worker == nullptr)
		{
			return error{error_code::invalid_argument, "cannot enqueue null worker"};
		}

		if (job_queue_ == nullptr)
		{
			return error{error_code::resource_allocation_failed, "cannot enqueue worker due to null job queue"};
		}

		worker->set_job_queue(job_queue_);
		worker->set_context(context_);

		if (start_pool_.load())
		{
			auto start_result = worker->start();
			if (start_result.has_error())
			{
				stop();
				return start_result.get_error();
			}
		}

		workers_.emplace_back(std::move(worker));

		return {};
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::enqueue_batch(
		std::vector<std::unique_ptr<typed_thread_worker_t<job_type>>>&& workers)
		-> result_void
	{
		if (workers.empty())
		{
			return error{error_code::invalid_argument, "cannot enqueue empty batch of workers"};
		}

		if (job_queue_ == nullptr)
		{
			return error{error_code::resource_allocation_failed, "cannot enqueue batch of workers due to null job queue"};
		}

		for (auto& worker : workers)
		{
			worker->set_job_queue(job_queue_);
			worker->set_context(context_);
		}

		if (start_pool_.load())
		{
			for (auto& worker : workers)
			{
				auto start_result = worker->start();
				if (start_result.has_error())
				{
					stop();
					return start_result.get_error();
				}
			}
		}

		for (auto& worker : workers)
		{
			workers_.emplace_back(std::move(worker));
		}

		return {};
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::stop(bool clear_queue) -> result_void
	{
		if (job_queue_ != nullptr)
		{
			job_queue_->stop();

			if (clear_queue)
			{
				job_queue_->clear();
			}
		}

		for (auto& worker : workers_)
		{
			auto stop_result = worker->stop();
			if (stop_result.has_error())
			{
				// Error stopping worker - log if context has logger
				context_.log(log_level::error, 
					formatter::format("error stopping worker: {}", stop_result.get_error().message()));
			}
		}

		start_pool_.store(false);
		return {};
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::to_string(void) const -> std::string
	{
		std::string format_string;

		if (job_queue_ == nullptr)
		{
			formatter::format_to(std::back_inserter(format_string),
								 "{} is {},\n\tjob_queue: nullptr\n", thread_title_,
								 start_pool_.load() ? "running" : "stopped");
			formatter::format_to(std::back_inserter(format_string), "\tworkers: {}\n",
								 workers_.size());
			for (const auto& worker : workers_)
			{
				formatter::format_to(std::back_inserter(format_string), "\t{}\n",
									 worker->to_string());
			}

			return format_string;
		}

		formatter::format_to(std::back_inserter(format_string), "{} is {},\n\tjob_queue: {}\n",
							 thread_title_, start_pool_.load() ? "running" : "stopped",
							 job_queue_->to_string());
		formatter::format_to(std::back_inserter(format_string), "\tworkers: {}\n",
							 workers_.size());

		for (const auto& worker : workers_)
		{
			formatter::format_to(std::back_inserter(format_string), "\t{}\n",
								 worker->to_string());
		}

		return format_string;
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::set_job_queue(
		std::shared_ptr<typed_job_queue_t<job_type>> job_queue) -> void
	{
		job_queue_ = job_queue;

		for (auto& worker : workers_)
		{
			worker->set_job_queue(job_queue_);
			worker->set_context(context_);
		}
	}

	template <typename job_type>
	auto typed_thread_pool_t<job_type>::get_context(void) const -> const thread_context&
	{
		return context_;
	}
} // namespace typed_thread_pool_module