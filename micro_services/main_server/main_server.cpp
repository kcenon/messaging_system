#include <iostream>

#include "logging.h"
#include "tcp_server.h"
#include "argument_parsing.h"

#include <wchar.h>
#include <algorithm>

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"main_server";

using namespace logging;
using namespace network;
using namespace argument_parsing;

void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message(std::shared_ptr<container::value_container> container);
void received_file(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path);
void display_help(void);

int main(int argc, char* argv[])
{
	std::map<std::wstring, std::wstring> arguments = argument_parser::parse(argc, argv);

	std::wstring temp;
	bool encrypt_mode = false;
	bool compress_mode = false;
	logging_level log_level = logging_level::information;
	std::wstring connection_key = L"main_connection_key";
	unsigned short server_port = 9753;
	unsigned short high_priority_count = 1;
	unsigned short normal_priority_count = 2;
	unsigned short low_priority_count = 3;

	auto target = arguments.find(L"--help");
	if (target != arguments.end())
	{
		display_help();

		return 0;
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

	target = arguments.find(L"--logging_level");
	if (target != arguments.end())
	{
		log_level = (logging_level)_wtoi(target->second.c_str());
	}

	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	std::shared_ptr<tcp_server> server = std::make_shared<tcp_server>(PROGRAM_NAME);
	server->set_encrypt_mode(encrypt_mode);
	server->set_compress_mode(compress_mode);
	server->set_connection_key(connection_key);
	server->set_connection_notification(&connection);
	server->set_message_notification(&received_message);
	server->set_file_notification(&received_file);
	server->start(server_port, high_priority_count, normal_priority_count, low_priority_count);

	server->wait_stop();

	logger::handle().stop();

	return 0;
}

void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"target_id: {}, target_sub_id: {}, condition: {}", target_id, target_sub_id, condition));
}

void received_message(std::shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	logger::handle().write(logging::logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
}

void received_file(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"source_id: {}, source_sub_id: {}, indication_id: {}, file_path: {}", source_id, source_sub_id, indication_id, target_path));
}

void display_help(void)
{
	std::wcout << L"--encrypt_mode" << std::endl;
	std::wcout << L"--compress_mode" << std::endl;
	std::wcout << L"--connection_key" << std::endl;
	std::wcout << L"--server_port" << std::endl;
	std::wcout << L"--high_priority_count" << std::endl;
	std::wcout << L"--normal_priority_count" << std::endl;
	std::wcout << L"--low_priority_count" << std::endl;
	std::wcout << L"--logging_level" << std::endl;
}