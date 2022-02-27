#include "container.h"

#ifdef __USE_TYPE_CONTAINER__

#include "value_types.h"
#include "converting.h"
#include "file_handler.h"

#include "values/container_value.h"

#include <io.h>
#include <fcntl.h>
#include <wchar.h>

#include <regex>
#include <sstream>

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	constexpr auto TARGET_ID = L"1";
	constexpr auto TARGET_SUB_ID = L"2";
	constexpr auto SOURCE_ID = L"3";
	constexpr auto SOURCE_SUB_ID = L"4";
	constexpr auto MESSAGE_TYPE = L"5";
	constexpr auto MESSAGE_VERSION = L"6";

	using namespace converting;
	using namespace file_handler;

	value_container::value_container(void)
		: _source_id(L""), _source_sub_id(L""), _target_id(L""), _target_sub_id(L""), _message_type(L"data_container"), _version(L"1.0.0.0"),
		_parsed_data(true), _data_string(L"")
	{
	}

	value_container::value_container(const wstring& data_string, const bool& parse_only_header) : value_container()
	{
		deserialize(data_string, parse_only_header);
	}

	value_container::value_container(const vector<unsigned char>& data_array, const bool& parse_only_header) : value_container()
	{
		deserialize(data_array, parse_only_header);
	}

	value_container::value_container(const value_container& data_container, const bool& parse_only_header) : value_container()
	{
		deserialize(data_container.serialize(), parse_only_header);
	}

	value_container::value_container(shared_ptr<value_container> data_container, const bool& parse_only_header) : value_container()
	{
		if (data_container == nullptr)
		{
			return;
		}

		deserialize(data_container->serialize(), parse_only_header);
	}

	value_container::value_container(const wstring& message_type,
		const vector<shared_ptr<value>>& units) : value_container()
	{
		set_message_type(message_type);
		set_units(units);
	}

	value_container::value_container(const wstring& target_id, const wstring& target_sub_id, const wstring& message_type,
		const vector<shared_ptr<value>>& units) : value_container()
	{
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	value_container::value_container(const wstring& source_id, const wstring& source_sub_id,
		const wstring& target_id, const wstring& target_sub_id, const wstring& message_type,
		const vector<shared_ptr<value>>& units) : value_container()
	{
		set_source(source_id, source_sub_id);
		set_target(target_id, target_sub_id);
		set_message_type(message_type);
		set_units(units);
	}

	value_container::~value_container(void)
	{
	}

	shared_ptr<value_container> value_container::get_ptr(void)
	{
		return shared_from_this();
	}

	void value_container::set_source(const wstring& source_id, const wstring& source_sub_id)
	{
		_source_id = source_id;
		_source_sub_id = source_sub_id;
	}

	void value_container::set_target(const wstring& target_id, const wstring& target_sub_id)
	{
		_target_id = target_id;
		_target_sub_id = target_sub_id;
	}

	void value_container::set_message_type(const wstring& message_type)
	{
		_message_type = message_type;
	}

	wstring value_container::source_id(void) const
	{
		return _source_id;
	}

	wstring value_container::source_sub_id(void) const
	{
		return _source_sub_id;
	}

	wstring value_container::target_id(void) const
	{
		return _target_id;
	}

	wstring value_container::target_sub_id(void) const
	{
		return _target_sub_id;
	}

	wstring value_container::message_type(void) const
	{
		return _message_type;
	}

	void value_container::set_units(const vector<shared_ptr<value>>& target_values)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		vector<shared_ptr<value>>::iterator target;
		for (auto& target_value : target_values)
		{
			target = find_if(_units.begin(), _units.end(),
				[&target_value](shared_ptr<value> item)
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
		wstring temp = _source_id;
		_source_id = _target_id;
		_target_id = move(temp);

		temp = _source_sub_id;
		_source_sub_id = _target_sub_id;
		_target_sub_id = move(temp);
	}

	void value_container::clear_value(void)
	{
		_parsed_data = true;
		_data_string = L"";
		_units.clear();
	}

	shared_ptr<value_container> value_container::copy(const bool& containing_values)
	{
		shared_ptr<value_container> new_container = make_shared<value_container>(serialize(), !containing_values);
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

	shared_ptr<value> value_container::add(const value& target_value)
	{
		return add(value::generate_value(target_value.name(), convert_value_type(target_value.type()), target_value.to_string()));
	}

	shared_ptr<value> value_container::add(shared_ptr<value> target_value)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		vector<shared_ptr<value>>::iterator target;
		target = find_if(_units.begin(), _units.end(),
			[&target_value](shared_ptr<value> item)
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

	void value_container::remove(const wstring& target_name)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		vector<shared_ptr<value>>::iterator target;

		while (true) {
			target = find_if(_units.begin(), _units.end(),
				[&target_name](shared_ptr<value> item)
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

	void value_container::remove(shared_ptr<value> target_value)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		vector<shared_ptr<value>>::iterator target;
		target = find_if(_units.begin(), _units.end(),
			[&target_value](shared_ptr<value> item)
			{
				return item == target_value;
			});

		if (target == _units.end())
		{
			return;
		}

		_units.erase(target);
	}

	vector<shared_ptr<value>> value_container::value_array(const wstring& target_name)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		vector<shared_ptr<value>> result_list;

		for_each(_units.begin(), _units.end(), [&target_name, &result_list](shared_ptr<value> source) {
			if (source->name() == target_name)
			{
				result_list.push_back(source);
			}
			});

		return result_list;
	}

	shared_ptr<value> value_container::get_value(const wstring& target_name, const unsigned int& index)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		vector<shared_ptr<value>> result_list = value_array(target_name);
		if (result_list.empty())
		{
			return make_shared<value>(target_name);
		}

		if (index >= result_list.size())
		{
			return make_shared<value>(target_name);
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

		clear_value();
	}

	wstring value_container::serialize(void) const
	{
		fmt::wmemory_buffer result;
		result.clear();

		// header
		fmt::format_to(back_inserter(result), L"@header={}", L"{");
		if (_message_type != L"data_container")
		{
			fmt::format_to(back_inserter(result), L"[{},{}];", TARGET_ID, _target_id);
			fmt::format_to(back_inserter(result), L"[{},{}];", TARGET_SUB_ID, _target_sub_id);
			fmt::format_to(back_inserter(result), L"[{},{}];", SOURCE_ID, _source_id);
			fmt::format_to(back_inserter(result), L"[{},{}];", SOURCE_SUB_ID, _source_sub_id);
		}
		fmt::format_to(back_inserter(result), L"[{},{}];", MESSAGE_TYPE, _message_type);
		fmt::format_to(back_inserter(result), L"[{},{}];", MESSAGE_VERSION, _version);
		fmt::format_to(back_inserter(result), L"{}", L"};");

		if (!_parsed_data)
		{
			fmt::format_to(back_inserter(result), L"{}", _data_string);

			return result.data();
		}

		fmt::format_to(back_inserter(result), L"{}", datas());

		return result.data();
	}

	vector<unsigned char> value_container::serialize_array(void) const
	{
		return converter::to_array(serialize());
	}

	bool value_container::deserialize(const wstring& data_string, const bool& parse_only_header)
	{
		initialize();

		if (data_string.empty())
		{
			return false;
		}

		wregex newlines_re(L"\\r\\n?|\\n");
		wstring removed_newline = regex_replace(data_string, newlines_re, L"");

		wregex full_condition(L"@header=[\\s?]*\\{[\\s?]*(.*?)[\\s?]*\\};");
		wsregex_iterator full_iter(removed_newline.begin(), removed_newline.end(), full_condition);
		wsregex_iterator full_end;
		if (full_iter == full_end)
		{
			return deserialize_values(removed_newline, parse_only_header);
		}

		wstring temp = (*full_iter)[1];
		wregex header_condition(L"\\[(\\w+),(.*?)\\];");
		wsregex_iterator header_iter(temp.begin(), temp.end(), header_condition);
		wsregex_iterator header_end;
		while (header_iter != header_end)
		{
			parsing((*header_iter)[1], TARGET_ID, (*header_iter)[2], _target_id);
			parsing((*header_iter)[1], TARGET_SUB_ID, (*header_iter)[2], _target_sub_id);
			parsing((*header_iter)[1], SOURCE_ID, (*header_iter)[2], _source_id);
			parsing((*header_iter)[1], SOURCE_SUB_ID, (*header_iter)[2], _source_sub_id);
			parsing((*header_iter)[1], MESSAGE_TYPE, (*header_iter)[2], _message_type);
			parsing((*header_iter)[1], MESSAGE_VERSION, (*header_iter)[2], _version);

			header_iter++;
		}

		return deserialize_values(removed_newline, parse_only_header);
	}

	bool value_container::deserialize(const vector<unsigned char>& data_array, const bool& parse_only_header)
	{
		return deserialize(converter::to_wstring(data_array), parse_only_header);
	}

	const wstring value_container::to_xml(void)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		wstring result;

		fmt::format_to(back_inserter(result), L"{}", L"<container>");
		fmt::format_to(back_inserter(result), L"{}", L"<header>");
		if (_message_type != L"data_container")
		{
			fmt::format_to(back_inserter(result), L"<target_id>{}</target_id>", _target_id);
			fmt::format_to(back_inserter(result), L"<target_sub_id>{}</target_sub_id>", _target_sub_id);
			fmt::format_to(back_inserter(result), L"<source_id>{}</source_id>", _source_id);
			fmt::format_to(back_inserter(result), L"<source_sub_id>{}</source_sub_id>", _source_sub_id);
		}
		fmt::format_to(back_inserter(result), L"<message_type>{}</message_type>", _message_type);
		fmt::format_to(back_inserter(result), L"<version>{}</version>", _version);
		fmt::format_to(back_inserter(result), L"{}", L"</header>");

		fmt::format_to(back_inserter(result), L"{}", L"<values>");
		for (auto& unit : _units)
		{
			fmt::format_to(back_inserter(result), L"{}", unit->to_xml());
		}
		fmt::format_to(back_inserter(result), L"{}", L"</values>");
		fmt::format_to(back_inserter(result), L"{}", L"</container>");

		return result;
	}

	const wstring value_container::to_json(void)
	{
		if (!_parsed_data)
		{
			deserialize_values(_data_string, false);
		}

		wstring result;

		fmt::format_to(back_inserter(result), L"{}", L"{");
		fmt::format_to(back_inserter(result), L"{}", L"\"header\":[");
		if (_message_type != L"data_container")
		{
			fmt::format_to(back_inserter(result), L"{}\"target_id\":\"{}\"{},", L"{", _target_id, L"}");
			fmt::format_to(back_inserter(result), L"{}\"target_sub_id\":\"{}\"{},", L"{", _target_sub_id, L"}");
			fmt::format_to(back_inserter(result), L"{}\"source_id\":\"{}\"{},", L"{", _source_id, L"}");
			fmt::format_to(back_inserter(result), L"{}\"source_sub_id\":\"{}\"{},", L"{", _source_sub_id, L"}");
		}
		fmt::format_to(back_inserter(result), L"{}\"message_type\":\"{}\"{}", L"{", _message_type, L"}");
		fmt::format_to(back_inserter(result), L",{}\"version\":\"{}\"{}", L"{", _version, L"}");
		fmt::format_to(back_inserter(result), L"{}", L"]");

		fmt::format_to(back_inserter(result), L",{}", L"\"values\":[");

		bool first = true;
		for (auto& unit : _units)
		{
			fmt::format_to(back_inserter(result), first ? L"{}" : L",{}", unit->to_json());
			first = false;
		}
		fmt::format_to(back_inserter(result), L"{}", L"]");
		fmt::format_to(back_inserter(result), L"{}", L"}");

		return result;;
	}

	wstring value_container::datas(void) const
	{
		if (!_parsed_data)
		{
			return _data_string;
		}

		fmt::wmemory_buffer result;
		result.clear();

		// data
		fmt::format_to(back_inserter(result), L"@data={}", L"{");
		for (auto& unit : _units)
		{
			fmt::format_to(back_inserter(result), L"{}", unit->serialize());
		}
		fmt::format_to(back_inserter(result), L"{}", L"};");

		return result.data();
	}

	void value_container::load_packet(const wstring& file_path)
	{
		deserialize(converter::to_wstring(file::load(file_path)));
	}

	void value_container::save_packet(const wstring& file_path)
	{
		file::save(file_path, converter::to_array(serialize()));
	}

	vector<shared_ptr<value>> value_container::operator[](const wstring& key)
	{
		return value_array(key);
	}

	value_container operator<<(value_container target_container, value& other)
	{
		target_container.add(other);

		return target_container;
	}

	value_container operator<<(value_container target_container, shared_ptr<value> other)
	{
		target_container.add(other);

		return target_container;
	}

	shared_ptr<value_container> operator<<(shared_ptr<value_container> target_container, value& other)
	{
		target_container->add(other);

		return target_container;
	}

	shared_ptr<value_container> operator<<(shared_ptr<value_container> target_container, shared_ptr<value> other)
	{
		target_container->add(other);

		return target_container;
	}

	ostream& operator <<(ostream& out, value_container& other) // output
	{
		out << converter::to_string(other.serialize());

		return out;
	}

	ostream& operator <<(ostream& out, shared_ptr<value_container> other) // output
	{
		out << converter::to_string(other->serialize());

		return out;
	}

	wostream& operator <<(wostream& out, value_container& other) // output
	{
		out << other.serialize();

		return out;
	}

	wostream& operator <<(wostream& out, shared_ptr<value_container> other) // output
	{
		out << other->serialize();

		return out;
	}

	string& operator <<(string& out, value_container& other)
	{
		out = converter::to_string(other.serialize());

		return out;
	}

	string& operator <<(string& out, shared_ptr<value_container> other)
	{
		out = converter::to_string(other->serialize());

		return out;
	}

	wstring& operator <<(wstring& out, value_container& other)
	{
		out = other.serialize();

		return out;
	}

	wstring& operator <<(wstring& out, shared_ptr<value_container> other)
	{
		out = other->serialize();

		return out;
	}

	bool value_container::deserialize_values(const wstring& data, const bool& parse_only_header)
	{
		if (_units.size() > 0)
		{
			_units.clear();
		}

		wregex full_condition(L"@data=[\\s?]*\\{[\\s?]*(.*?)[\\s?]*\\};");
		wsregex_iterator full_iter(data.begin(), data.end(), full_condition);
		wsregex_iterator full_end;
		if (full_iter == full_end)
		{
			_data_string = L"";
			_parsed_data = true;

			return false;
		}

		wstring regex_temp = (*full_iter)[0].str();
		
		if (parse_only_header)
		{
			_data_string = regex_temp;
			_parsed_data = false;

			return true;
		}

		_data_string = L"";
		_parsed_data = true;

		wregex regex_condition(L"\\[(\\w+),[\\s?]*(\\w+),[\\s?]*(.*?)\\];");
		wsregex_iterator start(regex_temp.begin(), regex_temp.end(), regex_condition);
		wsregex_iterator end;

		vector<shared_ptr<value>> temp_list;
		while (start != end)
		{
			temp_list.push_back(value::generate_value((*start)[1], (*start)[2], (*start)[3]));

			start++;
		}

		shared_ptr<value> container = nullptr;
		vector<shared_ptr<value>>::iterator iterator;
		for (iterator = temp_list.begin(); iterator != temp_list.end(); ++iterator)
		{
			if (container == nullptr)
			{
				add(*iterator);

				if ((*iterator)->is_container() != true || (*iterator)->to_long() == 0)
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

			while (container != nullptr && container->to_long() == container->child_count())
			{
				container = container->parent();
			}
		}

		return true;
	}

	void value_container::parsing(const wstring& source_name, const wstring& target_name, const wstring& target_value, wstring& target_variable)
	{
		if (source_name != target_name)
		{
			return;
		}

		target_variable = target_value;

		if (target_value.find_first_not_of(' ') != string::npos)
		{
			target_variable = target_variable.erase(target_value.find_last_not_of(' ') + 1);
		}

		return;
	}
}

#endif