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

#include "job.h"
#include "../../utilities/core/formatter.h"
#include "callback_job.h"
#include "../../utilities/conversion/convert_string.h"
#include "../sync/error_handling.h"

#include <mutex>
#include <deque>
#include <tuple>
#include <atomic>
#include <optional>
#include <string_view>
#include <condition_variable>
#include <memory>

using namespace utility_module;

namespace thread_module
{
	/**
	 * @class job_queue
	 * @brief A thread-safe job queue for managing and dispatching work items.
	 *
	 * The @c job_queue class provides a synchronized queue for storing and retrieving
	 * @c job objects (or derived classes). Multiple threads can safely enqueue and
	 * dequeue jobs, ensuring proper synchronization and preventing data races.
	 *
	 * This class inherits from @c std::enable_shared_from_this, which allows it to
	 * create @c std::shared_ptr instances referring to itself via @c get_ptr(). This
	 * is often useful when passing a @c job_queue pointer to jobs themselves or other
	 * components that need a safe, shared reference to the queue.
	 *
	 * ### Typical Usage
	 * 1. Create a @c job_queue instance, typically via @c std::make_shared.
	 * 2. Enqueue @c job objects (or derived types) using @c enqueue().
	 * 3. One or more worker threads repeatedly call @c dequeue() to retrieve jobs
	 *    and process them.
	 * 4. Call @c stop_waiting_dequeue() and possibly @c clear() to shut down the queue
	 *    gracefully when all jobs are done or when the system is stopping.
	 */
	class job_queue : public std::enable_shared_from_this<job_queue>
	{
	public:
		/**
		 * @brief Constructs a new, empty @c job_queue.
		 *
		 * Initializes internal synchronization primitives and sets all flags to
		 * their default states (e.g., @c notify_ = false, @c stop_ = false).
		 */
		job_queue();

		/**
		 * @brief Virtual destructor. Cleans up resources used by the @c job_queue.
		 */
		virtual ~job_queue(void);

		/**
		 * @brief Obtains a @c std::shared_ptr that points to this queue instance.
		 * @return A shared pointer to the current @c job_queue object.
		 *
		 * Because @c job_queue inherits from @c std::enable_shared_from_this, calling
		 * @c get_ptr() allows retrieving a @c shared_ptr<job_queue> from within
		 * member functions of @c job_queue.
		 */
		[[nodiscard]] auto get_ptr(void) -> std::shared_ptr<job_queue>;

		/**
		 * @brief Checks if the queue is in a "stopped" state.
		 * @return @c true if the queue is stopped, @c false otherwise.
		 *
		 * When stopped, worker threads are typically notified to cease waiting for
		 * new jobs. New jobs may still be enqueued, but it is up to the system design
		 * how they are handled in a stopped state.
		 */
		[[nodiscard]] auto is_stopped() const -> bool;

		/**
		 * @brief Sets the 'notify' flag for this queue.
		 * @param notify If @c true, signals that enqueue should notify waiting threads.
		 *               If @c false, jobs can still be enqueued, but waiting threads
		 *               won't be automatically notified.
		 */
		auto set_notify(bool notify) -> void;

		/**
		 * @brief Enqueues a new job into the queue.
		 * @param value A unique pointer to the job being added.
		 * @return A result_void indicating success or an error message.
		 *
		 * This method is thread-safe. If @c notify_ is set to @c true, a waiting
		 * thread (if any) will be notified upon successful enqueue.
		 */
		[[nodiscard]] virtual auto enqueue(std::unique_ptr<job>&& value) -> result_void;

		/**
		 * @brief Enqueues a batch of jobs into the queue.
		 * @param jobs A vector of unique pointers to the jobs being added.
		 * @return A result_void indicating success or an error message.
		 */
		[[nodiscard]] virtual auto enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs) -> result_void;

