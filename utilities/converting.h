#pragma once

#include <locale>
#include <vector>
#include <string>

namespace converting
{
	class converter
	{
	public:
		static std::wstring to_wstring(const std::string& value);
		static std::string to_string(const std::wstring& value);

	public:
		static std::vector<char> to_array(const std::wstring& value);
		static std::wstring to_wstring(const std::vector<char>& value);
	};
}

