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

#include <kcenon/thread/core/job_queue.h>

/**
 * @file job_queue.cpp
 * @brief Implementation of thread-safe FIFO job queue for thread system.
 *
 * This file contains the implementation of the job_queue class, which provides
 * a thread-safe container for storing and retrieving job objects in FIFO order.
 * The queue supports both single and batch operations with blocking/non-blocking variants.
 */

namespace kcenon::thread
{
	/**
	 * @brief Constructs a new job_queue with default settings.
	 * 
	 * Implementation details:
	 * - notify_ starts as true (enables condition variable notifications)
	 * - stop_ starts as false (queue is active)
	 * - mutex_ and condition_ are default constructed
	 * - queue_ is an empty deque ready for job storage
	 * 
	 * The queue is immediately ready for use after construction.
	 */
	job_queue::job_queue() : notify_(true), stop_(false), mutex_(), condition_(), queue_() {}

	/**
	 * @brief Destroys the job_queue.
	 * 
	 * Implementation details:
	 * - Any remaining jobs in the queue are automatically destroyed
	 * - No explicit cleanup needed due to RAII design
	 * - Any threads waiting on dequeue() will be unblocked by the stop mechanism
	 */
	job_queue::~job_queue(void) {}

	/**
	 * @brief Returns a shared pointer to this job_queue instance.
	 * 
	 * Implementation details:
	 * - Uses std::enable_shared_from_this to safely create shared_ptr
	 * - Required for passing the queue to multiple threads safely
	 * - Ensures proper lifetime management of the queue object
	 * 
	 * @return Shared pointer to this job_queue
	 */
	auto job_queue::get_ptr(void) -> std::shared_ptr<job_queue> { return shared_from_this(); }

	/**
	 * @brief Checks if the queue has been stopped.
	 * 
	 * Implementation details:
	 * - Uses atomic load for thread-safe access
	 * - When stopped, dequeue operations will fail instead of blocking
	 * - Used to coordinate shutdown between producer and consumer threads
	 * 
	 * @return true if queue is stopped, false if active
	 */
	auto job_queue::is_stopped() const -> bool { return stop_.load(); }

	/**
	 * @brief Controls whether enqueue operations notify waiting threads.
	 * 
	 * Implementation details:
	 * - Uses atomic store for thread-safe modification
	 * - When false, enqueue won't wake up threads waiting in dequeue()
	 * - Useful for batch operations to avoid excessive notifications
	 * - Default is true (notifications enabled)
	 * 
	 * @param notify true to enable notifications, false to disable
	 */
	auto job_queue::set_notify(bool notify) -> void { notify_.store(notify); }

	/**
	 * @brief Adds a single job to the back of the queue.
	 * 
	 * Implementation details:
	 * - First checks if queue is stopped to fail fast
	 * - Validates that the job pointer is not null
	 * - Uses scoped_lock for automatic mutex management
	 * - Moves the job into the queue (zero-copy transfer)
	 * - Optionally notifies one waiting thread if notifications enabled
	 * 
	 * Thread Safety:
	 * - Uses mutex to protect queue modification
	 * - Safe to call concurrently from multiple threads
	 * - notify_one() wakes exactly one waiting thread (fair scheduling)
	 * 
	 * Error Conditions:
	 * - Returns error if queue is stopped
	 * - Returns error if job is null
	 * 
	 * @param value Unique pointer to job (moved into queue)
	 * @return Empty result on success, error on failure
	 */
	auto job_queue::enqueue(std::unique_ptr<job>&& value) -> result_void
	{
		// Early validation: check if queue is still accepting jobs
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		// Validate input: null jobs are not allowed
		if (value == nullptr)
		{
			return error{error_code::invalid_argument, "cannot enqueue null job"};
		}

		// Critical section: modify queue with proper synchronization
		std::scoped_lock<std::mutex> lock(mutex_);

		// Move job into queue (efficient transfer of ownership)
		queue_.push_back(std::move(value));

		// Conditionally notify waiting consumers
		if (notify_)
		{
			condition_.notify_one();  // Wake exactly one waiting thread
		}

		return {};
	}

