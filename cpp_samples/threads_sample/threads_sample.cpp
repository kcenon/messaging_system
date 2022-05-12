#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"

#include "argument_parser.h"

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"thread_sample";

using namespace logging;
using namespace threads;
using namespace converting;
using namespace argument_parser;

bool write_console = false;
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif

bool parse_arguments(const map<wstring, wstring>& arguments);
void display_help(void);

void write_data(const vector<unsigned char>& data)
{
	auto start = logger::handle().chrono_start();
	logger::handle().write(logging_level::information, converter::to_wstring(data), start);
}

void write_high(void)
{
	write_data(converter::to_array(L"테스트2_high_in_thread"));
}

void write_normal(void)
{
	write_data(converter::to_array(L"테스트2_normal_in_thread"));
}

void write_low(void)
{
	write_data(converter::to_array(L"테스트2_low_in_thread"));
}

class saving_test_job : public job
{
public:
	saving_test_job(const priorities& priority, const vector<unsigned char>& data) : job(priority, data)
	{
		save();
	}

protected:
	void working(const priorities& worker_priority) override
	{
		auto start = logger::handle().chrono_start();
		logger::handle().write(logging_level::information, converter::to_wstring(_data), start);
	}
};

class test_job_without_data : public job
{
public:
	test_job_without_data(const priorities& priority) : job(priority)
	{
	}

protected:
	void working(const priorities& worker_priority) override
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
		default:
			break;
		}
	}
};

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument::parse(argc, argv)))
	{
		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
#ifdef _WIN32
	logger::handle().start(PROGRAM_NAME, locale("ko_KR.UTF-8"));
#else
	logger::handle().start(PROGRAM_NAME);
#endif

	thread_pool manager;
	manager.append(make_shared<thread_worker>(priorities::high));
	manager.append(make_shared<thread_worker>(priorities::high));
	manager.append(make_shared<thread_worker>(priorities::high));
	manager.append(make_shared<thread_worker>(priorities::normal, vector<priorities> { priorities::high }));
	manager.append(make_shared<thread_worker>(priorities::normal, vector<priorities> { priorities::high }));
	manager.append(make_shared<thread_worker>(priorities::low, vector<priorities> { priorities::high, priorities::normal }));
	
	// unit job with callback and data
	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(make_shared<job>(priorities::high, converter::to_array(L"테스트_high_in_thread"), &write_data));
		manager.push(make_shared<job>(priorities::normal, converter::to_array(L"테스트_normal_in_thread"), &write_data));
		manager.push(make_shared<job>(priorities::low, converter::to_array(L"테스트_low_in_thread"), &write_data));
	}

	// unit job with callback
	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(make_shared<job>(priorities::high, &write_high));
		manager.push(make_shared<job>(priorities::normal, &write_normal));
		manager.push(make_shared<job>(priorities::low, &write_low));
	}

	// derived job with data
	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(make_shared<saving_test_job>(priorities::high, converter::to_array(L"테스트3_high_in_thread")));
		manager.push(make_shared<saving_test_job>(priorities::normal, converter::to_array(L"테스트3_normal_in_thread")));
		manager.push(make_shared<saving_test_job>(priorities::low, converter::to_array(L"테스트3_low_in_thread")));
	}

	// derived job without data
	for (unsigned int log_index = 0; log_index < 1000; ++log_index)
	{
		manager.push(make_shared<test_job_without_data>(priorities::high));
		manager.push(make_shared<test_job_without_data>(priorities::normal));
		manager.push(make_shared<test_job_without_data>(priorities::low));
	}

	manager.start();
	manager.stop(false);

	logger::handle().stop();

	return 0;
}

bool parse_arguments(const map<wstring, wstring>& arguments)
{
	wstring temp;

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
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

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
		log_level = (logging_level)atoi(converter::to_string(target->second).c_str());
	}

	return true;
}

void display_help(void)
{
	wcout << L"download sample options:" << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}
