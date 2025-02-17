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

#include "value_types.h"
#include <map>

namespace container
{
	static std::map<std::string, value_types> value_to_type_map{
		{ "1", value_types::bool_value },
		{ "2", value_types::short_value },
		{ "3", value_types::ushort_value },
		{ "4", value_types::int_value },
		{ "5", value_types::uint_value },
		{ "6", value_types::long_value },
		{ "7", value_types::ulong_value },
		{ "8", value_types::llong_value },
		{ "9", value_types::ullong_value },
		{ "a", value_types::float_value },
		{ "b", value_types::double_value },
		{ "c", value_types::bytes_value },
		{ "d", value_types::string_value },
		{ "e", value_types::container_value },
	};

	static std::map<value_types, std::string> type_to_value_map{
		{ value_types::bool_value, "1" },
		{ value_types::short_value, "2" },
		{ value_types::ushort_value, "3" },
		{ value_types::int_value, "4" },
		{ value_types::uint_value, "5" },
		{ value_types::long_value, "6" },
		{ value_types::ulong_value, "7" },
		{ value_types::llong_value, "8" },
		{ value_types::ullong_value, "9" },
		{ value_types::float_value, "a" },
		{ value_types::double_value, "b" },
		{ value_types::bytes_value, "c" },
		{ value_types::string_value, "d" },
		{ value_types::container_value, "e" }
	};

	value_types convert_value_type(const std::string& target)
	{
		auto it = value_to_type_map.find(target);
		if (it == value_to_type_map.end())
		{
			return value_types::null_value;
		}
		return it->second;
	}

	std::string convert_value_type(const value_types& target)
	{
		auto it = type_to_value_map.find(target);
		if (it == type_to_value_map.end())
		{
			return "0";
		}
		return it->second;
	}
} // namespace container
