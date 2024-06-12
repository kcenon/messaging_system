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
	auto converter::split(const std::string& source, const std::string& token)
		-> std::tuple<std::optional<std::vector<std::string>>,
					  std::optional<std::string>>
	{
		if (source.empty() == true)
		{
			return { std::nullopt, "source is empty" };
		}

		size_t offset = 0;
		size_t last_offset = 0;
		std::vector<std::string> result = {};
		std::string temp;

		while (true)
		{
			offset = source.find(token, last_offset);
			if (offset == std::string::npos)
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

		if (last_offset != 0 && last_offset != std::string::npos)
		{
			temp = source.substr(last_offset, offset - last_offset);
			if (!temp.empty())
			{
				result.push_back(std::move(temp));
			}
		}

		if (last_offset == 0)
		{
			return { std::vector<std::string>{ source }, std::nullopt };
		}

		return { result, std::nullopt };
	}

	auto converter::replace(std::string& source,
							const std::string& token,
							const std::string& target) -> void
	{
		source = replace2(source, token, target);
	}

	auto converter::replace2(const std::string& source,
							 const std::string& token,
							 const std::string& target) -> const std::string
	{
		if (source.empty())
		{
			return std::string();
		}

		fmt::wmemory_buffer result;

		size_t last_offset = 0;
		for (size_t offset = source.find(token, last_offset);
			 offset != std::string::npos; last_offset = offset + token.size(),
					offset = source.find(token, last_offset))
		{
			fmt::format_to(std::back_inserter(result), "{}{}",
						   source.substr(last_offset, offset - last_offset),
						   target);
		}

		// Add the last part of the std::string (after the last token, if
		// any)
		fmt::format_to(std::back_inserter(result), "{}",
					   source.substr(last_offset));

		return std::string(result.begin(), result.end());
	}

	auto converter::to_string(const std::string& value,
							  std::locale target_locale) -> std::string
	{
		if (value.empty())
		{
			return std::string();
		}

		std::u16string temp
			= convert(value); // Assuming convert is correctly implemented

		using codecvt_t = std::codecvt<char16_t, char, std::mbstate_t>;
		auto const& codecvt = std::use_facet<codecvt_t>(target_locale);

		std::mbstate_t state{}; // Zero-initialized

		std::vector<char> result(temp.size() * codecvt.max_length(), '\0');
		const char16_t* in_text = temp.data();
		char* out_text = result.data();

		auto condition
			= codecvt.out(state, in_text, in_text + temp.size(), in_text,
						  out_text, out_text + result.size(), out_text);

		if (condition != codecvt_t::ok)
		{
			return std::string();
		}

		return std::string(result.data());
	}

	auto converter::to_array(const std::string& value) -> std::vector<uint8_t>
	{
		if (value.empty())
		{
			return {};
		}

		std::string temp = to_string(value);

		return std::vector<uint8_t>(temp.begin(), temp.end());
	}

	auto converter::to_string(const std::vector<uint8_t>& value) -> std::string
	{
		if (value.empty())
		{
			return std::string();
		}

		// UTF-8 BOM
		if (value.size() >= 3 && value[0] == 0xef && value[1] == 0xbb
			&& value[2] == 0xbf)
		{
			return to_string(
				std::string(reinterpret_cast<const char*>(value.data()) + 3,
							value.size() - 3));
		}

		// UTF-8 no BOM
		return to_string(std::string(
			reinterpret_cast<const char*>(value.data()), value.size()));
	}

	auto converter::from_base64(const std::string& value)
		-> std::vector<uint8_t>
	{
		if (value.empty())
		{
			return std::vector<uint8_t>();
		}

		std::string source = to_string(value);
		std::string encoded;
		StringSource(source.data(), true,
					 new Base64Decoder(new StringSink(encoded)));

		return std::vector<uint8_t>(encoded.begin(), encoded.end());
	}

	auto converter::to_base64(const std::vector<uint8_t>& value) -> std::string
	{
		if (value.empty())
		{
			return std::string();
		}

		std::string decoded;
		StringSource(value.data(), value.size(), true,
					 new Base64Encoder(new StringSink(decoded)));

		return to_string(decoded);
	}

	auto converter::convert(const std::u16string& value) -> std::string
	{
		return std::string(value.begin(), value.end());
	}

	auto converter::convert(const std::string& value) -> std::u16string
	{
		return std::u16string(value.begin(), value.end());
	}
} // namespace converting