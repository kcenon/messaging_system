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

#include "converting.h"
#include "file_handler.h"
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

#include "fmt/format.h"
#include "fmt/xchar.h"

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
      : _source_id(L"")
      , _source_sub_id(L"")
      , _target_id(L"")
      , _target_sub_id(L"")
      , _message_type(L"data_container")
      , _version(L"1.0.0.0")
      , _parsed_data(true)
      , _data_string(L"@data={};")
      , _changed_data(false)
  {
    _data_type_map.insert(
        { value_types::bool_value, bind(&value_container::set_boolean, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::short_value, bind(&value_container::set_short, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::ushort_value, bind(&value_container::set_ushort, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::int_value, bind(&value_container::set_int, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::uint_value, bind(&value_container::set_uint, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::long_value, bind(&value_container::set_long, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::ulong_value, bind(&value_container::set_ulong, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::llong_value, bind(&value_container::set_llong, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::ullong_value, bind(&value_container::set_ullong, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::float_value, bind(&value_container::set_float, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::double_value, bind(&value_container::set_double, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::bytes_value, bind(&value_container::set_bytes, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::string_value, bind(&value_container::set_string, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert({ value_types::container_value,
                            bind(&value_container::set_container, this, placeholders::_1, placeholders::_2) });
  }

  value_container::value_container(const wstring &data_string, const bool &parse_only_header) : value_container()
  {
    deserialize(data_string, parse_only_header);
  }

  value_container::value_container(const vector<uint8_t> &data_array, const bool &parse_only_header) : value_container()
  {
    deserialize(data_array, parse_only_header);
  }

  value_container::value_container(const value_container &data_container, const bool &parse_only_header)
      : value_container()
  {
    deserialize(data_container.serialize(), parse_only_header);
  }

  value_container::value_container(shared_ptr<value_container> data_container, const bool &parse_only_header)
      : value_container()
  {
    if (data_container == nullptr)
    {
      return;
    }

    deserialize(data_container->serialize(), parse_only_header);
  }

  value_container::value_container(const wstring &message_type, const vector<shared_ptr<value> > &units)
      : value_container()
  {
    set_message_type(message_type);
    set_units(units);
  }

  value_container::value_container(const wstring &target_id,
                                   const wstring &target_sub_id,
                                   const wstring &message_type,
                                   const vector<shared_ptr<value> > &units)
      : value_container()
  {
    set_target(target_id, target_sub_id);
    set_message_type(message_type);
    set_units(units);
  }

  value_container::value_container(const wstring &source_id,
                                   const wstring &source_sub_id,
                                   const wstring &target_id,
                                   const wstring &target_sub_id,
                                   const wstring &message_type,
                                   const vector<shared_ptr<value> > &units)
      : value_container()
  {
    set_source(source_id, source_sub_id);
    set_target(target_id, target_sub_id);
    set_message_type(message_type);
    set_units(units);
  }

  value_container::~value_container(void) {}

  shared_ptr<value_container> value_container::get_ptr(void) { return shared_from_this(); }

  void value_container::set_source(const wstring &source_id, const wstring &source_sub_id)
  {
    _source_id = source_id;
    _source_sub_id = source_sub_id;
  }

  void value_container::set_target(const wstring &target_id, const wstring &target_sub_id)
  {
    _target_id = target_id;
    _target_sub_id = target_sub_id;
  }

  void value_container::set_message_type(const wstring &message_type) { _message_type = message_type; }

  wstring value_container::source_id(void) const { return _source_id; }

  wstring value_container::source_sub_id(void) const { return _source_sub_id; }

  wstring value_container::target_id(void) const { return _target_id; }

  wstring value_container::target_sub_id(void) const { return _target_sub_id; }

  wstring value_container::message_type(void) const { return _message_type; }

  void value_container::set_units(const vector<shared_ptr<value> > &target_values, const bool &update_immediately)
  {
    if (!_parsed_data)
    {
      deserialize_values(_data_string, false);
    }

    vector<shared_ptr<value> >::iterator target;
    for (auto &target_value : target_values)
    {
      target = find_if(_units.begin(), _units.end(),
                       [&target_value](shared_ptr<value> item) { return item == target_value; });

      if (target != _units.end())
      {
        continue;
      }

      _units.push_back(target_value);
      target_value->set_parent(nullptr);
    }

    _changed_data = !update_immediately;
    if (update_immediately)
    {
      _data_string = datas();
    }
  }

  void value_container::swap_header(void)
  {
    wstring temp = _source_id;
    _source_id = _target_id;
    _target_id = std::move(temp);

    temp = _source_sub_id;
    _source_sub_id = _target_sub_id;
    _target_sub_id = std::move(temp);
  }

  void value_container::clear_value(void)
  {
    _parsed_data = true;
    _changed_data = false;
    _data_string = L"@data={};";
    _units.clear();
  }

  shared_ptr<value_container> value_container::copy(const bool &containing_values)
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

  shared_ptr<value> value_container::add(const value &target_value, const bool &update_immediately)
  {
    auto target = _data_type_map.find(target_value.type());
    if (target == _data_type_map.end())
    {
      return add(make_shared<value>(target_value.name(), nullptr, 0, value_types::null_value), update_immediately);
    }

    return add(target->second(target_value.name(), target_value.to_string()), update_immediately);
  }

  shared_ptr<value> value_container::add(shared_ptr<value> target_value, const bool &update_immediately)
  {
    if (!_parsed_data)
    {
      deserialize_values(_data_string, false);
    }

    vector<shared_ptr<value> >::iterator target;
    target = find_if(_units.begin(), _units.end(),
                     [&target_value](shared_ptr<value> item) { return item == target_value; });

    if (target != _units.end())
    {
      return nullptr;
    }

    _units.push_back(target_value);
    target_value->set_parent(nullptr);

    _changed_data = !update_immediately;
    if (update_immediately)
    {
      _data_string = datas();
    }

    return target_value;
  }

  void value_container::remove(const wstring &target_name, const bool &update_immediately)
  {
    if (!_parsed_data)
    {
      deserialize_values(_data_string, false);
    }

    vector<shared_ptr<value> >::iterator target;

    while (true)
    {
      target = find_if(_units.begin(), _units.end(),
                       [&target_name](shared_ptr<value> item) { return item->name() == target_name; });

      if (target == _units.end())
      {
        break;
      }

      _units.erase(target);
    }

    _changed_data = !update_immediately;
    if (update_immediately)
    {
      _data_string = datas();
    }
  }

  void value_container::remove(shared_ptr<value> target_value, const bool &update_immediately)
  {
    if (!_parsed_data)
    {
      deserialize_values(_data_string, false);
    }

    vector<shared_ptr<value> >::iterator target;
    target = find_if(_units.begin(), _units.end(),
                     [&target_value](shared_ptr<value> item) { return item == target_value; });

    if (target == _units.end())
    {
      return;
    }

    _units.erase(target);

    _changed_data = !update_immediately;
    if (update_immediately)
    {
      _data_string = datas();
    }
  }

  vector<shared_ptr<value> > value_container::value_array(const wstring &target_name)
  {
    if (!_parsed_data)
    {
      deserialize_values(_data_string, false);
    }

    vector<shared_ptr<value> > result_list;

    for_each(_units.begin(), _units.end(),
             [&target_name, &result_list](shared_ptr<value> source)
             {
               if (source->name() == target_name)
               {
                 result_list.push_back(source);
               }
             });

    return result_list;
  }

  shared_ptr<value> value_container::get_value(const wstring &target_name, const unsigned int &index)
  {
    if (!_parsed_data)
    {
      deserialize_values(_data_string, false);
    }

    vector<shared_ptr<value> > result_list = value_array(target_name);
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
    wstring data_string = _data_string;
    if (_parsed_data)
    {
      data_string = datas();
    }

    wstring result;

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
    fmt::format_to(back_inserter(result), L"{}", data_string);

    return result;
  }

  vector<uint8_t> value_container::serialize_array(void) const { return converter::to_array(serialize()); }

  bool value_container::deserialize(const wstring &data_string, const bool &parse_only_header)
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

  bool value_container::deserialize(const vector<uint8_t> &data_array, const bool &parse_only_header)
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
    for (auto &unit : _units)
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
    fmt::format_to(back_inserter(result), L"{}", L"\"header\":{");
    if (_message_type != L"data_container")
    {
      fmt::format_to(back_inserter(result), L"\"target_id\":\"{}\",", _target_id);
      fmt::format_to(back_inserter(result), L"\"target_sub_id\":\"{}\",", _target_sub_id);
      fmt::format_to(back_inserter(result), L"\"source_id\":\"{}\",", _source_id);
      fmt::format_to(back_inserter(result), L"\"source_sub_id\":\"{}\",", _source_sub_id);
    }
    fmt::format_to(back_inserter(result), L"\"message_type\":\"{}\"", _message_type);
    fmt::format_to(back_inserter(result), L",\"version\":\"{}\"", _version);
    fmt::format_to(back_inserter(result), L"{}", L"}");

    fmt::format_to(back_inserter(result), L",{}", L"\"values\":{");

    bool first = true;
    for (auto &unit : _units)
    {
      fmt::format_to(back_inserter(result), first ? L"{}" : L",{}", unit->to_json());
      first = false;
    }

    fmt::format_to(back_inserter(result), L"{}", L"}");
    fmt::format_to(back_inserter(result), L"{}", L"}");

    return result;
    ;
  }

  wstring value_container::datas(void) const
  {
    if (!_parsed_data)
    {
      return _data_string;
    }

    wstring result;

    // data
    fmt::format_to(back_inserter(result), L"@data={}", L"{");
    for (auto &unit : _units)
    {
      fmt::format_to(back_inserter(result), L"{}", unit->serialize());
    }
    fmt::format_to(back_inserter(result), L"{}", L"};");

    return result;
  }

  void value_container::load_packet(const wstring &file_path)
  {
    deserialize(converter::to_wstring(file::load(file_path)));
  }

  void value_container::save_packet(const wstring &file_path)
  {
    file::save(file_path, converter::to_array(serialize()));
  }

  vector<shared_ptr<value> > value_container::operator[](const wstring &key) { return value_array(key); }

  value_container operator<<(value_container target_container, value &other)
  {
    target_container.add(other);

    return target_container;
  }

  value_container operator<<(value_container target_container, shared_ptr<value> other)
  {
    target_container.add(other);

    return target_container;
  }

  shared_ptr<value_container> operator<<(shared_ptr<value_container> target_container, value &other)
  {
    target_container->add(other);

    return target_container;
  }

  shared_ptr<value_container> operator<<(shared_ptr<value_container> target_container, shared_ptr<value> other)
  {
    target_container->add(other);

    return target_container;
  }

  ostream &operator<<(ostream &out, value_container &other) // output
  {
    out << converter::to_string(other.serialize());

    return out;
  }

  ostream &operator<<(ostream &out, shared_ptr<value_container> other) // output
  {
    out << converter::to_string(other->serialize());

    return out;
  }

  wostream &operator<<(wostream &out, value_container &other) // output
  {
    out << other.serialize();

    return out;
  }

  wostream &operator<<(wostream &out, shared_ptr<value_container> other) // output
  {
    out << other->serialize();

    return out;
  }

  string &operator<<(string &out, value_container &other)
  {
    out = converter::to_string(other.serialize());

    return out;
  }

  string &operator<<(string &out, shared_ptr<value_container> other)
  {
    out = converter::to_string(other->serialize());

    return out;
  }

  wstring &operator<<(wstring &out, value_container &other)
  {
    out = other.serialize();

    return out;
  }

  wstring &operator<<(wstring &out, shared_ptr<value_container> other)
  {
    out = other->serialize();

    return out;
  }

  bool value_container::deserialize_values(const wstring &data, const bool &parse_only_header)
  {
    if (_units.size() > 0)
    {
      _units.clear();
    }

    _changed_data = false;

    wregex full_condition(L"@data=[\\s?]*\\{[\\s?]*(.*?)[\\s?]*\\};");
    wsregex_iterator full_iter(data.begin(), data.end(), full_condition);
    wsregex_iterator full_end;
    if (full_iter == full_end)
    {
      _data_string = L"@data={};";
      _parsed_data = true;

      return false;
    }

    _data_string = (*full_iter)[0].str();

    if (parse_only_header)
    {
      _parsed_data = false;

      return true;
    }

    _parsed_data = true;

    wregex regex_condition(L"\\[(\\w+),[\\s?]*(\\w+),[\\s?]*(.*?)\\];");
    wsregex_iterator start(_data_string.begin(), _data_string.end(), regex_condition);
    wsregex_iterator end;

    vector<shared_ptr<value> > temp_list;
    while (start != end)
    {
      auto target = _data_type_map.find(convert_value_type((*start)[2]));
      if (target == _data_type_map.end())
      {
        temp_list.push_back(make_shared<value>((*start)[1], nullptr, 0, value_types::null_value));
        continue;
      }

      temp_list.push_back(target->second((*start)[1], (*start)[3]));

      start++;
    }

    shared_ptr<value> container = nullptr;
    vector<shared_ptr<value> >::iterator iterator;
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

  void value_container::parsing(const wstring &source_name,
                                const wstring &target_name,
                                const wstring &target_value,
                                wstring &target_variable)
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

  shared_ptr<value> value_container::set_boolean(const wstring &name, const wstring &data)
  {
    return make_shared<bool_value>(name, data);
  }

  shared_ptr<value> value_container::set_short(const wstring &name, const wstring &data)
  {
    return make_shared<short_value>(name, (short)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_ushort(const wstring &name, const wstring &data)
  {
    return make_shared<ushort_value>(name, (unsigned short)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_int(const wstring &name, const wstring &data)
  {
    return make_shared<int_value>(name, (int)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_uint(const wstring &name, const wstring &data)
  {
    return make_shared<uint_value>(name, (unsigned int)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_long(const wstring &name, const wstring &data)
  {
    return make_shared<long_value>(name, (long)atol(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_ulong(const wstring &name, const wstring &data)
  {
    return make_shared<ulong_value>(name, (unsigned long)atol(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_llong(const wstring &name, const wstring &data)
  {
    return make_shared<llong_value>(name, (long long)atoll(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_ullong(const wstring &name, const wstring &data)
  {
    return make_shared<ullong_value>(name, (unsigned long long)atoll(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_float(const wstring &name, const wstring &data)
  {
    return make_shared<float_value>(name, (float)atof(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_double(const wstring &name, const wstring &data)
  {
    return make_shared<double_value>(name, (double)atof(converter::to_string(data).c_str()));
  }

  shared_ptr<value> value_container::set_bytes(const wstring &name, const wstring &data)
  {
    return make_shared<bytes_value>(name, converter::from_base64(data.c_str()));
  }

  shared_ptr<value> value_container::set_string(const wstring &name, const wstring &data)
  {
    return make_shared<string_value>(name, data);
  }

  shared_ptr<value> value_container::set_container(const wstring &name, const wstring &data)
  {
    return make_shared<container_value>(name, (long)atol(converter::to_string(data).c_str()));
  }
} // namespace container