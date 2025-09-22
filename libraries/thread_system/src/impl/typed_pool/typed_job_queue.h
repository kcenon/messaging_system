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
#include <kcenon/thread/utils/span.h>
#include <kcenon/thread/core/job_queue.h>
#include "typed_job.h"
#include <kcenon/thread/utils/convert_string.h>
#include "job_types.h"
#include "callback_typed_job.h"

#include <map>
#include <unordered_map>
#include <shared_mutex>

using namespace kcenon::thread;

namespace kcenon::thread
{
	/**
	 * @class typed_job_queue_t
	 * @brief A template-based queue that manages jobs with distinct priority levels.
	 *
	 * This class inherits from @c job_queue and provides functionality to enqueue and
	 * dequeue priority-based jobs. Internally, it maintains multiple queues (one per
	 * priority level) and tracks their sizes.
	 *
	 * @tparam job_type The type used to represent job priority levels. Defaults to
	 *         @c job_types.
	 */
	template <typename job_type = job_types> class typed_job_queue_t : public job_queue
	{
	public:
		/**
		 * @brief Constructs an empty priority job queue.
		 */
		typed_job_queue_t();

		/**
		 * @brief Destroys the priority job queue and all remaining jobs within it.
		 */
		~typed_job_queue_t() override;

		/**
		 * @brief Enqueues a non-priority (base) job into the queue.
		 *
		 * This function accepts a @c std::unique_ptr<job> which does not carry specific
		 * priority information. Typically, these jobs may be handled in a default or
		 * low-priority manner, depending on the implementation.
		 *
		 * @param value A unique pointer to the base job to enqueue.
		 * @return @c result_void containing an error if the operation fails, or a success value.
		 */
		[[nodiscard]] auto enqueue(std::unique_ptr<job>&& value) -> result_void override;

		/**
		 * @brief Enqueues a batch of jobs into the queue.
		 *
		 * This function accepts a vector of unique pointers to jobs and enqueues them
		 * in an unspecified order. The method returns an error if any job fails
		 * to enqueue.
		 *
		 * @param jobs A vector of unique pointers to jobs to enqueue.
		 * @return @c result_void containing an error if the operation fails, or a success value.
		 */
		// This method accepts job references (different from base class which takes rvalue
		// references)
		[[nodiscard]] auto enqueue_batch_ref(std::vector<std::unique_ptr<job>>& jobs)
			-> result_void;

		// Override from base class
		[[nodiscard]] auto enqueue_batch(std::vector<std::unique_ptr<job>>&& jobs)
			-> result_void override;

		/**
		 * @brief Enqueues a priority job into the appropriate priority queue.
		 *
		 * @param value A unique pointer to the priority job to enqueue.
		 * @return @c result_void containing an error if the operation fails, or a success value.
		 */
		[[nodiscard]] auto enqueue(std::unique_ptr<typed_job_t<job_type>>&& value)
			-> result_void;

		/**
		 * @brief Enqueues a derived typed job (like callback_typed_job) into the appropriate priority queue.
		 *
		 * @tparam DerivedJob A type that derives from typed_job_t<job_type>
		 * @param value A unique pointer to the derived job to enqueue.
		 * @return @c result_void containing an error if the operation fails, or a success value.
		 */
		template<typename DerivedJob>
		[[nodiscard]] auto enqueue(std::unique_ptr<DerivedJob>&& value) -> result_void
			requires std::is_base_of_v<typed_job_t<job_type>, DerivedJob>
		{
			return enqueue(std::unique_ptr<typed_job_t<job_type>>(std::move(value)));
		}

		/**
		 * @brief Enqueues a batch of priority jobs into the appropriate priority queues.
		 *
		 * This function accepts a vector of unique pointers to priority jobs and enqueues
		 * them in an unspecified order. The method returns an error if any job fails
		 * to enqueue.
		 *
		 * @param jobs A vector of unique pointers to priority jobs to enqueue.
		 * @return @c result_void containing an error if the operation fails, or a success value.
		 */
		[[nodiscard]] auto enqueue_batch(
			std::vector<std::unique_ptr<typed_job_t<job_type>>>&& jobs) -> result_void;

		/**
		 * @brief Dequeues the next available job (of any type or priority).
		 *
		 * This method attempts to dequeue from the front of any internal priority queue
		 * that may contain a job, typically checking in an unspecified priority order.
		 *
		 * @return A result<std::unique_ptr<job>> containing either the dequeued job or an error.
		 */
		[[nodiscard]] auto dequeue() -> result<std::unique_ptr<job>> override;

		/**
		 * @brief Dequeues a job with one of the specified types.
		 *
		 * This method checks the queues corresponding to the given priority levels
		 * in an implementation-defined sequence and removes the first job found.
		 *
		 * @param types A list of priority levels from which to attempt a dequeue.
		 * @return A result<std::unique_ptr<typed_job_t<job_type>>> containing either
		 *         the dequeued job or an error.
		 */
		[[nodiscard]] auto dequeue(const std::vector<job_type>& types)
			-> result<std::unique_ptr<typed_job_t<job_type>>>;
			
		/**
		 * @brief Dequeues a job with one of the specified types using span.
		 *
		 * This method checks the queues corresponding to the given priority levels
		 * in an implementation-defined sequence and removes the first job found.
		 * This overload accepts a span to avoid copying the types collection.
		 *
		 * @param types A span of priority levels from which to attempt a dequeue.
		 * @return A result<std::unique_ptr<typed_job_t<job_type>>> containing either
		 *         the dequeued job or an error.
		 */
		[[nodiscard]] auto dequeue(utility_module::span<const job_type> types)
			-> result<std::unique_ptr<typed_job_t<job_type>>>;

		/**
		 * @brief Removes all jobs from all priority queues.
		 *
		 * After this call, the queue will be empty for every priority level.
		 */
		auto clear() -> void override;

		/**
		 * @brief Checks if there are no jobs in any of the specified priority queues.
		 *
		 * @param types A list of priority levels to check.
		 * @return @c true if all specified priority queues are empty, otherwise @c false.
		 */
		[[nodiscard]] auto empty(const std::vector<job_type>& types) const -> bool;
		
		/**
		 * @brief Checks if there are no jobs in any of the specified priority queues using span.
		 *
		 * @param types A span of priority levels to check.
		 * @return @c true if all specified priority queues are empty, otherwise @c false.
		 */
		[[nodiscard]] auto empty(utility_module::span<const job_type> types) const -> bool;

		/**
		 * @brief Returns a string representation of the entire priority job queue.
		 *
		 * The format of the returned string may vary, but typically includes information
		 * about the number of jobs and their types.
		 *
		 * @return A string describing the current state of the queue.
		 */
		[[nodiscard]] auto to_string() const -> std::string override;

		/**
		 * @brief Stops accepting new jobs and marks the queue as stopped.
		 *
		 * After this method is called, any attempt to enqueue a new job will result in
		 * an error until the queue is reset or recreated.
		 */
		auto stop() -> void;

	protected:
		/**
		 * @brief Checks if the specified priority queues are empty without acquiring any locks.
		 *
		 * This function is intended for internal use, where external locking is expected
		 * to be handled by the caller.
		 *
		 * @param types A list of priority levels to check.
		 * @return @c true if all specified priority queues are empty, otherwise @c false.
		 */
		[[nodiscard]] auto empty_check_without_lock(
			const std::vector<job_type>& types) const -> bool;
			
		/**
		 * @brief Checks if the specified priority queues are empty without acquiring any locks using span.
		 *
		 * This function is intended for internal use, where external locking is expected
		 * to be handled by the caller.
		 *
		 * @param types A span of priority levels to check.
		 * @return @c true if all specified priority queues are empty, otherwise @c false.
		 */
		[[nodiscard]] auto empty_check_without_lock(
			utility_module::span<const job_type> types) const -> bool;

		/**
		 * @brief Attempts to dequeue a single job from the queue for a given priority.
		 *
		 * This function is intended for internal use to remove a job from one specific
		 * priority level. If no jobs exist at that priority, returns @c std::nullopt.
		 *
		 * @param priority The priority level from which to dequeue.
		 * @return @c std::optional<std::unique_ptr<typed_job_t<job_type>>>
		 *         - The dequeued priority job, or @c std::nullopt if the queue for
		 *           that priority is empty.
		 */
		[[nodiscard]] auto try_dequeue_from_priority(const job_type& priority)
			-> std::optional<std::unique_ptr<typed_job_t<job_type>>>;

	private:
		/**
		 * @brief A map of priority levels to job queues that store jobs of that priority.
		 */
		std::unordered_map<job_type, std::unique_ptr<job_queue>> job_queues_;

		/**
		 * @brief Mutex for protecting queue map modifications.
		 */
		mutable std::shared_mutex queues_mutex_;

		/**
		 * @brief Get or create a job queue for the specified type.
		 */
		auto get_or_create_queue(const job_type& type) -> job_queue*;
	};

