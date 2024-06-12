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
#include "values/container_value.h"
#include "values/double_value.h"
#include "values/float_value.h"
#include "values/int_value.h"
#include "values/llong_value.h"
#include "values/long_value.h"
#include "values/short_value.h"
#include "values/string_value.h"
#include "values/uint_value.h"
#include "values/ullong_value.h"
#include "values/ulong_value.h"
#include "values/ushort_value.h"

#include <sstream>

#include "fmt/format.h"
#include "fmt/xchar.h"

namespace container
{
	using namespace converting;

	value::value(void) : name_(""), type_(value_types::null_value), size_(0)
	{
		data_type_map_.insert(
			{ value_types::bool_value,
			  std::bind(&value::set_boolean, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::short_value,
			  std::bind(&value::set_short, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::ushort_value,
			  std::bind(&value::set_ushort, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::int_value,
			  std::bind(&value::set_int, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::uint_value,
			  std::bind(&value::set_uint, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::long_value,
			  std::bind(&value::set_long, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::ulong_value,
			  std::bind(&value::set_ulong, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::llong_value,
			  std::bind(&value::set_llong, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::ullong_value,
			  std::bind(&value::set_ullong, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::float_value,
			  std::bind(&value::set_float, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::double_value,
			  std::bind(&value::set_double, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::bytes_value, std::bind(&value::set_byte_string, this,
												  std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::string_value,
			  std::bind(&value::set_string, this, std::placeholders::_1) });
		data_type_map_.insert(
			{ value_types::container_value,
			  std::bind(&value::set_long, this, std::placeholders::_1) });
	}

	value::value(std::shared_ptr<value> object) : value()
	{
		if (object == nullptr)
		{
			name_ = "";
			type_ = value_types::null_value;
			size_ = 0;

			return;
		}

		name_ = object->name();
		type_ = object->type();
		size_ = object->size();
		parent_ = object->parent();
		units_ = object->children();
	}

	value::value(const std::string& name,
				 const std::vector<std::shared_ptr<value>>& units)
		: value()
	{
		name_ = name;
		units_ = units;

		long size = static_cast<long>(units_.size());
		set_data((const unsigned char*)&size, sizeof(long),
				 value_types::container_value);
	}

	value::value(const std::string& name,
				 const value_types& type,
				 const std::string& data)
		: value()
	{
		set_data(name, type, data);
	}

	value::value(const std::string& name,
				 const unsigned char* data,
				 const size_t& size,
				 const value_types& type)
		: value()
	{
		name_ = name;
		set_data(data, size, type);
	}

	value::~value(void) {}

	std::shared_ptr<value> value::get_ptr(void) { return shared_from_this(); }

	void value::set_parent(std::shared_ptr<value> parent) { parent_ = parent; }

	void value::set_data(const unsigned char* data,
						 const size_t& size,
						 const value_types& type)
	{
		if (data == nullptr || size == 0)
		{
			type_ = type;
			size_ = 0;
			data_.clear();
			return;
		}

		type_ = type;
		size_ = size;
		data_ = std::vector<uint8_t>(data, data + size);
	}

	void value::set_data(const std::string& name,
						 const value_types& type,
						 const std::string& data)
	{
		name_ = name;
		type_ = type;

		auto target = data_type_map_.find(type_);
		if (target == data_type_map_.end())
		{
			data_.clear();
			return;
		}

		target->second(data);
	}

	std::string value::name(void) const { return name_; }

	value_types value::type(void) const { return type_; }

	std::string value::data(void) const
	{
		if (type_ != value_types::string_value)
		{
			return to_string();
		}

		return convert_specific_string(data_);
	}

	size_t value::size(void) const { return data_.size(); }

	std::shared_ptr<value> value::parent(void)
	{
		std::shared_ptr<value> parent = parent_.lock();

		return parent;
	}

	size_t value::child_count(void) const { return units_.size(); }

	std::vector<std::shared_ptr<value>> value::children(
		const bool& only_container)
	{
		if (!only_container)
		{
			return units_;
		}

		std::vector<std::shared_ptr<value>> result_list;

		for (auto& unit : units_)
		{
			if (unit->is_container())
			{
				result_list.push_back(unit);
			}
		};

		return result_list;
	}

	std::vector<std::shared_ptr<value>> value::value_array(
		const std::string& key)
	{
		std::vector<std::shared_ptr<value>> result_list;

		for (auto& unit : units_)
		{
			if (unit->name() == key)
			{
				result_list.push_back(unit);
			}
		};

		return result_list;
	}

	const std::vector<uint8_t> value::to_bytes(void) const { return data_; }

	bool value::is_null(void) const { return type_ == value_types::null_value; }

	bool value::is_bytes(void) const
	{
		return type_ == value_types::bytes_value;
	}

	bool value::is_boolean(void) const
	{
		return type_ == value_types::bool_value;
	}

	bool value::is_numeric(void) const
	{
		return type_ == value_types::short_value
			   || type_ == value_types::ushort_value
			   || type_ == value_types::int_value
			   || type_ == value_types::uint_value
			   || type_ == value_types::long_value
			   || type_ == value_types::ulong_value
			   || type_ == value_types::llong_value
			   || type_ == value_types::ullong_value
			   || type_ == value_types::float_value
			   || type_ == value_types::double_value;
	}

	bool value::is_string(void) const
	{
		return type_ == value_types::string_value;
	}

	bool value::is_container(void) const
	{
		return type_ == value_types::container_value;
	}

	const std::string value::to_xml(void)
	{
		std::string result;

		if (units_.size() == 0)
		{
			fmt::format_to(std::back_inserter(result), "<{0}>{1}</{0}>", name(),
						   to_string(false));

			return result;
		}

		fmt::format_to(std::back_inserter(result), "<{}>", name());
		for (auto& unit : units_)
		{
			fmt::format_to(std::back_inserter(result), "{}", unit->to_xml());
		}
		fmt::format_to(std::back_inserter(result), "</{}>", name());

		return result;
	}

	const std::string value::to_json(void)
	{
		std::string result;

		if (units_.size() == 0)
		{
			switch (type_)
			{
			case value_types::bytes_value:
			case value_types::string_value:
				fmt::format_to(std::back_inserter(result), "\"{}\":\"{}\"",
							   name(), to_string(false));
				break;
			default:
				fmt::format_to(std::back_inserter(result), "\"{}\":{}", name(),
							   to_string(false));
				break;
			}

			return result;
		}

		fmt::format_to(std::back_inserter(result), "\"{}\":{}", name(), "{");

		bool first = true;
		for (auto& unit : units_)
		{
			fmt::format_to(std::back_inserter(result), first ? "{}" : ",{}",
						   unit->to_json());
			first = false;
		}

		fmt::format_to(std::back_inserter(result), "{}", "}");

		return result;
	}

	const std::string value::serialize(void)
	{
		std::string result;

		fmt::format_to(std::back_inserter(result), "[{},{},{}];", name(),
					   convert_value_type(type_), to_string(false));

		for (auto& unit : units_)
		{
			fmt::format_to(std::back_inserter(result), "{}", unit->serialize());
		}

		return result;
	}

	std::shared_ptr<value> value::operator[](const std::string& key)
	{
		std::vector<std::shared_ptr<value>> searched_values = value_array(key);
		if (searched_values.empty())
		{
			return std::make_shared<value>(key);
		}

		return searched_values[0];
	}

	std::shared_ptr<value> operator<<(std::shared_ptr<value> container,
									  std::shared_ptr<value> other)
	{
		container->add(other);

		return container;
	}

	std::ostream& operator<<(std::ostream& out,
							 std::shared_ptr<value> other) // output
	{
		out << other->serialize();

		return out;
	}

	std::string& operator<<(std::string& out, std::shared_ptr<value> other)
	{
		out = other->serialize();

		return out;
	}

	std::string value::convert_specific_string(
		const std::vector<uint8_t>& data) const
	{
		std::string temp = converter::to_string(data);
		converter::replace(temp, "</0x0A;>", "\r");
		converter::replace(temp, "</0x0B;>", "\n");
		converter::replace(temp, "</0x0C;>", " ");
		converter::replace(temp, "</0x0D;>", "\t");

		return temp;
	}

	std::vector<uint8_t> value::convert_specific_string(std::string data) const
	{
		converter::replace(data, "\r", "</0x0A;>");
		converter::replace(data, "\n", "</0x0B;>");
		converter::replace(data, " ", "</0x0C;>");
		converter::replace(data, "\t", "</0x0D;>");

		return converter::to_array(data);
	}

	template <typename T> void value::set_data(T data)
	{
		char* data_ptr = (char*)&data;

		size_ = sizeof(T);
		data_ = std::vector<uint8_t>(data_ptr, data_ptr + size_);
	}

	void value::set_byte_string(const std::string& data)
	{
		data_ = converter::from_base64(data);
		size_ = data_.size();
		type_ = value_types::bytes_value;
	}

	void value::set_string(const std::string& data)
	{
		data_ = converter::to_array(data);
		size_ = data_.size();
		type_ = value_types::string_value;
	}

	void value::set_boolean(const std::string& data)
	{
		set_data((data == "true") ? true : false);
		type_ = value_types::bool_value;
	}

	void value::set_short(const std::string& data)
	{
		set_data((short)atoi(converter::to_string(data).c_str()));
	}

	void value::set_ushort(const std::string& data)
	{
		set_data((unsigned short)atoi(converter::to_string(data).c_str()));
	}

	void value::set_int(const std::string& data)
	{
		set_data((int)atoi(converter::to_string(data).c_str()));
	}

	void value::set_uint(const std::string& data)
	{
		set_data((unsigned int)atoi(converter::to_string(data).c_str()));
	}

	void value::set_long(const std::string& data)
	{
		set_data((long)atol(converter::to_string(data).c_str()));
	}

	void value::set_ulong(const std::string& data)
	{
		set_data((unsigned long)atol(converter::to_string(data).c_str()));
	}

	void value::set_llong(const std::string& data)
	{
		set_data((long long)atoll(converter::to_string(data).c_str()));
	}

	void value::set_ullong(const std::string& data)
	{
		set_data((unsigned long long)atoll(converter::to_string(data).c_str()));
	}

	void value::set_float(const std::string& data)
	{
		set_data((float)atof(converter::to_string(data).c_str()));
	}

	void value::set_double(const std::string& data)
	{
		set_data((double)atof(converter::to_string(data).c_str()));
	}
} // namespace container