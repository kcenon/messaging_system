#pragma once

#include <map>
#include <string>

namespace argument_parser
{
	class argument
	{
	public:
		static std::map<std::wstring, std::wstring> parse(int argc, char* argv[]);
		static std::map<std::wstring, std::wstring> parse(int argc, wchar_t* argv[]);
	};
}

