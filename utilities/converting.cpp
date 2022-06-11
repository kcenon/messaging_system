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

#include "fmt/xchar.h"
#include "fmt/format.h"
#include "cryptopp/base64.h"

using namespace CryptoPP;

namespace converting
{
	void converter::replace(wstring& source, const wstring& token, const wstring& target)
	{
		source = replace2(source, token, target);
	}

	const wstring converter::replace2(const wstring& source, const wstring& token, const wstring& target)
	{
		if (source.empty() == true)
		{
			return L"";
		}

		size_t offset = 0;
		size_t last_offset = 0;
		fmt::wmemory_buffer result;
		result.clear();

		while (true)
		{
			offset = source.find(token, last_offset);
			if (offset == wstring::npos)
			{
				break;
			}

			fmt::format_to(back_inserter(result), L"{}{}", source.substr(last_offset, offset - last_offset), target);

			last_offset = offset + token.size();
		}

		if (last_offset != 0 && last_offset != string::npos)
		{
			fmt::format_to(back_inserter(result), L"{}", source.substr(last_offset, offset - last_offset));
		}

		if (last_offset == 0)
		{
			return source;
		}

		return wstring(result.begin(), result.end());
	}

	wstring converter::to_wstring(const string& value, locale target_locale)
	{
		if (value.empty())
		{
			return wstring();
		}

		typedef codecvt<char16_t, char, mbstate_t> codecvt_t;
		codecvt_t const& codecvt = use_facet<codecvt_t>(target_locale);

		mbstate_t state;
		memset(&state, 0, sizeof(mbstate_t));

		vector<char16_t> result(value.size() + 1);
		char const* in_text = value.data();
		char16_t* out_text = &result[0];
		codecvt_t::result condition = codecvt.in(state, in_text, in_text + value.size(), in_text, 
			out_text, out_text + result.size(), out_text);

		return convert(result.data());
	}

	string converter::to_string(const wstring& value, locale target_locale)
	{
		if (value.empty())
		{
			return string();
		}

		u16string temp = convert(value);

		typedef codecvt<char16_t, char, mbstate_t> codecvt_t;
		codecvt_t const& codecvt = use_facet<codecvt_t>(target_locale);

		mbstate_t state = mbstate_t();

		vector<char> result((temp.size() + 1) * codecvt.max_length());
		char16_t const* in_text = temp.data();
		char* out_text = &result[0];

		codecvt_t::result condition = codecvt.out(state, in_text, in_text + value.size(), in_text, 
			out_text, out_text + result.size(), out_text);

		return result.data();
	}

	vector<unsigned char> converter::to_array(const wstring& value)
	{
		if (value.empty())
		{
			return vector<unsigned char>();
		}

		string temp = to_string(value);

		return vector<unsigned char>(temp.data(), temp.data() + temp.size());
	}

	vector<unsigned char> converter::to_array(const string& value)
	{
		return to_array(to_wstring(value));
	}

	wstring converter::to_wstring(const vector<unsigned char>& value)
	{
		if (value.empty())
		{
			return wstring();
		}

		// UTF-8 BOM
		if (value.size() >= 3 && value[0] == 0xef && value[1] == 0xbb && value[2] == 0xbf)
		{
			return to_wstring(string((char*)value.data() + 2, value.size() - 2));
		}

		// UTF-8 no BOM
		return to_wstring(string((char*)value.data(), value.size()));
	}

	string converter::to_string(const vector<unsigned char>& value)
	{
		return to_string(to_wstring(value));
	}

	vector<unsigned char> converter::from_base64(const wstring& value)
	{
		if (value.empty())
		{
			return vector<unsigned char>();
		}

		string source = to_string(value);
		string encoded;
		StringSource(source.data(), true, new Base64Decoder(new StringSink(encoded)));

		return vector<unsigned char>(encoded.data(), encoded.data() + encoded.size());
	}

	wstring converter::to_base64(const vector<unsigned char>& value)
	{
		if (value.empty())
		{
			return wstring();
		}

		string decoded;
		StringSource(value.data(), value.size(), true, new Base64Encoder(new StringSink(decoded)));

		return to_wstring(decoded);
	}

	wstring converter::convert(const u16string& value)
	{
		return wstring(value.begin(), value.end());
	}

	u16string converter::convert(const wstring& value)
	{
		return u16string(value.begin(), value.end());
	}
}