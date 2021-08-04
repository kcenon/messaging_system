#include <iostream>

#include "logging.h"
#include "messaging_server.h"
#include "compressing.h"
#include "argument_parsing.h"

#include "value.h"
#include "values/string_value.h"

#include <wchar.h>
#include <algorithm>
#include <signal.h>

#ifdef _CONSOLE
#include <Windows.h>
#endif

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"main_server";

using namespace logging;
using namespace network;
using namespace compressing;
using namespace argument_parsing;

#ifdef _DEBUG
bool write_console = true;
#else
bool write_console = false;
#endif
bool encrypt_mode = false;
bool compress_mode = false;
unsigned short compress_block_size = 1024;
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif
std::wstring connection_key = L"main_connection_key";
unsigned short server_port = 9753;
unsigned short high_priority_count = 4;
unsigned short normal_priority_count = 4;
unsigned short low_priority_count = 4;
size_t session_limit_count = 0;

std::shared_ptr<messaging_server> _main_server = nullptr;

#ifdef _CONSOLE
BOOL ctrl_handler(DWORD ctrl_type);
#endif

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments);
void create_main_server(void);
void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message(std::shared_ptr<container::value_container> container);
void received_file(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument_parser::parse(argc, argv)))
	{
		return 0;
	}

#ifdef _CONSOLE
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE);
#endif

	if (compress_mode)
	{
		compressor::set_block_bytes(compress_block_size);
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	create_main_server();

	_main_server->wait_stop();

	logger::handle().stop();

	return 0;
}

#ifdef _CONSOLE
BOOL ctrl_handler(DWORD ctrl_type)
{
	switch (ctrl_type)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_BREAK_EVENT:
		_main_server.reset();

		logger::handle().stop();
		break;
	}

	return FALSE;
}
#endif

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments)
{
	std::wstring temp;

	auto target = arguments.find(L"--help");
	if (target != arguments.end())
	{
		display_help();

		return false;
	}

	target = arguments.find(L"--encrypt_mode");
	if (target != arguments.end())
	{
		temp = target->second;
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true") == 0)
		{
			encrypt_mode = true;
		}
		else
		{
			encrypt_mode = false;
		}
	}

	target = arguments.find(L"--compress_mode");
	if (target != arguments.end())
	{
		temp = target->second;
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true") == 0)
		{
			compress_mode = true;
		}
		else
		{
			compress_mode = false;
		}
	}

	target = arguments.find(L"--compress_block_size");
	if (target != arguments.end())
	{
		compress_block_size = (unsigned short)_wtoi(target->second.c_str());
	}

	target = arguments.find(L"--connection_key");
	if (target != arguments.end())
	{
		connection_key = target->second;
	}

	target = arguments.find(L"--server_port");
	if (target != arguments.end())
	{
		server_port = (unsigned short)_wtoi(target->second.c_str());
	}

	target = arguments.find(L"--high_priority_count");
	if (target != arguments.end())
	{
		high_priority_count = (unsigned short)_wtoi(target->second.c_str());
	}

	target = arguments.find(L"--normal_priority_count");
	if (target != arguments.end())
	{
		normal_priority_count = (unsigned short)_wtoi(target->second.c_str());
	}

	target = arguments.find(L"--low_priority_count");
	if (target != arguments.end())
	{
		low_priority_count = (unsigned short)_wtoi(target->second.c_str());
	}

	target = arguments.find(L"--session_limit_count");
	if (target != arguments.end())
	{
		session_limit_count = (unsigned short)_wtoi(target->second.c_str());
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

void create_main_server(void)
{
	if (_main_server != nullptr)
	{
		_main_server.reset();
	}

	_main_server = std::make_shared<messaging_server>(PROGRAM_NAME);
	_main_server->set_encrypt_mode(encrypt_mode);
	_main_server->set_compress_mode(compress_mode);
	_main_server->set_connection_key(connection_key);
	_main_server->set_session_limit_count(session_limit_count);
	_main_server->set_possible_session_types({ session_types::message_line, session_types::file_line });
	_main_server->set_connection_notification(&connection);
	_main_server->set_message_notification(&received_message);
	_main_server->set_file_notification(&received_file);
	_main_server->start(server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"a client on main server: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
}

void received_message(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	if (container->message_type() == L"transfer_file")
	{
		if (_main_server != nullptr)
		{
			_main_server->send_files(container);
		}

		return;
	}

	logger::handle().write(logging::logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
}

void received_file(const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& indication_id, const std::wstring& target_path)
{
	if (_main_server != nullptr)
	{
		_main_server->send(std::make_shared<container::value_container>(target_id, target_sub_id, L"uploaded_file",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::string_value>(L"indication_id", indication_id),
				std::make_shared<container::string_value>(L"target_path", target_path)
		}));
	}
}

void display_help(void)
{
	std::wcout << L"Options:" << std::endl << std::endl;
	std::wcout << L"--encrypt_mode [value] " << std::endl;
	std::wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << std::endl << std::endl;
	std::wcout << L"--compress_mode [value]" << std::endl;
	std::wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << std::endl << std::endl;
	std::wcout << L"--compress_block_size [value]" << std::endl;
	std::wcout << L"\tThe compress_mode on/off. If you want to change compress block size must be appended '--compress_block_size size'.\n\tInitialize value is --compress_mode 1024." << std::endl << std::endl;
	std::wcout << L"--connection_key [value]" << std::endl;
	std::wcout << L"\tIf you want to change a specific key string for the connection to the main server must be appended\n\t'--connection_key [specific key string]'." << std::endl << std::endl;
	std::wcout << L"--server_port [value]" << std::endl;
	std::wcout << L"\tIf you want to change a port number for the connection to the main server must be appended\n\t'--server_port [port number]'." << std::endl << std::endl;
	std::wcout << L"--high_priority_count [value]" << std::endl;
	std::wcout << L"\tIf you want to change high priority thread workers must be appended '--high_priority_count [count]'." << std::endl << std::endl;
	std::wcout << L"--normal_priority_count [value]" << std::endl;
	std::wcout << L"\tIf you want to change normal priority thread workers must be appended '--normal_priority_count [count]'." << std::endl << std::endl;
	std::wcout << L"--low_priority_count [value]" << std::endl;
	std::wcout << L"\tIf you want to change low priority thread workers must be appended '--low_priority_count [count]'." << std::endl << std::endl;
	std::wcout << L"--session_limit_count [value]" << std::endl;
	std::wcout << L"\tIf you want to change session limit count must be appended '--session_limit_count [count]'." << std::endl << std::endl;
	std::wcout << L"--write_console_mode [value] " << std::endl;
	std::wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << std::endl << std::endl;
	std::wcout << L"--logging_level [value]" << std::endl;
	std::wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << std::endl;
}