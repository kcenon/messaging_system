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

#include "bytes_value.h"

#include "utilities/core/convert_string.h"

namespace container_module
{
	using namespace utility_module;

	bytes_value::bytes_value() : value()
	{
		type_ = value_types::bytes_value;
		size_ = 0;
	}

	bytes_value::bytes_value(const std::string& name,
							 const std::vector<uint8_t>& dataVec)
		: bytes_value()
	{
		name_ = name;
		data_ = dataVec;
		size_ = data_.size();
	}

	bytes_value::bytes_value(const std::string& name,
							 const unsigned char* dataPtr,
							 size_t sz)
		: bytes_value()
	{
		name_ = name;
		if (dataPtr && sz > 0)
		{
			data_.assign(dataPtr, dataPtr + sz);
			size_ = data_.size();
		}
	}

	std::string bytes_value::to_string(const bool&) const
	{
		// Convert the raw data to base64
		auto [encoded, err] = convert_string::to_base64(data_);
		if (!err.empty())
		{
			return "";
		}
		return encoded;
	}
} // namespace container_module