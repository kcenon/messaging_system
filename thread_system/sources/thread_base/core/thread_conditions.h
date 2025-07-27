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

#include "../../utilities/core/formatter.h"
#include "../../utilities/conversion/convert_string.h"

#include <string>
#include <array>
#include <cstdint>
#include <string_view>

using namespace utility_module;

namespace thread_module
{
	/**
	 * @enum thread_conditions
	 * @brief Enumeration of various states in a thread's lifecycle.
	 *
	 * Represents distinct conditions that a thread may occupy during its
	 * execution lifecycle. This enum class is backed by @c uint8_t for
	 * compact storage.
	 *
	 * | Value    | Description                            |
	 * |----------|----------------------------------------|
	 * | Created  | The thread object is constructed but not yet started. |
	 * | Waiting  | The thread is started and idle, waiting for work.      |
	 * | Working  | The thread is actively processing work.               |
	 * | Stopping | The thread is in the process of stopping its work.     |
	 * | Stopped  | The thread has fully stopped.                          |
	 */
	enum class thread_conditions : uint8_t
	{
		Created,  ///< Thread created but not started.
		Waiting,  ///< Thread waiting for work or tasks.
		Working,  ///< Thread currently processing a task.
		Stopping, ///< Thread in the process of stopping.
		Stopped,  ///< Thread fully stopped.
	};

	namespace job_detail
	{
		/**
		 * @brief Array of string representations corresponding to each @c thread_conditions state.
		 *
		 * This array is indexed by the underlying enum value cast to @c size_t, allowing
		 * compile-time string retrieval in @c to_string().
		 */
		constexpr std::array thread_conditions_strings
			= { "created", "waiting", "working", "stopping", "stopped" };

		/**
		 * @brief Total number of states defined in @c thread_conditions.
		 */
		constexpr size_t thread_conditions_count = thread_conditions_strings.size();

		/**
		 * @brief Compile-time check to ensure the string array matches the enum count.
		 *
		 * If this static_assert fails, it likely means a new enum value was added to
		 * @c thread_conditions without updating @c thread_conditions_strings.
		 */
		static_assert(thread_conditions_count
						  == static_cast<size_t>(thread_conditions::Stopped) + 1,
					  "thread_conditions_strings and thread_conditions enum are out of sync");
	}

	/**
	 * @brief Converts a @c thread_conditions value to its corresponding string representation.
	 * @param condition The @c thread_conditions value to convert.
	 * @return A @c std::string_view representing the condition name. If the value is out
	 *         of range, returns "UNKNOWN".
	 *
	 * Example:
	 * @code
	 * auto state = thread_conditions::Working;
	 * auto name = to_string(state); // "working"
	 * @endcode
	 */
	[[nodiscard]] constexpr std::string_view to_string(thread_conditions condition)
	{
		auto index = static_cast<size_t>(condition);
		return (index < job_detail::thread_conditions_count)
				   ? job_detail::thread_conditions_strings[index]
				   : "UNKNOWN";
	}

	/**
	 * @brief Retrieves a vector containing all possible @c thread_conditions values.
	 * @return A @c std::vector of all enumerators in @c thread_conditions, in ascending order.
	 *
	 * This can be useful for iteration, logging, or building UI elements that
	 * list thread states. For example:
	 * @code
	 * for (auto cond : all_types()) {
	 *     std::cout << to_string(cond) << std::endl;
	 * }
	 * @endcode
	 */
	[[nodiscard]] inline auto all_types(void) -> std::vector<thread_conditions>
	{
		return { thread_conditions::Created, thread_conditions::Waiting, thread_conditions::Working,
				 thread_conditions::Stopping, thread_conditions::Stopped };
	}
} // namespace thread_module

// ----------------------------------------------------------------------------
// Formatter specializations for thread_conditions
// ----------------------------------------------------------------------------

#ifdef USE_STD_FORMAT
/**
 * @brief Specialization of std::formatter for @c thread_module::thread_conditions.
 *
 * Allows @c thread_conditions enum values to be formatted as strings using the C++20 <format>
 * library (when @c USE_STD_FORMAT is defined).
 *
 * ### Example
 * @code
 * auto cond = thread_module::thread_conditions::Working;
 * std::string output = std::format("Thread state is {}", cond); // "Thread state is working"
 * @endcode
 */
template <>
struct std::formatter<thread_module::thread_conditions> : std::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c thread_conditions value as a string.
	 * @tparam FormatContext The type of the format context.
	 * @param thread_condition The @c thread_conditions enum value to format.
	 * @param ctx The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const thread_module::thread_conditions& thread_condition, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(thread_module::to_string(thread_condition),
														ctx);
	}
};

/**
 * @brief Specialization of std::formatter for wide-character @c thread_module::thread_conditions.
 *
 * Enables wide-string formatting of @c thread_conditions values using the C++20 <format> library.
 */
template <>
struct std::formatter<thread_module::thread_conditions, wchar_t>
	: std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a @c thread_conditions value as a wide string.
	 * @tparam FormatContext The type of the format context.
	 * @param thread_condition The @c thread_conditions enum value to format.
	 * @param ctx The wide-character format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const thread_module::thread_conditions& thread_condition, FormatContext& ctx) const
	{
		auto str = thread_module::to_string(thread_condition);
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};

#else // USE_STD_FORMAT

/**
 * @brief Specialization of fmt::formatter for @c thread_module::thread_conditions.
 *
 * Allows @c thread_conditions enum values to be formatted as strings using the {fmt} library.
 *
 * ### Example
 * @code
 * auto cond = thread_module::thread_conditions::Working;
 * std::string output = fmt::format("Thread state is {}", cond); // "Thread state is working"
 * @endcode
 */
template <>
struct fmt::formatter<thread_module::thread_conditions> : fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c thread_conditions value as a string.
	 * @tparam FormatContext The type of the format context.
	 * @param thread_condition The @c thread_conditions enum value to format.
	 * @param ctx The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const thread_module::thread_conditions& thread_condition, FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(thread_module::to_string(thread_condition),
														ctx);
	}
};
#endif