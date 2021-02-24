#include "logging.h"

#include "limits.h"

#include "fmt/format.h"

#include <iostream>

int main()
{
	logging::util::handle().start();

	std::vector<std::thread> threads;
	for (unsigned short index = 0; index < 10; ++index)
	{
		threads.push_back(
			std::thread([]
				{
					for (unsigned int index = 0; index < 1000; ++index)
					{
						logging::util::handle().write(logging::logging_level::information, fmt::format(L"테스트_in_thread_{}", index));
					}
				})
		);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	logging::util::handle().stop();

    return 0;
}