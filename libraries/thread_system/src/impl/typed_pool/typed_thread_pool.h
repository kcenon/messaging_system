/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this
   list of conditions and the following disclaimer in the documentation
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
#include <kcenon/thread/utils/convert_string.h>
#include <kcenon/thread/interfaces/thread_context.h>
#include "typed_job_queue.h"
#include "typed_thread_worker.h"
#include <kcenon/thread/interfaces/executor_interface.h>

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
 * @brief Type-based thread pool implementation for job scheduling with types.
 *
 * The kcenon::thread namespace extends the basic thread pool concept
 * with priority-based scheduling of jobs. It allows jobs to be processed according
 * to their importance or urgency rather than just their order of submission.
 *
 * Key components include:
 * - typed_thread_pool_t: A templated thread pool class supporting prioritized job scheduling
 * - typed_thread_worker_t: A worker thread that retrieves jobs based on priority
 * - typed_job_t: A job with an associated priority level
 * - typed_job_queue_t: A thread-safe job queue that orders jobs by priority
 * - job_types: Default enumeration of priority levels
 *
 * This implementation allows for:
 * - Custom priority types through template parameters
 * - RealTimeer priority jobs being processed ahead of lower priority ones
 * - Dynamic adjustment of types based on application needs
 *
 * @see thread_pool_module for the basic non-prioritized implementation
 */
