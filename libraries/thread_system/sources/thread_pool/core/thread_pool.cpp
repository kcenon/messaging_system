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

#include "thread_pool.h"
#include "../../thread_base/lockfree/queues/adaptive_job_queue.h"

#include "../../interfaces/logger_interface.h"
#include "../../utilities/core/formatter.h"

using namespace utility_module;

/**
 * @file thread_pool.cpp
 * @brief Implementation of the thread pool class for managing multiple worker threads.
 *
 * This file contains the implementation of the thread_pool class, which coordinates
 * multiple worker threads processing jobs from a shared queue. The pool supports
 * adaptive queue strategies for optimal performance under varying load conditions.
 */

namespace thread_pool_module
{
	// Initialize static member
	std::atomic<std::uint32_t> thread_pool::next_pool_instance_id_{0};
	/**
	 * @brief Constructs a thread pool with adaptive job queue.
	 * 
	 * Implementation details:
	 * - Initializes with provided thread title for identification
	 * - Creates adaptive job queue that automatically optimizes based on contention
	 * - Pool starts in stopped state (start_pool_ = false)
	 * - No workers are initially assigned (workers_ is empty)
	 * - Stores thread context for logging and monitoring
	 * 
	 * Adaptive Queue Strategy:
	 * - ADAPTIVE mode automatically switches between mutex and lock-free implementations
	 * - Provides optimal performance across different contention levels
	 * - Eliminates need for manual queue strategy selection
	 * 
	 * @param thread_title Descriptive name for this thread pool instance
	 * @param context Thread context providing logging and monitoring services
	 */
	thread_pool::thread_pool(const std::string& thread_title, const thread_context& context)
		: thread_title_(thread_title), 
		  pool_instance_id_(next_pool_instance_id_.fetch_add(1)),
		  start_pool_(false), 
		  job_queue_(thread_module::create_job_queue(thread_module::adaptive_job_queue::queue_strategy::ADAPTIVE)),
		  context_(context)
	{
		// Report initial pool registration if monitoring is available
		if (context_.monitoring())
		{
			monitoring_interface::thread_pool_metrics initial_metrics;
			initial_metrics.pool_name = thread_title_;
			initial_metrics.pool_instance_id = pool_instance_id_;
			initial_metrics.worker_threads = 0;
			initial_metrics.timestamp = std::chrono::steady_clock::now();
			context_.update_thread_pool_metrics(thread_title_, pool_instance_id_, initial_metrics);
		}
	}

	/**
	 * @brief Destroys the thread pool, ensuring all workers are stopped.
	 * 
	 * Implementation details:
	 * - Automatically calls stop() to ensure clean shutdown
	 * - Workers will complete current jobs before terminating
	 * - Prevents resource leaks from running threads
	 */
	thread_pool::~thread_pool() { stop(); }

	/**
	 * @brief Returns a shared pointer to this thread pool instance.
	 * 
	 * Implementation details:
	 * - Uses std::enable_shared_from_this for safe shared_ptr creation
	 * - Required for passing pool reference to workers or other components
	 * - Ensures proper lifetime management in multi-threaded environment
	 * 
	 * @return Shared pointer to this thread_pool
	 */
	auto thread_pool::get_ptr(void) -> std::shared_ptr<thread_pool>
	{
		return this->shared_from_this();
	}

	/**
	 * @brief Starts all worker threads in the pool.
	 * 
	 * Implementation details:
	 * - Validates that workers have been added to the pool
	 * - Starts each worker thread individually
	 * - If any worker fails to start, stops all workers and returns error
	 * - Sets start_pool_ flag to true on successful startup
	 * - Workers begin processing jobs from the shared queue immediately
	 * 
	 * Startup Sequence:
	 * 1. Check that workers exist
	 * 2. Start each worker sequentially
	 * 3. On failure: stop all workers and return error message
	 * 4. On success: mark pool as started
	 * 
	 * Error Handling:
	 * - All-or-nothing startup (if one fails, all stop)
	 * - Provides detailed error message from failed worker
	 * - Ensures consistent pool state (either all running or all stopped)
	 * 
	 * @return std::nullopt on success, error message on failure
	 */
	auto thread_pool::start(void) -> std::optional<std::string>
	{
		// Validate that workers have been added
		if (workers_.empty())
		{
			return "No workers to start";
		}

		// Attempt to start each worker
		for (auto& worker : workers_)
		{
			auto start_result = worker->start();
			if (start_result.has_error())
			{
				// If any worker fails, stop all and return error
				stop();
				return start_result.get_error().to_string();
			}
		}

		// Mark pool as successfully started
		start_pool_.store(true);

		return std::nullopt;  // Success
	}

	/**
	 * @brief Returns the shared job queue used by all workers.
	 * 
	 * Implementation details:
	 * - Provides access to the adaptive job queue for external job submission
	 * - Queue is shared among all workers for load balancing
	 * - Adaptive queue automatically optimizes based on contention patterns
	 * 
	 * @return Shared pointer to the job queue
	 */
	auto thread_pool::get_job_queue(void) -> std::shared_ptr<job_queue> { return job_queue_; }

