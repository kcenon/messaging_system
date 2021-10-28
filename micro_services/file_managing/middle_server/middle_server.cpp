#include <iostream>

#include "logging.h"
#include "messaging_server.h"
#include "messaging_client.h"
#include "compressing.h"
#include "file_manager.h"
#include "argument_parser.h"

#include "value.h"
#include "values/bool_value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"

#ifdef _CONSOLE
#include <Windows.h>
#endif

#include <signal.h>

#include "fmt/xchar.h"
#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"middle_server";

using namespace logging;
using namespace network;
using namespace compressing;
using namespace argument_parser;

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
std::wstring main_connection_key = L"main_connection_key";
std::wstring middle_connection_key = L"middle_connection_key";
unsigned short middle_server_port = 8642;
std::wstring main_server_ip = L"127.0.0.1";
unsigned short main_server_port = 9753;
unsigned short high_priority_count = 4;
unsigned short normal_priority_count = 4;
unsigned short low_priority_count = 4;
size_t session_limit_count = 0;

file_manager _file_manager;

std::map<std::wstring, std::function<bool(std::shared_ptr<container::value_container>)>> _file_commands;

std::shared_ptr<messaging_client> _data_line = nullptr;
std::shared_ptr<messaging_client> _file_line = nullptr;
std::shared_ptr<messaging_server> _middle_server = nullptr;

#ifdef _CONSOLE
BOOL ctrl_handler(DWORD ctrl_type);
#endif

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments);
void create_middle_server(void);
void create_data_line(void);
void create_file_line(void);
void connection_from_middle_server(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message_from_middle_server(std::shared_ptr<container::value_container> container);
void connection_from_data_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message_from_data_line(std::shared_ptr<container::value_container> container);
void connection_from_file_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message_from_file_line(std::shared_ptr<container::value_container> container);
void received_file_from_file_line(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path);
bool download_files(std::shared_ptr<container::value_container> container);
bool upload_files(std::shared_ptr<container::value_container> container);
void uploaded_file(std::shared_ptr<container::value_container> container);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument::parse(argc, argv)))
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

	_file_commands.insert({ L"download_files", &download_files });
	_file_commands.insert({ L"upload_files", &upload_files });

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	create_middle_server();
	create_data_line();
	create_file_line();

	_middle_server->wait_stop();

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
		{
			_data_line.reset();
			_file_line.reset();
			_middle_server.reset();

			logger::handle().stop();
		}
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

	target = arguments.find(L"--main_connection_key");
	if (target != arguments.end())
	{
		main_connection_key = target->second;
	}

	target = arguments.find(L"--middle_connection_key");
	if (target != arguments.end())
	{
		middle_connection_key = target->second;
	}

	target = arguments.find(L"--main_server_ip");
	if (target != arguments.end())
	{
		main_server_ip = target->second;
	}

	target = arguments.find(L"--main_server_port");
	if (target != arguments.end())
	{
		main_server_port = (unsigned short)_wtoi(target->second.c_str());
	}

	target = arguments.find(L"--middle_server_port");
	if (target != arguments.end())
	{
		middle_server_port = (unsigned short)_wtoi(target->second.c_str());
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

void create_middle_server(void)
{
	if (_middle_server != nullptr)
	{
		_middle_server.reset();
	}

	_middle_server = std::make_shared<messaging_server>(PROGRAM_NAME);
	_middle_server->set_encrypt_mode(encrypt_mode);
	_middle_server->set_compress_mode(compress_mode);
	_middle_server->set_connection_key(middle_connection_key);
	_middle_server->set_session_limit_count(session_limit_count);
	_middle_server->set_possible_session_types({ session_types::message_line });
	_middle_server->set_connection_notification(&connection_from_middle_server);
	_middle_server->set_message_notification(&received_message_from_middle_server);
	_middle_server->start(middle_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void create_data_line(void)
{
	if (_data_line != nullptr)
	{
		_data_line.reset();
	}

	_data_line = std::make_shared<messaging_client>(L"data_line");
	_data_line->set_compress_mode(compress_mode);
	_data_line->set_connection_key(main_connection_key);
	_data_line->set_session_types(session_types::message_line);
	_data_line->set_connection_notification(&connection_from_data_line);
	_data_line->set_message_notification(&received_message_from_data_line);
	_data_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void create_file_line(void)
{
	if (_file_line != nullptr)
	{
		_file_line.reset();
	}

	_file_line = std::make_shared<messaging_client>(L"file_line");
	_file_line->set_compress_mode(compress_mode);
	_file_line->set_connection_key(main_connection_key);
	_file_line->set_session_types(session_types::file_line);
	_file_line->set_connection_notification(&connection_from_file_line);
	_file_line->set_message_notification(&received_message_from_file_line);
	_file_line->set_file_notification(&received_file_from_file_line);
	_file_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void connection_from_middle_server(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"a client on middle server: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
}

void received_message_from_middle_server(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	auto target = _file_commands.find(container->message_type());
	if (target == _file_commands.end())
	{
		if (_data_line == nullptr || !_data_line->is_confirmed())
		{
			if (_middle_server)
			{
				std::shared_ptr<container::value_container> response = container->copy(false);
				response->swap_header();

				response << std::make_shared<container::bool_value>(L"error", true);
				response << std::make_shared<container::string_value>(L"reason", L"main_server has not been connected.");

				_middle_server->send(response);
			}

			return;
		}

		if (_data_line)
		{
			_data_line->send(container);
		}

		return;
	}

	if (_file_line == nullptr || !_file_line->is_confirmed())
	{
		if (_middle_server)
		{
			std::shared_ptr<container::value_container> response = container->copy(false);
			response->swap_header();

			response << std::make_shared<container::bool_value>(L"error", true);
			response << std::make_shared<container::string_value>(L"reason", L"main_server has not been connected.");

			_middle_server->send(response);
		}

		return;
	}

	target->second(container);
}

void connection_from_data_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	if (_data_line == nullptr)
	{
		return;
	}

	logger::handle().write(logging::logging_level::sequence,
		fmt::format(L"{} on middle server is {} from target: {}[{}]", _data_line->source_id(), condition ? L"connected" : L"disconnected", target_id, target_sub_id));

	if (condition)
	{
		return;
	}

	if (_middle_server == nullptr)
	{
		return;
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));

	_data_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void received_message_from_data_line(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	if (_middle_server)
	{
		_middle_server->send(container);
	}
}

void connection_from_file_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	if (_file_line == nullptr)
	{
		return;
	}

	logger::handle().write(logging::logging_level::sequence,
		fmt::format(L"{} on middle server is {} from target: {}[{}]", _file_line->source_id(), condition ? L"connected" : L"disconnected", target_id, target_sub_id));

	if (condition)
	{
		return;
	}

	if (_middle_server == nullptr)
	{
		return;
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));

	_file_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void received_message_from_file_line(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	if (container->message_type() == L"uploaded_file")
	{
		uploaded_file(container);

		return;
	}

	if (_middle_server)
	{
		_middle_server->send(container);
	}
}

void received_file_from_file_line(const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& indication_id, const std::wstring& target_path)
{
	logger::handle().write(logging::logging_level::parameter,
		fmt::format(L"target_id: {}, target_sub_id: {}, indication_id: {}, file_path: {}", target_id, target_sub_id, indication_id, target_path));

	std::shared_ptr<container::value_container> container = _file_manager.received(target_id, target_sub_id, indication_id, target_path);
	if(container != nullptr)
	{
		if (_middle_server)
		{
			_middle_server->send(container);
		}
	}
}

bool download_files(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return false;
	}

	std::vector<std::shared_ptr<container::value>> files = container->value_array(L"file");

	std::vector<std::wstring> target_paths;
	for (auto& file : files)
	{
		target_paths.push_back((*file)[L"target"]->to_string());
	}
	_file_manager.set(container->get_value(L"indication_id")->to_string(), target_paths);

	if (_middle_server)
	{
		_middle_server->send(std::make_shared<container::value_container>(container->source_id(), container->source_sub_id(), L"transfer_condition",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::string_value>(L"indication_id", container->get_value(L"indication_id")->to_string()),
				std::make_shared<container::ushort_value>(L"percentage", 0)
		}));
	}

	std::shared_ptr<container::value_container> temp = container->copy();
	temp->set_message_type(L"request_files");

	if (_file_line)
	{
		_file_line->send(temp);
	}

	return true;
}

bool upload_files(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return false;
	}

	std::vector<std::shared_ptr<container::value>> files = container->value_array(L"file");

	std::vector<std::wstring> target_paths;
	for (auto& file : files)
	{
		target_paths.push_back((*file)[L"target"]->to_string());
	}
	_file_manager.set(container->get_value(L"indication_id")->to_string(), target_paths);

	if (_middle_server)
	{
		_middle_server->send(std::make_shared<container::value_container>(container->source_id(), container->source_sub_id(), L"transfer_condition",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::string_value>(L"indication_id", container->get_value(L"indication_id")->to_string()),
				std::make_shared<container::ushort_value>(L"percentage", 0)
		}));
	}

	container->set_message_type(L"transfer_file");
	
	if (_file_line)
	{
		container << std::make_shared<container::string_value>(L"gateway_source_id", container->source_id());
		container << std::make_shared<container::string_value>(L"gateway_source_sub_id", container->source_sub_id());
		container->set_source(_file_line->source_id(), _file_line->source_sub_id());

		_file_line->send(container);
	}

	return true;
}

