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

#include <tuple>
#include <locale>
#include <string>
#include <vector>
#include <optional>

namespace converting
{
	class converter
	{
	public:
		static auto split(const std::wstring& source, const std::wstring& token)
			-> std::tuple<std::optional<std::vector<std::wstring>>,
						  std::optional<std::wstring>>;

	public:
		static void replace(std::wstring& source,
							const std::wstring& token,
							const std::wstring& target);
		static auto replace2(const std::wstring& source,
							 const std::wstring& token,
							 const std::wstring& target) -> const std::wstring;

	public:
		static auto to_wstring(const std::string& value,
							   std::locale target_locale = std::locale(""))
			-> std::wstring;
		static auto to_string(const std::wstring& value,
							  std::locale target_locale = std::locale(""))
			-> std::string;

	public:
		static auto to_array(const std::wstring& value) -> std::vector<uint8_t>;
		static auto to_array(const std::string& value) -> std::vector<uint8_t>;
		static auto to_wstring(const std::vector<uint8_t>& value)
			-> std::wstring;
		static auto to_string(const std::vector<uint8_t>& value) -> std::string;

	public:
		static auto from_base64(const std::wstring& value)
			-> std::vector<uint8_t>;
		static auto to_base64(const std::vector<uint8_t>& value)
			-> std::wstring;

	private:
		static auto convert(const std::u16string& value) -> std::wstring;
		static auto convert(const std::wstring& value) -> std::u16string;
	};
} // namespace converting
