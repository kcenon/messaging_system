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

#include "thread_worker.h"

#include "../../interfaces/logger_interface.h"
#include "../../utilities/core/formatter.h"

#include <thread>

using namespace utility_module;

/**
 * @file thread_worker.cpp
 * @brief Implementation of worker thread for thread pool execution.
 *
 * This file contains the implementation of the thread_worker class, which represents
 * individual worker threads within a thread pool. Each worker thread continuously
 * processes jobs from a shared job queue until the thread pool is shut down.
 * 
 * Key Features:
 * - Continuous job processing from shared queue
 * - Optional performance timing and logging
 * - Graceful shutdown handling
 * - Error propagation and logging
 * - Job queue association management
 * 
 * Worker Lifecycle:
 * 1. Worker is created and associated with a job queue
 * 2. Worker thread starts and enters continuous work loop
 * 3. Worker dequeues and executes jobs until queue is empty/stopped
 * 4. Worker logs execution results and timing information
 * 5. Worker shuts down gracefully when work is complete
 * 
 * Performance Characteristics:
 * - Low latency job pickup (typically <1µs)
 * - Efficient queue polling with minimal CPU overhead
 * - Optional timing measurements for performance analysis
 */

namespace thread_pool_module
{
	// Initialize static member
	std::atomic<std::size_t> thread_worker::next_worker_id_{0};
	/**
	 * @brief Constructs a worker thread with optional timing capabilities.
	 * 
	 * Implementation details:
	 * - Inherits from thread_base to get thread management functionality
	 * - Sets descriptive name "thread_worker" for debugging and logging
	 * - Initializes timing flag for optional performance measurement
	 * - Job queue is not set initially (must be set before starting work)
	 * - Stores thread context for logging and monitoring
	 * 
	 * Performance Timing:
	 * - When enabled, measures execution time for each job
	 * - Uses high_resolution_clock for precise measurements
	 * - Minimal overhead when disabled (single boolean check)
	 * 
	 * @param use_time_tag If true, enables timing measurements for job execution
	 * @param context Thread context providing logging and monitoring services
	 */
	thread_worker::thread_worker(const bool& use_time_tag, const thread_context& context)
		: thread_base("thread_worker"), 
		  worker_id_(next_worker_id_.fetch_add(1)),
		  use_time_tag_(use_time_tag), 
		  job_queue_(nullptr),
		  context_(context)
	{
	}

	/**
	 * @brief Destroys the worker thread.
	 * 
	 * Implementation details:
	 * - Base class destructor handles thread shutdown
	 * - No manual cleanup required due to RAII design
	 * - Shared pointer to job queue is automatically released
	 */
	thread_worker::~thread_worker(void) {}

	/**
	 * @brief Associates this worker with a job queue for processing.
	 * 
	 * Implementation details:
	 * - Stores shared pointer to enable job dequeuing
	 * - Must be called before starting the worker thread
	 * - Thread-safe operation (atomic pointer assignment)
	 * - Multiple workers can share the same job queue
	 * 
	 * Queue Relationship:
	 * - Worker holds shared ownership of the queue
	 * - Queue lifetime is managed by thread pool
	 * - Worker can safely access queue throughout its lifetime
	 * 
	 * @param job_queue Shared pointer to the job queue for this worker
	 */
	auto thread_worker::set_job_queue(std::shared_ptr<job_queue> job_queue) -> void
	{
		job_queue_ = job_queue;
	}

	/**
	 * @brief Sets the thread context for this worker.
	 * 
	 * Implementation details:
	 * - Stores the context for use in logging and monitoring
	 * - Should be called before starting the worker thread
	 * - Context provides access to optional services
	 * 
	 * @param context Thread context with logging and monitoring services
	 */
	auto thread_worker::set_context(const thread_context& context) -> void
	{
		context_ = context;
	}

	/**
	 * @brief Gets the thread context for this worker.
	 * 
	 * @return The thread context providing access to logging and monitoring services
	 */
	auto thread_worker::get_context(void) const -> const thread_context&
	{
		return context_;
	}

	/**
	 * @brief Determines if the worker should continue processing jobs.
	 * 
	 * Implementation details:
	 * - Used by thread_base to control the work loop
	 * - Returns false if no job queue is set (prevents infinite loop)
	 * - Returns false if job queue is empty (allows worker to exit)
	 * - Thread-safe operation (job_queue methods are thread-safe)
	 * 
	 * Work Loop Control:
	 * - Called repeatedly by thread_base work loop
	 * - Worker exits gracefully when this returns false
	 * - Prevents CPU spinning when no work is available
	 * 
	 * @return true if worker should continue processing, false to exit
	 */
	auto thread_worker::should_continue_work() const -> bool
	{
		if (job_queue_ == nullptr)
		{
			return false;
		}

		return !job_queue_->empty();
	}

