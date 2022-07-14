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

#include "../value.h"

using namespace std;

namespace container
{
	class container_value : public value
	{
	public:
		container_value(void);
		container_value(const wstring& name, const long& reserved_count);
		container_value(const wstring& name, const vector<shared_ptr<value>>& units = {});
		~container_value(void);

	public:
		shared_ptr<value> add(const value& item, const bool& update_count = true) override;
		shared_ptr<value> add(shared_ptr<value> item, const bool& update_count = true) override;
		void add(const vector<value>& target_values, const bool& update_count = true) override;
		void add(const vector<shared_ptr<value>>& target_values, const bool& update_count = true) override;
		void remove(const wstring& target_name, const bool& update_count = true) override;
		void remove(shared_ptr<value> item, const bool& update_count = true) override;
		void remove_all(void) override;

	public:
		long to_long(void) const override;
		wstring to_string(const bool& original = true) const override;

	private:
		shared_ptr<value> set_boolean(const wstring& name, const wstring& data);
		shared_ptr<value> set_short(const wstring& name, const wstring& data);
		shared_ptr<value> set_ushort(const wstring& name, const wstring& data);
		shared_ptr<value> set_int(const wstring& name, const wstring& data);
		shared_ptr<value> set_uint(const wstring& name, const wstring& data);
		shared_ptr<value> set_long(const wstring& name, const wstring& data);
		shared_ptr<value> set_ulong(const wstring& name, const wstring& data);
		shared_ptr<value> set_llong(const wstring& name, const wstring& data);
		shared_ptr<value> set_ullong(const wstring& name, const wstring& data);
		shared_ptr<value> set_float(const wstring& name, const wstring& data);
		shared_ptr<value> set_double(const wstring& name, const wstring& data);
		shared_ptr<value> set_byte_string(const wstring& name, const wstring& data);
		shared_ptr<value> set_string(const wstring& name, const wstring& data);
		shared_ptr<value> set_container(const wstring& name, const wstring& data);

	private:
		map<value_types, function<shared_ptr<value>(const wstring&, const wstring&)>> _data_type_map;
	};
}