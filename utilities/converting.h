#pragma once

#include <locale>
#include <vector>
#include <string>

namespace converting
{
	class converter
	{
	public:
		static void replace_all(std::wstring& source, const std::wstring& token, const std::wstring& target);

	public:
		static std::wstring to_wstring(const std::string& value);
		static std::string to_string(const std::wstring& value);

	public:
		static std::vector<char> to_array(const std::wstring& value);
		static std::wstring to_wstring(const std::vector<char>& value);

	public:
		static std::vector<char> from_base64(const std::wstring& value);
		static std::wstring to_base64(const std::vector<char>& value);

	private:
		static std::wstring convert(const std::u16string& value);
		static std::u16string convert(const std::wstring& value);
	};
}

