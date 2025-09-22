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

#include <kcenon/thread/core/callback_job.h>

/**
 * @file callback_job.cpp
 * @brief Implementation of callback-based job execution for the thread system.
 *
 * This file contains the implementation of the callback_job class, which provides
 * a flexible way to create jobs using lambda functions, function objects, or 
 * plain function pointers. The callback_job class supports multiple callback
 * types for different use cases and automatic error handling conversion.
 * 
 * Key Features:
 * - Multiple callback types (parameterless, data-based)
 * - Automatic error conversion from legacy to modern error handling
 * - Type-safe callback storage and execution
 * - Support for both lambda functions and function objects
 * - Efficient data handling for data-based callbacks
 */

namespace kcenon::thread
{
	/**
	 * @brief Constructs a callback job with legacy error-returning callback.
	 * 
	 * Implementation details:
	 * - Adapts legacy std::optional<std::string> error format to modern result_void
	 * - Wraps the provided callback in a lambda that performs error conversion
	 * - Sets data_callback_ to nullptr since this is not a data-based callback
	 * - Inherits job name from base job class for identification
	 * 
	 * Error Conversion:
	 * - std::nullopt (no error) -> successful result_void
	 * - std::string (error message) -> error with job_execution_failed code
	 * 
	 * Performance Considerations:
	 * - Single lambda capture by value (efficient for small callbacks)
	 * - No dynamic allocation for callback storage
	 * - Minimal overhead during execution
	 * 
	 * @param callback Legacy callback function returning optional error string
	 * @param name Descriptive name for this job (for debugging/logging)
	 */
	callback_job::callback_job(const std::function<std::optional<std::string>(void)>& callback,
							   const std::string& name)
		: job(name), callback_([callback](void) -> result_void {
			// Call the original callback and convert error format
			auto result = callback();
			if (result.has_value()) {
				// Error occurred: convert string to error object
				return error{error_code::job_execution_failed, result.value()};
			}
			// Success: return empty result_void
			return result_void{};
		}), data_callback_(nullptr)
	{
	}

	/**
	 * @brief Constructs a callback job with modern result_void-returning callback.
	 * 
	 * Implementation details:
	 * - Direct storage of modern callback without error conversion
	 * - More efficient than legacy version (no wrapper lambda)
	 * - Sets data_callback_ to nullptr since this is not a data-based callback
	 * - Preferred constructor for new code due to better error handling
	 * 
	 * Modern Error Handling:
	 * - Uses result_void type with rich error information
	 * - Supports error codes and detailed error messages
	 * - Better composability with other result-based APIs
	 * 
	 * @param callback Modern callback function returning result_void
	 * @param name Descriptive name for this job (for debugging/logging)
	 */
	callback_job::callback_job(
		const std::function<result_void(void)>& callback,
		const std::string& name)
		: job(name), callback_(callback), data_callback_(nullptr)
	{
	}

	/**
	 * @brief Constructs a data-based callback job with legacy error handling.
	 * 
	 * Implementation details:
	 * - Stores binary data in base job class for later processing
	 * - Adapts legacy data callback to modern error handling
	 * - Sets regular callback_ to nullptr since this uses data processing
	 * - Wraps data callback in lambda for error format conversion
	 * 
	 * Data Processing:
	 * - Binary data is passed to callback during execution
	 * - Useful for jobs that need to process files, network data, etc.
	 * - Data is stored efficiently in the job object
	 * 
	 * Error Conversion:
	 * - Same conversion logic as parameterless legacy callback
	 * - Maintains backward compatibility with existing code
	 * 
	 * @param data_callback Legacy data-processing callback
	 * @param data Binary data to be processed by the callback
	 * @param name Descriptive name for this job
	 */
	callback_job::callback_job(
		const std::function<std::optional<std::string>(const std::vector<uint8_t>&)>& data_callback,
		const std::vector<uint8_t>& data, const std::string& name)
		: job(data, name), callback_(nullptr), data_callback_([data_callback](const std::vector<uint8_t>& callback_data) -> result_void {
			// Call the original data callback with provided data
			auto result = data_callback(callback_data);
			if (result.has_value()) {
				// Error occurred: convert string to error object
				return error{error_code::job_execution_failed, result.value()};
			}
			// Success: return empty result_void
			return result_void{};
		})
	{
	}

	/**
	 * @brief Constructs a data-based callback job with modern error handling.
	 * 
	 * Implementation details:
	 * - Direct storage of modern data callback without conversion
	 * - Most efficient option for data-processing jobs
	 * - Sets regular callback_ to nullptr since this uses data processing
	 * - Preferred constructor for new data-processing code
	 * 
	 * Data Handling:
	 * - Efficiently stores data in base job class
	 * - Data is passed by const reference during execution
	 * - Supports large data sets without excessive copying
	 * 
	 * @param data_callback Modern data-processing callback
	 * @param data Binary data to be processed by the callback
	 * @param name Descriptive name for this job
	 */
	callback_job::callback_job(
		const std::function<result_void(const std::vector<uint8_t>&)>& data_callback,
		const std::vector<uint8_t>& data, const std::string& name)
		: job(data, name), callback_(nullptr), data_callback_(data_callback)
	{
	}

	/**
	 * @brief Destroys the callback job.
	 * 
	 * Implementation details:
	 * - std::function destructors handle callback cleanup automatically
	 * - No manual cleanup required due to RAII design
	 * - Base job destructor handles data cleanup if present
	 */
	callback_job::~callback_job(void) {}

	/**
	 * @brief Executes the callback job by calling the appropriate callback function.
	 * 
	 * Implementation details:
	 * - Uses priority-based callback selection for optimal performance
	 * - Data callbacks take precedence over regular callbacks
	 * - Falls back to base class implementation if no callbacks are set
	 * - Provides consistent error handling across all callback types
	 * 
	 * Execution Priority:
	 * 1. Data callback (if data_callback_ is set)
	 * 2. Regular callback (if callback_ is set)
	 * 3. Base class do_work() (fallback for empty job)
	 * 
	 * Performance Considerations:
	 * - Minimal overhead for callback selection (simple pointer checks)
	 * - Direct function call without additional abstraction layers
	 * - No dynamic dispatch - compile-time type safety
	 * 
	 * Error Propagation:
	 * - Errors from callbacks are passed through unchanged
	 * - Modern callbacks return result_void directly
	 * - Legacy callbacks have errors converted automatically
	 * 
	 * @return Result of callback execution or base class implementation
	 */
	auto callback_job::do_work(void) -> result_void
	{
		// Priority 1: Data callback with stored binary data
		if (data_callback_)
		{
			// Execute data-processing callback with job's stored data
			return data_callback_(data_);
		}
		// Priority 2: Standard parameterless callback
		else if (callback_)
		{
			// Execute standard callback function
			return callback_();
		}
		// Priority 3: Fallback to base class implementation
		else
		{
			// No callbacks set - delegate to base job class
			// This typically returns a "not implemented" error
			return job::do_work();
		}
	}
} // namespace kcenon::thread