void uploaded_file(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	std::shared_ptr<container::value_container> temp = _file_manager.received(
		container->target_id(), container->target_sub_id(), container->get_value(L"indication_id")->to_string(), container->get_value(L"target_path")->to_string());
	if (temp != nullptr)
	{
		if (_middle_server)
		{
			_middle_server->send(temp);
		}
	}
}

void display_help(void)
{
	std::wcout << L"main server options:" << std::endl << std::endl;
	std::wcout << L"--encrypt_mode [value] " << std::endl;
	std::wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << std::endl << std::endl;
	std::wcout << L"--compress_mode [value]" << std::endl;
	std::wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << std::endl << std::endl;
	std::wcout << L"--compress_block_size [value]" << std::endl;
	std::wcout << L"\tThe compress_mode on/off. If you want to change compress block size must be appended '--compress_block_size size'.\n\tInitialize value is --compress_mode 1024." << std::endl << std::endl;
	std::wcout << L"--main_connection_key [value]" << std::endl;
	std::wcout << L"\tIf you want to change a specific key string for the connection to the main server must be appended\n\t'--main_connection_key [specific key string]'." << std::endl << std::endl;
	std::wcout << L"--middle_connection_key [value]" << std::endl;
	std::wcout << L"\tIf you want to change a specific key string for the connection to the middle server must be appended\n\t'--middle_connection_key [specific key string]'." << std::endl << std::endl;
	std::wcout << L"--main_server_port [value]" << std::endl;
	std::wcout << L"\tIf you want to change a port number for the connection to the main server must be appended\n\t'--main_server_port [port number]'." << std::endl << std::endl;
	std::wcout << L"--middle_server_port [value]" << std::endl;
	std::wcout << L"\tIf you want to change a port number for the connection to the middle server must be appended\n\t'--middle_server_port [port number]'." << std::endl << std::endl;
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