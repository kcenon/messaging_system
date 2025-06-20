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

#include "container/values/container_value.h"
#include <algorithm>
#include <cstring>
#include "utilities/core/formatter.h"
#include "utilities/conversion/convert_string.h"
#include "container/values/bool_value.h"
#include "container/values/bytes_value.h"
#include "container/values/numeric_value.h"
#include "container/values/string_value.h"

namespace container_module
{
	using namespace utility_module;

	container_value::container_value() : value()
	{
		type_ = value_types::container_value;
		long zeroCount = 0;
		set_data(reinterpret_cast<const unsigned char*>(&zeroCount),
				 sizeof(long), value_types::container_value);

		// Fill data_type_map_ if you need parse from string -> child creation
		data_type_map_.insert(
			{ value_types::bool_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_boolean(n, d); } });
		data_type_map_.insert(
			{ value_types::short_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_short(n, d); } });
		data_type_map_.insert(
			{ value_types::ushort_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_ushort(n, d); } });
		data_type_map_.insert(
			{ value_types::int_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_int(n, d); } });
		data_type_map_.insert(
			{ value_types::uint_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_uint(n, d); } });
		data_type_map_.insert(
			{ value_types::long_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_long(n, d); } });
		data_type_map_.insert(
			{ value_types::ulong_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_ulong(n, d); } });
		data_type_map_.insert(
			{ value_types::llong_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_llong(n, d); } });
		data_type_map_.insert(
			{ value_types::ullong_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_ullong(n, d); } });
		data_type_map_.insert(
			{ value_types::float_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_float(n, d); } });
		data_type_map_.insert(
			{ value_types::double_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_double(n, d); } });
		data_type_map_.insert(
			{ value_types::bytes_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_byte_string(n, d); } });
		data_type_map_.insert(
			{ value_types::string_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_string(n, d); } });
		data_type_map_.insert(
			{ value_types::container_value,
			  [this](const std::string& n, const std::string& d)
			  { return set_container(n, d); } });
	}

	container_value::container_value(const std::string& name,
									 long reserved_count)
		: container_value()
	{
		name_ = name;
		set_data(reinterpret_cast<const unsigned char*>(&reserved_count),
				 sizeof(long), value_types::container_value);
	}

	container_value::container_value(
		const std::string& name,
		const std::vector<std::shared_ptr<value>>& units)
		: container_value()
	{
		name_ = name;
		units_ = units;
		long sz = static_cast<long>(units_.size());
		set_data(reinterpret_cast<const unsigned char*>(&sz), sizeof(long),
				 value_types::container_value);

		for (auto& u : units_)
		{
			if (u)
			{
				u->set_parent(shared_from_this());
			}
		}
	}

	container_value::~container_value() { units_.clear(); }

	std::shared_ptr<value> container_value::add(const value& item,
												bool update_count)
	{
		// Example: interpret item => create correct child type
		value_types t = item.type();
		std::shared_ptr<value> child;
		switch (t)
		{
		case value_types::bool_value: {
			bool b = item.to_boolean();
			child = std::make_shared<bool_value>(item.name(), b);
			break;
		}
		case value_types::short_value: {
			short s = item.to_short();
			child = std::make_shared<short_value>(item.name(), s);
			break;
		}
		// ... similarly for other numeric/bytes/string
		default: {
			child = std::make_shared<value>(item.name());
			break;
		}
		}
		return add(child, update_count);
	}

	std::shared_ptr<value> container_value::add(std::shared_ptr<value> item,
												bool update_count)
	{
		auto it = std::find(units_.begin(), units_.end(), item);
		if (it != units_.end())
		{
			return nullptr;
		}
		units_.push_back(item);
		item->set_parent(shared_from_this());

		if (update_count)
		{
			long sz = static_cast<long>(units_.size());
			set_data(reinterpret_cast<const unsigned char*>(&sz), sizeof(long),
					 value_types::container_value);
		}
		return item;
	}

	void container_value::add(const std::vector<value>& target_values,
							  bool update_count)
	{
		for (auto& tv : target_values)
		{
			add(tv, false);
		}
		if (update_count)
		{
			long sz = static_cast<long>(units_.size());
			set_data(reinterpret_cast<const unsigned char*>(&sz), sizeof(long),
					 value_types::container_value);
		}
	}

	void container_value::add(
		const std::vector<std::shared_ptr<value>>& target_values,
		bool update_count)
	{
		for (auto& tv : target_values)
		{
			add(tv, false);
		}
		if (update_count)
		{
			long sz = static_cast<long>(units_.size());
			set_data(reinterpret_cast<const unsigned char*>(&sz), sizeof(long),
					 value_types::container_value);
		}
	}

	void container_value::remove(std::string_view target_name,
								 bool update_count)
	{
		bool found = true;
		while (found)
		{
			found = false;
			auto it = std::find_if(units_.begin(), units_.end(),
								   [&target_name](std::shared_ptr<value> v)
								   { return (v->name() == target_name); });
			if (it != units_.end())
			{
				units_.erase(it);
				found = true;
			}
		}
		if (update_count)
		{
			long sz = static_cast<long>(units_.size());
			set_data(reinterpret_cast<const unsigned char*>(&sz), sizeof(long),
					 value_types::container_value);
		}
	}

	void container_value::remove(std::shared_ptr<value> item, bool update_count)
	{
		auto it = std::find(units_.begin(), units_.end(), item);
		if (it != units_.end())
		{
			units_.erase(it);
			if (update_count)
			{
				long sz = static_cast<long>(units_.size());
				set_data(reinterpret_cast<const unsigned char*>(&sz),
						 sizeof(long), value_types::container_value);
			}
		}
	}

	void container_value::remove_all()
	{
		units_.clear();
		long zero = 0;
		set_data(reinterpret_cast<const unsigned char*>(&zero), sizeof(long),
				 value_types::container_value);
	}

	long container_value::to_long() const
	{
		long temp = 0;
		if (data_.size() >= sizeof(long))
		{
			std::memcpy(&temp, data_.data(), sizeof(long));
		}
		return temp;
	}

	std::string container_value::to_string(const bool& /*original*/) const
	{
		long cnt = to_long();
		return formatter::format("{}", cnt);
	}

	std::shared_ptr<value> container_value::set_boolean(
		const std::string& name, const std::string& dataStr)
	{
		bool b = (dataStr == "true");
		return std::make_shared<bool_value>(name, b);
	}

	std::shared_ptr<value> container_value::set_short(
		const std::string& name, const std::string& dataStr)
	{
		short s = static_cast<short>(std::atoi(dataStr.c_str()));
		return std::make_shared<short_value>(name, s);
	}

	std::shared_ptr<value> container_value::set_ushort(
		const std::string& name, const std::string& dataStr)
	{
		unsigned short us
			= static_cast<unsigned short>(std::atoi(dataStr.c_str()));
		return std::make_shared<ushort_value>(name, us);
	}

	std::shared_ptr<value> container_value::set_int(const std::string& name,
													const std::string& dataStr)
	{
		int i = std::atoi(dataStr.c_str());
		return std::make_shared<int_value>(name, i);
	}

	std::shared_ptr<value> container_value::set_uint(const std::string& name,
													 const std::string& dataStr)
	{
		unsigned int ui = static_cast<unsigned int>(std::atoi(dataStr.c_str()));
		return std::make_shared<uint_value>(name, ui);
	}

	std::shared_ptr<value> container_value::set_long(const std::string& name,
													 const std::string& dataStr)
	{
		long l = std::atol(dataStr.c_str());
		return std::make_shared<long_value>(name, l);
	}

	std::shared_ptr<value> container_value::set_ulong(
		const std::string& name, const std::string& dataStr)
	{
		unsigned long ul
			= static_cast<unsigned long>(std::atol(dataStr.c_str()));
		return std::make_shared<ulong_value>(name, ul);
	}

	std::shared_ptr<value> container_value::set_llong(
		const std::string& name, const std::string& dataStr)
	{
		long long ll = std::atoll(dataStr.c_str());
		return std::make_shared<llong_value>(name, ll);
	}

	std::shared_ptr<value> container_value::set_ullong(
		const std::string& name, const std::string& dataStr)
	{
		unsigned long long ull
			= static_cast<unsigned long long>(std::atoll(dataStr.c_str()));
		return std::make_shared<ullong_value>(name, ull);
	}

	std::shared_ptr<value> container_value::set_float(
		const std::string& name, const std::string& dataStr)
	{
		float f = std::atof(dataStr.c_str());
		return std::make_shared<float_value>(name, f);
	}

	std::shared_ptr<value> container_value::set_double(
		const std::string& name, const std::string& dataStr)
	{
		double d = std::atof(dataStr.c_str());
		return std::make_shared<double_value>(name, d);
	}

	std::shared_ptr<value> container_value::set_byte_string(
		const std::string& name, const std::string& dataStr)
	{
		auto [val, err] = convert_string::from_base64(dataStr);
		if (err.has_value())
		{
			std::vector<uint8_t> empty;
			return std::make_shared<bytes_value>(name, empty);
		}
		return std::make_shared<bytes_value>(name, val.data(), val.size());
	}

	std::shared_ptr<value> container_value::set_string(
		const std::string& name, const std::string& dataStr)
	{
		return std::make_shared<string_value>(name, dataStr);
	}

	std::shared_ptr<value> container_value::set_container(
		const std::string& name, const std::string& dataStr)
	{
		long count = std::atol(dataStr.c_str());
		return std::make_shared<container_value>(name, count);
	}
} // namespace container_module
