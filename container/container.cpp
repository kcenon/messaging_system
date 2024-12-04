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

#include "container.h"

#include "formatter.h"
#include "file_handler.h"
#include "convert_string.h"

#include "value_types.h"
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

#include <fcntl.h>
#include <wchar.h>

#include <regex>
#include <sstream>

namespace container
{
	constexpr auto TARGET_ID = "1";
	constexpr auto TARGET_SUB_ID = "2";
	constexpr auto SOURCE_ID = "3";
	constexpr auto SOURCE_SUB_ID = "4";
	constexpr auto MESSAGE_TYPE = "5";
	constexpr auto MESSAGE_VERSION = "6";

	using namespace utility_module;

	value_container::value_container(void)
		: source_id_("")
		, source_sub_id_("")
		, target_id_("")
		, target_sub_id_("")
		, message_type_("data_container")
		, version_("1.0.0.0")
		, parsed_data_(true)
		, data_string_("@data={};")
		, changed_data_(false)
	{
		data_type_map_.insert(
			{ value_types::bool_value,
			  std::bind(&value_container::set_boolean, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::short_value,
			  std::bind(&value_container::set_short, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::ushort_value,
			  std::bind(&value_container::set_ushort, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::int_value,
			  std::bind(&value_container::set_int, this, std::placeholders::_1,
						std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::uint_value,
			  std::bind(&value_container::set_uint, this, std::placeholders::_1,
						std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::long_value,
			  std::bind(&value_container::set_long, this, std::placeholders::_1,
						std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::ulong_value,
			  std::bind(&value_container::set_ulong, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::llong_value,
			  std::bind(&value_container::set_llong, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::ullong_value,
			  std::bind(&value_container::set_ullong, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::float_value,
			  std::bind(&value_container::set_float, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::double_value,
			  std::bind(&value_container::set_double, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::bytes_value,
			  std::bind(&value_container::set_bytes, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::string_value,
			  std::bind(&value_container::set_string, this,
						std::placeholders::_1, std::placeholders::_2) });
		data_type_map_.insert(
			{ value_types::container_value,
			  std::bind(&value_container::set_container, this,
						std::placeholders::_1, std::placeholders::_2) });
	}

	value_container::value_container(const std::string& data_string,
									 const bool& parse_only_header)
		: value_container()
	{
		deserialize(data_string, parse_only_header);
	}

	value_container::value_container(const std::vector<uint8_t>& data_array,
									 const bool& parse_only_header)
		: value_container()
	{
		deserialize(data_array, parse_only_header);
	}

	value_container::value_container(const value_container& data_container,
									 const bool& parse_only_header)
		: value_container()
	{
		deserialize(data_container.serialize(), parse_only_header);
	}

	value_container::value_container(
		std::shared_ptr<value_container> data_container,
		const bool& parse_only_header)
		: value_container()
	{
		if (data_container == nullptr)
		{
			return;
		}

		deserialize(data_container->serialize(), parse_only_header);
	}

	value_container::value_container(
		const std::string& message_type,
		const std::vector<std::shared_ptr<value>>& units)
		: value_container()
	{
		set_message_type(message_type);
		set_units(units);
	}

	value_container::value_container(
		const std::string& target_id,
		const std::string& target_sub_id,
		const std::string& message_type,
		const std::vector<std::shared_ptr<value>>& units)
		: value_container()
	{
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	value_container::value_container(
		const std::string& source_id,
		const std::string& source_sub_id,
		const std::string& target_id,
		const std::string& target_sub_id,
		const std::string& message_type,
		const std::vector<std::shared_ptr<value>>& units)
		: value_container()
	{
		set_source(source_id, source_sub_id);
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	value_container::~value_container(void) {}

	std::shared_ptr<value_container> value_container::get_ptr(void)
	{
		return shared_from_this();
	}

	void value_container::set_source(const std::string& source_id,
									 const std::string& source_sub_id)
	{
		source_id_ = source_id;
		source_sub_id_ = source_sub_id;
	}

	void value_container::set_target(const std::string& target_id,
									 const std::string& target_sub_id)
	{
		target_id_ = target_id;
		target_sub_id_ = target_sub_id;
	}

	void value_container::set_message_type(const std::string& message_type)
	{
		message_type_ = message_type;
	}

	std::string value_container::source_id(void) const { return source_id_; }

	std::string value_container::source_sub_id(void) const
	{
		return source_sub_id_;
	}

	std::string value_container::target_id(void) const { return target_id_; }

	std::string value_container::target_sub_id(void) const
	{
		return target_sub_id_;
	}

	std::string value_container::message_type(void) const
	{
		return message_type_;
	}

	void value_container::set_units(
		const std::vector<std::shared_ptr<value>>& target_values,
		const bool& update_immediately)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::vector<std::shared_ptr<value>>::iterator target;
		for (auto& target_value : target_values)
		{
			target = find_if(units_.begin(), units_.end(),
							 [&target_value](std::shared_ptr<value> item)
							 { return item == target_value; });

			if (target != units_.end())
			{
				continue;
			}

			units_.push_back(target_value);
			target_value->set_parent(nullptr);
		}

		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}
	}

	void value_container::swap_header(void)
	{
		std::string temp = source_id_;
		source_id_ = target_id_;
		target_id_ = std::move(temp);

		temp = source_sub_id_;
		source_sub_id_ = target_sub_id_;
		target_sub_id_ = std::move(temp);
	}

	void value_container::clear_value(void)
	{
		parsed_data_ = true;
		changed_data_ = false;
		data_string_ = "@data={};";
		units_.clear();
	}

	std::shared_ptr<value_container> value_container::copy(
		const bool& containing_values)
	{
		std::shared_ptr<value_container> new_container
			= std::make_shared<value_container>(serialize(),
												!containing_values);
		if (new_container == nullptr)
		{
			return nullptr;
		}

		if (!containing_values)
		{
			new_container->clear_value();
		}

		return new_container;
	}

	std::shared_ptr<value> value_container::add(const value& target_value,
												const bool& update_immediately)
	{
		auto target = data_type_map_.find(target_value.type());
		if (target == data_type_map_.end())
		{
			return add(std::make_shared<value>(target_value.name(), nullptr, 0,
											   value_types::null_value),
					   update_immediately);
		}

		return add(
			target->second(target_value.name(), target_value.to_string()),
			update_immediately);
	}

	std::shared_ptr<value> value_container::add(
		std::shared_ptr<value> target_value, const bool& update_immediately)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::vector<std::shared_ptr<value>>::iterator target;
		target = find_if(units_.begin(), units_.end(),
						 [&target_value](std::shared_ptr<value> item)
						 { return item == target_value; });

		if (target != units_.end())
		{
			return nullptr;
		}

		units_.push_back(target_value);
		target_value->set_parent(nullptr);

		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}

		return target_value;
	}

	void value_container::remove(const std::string& target_name,
								 const bool& update_immediately)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::vector<std::shared_ptr<value>>::iterator target;

		while (true)
		{
			target = find_if(units_.begin(), units_.end(),
							 [&target_name](std::shared_ptr<value> item)
							 { return item->name() == target_name; });

			if (target == units_.end())
			{
				break;
			}

			units_.erase(target);
		}

		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}
	}

	void value_container::remove(std::shared_ptr<value> target_value,
								 const bool& update_immediately)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::vector<std::shared_ptr<value>>::iterator target;
		target = find_if(units_.begin(), units_.end(),
						 [&target_value](std::shared_ptr<value> item)
						 { return item == target_value; });

		if (target == units_.end())
		{
			return;
		}

		units_.erase(target);

		changed_data_ = !update_immediately;
		if (update_immediately)
		{
			data_string_ = datas();
		}
	}

	std::vector<std::shared_ptr<value>> value_container::value_array(
		const std::string& target_name)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::vector<std::shared_ptr<value>> result_list;

		for_each(units_.begin(), units_.end(),
				 [&target_name, &result_list](std::shared_ptr<value> source)
				 {
					 if (source->name() == target_name)
					 {
						 result_list.push_back(source);
					 }
				 });

		return result_list;
	}

	std::shared_ptr<value> value_container::get_value(
		const std::string& target_name, const unsigned int& index)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::vector<std::shared_ptr<value>> result_list
			= value_array(target_name);
		if (result_list.empty())
		{
			return std::make_shared<value>(target_name);
		}

		if (index >= result_list.size())
		{
			return std::make_shared<value>(target_name);
		}

		return result_list[index];
	}

	void value_container::initialize(void)
	{
		source_id_ = "";
		source_sub_id_ = "";
		target_id_ = "";
		target_sub_id_ = "";
		message_type_ = "data_container";
		version_ = "1.0";

		clear_value();
	}

	std::string value_container::serialize(void) const
	{
		std::string data_string = data_string_;
		if (parsed_data_)
		{
			data_string = datas();
		}

		std::string result;

		// header
		formatter::format_to(std::back_inserter(result), "@header={}", "{");
		if (message_type_ != "data_container")
		{
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 TARGET_ID, target_id_);
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 TARGET_SUB_ID, target_sub_id_);
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 SOURCE_ID, source_id_);
			formatter::format_to(std::back_inserter(result), "[{},{}];",
								 SOURCE_SUB_ID, source_sub_id_);
		}
		formatter::format_to(std::back_inserter(result), "[{},{}];",
							 MESSAGE_TYPE, message_type_);
		formatter::format_to(std::back_inserter(result), "[{},{}];",
							 MESSAGE_VERSION, version_);
		formatter::format_to(std::back_inserter(result), "{}", "};");
		formatter::format_to(std::back_inserter(result), "{}", data_string);

		return result;
	}

	std::vector<uint8_t> value_container::serialize_array(void) const
	{
		auto [value, value_error] = convert_string::to_array(serialize());
		if (value_error.has_value())
		{
			return std::vector<uint8_t>();
		}

		return value.value();
	}

	bool value_container::deserialize(const std::string& data_string,
									  const bool& parse_only_header)
	{
		initialize();

		if (data_string.empty())
		{
			return false;
		}

		std::regex newlines_re("\\r\\n?|\\n");
		std::string removed_newline
			= regex_replace(data_string, newlines_re, "");

		std::regex full_condition("@header=[\\s?]*\\{[\\s?]*(.*?)[\\s?]*\\};");
		std::sregex_iterator full_iter(removed_newline.begin(),
									   removed_newline.end(), full_condition);
		std::sregex_iterator full_end;
		if (full_iter == full_end)
		{
			return deserialize_values(removed_newline, parse_only_header);
		}

		std::string temp = (*full_iter)[1];
		std::regex header_condition("\\[(\\w+),(.*?)\\];");
		std::sregex_iterator header_iter(temp.begin(), temp.end(),
										 header_condition);
		std::sregex_iterator header_end;
		while (header_iter != header_end)
		{
			parsing((*header_iter)[1], TARGET_ID, (*header_iter)[2],
					target_id_);
			parsing((*header_iter)[1], TARGET_SUB_ID, (*header_iter)[2],
					target_sub_id_);
			parsing((*header_iter)[1], SOURCE_ID, (*header_iter)[2],
					source_id_);
			parsing((*header_iter)[1], SOURCE_SUB_ID, (*header_iter)[2],
					source_sub_id_);
			parsing((*header_iter)[1], MESSAGE_TYPE, (*header_iter)[2],
					message_type_);
			parsing((*header_iter)[1], MESSAGE_VERSION, (*header_iter)[2],
					version_);

			header_iter++;
		}

		return deserialize_values(removed_newline, parse_only_header);
	}

	bool value_container::deserialize(const std::vector<uint8_t>& data_array,
									  const bool& parse_only_header)
	{
		auto [utf8, convert_error] = convert_string::to_string(data_array);
		if (convert_error.has_value())
		{
			return false;
		}

		return deserialize(utf8.value(), parse_only_header);
	}

	const std::string value_container::to_xml(void)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::string result;

		formatter::format_to(std::back_inserter(result), "{}", "<container>");
		formatter::format_to(std::back_inserter(result), "{}", "<header>");
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
		formatter::format_to(std::back_inserter(result), "{}", "</header>");

		formatter::format_to(std::back_inserter(result), "{}", "<values>");
		for (auto& unit : units_)
		{
			formatter::format_to(std::back_inserter(result), "{}",
								 unit->to_xml());
		}
		formatter::format_to(std::back_inserter(result), "{}", "</values>");
		formatter::format_to(std::back_inserter(result), "{}", "</container>");

		return result;
	}

	const std::string value_container::to_json(void)
	{
		if (!parsed_data_)
		{
			deserialize_values(data_string_, false);
		}

		std::string result;

		formatter::format_to(std::back_inserter(result), "{}", "{");
		formatter::format_to(std::back_inserter(result), "{}", "\"header\":{");
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
		formatter::format_to(std::back_inserter(result), "{}", "}");

		formatter::format_to(std::back_inserter(result), ",{}", "\"values\":{");

		bool first = true;
		for (auto& unit : units_)
		{
			formatter::format_to(std::back_inserter(result),
								 first ? "{}" : ",{}", unit->to_json());
			first = false;
		}

		formatter::format_to(std::back_inserter(result), "{}", "}");
		formatter::format_to(std::back_inserter(result), "{}", "}");

		return result;
		;
	}

	std::string value_container::datas(void) const
	{
		if (!parsed_data_)
		{
			return data_string_;
		}

		std::string result;

		// data
		formatter::format_to(std::back_inserter(result), "@data={}", "{");
		for (auto& unit : units_)
		{
			formatter::format_to(std::back_inserter(result), "{}",
								 unit->serialize());
		}
		formatter::format_to(std::back_inserter(result), "{}", "};");

		return result;
	}

	void value_container::load_packet(const std::string& file_path)
	{
		auto [data, load_error] = file::load(file_path);
		if (load_error.has_value())
		{
			return;
		}

		deserialize(data);
	}

	void value_container::save_packet(const std::string& file_path)
	{
		auto [value, value_error] = convert_string::to_array(serialize());
		if (value_error.has_value())
		{
			return;
		}

		file::save(file_path, value.value());
	}

	std::vector<std::shared_ptr<value>> value_container::operator[](
		const std::string& key)
	{
		return value_array(key);
	}

	value_container operator<<(value_container target_container, value& other)
	{
		target_container.add(other);

		return target_container;
	}

	value_container operator<<(value_container target_container,
							   std::shared_ptr<value> other)
	{
		target_container.add(other);

		return target_container;
	}

	std::shared_ptr<value_container> operator<<(
		std::shared_ptr<value_container> target_container, value& other)
	{
		target_container->add(other);

		return target_container;
	}

	std::shared_ptr<value_container> operator<<(
		std::shared_ptr<value_container> target_container,
		std::shared_ptr<value> other)
	{
		target_container->add(other);

		return target_container;
	}

	std::ostream& operator<<(std::ostream& out,
							 value_container& other) // output
	{
		auto [utf8, convert_error]
			= convert_string::system_to_utf8(other.serialize());
		if (convert_error.has_value())
		{
			return out;
		}

		out << utf8.value();

		return out;
	}

	std::ostream& operator<<(std::ostream& out,
							 std::shared_ptr<value_container> other) // output
	{
		auto [utf8, convert_error]
			= convert_string::system_to_utf8(other->serialize());
		if (convert_error.has_value())
		{
			return out;
		}

		out << utf8.value();

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
		out = other->serialize();

		return out;
	}

	bool value_container::deserialize_values(const std::string& data,
											 const bool& parse_only_header)
	{
		if (units_.size() > 0)
		{
			units_.clear();
		}

		changed_data_ = false;

		std::regex full_condition("@data=[\\s?]*\\{[\\s?]*(.*?)[\\s?]*\\};");
		std::sregex_iterator full_iter(data.begin(), data.end(),
									   full_condition);
		std::sregex_iterator full_end;
		if (full_iter == full_end)
		{
			data_string_ = "@data={};";
			parsed_data_ = true;

			return false;
		}

		data_string_ = (*full_iter)[0].str();

		if (parse_only_header)
		{
			parsed_data_ = false;

			return true;
		}

		parsed_data_ = true;

		std::regex regex_condition("\\[(\\w+),[\\s?]*(\\w+),[\\s?]*(.*?)\\];");
		std::sregex_iterator start(data_string_.begin(), data_string_.end(),
								   regex_condition);
		std::sregex_iterator end;

		std::vector<std::shared_ptr<value>> temp_list;
		while (start != end)
		{
			auto target = data_type_map_.find(convert_value_type((*start)[2]));
			if (target == data_type_map_.end())
			{
				temp_list.push_back(std::make_shared<value>(
					(*start)[1], nullptr, 0, value_types::null_value));
				continue;
			}

			temp_list.push_back(target->second((*start)[1], (*start)[3]));

			start++;
		}

		std::shared_ptr<value> container = nullptr;
		std::vector<std::shared_ptr<value>>::iterator iterator;
		for (iterator = temp_list.begin(); iterator != temp_list.end();
			 ++iterator)
		{
			if (container == nullptr)
			{
				add(*iterator);

				if ((*iterator)->is_container() != true
					|| (*iterator)->to_long() == 0)
				{
					continue;
				}

				container = *iterator;

				continue;
			}

			container->add(*iterator, false);

			if ((*iterator)->is_container() == true)
			{
				container = *iterator;

				continue;
			}

			while (container != nullptr
				   && container->to_long() == container->child_count())
			{
				container = container->parent();
			}
		}

		return true;
	}

	void value_container::parsing(const std::string& source_name,
								  const std::string& target_name,
								  const std::string& target_value,
								  std::string& target_variable)
	{
		if (source_name != target_name)
		{
			return;
		}

		target_variable = target_value;

		if (target_value.find_first_not_of(' ') != std::string::npos)
		{
			target_variable
				= target_variable.erase(target_value.find_last_not_of(' ') + 1);
		}

		return;
	}

	std::shared_ptr<value> value_container::set_boolean(const std::string& name,
														const std::string& data)
	{
		return std::make_shared<bool_value>(name, data);
	}

	std::shared_ptr<value> value_container::set_short(const std::string& name,
													  const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<short_value>(name,
											 (short)atoi(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_ushort(const std::string& name,
													   const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<ushort_value>(
			name, (unsigned short)atoi(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_int(const std::string& name,
													const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<int_value>(name,
										   (int)atoi(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_uint(const std::string& name,
													 const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<uint_value>(
			name, (unsigned int)atoi(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_long(const std::string& name,
													 const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<long_value>(name,
											(long)atol(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_ulong(const std::string& name,
													  const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<ulong_value>(
			name, (unsigned long)atol(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_llong(const std::string& name,
													  const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<llong_value>(
			name, (long long)atoll(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_ullong(const std::string& name,
													   const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<ullong_value>(
			name, (unsigned long long)atoll(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_float(const std::string& name,
													  const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<float_value>(name,
											 (float)atof(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_double(const std::string& name,
													   const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<double_value>(
			name, (double)atof(utf8.value().c_str()));
	}

	std::shared_ptr<value> value_container::set_bytes(const std::string& name,
													  const std::string& data)
	{
		auto [value, convert_error] = convert_string::from_base64(data.c_str());
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<bytes_value>(name, value);
	}

	std::shared_ptr<value> value_container::set_string(const std::string& name,
													   const std::string& data)
	{
		return std::make_shared<string_value>(name, data);
	}

	std::shared_ptr<value> value_container::set_container(
		const std::string& name, const std::string& data)
	{
		auto [utf8, convert_error] = convert_string::system_to_utf8(data);
		if (convert_error.has_value())
		{
			return std::make_shared<short_value>(name, 0);
		}

		return std::make_shared<container_value>(
			name, (long)atol(utf8.value().c_str()));
	}
} // namespace container