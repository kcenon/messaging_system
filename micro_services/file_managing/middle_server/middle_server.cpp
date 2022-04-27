#include <iostream>

#include "logging.h"
#include "messaging_server.h"
#include "messaging_client.h"
#include "converting.h"
#include "compressing.h"
#include "file_manager.h"
#include "argument_parser.h"

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "value.h"
#include "values/bool_value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"
#endif

#ifdef _CONSOLE
#include <Windows.h>
#endif

#include <signal.h>

#include "fmt/xchar.h"
#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"middle_server";

using namespace std;
using namespace logging;
using namespace network;
using namespace converting;
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
wstring main_connection_key = L"main_connection_key";
wstring middle_connection_key = L"middle_connection_key";
unsigned short middle_server_port = 8642;
wstring main_server_ip = L"127.0.0.1";
unsigned short main_server_port = 9753;
unsigned short high_priority_count = 4;
unsigned short normal_priority_count = 4;
unsigned short low_priority_count = 4;
size_t session_limit_count = 0;

#ifndef __USE_TYPE_CONTAINER__
map<wstring, function<void(shared_ptr<json::value>)>> _file_commands;
#else
map<wstring, function<void(shared_ptr<container::value_container>)>> _file_commands;
#endif

shared_ptr<file_manager> _file_manager = nullptr;
shared_ptr<messaging_client> _data_line = nullptr;
shared_ptr<messaging_client> _file_line = nullptr;
shared_ptr<messaging_server> _middle_server = nullptr;

#ifdef _CONSOLE
BOOL ctrl_handler(DWORD ctrl_type);
#endif

bool parse_arguments(const map<wstring, wstring>& arguments);
void create_middle_server(void);
void create_data_line(void);
void create_file_line(void);
void connection_from_middle_server(const wstring& target_id, const wstring& target_sub_id, const bool& condition);
void connection_from_data_line(const wstring& target_id, const wstring& target_sub_id, const bool& condition);
void connection_from_file_line(const wstring& target_id, const wstring& target_sub_id, const bool& condition);

#ifndef __USE_TYPE_CONTAINER__
void received_message_from_middle_server(shared_ptr<json::value> container);
void received_message_from_data_line(shared_ptr<json::value> container);
void received_message_from_file_line(shared_ptr<json::value> container);
#else
void received_message_from_middle_server(shared_ptr<container::value_container> container);
void received_message_from_data_line(shared_ptr<container::value_container> container);
void received_message_from_file_line(shared_ptr<container::value_container> container);
#endif

void received_file_from_file_line(const wstring& source_id, const wstring& source_sub_id, const wstring& indication_id, const wstring& target_path);

