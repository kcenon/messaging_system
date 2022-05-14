#include "argument_parser.h"

#include "converting.h"

using namespace converting;

namespace argument_parser
{
	map<wstring, wstring> argument::parse(int argc, char* argv[])
	{
		map<wstring, wstring> result;

		size_t offset = 0;
		wstring argument_id;
		for (int index = 1; index < argc; ++index)
		{
			argument_id = converter::to_wstring(argv[index]);
			offset = argument_id.find(L"--", 0);
			if (offset != 0)
			{
				continue;
			}

			if (argument_id.compare(L"--help") == 0)
			{
				result.insert({ argument_id, L"" });
				continue;
			}

			if (index + 1 >= argc)
			{
				break;
			}

			auto target = result.find(argument_id);
			if (target == result.end())
			{
				result.insert({ argument_id, converter::to_wstring(argv[index + 1]) });
				++index;

				continue;
			}
			
			target->second = converter::to_wstring(argv[index + 1]);
			++index;
		}

		return result;
	}
	
	map<wstring, wstring> argument::parse(int argc, wchar_t* argv[])
	{
		map<wstring, wstring> result;

		size_t offset = 0;
		wstring argument_id;
		for (int index = 1; index < argc; ++index)
		{
			argument_id = argv[index];
			offset = argument_id.find(L"--", 0);
			if (offset != 0)
			{
				continue;
			}

			if (argument_id.compare(L"--help") == 0)
			{
				result.insert({ argument_id, L"" });
				continue;
			}

			if (index + 1 >= argc)
			{
				break;
			}

			auto target = result.find(argument_id);
			if (target == result.end())
			{
				result.insert({ argument_id, argv[index + 1] });
				++index;

				continue;
			}

			target->second = argv[index + 1];
			++index;
		}

		return result;
	}
}