namespace kcenon::thread
{
	/**
	 * @class typed_thread_pool_t
	 * @brief A thread pool that schedules and executes jobs based on their priority levels.
	 *
	 * @tparam job_type The type representing job types (e.g., enum or integral type).
	 * @ingroup thread_pools
	 *
	 * The typed_thread_pool_t template class provides a thread pool implementation that
	 * processes jobs according to their assigned types rather than just submission order.
	 * This allows more important work to be processed ahead of less important work, improving
	 * responsiveness for critical tasks.
	 *
	 * ### Key Features
	 * - **Type-Based Scheduling**: Jobs with higher priority are processed first.
	 * - **Customizable Type Types**: Supports custom priority types through templates.
	 * - **Worker Thread Model**: Each worker runs in its own thread, processing jobs.
	 * - **Type Job Queue**: Thread-safe queue that orders jobs by priority.
	 * - **Dynamic Thread Management**: Add/remove workers at runtime.
	 * - **Graceful Shutdown**: Option to complete current jobs before stopping.
	 *
	 * ### Use Cases
	 * - **Mixed Workloads**: Applications that process both critical and non-critical tasks.
	 * - **Responsive User Interfaces**: Prioritize UI-related tasks over background work.
	 * - **Resource Management**: Ensure important work gets resources when contention occurs.
	 * - **Service Level Guarantees**: Meet varying response time requirements for different request
	 * types.
	 *
	 * ### Thread Safety
	 * All public methods of this class are thread-safe and can be called from any thread.
	 *
	 * ### Performance Considerations
	 * - Type queue operations are more expensive than simple FIFO queues.
	 * - In high-throughput scenarios with all equal-priority jobs, consider using
	 *   the non-prioritized thread_pool instead for better performance.
	 * - For scenarios with mixed types, this implementation can significantly
	 *   improve response time for high-priority tasks.
	 *
	 * ### Example Usage
	 * @code{.cpp}
	 * using my_thread_pool = typed_thread_pool_t<my_priority_enum>;
	 *
	 * auto pool = std::make_shared<my_thread_pool>("My Thread Pool");
	 * auto start_result = pool->start();
	 * if (start_result.has_error())
	 * {
	 *     // Handle error starting the pool
	 *     std::cerr << start_result.get_error().message() << std::endl;
	 * }
	 *
	 * // Enqueue a job
	 * auto job = std::make_unique<typed_job_t<my_priority_enum>>(
	 *     my_priority_enum::HIGH,
	 *     [](){ std::cout << "RealTime priority job executed\n"; }
	 * );
	 * pool->enqueue(std::move(job));
	 *
	 * // Stop the pool
	 * pool->stop();
	 * @endcode
	 *
	 * @see typed_thread_worker_t The worker thread class used by the pool
	 * @see typed_job_t Jobs with priority information
	 * @see typed_job_queue_t The queue that orders jobs by priority
	 * @see kcenon::thread::thread_pool The non-prioritized thread pool implementation
	 */
	template <typename job_type = job_types>
	class typed_thread_pool_t
		: public std::enable_shared_from_this<typed_thread_pool_t<job_type>>
		, public kcenon::thread::executor_interface
	{
	public:
		/**
		 * @brief Constructs a new typed_thread_pool_t instance.
		 *
		 * @param thread_title A human-readable title or name for the thread pool.
		 * This can help in debugging or logging.
		 * @param context Thread context providing optional logging and monitoring services
		 */
		typed_thread_pool_t(const std::string& thread_title = "typed_thread_pool",
		                   const thread_context& context = thread_context());

		/**
		 * @brief Destroys the typed_thread_pool_t object.
		 *
		 * The destructor will invoke stop() if the pool is still running,
		 * ensuring that all threads are properly terminated before destruction.
		 */
		virtual ~typed_thread_pool_t(void);

		/**
		 * @brief Returns a shared pointer to the current typed_thread_pool_t.
		 *
		 * This is a convenience method when you only have a raw pointer to the
		 * pool but need a std::shared_ptr.
		 *
		 * @return std::shared_ptr<typed_thread_pool_t<job_type>>
		 * A shared pointer to this thread pool.
		 */
		[[nodiscard]] auto get_ptr(void) -> std::shared_ptr<typed_thread_pool_t<job_type>>;

		/**
		 * @brief Starts the thread pool by creating worker threads and
		 * initializing internal structures.
		 *
		 * @return result_void
		 *         - If an error occurs during start-up, the returned result
		 *           will contain an error object.
		 *         - If no error occurs, the result will be a success value.
		 *
		 * ### Thread Safety
		 * This method is typically called once, before using other methods such as
		 * enqueue(). Calling start() multiple times without stopping is not recommended.
		 */
		auto start(void) -> result_void;

		/**
		 * @brief Retrieves the underlying priority job queue managed by this thread pool.
		 *
		 * @return std::shared_ptr<typed_job_queue_t<job_type>>
		 * A shared pointer to the thread-safe priority job queue.
		 *
		 * ### Thread Safety
		 * This queue is shared and used by worker threads, so care should be taken
		 * if you modify or replace the queue. Typically, external code only needs
		 * to enqueue new jobs via enqueue(), rather than directly accessing the queue.
		 */
		[[nodiscard]] auto get_job_queue(void)
			-> std::shared_ptr<typed_job_queue_t<job_type>>;

		// executor_interface
		auto execute(std::unique_ptr<job>&& work) -> result_void override;
		auto shutdown() -> result_void override { return stop(false); }

		/**
		 * @brief Enqueues a priority job into the thread pool's job queue.
		 *
		 * @param job A unique pointer to the priority job to be added.
		 *
		 * @return result_void
		 *         - Contains an error if the enqueue operation fails.
		 *         - Otherwise, returns a success value.
		 *
		 * ### Thread Safety
		 * This method is thread-safe; multiple threads can safely enqueue jobs
		 * concurrently.
		 */
		auto enqueue(std::unique_ptr<typed_job_t<job_type>>&& job) -> result_void;

		/**
		 * @brief Enqueues a batch of priority jobs into the thread pool's job queue.
		 *
		 * @param jobs A vector of unique pointers to priority jobs to be added.
		 *
		 * @return result_void
		 *         - Contains an error if the enqueue operation fails.
		 *         - Otherwise, returns a success value.
		 *
		 * ### Thread Safety
		 * This method is thread-safe; multiple threads can safely enqueue jobs
		 * concurrently.
		 */
		auto enqueue_batch(std::vector<std::unique_ptr<typed_job_t<job_type>>>&& jobs)
			-> result_void;

		/**
		 * @brief Enqueues a new worker thread for this thread pool.
		 *
		 * This allows dynamic addition of worker threads while the pool is running.
		 *
		 * @param worker A unique pointer to the priority thread worker to be added.
		 *
		 * @return result_void
		 *         - Contains an error if the enqueue operation fails.
		 *         - Otherwise, returns a success value.
		 *
		 * ### Note
		 * Typically, most applications create a fixed number of workers at startup.
		 * However, if your workload changes significantly, adding more workers at
		 * runtime can help handle increased job load.
		 *
		 * ### Thread Safety
		 * This method is thread-safe.
		 */
		auto enqueue(std::unique_ptr<typed_thread_worker_t<job_type>>&& worker)
			-> result_void;

		/**
		 * @brief Enqueues a batch of new worker threads for this thread pool.
		 *
		 * This allows dynamic addition of multiple worker threads while the pool is running.
		 *
		 * @param workers A vector of unique pointers to priority thread workers to be added.
		 *
		 * @return result_void
		 *         - Contains an error if the enqueue operation fails.
		 *         - Otherwise, returns a success value.
		 *
		 * ### Note
		 * Typically, most applications create a fixed number of workers at startup.
		 * However, if your workload changes significantly, adding more workers at
		 * runtime can help handle increased job load.
		 *
		 * ### Thread Safety
		 * This method is thread-safe.
		 */
		auto enqueue_batch(
			std::vector<std::unique_ptr<typed_thread_worker_t<job_type>>>&& workers)
			-> result_void;

		/**
		 * @brief Stops the thread pool and optionally waits for currently running
		 * jobs to finish.
		 *
		 * @param clear_queue If `true`, any queued jobs are removed.
		 *                   If `false` (default), the pool stops accepting new jobs
		 *                   but allows currently running jobs to complete.
		 * @return result_void
		 *         - Contains an error if the stop operation fails.
		 *         - Otherwise, returns a success value.
		 *
		 * ### Thread Safety
		 * Calling stop() from multiple threads simultaneously is safe,
		 * but redundant calls to stop() will have no additional effect after the first.
		 */
		auto stop(bool clear_queue = false) -> result_void;

		/**
		 * @brief Generates a string representation of the thread pool's internal state.
		 *
		 * @return std::string A human-readable string containing pool details,
		 *                     such as whether the pool is running, the thread title,
		 *                     and potentially the number of workers.
		 *
		 * ### Example
		 * @code{.cpp}
		 * std::cout << pool.to_string() << std::endl;
		 * // Output might look like:
		 * // "typed_thread_pool [Title: typed_thread_pool, Started: true, Workers: 4]"
		 * @endcode
		 */
		[[nodiscard]] auto to_string(void) const -> std::string;

		/**
		 * @brief Sets the job queue for this thread pool and its workers.
		 *
		 * @param job_queue A shared pointer to the job queue to use.
		 */
		auto set_job_queue(std::shared_ptr<typed_job_queue_t<job_type>> job_queue) -> void;

		/**
		 * @brief Gets the thread context for this pool.
		 *
		 * @return const thread_context& Reference to the thread context
		 */
		[[nodiscard]] auto get_context(void) const -> const thread_context&;

	private:
		/** @brief A descriptive name or title for this thread pool, useful for logging. */
		std::string thread_title_;

		/** @brief Indicates whether the thread pool has been started. */
		std::atomic<bool> start_pool_;

		/** @brief The shared priority job queue from which workers fetch jobs. */
		std::shared_ptr<typed_job_queue_t<job_type>> job_queue_;

		/** @brief The collection of worker threads responsible for processing jobs. */
		std::vector<std::unique_ptr<typed_thread_worker_t<job_type>>> workers_;

		/** @brief The thread context providing optional services. */
		thread_context context_;
	};

