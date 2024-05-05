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

#include "value_types.h"

#include <map>

namespace container
{
  map<wstring, value_types> value_to_type_map = {
    { L"1", value_types::bool_value },   { L"2", value_types::short_value },    { L"3", value_types::ushort_value },
    { L"4", value_types::int_value },    { L"5", value_types::uint_value },     { L"6", value_types::long_value },
    { L"7", value_types::ulong_value },  { L"8", value_types::llong_value },    { L"9", value_types::ullong_value },
    { L"a", value_types::float_value },  { L"b", value_types::double_value },   { L"c", value_types::bytes_value },
    { L"d", value_types::string_value }, { L"e", value_types::container_value }
  };

  map<value_types, wstring> type_to_value_map = {
    { value_types::bool_value, L"1" },   { value_types::short_value, L"2" },    { value_types::ushort_value, L"3" },
    { value_types::int_value, L"4" },    { value_types::uint_value, L"5" },     { value_types::long_value, L"6" },
    { value_types::ulong_value, L"7" },  { value_types::llong_value, L"8" },    { value_types::ullong_value, L"9" },
    { value_types::float_value, L"a" },  { value_types::double_value, L"b" },   { value_types::bytes_value, L"c" },
    { value_types::string_value, L"d" }, { value_types::container_value, L"e" }
  };

  const value_types convert_value_type(const wstring &target)
  {
    map<wstring, value_types>::iterator iterator = value_to_type_map.find(target);
    if (iterator == value_to_type_map.end())
    {
      return value_types::null_value;
    }

    return iterator->second;
  }

  const wstring convert_value_type(const value_types &target)
  {
    map<value_types, wstring>::iterator iterator = type_to_value_map.find(target);
    if (iterator == type_to_value_map.end())
    {
      return L"0";
    }

    return iterator->second;
  }
} // namespace container