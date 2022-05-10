#include <iostream>

#include "logging.h"
#include "converting.h"
#include "compressing.h"
#include "file_handler.h"
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
using namespace file_handler;
using namespace argument_parser;

#ifdef __USE_TYPE_CONTAINER__
using namespace container;
#endif

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

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

map<wstring, vector<shared_ptr<json::value>>> _messages;

#ifdef __USE_TYPE_CONTAINER__
map<wstring, function<void(shared_ptr<container::value_container>)>> _registered_messages;
#else
map<wstring, function<void(shared_ptr<json::value>)>> _registered_messages;
#endif

map<wstring, function<void(shared_ptr<json::value>)>> _registered_restapi;

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

void transfer_files(shared_ptr<json::value> request);

void get_method(http_request request);
void post_method(http_request request);
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

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	_registered_messages.insert({ L"transfer_condition", transfer_condition });

	_registered_restapi.insert({ L"upload_files", transfer_files });
	_registered_restapi.insert({ L"download_files", transfer_files });

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

	target = arguments.find(L"--server_ip");
	if (target != arguments.end())
	{
		server_ip = target->second;
	}

	target = arguments.find(L"--server_port");
	if (target != arguments.end())
	{
		server_port = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--rest_port");
	if (target != arguments.end())
	{
		rest_port = (unsigned short)atoi(converter::to_string(target->second).c_str());
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
#ifdef _WIN32
	_http_listener = make_shared<http_listener>(fmt::format(L"http://localhost:{}/restapi", rest_port));
#else
	_http_listener = make_shared<http_listener>(fmt::format("http://localhost:{}/restapi", rest_port));
#endif
	_http_listener->support(methods::GET, get_method);
	_http_listener->support(methods::POST, post_method);
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
		fmt::format(L"{} on middle server is {} from target: {}[{}]", 
			_data_line->source_id(), condition ? L"connected" : L"disconnected", target_id, target_sub_id));

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
#ifdef _WIN32
	auto message_type = _registered_messages.find((*container)[HEADER][MESSAGE_TYPE].as_string());
#else
	auto message_type = _registered_messages.find(converter::to_wstring((*container)[HEADER][MESSAGE_TYPE].as_string()));
#endif
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
	if (container->message_type() != TRANSFER_CONDITON)
#else
#ifdef _WIN32
	if ((*container)[HEADER][MESSAGE_TYPE].as_string() != TRANSFER_CONDITON)
#else
	if ((*container)[HEADER][MESSAGE_TYPE].as_string() != TRANSFER_CONDITON)
#endif
#endif
	{
		return;
	}

	wstring indication_id = L"";
	shared_ptr<json::value> condition = make_shared<json::value>(json::value::object(true));

#ifdef __USE_TYPE_CONTAINER__
	indication_id = container->get_value(INDICATION_ID)->to_string();

	(*condition)[MESSAGE_TYPE] = json::value::string(container->message_type());
	(*condition)[INDICATION_ID] = json::value::string(indication_id);
	(*condition)[L"percentage"] = json::value::number(container->get_value(L"percentage")->to_ushort());
	(*condition)[L"completed"] = json::value::boolean(container->get_value(L"completed")->to_boolean());
#else
#ifdef _WIN32
	indication_id = (*container)[DATA][INDICATION_ID].as_string();

	(*condition)[MESSAGE_TYPE] = (*container)[HEADER][MESSAGE_TYPE];
	(*condition)[INDICATION_ID] = (*container)[DATA][INDICATION_ID];
	(*condition)[L"percentage"] = (*container)[DATA][L"percentage"];
	(*condition)[L"completed"] = (*container)[DATA][L"completed"].is_null() ?
		json::value::boolean(false) : (*container)[DATA][L"completed"];
#else
	indication_id = converter::to_wstring((*container)[DATA][INDICATION_ID].as_string());

	(*condition)[MESSAGE_TYPE] = (*container)[HEADER][MESSAGE_TYPE];
	(*condition)[INDICATION_ID] = (*container)[DATA][INDICATION_ID];
	(*condition)["percentage"] = (*container)[DATA]["percentage"];
	(*condition)["completed"] = (*container)[DATA]["completed"].is_null() ?
		json::value::boolean(false) : (*container)[DATA]["completed"];
#endif
#endif
	auto indication = _messages.find(indication_id);
	if (indication == _messages.end())
	{
		_messages.insert({ indication_id, { condition } });
		return;
	}

	indication->second.push_back(condition);
}

