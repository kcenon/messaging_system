#pragma once

#include <string>
#include <vector>

using namespace std;

namespace file_handler
{
	class file
	{
	public:
		static bool remove(const wstring& path);
		static vector<unsigned char> load(const wstring& path);
		static bool save(const wstring& path, const vector<unsigned char>& data);
		static bool append(const wstring& path, const vector<unsigned char>& data);
	};
}