	/**
	 * @brief Executes a single work cycle by processing one job from the queue.
	 * 
	 * Implementation details:
	 * - Dequeues one job from the associated job queue
	 * - Validates job pointer before execution
	 * - Optionally measures execution timing for performance analysis
	 * - Associates job with queue for potential re-submission
	 * - Logs execution results with appropriate detail level
	 * 
	 * Job Processing Workflow:
	 * 1. Validate job queue availability
	 * 2. Attempt to dequeue next job
	 * 3. Handle empty queue or stopped queue scenarios
	 * 4. Validate dequeued job pointer
	 * 5. Optionally record start time for measurement
	 * 6. Associate job with queue for context
	 * 7. Execute job's do_work() method
	 * 8. Handle execution errors with detailed logging
	 * 9. Log successful completion with timing info if enabled
	 * 
	 * Error Handling:
	 * - Missing job queue: Returns resource allocation error
	 * - Empty queue: Returns success if queue is stopped, error otherwise
	 * - Null job pointer: Returns job invalid error
	 * - Job execution failure: Returns execution failed error with details
	 * 
	 * Performance Measurements:
	 * - High-resolution timing when use_time_tag_ is enabled
	 * - Nanosecond precision for accurate profiling
	 * - Minimal overhead when timing is disabled
	 * 
	 * Logging Behavior:
	 * - Standard success message when timing is disabled
	 * - Timestamped success message when timing is enabled
	 * - Error details are propagated up the call stack
	 * 
	 * @return result_void indicating success or detailed error information
	 */
	auto thread_worker::do_work() -> result_void
	{
		// Validate that job queue is available for processing
		if (job_queue_ == nullptr)
		{
			return error{error_code::resource_allocation_failed, "there is no job_queue"};
		}

		// Attempt to dequeue the next job from the shared queue
		auto dequeue_result = job_queue_->dequeue();
		if (!dequeue_result.has_value())
		{
			// Handle empty queue scenarios based on queue state
			if (!job_queue_->is_stopped())
			{
				// Queue is empty but still active - this is an error condition
				return error{error_code::queue_empty, 
					formatter::format("error dequeue job: {}", dequeue_result.get_error().to_string())};
			}

			// Queue is stopped and empty - normal shutdown condition
			return result_void{};
		}

		// Extract the job from the result and validate it
		auto current_job = std::move(dequeue_result.value());
		if (current_job == nullptr)
		{
			return error{error_code::job_invalid, "error executing job: nullptr"};
		}

		// Initialize timing measurement if performance monitoring is enabled
		std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>>
			started_time_point = std::nullopt;
		if (use_time_tag_)
		{
			started_time_point = std::chrono::high_resolution_clock::now();
		}

		// Associate the job with its source queue for potential re-submission
		current_job->set_job_queue(job_queue_);

		// Mark worker as processing a job
		is_processing_job_.store(true);

		// Execute the job's work method and capture the result
		auto work_result = current_job->do_work();

		// Mark worker as idle after job completion (regardless of success/failure)
		is_processing_job_.store(false);
		if (work_result.has_error())
		{
			return error{error_code::job_execution_failed,
				formatter::format("error executing job: {}", work_result.get_error().to_string())};
		}

		// Log successful job completion based on timing configuration
		if (!started_time_point.has_value())
		{
			// Standard logging without timing information
			context_.log(log_level::debug,
			            formatter::format("job executed successfully: {} on thread_worker",
			                            current_job->get_name()));
		}
		else
		{
			// Enhanced logging with execution timing information
			auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
				end_time - started_time_point.value()).count();
			
			context_.log(log_level::debug,
			            formatter::format("job executed successfully: {} on thread_worker ({}ns)",
			                            current_job->get_name(), duration));
			
			// Update worker metrics if monitoring is available
			if (context_.monitoring())
			{
				monitoring_interface::worker_metrics metrics;
				metrics.jobs_processed = 1;
				metrics.total_processing_time_ns = static_cast<std::uint64_t>(duration);
				metrics.timestamp = std::chrono::steady_clock::now();
				// Use proper worker ID instead of thread hash
				context_.update_worker_metrics(worker_id_, metrics);
			}
		}

		return result_void{};
	}

	std::size_t thread_worker::get_worker_id() const
	{
		return worker_id_;
	}

	bool thread_worker::is_idle() const
	{
		// Worker is idle if it's not currently processing a job and the thread is running
		return !is_processing_job_.load() && is_running();
	}
} // namespace thread_pool_module