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

#pragma once

#include "typed_job_queue.h"
#include "typed_lockfree_job_queue.h"
#include <atomic>
#include <chrono>
#include <thread>

namespace typed_thread_pool_module
{
	/**
	 * @class adaptive_typed_job_queue_t
	 * @brief Adaptive priority queue that switches between mutex-based and lock-free implementations
	 * 
	 * This queue monitors performance metrics and automatically switches between
	 * a traditional mutex-based typed queue and a lock-free typed queue based on
	 * contention levels and performance characteristics.
	 */
	template <typename job_type = job_types>
	class adaptive_typed_job_queue_t : public typed_job_queue_t<job_type>
	{
	public:
		/**
		 * @brief Queue implementation strategy
		 */
		enum class queue_strategy
		{
			AUTO_DETECT,      ///< Automatically detect best strategy
			FORCE_LEGACY,     ///< Always use mutex-based queue
			FORCE_LOCKFREE,   ///< Always use lock-free queue
			ADAPTIVE          ///< Switch based on runtime metrics
		};
		
		/**
		 * @brief Constructor
		 * @param initial_strategy Initial queue strategy
		 */
		explicit adaptive_typed_job_queue_t(queue_strategy initial_strategy = queue_strategy::AUTO_DETECT);
		
		/**
		 * @brief Destructor
		 */
		virtual ~adaptive_typed_job_queue_t();
		
		// typed_job_queue_t interface implementation
		[[nodiscard]] auto enqueue(std::unique_ptr<thread_module::job>&& value) -> thread_module::result_void override;
		[[nodiscard]] auto enqueue(std::unique_ptr<typed_job_t<job_type>>&& value) -> thread_module::result_void;
		[[nodiscard]] auto enqueue_batch(std::vector<std::unique_ptr<thread_module::job>>&& jobs) -> thread_module::result_void override;
		[[nodiscard]] auto dequeue() -> thread_module::result<std::unique_ptr<thread_module::job>> override;
		[[nodiscard]] auto dequeue_batch() -> std::deque<std::unique_ptr<thread_module::job>> override;
		[[nodiscard]] auto dequeue(const std::vector<job_type>& types) -> thread_module::result<std::unique_ptr<typed_job_t<job_type>>>;
		auto clear() -> void override;
		[[nodiscard]] auto empty(const std::vector<job_type>& types) const -> bool;
		[[nodiscard]] auto size(const std::vector<job_type>& types) const -> std::size_t;
		[[nodiscard]] auto to_string() const -> std::string override;
		
		/**
		 * @brief Force evaluation and potential switch of queue implementation
		 */
		auto evaluate_and_switch() -> void;
		
		/**
		 * @brief Get current queue type
		 * @return Current queue implementation type
		 */
		[[nodiscard]] auto get_current_type() const -> std::string;
		
		/**
		 * @brief Performance metrics structure
		 */
		struct performance_metrics
		{
			uint64_t operation_count{0};
			uint64_t total_latency_ns{0};
			uint64_t contention_count{0};
			uint64_t switch_count{0};
			std::chrono::steady_clock::time_point last_evaluation;
			
			[[nodiscard]] auto get_average_latency_ns() const -> double
			{
				if (operation_count == 0) return 0.0;
				return static_cast<double>(total_latency_ns) / static_cast<double>(operation_count);
			}
			
			[[nodiscard]] auto get_contention_ratio() const -> double
			{
				if (operation_count == 0) return 0.0;
				return static_cast<double>(contention_count) / static_cast<double>(operation_count);
			}
		};
		
		/**
		 * @brief Get performance metrics
		 * @return Current performance metrics
		 */
		[[nodiscard]] auto get_metrics() const -> performance_metrics;
		
	private:
		enum class queue_type
		{
			LEGACY_MUTEX,
			LOCKFREE,
			HYBRID
		};
		
		// Queue implementations
		std::unique_ptr<typed_job_queue_t<job_type>> legacy_queue_;
		std::unique_ptr<typed_lockfree_job_queue_t<job_type>> lockfree_queue_;
		
		// Current state
		std::atomic<queue_type> current_type_;
		queue_strategy strategy_;
		
		// Performance monitoring
		mutable struct {
			std::atomic<uint64_t> operation_count{0};
			std::atomic<uint64_t> total_latency_ns{0};
			std::atomic<uint64_t> contention_count{0};
			std::atomic<uint64_t> switch_count{0};
			std::chrono::steady_clock::time_point last_evaluation;
		} metrics_;
		
		std::unique_ptr<std::thread> monitor_thread_;
		std::atomic<bool> stop_monitor_{false};
		
		// Configuration
		static constexpr auto EVALUATION_INTERVAL = std::chrono::seconds(5);
		static constexpr double CONTENTION_THRESHOLD_HIGH = 0.1;
		static constexpr double CONTENTION_THRESHOLD_LOW = 0.05;
		static constexpr double LATENCY_THRESHOLD_HIGH_NS = 1000.0;
		static constexpr double LATENCY_THRESHOLD_LOW_NS = 500.0;
		static constexpr size_t MIN_OPERATIONS_FOR_SWITCH = 1000;
		
		// Internal methods
		auto initialize_strategy() -> void;
		auto start_performance_monitor() -> void;
		auto stop_performance_monitor() -> void;
		auto monitor_performance() -> void;
		auto should_switch_to_lockfree() const -> bool;
		auto should_switch_to_legacy() const -> bool;
		auto migrate_to_lockfree() -> void;
		auto migrate_to_legacy() -> void;
		auto update_metrics(std::chrono::nanoseconds duration, bool had_contention = false) -> void;
		
		// Helper to get current implementation
		[[nodiscard]] auto get_current_impl() -> typed_job_queue_t<job_type>*;
		[[nodiscard]] auto get_current_impl() const -> const typed_job_queue_t<job_type>*;
	};
	
	/**
	 * @brief Factory function to create appropriate typed job queue
	 * @param strategy Queue selection strategy
	 * @param max_threads Maximum number of threads (for lockfree queue)
	 * @return Shared pointer to created queue
	 */
	template <typename job_type = job_types>
	[[nodiscard]] auto create_typed_job_queue(
		typename adaptive_typed_job_queue_t<job_type>::queue_strategy strategy = 
			adaptive_typed_job_queue_t<job_type>::queue_strategy::AUTO_DETECT,
		size_t max_threads = 128
	) -> std::shared_ptr<typed_job_queue_t<job_type>>;

} // namespace typed_thread_pool_module

#include "thread_system_core/typed_thread_pool/scheduling/adaptive_typed_job_queue.tpp"