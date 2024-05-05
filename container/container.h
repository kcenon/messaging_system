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

#include "value.h"

#include <memory>
#include <vector>

using namespace std;

namespace container
{
	class value_container : public enable_shared_from_this<value_container>
	{
	public:
		value_container(void);
		value_container(const wstring& data_string,
						const bool& parse_only_header = true);
		value_container(const vector<uint8_t>& data_array,
						const bool& parse_only_header = true);
		value_container(const value_container& data_container,
						const bool& parse_only_header = true);
		value_container(shared_ptr<value_container> data_container,
						const bool& parse_only_header = true);
		value_container(const wstring& message_type,
						const vector<shared_ptr<value>>& units);
		value_container(const wstring& target_id,
						const wstring& target_sub_id,
						const wstring& message_type,
						const vector<shared_ptr<value>>& units);
		value_container(const wstring& source_id,
						const wstring& source_sub_id,
						const wstring& target_id,
						const wstring& target_sub_id,
						const wstring& message_type,
						const vector<shared_ptr<value>>& units);
		virtual ~value_container(void);

	public:
		shared_ptr<value_container> get_ptr(void);

	public:
		void set_source(const wstring& source_id, const wstring& source_sub_id);
		void set_target(const wstring& target_id,
						const wstring& target_sub_id = L"");
		void set_message_type(const wstring& message_type);
		void set_units(const vector<shared_ptr<value>>& target_values,
					   const bool& update_immediately = false);

	public:
		void swap_header(void);
		void clear_value(void);
		shared_ptr<value_container> copy(const bool& containing_values = true);

	public:
		wstring source_id(void) const;
		wstring source_sub_id(void) const;
		wstring target_id(void) const;
		wstring target_sub_id(void) const;
		wstring message_type(void) const;

	public:
		shared_ptr<value> add(const value& target_value,
							  const bool& update_immediately = false);
		shared_ptr<value> add(shared_ptr<value> target_value,
							  const bool& update_immediately = false);
		void remove(const wstring& target_name,
					const bool& update_immediately = false);
		void remove(shared_ptr<value> target_value,
					const bool& update_immediately = false);
		vector<shared_ptr<value>> value_array(const wstring& target_name);
		shared_ptr<value> get_value(const wstring& target_name,
									const unsigned int& index = 0);

	public:
		void initialize(void);

	public:
		wstring serialize(void) const;
		vector<uint8_t> serialize_array(void) const;
		bool deserialize(const wstring& data_string,
						 const bool& parse_only_header = true);
		bool deserialize(const vector<uint8_t>& data_array,
						 const bool& parse_only_header = true);

	public:
		const wstring to_xml(void);
		const wstring to_json(void);

	public:
		wstring datas(void) const;

	public:
		void load_packet(const wstring& file_path);
		void save_packet(const wstring& file_path);

	public:
		vector<shared_ptr<value>> operator[](const wstring& key);

		friend value_container operator<<(value_container target_container,
										  value& other);
		friend value_container operator<<(value_container target_container,
										  shared_ptr<value> other);
		friend shared_ptr<value_container> operator<<(
			shared_ptr<value_container> target_container, value& other);
		friend shared_ptr<value_container> operator<<(
			shared_ptr<value_container> target_container,
			shared_ptr<value> other);

		friend ostream& operator<<(ostream& out, value_container& other);
		friend ostream& operator<<(ostream& out,
								   shared_ptr<value_container> other);
		friend wostream& operator<<(wostream& out, value_container& other);
		friend wostream& operator<<(wostream& out,
									shared_ptr<value_container> other);

		friend string& operator<<(string& out, value_container& other);
		friend string& operator<<(string& out,
								  shared_ptr<value_container> other);
		friend wstring& operator<<(wstring& out, value_container& other);
		friend wstring& operator<<(wstring& out,
								   shared_ptr<value_container> other);

	protected:
		bool deserialize_values(const wstring& data,
								const bool& parse_only_header = true);
		void parsing(const wstring& source_name,
					 const wstring& target_name,
					 const wstring& target_value,
					 wstring& target_variable);

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
		shared_ptr<value> set_bytes(const wstring& name, const wstring& data);
		shared_ptr<value> set_string(const wstring& name, const wstring& data);
		shared_ptr<value> set_container(const wstring& name,
										const wstring& data);

	private:
		bool _parsed_data;
		bool _changed_data;
		wstring _data_string;

	private:
		wstring _source_id;
		wstring _source_sub_id;
		wstring _target_id;
		wstring _target_sub_id;
		wstring _message_type;
		wstring _version;
		vector<shared_ptr<value>> _units;

	private:
		map<value_types,
			function<shared_ptr<value>(const wstring&, const wstring&)>>
			_data_type_map;
	};
} // namespace container