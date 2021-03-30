#include "argument_parsing.h"

#include "converting.h"

using namespace converting;

namespace argument_parsing
{
	std::map<std::wstring, std::wstring> argument_parser::parse(int argc, char* argv[])
	{
		std::map<std::wstring, std::wstring> result;

		size_t offset = 0;
		std::wstring argument_id;
		for (int index = 1; index < argc; ++index)
		{
			if (index + 1 >= argc)
			{
				break;
			}

			argument_id = converter::to_wstring(argv[index]);
			offset = argument_id.find(L"--", 0);
			if (offset != 0)
			{
				continue;
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
}