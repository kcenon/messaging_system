#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"

#include "fmt/format.h"

bool write(void)
{
	auto start = logging::util::handle().chrono_start();
	logging::util::handle().write(logging::logging_level::information, L"테스트_in_thread", start);

	return true;
}

bool write_data(const std::vector<char>& data)
{
	auto start = logging::util::handle().chrono_start();
	logging::util::handle().write(logging::logging_level::information, converting::util::to_wstring(data), start);

	return true;
}

int main()
{
	logging::util::handle().start();

	concurrency::thread_pool::handle().append(std::make_shared<concurrency::thread_worker>(concurrency::priorities::high));
	concurrency::thread_pool::handle().append(std::make_shared<concurrency::thread_worker>(concurrency::priorities::normal, 
		std::vector<concurrency::priorities> { concurrency::priorities::high }));
	concurrency::thread_pool::handle().append(std::make_shared<concurrency::thread_worker>(concurrency::priorities::low, 
		std::vector<concurrency::priorities> { concurrency::priorities::high, concurrency::priorities::normal }));
	concurrency::thread_pool::handle().start();

	for (unsigned int log_index = 0; log_index < 10000; ++log_index)
	{
		concurrency::job_pool::handle().push(std::make_shared<concurrency::job>(concurrency::priorities::high, &write));
		concurrency::job_pool::handle().push(std::make_shared<concurrency::job>(concurrency::priorities::high, converting::util::to_array(L"test2_in_thread"), &write_data));
	}

	std::this_thread::sleep_for(std::chrono::seconds(5));

	concurrency::thread_pool::handle().stop();
	logging::util::handle().stop();
}