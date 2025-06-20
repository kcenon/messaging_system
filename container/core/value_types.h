/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
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

#include <string>
#include <string_view>
#include <array>
#include <utility>

namespace container_module
{
	/**
	 * @brief Enumeration of available value types in the container system.
	 */
	enum class value_types {
		null_value,
		bool_value,
		short_value,
		ushort_value,
		int_value,
		uint_value,
		long_value,
		ulong_value,
		llong_value,
		ullong_value,
		float_value,
		double_value,
		bytes_value,
		string_value,
		container_value
	};

	// Compile-time type mapping
	constexpr std::array<std::pair<std::string_view, value_types>, 15> type_map{{
		{"0", value_types::null_value},
		{"1", value_types::bool_value},
		{"2", value_types::short_value},
		{"3", value_types::ushort_value},
		{"4", value_types::int_value},
		{"5", value_types::uint_value},
		{"6", value_types::long_value},
		{"7", value_types::ulong_value},
		{"8", value_types::llong_value},
		{"9", value_types::ullong_value},
		{"10", value_types::float_value},
		{"11", value_types::double_value},
		{"12", value_types::bytes_value},
		{"13", value_types::string_value},
		{"14", value_types::container_value}
	}};

	/**
	 * @brief Compile-time conversion from string to value_types
	 */
	constexpr value_types get_type_from_string(std::string_view str) noexcept
	{
		for (const auto& [key, type] : type_map)
		{
			if (key == str) return type;
		}
		return value_types::null_value;
	}

	/**
	 * @brief Compile-time conversion from value_types to string
	 */
	constexpr std::string_view get_string_from_type(value_types type) noexcept
	{
		for (const auto& [key, val] : type_map)
		{
			if (val == type) return key;
		}
		return "0";
	}

	/**
	 * @brief Convert a string-based type indicator (e.g., "4") to a value_types
	 * enum.
	 * @param target The string indicator.
	 * @return The corresponding value_types. Returns null_value if not found.
	 */
	value_types convert_value_type(const std::string& target);

	/**
	 * @brief Convert a value_types enum to its associated string indicator
	 * (e.g., "4").
	 * @param target The value_types enum.
	 * @return The string. Returns "0" if not found.
	 */
	std::string convert_value_type(const value_types& target);
} // namespace container_module
