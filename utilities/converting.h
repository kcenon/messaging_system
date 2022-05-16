#pragma once

#include <locale>
#include <vector>
#include <string>

using namespace std;

namespace converting
{
	class converter
	{
	public:
		static void replace(wstring& source, const wstring& token, const wstring& target);
		static const wstring replace2(const wstring& source, const wstring& token, const wstring& target);

	public:
		static wstring to_wstring(const string& value, locale target_locale = locale(""));
		static string to_string(const wstring& value, locale target_locale = locale(""));

	public:
		static vector<unsigned char> to_array(const wstring& value);
		static vector<unsigned char> to_array(const string& value);
		static wstring to_wstring(const vector<unsigned char>& value);
		static string to_string(const vector<unsigned char>& value);

	public:
		static vector<unsigned char> from_base64(const wstring& value);
		static wstring to_base64(const vector<unsigned char>& value);

	private:
		static wstring convert(const u16string& value);
		static u16string convert(const wstring& value);
	};
}