	/// @brief Alias for a typed_job_queue using default job types.
	using typed_job_queue = typed_job_queue_t<job_types>;
} // namespace kcenon::thread

// Formatter specializations for typed_job_queue_t<job_type>
#ifdef USE_STD_FORMAT
/**
 * @brief Specialization of std::formatter for typed_job_queue_t<job_type> when using
 * C++20's <format>.
 *
 * This allows a typed_job_queue_t<job_type> to be formatted using the standard library
 * @c std::format function. It calls @c to_string() on the object to obtain its string
 * representation.
 *
 * @tparam job_type The type representing priority levels.
 */
template <typename job_type>
struct std::formatter<typed_kcenon::thread::typed_job_queue_t<job_type>>
	: std::formatter<std::string_view>
{
	/**
	 * @brief Formats a typed_job_queue_t<job_type> into a string view.
	 * @tparam FormatContext The formatting context type provided by std::format.
	 * @param item The typed_job_queue_t instance to format.
	 * @param ctx The formatting context where the output will be appended.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_job_queue_t<job_type>& item,
				FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};

/**
 * @brief Specialization of std::formatter for wide-character formatting of
 * typed_job_queue_t<job_type>.
 *
 * Similar to the above specialization, but uses wide-character strings, allowing the object
 * to be formatted with @c std::wformat or related wide-character formatting functions.
 *
 * @tparam job_type The type representing priority levels.
 */
template <typename job_type>
struct std::formatter<typed_kcenon::thread::typed_job_queue_t<job_type>, wchar_t>
	: std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a typed_job_queue_t<job_type> into a wide string view.
	 * @tparam FormatContext The wide-character formatting context type.
	 * @param item The typed_job_queue_t instance to format.
	 * @param ctx The wide-character formatting context for output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_job_queue_t<job_type>& item,
				FormatContext& ctx) const
	{
		auto str = item.to_string();
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};
#else
/**
 * @brief Specialization of fmt::formatter for typed_job_queue_t<job_type> when using the
 * {fmt} library.
 *
 * This allows a typed_job_queue_t<job_type> to be formatted using the {fmt} library,
 * by invoking the @c to_string() method on the instance.
 *
 * @tparam job_type The type representing priority levels.
 */
template <typename job_type>
struct fmt::formatter<typed_kcenon::thread::typed_job_queue_t<job_type>>
	: fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a typed_job_queue_t<job_type> into a string view for {fmt}.
	 * @tparam FormatContext The context type provided by {fmt}.
	 * @param item The typed_job_queue_t instance to format.
	 * @param ctx The {fmt} context where the output will be appended.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_kcenon::thread::typed_job_queue_t<job_type>& item,
				FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(item.to_string(), ctx);
	}
};
#endif

