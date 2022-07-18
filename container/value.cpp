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

#include "value.h"

#include "converting.h"

#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/double_value.h"
#include "values/float_value.h"
#include "values/int_value.h"
#include "values/long_value.h"
#include "values/ulong_value.h"
#include "values/llong_value.h"
#include "values/ullong_value.h"
#include "values/short_value.h"
#include "values/string_value.h"
#include "values/uint_value.h"
#include "values/ushort_value.h"
#include "values/container_value.h"

#include <sstream>

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	using namespace converting;

	value::value(void) : _name(L""), _type(value_types::null_value), _size(0)
	{
		_data_type_map.insert({ value_types::bool_value, bind(&value::set_boolean, this, placeholders::_1) });
		_data_type_map.insert({ value_types::short_value, bind(&value::set_short, this, placeholders::_1) });
		_data_type_map.insert({ value_types::ushort_value, bind(&value::set_ushort, this, placeholders::_1) });
		_data_type_map.insert({ value_types::int_value, bind(&value::set_int, this, placeholders::_1) });
		_data_type_map.insert({ value_types::uint_value, bind(&value::set_uint, this, placeholders::_1) });
		_data_type_map.insert({ value_types::long_value, bind(&value::set_long, this, placeholders::_1) });
		_data_type_map.insert({ value_types::ulong_value, bind(&value::set_ulong, this, placeholders::_1) });
		_data_type_map.insert({ value_types::llong_value, bind(&value::set_llong, this, placeholders::_1) });
		_data_type_map.insert({ value_types::ullong_value, bind(&value::set_ullong, this, placeholders::_1) });
		_data_type_map.insert({ value_types::float_value, bind(&value::set_float, this, placeholders::_1) });
		_data_type_map.insert({ value_types::double_value, bind(&value::set_double, this, placeholders::_1) });
		_data_type_map.insert({ value_types::bytes_value, bind(&value::set_byte_string, this, placeholders::_1) });
		_data_type_map.insert({ value_types::string_value, bind(&value::set_string, this, placeholders::_1) });
		_data_type_map.insert({ value_types::container_value, bind(&value::set_long, this, placeholders::_1) });
	}

	value::value(shared_ptr<value> object) : value()
	{
		if(object == nullptr)
		{
			_name = L"";
			_type = value_types::null_value;
			_size = 0;
			
			return;
		}

		_name = object->name();
		_type = object->type();
		_size = object->size();
		_parent = object->parent();
		_units = object->children();
	}

	value::value(const wstring& name, const vector<shared_ptr<value>>& units)
		: value()
	{
		_name = name;
		_units = units;

		long size = static_cast<long>(_units.size());
		set_data((const unsigned char*)&size, sizeof(long), value_types::container_value);
	}

	value::value(const wstring& name, const value_types& type, const wstring& data)
		: value()
	{
		set_data(name, type, data);
	}

	value::value(const wstring& name, const unsigned char* data, const size_t& size, const value_types& type)
		: value()
	{
		_name = name;
		set_data(data, size, type);
	}

	value::~value(void)
	{
	}

	shared_ptr<value> value::get_ptr(void)
	{
		return shared_from_this();
	}

	void value::set_parent(shared_ptr<value> parent)
	{
		_parent = parent;
	}

	void value::set_data(const unsigned char* data, const size_t& size, const value_types& type)
	{
		if (data == nullptr || size == 0)
		{
			_type = type;
			_size = 0;
			_data.clear();
			return;
		}

		_type = type;
		_size = size;
		_data = vector<uint8_t>(data, data + size);
	}

	void value::set_data(const wstring& name, const value_types& type, const wstring& data)
	{
		_name = name;
		_type = type;

		auto target = _data_type_map.find(_type);
		if (target == _data_type_map.end())
		{
			_data.clear();
			return;
		}

		target->second(data);
	}

	wstring value::name(void) const
	{
		return _name;
	}

	value_types value::type(void) const
	{
		return _type;
	}

	wstring value::data(void) const
	{
		if (_type != value_types::string_value)
		{
			return to_string();
		}
		
		return convert_specific_string(_data);
	}

	size_t value::size(void) const
	{
		return _data.size();
	}

	shared_ptr<value> value::parent(void)
	{
		shared_ptr<value> parent = _parent.lock();

		return parent;
	}

	size_t value::child_count(void) const
	{
		return _units.size();
	}

	vector<shared_ptr<value>> value::children(const bool& only_container)
	{
		if (!only_container)
		{
			return _units;
		}

		vector<shared_ptr<value>> result_list;

		for (auto& unit : _units)
		{
			if (unit->is_container())
			{
				result_list.push_back(unit);
			}
		};

		return result_list;
	}

	vector<shared_ptr<value>> value::value_array(const wstring& key)
	{
		vector<shared_ptr<value>> result_list;

		for (auto& unit : _units)
		{
			if (unit->name() == key)
			{
				result_list.push_back(unit);
			}
		};

		return result_list;
	}

	const vector<uint8_t> value::to_bytes(void) const
	{
		return _data;
	}

	bool value::is_null(void) const
	{
		return _type == value_types::null_value;
	}

	bool value::is_bytes(void) const
	{
		return _type == value_types::bytes_value;
	}

	bool value::is_boolean(void) const
	{
		return _type == value_types::bool_value;
	}

	bool value::is_numeric(void) const
	{
		return _type == value_types::short_value || _type == value_types::ushort_value ||
			_type == value_types::int_value || _type == value_types::uint_value ||
			_type == value_types::long_value || _type == value_types::ulong_value ||
			_type == value_types::llong_value || _type == value_types::ullong_value ||
			_type == value_types::float_value || _type == value_types::double_value;
	}

	bool value::is_string(void) const
	{
		return _type == value_types::string_value;
	}

	bool value::is_container(void) const
	{
		return _type == value_types::container_value;
	}

	const wstring value::to_xml(void)
	{
		wstring result;

		if (_units.size() == 0)
		{
			fmt::format_to(back_inserter(result), L"<{0}>{1}</{0}>", name(), to_string(false));

			return result;
		}

		fmt::format_to(back_inserter(result), L"<{}>", name());
		for (auto& unit : _units)
		{
			fmt::format_to(back_inserter(result), L"{}", unit->to_xml());
		}
		fmt::format_to(back_inserter(result), L"</{}>", name());

		return result;
	}

	const wstring value::to_json(void)
	{
		wstring result;

		if (_units.size() == 0)
		{
			switch (_type)
			{
			case value_types::bytes_value: 
			case value_types::string_value: 
				fmt::format_to(back_inserter(result), L"\"{}\":\"{}\"", name(), to_string(false)); break;
			default:
				fmt::format_to(back_inserter(result), L"\"{}\":{}", name(), to_string(false)); break;
			}

			return result;
		}

		fmt::format_to(back_inserter(result), L"\"{}\":{}", name(), L"{");

		bool first = true;
		for (auto& unit : _units)
		{
			fmt::format_to(back_inserter(result), first ? L"{}" : L",{}", unit->to_json());
			first = false;
		}

		fmt::format_to(back_inserter(result), L"{}", L"}");
		
		return result;
	}

	const wstring value::serialize(void)
	{
		wstring result;

		fmt::format_to(back_inserter(result), L"[{},{},{}];", name(), convert_value_type(_type), to_string(false));

		for (auto& unit : _units)
		{
			fmt::format_to(back_inserter(result), L"{}", unit->serialize());
		}

		return result;
	}

	shared_ptr<value> value::operator[](const wstring& key)
	{
		vector<shared_ptr<value>> searched_values = value_array(key);
		if (searched_values.empty())
		{
			return make_shared<value>(key);
		}

		return searched_values[0];
	}

	shared_ptr<value> operator<<(shared_ptr<value> container, shared_ptr<value> other)
	{
		container->add(other);

		return container;
	}

	ostream& operator<<(ostream& out, shared_ptr<value> other) // output
	{
		out << converter::to_string(other->serialize());

		return out;
	}

	wostream& operator<<(wostream& out, shared_ptr<value> other) // output
	{
		out << other->serialize();

		return out;
	}

	string& operator<<(string& out, shared_ptr<value> other)
	{
		out = converter::to_string(other->serialize());

		return out;
	}

	wstring& operator<<(wstring& out, shared_ptr<value> other)
	{
		out = other->serialize();

		return out;
	}

	wstring value::convert_specific_string(const vector<uint8_t>& data) const
	{
		wstring temp = converter::to_wstring(data);
		converter::replace(temp, L"</0x0A;>", L"\r");
		converter::replace(temp, L"</0x0B;>", L"\n");
		converter::replace(temp, L"</0x0C;>", L" ");
		converter::replace(temp, L"</0x0D;>", L"\t");

		return temp;
	}

	vector<uint8_t> value::convert_specific_string(wstring data) const
	{
		converter::replace(data, L"\r", L"</0x0A;>");
		converter::replace(data, L"\n", L"</0x0B;>");
		converter::replace(data, L" ", L"</0x0C;>");
		converter::replace(data, L"\t", L"</0x0D;>");

		return converter::to_array(data);
	}

	template <typename T> void value::set_data(T data)
	{
		char* data_ptr = (char*)&data;

		_size = sizeof(T);
		_data = vector<uint8_t>(data_ptr, data_ptr + _size);
	}

	void value::set_byte_string(const wstring& data)
	{
		_data = converter::from_base64(data);
		_size = _data.size();
		_type = value_types::bytes_value;
	}

	void value::set_string(const wstring& data)
	{
		_data = converter::to_array(data);
		_size = _data.size();
		_type = value_types::string_value;
	}

	void value::set_boolean(const wstring& data)
	{
		set_data((data == L"true") ? true : false);
		_type = value_types::bool_value;
	}

	void value::set_short(const wstring& data)
	{
		set_data((short)atoi(converter::to_string(data).c_str()));
	}

	void value::set_ushort(const wstring& data)
	{
		set_data((unsigned short)atoi(converter::to_string(data).c_str()));
	}
	
	void value::set_int(const wstring& data)
	{
		set_data((int)atoi(converter::to_string(data).c_str()));
	}
	
	void value::set_uint(const wstring& data)
	{
		set_data((unsigned int)atoi(converter::to_string(data).c_str()));
	}
	
	void value::set_long(const wstring& data)
	{
		set_data((long)atol(converter::to_string(data).c_str()));
	}
	
	void value::set_ulong(const wstring& data)
	{
		set_data((unsigned long)atol(converter::to_string(data).c_str()));
	}
	
	void value::set_llong(const wstring& data)
	{
		set_data((long long)atoll(converter::to_string(data).c_str()));
	}
	
	void value::set_ullong(const wstring& data)
	{
		set_data((unsigned long long)atoll(converter::to_string(data).c_str()));
	}
	
	void value::set_float(const wstring& data)
	{
		set_data((float)atof(converter::to_string(data).c_str()));
	}
	
	void value::set_double(const wstring& data)
	{
		set_data((double)atof(converter::to_string(data).c_str()));
	}
}