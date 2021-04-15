#include "container.h"

#include "value_types.h"
#include "converting.h"
#include "file_handling.h"

#include "values/container_value.h"

#include <io.h>
#include <fcntl.h>
#include <wchar.h>

#include <regex>
#include <sstream>

#include "fmt/format.h"

namespace container
{
	using namespace converting;
	using namespace file_handling;

	value_container::value_container(void)
		: _source_id(L""), _source_sub_id(L""), _target_id(L""), _target_sub_id(L""), _message_type(L"data_container"), _version(L"1.0")
	{
	}

	value_container::value_container(const std::wstring& data_string, const bool& parse_only_header) : value_container()
	{
		deserialize(data_string, parse_only_header);
	}

	value_container::value_container(const std::vector<unsigned char>& data_array, const bool& parse_only_header) : value_container()
	{
		deserialize(data_array, parse_only_header);
	}

	value_container::value_container(const value_container& data_container, const bool& parse_only_header) : value_container()
	{
		deserialize(data_container.serialize(), parse_only_header);
	}

	value_container::value_container(std::shared_ptr<value_container> data_container, const bool& parse_only_header) : value_container()
	{
		if (data_container == nullptr)
		{
			return;
		}

		deserialize(data_container->serialize(), parse_only_header);
	}

	value_container::value_container(const std::wstring& message_type,
		const std::vector<std::shared_ptr<value>>& units) : value_container()
	{
		set_message_type(message_type);
		set_units(units);
	}

	value_container::value_container(const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& message_type,
		const std::vector<std::shared_ptr<value>>& units) : value_container()
	{
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	value_container::value_container(const std::wstring& source_id, const std::wstring& source_sub_id,
		const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& message_type,
		const std::vector<std::shared_ptr<value>>& units) : value_container()
	{
		set_source(source_id, source_sub_id);
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	value_container::~value_container(void)
	{
	}

	std::shared_ptr<value_container> value_container::get_ptr(void)
	{
		return shared_from_this();
	}

	void value_container::set_source(const std::wstring& source_id, const std::wstring& source_sub_id)
	{
		_source_id = source_id;
		_source_sub_id = source_sub_id;
	}

	void value_container::set_target(const std::wstring& target_id, const std::wstring& target_sub_id)
	{
		_target_id = target_id;
		_target_sub_id = target_sub_id;
	}

	void value_container::set_message_type(const std::wstring& message_type)
	{
		_message_type = message_type;
	}

	std::wstring value_container::source_id(void) const
	{
		return _source_id;
	}

	std::wstring value_container::source_sub_id(void) const
	{
		return _source_sub_id;
	}

	std::wstring value_container::target_id(void) const
	{
		return _target_id;
	}

	std::wstring value_container::target_sub_id(void) const
	{
		return _target_sub_id;
	}

	std::wstring value_container::message_type(void) const
	{
		return _message_type;
	}

	void value_container::set_units(const std::vector<std::shared_ptr<value>>& target_values)
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
			target_value->set_parent(nullptr);
		}
	}

	void value_container::swap_header(void)
	{
		std::wstring temp = _source_id;
		_source_id = _target_id;
		_target_id = std::move(temp);

		temp = _source_sub_id;
		_source_sub_id = _target_sub_id;
		_target_sub_id = std::move(temp);
	}

	void value_container::clear_value(void)
	{
		_units.clear();
	}

	std::shared_ptr<value_container> value_container::copy(const bool& swap_header, const bool& parse_only_header)
	{
		std::shared_ptr<value_container> new_container = std::make_shared<value_container>(serialize(), parse_only_header);
		if (swap_header)
		{
			new_container->swap_header();
		}

		return new_container;
	}

	std::shared_ptr<value> value_container::add(const value& target_value)
	{
		return add(value::generate_value(target_value.name(), convert_value_type(target_value.type()), target_value.to_string()));
	}

	std::shared_ptr<value> value_container::add(std::shared_ptr<value> target_value)
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

	void value_container::remove(const std::wstring& target_name)
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

	void value_container::remove(std::shared_ptr<value> target_value)
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

	std::vector<std::shared_ptr<value>> value_container::value_array(const std::wstring& target_name)
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

	std::shared_ptr<value> value_container::get_value(const std::wstring& target_name, const unsigned int& index)
	{
		std::vector<std::shared_ptr<value>> result_list = value_array(target_name);
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
		_source_id = L"";
		_source_sub_id = L"";
		_target_id = L"";
		_target_sub_id = L"";
		_message_type = L"data_container";
		_version = L"1.0";

		_units.clear();
	}

