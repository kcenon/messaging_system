#include <iostream>

#include "logging.h"
#include "messaging_server.h"
#include "compressing.h"
#include "file_manager.h"
#include "argument_parser.h"

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"
#endif

#include <wchar.h>
#include <algorithm>
#include <signal.h>

#ifdef _CONSOLE
#include <Windows.h>
#endif

#include "fmt/xchar.h"
#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"main_server";

using namespace std;
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
logging_level log_level = logging_level::packet;
#else
logging_level log_level = logging_level::information;
#endif
wstring connection_key = L"main_connection_key";
unsigned short server_port = 9753;
unsigned short high_priority_count = 4;
unsigned short normal_priority_count = 4;
unsigned short low_priority_count = 4;
size_t session_limit_count = 0;

shared_ptr<file_manager> _file_manager = nullptr;
shared_ptr<messaging_server> _main_server = nullptr;

#ifdef _CONSOLE
BOOL ctrl_handler(DWORD ctrl_type);
#endif

#ifndef __USE_TYPE_CONTAINER__
map<wstring, function<void(shared_ptr<json::value>)>> _registered_messages;
#else
map<wstring, function<void(shared_ptr<container::value_container>)>> _registered_messages;
#endif

bool parse_arguments(const map<wstring, wstring>& arguments);
void create_main_server(void);
void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition);

#ifndef __USE_TYPE_CONTAINER__
void received_message(shared_ptr<json::value> container);
void transfer_file(shared_ptr<json::value> container);
void upload_files(shared_ptr<json::value> container);
#else
void received_message(shared_ptr<container::value_container> container);
void transfer_file(shared_ptr<container::value_container> container);
void upload_files(shared_ptr<container::value_container> container);
#endif

void received_file(const wstring& source_id, const wstring& source_sub_id, const wstring& indication_id, const wstring& target_path);
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

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	_registered_messages.insert({ L"transfer_file", &transfer_file });
	_registered_messages.insert({ L"upload_files", &upload_files });

	_file_manager = make_shared<file_manager>();

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