	/// Alias for a typed_thread_pool with the default job_types type.
	using typed_thread_pool = typed_thread_pool_t<job_types>;

} // namespace kcenon::thread

// Formatter specializations for typed_thread_pool_t<job_type>
#ifdef USE_STD_FORMAT
/**
 * @brief Specialization of std::formatter for typed_thread_pool_t<job_type>.
 *
 * Allows formatting of typed_thread_pool_t<job_type> objects as strings
 * using the standard library's format facilities (C++20 or later).
 */
template <typename job_type>
struct std::formatter<typed_kcenon::thread::typed_thread_pool_t<job_type>>
	: std::formatter<std::string_view>
{
	/**
	 * @brief Formats a typed_thread_pool_t<job_type> object as a string.
	 *
	 * @tparam FormatContext Type of the format context.
	 * @param item The typed_thread_pool_t<job_type> object to format.
	 * @param ctx The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_thread_pool_t<job_type>& item,
				FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};

/**
 * @brief Specialization of std::formatter for wide-character
 * typed_thread_pool_t<job_type>.
 *
 * This enables formatting of typed_thread_pool_t<job_type> objects as
 * wide strings using the standard library's format facilities (C++20 or later).
 */
template <typename job_type>
struct std::formatter<typed_kcenon::thread::typed_thread_pool_t<job_type>, wchar_t>
	: std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a typed_thread_pool_t<job_type> object as a wide string.
	 *
	 * @tparam FormatContext Type of the format context.
	 * @param item The typed_thread_pool_t<job_type> object to format.
	 * @param ctx The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_thread_pool_t<job_type>& item,
				FormatContext& ctx) const
	{
		auto str = item.to_string();
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};
#else
/**
 * @brief Specialization of fmt::formatter for typed_thread_pool_t<job_type>.
 *
 * Allows formatting of typed_thread_pool_t<job_type> objects as strings
 * using the {fmt} library (https://github.com/fmtlib/fmt).
 */
template <typename job_type>
struct fmt::formatter<typed_kcenon::thread::typed_thread_pool_t<job_type>>
	: fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a typed_thread_pool_t<job_type> object as a string.
	 *
	 * @tparam FormatContext Type of the format context.
	 * @param item The typed_thread_pool_t<job_type> object to format.
	 * @param ctx The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_thread_pool_t<job_type>& item,
				FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};
#endif

