#pragma once

#include <map>
#include <string>

namespace argument_parsing
{
	class argument_parser
	{
	public:
		static std::map<std::wstring, std::wstring> parse(int argc, char* argv[]);
	};
}

