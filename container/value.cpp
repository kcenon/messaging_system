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

#include "fmt/format.h"

namespace container
{
	using namespace converting;

	value::value(void) : _name(L""), _type(value_types::null_value), _size(0)
	{
	}

	value::value(std::shared_ptr<value> object)
	{
		_name = object->name();
		_type = object->type();
		_size = object->size();
		_parent = object->parent();
		_units = object->children();
	}

	value::value(const std::wstring& name, const std::vector<std::shared_ptr<value>>& units)
		: value()
	{
		_name = name;
		_units = units;

		long size = static_cast<long>(_units.size());
		set_data((const unsigned char*)&size, sizeof(long), value_types::container_value);
	}

	value::value(const std::wstring& name, const std::wstring& type, const std::wstring& data)
		: value()
	{
		set_data(name, type, data);
	}

	value::value(const std::wstring& name, const unsigned char* data, const size_t& size, const value_types& type)
		: value()
	{
		_name = name;
		set_data(data, size, type);
	}

	value::~value(void)
	{
	}

	std::shared_ptr<value> value::get_ptr(void)
	{
		return shared_from_this();
	}

	void value::set_parent(std::shared_ptr<value> parent)
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
		_data = std::vector<unsigned char>(data, data + size);
	}

	void value::set_data(const std::wstring& name, const std::wstring& type, const std::wstring& data)
	{
		_name = name;
		_type = convert_value_type(type);

		switch (_type)
		{
		case value_types::bool_value: set_boolean(data); break;
		case value_types::short_value: set_data((short)_wtoi(data.c_str())); break;
		case value_types::ushort_value: set_data((unsigned short)_wtoi(data.c_str())); break;
		case value_types::int_value: set_data((int)_wtoi(data.c_str())); break;
		case value_types::uint_value: set_data((unsigned int)_wtoi(data.c_str())); break;
		case value_types::long_value: set_data((long)_wtol(data.c_str())); break;
		case value_types::ulong_value: set_data((unsigned long)_wtol(data.c_str())); break;
		case value_types::llong_value: set_data((long long)_wtoll(data.c_str())); break;
		case value_types::ullong_value: set_data((unsigned long long)_wtoll(data.c_str())); break;
		case value_types::float_value: set_data((float)_wtof(data.c_str())); break;
		case value_types::double_value: set_data((double)_wtof(data.c_str())); break;
		case value_types::bytes_value: set_byte_string(data); break;
		case value_types::string_value: set_string(data); break;
		case value_types::container_value: set_data((long)_wtol(data.c_str())); break;
		}
	}

	std::wstring value::name(void) const
	{
		return _name;
	}

	value_types value::type(void) const
	{
		return _type;
	}

	std::wstring value::data(void) const
	{
		if (_type != value_types::string_value)
		{
			return to_string();
		}

		std::wstring temp = to_string();

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

	std::shared_ptr<value> value::parent(void)
	{
		std::shared_ptr<value> parent = _parent.lock();

		return parent;
	}

	size_t value::child_count(void) const
	{
		return _units.size();
	}

	std::vector<std::shared_ptr<value>> value::children(const bool& only_container)
	{
		if (!only_container)
		{
			return _units;
		}

		std::vector<std::shared_ptr<value>> result_list;

		std::for_each(_units.begin(), _units.end(), [&result_list](std::shared_ptr<value> source) 
			{
				if (source->is_container())
				{
					result_list.push_back(source);
				}
			});

		return result_list;
	}

	std::vector<std::shared_ptr<value>> value::value_array(const std::wstring& key)
	{
		std::vector<std::shared_ptr<value>> result_list;

		std::for_each(_units.begin(), _units.end(), [&key, &result_list](std::shared_ptr<value> source) 
			{
				if (source->name() == key)
				{
					result_list.push_back(source);
				}
			});

		return result_list;
	}

	const std::vector<unsigned char> value::to_bytes(void) const
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

	std::wstring value::serialize(void)
	{
		fmt::wmemory_buffer result;

		fmt::format_to(std::back_inserter(result), L"[{},{},{}];", name(), convert_value_type(_type), to_string(false));

		for (auto& unit : _units)
		{
			fmt::format_to(std::back_inserter(result), L"{}", unit->serialize());
		}

		return result.data();
	}

	std::shared_ptr<value> value::operator[](const std::wstring& key)
	{
		std::vector<std::shared_ptr<value>> searched_values = value_array(key);
		if (searched_values.empty())
		{
			return std::make_shared<value>(key);
		}

		return searched_values[0];
	}

	std::shared_ptr<value> operator<<(std::shared_ptr<value> container, std::shared_ptr<value> other)
	{
		container->add(other);

		return container;
	}

	std::ostream& operator<<(std::ostream& out, std::shared_ptr<value> other) // output
	{
		out << converter::to_string(other->serialize());

		return out;
	}

	std::wostream& operator<<(std::wostream& out, std::shared_ptr<value> other) // output
	{
		out << other->serialize();

		return out;
	}

	std::string& operator<<(std::string& out, std::shared_ptr<value> other)
	{
		out = converter::to_string(other->serialize());

		return out;
	}

	std::wstring& operator<<(std::wstring& out, std::shared_ptr<value> other)
	{
		out = other->serialize();

		return out;
	}

	std::shared_ptr<value> value::generate_value(const std::wstring& target_name, const std::wstring& target_type, const std::wstring& target_value)
	{
		std::shared_ptr<value> result = nullptr;
		value_types current_type = convert_value_type(target_type);

		switch (current_type)
		{
		case value_types::bool_value: result = std::make_shared<bool_value>(target_name, target_value); break;
		case value_types::short_value: result = std::make_shared<short_value>(target_name, (short)_wtoi(target_value.c_str())); break;
		case value_types::ushort_value: result = std::make_shared<ushort_value>(target_name, (unsigned short)_wtoi(target_value.c_str())); break;
		case value_types::int_value: result = std::make_shared<int_value>(target_name, (int)_wtoi(target_value.c_str())); break;
		case value_types::uint_value: result = std::make_shared<uint_value>(target_name, (unsigned int)_wtoi(target_value.c_str())); break;
		case value_types::long_value: result = std::make_shared<long_value>(target_name, (long)_wtol(target_value.c_str())); break;
		case value_types::ulong_value: result = std::make_shared<ulong_value>(target_name, (unsigned long)_wtol(target_value.c_str())); break;
		case value_types::llong_value: result = std::make_shared<llong_value>(target_name, (long long)_wtoll(target_value.c_str())); break;
		case value_types::ullong_value: result = std::make_shared<ullong_value>(target_name, (unsigned long long)_wtoll(target_value.c_str())); break;
		case value_types::float_value: result = std::make_shared<float_value>(target_name, (float)_wtof(target_value.c_str())); break;
		case value_types::double_value: result = std::make_shared<double_value>(target_name, (double)_wtof(target_value.c_str())); break;
		case value_types::bytes_value: result = std::make_shared<bytes_value>(target_name, converter::from_base64(target_value.c_str())); break;
		case value_types::string_value: result = std::make_shared<string_value>(target_name, target_value); break;
		case value_types::container_value: result = std::make_shared<container_value>(target_name, (long)_wtol(target_value.c_str())); break;
		default: result = std::make_shared<value>(target_name, nullptr, 0, value_types::null_value); break;
		}

		return result;
	}

	template <typename T> void value::set_data(T data)
	{
		char* data_ptr = (char*)&data;

		_size = sizeof(T);
		_data = std::vector<unsigned char>(data_ptr, data_ptr + _size);
	}

	void value::set_byte_string(const std::wstring& data)
	{
		_data = converter::from_base64(data);
		_size = _data.size();
		_type = value_types::bytes_value;
	}

	void value::set_string(const std::wstring& data)
	{
		_data = converter::to_array(data);
		_size = _data.size();
		_type = value_types::string_value;
	}

	void value::set_boolean(const std::wstring& data)
	{
		set_data((data == L"true") ? true : false);
		_type = value_types::bool_value;
	}
}