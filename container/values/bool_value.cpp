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

#include "bool_value.h"

#include <cstring>

#ifdef __USE_TYPE_CONTAINER__

namespace container
{
	bool_value::bool_value(void)
		: value()
	{
		_type = value_types::bool_value;
	}

	bool_value::bool_value(const wstring& name, const bool& value)
		: bool_value()
	{
		set_data(name, value_types::bool_value, value ? L"true" : L"false");
	}

	bool_value::bool_value(const wstring& name, const wstring& value)
		: bool_value()
	{
		set_data(name, value_types::bool_value, value);
	}

	bool_value::~bool_value(void)
	{
	}

	bool bool_value::to_boolean(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return temp;
	}

	short bool_value::to_short(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short bool_value::to_ushort(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int bool_value::to_int(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int bool_value::to_uint(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long bool_value::to_long(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long bool_value::to_ulong(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long bool_value::to_llong(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long bool_value::to_ullong(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float bool_value::to_float(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double bool_value::to_double(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring bool_value::to_string(const bool&) const
	{
		return (to_boolean() ? L"true" : L"false");
	}
}

#endif
