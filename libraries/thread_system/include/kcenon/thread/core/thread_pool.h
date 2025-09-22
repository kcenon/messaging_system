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

#include <kcenon/thread/utils/formatter.h>
#include <kcenon/thread/core/job_queue.h>
#include <kcenon/thread/core/thread_worker.h>
#include <kcenon/thread/utils/convert_string.h>
#include <kcenon/thread/core/forward_declarations.h>
#include <kcenon/thread/interfaces/executor_interface.h>
#include <kcenon/thread/interfaces/thread_context.h>
#include "config.h"

#include <tuple>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <optional>

using namespace utility_module;
using namespace kcenon::thread;

/**
 * @namespace kcenon::thread
 * @brief Thread pool implementation for managing worker threads.
 *
 * The thread_pool_module namespace provides a standard thread pool implementation
 * for processing jobs concurrently using a team of worker threads.
 *
 * Key components include:
 * - thread_pool: The primary thread pool class managing multiple workers and a shared job queue
 * - thread_worker: A specialized worker thread that processes jobs from a shared queue
 * - task: A template-based convenience wrapper for creating and submitting callable jobs
 *
 * The thread pool pattern improves performance by:
 * - Reusing threads rather than creating new ones for each task
 * - Reducing thread creation overhead
 * - Limiting the total number of threads to control resource usage
 * - Providing a simple interface for async task execution
 *
 * @see kcenon::thread for a more advanced implementation with job prioritization
 */
namespace kcenon::thread
{
	/**
	 * @class thread_pool
	 * @brief A thread pool for concurrent execution of jobs using multiple worker threads.
	 *
	 * @ingroup thread_pools
	 *
	 * The @c thread_pool class manages a group of worker threads that process jobs from
	 * a shared @c job_queue. This implementation provides:
	 * - Efficient reuse of threads to reduce thread creation/destruction overhead
	 * - Controlled concurrency through a fixed or dynamic thread count
	 * - A simple interface for submitting jobs of various types
	 * - Graceful handling of thread startup, execution, and shutdown
	 *
	 * The thread pool is designed for scenarios where many short-lived tasks need to
	 * be executed asynchronously without creating a new thread for each task.
	 *
	 * ### Design Principles
	 * - **Worker Thread Model**: Each worker runs in its own thread, processing jobs
	 *   from the shared queue.
	 * - **Shared Job Queue**: A single, thread-safe queue holds all pending jobs.
	 * - **Job-Based Work Units**: Jobs encapsulate work to be executed.
	 * - **Non-Blocking Submission**: Adding jobs to the pool never blocks the caller thread.
	 * - **Cooperative Shutdown**: Workers can complete current jobs before stopping.
	 *
	 * ### Thread Safety
	 * All public methods of this class are thread-safe and can be called from any thread.
	 * The underlying @c job_queue is also thread-safe, allowing multiple workers to dequeue
	 * jobs concurrently.
	 *
	 * ### Performance Considerations
	 * - The number of worker threads should typically be close to the number of available
	 *   CPU cores for CPU-bound tasks.
	 * - For I/O-bound tasks, more threads may be beneficial to maximize throughput while
	 *   some threads are blocked on I/O.
	 * - Very large thread pools (significantly more threads than cores) may degrade
	 *   performance due to context switching overhead.
	 *
	 * @see thread_worker The worker thread class used by the pool
	 * @see job_queue The shared queue for storing pending jobs
	 * @see typed_kcenon::thread::typed_thread_pool For a priority-based version
	 */
	class thread_pool : public std::enable_shared_from_this<thread_pool>,
	                   public kcenon::thread::executor_interface
	{
	public:
		/**
		 * @brief Constructs a new @c thread_pool instance.
		 * @param thread_title An optional title or identifier for the thread pool (defaults to
		 * "thread_pool").
		 * @param context Optional thread context for logging and monitoring (defaults to empty context).
		 *
		 * This title can be used for logging or debugging purposes.
		 * The context provides access to logging and monitoring services.
		 */
		thread_pool(const std::string& thread_title = "thread_pool", 
		           const thread_context& context = thread_context());

		/**
		 * @brief Virtual destructor. Cleans up resources used by the thread pool.
		 *
		 * If the pool is still running, this typically calls @c stop() internally
		 * to ensure all worker threads are properly shut down.
		 */
		virtual ~thread_pool(void);

		/**
		 * @brief Retrieves a @c std::shared_ptr to this @c thread_pool instance.
		 * @return A shared pointer to the current @c thread_pool object.
		 *
		 * By inheriting from @c std::enable_shared_from_this, you can call @c get_ptr()
		 * within member functions to avoid storing a separate shared pointer.
		 */
		[[nodiscard]] auto get_ptr(void) -> std::shared_ptr<thread_pool>;

		// executor_interface
		auto execute(std::unique_ptr<job>&& work) -> result_void override;
		auto shutdown() -> result_void override;

        /**
         * @brief Starts the thread pool and all associated workers.
         * @return @c result_void containing an error on failure, or success value on success.
         *
         * If the pool is already running, a subsequent call to @c start() may return an error.
         * On success, each @c thread_worker in @c workers_ is started, enabling them to process
         * jobs from the @c job_queue_.
         */
        auto start(void) -> result_void;

		/**
		 * @brief Returns the shared @c job_queue used by this thread pool.
		 * @return A @c std::shared_ptr<job_queue> that stores the queued jobs.
		 *
		 * The returned queue is shared among all worker threads in the pool, which
		 * can concurrently dequeue and process jobs.
		 */
		[[nodiscard]] auto get_job_queue(void) -> std::shared_ptr<job_queue>;

        /**
         * @brief Enqueues a new job into the shared @c job_queue.
         * @param job A @c std::unique_ptr<job> representing the work to be done.
         * @return @c result_void containing an error on failure, or success value on success.
         */
        auto enqueue(std::unique_ptr<job>&& job) -> result_void;

        /**
         * @brief Enqueues a batch of jobs into the shared @c job_queue.
         * @param jobs A vector of @c std::unique_ptr<job> objects to be added.
         * @return @c result_void containing an error on failure, or success value on success.
         */
        auto enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs) -> result_void;

