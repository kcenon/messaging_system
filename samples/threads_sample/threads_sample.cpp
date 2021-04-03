#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"thread_sample";

using namespace logging;
using namespace converting;
using namespace threads;

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

bool write_data(const std::vector<unsigned char>& data)
{
	auto start = logger::handle().chrono_start();
	logger::handle().write(logging_level::information, converter::to_wstring(data), start);

	return true;
}

int main()
{
	logger::handle().set_target_level(logging_level::information);
	logger::handle().start(PROGRAM_NAME);

	thread_pool manager;
	manager.append(std::make_shared<thread_worker>(priorities::high), true);
	manager.append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }), true);
	manager.append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }), true);
	manager.append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }), true);
	manager.append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }), true);
	manager.append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }), true);

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(std::make_shared<job>(priorities::high, converter::to_array(L"테스트_high_in_thread"), &write_data));
		manager.push(std::make_shared<job>(priorities::normal, converter::to_array(L"테스트_normal_in_thread"), &write_data));
		manager.push(std::make_shared<job>(priorities::low, converter::to_array(L"테스트_low_in_thread"), &write_data));
	}

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(std::make_shared<job>(priorities::high, &write_high));
		manager.push(std::make_shared<job>(priorities::normal, &write_normal));
		manager.push(std::make_shared<job>(priorities::low, &write_low));
	}

	std::this_thread::sleep_for(std::chrono::seconds(5));

	manager.stop();
	logger::handle().stop();

	return 0;
}