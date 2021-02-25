#include "logging.h"

#include "limits.h"

#include "fmt/format.h"

#include <iostream>

int main()
{
	logging::util::handle().start();

	std::vector<std::thread> threads;
	for (unsigned short thread_index = 0; thread_index < 10; ++thread_index)
	{
		threads.push_back(
			std::thread([](const unsigned short& thread_index)
				{
					for (unsigned int log_index = 0; log_index < 1000; ++log_index)
					{
						logging::util::handle().write(logging::logging_level::information, fmt::format(L"테스트_in_thread_{}: {}", thread_index, log_index));
					}
				}, thread_index)
		);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	logging::util::handle().stop();

    return 0;
}