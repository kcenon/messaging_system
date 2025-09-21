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

#include "typed_job.h"

using namespace thread_module;

namespace typed_thread_pool_module
{
	/**
	 * @class callback_typed_job_t
	 * @brief A template for creating priority-based jobs that execute a user-defined callback.
	 *
	 * This class inherits from @c typed_job_t and stores a callback function along with a
	 * priority value. When scheduled by a priority-based thread pool or any task scheduler,
	 * the higher-priority jobs generally take precedence.
	 *
	 * @tparam job_type
	 *   The type used to represent the priority level.
	 *   Typically an enum or comparable value to determine job ordering.
	 */
	template <typename job_type>
	class callback_typed_job_t : public typed_job_t<job_type>
	{
	public:
		/**
		 * @brief Constructs a new @c callback_typed_job_t with a callback, priority, and name.
		 *
		 * @param callback
		 *   The function object to be executed when the job is processed. It must return
		 *   a @c result_void, where an error typically contains additional
		 *   status or error information.
		 * @param priority
		 *   The priority level of the job (e.g., high, normal, low).
		 * @param name
		 *   The name of the job, used primarily for logging or debugging. Defaults to
		 * "typed_job".
		 *
		 * Example usage:
		 * @code
		 * auto jobCallback = []() -> result_void {
		 *     // Your job logic here
		 *     return {};
		 * };
		 * auto myJob = std::make_shared<callback_typed_job_t<int>>(jobCallback, 10, "MyJob");
		 * @endcode
		 */
		callback_typed_job_t(const std::function<result_void(void)>& callback,
							job_type priority,
							const std::string& name = "typed_job");

		/**
		 * @brief Virtual destructor for the @c callback_typed_job_t class.
		 */
		~callback_typed_job_t(void) override;

		/**
		 * @brief Executes the stored callback function for this job.
		 *
		 * This method overrides @c typed_job_t::do_work. When invoked by the job executor,
		 * the stored callback will be called and its result will be propagated.
		 *
		 * @return result_void
		 *   - If the callback returns an error, it typically contains an informational or error
		 * message.
		 *   - If it returns a success value, the execution completed without additional
		 * info.
		 */
		[[nodiscard]] auto do_work(void) -> result_void override;

	private:
		/**
		 * @brief The user-provided callback function to execute when the job is processed.
		 *
		 * This function should encapsulate the main logic of the job.
		 * It must return a @c result_void, often representing
		 * any error messages or status feedback.
		 */
		std::function<result_void(void)> callback_;
	};

	/**
	 * @typedef callback_typed_job
	 * @brief Type alias for a @c callback_typed_job_t that uses @c job_types as its
	 * priority type.
	 */
	using callback_typed_job = callback_typed_job_t<job_types>;
} // namespace typed_thread_pool_module

#include "thread_system_core/typed_thread_pool/jobs/callback_typed_job.tpp"