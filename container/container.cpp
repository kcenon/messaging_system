#include "container.h"

#include "value_types.h"
#include "converting.h"
#include "file_handling.h"

#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/double_value.h"
#include "values/float_value.h"
#include "values/int_value.h"
#include "values/long_value.h"
#include "values/short_value.h"
#include "values/string_value.h"
#include "values/uint_value.h"
#include "values/ushort_value.h"
#include "values/container_value.h"

#include <io.h>
#include <fcntl.h>
#include <wchar.h>

#include <regex>
#include <locale>
#include <codecvt>
#include <algorithm>

#include "fmt/format.h"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace container
{
	using namespace converting;
	using namespace file_handling;

	values::values(void)
		: _source_id(L""), _source_sub_id(L""), _target_id(L""), _target_sub_id(L""), _message_type(L"data_container"), _version(L"1.0")
	{
	}

	values::values(const std::wstring& data_string, const bool& parse_only_header) : values()
	{
		deserialize(data_string, parse_only_header);
	}

	values::values(std::shared_ptr<values> data_container, const bool& parse_only_header) : values()
	{
		if (data_container == nullptr)
		{
			return;
		}

		deserialize(data_container->serialize(), parse_only_header);
	}

	values::values(const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& message_type,
		const std::vector<std::shared_ptr<value>>& units) : values()
	{
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	values::values(const std::wstring& source_id, const std::wstring& source_sub_id,
		const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& message_type,
		const std::vector<std::shared_ptr<value>>& units) : values()
	{
		set_source(source_id, source_sub_id);
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	values::~values(void)
	{
	}

	std::shared_ptr<values> values::get_ptr(void)
	{
		return shared_from_this();
	}

	void values::set_source(const std::wstring& source_id, const std::wstring& source_sub_id)
	{
		_source_id = source_id;
		_source_sub_id = source_sub_id;
	}

	void values::set_target(const std::wstring& target_id, const std::wstring& target_sub_id)
	{
		_target_id = target_id;
		_target_sub_id = target_sub_id;
	}

	void values::set_message_type(const std::wstring& message_type)
	{
		_message_type = message_type;
	}

	std::wstring values::source_id(void) const
	{
		return _source_id;
	}

	std::wstring values::source_sub_id(void) const
	{
		return _source_sub_id;
	}

	std::wstring values::target_id(void) const
	{
		return _target_id;
	}

	std::wstring values::target_sub_id(void) const
	{
		return _target_sub_id;
	}

	std::wstring values::message_type(void) const
	{
		return _message_type;
	}

	void values::set_units(const std::vector<std::shared_ptr<value>>& target_values)
	{
		std::vector<std::shared_ptr<value>>::iterator target;
		for (auto& target_value : target_values)
		{
			target = std::find_if(_units.begin(), _units.end(),
				[&target_value](std::shared_ptr<value> item)
				{
					return item == target_value;
				});

			if (target != _units.end())
			{
				continue;
			}

			_units.push_back(target_value);
		}
	}

	void values::swap_header(void)
	{
		std::wstring temp = _source_id;
		_source_id = _target_id;
		_target_id = temp;

		temp = _source_sub_id;
		_source_sub_id = _target_sub_id;
		_target_sub_id = temp;
	}

	void values::clear_value(void)
	{
		_units.clear();
	}

	std::shared_ptr<value> values::add(std::shared_ptr<value> target_value)
	{
		std::vector<std::shared_ptr<value>>::iterator target;
		target = std::find_if(_units.begin(), _units.end(),
			[&target_value](std::shared_ptr<value> item)
			{
				return item == target_value;
			});

		if (target != _units.end())
		{
			return nullptr;
		}

		_units.push_back(target_value);
		target_value->set_parent(nullptr);

		return target_value;
	}

	void values::remove(const std::wstring& target_name)
	{
		std::vector<std::shared_ptr<value>>::iterator target;

		while (true) {
			target = std::find_if(_units.begin(), _units.end(),
				[&target_name](std::shared_ptr<value> item)
				{
					return item->name() == target_name;
				});

			if (target == _units.end())
			{
				break;
			}

			_units.erase(target);
		}
	}

	void values::remove(std::shared_ptr<value> target_value)
	{
		std::vector<std::shared_ptr<value>>::iterator target;
		target = std::find_if(_units.begin(), _units.end(),
			[&target_value](std::shared_ptr<value> item)
			{
				return item == target_value;
			});

		if (target == _units.end())
		{
			return;
		}

		_units.erase(target);
	}

	std::vector<std::shared_ptr<value>> values::value_array(const std::wstring& target_name)
	{
		std::vector<std::shared_ptr<value>> result_list;

		std::for_each(_units.begin(), _units.end(), [&target_name, &result_list](std::shared_ptr<value> source) {
			if (source->name() == target_name)
			{
				result_list.push_back(source);
			}
			});

		return result_list;
	}

	void values::initialize(void)
	{
		_source_id = L"";
		_source_sub_id = L"";
		_target_id = L"";
		_target_sub_id = L"";
		_message_type = L"data_container";
		_version = L"1.0";

		_units.clear();
	}

	std::wstring values::serialize(const bool& contain_whitespace)
	{
		fmt::wmemory_buffer result;

		std::wstring new_line_string = L"";
		std::wstring tab_string = L"";
		if (contain_whitespace)
		{
			new_line_string = L"\n";
			tab_string = L"\t";
		}

		// header
		fmt::format_to(std::back_inserter(result), L"@header={}{}{}", new_line_string, L"{", new_line_string);
		if (_message_type != L"data_container")
		{
			fmt::format_to(std::back_inserter(result), L"[target_id,{}{}];{}", tab_string, _target_id, new_line_string);
			fmt::format_to(std::back_inserter(result), L"[target_sub_id,{}{}];{}", tab_string, _target_sub_id, new_line_string);
			fmt::format_to(std::back_inserter(result), L"[source_id,{}{}];{}", tab_string, _source_id, new_line_string);
			fmt::format_to(std::back_inserter(result), L"[source_sub_id,{}{}];{}", tab_string, _source_sub_id, new_line_string);
		}
		fmt::format_to(std::back_inserter(result), L"[message_type,{}{}];{}", tab_string, _message_type, new_line_string);
		fmt::format_to(std::back_inserter(result), L"[version,{}{}];{}", tab_string, _version, new_line_string);

		// data
		fmt::format_to(std::back_inserter(result), L"@data={}{}{}", new_line_string, L"{", new_line_string);
		for (auto& unit : _units)
		{
			fmt::format_to(std::back_inserter(result), L"{}", unit->serialize(contain_whitespace, 1));
		}
		fmt::format_to(std::back_inserter(result), L"{}", L"};");

		return result.data();
	}

	bool values::deserialize(const std::wstring& data_string, const bool& parse_only_header)
	{
		initialize();

		if (data_string.empty())
		{
			return false;
		}

		std::wregex newlines_re(L"\\r\\n?|\\n");
		std::wstring removed_newline = std::regex_replace(data_string, newlines_re, L"");

		std::wregex full_condition(L"@header=[\\s?]*\\{[\\s?]*(.*?)[\\s?]*\\};");
		std::wsregex_iterator full_iter(removed_newline.begin(), removed_newline.end(), full_condition);
		std::wsregex_iterator full_end;
		if (full_iter == full_end)
		{
			return deserialize_values(removed_newline);
		}

		std::wstring temp = (*full_iter)[1];
		std::wregex header_condition(L"\\[(\\w+),(.*?)\\];");
		std::wsregex_iterator header_iter(temp.begin(), temp.end(), header_condition);
		std::wsregex_iterator header_end;
		while (header_iter != header_end)
		{
			parsing((*header_iter)[1], L"target_id", (*header_iter)[2], _target_id);
			parsing((*header_iter)[1], L"target_sub_id", (*header_iter)[2], _target_sub_id);
			parsing((*header_iter)[1], L"source_id", (*header_iter)[2], _source_id);
			parsing((*header_iter)[1], L"source_sub_id", (*header_iter)[2], _source_sub_id);
			parsing((*header_iter)[1], L"message_type", (*header_iter)[2], _message_type);
			parsing((*header_iter)[1], L"version", (*header_iter)[2], _version);

			header_iter++;
		}

		if (parse_only_header)
		{
			return true;
		}

		return deserialize_values(removed_newline);
	}

	std::wstring values::datas(void) const
	{
		fmt::wmemory_buffer result;

		// data
		fmt::format_to(std::back_inserter(result), L"@data={}", L"{");
		for (auto& unit : _units)
		{
			fmt::format_to(std::back_inserter(result), L"{}", unit->serialize(false, 1));
		}
		fmt::format_to(std::back_inserter(result), L"{}", L"};");

		return result.data();
	}

	void values::load_packet(const std::wstring& file_path)
	{
		deserialize(converter::to_wstring(file_handler::load(file_path)));
	}

	void values::save_packet(const std::wstring& file_path, const bool& contain_whitespace)
	{
		file_handler::save(file_path, converter::to_array(serialize(contain_whitespace)));
	}

	std::shared_ptr<value> values::operator[](const std::wstring& key)
	{
		std::vector<std::shared_ptr<value>> searched_values = value_array(key);
		if (searched_values.empty())
		{
			return std::make_shared<value>(key);
		}

		return searched_values[0];
	}

	std::shared_ptr<values> operator<<(std::shared_ptr<values> target_container, std::shared_ptr<value> other)
	{
		target_container->add(other);

		return target_container;
	}

	std::ostream& operator <<(std::ostream& out, std::shared_ptr<values> other) // output
	{
		out << converter::to_string(other->serialize(false));

		return out;
	}

	std::wostream& operator <<(std::wostream& out, std::shared_ptr<values> other) // output
	{
		out << other->serialize(false);

		return out;
	}

	std::string& operator <<(std::string& out, std::shared_ptr<values> other)
	{
		out = converter::to_string(other->serialize(false));

		return out;
	}

	std::wstring& operator <<(std::wstring& out, std::shared_ptr<values> other)
	{
		out = other->serialize(false);

		return out;
	}

	bool values::deserialize_values(const std::wstring& data)
	{
		if (_units.size() > 0)
		{
			_units.clear();
		}

		std::wregex full_condition(L"@data=[\\s?]*\\{[\\s?]*(.*?)[\\s?]*\\};");
		std::wsregex_iterator full_iter(data.begin(), data.end(), full_condition);
		std::wsregex_iterator full_end;
		if (full_iter == full_end)
		{
			return false;
		}

		std::wstring regex_temp = (*full_iter)[0].str();

		std::wregex regex_condition(L"\\[(\\w+),[\\s?]*(\\w+),[\\s?]*(.*?)\\];");
		std::wsregex_iterator start(regex_temp.begin(), regex_temp.end(), regex_condition);
		std::wsregex_iterator end;

		value_types type;
		std::wstring name;
		std::wstring temp;
		std::vector<std::shared_ptr<value>> temp_list;
		while (start != end)
		{
			name = (*start)[1];
			type = convert_value_type((*start)[2]);
			temp = (*start)[3];

			switch (type)
			{
			case value_types::bool_value: temp_list.push_back(std::make_shared<bool_value>(name, temp)); break;
			case value_types::short_value: temp_list.push_back(std::make_shared<short_value>(name, (short)_wtoi(temp.c_str()))); break;
			case value_types::ushort_value: temp_list.push_back(std::make_shared<ushort_value>(name, (unsigned short)_wtoi(temp.c_str()))); break;
			case value_types::int_value: temp_list.push_back(std::make_shared<int_value>(name, (int)_wtoi(temp.c_str()))); break;
			case value_types::uint_value: temp_list.push_back(std::make_shared<uint_value>(name, (unsigned int)_wtoi(temp.c_str()))); break;
			case value_types::long_value: temp_list.push_back(std::make_shared<long_value>(name, (long)_wtol(temp.c_str()))); break;
			case value_types::ulong_value: temp_list.push_back(std::make_shared<long_value>(name, (unsigned long)_wtol(temp.c_str()))); break;
			case value_types::llong_value: temp_list.push_back(std::make_shared<long_value>(name, (long long)_wtoll(temp.c_str()))); break;
			case value_types::ullong_value: temp_list.push_back(std::make_shared<long_value>(name, (unsigned long long)_wtoll(temp.c_str()))); break;
			case value_types::float_value: temp_list.push_back(std::make_shared<float_value>(name, (float)_wtof(temp.c_str()))); break;
			case value_types::double_value: temp_list.push_back(std::make_shared<double_value>(name, (double)_wtof(temp.c_str()))); break;
			case value_types::bytes_value: temp_list.push_back(std::make_shared<bytes_value>(name, converter::from_base64(temp.c_str()))); break;
			case value_types::string_value: temp_list.push_back(std::make_shared<string_value>(name, temp)); break;
			case value_types::container_value: temp_list.push_back(std::make_shared<container_value>(name, (long)_wtol(temp.c_str()))); break;
			default: temp_list.push_back(std::make_shared<value>(name, nullptr, 0, value_types::null_value)); break;
			}

			start++;
		}

		container_value* container = nullptr;
		std::vector<std::shared_ptr<value>>::iterator iterator;
		for (iterator = temp_list.begin(); iterator != temp_list.end(); ++iterator)
		{
			if (container == nullptr)
			{
				add(*iterator);

				if ((*iterator)->is_container() != true || (*iterator)->to_long() == 0)
				{
					continue;
				}

				container = (container_value*)(*iterator).get();

				continue;
			}

			container->add(*iterator, false);

			if ((*iterator)->is_container() == true)
			{
				container = (container_value*)(*iterator).get();

				continue;
			}

			while (container != nullptr && container->to_long() == container->child_count())
			{
				container = (container_value*)(container->parent().get());
			}
		}

		return true;
	}

	void values::parsing(const std::wstring& source_name, const std::wstring& target_name, const std::wstring& target_value, std::wstring& target_variable)
	{
		if (source_name != target_name)
		{
			return;
		}

		target_variable = target_value;
		boost::trim(target_variable);

		return;
	}
}