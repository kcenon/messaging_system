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
#include <kcenon/thread/core/thread_base.h>
#include <kcenon/thread/utils/convert_string.h>
#include <kcenon/thread/interfaces/thread_context.h>
#include "typed_job_queue.h"

#include <memory>
#include <vector>

using namespace utility_module;
using namespace kcenon::thread;

namespace kcenon::thread
{
	/**
	 * @class typed_thread_worker_t
	 * @brief A template-based worker thread class that processes jobs from a priority job queue.
	 *
	 * This class extends the functionality of \c thread_base by continually
	 * retrieving and executing jobs from a \c typed_job_queue_t. Each
	 * worker can be configured to handle specific priority levels, allowing
	 * for flexible job distribution among multiple workers.
	 *
	 * @tparam job_type
	 * A type that represents the priority levels. Often an enumeration (e.g., \c job_types).
	 * By default, it is set to \c job_types.
	 *
	 * ### Usage Example
	 * @code
	 * auto worker = std::make_shared<typed_thread_worker_t<job_types>>(
	 *     std::vector<job_types>{job_types::HIGH, job_types::MEDIUM},
	 *     true
	 * );
	 *
	 * auto queue = std::make_shared<typed_job_queue_t<job_types>>();
	 * worker->set_job_queue(queue);
	 *
	 * // Start the worker thread
	 * worker->start();
	 *
	 * // Enqueue some jobs
	 * queue->push_job(
	 *     // job lambda or functor
	 *     , job_types::HIGH
	 * );
	 ** // Stop and join the worker once done
	 *worker->stop();
	 *worker->join();
	 * @endcode
	 *
	 * ### Thread Safety
	 * The worker is designed to run its own thread that continuously checks
	 * the queue for new jobs. Any shared data structures should be either
	 * thread-safe or externally synchronized.
	 *
	 * @see thread_base
	 * @see typed_job_queue_t
	 */
	template <typename job_type = job_types>
	class typed_thread_worker_t : public thread_base
	{
	public:
		/**
		 * @brief Constructs a new typed_thread_worker_t object.
		 *
		 * @param types
		 * A list of priority levels that this worker is responsible for processing.
		 * If no list is provided, the default is \c all_types(), meaning
		 * the worker handles all known priority levels.
		 *
		 * @param use_time_tag
		 * A boolean flag indicating whether the worker should utilize
		 * time-tagged information (if available) for job scheduling
		 * or logging. Defaults to \c true.
		 * 
		 * @param context Thread context providing logging and monitoring services.
		 *
		 * @note
		 * The \c use_time_tag feature may be leveraged by derived classes or
		 * additional logging mechanisms if time-based processing is required.
		 */
		typed_thread_worker_t(std::vector<job_type> types = all_types(),
								 const bool& use_time_tag = true,
								 const thread_context& context = thread_context());

		/**
		 * @brief Virtual destructor for the typed_thread_worker_t class.
		 *
		 * Ensures proper cleanup of resources. The underlying thread must be
		 * stopped and joined before destruction to avoid potential data races
		 * or undefined behavior.
		 */
		virtual ~typed_thread_worker_t(void);

		/**
		 * @brief Assigns a priority job queue to this worker.
		 *
		 * Once a queue is set, the worker will continuously retrieve
		 * jobs that match the configured priority levels. If a different
		 * queue is assigned later, the worker will switch to that queue
		 * for subsequent job retrieval.
		 *
		 * @param job_queue
		 * A shared pointer to a \c typed_job_queue_t instance from which
		 * this worker will fetch jobs.
		 *
		 * @note
		 * It is the responsibility of the caller to ensure the \c job_queue
		 * remains valid for the lifespan of this worker.
		 */
		auto set_job_queue(std::shared_ptr<typed_job_queue_t<job_type>> job_queue) -> void;

		/**
		 * @brief Retrieves the list of priority levels this worker handles.
		 *
		 * @return
		 * A std::vector of \c job_type values indicating the priority
		 * levels the worker will process.
		 */
		[[nodiscard]] auto types(void) const -> std::vector<job_type>;

		/**
		 * @brief Sets the thread context for this worker.
		 *
		 * @param context Thread context providing optional services
		 */
		auto set_context(const thread_context& context) -> void;

		/**
		 * @brief Gets the thread context for this worker.
		 *
		 * @return const thread_context& Reference to the thread context
		 */
		[[nodiscard]] auto get_context(void) const -> const thread_context&;

	protected:
		/**
		 * @brief Determines if there is any pending work for this worker.
		 *
		 * This method checks the assigned job queue to see if any tasks of the
		 * configured types are available to be processed.
		 *
		 * @return
		 * \c true if there is at least one job waiting that matches the
		 * worker's priority settings. Otherwise, \c false.
		 */
		[[nodiscard]] auto should_continue_work() const -> bool override;

