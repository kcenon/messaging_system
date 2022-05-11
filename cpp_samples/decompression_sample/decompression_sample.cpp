#include <iostream>
#include <string>

#include "logging.h"
#include "converting.h"
#include "compressing.h"
#include "argument_parser.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"decompression_sample";

using namespace std;
using namespace logging;
using namespace converting;
using namespace compressing;
using namespace argument_parser;

#ifdef _DEBUG
bool write_console = true;
#else
bool write_console = false;
#endif

unsigned short compress_block_size = 1024;

#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif

wstring dump_file_path = L"file.dump";
wstring target_folder = L"";
wstring log_extention = L"log";
wstring log_root_path = L"";
bool decompression_mode = false;

#ifdef _WIN32
string locale_string = "ko_KR.UTF-8";
#else
string locale_string = "";
#endif

bool parse_arguments(const map<wstring, wstring>& arguments);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument::parse(argc, argv)))
	{
		return 0;
	}

	if (!decompression_mode)
	{
		display_help();

		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME, locale(locale_string), log_extention, log_root_path);

	if (compressor::decompression_folder(dump_file_path, target_folder, compress_block_size))
	{
		logger::handle().stop();

		return 0;
	}

	logger::handle().write(logging_level::error,
		fmt::format(L"cannot decompress dump: {}", dump_file_path));

	logger::handle().stop();

	return 0;

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

	target = arguments.find(L"--compress_block_size");
	if (target != arguments.end())
	{
		compress_block_size = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--dump_file_path");
	if (target != arguments.end())
	{
		dump_file_path = target->second;
	}

	target = arguments.find(L"--target_folder");
	if (target != arguments.end())
	{
		target_folder = target->second;
	}

	target = arguments.find(L"--decompression_mode");
	if (target != arguments.end())
	{
		temp = target->second;
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true") == 0)
		{
			decompression_mode = true;
		}
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
	}

	target = arguments.find(L"--logging_level");
	if (target != arguments.end())
	{
		log_level = (logging_level)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--logging_root_path");
	if (target != arguments.end())
	{
		log_root_path = target->second;
	}

	return true;
}

void display_help(void)
{
	wcout << L"compressor options:" << endl << endl;
	wcout << L"--compress_block_size [value]" << endl;
	wcout << L"\tIf you want to change compress block size must be appended '--compress_block_size size'.\n\tInitialize value is --compress_mode 1024." << endl << endl;
	wcout << L"--dump_file_path [value]" << endl;
	wcout << L"\tIf you want to use specific dump file must be appended '--dump_file_path [file path]'." << endl << endl;
	wcout << L"--target_folder [value]" << endl;
	wcout << L"\tIf you want to change target folder path must be appended '--target_folder [folder path]'." << endl << endl;
	wcout << L"--decompression_mode [value] " << endl;
	wcout << L"\tIf you want to decompress file must be appended '--decompression_mode true'.\n\tInitialize value is --decompression_mode false." << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tIf you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode false." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
	wcout << L"--logging_root_path [value]" << endl;
	wcout << L"\tIf you want to change logging root path must be appended\n\t'--logging_root_path [root path]'." << endl << endl;
}