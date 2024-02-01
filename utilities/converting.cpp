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

#include "converting.h"

#include <codecvt>

#include "cryptopp/base64.h"
#include "fmt/format.h"
#include "fmt/xchar.h"

using namespace CryptoPP;

namespace converting
{
  std::vector<std::wstring> converter::split(const std::wstring &source, const std::wstring &token)
  {
    if (source.empty() == true)
    {
      return {};
    }

    size_t offset = 0;
    size_t last_offset = 0;
    std::vector<std::wstring> result = {};
    std::wstring temp;

    while (true)
    {
      offset = source.find(token, last_offset);
      if (offset == std::wstring::npos)
      {
        break;
      }

      temp = source.substr(last_offset, offset - last_offset);
      if (!temp.empty())
      {
        result.push_back(std::move(temp));
      }

      last_offset = offset + token.size();
    }

    if (last_offset != 0 && last_offset != std::wstring::npos)
    {
      temp = source.substr(last_offset, offset - last_offset);
      if (!temp.empty())
      {
        result.push_back(std::move(temp));
      }
    }

    if (last_offset == 0)
    {
      return { source };
    }

    return result;
  }

  void converter::replace(std::wstring &source, const std::wstring &token, const std::wstring &target)
  {
    source = replace2(source, token, target);
  }

  const std::wstring
  converter::replace2(const std::wstring &source, const std::wstring &token, const std::wstring &target)
  {
    if (source.empty())
    {
      return std::wstring();
    }

    fmt::wmemory_buffer result;

    size_t last_offset = 0;
    for (size_t offset = source.find(token, last_offset); offset != std::wstring::npos;
         last_offset = offset + token.size(), offset = source.find(token, last_offset))
    {
      fmt::format_to(std::back_inserter(result), L"{}{}", source.substr(last_offset, offset - last_offset), target);
    }

    // Add the last part of the std::string (after the last token, if any)
    fmt::format_to(std::back_inserter(result), L"{}", source.substr(last_offset));

    return std::wstring(result.begin(), result.end());
  }

  std::wstring converter::to_wstring(const std::string &value, std::locale target_locale)
  {
    if (value.empty())
    {
      return std::wstring();
    }

    using codecvt_t = std::codecvt<char16_t, char, std::mbstate_t>;
    auto const &codecvt = std::use_facet<codecvt_t>(target_locale);

    std::mbstate_t state = std::mbstate_t(); // Zero-initialized state

    std::vector<char16_t> result(value.size());
    const char *in_text = value.data();
    char16_t *out_text = result.data();

    auto condition
        = codecvt.in(state, in_text, in_text + value.size(), in_text, out_text, out_text + result.size(), out_text);

    if (condition != codecvt_t::ok)
    {
      return std::wstring();
    }

    return convert(result.data());
  }

  std::string converter::to_string(const std::wstring &value, std::locale target_locale)
  {
    if (value.empty())
    {
      return std::string();
    }

    std::u16string temp = convert(value); // Assuming convert is correctly implemented

    using codecvt_t = std::codecvt<char16_t, char, std::mbstate_t>;
    auto const &codecvt = std::use_facet<codecvt_t>(target_locale);

    std::mbstate_t state{}; // Zero-initialized

    std::vector<char> result(temp.size() * codecvt.max_length(), '\0');
    const char16_t *in_text = temp.data();
    char *out_text = result.data();

    auto condition
        = codecvt.out(state, in_text, in_text + temp.size(), in_text, out_text, out_text + result.size(), out_text);

    if (condition != codecvt_t::ok)
    {
      return std::string();
    }

    return std::string(result.data());
  }

  std::vector<uint8_t> converter::to_array(const std::wstring &value)
  {
    if (value.empty())
    {
      return {};
    }

    std::string temp = to_string(value);

    return std::vector<uint8_t>(temp.begin(), temp.end());
  }

  std::vector<uint8_t> converter::to_array(const std::string &value) { return to_array(to_wstring(value)); }

  std::wstring converter::to_wstring(const std::vector<uint8_t> &value)
  {
    if (value.empty())
    {
      return std::wstring();
    }

    // UTF-8 BOM
    if (value.size() >= 3 && value[0] == 0xef && value[1] == 0xbb && value[2] == 0xbf)
    {
      return to_wstring(std::string(reinterpret_cast<const char *>(value.data()) + 3, value.size() - 3));
    }

    // UTF-8 no BOM
    return to_wstring(std::string(reinterpret_cast<const char *>(value.data()), value.size()));
  }

  std::string converter::to_string(const std::vector<uint8_t> &value) { return to_string(to_wstring(value)); }

  std::vector<uint8_t> converter::from_base64(const std::wstring &value)
  {
    if (value.empty())
    {
      return std::vector<uint8_t>();
    }

    std::string source = to_string(value);
    std::string encoded;
    StringSource(source.data(), true, new Base64Decoder(new StringSink(encoded)));

    return std::vector<uint8_t>(encoded.begin(), encoded.end());
  }

  std::wstring converter::to_base64(const std::vector<uint8_t> &value)
  {
    if (value.empty())
    {
      return std::wstring();
    }

    std::string decoded;
    StringSource(value.data(), value.size(), true, new Base64Encoder(new StringSink(decoded)));

    return to_wstring(decoded);
  }

  std::wstring converter::convert(const std::u16string &value) { return std::wstring(value.begin(), value.end()); }

  std::u16string converter::convert(const std::wstring &value) { return std::u16string(value.begin(), value.end()); }
} // namespace converting