#ifndef __USE_TYPE_CONTAINER__
void download_files(shared_ptr<json::value> container);
void upload_files(shared_ptr<json::value> container);
void uploaded_file(shared_ptr<json::value> container);
#else
void download_files(shared_ptr<container::value_container> container);
void upload_files(shared_ptr<container::value_container> container);
void uploaded_file(shared_ptr<container::value_container> container);
#endif

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

	_file_manager = make_shared<file_manager>();

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
			_middle_server.reset();
			_data_line.reset();
			_file_line.reset();

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
		main_server_port = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--middle_server_port");
	if (target != arguments.end())
	{
		middle_server_port = (unsigned short)atoi(converter::to_string(target->second).c_str());
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

void create_middle_server(void)
{
	if (_middle_server != nullptr)
	{
		_middle_server.reset();
	}

	_middle_server = make_shared<messaging_server>(PROGRAM_NAME);
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

	_data_line = make_shared<messaging_client>(L"data_line");
	_data_line->set_bridge_line(true);
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

	_file_line = make_shared<messaging_client>(L"file_line");
	_file_line->set_bridge_line(true);
	_file_line->set_compress_mode(compress_mode);
	_file_line->set_connection_key(main_connection_key);
	_file_line->set_session_types(session_types::file_line);
	_file_line->set_connection_notification(&connection_from_file_line);
	_file_line->set_message_notification(&received_message_from_file_line);
	_file_line->set_file_notification(&received_file_from_file_line);
	_file_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void connection_from_middle_server(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging_level::information,
		fmt::format(L"a client on middle server: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
}

#ifndef __USE_TYPE_CONTAINER__
void received_message_from_middle_server(shared_ptr<json::value> container)
#else
void received_message_from_middle_server(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
	auto target = _file_commands.find((*container)[L"header"][L"message_type"].as_string());
#else
	auto target = _file_commands.find(converter::to_wstring((*container)["header"]["message_type"].as_string()));
#endif
#else
	auto target = _file_commands.find(container->message_type());
#endif
	if (target != _file_commands.end())
	{
		target->second(container);

		return;
	}

	if (_data_line == nullptr || !_data_line->is_confirmed())
	{
		if (_middle_server == nullptr)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> response = make_shared<json::value>(json::value::parse(container->serialize()));

#ifdef _WIN32
		(*response)[L"header"][L"source_id"] = (*container)[L"header"][L"target_id"];
		(*response)[L"header"][L"source_sub_id"] = (*container)[L"header"][L"target_sub_id"];
		(*response)[L"header"][L"target_id"] = (*container)[L"header"][L"source_id"];
		(*response)[L"header"][L"target_sub_id"] = (*container)[L"header"][L"source_sub_id"];

		(*response)[L"data"][L"error"] = json::value::boolean(true);
		(*response)[L"data"][L"reason"] = json::value::string(L"main_server has not been connected.");
#else
		(*response)["header"]["source_id"] = (*container)["header"]["target_id"];
		(*response)["header"]["source_sub_id"] = (*container)["header"]["target_sub_id"];
		(*response)["header"]["target_id"] = (*container)["header"]["source_id"];
		(*response)["header"]["target_sub_id"] = (*container)["header"]["source_sub_id"];

		(*response)["data"]["error"] = json::value::boolean(true);
		(*response)["data"]["reason"] = json::value::string("main_server has not been connected.");
#endif
#else
		shared_ptr<container::value_container> response = container->copy(false);
		response->swap_header();

		response << make_shared<container::bool_value>(L"error", true);
		response << make_shared<container::string_value>(L"reason", L"main_server has not been connected.");
#endif

		_middle_server->send(response);

		return;
	}

	if (_data_line)
	{
		_data_line->send(container);
	}
}

void connection_from_data_line(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
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

	if (_middle_server == nullptr)
	{
		return;
	}

	this_thread::sleep_for(chrono::seconds(1));

	_data_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void connection_from_file_line(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	if (_file_line == nullptr)
	{
		return;
	}

	logger::handle().write(logging_level::sequence,
		fmt::format(L"{} on middle server is {} from target: {}[{}]", _file_line->source_id(), condition ? L"connected" : L"disconnected", target_id, target_sub_id));

	if (condition)
	{
		return;
	}

	if (_middle_server == nullptr)
	{
		return;
	}

	this_thread::sleep_for(chrono::seconds(1));

	_file_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

#ifndef __USE_TYPE_CONTAINER__
void received_message_from_data_line(shared_ptr<json::value> container)
#else
void received_message_from_data_line(shared_ptr<container::value_container> container)
#endif
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

#ifndef __USE_TYPE_CONTAINER__
void received_message_from_file_line(shared_ptr<json::value> container)
#else
void received_message_from_file_line(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
	if ((*container)[L"header"][L"message_type"].as_string() == L"uploaded_file")
#else
	if ((*container)["header"]["message_type"].as_string() == "uploaded_file")
#endif
#else
	if (container->message_type() == L"uploaded_file")
#endif
	{
		uploaded_file(container);

		return;
	}

	if (_middle_server)
	{
		_middle_server->send(container);
	}
}

void received_file_from_file_line(const wstring& target_id, const wstring& target_sub_id, const wstring& indication_id, const wstring& target_path)
{
	logger::handle().write(logging_level::parameter,
		fmt::format(L"target_id: {}, target_sub_id: {}, indication_id: {}, file_path: {}", target_id, target_sub_id, indication_id, target_path));

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> container = _file_manager->received(indication_id, target_path);
#else
	shared_ptr<container::value_container> container = _file_manager->received(indication_id, target_path);
#endif

	if(container != nullptr)
	{
		if (_middle_server)
		{
			_middle_server->send(container);
		}
	}
}

#ifndef __USE_TYPE_CONTAINER__
void download_files(shared_ptr<json::value> container)
#else
void download_files(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

	if (_file_line == nullptr || !_file_line->is_confirmed())
	{
		if (_middle_server)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> response = make_shared<json::value>(json::value::parse(container->serialize()));

#ifdef _WIN32
		(*response)[L"header"][L"source_id"] = (*container)[L"header"][L"target_id"];
		(*response)[L"header"][L"source_sub_id"] = (*container)[L"header"][L"target_sub_id"];
		(*response)[L"header"][L"target_id"] = (*container)[L"header"][L"source_id"];
		(*response)[L"header"][L"target_sub_id"] = (*container)[L"header"][L"source_sub_id"];

		(*response)[L"data"][L"error"] = json::value::boolean(true);
		(*response)[L"data"][L"reason"] = json::value::string(L"main_server has not been connected.");
#else
		(*response)["header"]["source_id"] = (*container)["header"]["target_id"];
		(*response)["header"]["source_sub_id"] = (*container)["header"]["target_sub_id"];
		(*response)["header"]["target_id"] = (*container)["header"]["source_id"];
		(*response)["header"]["target_sub_id"] = (*container)["header"]["source_sub_id"];

		(*response)["data"]["error"] = json::value::boolean(true);
		(*response)["data"]["reason"] = json::value::string("main_server has not been connected.");
#endif
#else
		shared_ptr<container::value_container> response = container->copy(false);
		response->swap_header();

		response << make_shared<container::bool_value>(L"error", true);
		response << make_shared<container::string_value>(L"reason", L"main_server has not been connected.");
#endif

		_middle_server->send(response);

		return;
	}

	vector<wstring> target_paths;

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
	auto& files = (*container)[L"data"][L"files"].as_array();
	for (int index = 0; index < files.size(); ++index)
	{
		target_paths.push_back(files[index][L"target"].as_string());
	}

	_file_manager->set((*container)[L"data"][L"indication_id"].as_string(),
		(*container)[L"header"][L"source_id"].as_string(),
		(*container)[L"header"][L"source_sub_id"].as_string(), target_paths);
#else
	auto& files = (*container)["data"]["files"].as_array();
	for (int index = 0; index < files.size(); ++index)
	{
		target_paths.push_back(converter::to_wstring(files[index]["target"].as_string()));
	}

	_file_manager->set(converter::to_wstring((*container)["data"]["indication_id"].as_string()),
		converter::to_wstring((*container)["header"]["source_id"].as_string()),
		converter::to_wstring((*container)["header"]["source_sub_id"].as_string()), target_paths);
#endif
#else
	vector<shared_ptr<container::value>> files = container->value_array(L"file");
	for (auto& file : files)
	{
		target_paths.push_back((*file)[L"target"]->to_string());
	}

	_file_manager->set(container->get_value(L"indication_id")->to_string(),
		container->source_id(), container->source_sub_id(), target_paths);
#endif

	if (_middle_server)
	{
#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> start_message = make_shared<json::value>(json::value::object(true));

#ifdef _WIN32
		(*start_message)[L"header"][L"source_id"] = json::value::string(L"");
		(*start_message)[L"header"][L"source_sub_id"] = json::value::string(L"");
		(*start_message)[L"header"][L"target_id"] = (*container)[L"header"][L"source_id"];
		(*start_message)[L"header"][L"target_sub_id"] = (*container)[L"header"][L"source_sub_id"];
		(*start_message)[L"header"][L"message_type"] = json::value::string(L"transfer_condition");

		(*start_message)[L"data"][L"indication_id"] = (*container)[L"data"][L"indication_id"];
		(*start_message)[L"data"][L"percentage"] = json::value::number(0);
#else
		(*start_message)["header"]["source_id"] = json::value::string("");
		(*start_message)["header"]["source_sub_id"] = json::value::string("");
		(*start_message)["header"]["target_id"] = (*container)["header"]["source_id"];
		(*start_message)["header"]["target_sub_id"] = (*container)["header"]["source_sub_id"];
		(*start_message)["header"]["message_type"] = json::value::string("transfer_condition");

		(*start_message)["data"]["indication_id"] = (*container)["data"]["indication_id"];
		(*start_message)["data"]["percentage"] = json::value::number(0);
#endif

		_middle_server->send(start_message);
#else
		_middle_server->send(make_shared<container::value_container>(container->source_id(), container->source_sub_id(), L"transfer_condition",
			vector<shared_ptr<container::value>> {
				make_shared<container::string_value>(L"indication_id", container->get_value(L"indication_id")->to_string()),
				make_shared<container::ushort_value>(L"percentage", 0)
		}));
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> temp = make_shared<json::value>(json::value::parse(container->serialize()));

#ifdef _WIN32
	(*temp)[L"header"][L"message_type"] = json::value::string(L"request_files");
#else
	(*temp)["header"]["message_type"] = json::value::string("request_files");
#endif
#else
	shared_ptr<container::value_container> temp = container->copy();
	temp->set_message_type(L"request_files");
#endif

	if (_file_line)
	{
		_file_line->send(temp);
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

	if (_file_line == nullptr || !_file_line->is_confirmed())
	{
		if (_middle_server)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> response = make_shared<json::value>(json::value::parse(container->serialize()));

#ifdef _WIN32
		(*response)[L"header"][L"source_id"] = (*container)[L"header"][L"target_id"];
		(*response)[L"header"][L"source_sub_id"] = (*container)[L"header"][L"target_sub_id"];
		(*response)[L"header"][L"target_id"] = (*container)[L"header"][L"source_id"];
		(*response)[L"header"][L"target_sub_id"] = (*container)[L"header"][L"source_sub_id"];

		(*response)[L"data"][L"error"] = json::value::boolean(true);
		(*response)[L"data"][L"reason"] = json::value::string(L"main_server has not been connected.");
#else
		(*response)["header"]["source_id"] = (*container)["header"]["target_id"];
		(*response)["header"]["source_sub_id"] = (*container)["header"]["target_sub_id"];
		(*response)["header"]["target_id"] = (*container)["header"]["source_id"];
		(*response)["header"]["target_sub_id"] = (*container)["header"]["source_sub_id"];

		(*response)["data"]["error"] = json::value::boolean(true);
		(*response)["data"]["reason"] = json::value::string("main_server has not been connected.");
#endif
#else
		shared_ptr<container::value_container> response = container->copy(false);
		response->swap_header();

		response << make_shared<container::bool_value>(L"error", true);
		response << make_shared<container::string_value>(L"reason", L"main_server has not been connected.");
#endif

		_middle_server->send(response);

		return;
	}
	
	if (_file_line)
	{
#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		(*container)[L"data"][L"gateway_source_id"] = (*container)[L"header"][L"source_id"];
		(*container)[L"data"][L"gateway_source_sub_id"] = (*container)[L"header"][L"source_sub_id"];
		(*container)[L"header"][L"source_id"] = json::value::string(_file_line->source_id());
		(*container)[L"header"][L"source_sub_id"] = json::value::string(_file_line->source_sub_id());
#else
		(*container)["data"]["gateway_source_id"] = (*container)["header"]["source_id"];
		(*container)["data"]["gateway_source_sub_id"] = (*container)["header"]["source_sub_id"];
		(*container)["header"]["source_id"] = json::value::string(converter::to_string(_file_line->source_id()));
		(*container)["header"]["source_sub_id"] = json::value::string(converter::to_string(_file_line->source_sub_id()));
#endif
#else
		container << make_shared<container::string_value>(L"gateway_source_id", container->source_id());
		container << make_shared<container::string_value>(L"gateway_source_sub_id", container->source_sub_id());
		container->set_source(_file_line->source_id(), _file_line->source_sub_id());
#endif

		_file_line->send(container);
	}
}

#ifndef __USE_TYPE_CONTAINER__
void uploaded_file(shared_ptr<json::value> container)
#else
void uploaded_file(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
	shared_ptr<json::value> temp = _file_manager->received(
		(*container)[L"data"][L"indication_id"].as_string(), (*container)[L"data"][L"target_path"].as_string());
#else
	shared_ptr<json::value> temp = _file_manager->received(
		converter::to_wstring((*container)["data"]["indication_id"].as_string()),
		converter::to_wstring((*container)["data"]["target_path"].as_string()));
#endif
#else
	shared_ptr<container::value_container> temp = _file_manager->received(
		container->get_value(L"indication_id")->to_string(), container->get_value(L"target_path")->to_string());
#endif

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
	wcout << L"main server options:" << endl << endl;
	wcout << L"--encrypt_mode [value] " << endl;
	wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << endl << endl;
	wcout << L"--compress_mode [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << endl << endl;
	wcout << L"--compress_block_size [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to change compress block size must be appended '--compress_block_size size'.\n\tInitialize value is --compress_mode 1024." << endl << endl;
	wcout << L"--main_connection_key [value]" << endl;
	wcout << L"\tIf you want to change a specific key string for the connection to the main server must be appended\n\t'--main_connection_key [specific key string]'." << endl << endl;
	wcout << L"--middle_connection_key [value]" << endl;
	wcout << L"\tIf you want to change a specific key string for the connection to the middle server must be appended\n\t'--middle_connection_key [specific key string]'." << endl << endl;
	wcout << L"--main_server_port [value]" << endl;
	wcout << L"\tIf you want to change a port number for the connection to the main server must be appended\n\t'--main_server_port [port number]'." << endl << endl;
	wcout << L"--middle_server_port [value]" << endl;
	wcout << L"\tIf you want to change a port number for the connection to the middle server must be appended\n\t'--middle_server_port [port number]'." << endl << endl;
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
