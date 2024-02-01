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

#pragma once

#include <locale>
#include <string>
#include <vector>

namespace converting
{
  class converter
  {
  public:
    static std::vector<std::wstring> split(const std::wstring &source, const std::wstring &token);

  public:
    static void replace(std::wstring &source, const std::wstring &token, const std::wstring &target);
    static const std::wstring
    replace2(const std::wstring &source, const std::wstring &token, const std::wstring &target);

  public:
    static std::wstring to_wstring(const std::string &value, std::locale target_locale = std::locale(""));
    static std::string to_string(const std::wstring &value, std::locale target_locale = std::locale(""));

  public:
    static std::vector<uint8_t> to_array(const std::wstring &value);
    static std::vector<uint8_t> to_array(const std::string &value);
    static std::wstring to_wstring(const std::vector<uint8_t> &value);
    static std::string to_string(const std::vector<uint8_t> &value);

  public:
    static std::vector<uint8_t> from_base64(const std::wstring &value);
    static std::wstring to_base64(const std::vector<uint8_t> &value);

  private:
    static std::wstring convert(const std::u16string &value);
    static std::u16string convert(const std::wstring &value);
  };
} // namespace converting
