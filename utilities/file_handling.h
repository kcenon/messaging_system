#pragma once

#include <string>
#include <vector>

namespace file_handling
{
	class file_handler
	{
	public:
		static std::vector<char> load(const std::wstring& path);
		static bool save(const std::wstring& path, const std::vector<char>& data);
		static bool append(const std::wstring& path, const std::vector<char>& data);
	};
}

