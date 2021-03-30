#include <iostream>

#include "logging.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "argument_parsing.h"

#include "values/bool_value.h"
#include "values/string_value.h"

#include "fmt/format.h"

using namespace logging;
using namespace network;
using namespace argument_parsing;

std::wstring main_server_ip = L"";
unsigned short main_server_port = 0;
unsigned short high_priority_count = 0;
unsigned short normal_priority_count = 0;
unsigned short low_priority_count = 0;

std::vector<std::wstring> _file_commands;

std::atomic<bool> _data_line_connected{ false };
std::atomic<bool> _file_line_connected{ false };

std::shared_ptr<tcp_server> _server = nullptr;
std::shared_ptr<tcp_client> _data_line = nullptr;
std::shared_ptr<tcp_client> _file_line = nullptr;

void connection_from_server(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message_from_server(std::shared_ptr<container::value_container> container);
void connection_from_data_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message_from_data_line(std::shared_ptr<container::value_container> container);
void connection_from_file_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message_from_file_line(std::shared_ptr<container::value_container> container);
void received_file_from_file_line(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path);

int main(int argc, char* argv[])
{
	std::map<std::wstring, std::wstring> arguments = argument_parser::parse(argc, argv);

	std::wstring temp;
	bool encrypt_mode = false;
	bool compress_mode = false;
	std::wstring main_connection_key = L"";
	std::wstring middle_connection_key = L"";
	unsigned short middle_server_port = 0;

	auto target = arguments.find(L"--encrypt_mode");
	if (target != arguments.end())
	{
		temp = target->second;
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true"))
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

		if (temp.compare(L"true"))
		{
			compress_mode = true;
		}
		else
		{
			compress_mode = false;
		}
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

	_file_commands.push_back(L"");
	_file_commands.push_back(L"");
	_file_commands.push_back(L"");

	logger::handle().set_target_level(logging_level::information);
	logger::handle().start();

	_data_line = std::make_shared<tcp_client>(L"data_line");
	_data_line->set_compress_mode(compress_mode);
	_data_line->set_session_types(session_types::message_line);
	_data_line->set_connection_key(main_connection_key);
	_data_line->set_connection_notification(&connection_from_data_line);
	_data_line->set_message_notification(&received_message_from_data_line);
	_data_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);

	_file_line = std::make_shared<tcp_client>(L"file_line");
	_file_line->set_compress_mode(compress_mode);
	_file_line->set_session_types(session_types::file_line);
	_file_line->set_connection_key(main_connection_key);
	_file_line->set_connection_notification(&connection_from_file_line);
	_file_line->set_message_notification(&received_message_from_file_line);
	_file_line->set_file_notification(&received_file_from_file_line);
	_file_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);

	_server = std::make_shared<tcp_server>(L"middle_server");
	_server->set_encrypt_mode(encrypt_mode);
	_server->set_compress_mode(compress_mode);
	_server->set_connection_key(middle_connection_key);
	_server->set_connection_notification(&connection_from_server);
	_server->set_message_notification(&received_message_from_server);
	_server->start(middle_server_port, high_priority_count, normal_priority_count, low_priority_count);

	_server->wait_stop();

	logger::handle().stop();

	return 0;
}

void connection_from_server(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"target_id: {}, target_sub_id: {}, condition: {}", target_id, target_sub_id, condition));
}

void received_message_from_server(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	auto target = std::find_if(_file_commands.begin(), _file_commands.end(),
		[&container](const std::wstring& item)
		{
			return item == container->message_type();
		});

	if (target != _file_commands.end())
	{
		if (!_data_line_connected.load())
		{
			if (_server)
			{
				std::shared_ptr<container::value_container> response = container->copy(true, true);
				response << std::make_shared<container::bool_value>(L"error", true);
				response << std::make_shared<container::string_value>(L"reason", L"main_server has not been connected.");

				_server->send(response);
			}

			return;
		}

		if (_data_line)
		{
			_data_line->send(container);
		}

		return;
	}

	if (!_file_line_connected.load())
	{
		if (_server)
		{
			std::shared_ptr<container::value_container> response = container->copy(true, true);
			response << std::make_shared<container::bool_value>(L"error", true);
			response << std::make_shared<container::string_value>(L"reason", L"main_server has not been connected.");

			_server->send(response);
		}

		return;
	}

	if (_file_line)
	{
		_file_line->send(container);
	}
}

void connection_from_data_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	_data_line_connected.store(condition);

	if (condition)
	{
		logger::handle().write(logging::logging_level::information,
			fmt::format(L"data_line is connected => target: {}[{}], condition: {}", target_id, target_sub_id, condition));

		return;
	}

	_data_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void received_message_from_data_line(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	if (_server)
	{
		_server->send(container);
	}
}

void connection_from_file_line(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	_file_line_connected.store(condition);

	if (condition)
	{
		logger::handle().write(logging::logging_level::information,
			fmt::format(L"file_line is connected => target: {}[{}], condition: {}", target_id, target_sub_id, condition));

		return;
	}

	_file_line->start(main_server_ip, main_server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void received_message_from_file_line(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	if (_server)
	{
		_server->send(container);
	}
}

void received_file_from_file_line(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"source_id: {}, source_sub_id: {}, indication_id: {}, file_path: {}", source_id, source_sub_id, indication_id, target_path));
}