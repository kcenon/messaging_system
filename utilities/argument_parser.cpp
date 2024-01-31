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

#include "argument_parser.h"

#include <algorithm>

#include "converting.h"

using namespace converting;

namespace argument_parser
{
  argument_manager::argument_manager(void) {}

  argument_manager::argument_manager(const string &arguments)
  {
    _arguments = parse(converter::split(converter::to_wstring(arguments), L" "));
  }

  argument_manager::argument_manager(const wstring &arguments)
  {
    _arguments = parse(converter::split(arguments, L" "));
  }

  argument_manager::argument_manager(int argc, char *argv[]) { _arguments = parse(argc, argv); }

  argument_manager::argument_manager(int argc, wchar_t *argv[]) { _arguments = parse(argc, argv); }

  optional<wstring> argument_manager::to_string(const wstring &key)
  {
    auto target = _arguments.find(key);
    if (target == _arguments.end())
    {
      return nullopt;
    }

    return target->second;
  }

  optional<bool> argument_manager::to_bool(const wstring &key)
  {
    auto target = to_string(key);
    if (target == nullopt)
    {
      return nullopt;
    }

    auto temp = *target;
    transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

    return temp.compare(L"true") == 0;
  }

  optional<short> argument_manager::to_short(const wstring &key)
  {
    auto target = to_string(key);
    if (target == nullopt)
    {
      return nullopt;
    }

    return (short)atoi(converter::to_string(*target).c_str());
  }

  optional<unsigned short> argument_manager::to_ushort(const wstring &key)
  {
    auto target = to_string(key);
    if (target == nullopt)
    {
      return nullopt;
    }

    return (unsigned short)atoi(converter::to_string(*target).c_str());
  }

  optional<int> argument_manager::to_int(const wstring &key)
  {
    auto target = to_string(key);
    if (target == nullopt)
    {
      return nullopt;
    }

    return (int)atoi(converter::to_string(*target).c_str());
  }

  optional<unsigned int> argument_manager::to_uint(const wstring &key)
  {
    auto target = to_string(key);
    if (target == nullopt)
    {
      return nullopt;
    }

    return (unsigned int)atoi(converter::to_string(*target).c_str());
  }

#ifdef _WIN32
  optional<long long> argument_manager::to_llong(const wstring &key)
#else
  optional<long> argument_manager::to_long(const wstring &key)
#endif
  {
    auto target = to_string(key);
    if (target == nullopt)
    {
      return nullopt;
    }

#ifdef _WIN32
    return (long long)atoll(converter::to_string(*target).c_str());
#else
    return (long)atol(converter::to_string(*target).c_str());
#endif
  }

#ifdef _WIN32
  optional<unsigned long long> argument_manager::to_ullong(const wstring &key)
#else
  optional<unsigned long> argument_manager::to_ulong(const wstring &key)
#endif
  {
    auto target = to_string(key);
    if (target == nullopt)
    {
      return nullopt;
    }

#ifdef _WIN32
    return (unsigned long long)atoll(converter::to_string(*target).c_str());
#else
    return (unsigned long)atol(converter::to_string(*target).c_str());
#endif
  }

  map<wstring, wstring> argument_manager::parse(int argc, char *argv[])
  {
    vector<wstring> arguments;
    for (int index = 1; index < argc; ++index)
    {
      arguments.push_back(converter::to_wstring(argv[index]));
    }

    return parse(arguments);
  }

  map<wstring, wstring> argument_manager::parse(int argc, wchar_t *argv[])
  {
    vector<wstring> arguments;
    for (int index = 1; index < argc; ++index)
    {
      arguments.push_back(argv[index]);
    }

    return parse(arguments);
  }

  map<wstring, wstring> argument_manager::parse(const vector<wstring> &arguments)
  {
    map<wstring, wstring> result;

    size_t argc = arguments.size();
    wstring argument_id;
    for (size_t index = 0; index < argc; ++index)
    {
      argument_id = arguments[index];
      size_t offset = argument_id.find(L"--", 0);
      if (offset != 0)
      {
        continue;
      }

      if (argument_id.compare(L"--help") == 0)
      {
        result.insert({ argument_id, L"display help" });
        continue;
      }

      if (index + 1 >= argc)
      {
        break;
      }

      auto target = result.find(argument_id);
      if (target == result.end())
      {
        result.insert({ argument_id, arguments[index + 1] });
        ++index;

        continue;
      }

      target->second = arguments[index + 1];
      ++index;
    }

    return result;
  }
} // namespace argument_parser