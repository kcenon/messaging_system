#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"

#include "argument_parsing.h"

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"thread_sample";

using namespace logging;
using namespace converting;
using namespace threads;
using namespace argument_parsing;

bool write_console = false;
logging_level log_level = logging_level::information;

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments);
void display_help(void);

bool write_data(const std::vector<unsigned char>& data)
{
	auto start = logger::handle().chrono_start();
	logger::handle().write(logging_level::information, converter::to_wstring(data), start);

	return true;
}

bool write_high(void)
{
	return write_data(converter::to_array(L"테스트2_high_in_thread"));
}

bool write_normal(void)
{
	return write_data(converter::to_array(L"테스트2_normal_in_thread"));
}

bool write_low(void)
{
	return write_data(converter::to_array(L"테스트2_low_in_thread"));
}

class test_job : public job
{
public:
	test_job(const priorities& priority, const std::vector<unsigned char>& data) : job(priority)
	{
		_data = data;
	}

protected:
	bool working(const priorities& worker_priority) override
	{
		auto start = logger::handle().chrono_start();
		logger::handle().write(logging_level::information, converter::to_wstring(_data), start);

		return true;
	}

private:
	std::vector<unsigned char> _data;
};

class test2_job : public job
{
public:
	test2_job(const priorities& priority) : job(priority)
	{
	}

protected:
	bool working(const priorities& worker_priority) override
	{
		auto start = logger::handle().chrono_start();

		switch (priority())
		{
		case priorities::high: 
			logger::handle().write(logging_level::information, L"테스트4_high_in_thread", start);
			break;
		case priorities::normal:
			logger::handle().write(logging_level::information, L"테스트4_normal_in_thread", start);
			break;
		case priorities::low:
			logger::handle().write(logging_level::information, L"테스트4_low_in_thread", start);
			break;
		}		

		return true;
	}
};

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument_parser::parse(argc, argv)))
	{
		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	thread_pool manager;
	manager.append(std::make_shared<thread_worker>(priorities::high));
	manager.append(std::make_shared<thread_worker>(priorities::high));
	manager.append(std::make_shared<thread_worker>(priorities::high));
	manager.append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }));
	manager.append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }));
	manager.append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }));
	
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

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(std::make_shared<test_job>(priorities::high, converter::to_array(L"테스트3_high_in_thread")));
		manager.push(std::make_shared<test_job>(priorities::normal, converter::to_array(L"테스트3_normal_in_thread")));
		manager.push(std::make_shared<test_job>(priorities::low, converter::to_array(L"테스트3_low_in_thread")));
	}

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(std::make_shared<test2_job>(priorities::high));
		manager.push(std::make_shared<test2_job>(priorities::normal));
		manager.push(std::make_shared<test2_job>(priorities::low));
	}

	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(std::make_shared<job>(priorities::high, converter::to_array(L"(()=>{return \'테스트5_high_in_thread\';})()")));
		manager.push(std::make_shared<job>(priorities::normal, converter::to_array(L"(()=>{return \'테스트5_normal_in_thread\';})()")));
		manager.push(std::make_shared<job>(priorities::low, converter::to_array(L"(()=>{return \'테스트5_low_in_thread\';})()")));
	}

	manager.start();
	manager.stop(false);

	logger::handle().stop();

	return 0;
}

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments)
{
	std::wstring temp;

	auto target = arguments.find(L"--help");
	if (target != arguments.end())
	{
		display_help();

		return false;
	}

	target = arguments.find(L"--write_console_mode");
	if (target != arguments.end())
	{
		temp = target->second;
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true") == 0)
		{
			write_console = true;
		}
		else
		{
			write_console = false;
		}
	}

	target = arguments.find(L"--logging_level");
	if (target != arguments.end())
	{
		log_level = (logging_level)_wtoi(target->second.c_str());
	}

	return true;
}

void display_help(void)
{
	std::wcout << L"download sample options:" << std::endl << std::endl;
	std::wcout << L"--write_console_mode [value] " << std::endl;
	std::wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << std::endl << std::endl;
	std::wcout << L"--logging_level [value]" << std::endl;
	std::wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << std::endl;
}