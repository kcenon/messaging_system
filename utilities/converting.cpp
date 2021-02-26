#include "converting.h"

#include <stdlib.h>

namespace converting
{
	std::wstring util::to_wstring(const std::string& value)
	{
		if (value.empty())
		{
			return L"";
		}

		size_t out_size;
		std::vector<wchar_t> result(value.size() + 1);
		mbstowcs_s(&out_size, &result[0], value.size() + 1, value.data(), value.size());

		return result.data();
	}

	std::string util::to_string(const std::wstring& value)
	{
		if (value.empty())
		{
			return "";
		}

		size_t out_size;
		std::vector<char> result(value.size());
		wcstombs_s(&out_size, &result[0], value.size() + 1, value.data(), value.size());

		return result.data();
	}

	std::vector<char> util::to_array(const std::wstring& value)
	{
		if (value.empty())
		{
			return std::vector<char>();
		}

		size_t out_size;
		std::vector<char> result(value.size());
		wcstombs_s(&out_size, &result[0], value.size() + 1, value.data(), value.size());

		return result;
	}

	std::wstring util::to_wstring(const std::vector<char>& value)
	{
		if (value.empty())
		{
			return L"";
		}

		size_t out_size;

		// UTF-8 BOM
		if (value.size() >= 3 && value[0] == 0xef && value[1] == 0xbb && value[2] == 0xbf)
		{
			std::vector<wchar_t> result(value.size() - 2);
			mbstowcs_s(&out_size, &result[0], value.size() - 2, value.data() + 3, value.size() - 3);

			return result.data();
		}

		// UTF-8 no BOM
		std::vector<wchar_t> result(value.size() + 1);
		mbstowcs_s(&out_size, &result[0], value.size() + 1, value.data(), value.size());

		return result.data();
	}
}