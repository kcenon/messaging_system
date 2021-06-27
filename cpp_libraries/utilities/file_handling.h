#pragma once

#include <string>
#include <vector>

namespace file_handling
{
	class file_handler
	{
	public:
		static bool remove(const std::wstring& path);
		static std::vector<unsigned char> load(const std::wstring& path);
		static bool save(const std::wstring& path, const std::vector<unsigned char>& data);
		static bool append(const std::wstring& path, const std::vector<unsigned char>& data);
	};
}

