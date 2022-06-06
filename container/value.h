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

#ifdef __USE_TYPE_CONTAINER__

#include "value_types.h"

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <functional>

using namespace std;

namespace container
{
	class value : public enable_shared_from_this<value>
	{
	public:
		value(void);
		value(shared_ptr<value> object);
		value(const wstring& name, const vector<shared_ptr<value>>& units = {});
		value(const wstring& name, const value_types& type, const wstring& data);
		value(const wstring& name, const unsigned char* data, const size_t& size, const value_types& type = value_types::null_value);
		virtual ~value(void);

	public:
		shared_ptr<value> get_ptr(void);

	public:
		void set_parent(shared_ptr<value> parent);
		void set_data(const unsigned char* data, const size_t& size, const value_types& type);
		void set_data(const wstring& name, const value_types& type, const wstring& data);

	public:
		wstring name(void) const;
		value_types type(void) const;
		wstring data(void) const;
		size_t size(void) const;
		shared_ptr<value> parent(void);
		size_t child_count(void) const;
		vector<shared_ptr<value>> children(const bool& only_container = false);

	public:
		virtual shared_ptr<value> add(const value& item, const bool& update_count = true) { throw exception(logic_error("cannot add value object on this object")); }
		virtual shared_ptr<value> add(shared_ptr<value> item, const bool& update_count = true) { throw exception(logic_error("cannot add value object on this object")); }
		virtual void add(const vector<value>& target_values, const bool& update_count = true) { throw exception(logic_error("cannot add value objects on this object")); }
		virtual void add(const vector<shared_ptr<value>>& target_values, const bool& update_count = true) { throw exception(logic_error("cannot add value objects on this object")); }
		virtual void remove(const wstring& target_name, const bool& update_count = true) { throw exception(logic_error("cannot remove value objects on this object")); }
		virtual void remove(shared_ptr<value> item, const bool& update_count = true) { throw exception(logic_error("cannot remove value object on this object")); }
		virtual void remove_all(void) { throw exception(logic_error("cannot remove value object on this object")); }
		vector<shared_ptr<value>> value_array(const wstring& key);

	public:
		const vector<unsigned char> to_bytes(void) const;

	public:
		bool is_null(void) const;
		bool is_bytes(void) const;
		bool is_boolean(void) const;
		bool is_numeric(void) const;
		bool is_string(void) const;
		bool is_container(void) const;

	public:
		const wstring to_xml(void);
		const wstring to_json(void);

	public:
		const wstring serialize(void);

	public:
		virtual bool to_boolean(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual short to_short(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual unsigned short to_ushort(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual int to_int(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual unsigned int to_uint(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual long to_long(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual unsigned long to_ulong(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual long long to_llong(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual unsigned long long to_ullong(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual float to_float(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual double to_double(void) const { throw exception(logic_error("Not implemented yet!")); }
		virtual wstring to_string(const bool& original = true) const { throw exception(logic_error("Not implemented yet!")); }

	public:
		shared_ptr<value> operator[](const wstring& key);

		friend shared_ptr<value> operator<<(shared_ptr<value> container, shared_ptr<value> other);

		friend ostream& operator<<(ostream& out, shared_ptr<value> other);
		friend wostream& operator<<(wostream& out, shared_ptr<value> other);

		friend string& operator<<(string& out, shared_ptr<value> other);
		friend wstring& operator<<(wstring& out, shared_ptr<value> other);

	protected:
		wstring convert_specific_string(const vector<unsigned char>& data) const;
		vector<unsigned char> convert_specific_string(wstring data) const;

	protected:
		template <typename T> void set_data(T data);
		void set_byte_string(const wstring& data);
		void set_string(const wstring& data);
		void set_boolean(const wstring& data);
	
	private:
		void set_short(const wstring& data);
		void set_ushort(const wstring& data);
		void set_int(const wstring& data);
		void set_uint(const wstring& data);
		void set_long(const wstring& data);
		void set_ulong(const wstring& data);
		void set_llong(const wstring& data);
		void set_ullong(const wstring& data);
		void set_float(const wstring& data);
		void set_double(const wstring& data);

	protected:
		size_t _size;
		value_types _type;
		wstring _name;
		vector<unsigned char> _data;

	protected:
		weak_ptr<value> _parent;
		vector<shared_ptr<value>> _units;

	private:
		map<value_types, function<void(const wstring&)>> _data_type_map;
	};
}

#endif