	/**
	 * @brief Adds a single job to the thread pool for processing.
	 * 
	 * Implementation details:
	 * - Validates job pointer before submission
	 * - Validates queue availability
	 * - Delegates to adaptive job queue for optimal scheduling
	 * - Job will be processed by next available worker thread
	 * 
	 * Queue Behavior:
	 * - Adaptive queue automatically selects best strategy (mutex/lock-free)
	 * - Jobs are processed in FIFO order within the selected strategy
	 * - Workers are notified when jobs become available
	 * 
	 * Thread Safety:
	 * - Safe to call from multiple threads simultaneously
	 * - Adaptive queue handles contention efficiently
	 * 
	 * @param job Unique pointer to job (ownership transferred)
	 * @return std::nullopt on success, error message on failure
	 */
	auto thread_pool::enqueue(std::unique_ptr<job>&& job) -> std::optional<std::string>
	{
		// Validate inputs
		if (job == nullptr)
		{
			return "Job is null";
		}

		if (job_queue_ == nullptr)
		{
			return "Job queue is null";
		}

		// Delegate to adaptive queue for optimal processing
		auto enqueue_result = job_queue_->enqueue(std::move(job));
		if (enqueue_result.has_error())
		{
			return enqueue_result.get_error().to_string();
		}

		return std::nullopt;  // Success
	}

	auto thread_pool::enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs)
		-> std::optional<std::string>
	{
		if (jobs.empty())
		{
			return "Jobs are empty";
		}

		if (job_queue_ == nullptr)
		{
			return "Job queue is null";
		}

		auto enqueue_result = job_queue_->enqueue_batch(std::move(jobs));
		if (enqueue_result.has_error())
		{
			return enqueue_result.get_error().to_string();
		}

		return std::nullopt;
	}

	auto thread_pool::enqueue(std::unique_ptr<thread_worker>&& worker) -> std::optional<std::string>
	{
		if (worker == nullptr)
		{
			return "Worker is null";
		}

		if (job_queue_ == nullptr)
		{
			return "Job queue is null";
		}

		worker->set_job_queue(job_queue_);
		worker->set_context(context_);

		if (start_pool_.load())
		{
			auto start_result = worker->start();
			if (start_result.has_error())
			{
				stop();
				return start_result.get_error().to_string();
			}
		}

		workers_.emplace_back(std::move(worker));

		return std::nullopt;
	}

	auto thread_pool::enqueue_batch(std::vector<std::unique_ptr<thread_worker>>&& workers)
		-> std::optional<std::string>
	{
		if (workers.empty())
		{
			return "Workers are empty";
		}

		if (job_queue_ == nullptr)
		{
			return "Job queue is null";
		}

		for (auto& worker : workers)
		{
			worker->set_job_queue(job_queue_);
			worker->set_context(context_);

			if (start_pool_.load())
			{
				auto start_result = worker->start();
				if (start_result.has_error())
				{
					stop();
					return start_result.get_error().to_string();
				}
			}

			workers_.emplace_back(std::move(worker));
		}

		return std::nullopt;
	}

	auto thread_pool::stop(const bool& immediately_stop) -> void
	{
		if (!start_pool_.load())
		{
			return;
		}

		if (job_queue_ != nullptr)
		{
			job_queue_->stop_waiting_dequeue();

			if (immediately_stop)
			{
				job_queue_->clear();
			}
		}

		for (auto& worker : workers_)
		{
			auto stop_result = worker->stop();
			if (stop_result.has_error())
			{
				context_.log(log_level::error, 
				            formatter::format("error stopping worker: {}",
				                            stop_result.get_error().to_string()));
			}
		}

		start_pool_.store(false);
	}

	auto thread_pool::to_string(void) const -> std::string
	{
		std::string format_string;

		formatter::format_to(std::back_inserter(format_string), "{} is {},\n", thread_title_,
							 start_pool_.load() ? "running" : "stopped");
		formatter::format_to(std::back_inserter(format_string), "\tjob_queue: {}\n\n",
							 (job_queue_ != nullptr ? job_queue_->to_string() : "nullptr"));
		formatter::format_to(std::back_inserter(format_string), "\tworkers: {}\n", workers_.size());
		for (const auto& worker : workers_)
		{
			formatter::format_to(std::back_inserter(format_string), "\t{}\n", worker->to_string());
		}

		return format_string;
	}

	auto thread_pool::get_context(void) const -> const thread_context&
	{
		return context_;
	}

	std::uint32_t thread_pool::get_pool_instance_id() const
	{
		return pool_instance_id_;
	}

	void thread_pool::report_metrics()
	{
		if (!context_.monitoring())
		{
			return;
		}

		monitoring_interface::thread_pool_metrics metrics;
		metrics.pool_name = thread_title_;
		metrics.pool_instance_id = pool_instance_id_;
		metrics.worker_threads = workers_.size();
		metrics.idle_threads = get_idle_worker_count();
		
		if (job_queue_)
		{
			metrics.jobs_pending = job_queue_->size();
		}
		
		metrics.timestamp = std::chrono::steady_clock::now();
		
		// Report metrics with pool identification
		context_.update_thread_pool_metrics(thread_title_, pool_instance_id_, metrics);
	}

	std::size_t thread_pool::get_idle_worker_count() const
	{
		std::size_t idle_count = 0;

		for (const auto& worker : workers_)
		{
			if (worker && worker->is_idle())
			{
				++idle_count;
			}
		}

		return idle_count;
	}
} // namespace thread_pool_module