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

#include "typed_job_queue.h"

namespace typed_thread_pool_module
{
	template <typename job_type>
	typed_job_queue_t<job_type>::typed_job_queue_t(void) : job_queue(), lockfree_queues_()
	{
	}

	template <typename job_type>
	typed_job_queue_t<job_type>::~typed_job_queue_t(void)
	{
		// Clear all queues before destruction
		clear();
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::get_or_create_queue(const job_type& type) -> lockfree_job_queue*
	{
		{
			// Try read lock first for better performance
			std::shared_lock<std::shared_mutex> read_lock(queues_mutex_);
			auto it = lockfree_queues_.find(type);
			if (it != lockfree_queues_.end()) {
				return it->second.get();
			}
		}
		
		// Need to create a new queue
		std::unique_lock<std::shared_mutex> write_lock(queues_mutex_);
		// Double check after acquiring write lock
		auto it = lockfree_queues_.find(type);
		if (it != lockfree_queues_.end()) {
			return it->second.get();
		}
		
		// Create new queue
		auto queue = std::make_unique<lockfree_job_queue>();
		auto* queue_ptr = queue.get();
		lockfree_queues_[type] = std::move(queue);
		return queue_ptr;
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::enqueue(std::unique_ptr<job>&& value)
		-> result_void
	{
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		auto typed_job_ptr = dynamic_cast<typed_job_t<job_type>*>(value.get());

		if (!typed_job_ptr)
		{
			return error{error_code::job_invalid, "Enqueued job is not a typed_job"};
		}

		// Take ownership
		value.release();
		return enqueue(std::unique_ptr<typed_job_t<job_type>>(typed_job_ptr));
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::enqueue_batch_ref(std::vector<std::unique_ptr<job>>& jobs)
		-> result_void
	{
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		std::vector<std::unique_ptr<typed_job_t<job_type>>> typed_jobs;
		typed_jobs.reserve(jobs.size());

		for (auto& job : jobs)
		{
			auto typed_job_ptr = dynamic_cast<typed_job_t<job_type>*>(job.get());
			if (!typed_job_ptr)
			{
				return error{error_code::job_invalid, "Enqueued job is not a typed_job"};
			}

			// Take ownership
			job.release();
			typed_jobs.push_back(
				std::unique_ptr<typed_job_t<job_type>>(typed_job_ptr));
		}

		// Clear the original vector since we took ownership
		jobs.clear();

		return enqueue_batch(std::move(typed_jobs));
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs)
		-> result_void
	{
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		std::vector<std::unique_ptr<typed_job_t<job_type>>> typed_jobs;
		typed_jobs.reserve(jobs.size());

		for (auto& job : jobs)
		{
			auto typed_job_ptr = dynamic_cast<typed_job_t<job_type>*>(job.release());
			if (!typed_job_ptr)
			{
				return error{error_code::job_invalid, "Enqueued job is not a typed_job"};
			}

			typed_jobs.push_back(
				std::unique_ptr<typed_job_t<job_type>>(typed_job_ptr));
		}

		return enqueue_batch(std::move(typed_jobs));
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::enqueue(
		std::unique_ptr<typed_job_t<job_type>>&& value) -> result_void
	{
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		if (value == nullptr)
		{
			return error{error_code::job_invalid, "Cannot enqueue null job"};
		}

		auto job_priority = value->priority();
		auto* queue = get_or_create_queue(job_priority);
		
		// Convert typed_job to base job for enqueue
		std::unique_ptr<job> base_job = std::move(value);
		auto result = queue->enqueue(std::move(base_job));
		
		if (result) {
			condition_.notify_one();
		}
		
		return result;
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::enqueue_batch(
		std::vector<std::unique_ptr<typed_job_t<job_type>>>&& jobs)
		-> result_void
	{
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		if (jobs.empty())
		{
			return error{error_code::job_invalid, "Cannot enqueue empty batch"};
		}

		// Group jobs by priority for efficient batch enqueue
		std::unordered_map<job_type, std::vector<std::unique_ptr<job>>> grouped_jobs;
		
		for (auto& typed_job : jobs)
		{
			if (!typed_job) continue;
			
			auto priority = typed_job->priority();
			grouped_jobs[priority].push_back(std::move(typed_job));
		}
		
		// Enqueue each group
		for (auto& [priority, job_group] : grouped_jobs)
		{
			auto* queue = get_or_create_queue(priority);
			auto result = queue->enqueue_batch(std::move(job_group));
			if (!result) {
				return result;
			}
		}

		condition_.notify_all();
		return result_void{};
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::dequeue()
		-> result<std::unique_ptr<job>>
	{
		return error{error_code::queue_empty, "Dequeue operation without specified types is "
								   "not supported in typed_job_queue"};
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::dequeue(const std::vector<job_type>& types)
		-> result<std::unique_ptr<typed_job_t<job_type>>>
	{
		// First try non-blocking dequeue from all specified types
		for (const auto& priority : types)
		{
			if (auto job = try_dequeue_from_priority(priority))
			{
				return std::move(job.value());
			}
		}

		// If no job found, wait with condition variable
		std::unique_lock<std::mutex> lock(mutex_);
		
		while (!stop_.load())
		{
			// Try again under lock
			for (const auto& priority : types)
			{
				if (auto job = try_dequeue_from_priority(priority))
				{
					return std::move(job.value());
				}
			}
			
			// Wait for notification
			condition_.wait(lock);
		}

		return error{error_code::queue_stopped, "Job queue is stopped"};
	}
	
	template <typename job_type>
	auto typed_job_queue_t<job_type>::dequeue(utility_module::span<const job_type> types)
		-> result<std::unique_ptr<typed_job_t<job_type>>>
	{
		// First try non-blocking dequeue from all specified types
		for (const auto& priority : types)
		{
			if (auto job = try_dequeue_from_priority(priority))
			{
				return std::move(job.value());
			}
		}

		// If no job found, wait with condition variable
		std::unique_lock<std::mutex> lock(mutex_);
		
		while (!stop_.load())
		{
			// Try again under lock
			for (const auto& priority : types)
			{
				if (auto job = try_dequeue_from_priority(priority))
				{
					return std::move(job.value());
				}
			}
			
			// Wait for notification
			condition_.wait(lock);
		}

		return error{error_code::queue_stopped, "Job queue is stopped"};
	}

	template <typename job_type> auto typed_job_queue_t<job_type>::clear() -> void
	{
		std::unique_lock<std::shared_mutex> lock(queues_mutex_);
		
		// Clear all lock-free queues
		for (auto& [type, queue] : lockfree_queues_)
		{
			if (queue) {
				queue->clear();
			}
		}
		
		// Could optionally clear the map itself
		// lockfree_queues_.clear();
		
		condition_.notify_all();
	}
    
	template <typename job_type>
	auto typed_job_queue_t<job_type>::stop() -> void
	{
		stop_.store(true);
		condition_.notify_all();
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::empty(
		const std::vector<job_type>& types) const -> bool
	{
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		return empty_check_without_lock(types);
	}
	
	template <typename job_type>
	auto typed_job_queue_t<job_type>::empty(
		utility_module::span<const job_type> types) const -> bool
	{
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		return empty_check_without_lock(types);
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::to_string(void) const -> std::string
	{
		std::string format_string;
		formatter::format_to(std::back_inserter(format_string), "Type job queue:\n");
		
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		for (const auto& [type, queue] : lockfree_queues_)
		{
			if (queue) {
				formatter::format_to(std::back_inserter(format_string), "\tType: {} -> {} jobs\n",
									 type, queue->size());
			}
		}

		return format_string;
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::empty_check_without_lock(
		const std::vector<job_type>& types) const -> bool
	{
		for (const auto& priority : types)
		{
			auto it = lockfree_queues_.find(priority);
			if (it != lockfree_queues_.end() && it->second && !it->second->empty())
			{
				return false;
			}
		}

		return true;
	}
	
	template <typename job_type>
	auto typed_job_queue_t<job_type>::empty_check_without_lock(
		utility_module::span<const job_type> types) const -> bool
	{
		for (const auto& priority : types)
		{
			auto it = lockfree_queues_.find(priority);
			if (it != lockfree_queues_.end() && it->second && !it->second->empty())
			{
				return false;
			}
		}

		return true;
	}

	template <typename job_type>
	auto typed_job_queue_t<job_type>::try_dequeue_from_priority(
		const job_type& priority)
		-> std::optional<std::unique_ptr<typed_job_t<job_type>>>
	{
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		
		auto it = lockfree_queues_.find(priority);
		if (it == lockfree_queues_.end() || !it->second)
		{
			return std::nullopt;
		}

		auto result = it->second->dequeue();
		if (!result.has_value())
		{
			return std::nullopt;
		}

		// Convert back to typed_job
		auto& base_job = result.value();
		auto* typed_ptr = dynamic_cast<typed_job_t<job_type>*>(base_job.release());
		if (!typed_ptr)
		{
			return std::nullopt;
		}

		return std::make_optional(std::unique_ptr<typed_job_t<job_type>>(typed_ptr));
	}
} // namespace typed_thread_pool_module