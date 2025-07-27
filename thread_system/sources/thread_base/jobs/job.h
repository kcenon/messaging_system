/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions of source code must reproduce the above copyright notice,
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

#include "../../utilities/core/formatter.h"
#include "../../utilities/conversion/convert_string.h"
#include "../sync/error_handling.h"
#include "../sync/cancellation_token.h"

#include <tuple>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <string_view>
#include <atomic>
#include <chrono>

using namespace utility_module;

namespace thread_module
{
	class job_queue;

	/**
	 * @class job
	 * @brief Represents a unit of work (task) to be executed, typically by a job queue.
	 *
	 * The @c job class provides a base interface for scheduling and executing discrete
	 * tasks within a multi-threaded environment. Derived classes can override the
	 * @c do_work() method to implement custom logic for their specific tasks.
	 *
	 * ### Thread-Safety
	 * - @c do_work() will generally be called from a worker thread. If your task accesses
	 *   shared data or interacts with shared resources, you must ensure your implementation
	 *   is thread-safe.
	 * - The job itself is meant to be used via @c std::shared_ptr, making it easier to
	 *   manage lifetimes across multiple threads.
	 *
	 * ### Error Handling
	 * - A job returns a @c result_void from @c do_work().
	 *   - Returning @c result_void{} indicates success.
	 *   - Returning an error indicates failure with a typed error code and message.
	 * - Error information can be used for logging, debugging, or to retry a job if desired.
	 *
	 * ### Usage Example
	 * @code
	 * // 1. Create a custom job that overrides do_work().
	 * class my_job : public thread_module::job
	 * {
	 * public:
	 *     my_job() : job("my_custom_job") {}
	 *
	 *     result_void do_work() override
	 *     {
	 *         // Execute custom logic:
	 *         bool success = perform_operation();
	 *         if (!success)
	 *             return error{error_code::job_execution_failed, "Operation failed in my_custom_job"};
	 *
	 *         return result_void{}; // success
	 *     }
	 * };
	 *
	 * // 2. Submit the job to a queue.
	 * auto q = std::make_shared<thread_module::job_queue>();
	 * auto job_ptr = std::make_shared<my_job>();
	 * q->enqueue(job_ptr);
	 * @endcode
	 */
	class job
	{
	public:
		/**
		 * @brief Constructs a new @c job with an optional human-readable name.
		 *
		 * Use this constructor when your derived job class does not need to store
		 * any initial data beyond what you might manually store in derived class members.
		 *
		 * @param name A descriptive name for the job (default is "job").
		 *
		 * #### Example
		 * @code
		 * // Creating a basic job with a custom name
		 * auto my_simple_job = std::make_shared<thread_module::job>("simple_job");
		 * @endcode
		 */
		job(const std::string& name = "job");

		/**
		 * @brief Constructs a new @c job with associated raw byte data and a name.
		 *
		 * This constructor is particularly useful if your job needs an inline payload or
		 * associated data that should be passed directly to @c do_work().
		 *
		 * @param data A vector of bytes serving as the job's payload.
		 * @param name A descriptive name for the job (default is "data_job").
		 *
		 * #### Example
		 * @code
		 * // Creating a job that has binary data.
		 * std::vector<uint8_t> my_data = {0xDE, 0xAD, 0xBE, 0xEF};
		 * auto data_job = std::make_shared<thread_module::job>(my_data, "data_handling_job");
		 * @endcode
		 */
		job(const std::vector<uint8_t>& data, const std::string& name = "data_job");

		/**
		 * @brief Virtual destructor for the @c job class to allow proper cleanup in derived
		 * classes.
		 */
		virtual ~job(void);

		/**
		 * @brief Retrieves the name of this job.
		 * @return A string containing the name assigned to this job.
		 *
		 * The name can be useful for logging and diagnostic messages, especially
		 * when multiple jobs are running concurrently.
		 */
		[[nodiscard]] auto get_name(void) const -> std::string;

