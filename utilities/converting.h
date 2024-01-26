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

using namespace std;

namespace converting {
class converter {
public:
  static vector<wstring> split(const wstring &source, const wstring &token);

public:
  static void replace(wstring &source, const wstring &token,
                      const wstring &target);
  static const wstring replace2(const wstring &source, const wstring &token,
                                const wstring &target);

public:
  static wstring to_wstring(const string &value,
                            locale target_locale = locale(""));
  static string to_string(const wstring &value,
                          locale target_locale = locale(""));

public:
  static vector<uint8_t> to_array(const wstring &value);
  static vector<uint8_t> to_array(const string &value);
  static wstring to_wstring(const vector<uint8_t> &value);
  static string to_string(const vector<uint8_t> &value);

public:
  static vector<uint8_t> from_base64(const wstring &value);
  static wstring to_base64(const vector<uint8_t> &value);

private:
  static wstring convert(const u16string &value);
  static u16string convert(const wstring &value);
};
} // namespace converting