        /**
         * @brief Adds a @c thread_worker to the thread pool for specialized or additional processing.
         * @param worker A @c std::unique_ptr<thread_worker> object.
         * @return @c result_void containing an error on failure, or success value on success.
         */
        auto enqueue(std::unique_ptr<thread_worker>&& worker) -> result_void;

        /**
         * @brief Adds a batch of @c thread_worker objects to the thread pool.
         * @param workers A vector of @c std::unique_ptr<thread_worker> objects.
         * @return @c result_void containing an error on failure, or success value on success.
         */
        auto enqueue_batch(std::vector<std::unique_ptr<thread_worker>>&& workers)
            -> result_void;

        /**
         * @brief Stops the thread pool and all worker threads.
         * @param immediately_stop If @c true, any ongoing jobs may be interrupted; if @c false
         *        (default), each worker attempts to finish its current job before stopping.
         * @return @c result_void containing an error on failure, or success value on success.
         */
        auto stop(const bool& immediately_stop = false) -> result_void;

		/**
		 * @brief Provides a string representation of this @c thread_pool.
		 * @return A string describing the pool, including its title and other optional details.
		 *
		 * Derived classes may override this to include more diagnostic or state-related info.
		 */
		[[nodiscard]] auto to_string(void) const -> std::string;

		/**
		 * @brief Get the pool instance id.
		 * @return Returns the unique instance id for this pool.
		 */
		[[nodiscard]] std::uint32_t get_pool_instance_id() const;

		/**
		 * @brief Collect and report current thread pool metrics.
		 * 
		 * This method gathers current metrics from the pool and reports them
		 * through the monitoring interface if available.
		 */
		void report_metrics();

		/**
		 * @brief Get the number of idle workers.
		 * @return Number of workers currently not processing jobs.
		 */
		[[nodiscard]] std::size_t get_idle_worker_count() const;

		/**
		 * @brief Gets the thread context for this pool.
		 * @return The thread context providing access to logging and monitoring services.
		 */
		[[nodiscard]] auto get_context(void) const -> const thread_context&;

		// Public API methods
		/**
		 * @brief Submit a task to the thread pool
		 * @param task The task to be executed
		 * @return true if task was successfully submitted, false otherwise
		 */
		auto submit_task(std::function<void()> task) -> bool;

		/**
		 * @brief Get the number of worker threads in the pool
		 * @return Number of active worker threads
		 */
		auto get_thread_count() const -> std::size_t;

		/**
		 * @brief Shutdown the thread pool
		 * @param immediate If true, stop immediately; if false, wait for current tasks to complete
		 * @return true if shutdown was successful, false otherwise
		 */
		auto shutdown_pool(bool immediate = false) -> bool;

