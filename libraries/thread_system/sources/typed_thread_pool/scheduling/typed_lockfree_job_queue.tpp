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

#include "typed_lockfree_job_queue.h"

namespace typed_thread_pool_module
{
	template <typename job_type>
	typed_lockfree_job_queue_t<job_type>::typed_lockfree_job_queue_t(size_t max_threads)
		: job_queue()
		, max_threads_(max_threads)
		, typed_queues_()
		, priority_order_()
	{
		// Initialize with default priority order if job_type has a get_priority_order method
		if constexpr (requires { job_type::get_priority_order(); }) {
			priority_order_ = job_type::get_priority_order();
		}
	}
	
	template <typename job_type>
	typed_lockfree_job_queue_t<job_type>::~typed_lockfree_job_queue_t()
	{
		clear();
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::get_or_create_queue(const job_type& type) 
		-> thread_module::lockfree_job_queue*
	{
		{
			// Try read lock first for better performance
			std::shared_lock<std::shared_mutex> read_lock(queues_mutex_);
			auto it = typed_queues_.find(type);
			if (it != typed_queues_.end()) {
				return it->second.get();
			}
		}
		
		// Need to create a new queue
		std::unique_lock<std::shared_mutex> write_lock(queues_mutex_);
		
		// Double-check after acquiring write lock
		auto it = typed_queues_.find(type);
		if (it != typed_queues_.end()) {
			return it->second.get();
		}
		
		// Create new queue
		auto queue = std::make_unique<thread_module::lockfree_job_queue>(max_threads_);
		auto* queue_ptr = queue.get();
		typed_queues_[type] = std::move(queue);
		
		// Update priority order if needed
		if (should_update_priority_order()) {
			update_priority_order();
		}
		
		return queue_ptr;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::get_queue(const job_type& type) const 
		-> thread_module::lockfree_job_queue*
	{
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		auto it = typed_queues_.find(type);
		return (it != typed_queues_.end()) ? it->second.get() : nullptr;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::update_priority_order() -> void
	{
		std::unique_lock<std::shared_mutex> lock(priority_mutex_);
		
		// If job_type has a natural ordering, use it
		if constexpr (requires { job_type::get_priority_order(); }) {
			priority_order_ = job_type::get_priority_order();
		} else {
			// Otherwise, just collect all existing types
			priority_order_.clear();
			for (const auto& [type, _] : typed_queues_) {
				priority_order_.push_back(type);
			}
			
			// Sort by natural comparison if available
			if constexpr (std::totally_ordered<job_type>) {
				std::sort(priority_order_.begin(), priority_order_.end());
			}
		}
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::stop() -> void
	{
		// Lock-free queues don't need explicit stopping
		// This is just for interface compatibility
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::should_update_priority_order() const -> bool
	{
		std::shared_lock<std::shared_mutex> lock(priority_mutex_);
		return priority_order_.size() != typed_queues_.size();
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::enqueue(std::unique_ptr<typed_job_t<job_type>>&& value)
		-> thread_module::result_void
	{
		if (!value) {
			return thread_module::error{thread_module::error_code::invalid_argument, 
				"Cannot enqueue null typed job"};
		}
		
		if (stop_.load(std::memory_order_acquire)) {
			return thread_module::error{thread_module::error_code::queue_stopped, 
				"Queue is stopped"};
		}
		
		// Get the appropriate queue for this job type
		auto* queue = get_or_create_queue(value->priority());
		if (!queue) {
			return thread_module::error{thread_module::error_code::resource_allocation_failed,
				"Failed to get or create queue for job type"};
		}
		
		// Move to base class pointer and enqueue
		std::unique_ptr<thread_module::job> base_job = std::move(value);
		auto result = queue->enqueue(std::move(base_job));
		
		// Notify if successful
		if (!result && notify_.load(std::memory_order_acquire)) {
			condition_.notify_one();
		}
		
		return result;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::enqueue(std::unique_ptr<thread_module::job>&& value)
		-> thread_module::result_void
	{
		if (!value) {
			return thread_module::error{thread_module::error_code::invalid_argument,
				"Cannot enqueue null job"};
		}
		
		// Try to cast to typed job
		auto* typed_ptr = dynamic_cast<typed_job_t<job_type>*>(value.get());
		if (!typed_ptr) {
			return thread_module::error{thread_module::error_code::job_invalid,
				"Job is not a typed_job_t"};
		}
		
		// Release ownership and re-wrap as typed job
		value.release();
		return enqueue(std::unique_ptr<typed_job_t<job_type>>(typed_ptr));
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::enqueue_batch(
		std::vector<std::unique_ptr<typed_job_t<job_type>>>&& jobs)
		-> thread_module::result_void
	{
		if (jobs.empty()) {
			return thread_module::error{thread_module::error_code::invalid_argument,
				"Cannot enqueue empty batch"};
		}
		
		if (stop_.load(std::memory_order_acquire)) {
			return thread_module::error{thread_module::error_code::queue_stopped,
				"Queue is stopped"};
		}
		
		// Group jobs by type for efficient batch insertion
		std::unordered_map<job_type, std::vector<std::unique_ptr<thread_module::job>>> grouped_jobs;
		
		for (auto& job : jobs) {
			if (!job) {
				return thread_module::error{thread_module::error_code::invalid_argument,
					"Null job in batch"};
			}
			
			auto type = job->priority();
			// Move typed job to base job pointer
			std::unique_ptr<thread_module::job> base_job = std::move(job);
			grouped_jobs[type].push_back(std::move(base_job));
		}
		
		// Enqueue each group
		for (auto& [type, type_jobs] : grouped_jobs) {
			auto* queue = get_or_create_queue(type);
			if (!queue) {
				return thread_module::error{thread_module::error_code::resource_allocation_failed,
					"Failed to get or create queue for job type"};
			}
			
			auto result = queue->enqueue_batch(std::move(type_jobs));
			if (!result) {
				return result;
			}
		}
		
		// Notify if successful
		if (notify_.load(std::memory_order_acquire)) {
			condition_.notify_all();
		}
		
		return {};
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::enqueue_batch(
		std::vector<std::unique_ptr<thread_module::job>>&& jobs)
		-> thread_module::result_void
	{
		// Convert to typed jobs
		std::vector<std::unique_ptr<typed_job_t<job_type>>> typed_jobs;
		typed_jobs.reserve(jobs.size());
		
		for (auto& job : jobs) {
			if (!job) {
				return thread_module::error{thread_module::error_code::invalid_argument,
					"Null job in batch"};
			}
			
			auto* typed_ptr = dynamic_cast<typed_job_t<job_type>*>(job.get());
			if (!typed_ptr) {
				return thread_module::error{thread_module::error_code::job_invalid,
					"Job is not a typed_job_t"};
			}
			
			job.release();
			typed_jobs.push_back(std::unique_ptr<typed_job_t<job_type>>(typed_ptr));
		}
		
		// Convert to base job vector
		std::vector<std::unique_ptr<thread_module::job>> base_jobs;
		base_jobs.reserve(typed_jobs.size());
		for (auto& typed_job : typed_jobs) {
			base_jobs.push_back(std::move(typed_job));
		}
		
		// Call the base class version
		return job_queue::enqueue_batch(std::move(base_jobs));
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::dequeue()
		-> thread_module::result<std::unique_ptr<thread_module::job>>
	{
		if (stop_.load(std::memory_order_acquire)) {
			return thread_module::error{thread_module::error_code::queue_stopped,
				"Queue is stopped"};
		}
		
		// Get priority order
		std::vector<job_type> types_to_check;
		{
			std::shared_lock<std::shared_mutex> lock(priority_mutex_);
			types_to_check = priority_order_;
		}
		
		// If no priority order, check all queues
		if (types_to_check.empty()) {
			std::shared_lock<std::shared_mutex> lock(queues_mutex_);
			for (const auto& [type, _] : typed_queues_) {
				types_to_check.push_back(type);
			}
		}
		
		// Try to dequeue from each type in priority order
		for (const auto& type : types_to_check) {
			auto* queue = get_queue(type);
			if (queue && !queue->empty()) {
				auto result = queue->try_dequeue();
				if (result.has_value()) {
					// Track type switching
					auto prev_type = last_dequeue_type_.exchange(type, std::memory_order_relaxed);
					if (prev_type != type) {
						type_switch_count_.fetch_add(1, std::memory_order_relaxed);
					}
					
					return result;
				}
			}
		}
		
		// No jobs available - wait if needed
		if (!stop_.load(std::memory_order_acquire)) {
			std::unique_lock<std::mutex> lock(mutex_);
			condition_.wait(lock, [this]() { 
				return !empty() || stop_.load(std::memory_order_acquire); 
			});
			
			// Try again after waking up
			if (!stop_.load(std::memory_order_acquire) && !empty()) {
				return dequeue();
			}
		}
		
		return thread_module::error{thread_module::error_code::queue_empty,
			"No jobs available to dequeue"};
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::dequeue(const job_type& type)
		-> thread_module::result<std::unique_ptr<typed_job_t<job_type>>>
	{
		if (stop_.load(std::memory_order_acquire)) {
			return thread_module::error{thread_module::error_code::queue_stopped,
				"Queue is stopped"};
		}
		
		auto* queue = get_queue(type);
		if (!queue) {
			return thread_module::error{thread_module::error_code::queue_empty,
				"No queue exists for specified type"};
		}
		
		auto result = queue->dequeue();
		if (result.has_value()) {
			// Cast back to typed job
			auto* typed_ptr = dynamic_cast<typed_job_t<job_type>*>(result.value().get());
			if (typed_ptr) {
				result.value().release();
				return std::unique_ptr<typed_job_t<job_type>>(typed_ptr);
			}
		}
		
		return thread_module::error{thread_module::error_code::queue_empty,
			"No jobs of specified type available"};
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::dequeue(utility_module::span<const job_type> types)
		-> thread_module::result<std::unique_ptr<typed_job_t<job_type>>>
	{
		if (stop_.load(std::memory_order_acquire)) {
			return thread_module::error{thread_module::error_code::queue_stopped,
				"Queue is stopped"};
		}
		
		// Try each type in the provided order
		for (const auto& type : types) {
			auto result = dequeue(type);
			if (result.has_value()) {
				return result;
			}
		}
		
		return thread_module::error{thread_module::error_code::queue_empty,
			"No jobs of specified types available"};
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::dequeue_batch()
		-> std::deque<std::unique_ptr<thread_module::job>>
	{
		std::deque<std::unique_ptr<thread_module::job>> all_jobs;
		
		// Get all queues
		std::vector<std::pair<job_type, thread_module::lockfree_job_queue*>> queues;
		{
			std::shared_lock<std::shared_mutex> lock(queues_mutex_);
			for (const auto& [type, queue] : typed_queues_) {
				queues.emplace_back(type, queue.get());
			}
		}
		
		// Dequeue from each queue
		for (const auto& [type, queue] : queues) {
			auto batch = queue->dequeue_batch();
			for (auto& job : batch) {
				all_jobs.push_back(std::move(job));
			}
		}
		
		return all_jobs;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::clear() -> void
	{
		std::unique_lock<std::shared_mutex> lock(queues_mutex_);
		
		for (auto& [_, queue] : typed_queues_) {
			queue->clear();
		}
		
		condition_.notify_all();
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::empty() const -> bool
	{
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		
		for (const auto& [_, queue] : typed_queues_) {
			if (!queue->empty()) {
				return false;
			}
		}
		
		return true;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::empty(const std::vector<job_type>& types) const -> bool
	{
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		
		for (const auto& type : types) {
			auto it = typed_queues_.find(type);
			if (it != typed_queues_.end() && it->second && !it->second->empty()) {
				return false;
			}
		}
		
		return true;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::size() const -> std::size_t
	{
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		
		std::size_t total = 0;
		for (const auto& [_, queue] : typed_queues_) {
			total += queue->size();
		}
		
		return total;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::size(const job_type& type) const -> std::size_t
	{
		auto* queue = get_queue(type);
		return queue ? queue->size() : 0;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::get_sizes() const 
		-> std::unordered_map<job_type, std::size_t>
	{
		std::unordered_map<job_type, std::size_t> sizes;
		
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		for (const auto& [type, queue] : typed_queues_) {
			sizes[type] = queue->size();
		}
		
		return sizes;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::to_string() const -> std::string
	{
		auto sizes = get_sizes();
		auto stats = get_typed_statistics();
		
		std::string result = utility_module::formatter::format(
			"typed_lockfree_job_queue[total_size={}, types={}, type_switches={}",
			size(), typed_queues_.size(), stats.type_switch_count
		);
		
		// Add per-type information
		for (const auto& [type, size] : sizes) {
			result += utility_module::formatter::format(", {}={}", type, size);
		}
		
		result += "]";
		return result;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::get_typed_statistics() const 
		-> typed_queue_statistics_t<job_type>
	{
		typed_queue_statistics_t<job_type> stats;
		
		// Aggregate statistics from all queues
		{
			std::shared_lock<std::shared_mutex> lock(queues_mutex_);
			for (const auto& [type, queue] : typed_queues_) {
				if (queue) {
					auto queue_stats = queue->get_statistics();
					stats.total_enqueues += queue_stats.enqueue_count;
					stats.total_dequeues += queue_stats.dequeue_count;
					stats.enqueue_latency_ns += queue_stats.total_enqueue_time;
					stats.dequeue_latency_ns += queue_stats.total_dequeue_time;
					
					// Track per-type statistics
					stats.per_type_enqueues[type] = queue_stats.enqueue_count;
					stats.per_type_dequeues[type] = queue_stats.dequeue_count;
				}
			}
		}
		
		// Add typed-specific stats
		stats.type_switch_count = type_switch_count_.load(std::memory_order_relaxed);
		
		return stats;
	}
	
	template <typename job_type>
	auto typed_lockfree_job_queue_t<job_type>::reset_statistics() -> void
	{
		// Reset typed-specific statistics
		type_switch_count_.store(0, std::memory_order_relaxed);
		
		// Reset per-queue statistics
		std::shared_lock<std::shared_mutex> lock(queues_mutex_);
		for (const auto& [_, queue] : typed_queues_) {
			queue->reset_statistics();
		}
	}

} // namespace typed_thread_pool_module