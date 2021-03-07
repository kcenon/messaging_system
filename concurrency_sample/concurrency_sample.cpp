#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"

#include "fmt/format.h"

using namespace logging;
using namespace converting;
using namespace concurrency;

bool write_high(void)
{
	auto start = logger::handle().chrono_start();
	logger::handle().write(logging_level::information, L"테스트2_high_in_thread", start);

	return true;
}

bool write_normal(void)
{
	auto start = logger::handle().chrono_start();
	logger::handle().write(logging_level::information, L"테스트2_normal_in_thread", start);

	return true;
}

bool write_low(void)
{
	auto start = logger::handle().chrono_start();
	logger::handle().write(logging_level::information, L"테스트2_low_in_thread", start);

	return true;
}

bool write_data(const std::vector<char>& data)
{
	auto start = logger::handle().chrono_start();
	logger::handle().write(logging_level::information, converter::to_wstring(data), start);

	return true;
}

int main()
{
	logger::handle().set_target_level(logging_level::information);
	logger::handle().start();

	thread_pool::handle().append(std::make_shared<thread_worker>(priorities::high));
	thread_pool::handle().append(std::make_shared<thread_worker>(priorities::high));
	thread_pool::handle().append(std::make_shared<thread_worker>(priorities::high));
	thread_pool::handle().append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }));
	thread_pool::handle().append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }));
	thread_pool::handle().append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }));

	thread_pool::handle().start();

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		job_pool::handle().push(std::make_shared<job>(priorities::high, converter::to_array(L"테스트_high_in_thread"), &write_data));
		job_pool::handle().push(std::make_shared<job>(priorities::normal, converter::to_array(L"테스트_normal_in_thread"), &write_data));
		job_pool::handle().push(std::make_shared<job>(priorities::low, converter::to_array(L"테스트_low_in_thread"), &write_data));
	}

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		job_pool::handle().push(std::make_shared<job>(priorities::high, &write_high));
		job_pool::handle().push(std::make_shared<job>(priorities::normal, &write_normal));
		job_pool::handle().push(std::make_shared<job>(priorities::low, &write_low));
	}

	std::this_thread::sleep_for(std::chrono::seconds(5));

	thread_pool::handle().stop();
	logger::handle().stop();
}