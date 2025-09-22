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

#include "utilities/core/formatter.h"
#include "utilities/conversion/convert_string.h"

#include <string>
#include <array>
#include <cstdint>
#include <string_view>

/**
 * @enum test_priority
 * @brief Enumeration of test priority levels.
 *
 * Defines priority levels for test cases. This enum class is based on uint8_t
 * to optimize storage, with three priority levels: Top, Middle, and Bottom.
 */
enum class test_priority : uint8_t
{
	Top = 0, ///< Top priority
	Middle,	 ///< Middle priority
	Bottom	 ///< Bottom priority
};

namespace test_detail
{
	/**
	 * @brief Array of string representations for each test priority level.
	 */
	constexpr std::array test_priority_strings = { "Top", "Middle", "Bottom" };

	/**
	 * @brief Total number of test types available in test_priority_strings.
	 */
	constexpr size_t test_priority_count = test_priority_strings.size();

	// Compile-time check to ensure test_priority_strings and test_priority are in sync
	static_assert(test_priority_count == static_cast<size_t>(test_priority::Bottom) + 1,
				  "test_priority_strings and test_priority enum are out of sync");
}

/**
 * @brief Converts a test_priority value to its string representation.
 * @param priority The test_priority value to convert.
 * @return std::string_view String representation of the test priority.
 */
[[nodiscard]] constexpr std::string_view to_string(test_priority priority)
{
	auto index = static_cast<size_t>(priority);
	return (index < test_detail::test_priority_count) ? test_detail::test_priority_strings[index]
													  : "Unknown";
}

/**
 * @brief Converts a string to a job_types value.
 * @return std::vector<test_priority>
 */
[[nodiscard]] inline auto all_types(void) -> std::vector<test_priority>
{
	return { test_priority::Top, test_priority::Middle, test_priority::Bottom };
}

// Formatter specializations for test_priority
#ifdef USE_STD_FORMAT
/**
 * @brief Specialization of std::formatter for test_priority.
 * Enables formatting of test_priority enum values as strings in the standard library format.
 */
template <> struct std::formatter<test_priority> : std::formatter<std::string_view>
{
	/**
	 * @brief Formats a test_priority value as a string.
	 * @tparam FormatContext Type of the format context.
	 * @param priority The test_priority enum value to format.
	 * @param ctx Format context for the output.
	 * @return Iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const test_priority& priority, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(to_string(priority), ctx);
	}
};

/**
 * @brief Specialization of std::formatter for wide-character test_priority.
 * Allows test_priority enum values to be formatted as wide strings in the standard library format.
 */
template <>
struct std::formatter<test_priority, wchar_t> : std::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a test_priority value as a wide string.
	 * @tparam FormatContext Type of the format context.
	 * @param priority The test_priority enum value to format.
	 * @param ctx Format context for the output.
	 * @return Iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const test_priority& priority, FormatContext& ctx) const
	{
		auto str = to_string(priority);
		auto wstr = convert_string::to_wstring(str);
		return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};
#else
/**
 * @brief Specialization of fmt::formatter for test_priority.
 * Enables formatting of test_priority enum values using the fmt library.
 */
template <> struct fmt::formatter<test_priority> : fmt::formatter<std::string_view>
{
	/**
	 * @brief Formats a test_priority value as a string.
	 * @tparam FormatContext Type of the format context.
	 * @param priority The test_priority enum value to format.
	 * @param ctx Format context for the output.
	 * @return Iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const test_priority& priority, FormatContext& ctx) const
	{
		return fmt::formatter<std::string_view>::format(to_string(priority), ctx);
	}
};

/**
 * @brief Specialization of fmt::formatter for wide-character test_priority.
 * Allows test_priority enum values to be formatted as wide strings in the standard library format.
 */
template <>
struct fmt::formatter<test_priority, wchar_t> : fmt::formatter<std::wstring_view, wchar_t>
{
	/**
	 * @brief Formats a test_priority value as a wide string.
	 * @tparam FormatContext Type of the format context.
	 * @param priority The test_priority enum value to format.
	 * @param ctx Format context for the output.
	 * @return Iterator to the end of the formatted output.
	 */
	template <typename FormatContext>
	auto format(const test_priority& priority, FormatContext& ctx) const
	{
		auto str = to_string(priority);
		auto wstr = convert_string::to_wstring(str);
		return fmt::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);
	}
};
#endif