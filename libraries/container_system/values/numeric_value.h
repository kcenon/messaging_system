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

#include "../core/value.h"
#include "../core/value_types.h"

#include <cstring>
#include <type_traits>
#include <string>

namespace container_module
{
	/**
	 * @class numeric_value
	 * @brief A template class for storing and converting numeric types
	 *        (integral or floating).
	 *
	 * @tparam T       A numeric C++ type (e.g. int, float, double, short, etc.)
	 * @tparam TypeTag A value_types enum that must match T (e.g.
	 * value_types::int_value).
	 */
	template <typename T, value_types TypeTag>
	class numeric_value : public value
	{
	public:
		/**
		 * @brief Default constructor sets the appropriate type and
		 * zero-initializes data.
		 */
		numeric_value(void);

		/**
		 * @brief Constructs a numeric_value with the given name and initial
		 * numeric content.
		 * @param name The name assigned to the value.
		 * @param initial_value The numeric value to store.
		 */
		numeric_value(const std::string& name, const T& initial_value);

		virtual ~numeric_value(void) = default;

		/// @name Overridden conversion methods
		/// @{
		bool to_boolean(void) const override;
		short to_short(void) const override;
		unsigned short to_ushort(void) const override;
		int to_int(void) const override;
		unsigned int to_uint(void) const override;
		long to_long(void) const override;
		unsigned long to_ulong(void) const override;
		long long to_llong(void) const override;
		unsigned long long to_ullong(void) const override;
		float to_float(void) const override;
		double to_double(void) const override;
		/// @}

		/**
		 * @brief Retrieve a string representation of the stored numeric value.
		 * @param original If true, you may return a standard string form.
		 * @return The numeric as a string (e.g. "123" or "45.67").
		 */
		std::string to_string(const bool& original = true) const override;

	private:
		/**
		 * @brief Extract the numeric T from data_ via memcpy.
		 * @return The value of type T.
		 */
		T get_value(void) const;
	};

	/**
	 * @brief Using-alias definitions for common numeric types.
	 *
	 * These aliases let you write short_value, int_value, etc. instead
	 * of numeric_value<short, value_types::short_value> directly.
	 */
	using short_value = numeric_value<short, value_types::short_value>;
	using ushort_value
		= numeric_value<unsigned short, value_types::ushort_value>;
	using int_value = numeric_value<int, value_types::int_value>;
	using uint_value = numeric_value<unsigned int, value_types::uint_value>;
	using long_value = numeric_value<long, value_types::long_value>;
	using ulong_value = numeric_value<unsigned long, value_types::ulong_value>;
	using llong_value = numeric_value<long long, value_types::llong_value>;
	using ullong_value
		= numeric_value<unsigned long long, value_types::ullong_value>;
	using float_value = numeric_value<float, value_types::float_value>;
	using double_value = numeric_value<double, value_types::double_value>;
} // namespace container_module

#include "numeric_value.tpp"
