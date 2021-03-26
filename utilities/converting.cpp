#include "converting.h"

#include <codecvt>

#include "fmt/format.h"
#include "cryptopp/base64.h"

namespace converting
{
	void converter::replace_all(std::wstring& source, const std::wstring& token, const std::wstring& target)
	{
		if (source.empty() == true)
		{
			return;
		}
		size_t offset = 0;
		size_t last_offset = 0;
		std::wstring temp_string = L"";
		fmt::wmemory_buffer result;

		while (true)
		{
			offset = source.find(token, last_offset);
			if (offset == std::wstring::npos)
			{
				break;
			}

			temp_string = source.substr(last_offset, offset - last_offset);
			if (temp_string.empty() != true)
			{
				fmt::format_to(std::back_inserter(result), L"{}{}", temp_string, target);
			}			

			last_offset = offset + wcslen(token.c_str());
		}

		if (last_offset != 0 && last_offset != std::string::npos)
		{
			temp_string = source.substr(last_offset, offset - last_offset);
			if (!(temp_string.empty() == true))
			{
				fmt::format_to(std::back_inserter(result), L"{}", temp_string);
			}
		}

		if (result.size() == 0)
		{
			return;
		}

		source = result.data();
	}

	std::wstring converter::to_wstring(const std::string& value)
	{
		if (value.empty())
		{
			return std::wstring();
		}

		typedef std::codecvt<char16_t, char, std::mbstate_t> codecvt_t;
		codecvt_t const& codecvt = std::use_facet<codecvt_t>(std::locale(""));

		std::mbstate_t state;
		memset(&state, 0, sizeof(std::mbstate_t));

		std::vector<char16_t> result(value.size() + 1);
		char const* in_text = value.data();
		char16_t* out_text = &result[0];
		codecvt_t::result condition = codecvt.in(state, value.data(), value.data() + value.size(), in_text, &result[0], &result[0] + result.size(), out_text);

		return convert(result.data());
	}

	std::string converter::to_string(const std::wstring& value)
	{
		if (value.empty())
		{
			return std::string();
		}

		std::u16string temp = convert(value);

		typedef std::codecvt<char16_t, char, std::mbstate_t> codecvt_t;
		codecvt_t const& codecvt = std::use_facet<codecvt_t>(std::locale());

		std::mbstate_t state = std::mbstate_t();

		std::vector<char> result((temp.size() + 1) * codecvt.max_length());
		char16_t const* in_text = temp.data();
		char* out_text = &result[0];

		codecvt_t::result condition = codecvt.out(state, temp.data(), temp.data() + value.size(), in_text, &result[0], &result[0] + result.size(), out_text);

		return result.data();
	}

	std::vector<unsigned char> converter::to_array(const std::wstring& value)
	{
		if (value.empty())
		{
			return std::vector<unsigned char>();
		}

		std::string temp = to_string(value);

		return std::vector<unsigned char>(temp.data(), temp.data() + temp.size());
	}

	std::wstring converter::to_wstring(const std::vector<unsigned char>& value)
	{
		if (value.empty())
		{
			return std::wstring();
		}

		// UTF-8 BOM
		if (value.size() >= 3 && value[0] == 0xef && value[1] == 0xbb && value[2] == 0xbf)
		{
			return to_wstring(std::string((char*)value.data() + 2, value.size() - 2));
		}

		// UTF-8 no BOM
		return to_wstring(std::string((char*)value.data(), value.size()));
	}

	std::vector<unsigned char> converter::from_base64(const std::wstring& value)
	{
		if (value.empty())
		{
			return std::vector<unsigned char>();
		}

		std::string source = to_string(value);
		std::string encoded;
		CryptoPP::StringSource(source.data(), true, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(encoded)));

		return std::vector<unsigned char>(encoded.data(), encoded.data() + encoded.size());
	}

	std::wstring converter::to_base64(const std::vector<unsigned char>& value)
	{
		if (value.empty())
		{
			return std::wstring();
		}

		std::string decoded;
		CryptoPP::StringSource(value.data(), true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(decoded)));

		return to_wstring(decoded);
	}

	std::wstring converter::convert(const std::u16string& value)
	{
		return std::wstring(value.begin(), value.end());
	}

	std::u16string converter::convert(const std::wstring& value)
	{
		return std::u16string(value.begin(), value.end());
	}
}