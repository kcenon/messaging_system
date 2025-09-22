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

#include "adaptive_typed_job_queue.h"
#include "thread_system_core/utilities/core/formatter.h"

namespace typed_thread_pool_module
{
	template <typename job_type>
	adaptive_typed_job_queue_t<job_type>::adaptive_typed_job_queue_t(
		queue_strategy initial_strategy)
		: legacy_queue_(std::make_unique<typed_job_queue_t<job_type>>())
		, lockfree_queue_(std::make_unique<typed_lockfree_job_queue_t<job_type>>())
		, strategy_(initial_strategy)
	{
		initialize_strategy();
		
		if (strategy_ == queue_strategy::ADAPTIVE)
		{
			start_performance_monitor();
		}
		
		metrics_.last_evaluation = std::chrono::steady_clock::now();
	}
	
	template <typename job_type>
	adaptive_typed_job_queue_t<job_type>::~adaptive_typed_job_queue_t()
	{
		stop_performance_monitor();
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::initialize_strategy() -> void
	{
		switch (strategy_)
		{
			case queue_strategy::FORCE_LEGACY:
				current_type_.store(queue_type::LEGACY_MUTEX);
				break;
				
			case queue_strategy::FORCE_LOCKFREE:
				current_type_.store(queue_type::LOCKFREE);
				break;
				
			case queue_strategy::AUTO_DETECT:
			case queue_strategy::ADAPTIVE:
			{
				// Start with mutex-based and let it adapt
				current_type_.store(queue_type::LEGACY_MUTEX);
				break;
			}
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::start_performance_monitor() -> void
	{
		stop_monitor_.store(false);
		monitor_thread_ = std::make_unique<std::thread>(&adaptive_typed_job_queue_t::monitor_performance, this);
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::stop_performance_monitor() -> void
	{
		stop_monitor_.store(true);
		if (monitor_thread_ && monitor_thread_->joinable())
		{
			monitor_thread_->join();
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::monitor_performance() -> void
	{
		while (!stop_monitor_.load())
		{
			std::this_thread::sleep_for(EVALUATION_INTERVAL);
			
			if (stop_monitor_.load()) break;
			
			evaluate_and_switch();
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::evaluate_and_switch() -> void
	{
		auto metrics = get_metrics();
		
		if (metrics.operation_count < MIN_OPERATIONS_FOR_SWITCH)
		{
			return; // Not enough data
		}
		
		if (current_type_.load() == queue_type::LEGACY_MUTEX && should_switch_to_lockfree())
		{
			migrate_to_lockfree();
		}
		else if (current_type_.load() == queue_type::LOCKFREE && should_switch_to_legacy())
		{
			migrate_to_legacy();
		}
		
		// Reset metrics after evaluation
		metrics_.operation_count.store(0);
		metrics_.total_latency_ns.store(0);
		metrics_.contention_count.store(0);
		metrics_.last_evaluation = std::chrono::steady_clock::now();
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::should_switch_to_lockfree() const -> bool
	{
		auto metrics = get_metrics();
		auto avg_latency = metrics.get_average_latency_ns();
		auto contention_ratio = metrics.get_contention_ratio();
		
		return (contention_ratio > CONTENTION_THRESHOLD_HIGH || 
				avg_latency > LATENCY_THRESHOLD_HIGH_NS);
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::should_switch_to_legacy() const -> bool
	{
		auto metrics = get_metrics();
		auto avg_latency = metrics.get_average_latency_ns();
		auto contention_ratio = metrics.get_contention_ratio();
		
		return (contention_ratio < CONTENTION_THRESHOLD_LOW && 
				avg_latency < LATENCY_THRESHOLD_LOW_NS);
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::migrate_to_lockfree() -> void
	{
		// Migration is complex and should be done carefully
		// For now, we just switch without migrating existing items
		current_type_.store(queue_type::LOCKFREE);
		metrics_.switch_count.fetch_add(1, std::memory_order_relaxed);
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::migrate_to_legacy() -> void
	{
		// Migration is complex and should be done carefully
		// For now, we just switch without migrating existing items
		current_type_.store(queue_type::LEGACY_MUTEX);
		metrics_.switch_count.fetch_add(1, std::memory_order_relaxed);
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::update_metrics(
		std::chrono::nanoseconds duration, bool had_contention) -> void
	{
		metrics_.operation_count.fetch_add(1, std::memory_order_relaxed);
		metrics_.total_latency_ns.fetch_add(duration.count(), std::memory_order_relaxed);
		if (had_contention)
		{
			metrics_.contention_count.fetch_add(1, std::memory_order_relaxed);
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::get_current_impl() -> typed_job_queue_t<job_type>*
	{
		return legacy_queue_.get();
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::get_current_impl() const -> const typed_job_queue_t<job_type>*
	{
		return legacy_queue_.get();
	}
	
	// Interface implementations
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::enqueue(std::unique_ptr<thread_module::job>&& value) 
		-> thread_module::result_void
	{
		auto start = std::chrono::high_resolution_clock::now();
		thread_module::result_void result;
		
		if (current_type_.load() == queue_type::LOCKFREE)
		{
			result = lockfree_queue_->enqueue(std::move(value));
		}
		else
		{
			result = legacy_queue_->enqueue(std::move(value));
		}
		
		auto duration = std::chrono::high_resolution_clock::now() - start;
		update_metrics(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
		
		return result;
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::enqueue(std::unique_ptr<typed_job_t<job_type>>&& value) 
		-> thread_module::result_void
	{
		auto start = std::chrono::high_resolution_clock::now();
		thread_module::result_void result;
		
		if (current_type_.load() == queue_type::LOCKFREE)
		{
			result = lockfree_queue_->enqueue(std::move(value));
		}
		else
		{
			result = legacy_queue_->enqueue(std::move(value));
		}
		
		auto duration = std::chrono::high_resolution_clock::now() - start;
		update_metrics(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
		
		return result;
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::enqueue_batch(
		std::vector<std::unique_ptr<thread_module::job>>&& jobs) -> thread_module::result_void
	{
		auto start = std::chrono::high_resolution_clock::now();
		thread_module::result_void result;
		
		if (current_type_.load() == queue_type::LOCKFREE)
		{
			result = lockfree_queue_->enqueue_batch(std::move(jobs));
		}
		else
		{
			result = legacy_queue_->enqueue_batch(std::move(jobs));
		}
		
		auto duration = std::chrono::high_resolution_clock::now() - start;
		update_metrics(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
		
		return result;
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::dequeue() 
		-> thread_module::result<std::unique_ptr<thread_module::job>>
	{
		auto start = std::chrono::high_resolution_clock::now();
		
		auto result = (current_type_.load() == queue_type::LOCKFREE)
			? lockfree_queue_->dequeue()
			: legacy_queue_->dequeue();
		
		auto duration = std::chrono::high_resolution_clock::now() - start;
		update_metrics(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
		
		return result;
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::dequeue_batch() 
		-> std::deque<std::unique_ptr<thread_module::job>>
	{
		if (current_type_.load() == queue_type::LOCKFREE)
		{
			return lockfree_queue_->dequeue_batch();
		}
		else
		{
			return legacy_queue_->dequeue_batch();
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::dequeue(const std::vector<job_type>& types) 
		-> thread_module::result<std::unique_ptr<typed_job_t<job_type>>>
	{
		auto start = std::chrono::high_resolution_clock::now();
		
		auto result = (current_type_.load() == queue_type::LOCKFREE)
			? lockfree_queue_->dequeue(types)
			: legacy_queue_->dequeue(types);
		
		auto duration = std::chrono::high_resolution_clock::now() - start;
		update_metrics(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
		
		return result;
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::clear() -> void
	{
		if (current_type_.load() == queue_type::LOCKFREE)
		{
			lockfree_queue_->clear();
		}
		else
		{
			legacy_queue_->clear();
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::empty(const std::vector<job_type>& types) const -> bool
	{
		if (current_type_.load() == queue_type::LOCKFREE)
		{
			return lockfree_queue_->empty(types);
		}
		else
		{
			return legacy_queue_->empty(types);
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::size(const std::vector<job_type>& types) const -> std::size_t
	{
		if (current_type_.load() == queue_type::LOCKFREE)
		{
			// Lock-free queue doesn't support size with types, sum individual sizes
			std::size_t total = 0;
			for (const auto& type : types)
			{
				total += lockfree_queue_->size(type);
			}
			return total;
		}
		else
		{
			// Legacy queue's size() returns total size, not filtered by types
			// This is an approximation - we can't filter by types in legacy queue
			return legacy_queue_->size();
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::to_string() const -> std::string
	{
		auto metrics = get_metrics();
		return utility_module::formatter::format(
			"adaptive_typed_job_queue[type={}, ops={}, avg_latency={:.2f}ns, contention={:.2%}, switches={}]",
			get_current_type(),
			metrics.operation_count,
			metrics.get_average_latency_ns(),
			metrics.get_contention_ratio(),
			metrics.switch_count
		);
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::get_current_type() const -> std::string
	{
		switch (current_type_.load())
		{
			case queue_type::LEGACY_MUTEX:
				return "mutex_based";
			case queue_type::LOCKFREE:
				return "lockfree";
			default:
				return "hybrid";
		}
	}
	
	template <typename job_type>
	auto adaptive_typed_job_queue_t<job_type>::get_metrics() const -> performance_metrics
	{
		return {
			metrics_.operation_count.load(),
			metrics_.total_latency_ns.load(),
			metrics_.contention_count.load(),
			metrics_.switch_count.load(),
			metrics_.last_evaluation
		};
	}
	
	// Factory function
	template <typename job_type>
	auto create_typed_job_queue(
		typename adaptive_typed_job_queue_t<job_type>::queue_strategy strategy,
		size_t max_threads) -> std::shared_ptr<typed_job_queue_t<job_type>>
	{
		return std::make_shared<adaptive_typed_job_queue_t<job_type>>(strategy);
	}
	
} // namespace typed_thread_pool_module