		/**
		 * @brief Dequeues a job from the queue in FIFO order.
		 * @return A result<std::unique_ptr<job>> containing either a valid job
		 *         or an error object.
		 *
		 * If the queue is empty, the caller may block depending on the internal
		 * concurrency model (unless @c stop_ is set, in which case it may return
		 * immediately).
		 */
		[[nodiscard]] virtual auto dequeue(void) -> result<std::unique_ptr<job>>;

		/**
		 * @brief Dequeues all remaining jobs from the queue without processing them.
		 * @return A @c std::deque of unique_ptr<job> containing all jobs that were
		 *         in the queue at the time of the call.
		 *
		 * Similar to @c clear(), but returns the dequeued jobs to the caller for
		 * potential inspection or manual processing.
		 */
		[[nodiscard]] virtual auto dequeue_batch(void) -> std::deque<std::unique_ptr<job>>;

		/**
		 * @brief Removes all jobs currently in the queue without processing them.
		 *
		 * This operation is thread-safe and can be used to discard pending jobs,
		 * typically during shutdown or error recovery. It does not affect the
		 * @c stop_ or @c notify_ flags.
		 */
		virtual auto clear(void) -> void;

		/**
		 * @brief Checks if the queue is currently empty.
		 * @return @c true if the queue has no pending jobs, @c false otherwise.
		 * 
		 * @note This method is thread-safe.
		 */
		[[nodiscard]] auto empty(void) const -> bool;

		/**
		 * @brief Returns the current number of jobs in the queue.
		 * @return The number of pending jobs.
		 * 
		 * @note This method is thread-safe.
		 */
		[[nodiscard]] auto size(void) const -> std::size_t;

		/**
		 * @brief Signals the queue to stop waiting for new jobs (e.g., during shutdown).
		 *
		 * Sets the @c stop_ flag to @c true and notifies any threads that might be
		 * blocked in @c dequeue(). This allows worker threads to exit gracefully
		 * rather than remain blocked indefinitely.
		 */
		auto stop_waiting_dequeue(void) -> void;

		/**
		 * @brief Returns a string representation of this job_queue.
		 * @return A std::string describing the state of the queue (e.g., size, flags).
		 *
		 * Primarily used for logging and debugging. Derived classes may override
		 * this to include additional diagnostic information.
		 */
		[[nodiscard]] virtual auto to_string(void) const -> std::string;

	protected:
		/**
		 * @brief If @c true, threads waiting for new jobs are notified when a new job
		 *        is enqueued. If @c false, enqueuing does not automatically trigger
		 *        a notification.
		 */
		std::atomic_bool notify_;

		/**
		 * @brief Indicates whether the queue has been signaled to stop.
		 *
		 * Setting @c stop_ to @c true typically causes waiting threads to
		 * unblock and exit their waiting loop.
		 */
		std::atomic_bool stop_;

		/**
		 * @brief Mutex to protect access to the underlying @c queue_ container and related state.
		 *
		 * Any operation that modifies or reads the queue should lock this mutex to
		 * ensure thread safety.
		 */
		mutable std::mutex mutex_;

		/**
		 * @brief Condition variable used to signal worker threads.
		 *
		 * Used in combination with @c mutex_ to block or notify threads waiting
		 * for new jobs.
		 */
		std::condition_variable condition_;

	private:
		/**
		 * @brief The underlying container storing the jobs in FIFO order.
		 *
		 * @note This container is guarded by @c mutex_ and should only be modified
		 *       while holding the lock. The size of this container should be the only
		 *       source of truth for the queue size to prevent inconsistencies.
		 */
		std::deque<std::unique_ptr<job>> queue_;
	};
}

// ----------------------------------------------------------------------------
// Formatter specializations for job_queue
// ----------------------------------------------------------------------------

#include "../../utilities/core/formatter_macros.h"

// Generate formatter specializations for thread_module::job_queue
DECLARE_FORMATTER(thread_module::job_queue)