		/**
		 * @brief The core task execution method to be overridden by derived classes.
		 *
		 * @return A @c std::optional<std::string> indicating success or error:
		 * - @c std::nullopt on success, meaning no error occurred.
		 * - A non-empty string on failure, providing an error message or explanation.
		 *
		 * #### Default Behavior
		 * The base class implementation simply returns @c std::nullopt (i.e., no error).
		 * Override this method in a derived class to perform meaningful work.
		 *
		 * #### Concurrency
		 * - Typically invoked by worker threads in a @c job_queue.
		 * - Ensure that any shared data or resources accessed here are protected with
		 *   appropriate synchronization mechanisms (mutexes, locks, etc.) if needed.
		 */
		/**
	 * @brief The core task execution method to be overridden by derived classes.
	 *
	 * @return A @c result_void indicating success or error:
	 * - A success result (constructed with result_void{}) if no error occurred.
	 * - An error result (constructed with error{error_code, message}) on failure.
	 *
	 * #### Default Behavior
	 * The base class implementation simply returns a success result.
	 * Override this method in a derived class to perform meaningful work.
	 *
	 * #### Concurrency
	 * - Typically invoked by worker threads in a @c job_queue.
	 * - Ensure that any shared data or resources accessed here are protected with
	 *   appropriate synchronization mechanisms (mutexes, locks, etc.) if needed.
	 * - This method should check the cancellation token if one is set and return
	 *   an error with code operation_canceled if the token is cancelled.
	 */
	[[nodiscard]] virtual auto do_work(void) -> result_void;
	
	/**
	 * @brief Sets a cancellation token that can be used to cancel the job.
	 * 
	 * @param token The cancellation token to associate with this job.
	 */
	virtual auto set_cancellation_token(const cancellation_token& token) -> void;
	
	/**
	 * @brief Gets the cancellation token associated with this job.
	 * 
	 * @return The cancellation token, or a default token if none was set.
	 */
	[[nodiscard]] virtual auto get_cancellation_token() const -> cancellation_token;

		/**
		 * @brief Associates this job with a specific @c job_queue.
		 *
		 * Once assigned, the job can be aware of the queue that manages it,
		 * enabling scenarios like re-enqueuing itself upon partial failure,
		 * or querying the queue for state (depending on the queue's interface).
		 *
		 * @param job_queue A shared pointer to the @c job_queue instance.
		 *
		 * #### Implementation Detail
		 * - Stored internally as a @c std::weak_ptr. If the queue is destroyed,
		 *   the pointer becomes invalid.
		 */
		virtual auto set_job_queue(const std::shared_ptr<job_queue>& job_queue) -> void;

		/**
		 * @brief Retrieves the @c job_queue associated with this job, if any.
		 *
		 * @return A @c std::shared_ptr to the associated @c job_queue.
		 *         Will be empty if no queue was set or if the queue has already been destroyed.
		 *
		 * #### Usage Example
		 * @code
		 * auto jq = get_job_queue();
		 * if (jq)
		 * {
		 *     // Safe to use jq here
		 * }
		 * else
		 * {
		 *     // The queue is no longer valid
		 * }
		 * @endcode
		 */
		[[nodiscard]] virtual auto get_job_queue(void) const -> std::shared_ptr<job_queue>;

		/**
		 * @brief Provides a string representation of the job for logging or debugging.
		 *
		 * By default, this returns the job's name. Derived classes can override
		 * to include extra diagnostic details (e.g., job status, data contents, etc.).
		 *
		 * @return A string describing the job (e.g., @c name_).
		 *
		 * #### Example
		 * @code
		 * std::shared_ptr<my_job> job_ptr = std::make_shared<my_job>();
		 * // ...
		 * std::string desc = job_ptr->to_string(); // "my_custom_job", for instance
		 * @endcode
		 */
		[[nodiscard]] virtual auto to_string(void) const -> std::string;

	protected:
		/**
		 * @brief The descriptive name of the job, used primarily for identification and logging.
		 */
		std::string name_;

		/**
		 * @brief An optional container of raw byte data that may be used by the job.
		 *
		 * If the constructor without the data parameter is used, this vector will remain empty
		 * unless manually populated by derived classes or other means.
		 */
		std::vector<uint8_t> data_;

		/**
		 * @brief A weak reference to the @c job_queue that currently manages this job.
		 *
		 * This reference can expire if the queue is destroyed, so always lock it into a
		 * @c std::shared_ptr before use to avoid invalid access.
		 */
		std::weak_ptr<job_queue> job_queue_;
		
		/**
		 * @brief The cancellation token associated with this job.
		 * 
		 * This token can be used to cancel the job during execution. The job should
		 * periodically check this token and abort if it is cancelled.
		 */
		cancellation_token cancellation_token_;
	};
} // namespace thread_module

// ----------------------------------------------------------------------------
// Formatter specializations for job
// ----------------------------------------------------------------------------

#include "../../utilities/core/formatter_macros.h"

// Generate formatter specializations for thread_module::job
DECLARE_FORMATTER(thread_module::job)
