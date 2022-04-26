#include "logging.h"

#include "argument_parser.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <iostream>

constexpr auto PROGRAM_NAME = L"logging_sample";

using namespace logging;
using namespace argument_parser;

bool write_console = false;
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif

bool parse_arguments(const map<wstring, wstring>& arguments);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument::parse(argc, argv)))
	{
		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME, locale("ko_KR.UTF-8"));

	vector<thread> threads;
	for (unsigned short thread_index = 0; thread_index < 10; ++thread_index)
	{
		threads.push_back(
			thread([](const unsigned short& thread_index)
				{
					for (unsigned int log_index = 0; log_index < 1000; ++log_index)
					{
						auto start = logger::handle().chrono_start();
						logger::handle().write(logging_level::information, fmt::format(L"테스트_in_thread_{}: {}", thread_index, log_index), start);
					}
				}, thread_index)
		);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

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
		log_level = (logging_level)_wtoi(target->second.c_str());
	}

	return true;
}

void display_help(void)
{
	wcout << L"logging sample options:" << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}