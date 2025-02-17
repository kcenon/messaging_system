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
#include <cstring>
#include <sstream>
#include <algorithm>
#include "formatter.h"		// custom formatter
#include "convert_string.h" // custom string conversion

namespace container
{
	using namespace utility_module;

	value::value() : name_(""), type_(value_types::null_value), size_(0)
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
		if (!object)
			return;

		name_ = object->name();
		type_ = object->type();
		size_ = object->size();
		parent_ = object->parent();
		units_ = object->children();
		data_ = object->to_bytes();
	}

	value::value(const std::string& name,
				 const std::vector<std::shared_ptr<value>>& units)
		: value()
	{
		name_ = name;
		units_ = units;
		long sz = static_cast<long>(units_.size());
		set_data(reinterpret_cast<const unsigned char*>(&sz), sizeof(long),
				 value_types::container_value);
	}

	value::value(const std::string& name,
				 const value_types& type,
				 const std::string& dataStr)
		: value()
	{
		set_data(name, type, dataStr);
	}

	value::value(const std::string& name,
				 const unsigned char* dataPtr,
				 const size_t& sz,
				 const value_types& type)
		: value()
	{
		name_ = name;
		set_data(dataPtr, sz, type);
	}

	value::~value() {}

	std::shared_ptr<value> value::get_ptr() { return shared_from_this(); }

	void value::set_parent(std::shared_ptr<value> parent) { parent_ = parent; }

	void value::set_data(const unsigned char* dataPtr,
						 const size_t& sz,
						 const value_types& t)
	{
		if (!dataPtr || sz == 0)
		{
			type_ = t;
			size_ = 0;
			data_.clear();
			return;
		}
		type_ = t;
		size_ = sz;
		data_ = std::vector<uint8_t>(dataPtr, dataPtr + sz);
	}

	void value::set_data(const std::string& n,
						 const value_types& t,
						 const std::string& d)
	{
		name_ = n;
		type_ = t;
		auto it = data_type_map_.find(t);
		if (it == data_type_map_.end())
		{
			data_.clear();
			size_ = 0;
			return;
		}
		it->second(d);
	}

	std::string value::name() const { return name_; }

	value_types value::type() const { return type_; }

	std::string value::data() const
	{
		if (type_ == value_types::string_value)
		{
			return to_string(true);
		}
		return to_string(false);
	}

	size_t value::size() const { return data_.size(); }

	std::shared_ptr<value> value::parent() { return parent_.lock(); }

	size_t value::child_count() const { return units_.size(); }

	std::vector<std::shared_ptr<value>> value::children(
		const bool& only_container)
	{
		if (!only_container)
		{
			return units_;
		}
		std::vector<std::shared_ptr<value>> result;
		for (auto& u : units_)
		{
			if (u->is_container())
			{
				result.push_back(u);
			}
		}
		return result;
	}

	std::vector<std::shared_ptr<value>> value::value_array(
		const std::string& key)
	{
		std::vector<std::shared_ptr<value>> result;
		for (auto& u : units_)
		{
			if (u->name() == key)
			{
				result.push_back(u);
			}
		}
		return result;
	}

	const std::vector<uint8_t> value::to_bytes() const { return data_; }

	bool value::is_null() const { return (type_ == value_types::null_value); }

	bool value::is_bytes() const { return (type_ == value_types::bytes_value); }

	bool value::is_boolean() const
	{
		return (type_ == value_types::bool_value);
	}

	bool value::is_numeric() const
	{
		switch (type_)
		{
		case value_types::short_value:
		case value_types::ushort_value:
		case value_types::int_value:
		case value_types::uint_value:
		case value_types::long_value:
		case value_types::ulong_value:
		case value_types::llong_value:
		case value_types::ullong_value:
		case value_types::float_value:
		case value_types::double_value:
			return true;
		default:
			return false;
		}
	}

	bool value::is_string() const
	{
		return (type_ == value_types::string_value);
	}

	bool value::is_container() const
	{
		return (type_ == value_types::container_value);
	}

	const std::string value::to_xml()
	{
		std::string result;
		if (units_.empty())
		{
			formatter::format_to(std::back_inserter(result), "<{0}>{1}</{0}>",
								 name_, to_string(false));
			return result;
		}
		formatter::format_to(std::back_inserter(result), "<{}>", name_);
		for (auto& u : units_)
		{
			formatter::format_to(std::back_inserter(result), "{}", u->to_xml());
		}
		formatter::format_to(std::back_inserter(result), "</{}>", name_);
		return result;
	}

	const std::string value::to_json()
	{
		std::string result;
		if (units_.empty())
		{
			if (type_ == value_types::bytes_value
				|| type_ == value_types::string_value)
			{
				formatter::format_to(std::back_inserter(result),
									 "\"{}\":\"{}\"", name_, to_string(false));
			}
			else
			{
				formatter::format_to(std::back_inserter(result), "\"{}\":{}",
									 name_, to_string(false));
			}
			return result;
		}
		formatter::format_to(std::back_inserter(result), "\"{}\":{{", name_);

		bool first = true;
		for (auto& u : units_)
		{
			formatter::format_to(std::back_inserter(result),
								 first ? "{}" : ",{}", u->to_json());
			first = false;
		}
		formatter::format_to(std::back_inserter(result), "}}");
		return result;
	}

	const std::string value::serialize()
	{
		std::string result;
		formatter::format_to(std::back_inserter(result), "[{},{},{}];", name_,
							 convert_value_type(type_), to_string(false));
		for (auto& u : units_)
		{
			formatter::format_to(std::back_inserter(result), "{}",
								 u->serialize());
		}
		return result;
	}

	std::shared_ptr<value> value::operator[](const std::string& key)
	{
		auto arr = value_array(key);
		if (arr.empty())
		{
			return std::make_shared<value>(key);
		}
		return arr[0];
	}

	std::shared_ptr<value> operator<<(std::shared_ptr<value> container,
									  std::shared_ptr<value> other)
	{
		// For non-container, it might throw. We do nothing here by default.
		// If container->is_container(), we could dynamic_cast to
		// container_value and call add(...).
		return container;
	}

	std::ostream& operator<<(std::ostream& out, std::shared_ptr<value> other)
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
		const std::vector<uint8_t>& dat) const
	{
		auto [converted, err] = convert_string::to_string(dat);
		if (err.has_value())
		{
			return "";
		}
		std::string temp = converted.value();
		convert_string::replace(temp, "</0x0A;>", "\r");
		convert_string::replace(temp, "</0x0B;>", "\n");
		convert_string::replace(temp, "</0x0C;>", " ");
		convert_string::replace(temp, "</0x0D;>", "\t");
		return temp;
	}

	std::vector<uint8_t> value::convert_specific_string(std::string d) const
	{
		convert_string::replace(d, "\r", "</0x0A;>");
		convert_string::replace(d, "\n", "</0x0B;>");
		convert_string::replace(d, " ", "</0x0C;>");
		convert_string::replace(d, "\t", "</0x0D;>");
		auto [arr, err] = convert_string::to_array(d);
		if (err.has_value())
		{
			return {};
		}
		return arr.value();
	}

	void value::set_byte_string(const std::string& dataStr)
	{
		auto [val, err] = convert_string::from_base64(dataStr);
		if (err.has_value())
		{
			data_.clear();
			size_ = 0;
			type_ = value_types::bytes_value;
			return;
		}
		data_ = val;
		size_ = data_.size();
		type_ = value_types::bytes_value;
	}

	void value::set_string(const std::string& dataStr)
	{
		auto [arr, err] = convert_string::to_array(dataStr);
		if (err.has_value())
		{
			data_.clear();
			size_ = 0;
			type_ = value_types::string_value;
			return;
		}
		data_ = arr.value();
		size_ = data_.size();
		type_ = value_types::string_value;
	}

	void value::set_boolean(const std::string& dataStr)
	{
		bool b = (dataStr == "true");
		set_data(b);
		type_ = value_types::bool_value;
	}

	void value::set_short(const std::string& dataStr)
	{
		short s = static_cast<short>(std::atoi(dataStr.c_str()));
		set_data(s);
		type_ = value_types::short_value;
	}

	void value::set_ushort(const std::string& dataStr)
	{
		unsigned short us
			= static_cast<unsigned short>(std::atoi(dataStr.c_str()));
		set_data(us);
		type_ = value_types::ushort_value;
	}

	void value::set_int(const std::string& dataStr)
	{
		int i = std::atoi(dataStr.c_str());
		set_data(i);
		type_ = value_types::int_value;
	}

	void value::set_uint(const std::string& dataStr)
	{
		unsigned int ui = static_cast<unsigned int>(std::atoi(dataStr.c_str()));
		set_data(ui);
		type_ = value_types::uint_value;
	}

	void value::set_long(const std::string& dataStr)
	{
		long l = std::atol(dataStr.c_str());
		set_data(l);
		type_ = value_types::long_value;
	}

	void value::set_ulong(const std::string& dataStr)
	{
		unsigned long ul
			= static_cast<unsigned long>(std::atol(dataStr.c_str()));
		set_data(ul);
		type_ = value_types::ulong_value;
	}

	void value::set_llong(const std::string& dataStr)
	{
		long long ll = std::atoll(dataStr.c_str());
		set_data(ll);
		type_ = value_types::llong_value;
	}

	void value::set_ullong(const std::string& dataStr)
	{
		unsigned long long ull
			= static_cast<unsigned long long>(std::atoll(dataStr.c_str()));
		set_data(ull);
		type_ = value_types::ullong_value;
	}

	void value::set_float(const std::string& dataStr)
	{
		float f = std::atof(dataStr.c_str());
		set_data(f);
		type_ = value_types::float_value;
	}

	void value::set_double(const std::string& dataStr)
	{
		double d = std::atof(dataStr.c_str());
		set_data(d);
		type_ = value_types::double_value;
	}
} // namespace container