		/**
		 * @brief Check if the thread pool is currently running
		 * @return true if the pool is active, false otherwise
		 */
		auto is_running() const -> bool;

		/**
		 * @brief Get the number of pending tasks in the queue
		 * @return Number of tasks waiting to be processed
		 */
		auto get_pending_task_count() const -> std::size_t;

	private:
		/**
		 * @brief Static counter for generating unique pool instance IDs.
		 */
		static std::atomic<std::uint32_t> next_pool_instance_id_;

		/**
		 * @brief A title or name for this thread pool, useful for identification and logging.
		 */
		std::string thread_title_;

		/**
		 * @brief Unique instance ID for this pool (for multi-pool scenarios).
		 */
		std::uint32_t pool_instance_id_{0};

		/**
		 * @brief Indicates whether the pool is currently running.
		 *
		 * Set to @c true after a successful call to @c start(), and reset to @c false after @c
		 * stop(). Used internally to prevent multiple active starts or erroneous state transitions.
		 */
		std::atomic<bool> start_pool_;

		/**
		 * @brief The shared job queue where jobs (@c job objects) are enqueued.
		 *
		 * Worker threads dequeue jobs from this queue to perform tasks. The queue persists
		 * for the lifetime of the pool or until no more references exist.
		 */
		std::shared_ptr<job_queue> job_queue_;

		/**
		 * @brief A collection of worker threads associated with this pool.
		 *
		 * Each @c thread_worker typically runs in its own thread context, processing jobs
		 * from @c job_queue_ or performing specialized logic. They are started together
		 * when @c thread_pool::start() is called.
		 */
		std::vector<std::unique_ptr<thread_worker>> workers_;

		/**
		 * @brief The thread context providing access to logging and monitoring services.
		 *
		 * This context is shared with all worker threads created by this pool,
		 * enabling consistent logging and monitoring throughout the pool.
		 */
		thread_context context_;
	};
} // namespace kcenon::thread

// ----------------------------------------------------------------------------
// Formatter specializations for thread_pool
// ----------------------------------------------------------------------------

#ifdef USE_STD_FORMAT
/**
 * @brief Specialization of std::formatter for @c kcenon::thread::thread_pool.
 *
 * Enables formatting of @c thread_pool objects as strings using the C++20 <format> library
 * (when @c USE_STD_FORMAT is defined).
 *
 * ### Example
 * @code
 * auto pool = std::make_shared<kcenon::thread::thread_pool>("MyPool");
 * std::string output = std::format("Pool Info: {}", *pool); // e.g. "Pool Info: [thread_pool:
 * MyPool]"
 * @endcode
 */
template <>
struct std::formatter<kcenon::thread::thread_pool> : std::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c thread_pool object as a string.
	 * @tparam FormatContext The type of the format context.
	 * @param item The @c thread_pool to format.
	 * @param ctx  The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const kcenon::thread::thread_pool& item, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};

/**
 * @brief Specialization of std::formatter for wide-character @c kcenon::thread::thread_pool.
 *
 * Allows wide-string formatting of @c thread_pool objects using the C++20 <format> library.
 */
template <>
struct std::formatter<kcenon::thread::thread_pool, wchar_t>
	: std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a @c thread_pool object as a wide string.
	 * @tparam FormatContext The type of the format context.
	 * @param item The @c thread_pool to format.
	 * @param ctx  The wide-character format context.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const kcenon::thread::thread_pool& item, FormatContext& ctx) const
	{
		auto str = item.to_string();
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};

#else // USE_STD_FORMAT

/**
 * @brief Specialization of fmt::formatter for @c kcenon::thread::thread_pool.
 *
 * Allows @c thread_pool objects to be formatted as strings using the {fmt} library.
 *
 * ### Example
 * @code
 * auto pool = std::make_shared<kcenon::thread::thread_pool>("MyPool");
 * pool->start();
 * std::string output = fmt::format("Pool Info: {}", *pool);
 * @endcode
 */
template <>
struct fmt::formatter<kcenon::thread::thread_pool> : fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c thread_pool object as a string using {fmt}.
	 * @tparam FormatContext The type of the format context.
	 * @param item The @c thread_pool to format.
	 * @param ctx  The format context for output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const kcenon::thread::thread_pool& item, FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};
#endif
