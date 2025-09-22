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

namespace typed_thread_pool_module
{
	/**
	 * @enum job_types
	 * @brief Defines different types of jobs for a typed thread pool.
	 *
	 * Each job in a typed thread pool can be assigned one of these types (RealTime, Batch, Background)
	 * to determine which specialized worker threads will process them. This enum uses @c uint8_t as its
	 * underlying type to minimize storage overhead.
	 */
	enum class job_types : uint8_t
	{
		RealTime,	///< Real-time job requiring immediate response
		Batch,		///< Batch processing job for throughput optimization
		Background	///< Background job for low-type maintenance tasks
	};

	namespace job_detail
	{
		/**
		 * @brief String representations corresponding to each @c job_types value.
		 *
		 * Indexed by casting a @c job_types value to @c size_t. E.g.,
		 * @code
		 * job_detail::job_type_strings[static_cast<size_t>(job_types::RealTime)] // "REALTIME"
		 * @endcode
		 */
		constexpr std::array job_type_strings = { "REALTIME", "BATCH", "BACKGROUND" };

		/**
		 * @brief The number of type levels defined in @c job_types.
		 *
		 * Used in boundary checks to prevent out-of-range access into @c job_type_strings.
		 */
		constexpr size_t job_type_count = job_type_strings.size();

		/**
		 * @brief Compile-time check ensuring @c job_type_strings matches the @c job_types
		 * enum.
		 *
		 * If this static_assert fails, it usually means a new type value was added
		 * to @c job_types without updating @c job_type_strings.
		 */
		static_assert(job_type_count == static_cast<size_t>(job_types::Background) + 1,
					  "job_type_strings and job_types enum are out of sync");
	}

	/**
	 * @brief Converts a @c job_types value to its corresponding string representation.
	 * @param job_type The @c job_types value to convert.
	 * @return A @c std::string_view containing one of "REALTIME", "BATCH", "BACKGROUND",
	 *         or "UNKNOWN" if @p job_type is out of expected range.
	 *
	 * ### Example
	 * @code
	 * auto p = job_types::RealTime;
	 * std::string_view sv = to_string(p); // "REALTIME"
	 * @endcode
	 */
	[[nodiscard]] constexpr std::string_view to_string(job_types job_type)
	{
		auto index = static_cast<size_t>(job_type);
		return (index < job_detail::job_type_count) ? job_detail::job_type_strings[index]
														: "UNKNOWN";
	}

	/**
	 * @brief Returns a vector containing all possible @c job_types values.
	 * @return A @c std::vector<job_types> with [RealTime, Batch, Background].
	 *
	 * This function is useful when iterating over all defined types (e.g. for logging,
	 * UI selection, or testing).
	 *
	 * ### Example
	 * @code
	 * for (auto type : all_types()) {
	 *     std::cout << to_string(type) << std::endl;
	 * }
	 * // Output:
	 * // REALTIME
	 * // BATCH
	 * // BACKGROUND
	 * @endcode
	 */
	[[nodiscard]] inline auto all_types(void) -> std::vector<job_types>
	{
		return { job_types::RealTime, job_types::Batch, job_types::Background };
	}
} // namespace typed_thread_pool_module

// ----------------------------------------------------------------------------
// Formatter specializations for job_types
// ----------------------------------------------------------------------------

#ifdef USE_STD_FORMAT
/**
 * @brief Specialization of @c std::formatter for @c job_types using narrow strings.
 *
 * Allows code such as:
 * @code
 * std::string s = std::format("Type is {}", job_types::RealTime); // "Type is REALTIME"
 * @endcode
 */
template <>
struct std::formatter<typed_thread_pool_module::job_types>
	: std::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c job_types value as a narrow string.
	 * @tparam FormatContext The type of the format context.
	 * @param job_type The @c job_types enum value to format.
	 * @param ctx The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_thread_pool_module::job_types& job_type,
				FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(
			typed_thread_pool_module::to_string(job_type), ctx);
	}
};

/**
 * @brief Specialization of @c std::formatter for @c job_types using wide strings.
 *
 * Allows code such as:
 * @code
 * std::wstring ws = std::format(L"Type is {}", job_types::Batch); // L"Type is
 * BATCH"
 * @endcode
 */
template <>
struct std::formatter<typed_thread_pool_module::job_types, wchar_t>
	: std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a @c job_types value as a wide string.
	 * @tparam FormatContext The type of the format context.
	 * @param job_type The @c job_types enum value to format.
	 * @param ctx The wide-character format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_thread_pool_module::job_types& job_type,
				FormatContext& ctx) const
	{
		auto str = typed_thread_pool_module::to_string(job_type);
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};

#else // USE_STD_FORMAT

/**
 * @brief Specialization of fmt::formatter for @c job_types using narrow strings.
 *
 * Allows code such as:
 * @code
 * std::string s = fmt::format("Type is {}", job_types::RealTime); // "Type is REALTIME"
 * @endcode
 */
template <>
struct fmt::formatter<typed_thread_pool_module::job_types>
	: fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a @c job_types value as a narrow string using {fmt}.
	 * @tparam FormatContext The type of the format context.
	 * @param job_type The @c job_types enum value to format.
	 * @param ctx The format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_thread_pool_module::job_types& job_type,
				FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(
			typed_thread_pool_module::to_string(job_type), ctx);
	}
};

/**
 * @brief Specialization of fmt::formatter for @c job_types using wide strings.
 *
 * Allows code such as:
 * @code
 * std::wstring ws = fmt::format(L"Type is {}", job_types::Background); // L"Type is BACKGROUND"
 * @endcode
 */
template <>
struct fmt::formatter<typed_thread_pool_module::job_types, wchar_t>
	: fmt::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a @c job_types value as a wide string using {fmt}.
	 * @tparam FormatContext The type of the format context.
	 * @param job_type The @c job_types enum value to format.
	 * @param ctx The wide-character format context for the output.
	 * @return An iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const typed_thread_pool_module::job_types& job_type,
				FormatContext& ctx) const
	{
		auto str = typed_thread_pool_module::to_string(job_type);
		auto wstr = convert_string::to_wstring(str);
		return fmt::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};
#endif