void transfer_files(shared_ptr<json::value> request)
{
#ifdef _WIN32
	auto& file_array = (*request)[FILES].as_array();
#else
	auto& file_array = (*request)[FILES].as_array();
#endif

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

#ifdef _WIN32
	(*container)[HEADER][TARGET_ID] = json::value::string(L"main_server");
	(*container)[HEADER][TARGET_SUB_ID] = json::value::string(L"");
	(*container)[HEADER][MESSAGE_TYPE] = (*request)[MESSAGE_TYPE];

	(*container)[DATA][INDICATION_ID] = (*request)[INDICATION_ID];

	int index = 0;
	(*container)[DATA][FILES] = json::value::array();
	for (auto& file : file_array)
	{
		(*container)[DATA][FILES][index][SOURCE] = file[SOURCE];
		(*container)[DATA][FILES][index][TARGET] = file[TARGET];
		index++;
	}
#else
	(*container)[HEADER][TARGET_ID] = json::value::string("main_server");
	(*container)[HEADER][TARGET_SUB_ID] = json::value::string("");
	(*container)[HEADER][MESSAGE_TYPE] = (*request)[MESSAGE_TYPE];

	(*container)[DATA][INDICATION_ID] = (*request)[INDICATION_ID];

	int index = 0;
	(*container)[DATA][FILES] = json::value::array();
	for (auto& file : file_array)
	{
		(*container)[DATA][FILES][index][SOURCE] = file[SOURCE];
		(*container)[DATA][FILES][index][TARGET] = file[TARGET];
		index++;
	}
#endif
#else
	vector<shared_ptr<container::value>> files;

	files.push_back(make_shared<container::string_value>(INDICATION_ID, (*request)[INDICATION_ID].as_string()));
	for (auto& file : file_array)
	{
		files.push_back(make_shared<container::container_value>(L"file", vector<shared_ptr<container::value>> {
			make_shared<container::string_value>(SOURCE, file[SOURCE].as_string()),
			make_shared<container::string_value>(TARGET, file[TARGET].as_string())
		}));
	}

	shared_ptr<container::value_container> container =
		make_shared<container::value_container>(L"main_server", L"", (*request)[MESSAGE_TYPE].as_string(), files);
#endif

	_data_line->send(container);
}

void get_method(http_request request)
{
	if (request.headers().empty())
	{
		request.reply(status_codes::NotAcceptable);
		return;
	}

#ifdef _WIN32
	auto indication = _messages.find(request.headers()[INDICATION_ID]);
#else
	auto indication = _messages.find(converter::to_wstring(request.headers()[INDICATION_ID]));
#endif
	if (indication == _messages.end())
	{
		request.reply(status_codes::NotAcceptable);
		return;
	}

	// do something
	vector<shared_ptr<json::value>> messages;

#ifdef _WIN32
	if (request.headers()[L"previous_message"] == L"clear")
#else
	if (request.headers()["previous_message"] == "clear")
#endif
	{
		messages.swap(indication->second);
	}
	else
	{
		messages = indication->second;
	}

	if (messages.empty())
	{
		request.reply(status_codes::NoContent);
		return;
	}

	json::value answer = json::value::object(true);

#ifdef _WIN32
	answer[L"messages"] = json::value::array();
#else
	answer["messages"] = json::value::array();
#endif

	int index = 0;
	for (auto& message : messages)
	{
#ifdef _WIN32
		answer[L"messages"][index][MESSAGE_TYPE] = (*message)[MESSAGE_TYPE];
		answer[L"messages"][index][INDICATION_ID] = (*message)[INDICATION_ID];
		answer[L"messages"][index][L"percentage"] = (*message)[L"percentage"];
		answer[L"messages"][index][L"completed"] = (*message)[L"completed"];
#else
		answer["messages"][index][MESSAGE_TYPE] = (*message)[MESSAGE_TYPE];
		answer["messages"][index][INDICATION_ID] = (*message)[INDICATION_ID];
		answer["messages"][index]["percentage"] = (*message)["percentage"];
		answer["messages"][index]["completed"] = (*message)["completed"];
#endif

		index++;
	}

	request.reply(status_codes::OK, answer);
}

void post_method(http_request request)
{
	auto action = request.extract_json().get();
	if (action.is_null())
	{
		request.reply(status_codes::NoContent);
		return;
	}

#ifdef _WIN32
	logger::handle().write(logging_level::packet, fmt::format(L"post method: {}", action.serialize()));
#else
	logger::handle().write(logging_level::packet, converter::to_wstring(fmt::format("post method: {}", action.serialize())));
#endif

#ifdef _WIN32
	auto message_type = _registered_restapi.find(action[MESSAGE_TYPE].as_string());
#else
	auto message_type = _registered_restapi.find(converter::to_wstring(action[MESSAGE_TYPE].as_string()));
#endif
	if (message_type != _registered_restapi.end())
	{
		message_type->second(make_shared<json::value>(action));

		request.reply(status_codes::OK);
		return;
	}

	request.reply(status_codes::NotImplemented);
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