	/**
	 * @brief Adds multiple jobs to the queue in a single operation.
	 * 
	 * Implementation details:
	 * - Validates queue state and input before any modifications
	 * - Pre-validates all jobs to ensure atomicity (all or nothing)
	 * - Uses single lock acquisition for entire batch (better performance)
	 * - Moves all jobs efficiently without copying
	 * - Only notifies once after all jobs are added (avoids notification spam)
	 * 
	 * Performance Benefits:
	 * - Single mutex acquisition vs multiple for individual enqueues
	 * - Single notification vs notification per job
	 * - Reduced contention for high-throughput scenarios
	 * 
	 * Atomicity:
	 * - Either all jobs are added or none are (on validation failure)
	 * - No partial batches in case of null job detection
	 * 
	 * @param jobs Vector of unique pointers to jobs (moved into queue)
	 * @return Empty result on success, error on failure
	 */
	auto job_queue::enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs) -> result_void
	{
		// Early validation: check if queue is still accepting jobs
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		// Validate batch: empty batches are not allowed
		if (jobs.empty())
		{
			return error{error_code::invalid_argument, "cannot enqueue empty batch"};
		}

		// Pre-validate all jobs before modifying queue (ensures atomicity)
		for (auto& job : jobs)
		{
			if (job == nullptr)
			{
				return error{error_code::invalid_argument, "cannot enqueue null job in batch"};
			}
		}

		// Critical section: add entire batch with single lock acquisition
		std::scoped_lock<std::mutex> lock(mutex_);

		// Move all jobs into queue efficiently
		for (auto& job : jobs)
		{
			queue_.push_back(std::move(job));
		}

		// Single notification for entire batch (performance optimization)
		if (notify_)
		{
			condition_.notify_one();  // Wake one thread to process batch
		}

		return {};
	}

	/**
	 * @brief Removes and returns the first job from the queue (blocking operation).
	 * 
	 * Implementation details:
	 * - Uses unique_lock (required by condition_variable)
	 * - Blocks until a job is available OR queue is stopped
	 * - Returns error if queue is empty after waking up (indicates stop condition)
	 * - Efficiently moves job out of queue (zero-copy transfer)
	 * - Automatically unlocks mutex when function exits
	 * 
	 * Blocking Behavior:
	 * - Waits indefinitely until job available or stop requested
	 * - Uses condition variable for efficient waiting (no busy polling)
	 * - Multiple threads can wait concurrently (fair wake-up via notify_one)
	 * 
	 * Stop Coordination:
	 * - Wakes up when stop_.load() becomes true
	 * - Returns error instead of blocking indefinitely during shutdown
	 * 
	 * Thread Safety:
	 * - Safe for concurrent access with other dequeue/enqueue operations
	 * - Uses proper synchronization primitives
	 * 
	 * @return Unique pointer to job on success, error if queue empty/stopped
	 */
	auto job_queue::dequeue() -> result<std::unique_ptr<job>>
	{
		// Use unique_lock for condition variable operations
		std::unique_lock<std::mutex> lock(mutex_);
		
		// Block until job available OR queue stopped
		condition_.wait(lock, [this]() { return !queue_.empty() || stop_.load(); });

		// Check if we woke up due to stop condition (queue empty after stop)
		if (queue_.empty())
		{
			return error{error_code::queue_empty, "there are no jobs to dequeue"};
		}

		// Efficiently extract first job from queue
		auto value = std::move(queue_.front());
		queue_.pop_front();

		return value;  // Return moved job (caller takes ownership)
	}

	/**
	 * @brief Attempts to dequeue a job without blocking (non-blocking operation).
	 *
	 * Implementation details:
	 * - Never blocks, returns immediately regardless of queue state
	 * - Returns error if queue is empty (vs blocking in dequeue())
	 * - Uses scoped_lock for quick queue access and release
	 * - Ideal for polling-based consumers or timeout-based operations
	 *
	 * Performance Benefits:
	 * - No condition variable overhead
	 * - No thread blocking/unblocking costs
	 * - Suitable for high-frequency polling scenarios
	 *
	 * Use Cases:
	 * - Non-blocking consumer threads
	 * - Timeout-based dequeue operations
	 * - Testing scenarios where blocking is undesirable
	 *
	 * @return Unique pointer to job on success, error if queue empty/stopped
	 */
	auto job_queue::try_dequeue() -> result<std::unique_ptr<job>>
	{
		// Early validation: check if queue is stopped
		if (stop_.load())
		{
			return error{error_code::queue_stopped, "Job queue is stopped"};
		}

		// Critical section: check and potentially extract job
		std::scoped_lock<std::mutex> lock(mutex_);

		// Non-blocking check: return error if empty
		if (queue_.empty())
		{
			return error{error_code::queue_empty, "there are no jobs to dequeue"};
		}

		// Efficiently extract first job from queue
		auto value = std::move(queue_.front());
		queue_.pop_front();

		return value;  // Return moved job (caller takes ownership)
	}

	/**
	 * @brief Removes and returns ALL jobs from the queue (non-blocking operation).
	 * 
	 * Implementation details:
	 * - Non-blocking: returns immediately regardless of queue state
	 * - Uses efficient swap operation to transfer entire queue contents
	 * - Notifies all waiting threads since queue is now empty
	 * - Returns empty deque if queue was already empty
	 * - Leaves queue in empty state after operation
	 * 
	 * Performance Characteristics:
	 * - O(1) complexity due to swap operation (very efficient)
	 * - No copying of job objects (moves ownership)
	 * - Single lock acquisition for entire batch
	 * 
	 * Use Cases:
	 * - Shutdown scenarios (drain all pending work)
	 * - Batch processing of accumulated jobs
	 * - Queue migration between workers
	 * 
	 * Thread Safety:
	 * - Uses scoped_lock for automatic cleanup
	 * - notify_all() ensures no threads remain blocked on empty queue
	 * 
	 * @return Deque containing all jobs that were in the queue
	 */
	auto job_queue::dequeue_batch(void) -> std::deque<std::unique_ptr<job>>
	{
		std::deque<std::unique_ptr<job>> all_items;
		{
			// Critical section: atomically transfer all queue contents
			std::scoped_lock<std::mutex> lock(mutex_);

			// Efficient O(1) transfer of all jobs
			std::swap(queue_, all_items);

			// Wake all waiting threads since queue is now empty
			condition_.notify_all();
		}

		return all_items;  // Return all extracted jobs
	}

	/**
	 * @brief Removes all jobs from the queue without returning them.
	 * 
	 * Implementation details:
	 * - Discards all jobs immediately (destructors called automatically)
	 * - Uses deque::clear() for efficient bulk removal
	 * - Notifies all waiting threads since queue is now empty
	 * - More efficient than dequeue_batch() when jobs don't need to be processed
	 * 
	 * Use Cases:
	 * - Emergency shutdown (discard pending work)
	 * - Queue reset/reinitialization
	 * - Error recovery scenarios
	 * 
	 * @note Jobs are destroyed immediately, so any cleanup logic in job destructors will run
	 */
	auto job_queue::clear(void) -> void
	{
		// Critical section: atomically clear all jobs
		std::scoped_lock<std::mutex> lock(mutex_);

		// Destroy all jobs in queue
		queue_.clear();

		// Wake all waiting threads since queue is now empty
		condition_.notify_all();
	}

	/**
	 * @brief Checks if the queue contains any jobs.
	 * 
	 * Implementation details:
	 * - Thread-safe read operation using mutex
	 * - Snapshot in time (may change immediately after call)
	 * - Useful for non-blocking queue state checks
	 * 
	 * @note Result may be stale by the time caller uses it in multi-threaded environment
	 * 
	 * @return true if queue has no jobs, false if jobs are present
	 */
	auto job_queue::empty(void) const -> bool
	{
		std::scoped_lock<std::mutex> lock(mutex_);

		return queue_.empty();
	}

	/**
	 * @brief Signals the queue to stop accepting jobs and wake waiting threads.
	 * 
	 * Implementation details:
	 * - Sets atomic stop flag to prevent new enqueue operations
	 * - Wakes all threads waiting in dequeue() operations
	 * - Those threads will then return with error instead of blocking
	 * - Used for coordinated shutdown of producer-consumer systems
	 * 
	 * Shutdown Sequence:
	 * 1. Set stop flag (prevents new jobs)
	 * 2. Notify all waiting consumers
	 * 3. Waiting dequeue() calls return with error
	 * 4. Threads can then complete their shutdown process
	 * 
	 * Thread Safety:
	 * - Safe to call from any thread
	 * - Idempotent operation (safe to call multiple times)
	 */
	auto job_queue::stop_waiting_dequeue(void) -> void
	{
		// Critical section: set stop flag and notify waiters
		std::scoped_lock<std::mutex> lock(mutex_);

		// Prevent new jobs from being added
		stop_.store(true);

		// Wake all threads waiting in dequeue()
		condition_.notify_all();
	}

	/**
	 * @brief Returns the current number of jobs in the queue.
	 * 
	 * Implementation details:
	 * - Thread-safe read operation using mutex
	 * - Snapshot in time (may change immediately after call)
	 * - O(1) operation for std::deque
	 * 
	 * @note Result may be stale by the time caller uses it in multi-threaded environment
	 * 
	 * @return Number of jobs currently in the queue
	 */
	auto job_queue::size(void) const -> std::size_t
	{
		std::scoped_lock<std::mutex> lock(mutex_);
		return queue_.size();
	}

	/**
	 * @brief Provides a string representation of the queue's current state.
	 * 
	 * Implementation details:
	 * - Uses size() method to get current job count
	 * - Formats output using the formatter utility
	 * - Useful for logging and debugging purposes
	 * 
	 * @return Formatted string showing current job count
	 */
	auto job_queue::to_string(void) const -> std::string
	{
		return formatter::format("contained {} jobs", size());
	}
} // namespace kcenon::thread