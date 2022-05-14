#pragma once

#include <map>
#include <string>

using namespace std;

namespace argument_parser
{
	class argument
	{
	public:
		static map<wstring, wstring> parse(int argc, char* argv[]);
		static map<wstring, wstring> parse(int argc, wchar_t* argv[]);
	};
}

