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

#include "container/values/string_value.h"

#include "utilities/core/convert_string.h"

namespace container_module
{
	using namespace utility_module;

	string_value::string_value() : value()
	{
		type_ = value_types::string_value;
		size_ = 0;
	}

	string_value::string_value(const std::string& name, const std::string& val)
		: string_value()
	{
		name_ = name;
		// Store as internal raw data with placeholders replaced
		std::vector<uint8_t> arr = convert_specific_string(val);
		data_ = arr;
		size_ = arr.size();
	}

	std::string string_value::to_string(const bool& original) const
	{
		if (!original)
		{
			// Return the raw data as-is, ignoring placeholder expansions
			auto [plain, err]
				= utility_module::convert_string::to_string(data_);
			if (!err.empty())
			{
				return "";
			}
			return plain;
		}
		// Return the placeholder-free version
		return convert_specific_string(data_);
	}
} // namespace container_module