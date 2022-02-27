#include <iostream>

#include "logging.h"
#include "converting.h"
#include "compressing.h"
#include "messaging_server.h"
#include "messaging_client.h"
#include "argument_parser.h"

#ifdef __USE_TYPE_CONTAINER__
#include "container.h"
#include "value.h"
#include "values/bool_value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"
#include "values/container_value.h"
#endif

#ifdef _CONSOLE
#include <Windows.h>
#endif

#include <future>
#include <vector>
#include <signal.h>

#include "fmt/xchar.h"
#include "fmt/format.h"

#include "cpprest/json.h"
#include "cpprest/http_listener.h"

constexpr auto PROGRAM_NAME = L"restapi_gateway";

using namespace std;
using namespace logging;
using namespace network;
using namespace converting;
using namespace compressing;
using namespace argument_parser;

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

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
wstring connection_key = L"middle_connection_key";
wstring server_ip = L"127.0.0.1";
unsigned short server_port = 8642;
unsigned short rest_port = 7654;
unsigned short high_priority_count = 4;
unsigned short normal_priority_count = 4;
unsigned short low_priority_count = 4;

promise<bool> _promise_status;
future<bool> _future_status;

shared_ptr<messaging_client> _data_line = nullptr;
shared_ptr<http_listener> _http_listener = nullptr;

vector<json::value> _messages;

#ifdef __USE_TYPE_CONTAINER__
map<wstring, function<void(shared_ptr<container::value_container>)>> _registered_messages;
#else
map<wstring, function<void(shared_ptr<json::value>)>> _registered_messages;
#endif

#ifdef _CONSOLE
BOOL ctrl_handler(DWORD ctrl_type);
#endif

bool parse_arguments(const map<wstring, wstring>& arguments);
void create_data_line(void);
void create_http_listener(void);
void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition);

#ifdef __USE_TYPE_CONTAINER__
void received_message(shared_ptr<container::value_container> container);
void transfer_condition(shared_ptr<container::value_container> container);
#else
void received_message(shared_ptr<json::value> container);
void transfer_condition(shared_ptr<json::value> container);
#endif

