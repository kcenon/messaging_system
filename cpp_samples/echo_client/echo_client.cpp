#include <iostream>
#include <string>
#include <stdlib.h>
#include <future>

#include "logging.h"
#include "converting.h"
#include "file_handler.h"
#include "argument_parser.h"
#include "messaging_client.h"

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "container.h"
#include "values/string_value.h"
#include "values/container_value.h"
#endif

#include "fmt/xchar.h"
#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"echo_client";

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
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif
wstring connection_key = L"echo_network";
wstring server_ip = L"127.0.0.1";
unsigned short server_port = 9876;
unsigned short high_priority_count = 1;
unsigned short normal_priority_count = 2;
unsigned short low_priority_count = 3;

#ifndef __USE_TYPE_CONTAINER__
map<wstring, function<void(shared_ptr<json::value>)>> _registered_messages;
#else
map<wstring, function<void(shared_ptr<container::value_container>)>> _registered_messages;
#endif

promise<bool> _promise_status;
future<bool> _future_status;
shared_ptr<messaging_client> _client = nullptr;

bool parse_arguments(const map<wstring, wstring>& arguments);
void display_help(void);

void create_client(void);
void send_echo_test_message(void);
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

	create_client();

	_future_status = _promise_status.get_future();

	_future_status.wait();

	_client->stop();

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

	target = arguments.find(L"--connection_key");
	if (target != arguments.end())
	{
		temp = converter::to_wstring(file::load(target->second));
		if (!temp.empty())
		{
			connection_key = temp;
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
	wcout << L"pathfinder connector options:" << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}

void create_client(void)
{
	if (_client != nullptr)
	{
		_client.reset();
	}

	_client = make_shared<messaging_client>(PROGRAM_NAME);
	_client->set_compress_mode(compress_mode);
	_client->set_connection_key(connection_key);
	_client->set_session_types(session_types::message_line);
	_client->set_connection_notification(&connection);
	_client->set_message_notification(&received_message);
	_client->start(server_ip, server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void send_echo_test_message(void)
{
#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

#ifdef _WIN32
	(*container)[HEADER][TARGET_ID] = json::value::string(L"main_server");
	(*container)[HEADER][TARGET_SUB_ID] = json::value::string(L"");
	(*container)[HEADER][MESSAGE_TYPE] = json::value::string(L"echo_test");
#else
	(*container)[HEADER][TARGET_ID] = json::value::string("main_server");
	(*container)[HEADER][TARGET_SUB_ID] = json::value::string("");
	(*container)[HEADER][MESSAGE_TYPE] = json::value::string("echo_test");
#endif
#else
	shared_ptr<container::value_container> container =
		make_shared<container::value_container>(L"main_server", L"", L"echo_test");
#endif

	_client->send(container);
}

void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging_level::information,
		fmt::format(L"a client on pathfinder: {}[{}] is {}", target_id, target_sub_id,
			condition ? L"connected" : L"disconnected"));

	if (condition)
	{
		send_echo_test_message();
	}
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
	auto message_type = _registered_messages.find((*container)[HEADER][MESSAGE_TYPE].as_string());
#else
	auto message_type = _registered_messages.find(converter::to_wstring((*container)[HEADER][MESSAGE_TYPE].as_string()));
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
	logger::handle().write(logging_level::sequence, fmt::format(L"unknown message: {}", container->serialize()));
#else
	logger::handle().write(logging_level::sequence, converter::to_wstring(fmt::format("unknown message: {}", container->serialize())));
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

	_promise_status.set_value(true);
}

void updated_backuplog(const wstring& file_path)
{
	system(converter::to_string(fmt::format(L"log_uploader --path {}", file_path)).c_str());
}