	std::wstring value_container::serialize(const bool& contain_whitespace) const
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
			fmt::format_to(std::back_inserter(result), L"{}[target_id,{}{}];{}", tab_string, tab_string, _target_id, new_line_string);
			fmt::format_to(std::back_inserter(result), L"{}[target_sub_id,{}{}];{}", tab_string, tab_string, _target_sub_id, new_line_string);
			fmt::format_to(std::back_inserter(result), L"{}[source_id,{}{}];{}", tab_string, tab_string, _source_id, new_line_string);
			fmt::format_to(std::back_inserter(result), L"{}[source_sub_id,{}{}];{}", tab_string, tab_string, _source_sub_id, new_line_string);
		}
		fmt::format_to(std::back_inserter(result), L"{}[message_type,{}{}];{}", tab_string, tab_string, _message_type, new_line_string);
		fmt::format_to(std::back_inserter(result), L"{}[version,{}{}];{}", tab_string, tab_string, _version, new_line_string);
		fmt::format_to(std::back_inserter(result), L"{}{}", L"};", new_line_string);

		// data
		fmt::format_to(std::back_inserter(result), L"@data={}{}{}", new_line_string, L"{", new_line_string);
		for (auto& unit : _units)
		{
			fmt::format_to(std::back_inserter(result), L"{}", unit->serialize(contain_whitespace, 1));
		}
		fmt::format_to(std::back_inserter(result), L"{}", L"};");

		return result.data();
	}

	std::vector<unsigned char> value_container::serialize_array(const bool& contain_whitespace) const
	{
		return converter::to_array(serialize(contain_whitespace));
	}

	bool value_container::deserialize(const std::wstring& data_string, const bool& parse_only_header)
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

	bool value_container::deserialize(const std::vector<unsigned char>& data_array, const bool& parse_only_header)
	{
		return deserialize(converter::to_wstring(data_array), parse_only_header);
	}

	std::wstring value_container::datas(void) const
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

	void value_container::load_packet(const std::wstring& file_path)
	{
		deserialize(converter::to_wstring(file_handler::load(file_path)));
	}

	void value_container::save_packet(const std::wstring& file_path, const bool& contain_whitespace)
	{
		file_handler::save(file_path, converter::to_array(serialize(contain_whitespace)));
	}

	std::vector<std::shared_ptr<value>> value_container::operator[](const std::wstring& key)
	{
		return value_array(key);
	}

	value_container operator<<(value_container target_container, value& other)
	{
		target_container.add(other);

		return target_container;
	}

	value_container operator<<(value_container target_container, std::shared_ptr<value> other)
	{
		target_container.add(other);

		return target_container;
	}

	std::shared_ptr<value_container> operator<<(std::shared_ptr<value_container> target_container, value& other)
	{
		target_container->add(other);

		return target_container;
	}

	std::shared_ptr<value_container> operator<<(std::shared_ptr<value_container> target_container, std::shared_ptr<value> other)
	{
		target_container->add(other);

		return target_container;
	}

	std::ostream& operator <<(std::ostream& out, value_container& other) // output
	{
		out << converter::to_string(other.serialize(false));

		return out;
	}

	std::ostream& operator <<(std::ostream& out, std::shared_ptr<value_container> other) // output
	{
		out << converter::to_string(other->serialize(false));

		return out;
	}

	std::wostream& operator <<(std::wostream& out, value_container& other) // output
	{
		out << other.serialize(false);

		return out;
	}

	std::wostream& operator <<(std::wostream& out, std::shared_ptr<value_container> other) // output
	{
		out << other->serialize(false);

		return out;
	}

	std::string& operator <<(std::string& out, value_container& other)
	{
		out = converter::to_string(other.serialize(false));

		return out;
	}

	std::string& operator <<(std::string& out, std::shared_ptr<value_container> other)
	{
		out = converter::to_string(other->serialize(false));

		return out;
	}

	std::wstring& operator <<(std::wstring& out, value_container& other)
	{
		out = other.serialize(false);

		return out;
	}

	std::wstring& operator <<(std::wstring& out, std::shared_ptr<value_container> other)
	{
		out = other->serialize(false);

		return out;
	}

	bool value_container::deserialize_values(const std::wstring& data)
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

		std::vector<std::shared_ptr<value>> temp_list;
		while (start != end)
		{
			temp_list.push_back(value::generate_value((*start)[1], (*start)[2], (*start)[3]));

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

	void value_container::parsing(const std::wstring& source_name, const std::wstring& target_name, const std::wstring& target_value, std::wstring& target_variable)
	{
		if (source_name != target_name)
		{
			return;
		}

		target_variable = target_value;

		target_variable.substr(0, target_variable.find_first_not_of(' '));
		target_variable.erase(target_variable.find_last_not_of(' ') + 1);

		return;
	}
}