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

#include "container_system/values/bool_value.h"

#include <cstring>

namespace container_module
{
	bool_value::bool_value() : value()
	{
		type_ = value_types::bool_value;
		// Default to false
		bool b = false;
		set_data(b);
	}

	bool_value::bool_value(const std::string& name, bool value) : bool_value()
	{
		name_ = name;
		set_data(value);
		type_ = value_types::bool_value;
	}

	bool_value::bool_value(const std::string& name, const std::string& valueStr)
		: bool_value()
	{
		name_ = name;
		bool b = (valueStr == "true");
		set_data(b);
		type_ = value_types::bool_value;
	}

	bool bool_value::to_boolean(void) const
	{
		bool temp = false;
		if (data_.size() == sizeof(bool))
		{
			std::memcpy(&temp, data_.data(), sizeof(bool));
		}
		return temp;
	}

	std::string bool_value::to_string(const bool&) const
	{
		return to_boolean() ? "true" : "false";
	}
} // namespace container_module