bool parse_arguments(const map<wstring, wstring>& arguments)
{
	wstring temp;

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
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

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
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

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

void create_main_server(void)
{
	if (_main_server != nullptr)
	{
		_main_server.reset();
	}

	_main_server = make_shared<messaging_server>(PROGRAM_NAME);
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

void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging_level::information,
		fmt::format(L"a client on main server: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
}

#ifndef __USE_TYPE_CONTAINER__
void received_message(shared_ptr<json::value> container)
#else
void received_message(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifndef __USE_TYPE_CONTAINER__
	auto message_type = _registered_messages.find((*container)[L"header"][L"message_type"].as_string());
#else
	auto message_type = _registered_messages.find(container->message_type());
#endif
	if (message_type != _registered_messages.end())
	{
		message_type->second(container);

		return;
	}

	logger::handle().write(logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
}

#ifndef __USE_TYPE_CONTAINER__
void transfer_file(shared_ptr<json::value> container)
#else
void transfer_file(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifndef __USE_TYPE_CONTAINER__
	if ((*container)[L"header"][L"message_type"].as_string() != L"transfer_file")
#else
	if (container->message_type() != L"transfer_file")
#endif
	{
		return;
	}

	logger::handle().write(logging_level::information, L"received message: transfer_file");

	if (_main_server != nullptr)
	{
		_main_server->send_files(container);
	}
}

#ifndef __USE_TYPE_CONTAINER__
void upload_files(shared_ptr<json::value> container)
#else
void upload_files(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifndef __USE_TYPE_CONTAINER__
	if ((*container)[L"header"][L"message_type"].as_string() != L"upload_files")
#else
	if (container->message_type() != L"upload_files")
#endif
	{
		return;
	}

	vector<wstring> target_paths;

#ifndef __USE_TYPE_CONTAINER__
	auto& files = (*container)[L"data"][L"files"].as_array();
	for (int index = 0; index < files.size(); ++index)
	{
		target_paths.push_back(files[index][L"target"].as_string());
	}

	_file_manager->set((*container)[L"data"][L"indication_id"].as_string(),
		(*container)[L"data"][L"gateway_source_id"].as_string(),
		(*container)[L"data"][L"gateway_source_sub_id"].as_string(), target_paths);
#else
	vector<shared_ptr<container::value>> files = container->value_array(L"file");
	for (auto& file : files)
	{
		target_paths.push_back((*file)[L"target"]->to_string());
	}

	_file_manager->set(container->get_value(L"indication_id")->to_string(),
		container->get_value(L"gateway_source_id")->to_string(),
		container->get_value(L"gateway_source_sub_id")->to_string(), target_paths);
#endif

	if (_main_server)
	{
#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> start_message = make_shared<json::value>(json::value::object(true));

		(*start_message)[L"header"][L"source_id"] = json::value::string(L"");
		(*start_message)[L"header"][L"source_sub_id"] = json::value::string(L"");
		(*start_message)[L"header"][L"target_id"] = (*container)[L"data"][L"gateway_source_id"];
		(*start_message)[L"header"][L"target_sub_id"] = (*container)[L"data"][L"gateway_source_sub_id"];
		(*start_message)[L"header"][L"message_type"] = json::value::string(L"transfer_condition");

		(*start_message)[L"data"][L"indication_id"] = (*container)[L"data"][L"indication_id"];
		(*start_message)[L"data"][L"percentage"] = json::value::number(0);

		_main_server->send(start_message, session_types::file_line);
#else
		_main_server->send(make_shared<container::value_container>(
			container->get_value(L"gateway_source_id")->to_string(), 
			container->get_value(L"gateway_source_sub_id")->to_string(),
			L"transfer_condition", 
			vector<shared_ptr<container::value>> {
				make_shared<container::string_value>(L"indication_id", container->get_value(L"indication_id")->to_string()),
				make_shared<container::ushort_value>(L"percentage", 0)
		}), session_types::file_line);
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> temp = make_shared<json::value>(json::value::parse(container->serialize()));
	
	(*temp)[L"header"][L"source_id"] = (*container)[L"header"][L"target_id"];
	(*temp)[L"header"][L"source_sub_id"] = (*container)[L"header"][L"target_sub_id"];
	(*temp)[L"header"][L"target_id"] = (*container)[L"header"][L"source_id"];
	(*temp)[L"header"][L"target_sub_id"] = (*container)[L"header"][L"source_sub_id"];

	(*temp)[L"header"][L"message_type"] = json::value::string(L"request_files");
#else
	shared_ptr<container::value_container> temp = container->copy();
	temp->swap_header();

	temp->set_message_type(L"request_files");
#endif

	if (_main_server)
	{
		_main_server->send(temp, session_types::file_line);
	}
}

void received_file(const wstring& target_id, const wstring& target_sub_id, const wstring& indication_id, const wstring& target_path)
{
	logger::handle().write(logging_level::parameter,
		fmt::format(L"target_id: {}, target_sub_id: {}, indication_id: {}, file_path: {}", target_id, target_sub_id, indication_id, target_path));

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> container = _file_manager->received(indication_id, target_path);
#else
	shared_ptr<container::value_container> container = _file_manager->received(indication_id, target_path);
#endif

	if (container != nullptr)
	{
		if (_main_server)
		{
			_main_server->send(container, session_types::file_line);
		}
	}
}

void display_help(void)
{
	wcout << L"Options:" << endl << endl;
	wcout << L"--encrypt_mode [value] " << endl;
	wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << endl << endl;
	wcout << L"--compress_mode [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << endl << endl;
	wcout << L"--compress_block_size [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to change compress block size must be appended '--compress_block_size size'.\n\tInitialize value is --compress_mode 1024." << endl << endl;
	wcout << L"--connection_key [value]" << endl;
	wcout << L"\tIf you want to change a specific key string for the connection to the main server must be appended\n\t'--connection_key [specific key string]'." << endl << endl;
	wcout << L"--server_port [value]" << endl;
	wcout << L"\tIf you want to change a port number for the connection to the main server must be appended\n\t'--server_port [port number]'." << endl << endl;
	wcout << L"--high_priority_count [value]" << endl;
	wcout << L"\tIf you want to change high priority thread workers must be appended '--high_priority_count [count]'." << endl << endl;
	wcout << L"--normal_priority_count [value]" << endl;
	wcout << L"\tIf you want to change normal priority thread workers must be appended '--normal_priority_count [count]'." << endl << endl;
	wcout << L"--low_priority_count [value]" << endl;
	wcout << L"\tIf you want to change low priority thread workers must be appended '--low_priority_count [count]'." << endl << endl;
	wcout << L"--session_limit_count [value]" << endl;
	wcout << L"\tIf you want to change session limit count must be appended '--session_limit_count [count]'." << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}