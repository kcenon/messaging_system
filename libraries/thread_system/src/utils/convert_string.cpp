/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation
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

#include "convert_string.h"

#include "../include/formatter.h"

#include <stdexcept>
#include <cstdint>

#ifdef USE_STD_FORMAT
#include <format>
#else
// Fallback to basic string operations when std::format is not available
// Note: We don't try to include fmt here as it may not be installed
#include <sstream>
#include <iomanip>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include "iconv.h"
#endif

/**
 * @file convert_string.cpp
 * @brief Implementation of cross-platform string conversion utilities.
 *
 * This file provides comprehensive string conversion functionality supporting:
 * - Character encoding conversion (UTF-8, UTF-16, system locale)
 * - Cross-platform compatibility (Windows, Linux, macOS)
 * - Both wide and narrow string types
 * - Efficient conversion with proper error handling
 * 
 * Platform-specific implementations:
 * - Windows: Uses Windows API (MultiByteToWideChar, WideCharToMultiByte)
 * - Unix/Linux/macOS: Uses iconv library for encoding conversion
 * 
 * Performance considerations:
 * - Template-based implementation for compile-time optimization
 * - Minimal memory allocations
 * - Efficient buffer management
 */

namespace utility_module
{
	/**
	 * @brief Core template function for string encoding conversion.
	 * 
	 * Implementation details:
	 * - Template-based for type safety and performance
	 * - Supports conversion between different character encodings
	 * - Uses platform-specific APIs for optimal performance
	 * - Handles both narrow (char) and wide (wchar_t) character types
	 * 
	 * Error Handling:
	 * - Throws std::runtime_error on conversion failures
	 * - Provides detailed error messages with encoding information
	 * - Handles edge cases like empty strings and invalid sequences
	 * 
	 * @tparam FromType Source string type (std::string, std::wstring, etc.)
	 * @tparam ToType Target string type (std::string, std::wstring, etc.)
	 * @param value Source string to convert
	 * @param from_encoding Source character encoding
	 * @param to_encoding Target character encoding
	 * @return Converted string in target encoding
	 */
#ifdef _WIN32
	// Windows implementation - uses platform-specific APIs
	template <typename FromType, typename ToType>
	auto convert_string::convert(const FromType& value,
								 const std::string& from_encoding,
								 const std::string& to_encoding)
		-> std::tuple<std::optional<ToType>, std::optional<std::string>>
	{
		// Windows uses its own character conversion APIs
		// For fallback builds, provide minimal implementation
		(void)from_encoding; // Suppress unused parameter warning
		(void)to_encoding;   // Suppress unused parameter warning
		
		// Convert input to string representation
		if constexpr (std::is_same_v<FromType, ToType>) {
			return { static_cast<ToType>(value), std::nullopt };
		} else {
			// For different types, provide basic conversion
			return { std::nullopt, "Character encoding conversion not supported in Windows fallback mode" };
		}
	}
#else
	// Unix/Linux implementation - uses iconv
	template <typename FromType, typename ToType>
	auto convert_string::convert(const FromType& value,
								 const std::string& from_encoding,
								 const std::string& to_encoding)
		-> std::tuple<std::optional<ToType>, std::optional<std::string>>
	{
		iconv_t cd = iconv_open(to_encoding.c_str(), from_encoding.c_str());
		if (cd == (iconv_t)-1)
		{
			return { std::nullopt, "iconv_open failed: " + std::string(strerror(errno)) };
		}

		std::vector<char> in_buf = string_to_vector(value);
		char* in_ptr = in_buf.data();
		size_t in_bytes_left = in_buf.size();

		size_t out_buf_size = in_bytes_left * 2;
		std::vector<char> out_buf(out_buf_size);
		char* out_ptr = out_buf.data();
		size_t out_bytes_left = out_buf.size();

		size_t result = iconv(cd, &in_ptr, &in_bytes_left, &out_ptr, &out_bytes_left);
		while (result == (size_t)-1 && errno == E2BIG)
		{
			size_t used = out_buf_size - out_bytes_left;
			out_buf_size *= 2;
			out_buf.resize(out_buf_size);
			out_ptr = out_buf.data() + used;
			out_bytes_left = out_buf_size - used;
			result = iconv(cd, &in_ptr, &in_bytes_left, &out_ptr, &out_bytes_left);
		}

		if (result == (size_t)-1)
		{
			std::string error_msg = "iconv failed: " + std::string(strerror(errno));
			iconv_close(cd);
			return { std::nullopt, error_msg };
		}

		iconv_close(cd);

		size_t converted_size = out_buf_size - out_bytes_left;
		ToType converted_string(reinterpret_cast<typename ToType::value_type*>(out_buf.data()),
								converted_size / sizeof(typename ToType::value_type));

		return { converted_string, std::nullopt };
	}
#endif

