#include "logging.h"

#include "argument_parser.h"

#include "container.h"
#include "values/bool_value.h"
#include "values/float_value.h"
#include "values/double_value.h"
#include "values/long_value.h"
#include "values/ulong_value.h"
#include "values/llong_value.h"
#include "values/ullong_value.h"
#include "values/container_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <limits.h>
#include <memory>
#include <iostream>

constexpr auto PROGRAM_NAME = L"container_sample";

using namespace logging;
using namespace container;
using namespace argument_parser;

bool write_console = false;
logging_level log_level = logging_level::information;

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument::parse(argc, argv)))
	{
		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	auto start = logger::handle().chrono_start();
	value_container data;
	data.add(bool_value(L"false_value", false));
	data.add(bool_value(L"true_value", true));
	data.add(float_value(L"float_value", (float)1.234567890123456789));
	data.add(double_value(L"double_value", (double)1.234567890123456789));
	logger::handle().write(logging_level::information, fmt::format(L"data serialize:\n{}", data.serialize()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data xml:\n{}", data.to_xml()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data json:\n{}", data.to_json()), start);

	start = logger::handle().chrono_start();
	value_container data2(data);
	data2.add(std::make_shared<long_value>(L"long_value", LONG_MAX));
	data2.add(std::make_shared<ulong_value>(L"ulong_value", ULONG_MAX));
	data2.add(std::make_shared<llong_value>(L"llong_value", LLONG_MAX));
	data2.add(std::make_shared<ullong_value>(L"ullong_value", ULLONG_MAX));
	data2.add(std::make_shared<container_value>(L"container_value", std::vector<std::shared_ptr<value>>
		{
			std::make_shared<long_value>(L"long_value", LONG_MAX),
			std::make_shared<ulong_value>(L"ulong_value", ULONG_MAX),
			std::make_shared<llong_value>(L"llong_value", LLONG_MAX),
			std::make_shared<ullong_value>(L"ullong_value", ULLONG_MAX)
		}));
	logger::handle().write(logging_level::information, fmt::format(L"data serialize:\n{}", data2.serialize()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data xml:\n{}", data2.to_xml()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data json:\n{}", data2.to_json()), start);

	start = logger::handle().chrono_start();
	value_container data3(data2);
	data3.remove(L"false_value");
	data3.remove(L"true_value");
	data3.remove(L"float_value");
	data3.remove(L"double_value");
	data3.remove(L"container_value");
	logger::handle().write(logging_level::information, fmt::format(L"data serialize:\n{}", data3.serialize()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data xml:\n{}", data3.to_xml()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data json:\n{}", data3.to_json()), start);

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
	std::wcout << L"container sample options:" << std::endl << std::endl;
	std::wcout << L"--write_console_mode [value] " << std::endl;
	std::wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << std::endl << std::endl;
	std::wcout << L"--logging_level [value]" << std::endl;
	std::wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << std::endl;
}