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

#include "container/core/container.h"

#include "utilities/core/formatter.h"
#include "utilities/io/file_handler.h"
#include "utilities/conversion/convert_string.h"

#include "container/core/value_types.h"
#include "container/values/bool_value.h"
#include "container/values/bytes_value.h"
#include "container/values/container_value.h"
#include "container/values/numeric_value.h"
#include "container/values/string_value.h"

#include <fcntl.h>
#include <wchar.h>

#include <regex>
#include <sstream>

namespace container_module
{
	inline constexpr std::string_view TARGET_ID = "1";
	inline constexpr std::string_view TARGET_SUB_ID = "2";
	inline constexpr std::string_view SOURCE_ID = "3";
	inline constexpr std::string_view SOURCE_SUB_ID = "4";
	inline constexpr std::string_view MESSAGE_TYPE = "5";
	inline constexpr std::string_view MESSAGE_VERSION = "6";

	using namespace utility_module;

	value_container::value_container()
		: parsed_data_(true)
		, changed_data_(false)
		, data_string_("@data={};")
		, source_id_("")
		, source_sub_id_("")
		, target_id_("")
		, target_sub_id_("")
		, message_type_("data_container")
		, version_("1.0.0.0")
	{
		// Fill data_type_map_ for dynamic parsing
		data_type_map_.insert(
			{ value_types::bool_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  auto b = (d == "true");
				  return std::make_shared<bool_value>(n, b);
			  } });
		data_type_map_.insert(
			{ value_types::short_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  try {
					  short s = static_cast<short>(std::stoi(d));
					  return std::make_shared<short_value>(n, s);
				  } catch (const std::exception&) {
					  return std::make_shared<short_value>(n, 0);
				  }
			  } });
		data_type_map_.insert(
			{ value_types::ushort_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  unsigned short s = (unsigned short)std::stoul(d.c_str());
				  return std::make_shared<ushort_value>(n, s);
			  } });
		data_type_map_.insert(
			{ value_types::int_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  try {
					  int i = std::stoi(d);
					  return std::make_shared<int_value>(n, i);
				  } catch (const std::exception&) {
					  return std::make_shared<int_value>(n, 0);
				  }
			  } });
		data_type_map_.insert(
			{ value_types::uint_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  unsigned int i = std::stoul(d.c_str());
				  return std::make_shared<uint_value>(n, i);
			  } });
		data_type_map_.insert(
			{ value_types::long_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  long l = std::atol(d.c_str());
				  return std::make_shared<long_value>(n, l);
			  } });
		data_type_map_.insert(
			{ value_types::ulong_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  unsigned long l = std::stoul(d.c_str());
				  return std::make_shared<ulong_value>(n, l);
			  } });
		data_type_map_.insert(
			{ value_types::llong_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  long long ll = std::stoll(d.c_str());
				  return std::make_shared<llong_value>(n, ll);
			  } });
		data_type_map_.insert(
			{ value_types::ullong_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  unsigned long long ll = std::stoull(d.c_str());
				  return std::make_shared<ullong_value>(n, ll);
			  } });
		data_type_map_.insert(
			{ value_types::float_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  float f = std::stof(d.c_str());
				  return std::make_shared<float_value>(n, f);
			  } });
		data_type_map_.insert(
			{ value_types::double_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  double dbl = std::stod(d.c_str());
				  return std::make_shared<double_value>(n, dbl);
			  } });
		data_type_map_.insert(
			{ value_types::bytes_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  // For bytes_value, we need to convert from string representation
				  // This is a simplified version - you may need more complex parsing
				  std::vector<uint8_t> bytes;
				  for (size_t i = 0; i < d.length(); i += 2) {
					  if (i + 1 < d.length()) {
						  std::string byte_str = d.substr(i, 2);
						  bytes.push_back(static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16)));
					  }
				  }
				  return std::make_shared<bytes_value>(n, bytes);
			  } });
		data_type_map_.insert(
			{ value_types::string_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  return std::make_shared<string_value>(n, d);
			  } });
		data_type_map_.insert(
			{ value_types::container_value,
			  [this](const std::string& n, const std::string& d)
			  {
				  long count = std::atol(d.c_str());
				  return std::make_shared<container_value>(n, count);
			  } });
	}

	value_container::value_container(const std::string& data_str,
									 bool parse_only_header)
		: value_container()
	{
		deserialize(data_str, parse_only_header);
	}

	value_container::value_container(const std::vector<uint8_t>& data_array,
									 bool parse_only_header)
		: value_container()
	{
		deserialize(data_array, parse_only_header);
	}

	value_container::value_container(const value_container& other,
									 bool parse_only_header)
		: value_container()
	{
		deserialize(other.serialize(), parse_only_header);
	}

	value_container::value_container(std::shared_ptr<value_container> other,
									 bool parse_only_header)
		: value_container()
	{
		if (other)
		{
			deserialize(other->serialize(), parse_only_header);
		}
	}

	value_container::value_container(
		const std::string& msg_type,
		std::vector<std::shared_ptr<value>> units)
		: value_container()
	{
		set_message_type(msg_type);
		units_ = std::move(units);
		for (auto& unit : units_)
		{
			if (unit) unit->set_parent(nullptr);
		}
		changed_data_ = true;
	}

	value_container::value_container(
		const std::string& tid,
		const std::string& tsubid,
		const std::string& msg_type,
		std::vector<std::shared_ptr<value>> units)
		: value_container()
	{
		set_target(tid, tsubid);
		set_message_type(msg_type);
		units_ = std::move(units);
		for (auto& unit : units_)
		{
			if (unit) unit->set_parent(nullptr);
		}
		changed_data_ = true;
	}

	value_container::value_container(
		const std::string& sid,
		const std::string& ssubid,
		const std::string& tid,
		const std::string& tsubid,
		const std::string& msg_type,
		std::vector<std::shared_ptr<value>> units)
		: value_container()
	{
		set_source(sid, ssubid);
		set_target(tid, tsubid);
		set_message_type(msg_type);
		units_ = std::move(units);
		for (auto& unit : units_)
		{
			if (unit) unit->set_parent(nullptr);
		}
		changed_data_ = true;
	}

	value_container::value_container(value_container&& other) noexcept
		: parsed_data_(other.parsed_data_)
		, changed_data_(other.changed_data_)
		, data_string_(std::move(other.data_string_))
		, source_id_(std::move(other.source_id_))
		, source_sub_id_(std::move(other.source_sub_id_))
		, target_id_(std::move(other.target_id_))
		, target_sub_id_(std::move(other.target_sub_id_))
		, message_type_(std::move(other.message_type_))
		, version_(std::move(other.version_))
		, units_(std::move(other.units_))
		, data_type_map_(std::move(other.data_type_map_))
	{
		// Reset other to a valid state
		other.parsed_data_ = true;
		other.changed_data_ = false;
	}
	
	value_container& value_container::operator=(value_container&& other) noexcept
	{
		if (this != &other)
		{
			source_id_ = std::move(other.source_id_);
			source_sub_id_ = std::move(other.source_sub_id_);
			target_id_ = std::move(other.target_id_);
			target_sub_id_ = std::move(other.target_sub_id_);
			message_type_ = std::move(other.message_type_);
			version_ = std::move(other.version_);
			parsed_data_ = other.parsed_data_;
			changed_data_ = other.changed_data_;
			data_string_ = std::move(other.data_string_);
			units_ = std::move(other.units_);
			data_type_map_ = std::move(other.data_type_map_);
			
			// Reset other to a valid state
			other.parsed_data_ = true;
			other.changed_data_ = false;
		}
		return *this;
	}

	value_container::~value_container() {}

	std::shared_ptr<value_container> value_container::get_ptr()
	{
		return shared_from_this();
	}

	void value_container::set_source(std::string_view sid,
									 std::string_view ssubid)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		source_id_ = std::string(sid);
		source_sub_id_ = std::string(ssubid);
	}

	void value_container::set_target(std::string_view tid,
									 std::string_view tsubid)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		target_id_ = std::string(tid);
		target_sub_id_ = std::string(tsubid);
	}

	void value_container::set_message_type(std::string_view msg_type)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		message_type_ = std::string(msg_type);
	}

	void value_container::set_units(
		const std::vector<std::shared_ptr<value>>& target_values,
		bool update_immediately)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		for (auto& tv : target_values)
		{
			auto it = std::find(units_.begin(), units_.end(), tv);
			if (it == units_.end())
			{
				units_.push_back(tv);
				tv->set_parent(nullptr);
			}
		}
		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}
	}

	void value_container::swap_header(void)
	{
		std::swap(source_id_, target_id_);
		std::swap(source_sub_id_, target_sub_id_);
	}

	void value_container::clear_value(void)
	{
		parsed_data_ = true;
		changed_data_ = false;
		data_string_ = "@data={};";
		units_.clear();
	}

	std::shared_ptr<value_container> value_container::copy(
		bool containing_values)
	{
		auto newC = std::make_shared<value_container>(serialize(),
													  !containing_values);
		if (!containing_values && newC)
		{
			newC->clear_value();
		}
		return newC;
	}

	std::string value_container::source_id(void) const { 
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return source_id_; 
	}

	std::string value_container::source_sub_id(void) const
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return source_sub_id_;
	}

	std::string value_container::target_id(void) const { 
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return target_id_; 
	}

	std::string value_container::target_sub_id(void) const
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return target_sub_id_;
	}

	std::string value_container::message_type(void) const
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		return message_type_;
	}

	std::shared_ptr<value> value_container::add(const value& tv,
												bool update_immediately)
	{
		// Possibly interpret tv => create a new child
		auto newChild = std::make_shared<value>(
			std::make_shared<value>(const_cast<value&>(tv).get_ptr()));
		return add(newChild, update_immediately);
	}

	std::shared_ptr<value> value_container::add(std::shared_ptr<value> tv,
												bool update_immediately)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		auto it = std::find(units_.begin(), units_.end(), tv);
		if (it != units_.end())
		{
			return nullptr;
		}
		units_.push_back(tv);
		tv->set_parent(nullptr);

		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}
		return tv;
	}

	void value_container::remove(std::string_view target_name,
								 bool update_immediately)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
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
		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}
	}

	void value_container::remove(std::shared_ptr<value> tv,
								 bool update_immediately)
	{
		std::unique_lock<std::shared_mutex> lock(mutex_);
		
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		auto it = std::find(units_.begin(), units_.end(), tv);
		if (it != units_.end())
		{
			units_.erase(it);
			changed_data_ = !update_immediately;
			if (update_immediately)
			{
				data_string_ = datas();
			}
		}
	}

	std::vector<std::shared_ptr<value>> value_container::value_array(
		std::string_view target_name)
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		std::vector<std::shared_ptr<value>> results;
		for (auto& v : units_)
		{
			if (v->name() == target_name)
			{
				results.push_back(v);
			}
		}
		return results;
	}

	std::shared_ptr<value> value_container::get_value(
		std::string_view target_name, unsigned int index)
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		auto arr = value_array(target_name);
		if (arr.empty() || index >= arr.size())
		{
			return std::make_shared<value>(std::string(target_name));
		}
		return arr[index];
	}

	void value_container::initialize(void)
	{
		source_id_.clear();
		source_sub_id_.clear();
		target_id_.clear();
		target_sub_id_.clear();
		message_type_ = "data_container";
		version_ = "1.0";

		clear_value();
	}

	std::string value_container::serialize(void) const
	{
		std::shared_lock<std::shared_mutex> lock(mutex_);
		const_cast<value_container*>(this)->serialization_count_.fetch_add(1, std::memory_order_relaxed);
		
		// If everything parsed, just rebuild data
		std::string ds = (parsed_data_ ? datas() : data_string_);

		// Compose header
		std::string header;
		formatter::format_to(std::back_inserter(header), "@header={{");

		if (message_type_ != "data_container")
		{
			formatter::format_to(std::back_inserter(header), "[{},{}];",
								 TARGET_ID, target_id_);
			formatter::format_to(std::back_inserter(header), "[{},{}];",
								 TARGET_SUB_ID, target_sub_id_);
			formatter::format_to(std::back_inserter(header), "[{},{}];",
								 SOURCE_ID, source_id_);
			formatter::format_to(std::back_inserter(header), "[{},{}];",
								 SOURCE_SUB_ID, source_sub_id_);
		}
		formatter::format_to(std::back_inserter(header), "[{},{}];",
							 MESSAGE_TYPE, message_type_);
		formatter::format_to(std::back_inserter(header), "[{},{}];",
							 MESSAGE_VERSION, version_);
		formatter::format_to(std::back_inserter(header), "}};");

		return header + ds;
	}

	std::vector<uint8_t> value_container::serialize_array(void) const
	{
		auto [arr, err] = convert_string::to_array(serialize());
		if (err.has_value())
		{
			return {};
		}
		return arr.value();
	}

	bool value_container::deserialize(const std::string& data_str,
									  bool parse_only_header)
	{
		initialize();
		if (data_str.empty())
			return false;

		// Remove newlines
		std::regex newlineRe("\\r\\n?|\\n");
		std::string clean = std::regex_replace(data_str, newlineRe, "");

		// parse header portion
		std::regex fullRe("@header=\\s*\\{\\s*(.*?)\\s*\\};");
		std::smatch match;
		if (!std::regex_search(clean, match, fullRe))
		{
			// No header => parse as data only
			return deserialize_values(clean, parse_only_header);
		}
		// match[1] => inside the header
		std::string headerInside = match[1].str();
		std::regex pairRe("\\[(\\w+),(.*?)\\];");
		auto it = std::sregex_iterator(headerInside.begin(), headerInside.end(),
									   pairRe);
		auto end = std::sregex_iterator();
		for (; it != end; ++it)
		{
			parsing((*it)[1].str(), std::string(TARGET_ID), (*it)[2].str(), target_id_);
			parsing((*it)[1].str(), std::string(TARGET_SUB_ID), (*it)[2].str(), target_sub_id_);
			parsing((*it)[1].str(), std::string(SOURCE_ID), (*it)[2].str(), source_id_);
			parsing((*it)[1].str(), std::string(SOURCE_SUB_ID), (*it)[2].str(), source_sub_id_);
			parsing((*it)[1].str(), std::string(MESSAGE_TYPE), (*it)[2].str(), message_type_);
			parsing((*it)[1].str(), std::string(MESSAGE_VERSION), (*it)[2].str(), version_);
		}

		return deserialize_values(clean, parse_only_header);
	}

	bool value_container::deserialize(const std::vector<uint8_t>& data_array,
									  bool parse_only_header)
	{
		auto [strVal, err] = convert_string::to_string(data_array);
		if (err.has_value())
		{
			return false;
		}
		return deserialize(strVal.value(), parse_only_header);
	}

	const std::string value_container::to_xml(void)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		std::string result;
		formatter::format_to(std::back_inserter(result), "<container>");
		formatter::format_to(std::back_inserter(result), "<header>");
		if (message_type_ != "data_container")
		{
			formatter::format_to(std::back_inserter(result),
								 "<target_id>{}</target_id>", target_id_);
			formatter::format_to(std::back_inserter(result),
								 "<target_sub_id>{}</target_sub_id>",
								 target_sub_id_);
			formatter::format_to(std::back_inserter(result),
								 "<source_id>{}</source_id>", source_id_);
			formatter::format_to(std::back_inserter(result),
								 "<source_sub_id>{}</source_sub_id>",
								 source_sub_id_);
		}
		formatter::format_to(std::back_inserter(result),
							 "<message_type>{}</message_type>", message_type_);
		formatter::format_to(std::back_inserter(result),
							 "<version>{}</version>", version_);
		formatter::format_to(std::back_inserter(result), "</header>");

		formatter::format_to(std::back_inserter(result), "<values>");
		for (auto& u : units_)
		{
			formatter::format_to(std::back_inserter(result), "{}", u->to_xml());
		}
		formatter::format_to(std::back_inserter(result), "</values>");
		formatter::format_to(std::back_inserter(result), "</container>");
		return result;
	}

	const std::string value_container::to_json(void)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}
		std::string result;
		formatter::format_to(std::back_inserter(result), "{{");
		// header
		formatter::format_to(std::back_inserter(result), "\"header\":{{");
		if (message_type_ != "data_container")
		{
			formatter::format_to(std::back_inserter(result),
								 "\"target_id\":\"{}\",", target_id_);
			formatter::format_to(std::back_inserter(result),
								 "\"target_sub_id\":\"{}\",", target_sub_id_);
			formatter::format_to(std::back_inserter(result),
								 "\"source_id\":\"{}\",", source_id_);
			formatter::format_to(std::back_inserter(result),
								 "\"source_sub_id\":\"{}\",", source_sub_id_);
		}
		formatter::format_to(std::back_inserter(result),
							 "\"message_type\":\"{}\"", message_type_);
		formatter::format_to(std::back_inserter(result), ",\"version\":\"{}\"",
							 version_);
		formatter::format_to(std::back_inserter(result),
							 "}},"); // end header

		// values
		formatter::format_to(std::back_inserter(result), "\"values\":{{");
		bool first = true;
		for (auto& u : units_)
		{
			formatter::format_to(std::back_inserter(result),
								 first ? "{}" : ",{}", u->to_json());
			first = false;
		}
		formatter::format_to(std::back_inserter(result),
							 "}}"); // end values
		formatter::format_to(std::back_inserter(result), "}}");
		return result;
	}

	std::string value_container::datas(void) const
	{
		if (!parsed_data_)
		{
			return data_string_;
		}
		// Rebuild from top-level units
		std::string result;
		formatter::format_to(std::back_inserter(result), "@data={{");
		for (auto& u : units_)
		{
			formatter::format_to(std::back_inserter(result), "{}",
								 u->serialize());
		}
		formatter::format_to(std::back_inserter(result), "}};");
		return result;
	}

	void value_container::load_packet(const std::string& file_path)
	{
		auto [fileData, err] = file::load(file_path);
		if (!err.has_value())
		{
			deserialize(fileData, false);
		}
	}

	void value_container::save_packet(const std::string& file_path)
	{
		auto dataArr = serialize_array();
		file::save(file_path, dataArr);
	}

	std::vector<std::shared_ptr<value>> value_container::operator[](
		std::string_view key)
	{
		return value_array(key);
	}

	value_container operator<<(value_container tc, value& other)
	{
		tc.add(other);
		return tc;
	}

	value_container operator<<(value_container tc, std::shared_ptr<value> other)
	{
		tc.add(other);
		return tc;
	}

	std::shared_ptr<value_container> operator<<(
		std::shared_ptr<value_container> tc, value& other)
	{
		tc->add(other);
		return tc;
	}

	std::shared_ptr<value_container> operator<<(
		std::shared_ptr<value_container> tc, std::shared_ptr<value> other)
	{
		tc->add(other);
		return tc;
	}

	std::ostream& operator<<(std::ostream& out, value_container& other)
	{
		out << other.serialize();
		return out;
	}

	std::ostream& operator<<(std::ostream& out,
							 std::shared_ptr<value_container> other)
	{
		if (other)
			out << other->serialize();
		return out;
	}

	std::string& operator<<(std::string& out, value_container& other)
	{
		out = other.serialize();
		return out;
	}

	std::string& operator<<(std::string& out,
							std::shared_ptr<value_container> other)
	{
		if (other)
			out = other->serialize();
		else
			out.clear();
		return out;
	}

	bool value_container::deserialize_values(const std::string& data,
											 bool parse_only_header)
	{
		if (!units_.empty())
		{
			units_.clear();
		}
		changed_data_ = false;

		std::regex reData("@data=\\s*\\{\\s*(.*?)\\s*\\};");
		std::smatch match;
		if (!std::regex_search(data, match, reData))
		{
			data_string_ = "@data={};";
			parsed_data_ = true;
			return false;
		}
		data_string_ = match[0]; // entire "@data= ... ;"

		if (parse_only_header)
		{
			parsed_data_ = false;
			return true;
		}
		parsed_data_ = true;

		// parse items: [name,type,data];
		std::regex reItems("\\[(\\w+),\\s*(\\w+),\\s*(.*?)\\];");
		auto it = std::sregex_iterator(data_string_.begin(), data_string_.end(),
									   reItems);
		auto end = std::sregex_iterator();

		std::vector<std::shared_ptr<value>> temp_list;
		for (; it != end; ++it)
		{
			auto nameStr = (*it)[1].str();
			auto typeStr = (*it)[2].str();
			auto dataStr = (*it)[3].str();

			// convert string -> value_types
			auto vt = convert_value_type(typeStr);
			// see if we have a dynamic constructor
			auto dtIt = data_type_map_.find(vt);
			if (dtIt == data_type_map_.end())
			{
				// fallback to a simple null value
				temp_list.push_back(std::make_shared<value>(
					nameStr, nullptr, 0, value_types::null_value));
				continue;
			}
			temp_list.push_back(dtIt->second(nameStr, dataStr));
		}

		// Next, handle container nesting. We rely on the container_value
		// storing child counts.
		std::shared_ptr<value> currentContainer = nullptr;
		for (auto& tVal : temp_list)
		{
			if (!currentContainer)
			{
				// top-level => add to units_
				units_.push_back(tVal);
				tVal->set_parent(nullptr);

				if (tVal->is_container() && (tVal->to_long() > 0))
				{
					currentContainer = tVal;
				}
			}
			else
			{
				// add to current container
				// we must dynamic_cast to container_value to call add(...)
				// directly
				if (auto c = std::dynamic_pointer_cast<container_value>(
						currentContainer))
				{
					c->add(tVal, false);
				}

				if (tVal->is_container() && tVal->to_long() > 0)
				{
					currentContainer = tVal;
				}
				else
				{
					// Check if parent's child count is now matched
					while (currentContainer)
					{
						long needed = currentContainer->to_long();
						long have = (long)currentContainer->child_count();
						if (have >= needed)
						{
							// move up
							currentContainer = currentContainer->parent();
						}
						else
						{
							// break if still more children needed
							break;
						}
					}
				}
			}
		}
		return true;
	}

	void value_container::parsing(std::string_view source_name,
								  std::string_view target_name,
								  std::string_view target_value,
								  std::string& target_variable)
	{
		if (source_name == target_name)
		{
			target_variable = std::string(target_value);
			// trim
			if (!target_variable.empty())
			{
				// simplistic trim
				while (!target_variable.empty()
					   && (target_variable.front() == ' '))
				{
					target_variable.erase(target_variable.begin());
				}
				while (!target_variable.empty()
					   && (target_variable.back() == ' '))
				{
					target_variable.pop_back();
				}
			}
		}
	}
} // namespace container_module