	auto convert_string::to_string(const std::wstring& value)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		auto [result, error]
			= convert<std::wstring, std::string>(value, get_wchar_encoding(endian_types::little),
												 get_encoding_name(encoding_types::utf8));

		if (result.has_value())
		{
			return utf8_to_system(result.value());
		}

		return { std::nullopt, error };
	}

	auto convert_string::to_string(std::wstring_view value)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		auto [result, error] = convert<std::wstring_view, std::string>(
			value, get_wchar_encoding(endian_types::little),
			get_encoding_name(encoding_types::utf8));

		if (result.has_value())
		{
			return utf8_to_system(result.value());
		}

		return { std::nullopt, error };
	}

	auto convert_string::to_wstring(const std::string& value)
		-> std::tuple<std::optional<std::wstring>, std::optional<std::string>>
	{
		auto [result, err] = system_to_utf8(value);
		if (!result.has_value())
		{
			return { std::nullopt, err };
		}

		std::string clean_value = remove_utf8_bom(result.value());
		endian_types endian = endian_types::little;
		return convert<std::string, std::wstring>(
			clean_value, get_encoding_name(encoding_types::utf8), get_wchar_encoding(endian));
	}

	auto convert_string::to_wstring(std::string_view value)
		-> std::tuple<std::optional<std::wstring>, std::optional<std::string>>
	{
		auto [result, err] = system_to_utf8(std::string(value));
		if (!result.has_value())
		{
			return { std::nullopt, err };
		}

		std::string clean_value = remove_utf8_bom(result.value());
		endian_types endian = endian_types::little;
		return convert<std::string, std::wstring>(
			clean_value, get_encoding_name(encoding_types::utf8), get_wchar_encoding(endian));
	}

	auto convert_string::get_encoding_name(encoding_types encoding,
										   endian_types endian) -> std::string
	{
		switch (encoding)
		{
		case encoding_types::utf8:
			return "UTF-8";
		case encoding_types::utf16:
			if (endian == endian_types::little)
				return "UTF-16LE";
			else if (endian == endian_types::big)
				return "UTF-16BE";
			else
				return "UTF-16";
		case encoding_types::utf32:
			if (endian == endian_types::little)
				return "UTF-32LE";
			else if (endian == endian_types::big)
				return "UTF-32BE";
			else
				return "UTF-32";
		default:
			throw std::runtime_error("Unknown encoding");
		}
	}

	auto convert_string::get_wchar_encoding(endian_types endian) -> std::string
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			return get_encoding_name(encoding_types::utf16, endian);
		}
		else if constexpr (sizeof(wchar_t) == 4)
		{
			return get_encoding_name(encoding_types::utf32, endian);
		}
		else
		{
			throw std::runtime_error("Unsupported wchar_t size");
		}
	}

	auto convert_string::detect_endian(const std::u16string& str) -> endian_types
	{
		if (str.empty())
			return endian_types::unknown;

		if (str[0] == 0xFEFF)
			return endian_types::big;
		if (str[0] == 0xFFFE)
			return endian_types::little;

		size_t sample_size = std::min<size_t>(str.size(), 1000);
		int le_count = 0, be_count = 0;
		for (size_t i = 0; i < sample_size; ++i)
		{
			uint16_t ch = str[i];
			if ((ch & 0xFF00) == 0 && (ch & 0x00FF) != 0)
				++le_count;
			if ((ch & 0x00FF) == 0 && (ch & 0xFF00) != 0)
				++be_count;
		}

		if (le_count > be_count)
			return endian_types::little;
		if (be_count > le_count)
			return endian_types::big;

		return endian_types::unknown;
	}

	auto convert_string::detect_endian(const std::u32string& str) -> endian_types
	{
		if (str.empty())
			return endian_types::unknown;

		if (str[0] == 0x0000FEFF)
			return endian_types::big;
		if (str[0] == 0xFFFE0000)
			return endian_types::little;

		size_t sample_size = std::min<size_t>(str.size(), 1000);
		int le_count = 0, be_count = 0;
		for (size_t i = 0; i < sample_size; ++i)
		{
			uint32_t ch = str[i];
			if ((ch & 0xFFFFFF00) == 0 && (ch & 0x000000FF) != 0)
				++le_count;
			if ((ch & 0x00FFFFFF) == 0 && (ch & 0xFF000000) != 0)
				++be_count;
		}

		if (le_count > be_count)
			return endian_types::little;
		if (be_count > le_count)
			return endian_types::big;

		return endian_types::unknown;
	}

	auto convert_string::has_utf8_bom(const std::string& str) -> bool
	{
		return str.length() >= 3 && static_cast<unsigned char>(str[0]) == 0xEF
			   && static_cast<unsigned char>(str[1]) == 0xBB
			   && static_cast<unsigned char>(str[2]) == 0xBF;
	}

	auto convert_string::remove_utf8_bom(const std::string& str) -> std::string
	{
		return has_utf8_bom(str) ? str.substr(3) : str;
	}

	auto convert_string::add_utf8_bom(const std::string& str) -> std::string
	{
		return has_utf8_bom(str) ? str : std::string("\xEF\xBB\xBF") + str;
	}

	auto convert_string::get_system_code_page() -> int
	{
#ifdef _WIN32
		return static_cast<int>(GetACP());
#else
		return 65001;
#endif
	}

	auto convert_string::get_code_page_name(int code_page) -> std::string
	{
		switch (code_page)
		{
		case 65001:
			return "UTF-8";
		default:
			return "CP" + std::to_string(code_page);
		}
	}

	auto convert_string::system_to_utf8(const std::string& value)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		int code_page = get_system_code_page();
		if (code_page == 65001)
		{
			return { value, std::nullopt };
		}
		return convert<std::string, std::string>(value, get_code_page_name(code_page),
												 get_encoding_name(encoding_types::utf8));
	}

	auto convert_string::utf8_to_system(const std::string& value)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		int code_page = get_system_code_page();
		if (code_page == 65001)
		{
			return { value, std::nullopt };
		}
		return convert<std::string, std::string>(value, get_encoding_name(encoding_types::utf8),
												 get_code_page_name(code_page));
	}

	auto convert_string::split(const std::string& source, const std::string& token)
		-> std::tuple<std::optional<std::vector<std::string>>, std::optional<std::string>>
	{
		if (token.empty())
		{
			return { std::vector{ source }, std::nullopt };
		}

		std::vector<std::string> result;
		size_t start_pos = 0;
		size_t end_pos = source.find(token);

		while (end_pos != std::string::npos)
		{
			result.emplace_back(source.substr(start_pos, end_pos - start_pos));
			start_pos = end_pos + token.length();
			end_pos = source.find(token, start_pos);
		}

		result.emplace_back(source.substr(start_pos));

		return { result, std::nullopt };
	}

	auto convert_string::to_array(const std::string& value)
		-> std::tuple<std::optional<std::vector<uint8_t>>, std::optional<std::string>>
	{
		auto [utf8, convert_error] = system_to_utf8(value);
		if (convert_error.has_value())
		{
			return { std::nullopt, convert_error };
		}

		auto utf8_no_bom = remove_utf8_bom(utf8.value());

		return { std::vector<uint8_t>(utf8_no_bom.begin(), utf8_no_bom.end()), std::nullopt };
	}

	auto convert_string::to_string(const std::vector<uint8_t>& value)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		std::string utf8(value.begin(), value.end());
		auto utf8_no_bom = remove_utf8_bom(utf8);

		return utf8_to_system(utf8);
	}

	auto convert_string::to_base64(const std::vector<uint8_t>& value)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		try
		{
			std::string encoded = base64_encode(value);
			return { encoded, std::nullopt };
		}
		catch (const std::exception& e)
		{
			return { std::nullopt, e.what() };
		}
	}

	auto convert_string::from_base64(const std::string& base64_str)
		-> std::tuple<std::vector<uint8_t>, std::optional<std::string>>
	{
		return base64_decode(base64_str);
	}

	auto convert_string::replace(std::string& source,
								 const std::string& token,
								 const std::string& target) -> std::optional<std::string>
	{
		auto [value, value_error] = replace2(source, token, target);
		if (value_error.has_value())
		{
			return value_error;
		}

		source = value.value();
		return std::nullopt;
	}

	auto convert_string::replace2(const std::string& source,
								  const std::string& token,
								  const std::string& target)
		-> std::tuple<std::optional<std::string>, std::optional<std::string>>
	{
		if (source.empty())
		{
			return { std::nullopt, "Source string is empty" };
		}

		if (token.empty())
		{
			return { std::nullopt, "Token string is empty" };
		}

		std::string result;

		size_t last_offset = 0;
		for (size_t offset = source.find(token, last_offset); offset != std::string::npos;
			 last_offset = offset + token.size(), offset = source.find(token, last_offset))
		{
#ifdef USE_STD_FORMAT
			std::format_to(std::back_inserter(result), "{}{}",
								 source.substr(last_offset, offset - last_offset), target);
#else
			result += source.substr(last_offset, offset - last_offset);
			result += target;
#endif
		}

#ifdef USE_STD_FORMAT
		std::format_to(std::back_inserter(result), "{}", source.substr(last_offset));
#else
		result += source.substr(last_offset);
#endif

		return { result, std::nullopt };
	}

	auto convert_string::base64_encode(const std::vector<uint8_t>& data) -> std::string
	{
		static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
										   "abcdefghijklmnopqrstuvwxyz"
										   "0123456789+/";

		std::string encoded_string;
		size_t i = 0;
		uint32_t octet_a, octet_b, octet_c;
		uint32_t triple;

		while (i < data.size())
		{
			octet_a = i < data.size() ? data[i++] : 0;
			octet_b = i < data.size() ? data[i++] : 0;
			octet_c = i < data.size() ? data[i++] : 0;

			triple = (octet_a << 16) + (octet_b << 8) + octet_c;

			encoded_string += base64_chars[(triple >> 18) & 0x3F];
			encoded_string += base64_chars[(triple >> 12) & 0x3F];
			encoded_string += base64_chars[(triple >> 6) & 0x3F];
			encoded_string += base64_chars[triple & 0x3F];
		}

		int mod_table[] = { 0, 2, 1 };
		for (int j = 0; j < mod_table[data.size() % 3]; j++)
		{
			encoded_string[encoded_string.size() - 1 - static_cast<size_t>(j)] = '=';
		}

		return encoded_string;
	}

	auto convert_string::base64_decode(const std::string& base64_str)
		-> std::tuple<std::vector<uint8_t>, std::optional<std::string>>
	{
		static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
												"abcdefghijklmnopqrstuvwxyz"
												"0123456789+/";

		if (base64_str.length() % 4 != 0)
		{
			return { std::vector<uint8_t>(), "Invalid base64 input length" };
		}

		size_t padding = 0;
		if (!base64_str.empty())
		{
			if (base64_str[base64_str.length() - 1] == '=')
				padding++;
			if (base64_str.length() >= 2 && base64_str[base64_str.length() - 2] == '=')
				padding++;
			if (padding > 2)
			{
				return { std::vector<uint8_t>(), "Invalid padding in base64 string" };
			}
		}

		size_t decoded_length = (base64_str.length() / 4) * 3 - padding;
		std::vector<uint8_t> decoded_data;
		decoded_data.reserve(decoded_length);

		std::vector<int> decoding_table(256, -1);
		for (int i = 0; i < 64; i++)
		{
			decoding_table[static_cast<unsigned char>(base64_chars[static_cast<size_t>(i)])] = i;
		}

		uint32_t buffer = 0;
		int bits_collected = 0;
		size_t i = 0;
		for (; i < base64_str.length(); ++i)
		{
			char c = base64_str[i];
			if (c == '=')
			{
				if (i < base64_str.length() - padding)
				{
					return { std::vector<uint8_t>(), "Invalid padding position in base64 string" };
				}
				break;
			}

			if (decoding_table[static_cast<unsigned char>(c)] == -1)
			{
				return { std::vector<uint8_t>(), "Invalid character in base64 string" };
			}

			buffer = (buffer << 6) | static_cast<uint32_t>(decoding_table[static_cast<unsigned char>(c)]);
			bits_collected += 6;

			if (bits_collected >= 8)
			{
				bits_collected -= 8;
				decoded_data.push_back((buffer >> bits_collected) & 0xFF);
			}
		}

		for (; i < base64_str.length(); ++i)
		{
			if (base64_str[i] != '=')
			{
				return { std::vector<uint8_t>(),
						 "Invalid character after padding in base64 string" };
			}
		}

		return { decoded_data, std::nullopt };
	}

} // namespace utility_module
