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

/**
 * @file numeric_value.tpp
 * @brief Template definitions for numeric_value.
 *
 * This file must be included at the end of numeric_value.h
 * so that all template definitions are visible to the compiler.
 */

#pragma once

#include <cstring>
#include <type_traits>
#include <string>

namespace container
{
	template <typename T, value_types TypeTag>
	numeric_value<T, TypeTag>::numeric_value(void) : value()
	{
		type_ = TypeTag;
		size_ = sizeof(T);
		data_.resize(size_, 0);
	}

	template <typename T, value_types TypeTag>
	numeric_value<T, TypeTag>::numeric_value(const std::string& name,
											 const T& initial_value)
		: numeric_value()
	{
		name_ = name;
		std::memcpy(data_.data(), &initial_value, sizeof(T));
	}

	template <typename T, value_types TypeTag>
	T numeric_value<T, TypeTag>::get_value(void) const
	{
		T temp{};
		if (data_.size() >= sizeof(T))
		{
			std::memcpy(&temp, data_.data(), sizeof(T));
		}
		return temp;
	}

	template <typename T, value_types TypeTag>
	bool numeric_value<T, TypeTag>::to_boolean(void) const
	{
		T v = get_value();
		if constexpr (std::is_floating_point<T>::value)
		{
			return (v != static_cast<T>(0.0));
		}
		else
		{
			return (v != 0);
		}
	}

	template <typename T, value_types TypeTag>
	short numeric_value<T, TypeTag>::to_short(void) const
	{
		return static_cast<short>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned short numeric_value<T, TypeTag>::to_ushort(void) const
	{
		return static_cast<unsigned short>(get_value());
	}

	template <typename T, value_types TypeTag>
	int numeric_value<T, TypeTag>::to_int(void) const
	{
		return static_cast<int>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned int numeric_value<T, TypeTag>::to_uint(void) const
	{
		return static_cast<unsigned int>(get_value());
	}

	template <typename T, value_types TypeTag>
	long numeric_value<T, TypeTag>::to_long(void) const
	{
		return static_cast<long>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned long numeric_value<T, TypeTag>::to_ulong(void) const
	{
		return static_cast<unsigned long>(get_value());
	}

	template <typename T, value_types TypeTag>
	long long numeric_value<T, TypeTag>::to_llong(void) const
	{
		return static_cast<long long>(get_value());
	}

	template <typename T, value_types TypeTag>
	unsigned long long numeric_value<T, TypeTag>::to_ullong(void) const
	{
		return static_cast<unsigned long long>(get_value());
	}

	template <typename T, value_types TypeTag>
	float numeric_value<T, TypeTag>::to_float(void) const
	{
		return static_cast<float>(get_value());
	}

	template <typename T, value_types TypeTag>
	double numeric_value<T, TypeTag>::to_double(void) const
	{
		return static_cast<double>(get_value());
	}

	template <typename T, value_types TypeTag>
	std::string numeric_value<T, TypeTag>::to_string(
		const bool& /*original*/) const
	{
		T val = get_value();
		if constexpr (std::is_floating_point<T>::value)
		{
			// For float/double => might show extra decimals, but it's fine
			// as a default. Use e.g. <format> if you want custom format.
			return std::to_string(val);
		}
		else
		{
			// For integers => cast to long long first, then use std::to_string.
			long long casted = static_cast<long long>(val);
			return std::to_string(casted);
		}
	}
} // namespace container
