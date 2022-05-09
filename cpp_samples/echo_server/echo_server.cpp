#include <iostream>
#include <string>
#include <stdlib.h>

#include "logging.h"
#include "converting.h"
#include "file_handler.h"
#include "argument_parser.h"
#include "messaging_server.h"

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "container.h"
#include "values/string_value.h"
#include "values/container_value.h"
#endif

#include "fmt/xchar.h"
#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"echo_server";

using namespace std;
using namespace logging;
using namespace network;
using namespace converting;
using namespace file_handler;
using namespace argument_parser;

#ifdef _DEBUG
bool write_console = true;
#else
bool write_console = false;
#endif
bool encrypt_mode = false;
bool compress_mode = true;
unsigned short compress_block_size = 1024;
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif
wstring connection_key = L"echo_network";
unsigned short server_port = 9876;
unsigned short high_priority_count = 4;
unsigned short normal_priority_count = 4;
unsigned short low_priority_count = 4;
size_t session_limit_count = 0;

#ifndef __USE_TYPE_CONTAINER__
map<wstring, function<void(shared_ptr<json::value>)>> _registered_messages;
#else
map<wstring, function<void(shared_ptr<container::value_container>)>> _registered_messages;
#endif

shared_ptr<messaging_server> _server = nullptr;

bool parse_arguments(const map<wstring, wstring>& arguments);
void display_help(void);

void create_server(void);
void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition);
#ifndef __USE_TYPE_CONTAINER__
void received_message(shared_ptr<json::value> container);
void received_echo_test(shared_ptr<json::value> container);
#else
void received_message(shared_ptr<container::value_container> container);
void received_echo_test(shared_ptr<container::value_container> container);
#endif

void updated_backuplog(const wstring& file_path);

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

	_registered_messages.insert({ L"echo_test", received_echo_test });

	create_server();

	_server->wait_stop();

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
		compress_block_size = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--connection_key");
	if (target != arguments.end())
	{
		temp = converter::to_wstring(file::load(target->second));
		if (!temp.empty())
		{
			connection_key = temp;
		}
	}

	target = arguments.find(L"--server_port");
	if (target != arguments.end())
	{
		server_port = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--high_priority_count");
	if (target != arguments.end())
	{
		high_priority_count = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--normal_priority_count");
	if (target != arguments.end())
	{
		normal_priority_count = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--low_priority_count");
	if (target != arguments.end())
	{
		low_priority_count = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--session_limit_count");
	if (target != arguments.end())
	{
		session_limit_count = (unsigned short)atoi(converter::to_string(target->second).c_str());
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
	wcout << L"pathfinder options:" << endl << endl;
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

void create_server(void)
{
	if (_server != nullptr)
	{
		_server.reset();
	}

	_server = make_shared<messaging_server>(PROGRAM_NAME);
	_server->set_encrypt_mode(encrypt_mode);
	_server->set_compress_mode(compress_mode);
	_server->set_connection_key(connection_key);
	_server->set_session_limit_count(session_limit_count);
	_server->set_possible_session_types({ session_types::message_line });
	_server->set_connection_notification(&connection);
	_server->set_message_notification(&received_message);
	_server->start(server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging_level::information,
		fmt::format(L"a client on pathfinder: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
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
#ifdef _WIN32
	auto message_type = _registered_messages.find((*container)[L"header"][L"message_type"].as_string());
#else
	auto message_type = _registered_messages.find(converter::to_wstring((*container)["header"]["message_type"].as_string()));
#endif
#else
	auto message_type = _registered_messages.find(container->message_type());
#endif
	if (message_type != _registered_messages.end())
	{
		message_type->second(container);

		return;
	}

#ifdef _WIN32
	logger::handle().write(logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
#else
	logger::handle().write(logging_level::information,
		converter::to_wstring(fmt::format("received message: {}", container->serialize())));
#endif
}

#ifndef __USE_TYPE_CONTAINER__
void received_echo_test(shared_ptr<json::value> container)
#else
void received_echo_test(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifdef _WIN32
	logger::handle().write(logging_level::information, fmt::format(L"received message: {}", container->serialize()));
#else
	logger::handle().write(logging_level::information, converter::to_wstring(fmt::format("received message: {}", container->serialize())));
#endif

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> message = make_shared<json::value>(json::value::object(true));
#ifdef _WIN32
	(*message)[L"header"][L"source_id"] = (*container)[L"header"][L"target_id"];
	(*message)[L"header"][L"source_sub_id"] = (*container)[L"header"][L"target_sub_id"];
	(*message)[L"header"][L"target_id"] = (*container)[L"header"][L"source_id"];
	(*message)[L"header"][L"target_sub_id"] = (*container)[L"header"][L"source_sub_id"];
	(*message)[L"header"][L"message_type"] = (*container)[L"header"][L"message_type"];
#else
	(*message)["header"]["source_id"] = (*container)["header"]["target_id"];
	(*message)["header"]["source_sub_id"] = (*container)["header"]["target_sub_id"];
	(*message)["header"]["target_id"] = (*container)["header"]["source_id"];
	(*message)["header"]["target_sub_id"] = (*container)["header"]["source_sub_id"];
	(*message)["header"]["message_type"] = (*container)["header"]["message_type"];
#endif
#else
	shared_ptr<container::value_container> message = container->copy(false);
	container->swap_header();
#endif
	_server->send(message);
}

void updated_backuplog(const wstring& file_path)
{
	system(converter::to_string(fmt::format(L"log_uploader --path {}", file_path)).c_str());
}