		/**
		 * @brief Executes pending work by processing one job at a time.
		 *
		 * This method pops a job from the queue (if available) and executes it.
		 * The method is repeatedly called in the worker thread’s loop until
		 * the worker is stopped or no more jobs are available.
		 *
		 * @return
		 * An optional string that may hold a status or error description:
		 * - If \c std::nullopt is returned, it implies the job was processed
		 *   without issues or there was no job to process.
		 * - If a non-empty string is returned, it typically contains debug or
		 *   error information that can be logged or reported.
		 *
		 * @note
		 * Derived classes can override this method if custom processing logic
		 * is needed for job handling or error management.
		 */
		auto do_work() -> result_void override;

	private:
		/**
		 * @brief Indicates whether time-tagged processing is utilized.
		 *
		 * If \c true, job processing may incorporate time-based logic (e.g.,
		 * scheduling or additional logging). Otherwise, no time-based tracking
		 * is applied.
		 */
		bool use_time_tag_;

		/**
		 * @brief The priority levels this worker will process.
		 *
		 * Jobs with a matching priority level in this vector will be dequeued
		 * and processed by this worker. Others will be skipped.
		 */
		std::vector<job_type> types_;

		/**
		 * @brief The priority job queue to retrieve and execute jobs from.
		 *
		 * The worker continually checks this queue for new jobs that match
		 * its priority levels. Must be valid for the duration of this worker’s
		 * execution.
		 */
		std::shared_ptr<typed_job_queue_t<job_type>> job_queue_;

		/**
		 * @brief The thread context providing optional services.
		 *
		 * Provides access to logging and monitoring services if available.
		 */
		thread_context context_;
	};

	/// Convenience typedef for a worker configured with default job_types.
	using typed_thread_worker = typed_thread_worker_t<job_types>;
} // namespace kcenon::thread

// Formatter specializations for typed_thread_worker_t<job_type>
#ifdef USE_STD_FORMAT

/**
 * @brief Specialization of std::formatter for typed_thread_worker_t<job_type>.
 *
 * Enables formatting of \c typed_thread_worker_t<job_type> objects as strings
 * in the C++20 (and later) standard library \c std::format.
 *
 * @tparam job_type
 * The type representing the priority levels.
 */
template <typename job_type>
struct std::formatter<typed_kcenon::thread::typed_thread_worker_t<job_type>>
	: std::formatter<std::string_view>
{
	/**
	 * @brief Formats a \c typed_thread_worker_t<job_type> instance to a string.
	 *
	 * Internally calls \c item.to_string(), which should provide a string
	 * representation of the worker’s state or identity.
	 *
	 * @tparam FormatContext
	 * The type of the formatting context (provided by \c std::format).
	 *
	 * @param item
	 * The \c typed_thread_worker_t<job_type> instance to format.
	 *
	 * @param ctx
	 * The format context.
	 *
	 * @return
	 * An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_thread_worker_t<job_type>& item,
				FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};

/**
 * @brief Specialization of std::formatter for wide-character \c
 * typed_thread_worker_t<job_type>.
 *
 * Allows formatting of \c typed_thread_worker_t<job_type> objects as wide strings,
 * making it compatible with wide-character streams and formats.
 *
 * @tparam job_type
 * The type representing the priority levels.
 */
template <typename job_type>
struct std::formatter<typed_kcenon::thread::typed_thread_worker_t<job_type>, wchar_t>
	: std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a \c typed_thread_worker_t<job_type> instance to a wide string.
	 *
	 * Internally uses \c convert_string::to_wstring() to convert the result
	 * of \c item.to_string() into a wide string.
	 *
	 * @tparam FormatContext
	 * The type of the formatting context (provided by \c std::format).
	 *
	 * @param item
	 * The \c typed_thread_worker_t<job_type> instance to format.
	 *
	 * @param ctx
	 * The wide-character format context.
	 *
	 * @return
	 * An iterator to the end of the formatted wide-string output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_thread_worker_t<job_type>& item,
				FormatContext& ctx) const
	{
		auto str = item.to_string();
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};
#else
/**
 * @brief Specialization of fmt::formatter for \c typed_thread_worker_t<job_type>.
 *
 * Enables formatting of \c typed_thread_worker_t<job_type> objects using
 * the {fmt} library (https://github.com/fmtlib/fmt).
 *
 * @tparam job_type
 * The type representing the priority levels.
 */
template <typename job_type>
struct fmt::formatter<typed_kcenon::thread::typed_thread_worker_t<job_type>>
	: fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a \c typed_thread_worker_t<job_type> instance as a string.
	 *
	 * Internally calls \c item.to_string(), which should provide a string
	 * representation of the worker’s state or identity.
	 *
	 * @tparam FormatContext
	 * The format context type provided by the {fmt} library.
	 *
	 * @param item
	 * The \c typed_thread_worker_t<job_type> instance to format.
	 *
	 * @param ctx
	 * The {fmt} format context.
	 *
	 * @return
	 * An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_thread_worker_t<job_type>& item,
				FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};
#endif

