#include "pch.h"
#include "CppUnitTest.h"

#include "logging.h"
#include "file_handling.h"
#include "converting.h"

#include <regex>

using namespace logging;
using namespace converting;
using namespace file_handling;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace loggingtest
{
	TEST_CLASS(loggingtest)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			file_handler::remove(L"test.txt");

			logger::handle().start(L"test", L"txt", L"", false);
			for (int index = 0; index < 100; ++index)
			{
				logger::handle().write(logging::logging_level::information, L"test");
			}
			logger::handle().stop();

			std::wstring data = converter::to_wstring(file_handler::load(L"test.txt"));

			std::wregex full_condition(L"\\[\\d+:\\d+:\\d+.\\d+\\]\\[\\S+\\]");
			std::wsregex_iterator full_iter(data.begin(), data.end(), full_condition);
			std::wsregex_iterator full_end;
			if (full_iter == full_end)
			{
				Assert::Fail(L"Does not work");
			}

			int count = 0;
			while (full_iter != full_end)
			{
				count++;
				full_iter++;
			}

			Assert::IsTrue(count == 102, std::to_wstring(count).c_str());
		}
	};
}
