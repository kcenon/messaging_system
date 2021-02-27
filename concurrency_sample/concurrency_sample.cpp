#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "fmt/format.h"

void write(void)
{
	auto start = logging::util::handle().chrono_start();
	logging::util::handle().write(logging::logging_level::information, L"테스트_in_thread", start);
}

int main()
{
	logging::util::handle().start();

	concurrency::thread_pool::handle().append(std::make_shared<concurrency::thread_worker>(concurrency::priorities::high));
	concurrency::thread_pool::handle().append(std::make_shared<concurrency::thread_worker>(concurrency::priorities::normal, std::vector<concurrency::priorities> { concurrency::priorities::high }));
	concurrency::thread_pool::handle().append(std::make_shared<concurrency::thread_worker>(concurrency::priorities::low, std::vector<concurrency::priorities> { concurrency::priorities::high, concurrency::priorities::normal }));
	concurrency::thread_pool::handle().start();

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		auto temp = std::make_shared<concurrency::job>(&write);
		temp->set_priority(concurrency::priorities::high);

		concurrency::job_pool::handle().push(temp);
	}

	std::this_thread::sleep_for(std::chrono::seconds(2));

	concurrency::thread_pool::handle().stop();
	logging::util::handle().stop();
}