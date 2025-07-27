#pragma once

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

#include "../../thread_base/core/thread_base.h"
#include "../../utilities/core/formatter.h"
#include "../../thread_base/jobs/job_queue.h"
#include "../../utilities/conversion/convert_string.h"
#include "../detail/forward_declarations.h"
#include "../../interfaces/thread_context.h"
#include "worker_policy.h"

#include <memory>
#include <vector>

using namespace utility_module;
using namespace thread_module;

namespace thread_pool_module
{
	/**
	 * @class thread_worker
	 * @brief A specialized worker thread that processes jobs from a @c job_queue.
	 *
	 * The @c thread_worker class inherits from @c thread_base, leveraging its
	 * life-cycle control methods (@c start, @c stop, etc.) and provides an
	 * implementation for job processing using a shared @c job_queue. By overriding
	 * @c should_continue_work() and @c do_work(), it polls the queue for available
	 * jobs and executes them.
	 *
	 * ### Typical Usage
	 * @code
	 * auto my_queue = std::make_shared<job_queue>();
	 * auto worker   = std::make_unique<thread_worker>();
	 * worker->set_job_queue(my_queue);
	 * worker->start(); // Worker thread begins processing jobs
	 *
	 * // Enqueue jobs into my_queue...
	 *
	 * // Eventually...
	 * worker->stop();  // Waits for current job to finish, then stops
	 * @endcode
	 */
	class thread_worker : public thread_base
	{
	public:
		/**
		 * @brief Constructs a new @c thread_worker.
		 * @param use_time_tag If set to @c true (default), the worker may log or utilize
		 *                     timestamps/tags when processing jobs.
		 * @param context Optional thread context for logging and monitoring (defaults to empty context).
		 *
		 * This flag can be used to measure job durations, implement logging with
		 * timestamps, or any other time-related features in your job processing.
		 * The context provides access to logging and monitoring services.
		 */
		thread_worker(const bool& use_time_tag = true, 
		             const thread_context& context = thread_context());

		/**
		 * @brief Virtual destructor. Ensures the worker thread is stopped before destruction.
		 */
		virtual ~thread_worker(void);

		/**
		 * @brief Sets the @c job_queue that this worker should process.
		 * @param job_queue A shared pointer to the queue containing jobs.
		 *
		 * Once the queue is set and @c start() is called, the worker will repeatedly poll
		 * the queue for new jobs and process them.
		 */
		auto set_job_queue(std::shared_ptr<job_queue> job_queue) -> void;

		/**
		 * @brief Sets the thread context for this worker.
		 * @param context The thread context providing access to logging and monitoring services.
		 */
		auto set_context(const thread_context& context) -> void;

		/**
		 * @brief Get the worker ID.
		 * @return The unique ID for this worker instance.
		 */
		[[nodiscard]] std::size_t get_worker_id() const;

		/**
		 * @brief Gets the thread context for this worker.
		 * @return The thread context providing access to logging and monitoring services.
		 */
		[[nodiscard]] auto get_context(void) const -> const thread_context&;

	protected:
		/**
		 * @brief Determines if there are jobs available in the queue to continue working on.
		 * @return @c true if there is work in the queue, @c false otherwise.
		 *
		 * Called in the thread's main loop (defined by @c thread_base) to decide if
		 * @c do_work() should be invoked. Returns @c true if the job queue is not empty;
		 * otherwise, @c false.
		 */
		[[nodiscard]] auto should_continue_work() const -> bool override;

		/**
		 * @brief Processes one or more jobs from the queue.
		 * @return @c result_void containing an error if the work fails, or success value otherwise.
		 *
		 * This method fetches a job from the queue (if available), executes it, and
		 * may repeat depending on the implementation. If any job fails, an error
		 * is returned. Otherwise, return a success value.
		 */
		auto do_work() -> result_void override;

	private:
		/**
		 * @brief Static counter for generating unique worker IDs.
		 */
		static std::atomic<std::size_t> next_worker_id_;

		/**
		 * @brief Unique ID for this worker instance.
		 */
		std::size_t worker_id_{0};

		/**
		 * @brief Indicates whether to use time tags or timestamps for job processing.
		 *
		 * When @c true, the worker might record timestamps (e.g., job start/end times)
		 * or log them for debugging/monitoring. The exact usage depends on the
		 * job and override details in derived classes.
		 */
		bool use_time_tag_;

		/**
		 * @brief A shared pointer to the job queue from which this worker obtains jobs.
		 *
		 * Multiple workers can share the same queue, enabling concurrent processing
		 * of queued jobs.
		 */
		std::shared_ptr<job_queue> job_queue_;

		/**
		 * @brief The thread context providing access to logging and monitoring services.
		 *
		 * This context enables the worker to log messages and report metrics
		 * through the configured services.
		 */
		thread_context context_;
	};
} // namespace thread_pool_module

// ----------------------------------------------------------------------------
// Formatter specializations for thread_worker
// ----------------------------------------------------------------------------

#ifdef USE_STD_FORMAT
/**
 * @brief Specialization of std::formatter for @c thread_pool_module::thread_worker.
 *
 * Allows @c thread_worker objects to be formatted as strings using the C++20 <format> library
 * (when @c USE_STD_FORMAT is defined).
 *
 * ### Example
 * @code
 * auto worker = std::make_unique<thread_pool_module::thread_worker>();
 * std::string output = std::format("Worker status: {}", *worker);
 * @endcode
 */
template <>
struct std::formatter<thread_pool_module::thread_worker> : std::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c thread_worker object as a string.
	 * @tparam FormatContext The type of the format context.
	 * @param item The @c thread_worker to format.
	 * @param ctx  The format context for output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const thread_pool_module::thread_worker& item, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};

/**
 * @brief Specialization of std::formatter for wide-character @c thread_pool_module::thread_worker.
 *
 * Enables wide-string formatting of @c thread_worker objects using the C++20 <format> library.
 */
template <>
struct std::formatter<thread_pool_module::thread_worker, wchar_t>
	: std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a @c thread_worker object as a wide string.
	 * @tparam FormatContext The type of the format context.
	 * @param item The @c thread_worker to format.
	 * @param ctx  The wide-character format context for output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const thread_pool_module::thread_worker& item, FormatContext& ctx) const
	{
		auto str = item.to_string();
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};

#else // USE_STD_FORMAT

/**
 * @brief Specialization of fmt::formatter for @c thread_pool_module::thread_worker.
 *
 * Allows @c thread_worker objects to be formatted as strings using the {fmt} library.
 *
 * ### Example
 * @code
 * auto worker = std::make_unique<thread_pool_module::thread_worker>();
 * std::string output = fmt::format("Worker status: {}", *worker);
 * @endcode
 */
template <>
struct fmt::formatter<thread_pool_module::thread_worker> : fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c thread_worker object as a string.
	 * @tparam FormatContext The type of the format context.
	 * @param item The @c thread_worker to format.
	 * @param ctx  The format context for output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const thread_pool_module::thread_worker& item, FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};
#endif