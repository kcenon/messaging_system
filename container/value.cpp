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

#ifdef __USE_TYPE_CONTAINER__

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
	}

	value::value(shared_ptr<value> object)
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
			_type = value_types::null_value;
			_size = 0;
			_data.clear();
			return;
		}

		_type = type;
		_size = size;
		_data = vector<unsigned char>(data, data + size);
	}

	void value::set_data(const wstring& name, const value_types& type, const wstring& data)
	{
		_name = name;
		_type = type;

		switch (_type)
		{
		case value_types::bool_value: set_boolean(data); break;
		case value_types::short_value: set_data((short)atoi(converter::to_string(data).c_str())); break;
		case value_types::ushort_value: set_data((unsigned short)atoi(converter::to_string(data).c_str())); break;
		case value_types::int_value: set_data((int)atoi(converter::to_string(data).c_str())); break;
		case value_types::uint_value: set_data((unsigned int)atoi(converter::to_string(data).c_str())); break;
		case value_types::long_value: set_data((long)atol(converter::to_string(data).c_str())); break;
		case value_types::ulong_value: set_data((unsigned long)atol(converter::to_string(data).c_str())); break;
		case value_types::llong_value: set_data((long long)atoll(converter::to_string(data).c_str())); break;
		case value_types::ullong_value: set_data((unsigned long long)atoll(converter::to_string(data).c_str())); break;
		case value_types::float_value: set_data((float)atof(converter::to_string(data).c_str())); break;
		case value_types::double_value: set_data((double)atof(converter::to_string(data).c_str())); break;
		case value_types::bytes_value: set_byte_string(data); break;
		case value_types::string_value: set_string(data); break;
		case value_types::container_value: set_data((long)atol(converter::to_string(data).c_str())); break;
		default: break;
		}
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

		wstring temp = to_string();

		converter::replace(temp, L"</0x0A;>", L"\r");
		converter::replace(temp, L"</0x0B;>", L"\n");
		converter::replace(temp, L"</0x0C;>", L" ");
		converter::replace(temp, L"</0x0D;>", L"\t");

		return temp;
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

	const vector<unsigned char> value::to_bytes(void) const
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

	shared_ptr<value> value::generate_value(const wstring& target_name, const wstring& target_type, const wstring& target_value)
	{
		shared_ptr<value> result = nullptr;
		value_types current_type = convert_value_type(target_type);

		switch (current_type)
		{
#ifdef _WIN32
		case value_types::bool_value: result = make_shared<bool_value>(target_name, target_value); break;
		case value_types::short_value: result = make_shared<short_value>(target_name, (short)_wtoi(target_value.c_str())); break;
		case value_types::ushort_value: result = make_shared<ushort_value>(target_name, (unsigned short)_wtoi(target_value.c_str())); break;
		case value_types::int_value: result = make_shared<int_value>(target_name, (int)_wtoi(target_value.c_str())); break;
		case value_types::uint_value: result = make_shared<uint_value>(target_name, (unsigned int)_wtoi(target_value.c_str())); break;
		case value_types::long_value: result = make_shared<long_value>(target_name, (long)_wtol(target_value.c_str())); break;
		case value_types::ulong_value: result = make_shared<ulong_value>(target_name, (unsigned long)_wtol(target_value.c_str())); break;
		case value_types::llong_value: result = make_shared<llong_value>(target_name, (long long)_wtoll(target_value.c_str())); break;
		case value_types::ullong_value: result = make_shared<ullong_value>(target_name, (unsigned long long)_wtoll(target_value.c_str())); break;
		case value_types::float_value: result = make_shared<float_value>(target_name, (float)_wtof(target_value.c_str())); break;
		case value_types::double_value: result = make_shared<double_value>(target_name, (double)_wtof(target_value.c_str())); break;
		case value_types::bytes_value: result = make_shared<bytes_value>(target_name, converter::from_base64(target_value.c_str())); break;
		case value_types::string_value: result = make_shared<string_value>(target_name, target_value); break;
		case value_types::container_value: result = make_shared<container_value>(target_name, (long)_wtol(target_value.c_str())); break;
		default: result = make_shared<value>(target_name, nullptr, 0, value_types::null_value); break;
#else
		case value_types::bool_value: result = make_shared<bool_value>(target_name, target_value); break;
		case value_types::short_value: result = make_shared<short_value>(target_name, (short)atoi(converter::to_string(target_value).c_str())); break;
		case value_types::ushort_value: result = make_shared<ushort_value>(target_name, (unsigned short)atoi(converter::to_string(target_value).c_str())); break;
		case value_types::int_value: result = make_shared<int_value>(target_name, (int)atoi(converter::to_string(target_value).c_str())); break;
		case value_types::uint_value: result = make_shared<uint_value>(target_name, (unsigned int)atoi(converter::to_string(target_value).c_str())); break;
		case value_types::long_value: result = make_shared<long_value>(target_name, (long)atol(converter::to_string(target_value).c_str())); break;
		case value_types::ulong_value: result = make_shared<ulong_value>(target_name, (unsigned long)atol(converter::to_string(target_value).c_str())); break;
		case value_types::llong_value: result = make_shared<llong_value>(target_name, (long long)atoll(converter::to_string(target_value).c_str())); break;
		case value_types::ullong_value: result = make_shared<ullong_value>(target_name, (unsigned long long)atoll(converter::to_string(target_value).c_str())); break;
		case value_types::float_value: result = make_shared<float_value>(target_name, (float)atof(converter::to_string(target_value).c_str())); break;
		case value_types::double_value: result = make_shared<double_value>(target_name, (double)atof(converter::to_string(target_value).c_str())); break;
		case value_types::bytes_value: result = make_shared<bytes_value>(target_name, converter::from_base64(target_value.c_str())); break;
		case value_types::string_value: result = make_shared<string_value>(target_name, target_value); break;
		case value_types::container_value: result = make_shared<container_value>(target_name, (long)atol(converter::to_string(target_value).c_str())); break;
		default: result = make_shared<value>(target_name, nullptr, 0, value_types::null_value); break;
#endif
		}

		return result;
	}

	template <typename T> void value::set_data(T data)
	{
		char* data_ptr = (char*)&data;

		_size = sizeof(T);
		_data = vector<unsigned char>(data_ptr, data_ptr + _size);
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
}

#endif