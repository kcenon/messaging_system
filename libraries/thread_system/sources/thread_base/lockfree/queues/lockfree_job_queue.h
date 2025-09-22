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

#include "../../jobs/job_queue.h"
#include "../memory/hazard_pointer.h"
#include "../memory/node_pool.h"
#include "../../utilities/core/formatter.h"

#include <atomic>
#include <memory>
#include <chrono>
#include <thread>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier
#endif

namespace thread_module
{
	/**
	 * @class lockfree_job_queue
	 * @brief High-performance lock-free Multiple Producer Multiple Consumer job queue
	 * 
	 * This implementation is based on the Michael & Scott algorithm with
	 * optimizations for the Thread System including:
	 * - Hazard pointers for safe memory reclamation
	 * - Node pooling for reduced allocation overhead
	 * - Batch operations for improved throughput
	 * - Performance statistics collection
	 * 
	 * @note This queue is designed to be a drop-in replacement for job_queue
	 *       with significantly better performance under high contention.
	 */
	class lockfree_job_queue : public job_queue
	{
	public:
		/**
		 * @brief Performance statistics structure (non-atomic version for returning)
		 */
		struct queue_statistics
		{
			uint64_t enqueue_count{0};
			uint64_t dequeue_count{0};
			uint64_t enqueue_batch_count{0};
			uint64_t dequeue_batch_count{0};
			uint64_t total_enqueue_time{0};
			uint64_t total_dequeue_time{0};
			uint64_t retry_count{0};
			uint64_t current_size{0};
			
			[[nodiscard]] auto get_average_enqueue_latency_ns() const -> double
			{
				if (enqueue_count == 0) return 0.0;
				return static_cast<double>(total_enqueue_time) / static_cast<double>(enqueue_count);
			}
			
			[[nodiscard]] auto get_average_dequeue_latency_ns() const -> double
			{
				if (dequeue_count == 0) return 0.0;
				return static_cast<double>(total_dequeue_time) / static_cast<double>(dequeue_count);
			}
		};
		
		/**
		 * @brief Internal atomic statistics structure
		 */
		struct atomic_statistics
		{
			std::atomic<uint64_t> enqueue_count{0};
			std::atomic<uint64_t> dequeue_count{0};
			std::atomic<uint64_t> enqueue_batch_count{0};
			std::atomic<uint64_t> dequeue_batch_count{0};
			std::atomic<uint64_t> total_enqueue_time{0};
			std::atomic<uint64_t> total_dequeue_time{0};
			std::atomic<uint64_t> retry_count{0};
			std::atomic<uint64_t> current_size{0};
		};

		/**
		 * @brief Constructor
		 * @param max_threads Maximum number of threads that will access the queue
		 */
		explicit lockfree_job_queue(size_t max_threads = 128);
		
		/**
		 * @brief Destructor
		 */
		virtual ~lockfree_job_queue();

		// Delete copy operations
		lockfree_job_queue(const lockfree_job_queue&) = delete;
		lockfree_job_queue& operator=(const lockfree_job_queue&) = delete;
		
		// Delete move operations for now (can be implemented later if needed)
		lockfree_job_queue(lockfree_job_queue&&) = delete;
		lockfree_job_queue& operator=(lockfree_job_queue&&) = delete;

		/**
		 * @brief Enqueue a single job
		 * @param value The job to enqueue
		 * @return Success or error result
		 */
		[[nodiscard]] auto enqueue(std::unique_ptr<job>&& value) -> result_void override;
		
		/**
		 * @brief Enqueue multiple jobs as a batch
		 * @param jobs Vector of jobs to enqueue
		 * @return Success or error result
		 */
		[[nodiscard]] auto enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs) -> result_void override;
		
		/**
		 * @brief Dequeue a single job
		 * @return The dequeued job or error
		 */
		[[nodiscard]] auto dequeue() -> result<std::unique_ptr<job>> override;
		
		/**
		 * @brief Dequeue all available jobs
		 * @return Deque containing all dequeued jobs
		 */
		[[nodiscard]] auto dequeue_batch() -> std::deque<std::unique_ptr<job>> override;
		
		/**
		 * @brief Try to enqueue without blocking
		 * @param value The job to enqueue
		 * @return Success or error result
		 */
		[[nodiscard]] auto try_enqueue(std::unique_ptr<job>&& value) -> result_void;
		
		/**
		 * @brief Try to dequeue without blocking
		 * @return The dequeued job or error
		 */
		[[nodiscard]] auto try_dequeue() -> result<std::unique_ptr<job>>;
		
		/**
		 * @brief Clear all jobs from the queue
		 */
		auto clear() -> void override;
		
		/**
		 * @brief Check if the queue is empty
		 * @return true if empty, false otherwise
		 */
		[[nodiscard]] auto empty() const -> bool;
		
		/**
		 * @brief Get the current size of the queue
		 * @return Number of jobs in the queue
		 */
		[[nodiscard]] auto size() const -> std::size_t;
		
		/**
		 * @brief Get performance statistics
		 * @return Current statistics
		 */
		[[nodiscard]] auto get_statistics() const -> queue_statistics;
		
		/**
		 * @brief Reset performance statistics
		 */
		auto reset_statistics() -> void;
		
		/**
		 * @brief Get string representation of the queue
		 * @return String describing the queue state
		 */
		[[nodiscard]] auto to_string() const -> std::string override;

	private:
		// Forward declaration
		struct Node;
		
		// Type aliases
		using job_ptr = std::unique_ptr<job>;
		using node_ptr = Node*;
		
		/**
		 * @brief Node structure for the queue
		 */
		struct alignas(64) Node
		{
			std::atomic<job_ptr*> data{nullptr};
			std::atomic<node_ptr> next{nullptr};
			std::atomic<uint64_t> version{0};
			
			Node() = default;
			~Node() = default;
			
			// Helper methods
			auto set_data(job_ptr* new_data, std::memory_order order = std::memory_order_release) -> bool
			{
				job_ptr* expected = nullptr;
				return data.compare_exchange_strong(expected, new_data, order);
			}
			
			auto get_data(std::memory_order order = std::memory_order_acquire) const -> job_ptr*
			{
				return data.load(order);
			}
			
			auto clear_data(std::memory_order order = std::memory_order_release) -> void
			{
				data.store(nullptr, order);
			}
		};
		
		// Queue head and tail pointers
		alignas(64) std::atomic<node_ptr> head_;
		alignas(64) std::atomic<node_ptr> tail_;
		
		// Memory management
		std::unique_ptr<node_pool<Node>> node_pool_;
		std::unique_ptr<hazard_pointer_manager> hp_manager_;
		
		// Statistics
		mutable atomic_statistics stats_;
		
		// Configuration
		static constexpr size_t MAX_BATCH_SIZE = 1024;
		static constexpr size_t RETRY_THRESHOLD = 16;
		static constexpr size_t MAX_TOTAL_RETRIES = 1000;  // Maximum total retries before giving up
		
		// Internal helper methods
		auto allocate_node() -> node_ptr;
		auto deallocate_node(node_ptr node) -> void;
		auto retire_node(node_ptr node) -> void;
		
		// Enqueue/dequeue helpers
		auto enqueue_impl(job_ptr* data_storage) -> result_void;
		auto dequeue_impl() -> result<job_ptr>;
		
		// Statistics helpers
		auto record_enqueue_time(std::chrono::nanoseconds duration) -> void;
		auto record_dequeue_time(std::chrono::nanoseconds duration) -> void;
		auto increment_retry_count() -> void;
	};

} // namespace thread_module

#ifdef _MSC_VER
#pragma warning(pop)
#endif