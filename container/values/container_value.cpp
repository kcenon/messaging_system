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

#include "container_value.h"

#include "converting.h"

#include "bool_value.h"
#include "bytes_value.h"
#include "double_value.h"
#include "float_value.h"
#include "int_value.h"
#include "llong_value.h"
#include "long_value.h"
#include "short_value.h"
#include "string_value.h"
#include "uint_value.h"
#include "ullong_value.h"
#include "ulong_value.h"
#include "ushort_value.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <algorithm>

namespace container
{
  using namespace converting;

  container_value::container_value(void) : value()
  {
    _type = value_types::container_value;

    _data_type_map.insert(
        { value_types::bool_value, bind(&container_value::set_boolean, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::short_value, bind(&container_value::set_short, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::ushort_value, bind(&container_value::set_ushort, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::int_value, bind(&container_value::set_int, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::uint_value, bind(&container_value::set_uint, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::long_value, bind(&container_value::set_long, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::ulong_value, bind(&container_value::set_ulong, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::llong_value, bind(&container_value::set_llong, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::ullong_value, bind(&container_value::set_ullong, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::float_value, bind(&container_value::set_float, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::double_value, bind(&container_value::set_double, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert({ value_types::bytes_value,
                            bind(&container_value::set_byte_string, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::string_value, bind(&container_value::set_string, this, placeholders::_1, placeholders::_2) });
    _data_type_map.insert(
        { value_types::container_value, bind(&container_value::set_long, this, placeholders::_1, placeholders::_2) });
  }

  container_value::container_value(const wstring &name, const long &reserved_count) : container_value()
  {
    _name = name;
    _type = value_types::container_value;
    _size = sizeof(long);

    char *data_ptr = (char *)&reserved_count;
    _data = vector<uint8_t>(data_ptr, data_ptr + _size);
  }

  container_value::container_value(const wstring &name, const vector<shared_ptr<value> > &units) : value(name, units) {}

  container_value::~container_value(void)
  {
    _data.clear();
    _units.clear();
  }

  shared_ptr<value> container_value::add(const value &item, const bool &update_count)
  {
    auto target = _data_type_map.find(item.type());
    if (target == _data_type_map.end())
    {
      return add(make_shared<value>(item.name(), nullptr, 0, value_types::null_value));
    }

    return add(target->second(item.name(), item.to_string()));
  }

  shared_ptr<value> container_value::add(shared_ptr<value> item, const bool &update_count)
  {
    vector<shared_ptr<value> >::iterator target;
    target = find_if(_units.begin(), _units.end(), [&item](shared_ptr<value> current) { return current == item; });

    if (target != _units.end())
    {
      return nullptr;
    }

    _units.push_back(item);
    item->set_parent(get_ptr());

    if (update_count == true)
    {
      long size = static_cast<long>(_units.size());
      set_data((const unsigned char *)&size, sizeof(long), value_types::container_value);
    }

    return item;
  }

  void container_value::add(const vector<value> &target_values, const bool &update_count)
  {
    vector<shared_ptr<value> > temp_values;
    for (auto &target_value : target_values)
    {
      auto target = _data_type_map.find(target_value.type());
      if (target == _data_type_map.end())
      {
        temp_values.push_back(make_shared<value>(target_value.name(), nullptr, 0, value_types::null_value));
        continue;
      }

      temp_values.push_back(target->second(target_value.name(), target_value.to_string()));
    }

    add(temp_values, update_count);
  }

  void container_value::add(const vector<shared_ptr<value> > &target_values, const bool &update_count)
  {
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
      target_value->set_parent(get_ptr());
    }

    if (update_count == true)
    {
      long size = static_cast<long>(_units.size());
      set_data((const unsigned char *)&size, sizeof(long), value_types::container_value);
    }
  }

  void container_value::remove(const wstring &target_name, const bool &update_count)
  {
    vector<shared_ptr<value> >::iterator target;

    while (true)
    {
      target = find_if(_units.begin(), _units.end(),
                       [&target_name](shared_ptr<value> current) { return current->name() == target_name; });

      if (target == _units.end())
      {
        break;
      }

      _units.erase(target);
    }

    if (update_count == true)
    {
      long size = static_cast<long>(_units.size());
      set_data((const unsigned char *)&size, sizeof(long), value_types::container_value);
    }
  }

  void container_value::remove(shared_ptr<value> item, const bool &update_count)
  {
    vector<shared_ptr<value> >::iterator target;
    target = find_if(_units.begin(), _units.end(), [&item](shared_ptr<value> current) { return current == item; });

    if (target == _units.end())
    {
      return;
    }

    _units.erase(target);

    if (update_count == true)
    {
      long size = static_cast<long>(_units.size());
      set_data((const unsigned char *)&size, sizeof(long), value_types::container_value);
    }
  }

  void container_value::remove_all(void)
  {
    _units.clear();

    long size = static_cast<long>(_units.size());
    set_data((const unsigned char *)&size, sizeof(long), value_types::container_value);
  }

  long container_value::to_long(void) const
  {
    long temp = 0;
    memcpy(&temp, _data.data(), _size);

    return static_cast<long>(temp);
  }

  wstring container_value::to_string(const bool &) const { return fmt::format(L"{}", to_long()); }

  shared_ptr<value> container_value::set_boolean(const wstring &name, const wstring &data)
  {
    return make_shared<bool_value>(name, data);
  }

  shared_ptr<value> container_value::set_short(const wstring &name, const wstring &data)
  {
    return make_shared<short_value>(name, (short)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_ushort(const wstring &name, const wstring &data)
  {
    return make_shared<ushort_value>(name, (unsigned short)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_int(const wstring &name, const wstring &data)
  {
    return make_shared<int_value>(name, (int)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_uint(const wstring &name, const wstring &data)
  {
    return make_shared<uint_value>(name, (unsigned int)atoi(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_long(const wstring &name, const wstring &data)
  {
    return make_shared<long_value>(name, (long)atol(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_ulong(const wstring &name, const wstring &data)
  {
    return make_shared<ulong_value>(name, (unsigned long)atol(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_llong(const wstring &name, const wstring &data)
  {
    return make_shared<llong_value>(name, (long long)atoll(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_ullong(const wstring &name, const wstring &data)
  {
    return make_shared<ullong_value>(name, (unsigned long long)atoll(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_float(const wstring &name, const wstring &data)
  {
    return make_shared<float_value>(name, (float)atof(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_double(const wstring &name, const wstring &data)
  {
    return make_shared<double_value>(name, (double)atof(converter::to_string(data).c_str()));
  }

  shared_ptr<value> container_value::set_byte_string(const wstring &name, const wstring &data)
  {
    return make_shared<bytes_value>(name, converter::from_base64(data.c_str()));
  }

  shared_ptr<value> container_value::set_string(const wstring &name, const wstring &data)
  {
    return make_shared<string_value>(name, data);
  }

  shared_ptr<value> container_value::set_container(const wstring &name, const wstring &data)
  {
    return make_shared<container_value>(name, (long)atol(converter::to_string(data).c_str()));
  }
} // namespace container