void get_method(http_request request);
void post_method(http_request request);
void put_method(http_request request);
void del_method(http_request request);
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

	_registered_messages.insert({ L"transfer_condition", transfer_condition });

	create_data_line();
	create_http_listener();

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
			_promise_status.set_value(true);
			_http_listener.reset();

			_data_line.reset();

			logger::handle().stop();
		}
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

	target = arguments.find(L"--server_ip");
	if (target != arguments.end())
	{
		server_ip = target->second;
	}

	target = arguments.find(L"--server_port");
	if (target != arguments.end())
	{
		server_port = (unsigned short)_wtoi(target->second.c_str());
	}

	target = arguments.find(L"--rest_port");
	if (target != arguments.end())
	{
		rest_port = (unsigned short)_wtoi(target->second.c_str());
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

void create_data_line(void)
{
	if (_data_line != nullptr)
	{
		_data_line.reset();
	}

	_data_line = make_shared<messaging_client>(L"data_line");
	_data_line->set_compress_mode(compress_mode);
	_data_line->set_connection_key(connection_key);
	_data_line->set_session_types(session_types::message_line);
	_data_line->set_connection_notification(&connection);
	_data_line->set_message_notification(&received_message);
	_data_line->start(server_ip, server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void create_http_listener(void)
{
	_http_listener = make_shared<http_listener>(fmt::format(L"http://localhost:{}/rest", rest_port));
	_http_listener->support(methods::GET, get_method);
	_http_listener->support(methods::POST, post_method);
	_http_listener->support(methods::PUT, put_method);
	_http_listener->support(methods::DEL, del_method);
	_http_listener->open()
		.then([&]()
			{
				logger::handle().write(logging_level::information, L"starting to listen");
			})
		.wait();

	_future_status = _promise_status.get_future(); 
	_future_status.wait();
}

void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	if (_data_line == nullptr)
	{
		return;
	}

	logger::handle().write(logging_level::sequence,
		fmt::format(L"{} on middle server is {} from target: {}[{}]", _data_line->source_id(), condition ? L"connected" : L"disconnected", target_id, target_sub_id));

	if (condition)
	{
		return;
	}

	this_thread::sleep_for(chrono::seconds(1));

	_data_line->start(server_ip, server_port, high_priority_count, normal_priority_count, low_priority_count);
}

#ifdef __USE_TYPE_CONTAINER__
void received_message(shared_ptr<container::value_container> container)
#else
void received_message(shared_ptr<json::value> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifdef __USE_TYPE_CONTAINER__
	auto message_type = _registered_messages.find(container->message_type());
#else
	auto message_type = _registered_messages.find((*container)[L"header"][L"message_type"].as_string());
#endif
	if (message_type != _registered_messages.end())
	{
		message_type->second(container);

		return;
	}

	logger::handle().write(logging_level::sequence, fmt::format(L"unknown message: {}", container->serialize()));
}

#ifdef __USE_TYPE_CONTAINER__
void transfer_condition(shared_ptr<container::value_container> container)
#else
void transfer_condition(shared_ptr<json::value> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifdef __USE_TYPE_CONTAINER__
	if (container->message_type() != L"transfer_condition")
#else
	if ((*container)[L"header"][L"message_type"].as_string() != L"transfer_condition")
#endif
	{
		return;
	}

	auto transfer_condition = json::value::object();

#ifdef __USE_TYPE_CONTAINER__
	transfer_condition[L"message_type"] = json::value::string(container->message_type());
	transfer_condition[L"indication_id"] = json::value::string(container->get_value(L"indication_id")->to_string());
	transfer_condition[L"percentage"] = json::value::number(container->get_value(L"percentage")->to_ushort());
	transfer_condition[L"completed"] = json::value::boolean(container->get_value(L"completed")->to_boolean());
#else
	transfer_condition[L"message_type"] = (*container)[L"header"][L"message_type"];
	transfer_condition[L"indication_id"] = (*container)[L"data"][L"message_type"];
	transfer_condition[L"percentage"] = (*container)[L"data"][L"percentage"];
	transfer_condition[L"completed"] = (*container)[L"data"][L"completed"];
#endif

	_messages.push_back(transfer_condition);
}

void get_method(http_request request)
{
	logger::handle().write(logging_level::information, fmt::format(L"call get method: {}", request.extract_string().get()));

	auto answer = json::value::object();

	// do something

	request.reply(status_codes::OK, answer);
}

void post_method(http_request request)
{
	logger::handle().write(logging_level::information, fmt::format(L"call post method: {}", request.extract_string().get()));

	auto answer = json::value::object();

	// do something
	auto action = request.extract_json().get();
	if (action.is_null())
	{
		request.reply(status_codes::OK, answer);
		return;
	}

	/*
	vector<shared_ptr<container::value>> files;

	files.push_back(make_shared<container::string_value>(L"indication_id", L"download_test"));
	for (auto& source : sources)
	{
		files.push_back(make_shared<container::container_value>(L"file", vector<shared_ptr<container::value>> {
			make_shared<container::string_value>(L"source", source),
				make_shared<container::string_value>(L"target", converter::replace2(source, source_folder, target_folder))
		}));
	}

	shared_ptr<container::value_container> container =
		make_shared<container::value_container>(L"main_server", L"", L"download_files", files);
	_data_line->send(container);
	*/

	request.reply(status_codes::OK, answer);
}

void put_method(http_request request)
{
	logger::handle().write(logging_level::information, fmt::format(L"call put method: {}", request.extract_string().get()));

	auto answer = json::value::object();

	// do something

	request.reply(status_codes::OK, answer);
}

void del_method(http_request request)
{
	logger::handle().write(logging_level::information, fmt::format(L"call del method: {}", request.extract_string().get()));

	auto answer = json::value::object();

	// do something

	request.reply(status_codes::OK, answer);
}

void display_help(void)
{
	wcout << L"main server options:" << endl << endl;
	wcout << L"--encrypt_mode [value] " << endl;
	wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << endl << endl;
	wcout << L"--compress_mode [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << endl << endl;
	wcout << L"--compress_block_size [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to change compress block size must be appended '--compress_block_size size'.\n\tInitialize value is --compress_mode 1024." << endl << endl;
	wcout << L"--connection_key [value]" << endl;
	wcout << L"\tIf you want to change a specific key string for the connection to the middle server must be appended\n\t'--connection_key [specific key string]'." << endl << endl;
	wcout << L"--server_port [value]" << endl;
	wcout << L"\tIf you want to change a port number for the connection to the middle server must be appended\n\t'--server_port [port number]'." << endl << endl;
	wcout << L"--high_priority_count [value]" << endl;
	wcout << L"\tIf you want to change high priority thread workers must be appended '--high_priority_count [count]'." << endl << endl;
	wcout << L"--normal_priority_count [value]" << endl;
	wcout << L"\tIf you want to change normal priority thread workers must be appended '--normal_priority_count [count]'." << endl << endl;
	wcout << L"--low_priority_count [value]" << endl;
	wcout << L"\tIf you want to change low priority thread workers must be appended '--low_priority_count [count]'." << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}