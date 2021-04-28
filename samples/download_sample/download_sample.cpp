﻿#include <iostream>

#include "logging.h"
#include "converting.h"
#include "messaging_client.h"
#include "folder_handling.h"
#include "argument_parsing.h"

#include "container.h"
#include "values/string_value.h"
#include "values/container_value.h"

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"download_sample";

using namespace logging;
using namespace network;
using namespace converting;
using namespace folder_handling;
using namespace argument_parsing;

bool encrypt_mode = false;
bool compress_mode = false;
logging_level log_level = logging_level::information;
std::wstring source_folder = L"";
std::wstring target_folder = L"";
std::wstring connection_key = L"middle_connection_key";
std::wstring server_ip = L"127.0.0.1";
unsigned short server_port = 8642;
unsigned short high_priority_count = 1;
unsigned short normal_priority_count = 2;
unsigned short low_priority_count = 3;

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments);
void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message(std::shared_ptr<container::value_container> container);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument_parser::parse(argc, argv)))
	{
		return 0;
	}

	std::vector<std::wstring> sources = folder_handler::get_files(source_folder);
	if (sources.empty())
	{
		display_help();

		return 0;
	}

	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	std::shared_ptr<messaging_client> client = std::make_shared<messaging_client>(PROGRAM_NAME);
	client->set_compress_mode(compress_mode);
	client->set_connection_key(connection_key);
	client->set_session_types(session_types::file_line);
	client->set_connection_notification(&connection);
	client->set_message_notification(&received_message);
	client->start(server_ip, server_port, high_priority_count, normal_priority_count, low_priority_count);

	std::vector<std::shared_ptr<container::value>> files;

	files.push_back(std::make_shared<container::string_value>(L"indication_id", L"download_test"));
	for (auto& source : sources)
	{
		files.push_back(std::make_shared<container::container_value>(L"file", std::vector<std::shared_ptr<container::value>> {
			std::make_shared<container::string_value>(L"source", source),
			std::make_shared<container::string_value>(L"target", converter::replace2(source, source_folder, target_folder))
		}));
	}

	std::shared_ptr<container::value_container> container = 
		std::make_shared<container::value_container>(L"main_server", L"", L"download_files", files);
	client->send(container);

	std::this_thread::sleep_for(std::chrono::seconds(100));

	client->stop();

	logger::handle().stop();

	return 0;
}

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

	target = arguments.find(L"--source_folder");
	if (target != arguments.end())
	{
		source_folder = target->second;
	}

	target = arguments.find(L"--target_folder");
	if (target != arguments.end())
	{
		target_folder = target->second;
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

	return true;
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

	if (container->message_type() == L"transfer_condition")
	{
		if (container->get_value(L"percentage")->to_ushort() == 0)
		{
			logger::handle().write(logging::logging_level::information,
				fmt::format(L"started download: [{}]", container->get_value(L"indication_id")->to_string()));

			return;
		}

		logger::handle().write(logging::logging_level::information,
			fmt::format(L"received percentage: [{}] {}%", container->get_value(L"indication_id")->to_string(), container->get_value(L"percentage")->to_ushort()));

		if (container->get_value(L"completed")->to_boolean())
		{
			logger::handle().write(logging::logging_level::information,
				fmt::format(L"completed download: [{}] success-{}, fail-{}", container->get_value(L"indication_id")->to_string(), container->get_value(L"completed_count")->to_ushort(), container->get_value(L"failed_count")->to_ushort()));
		}
		else if (container->get_value(L"percentage")->to_ushort() == 100)
		{
			logger::handle().write(logging::logging_level::information,
				fmt::format(L"completed download: [{}]", container->get_value(L"indication_id")->to_string()));
		}

		return;
	}

	logger::handle().write(logging::logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
}

void display_help(void)
{
	std::wcout << L"download sample options:" << std::endl << std::endl;
	std::wcout << L"--encrypt_mode [value] " << std::endl;
	std::wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << std::endl << std::endl;
	std::wcout << L"--compress_mode [value]" << std::endl;
	std::wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << std::endl << std::endl;
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
	std::wcout << L"--source_folder [path]" << std::endl;
	std::wcout << L"\tIf you want to download folder on middle server on computer must be appended '--source_folder [path]'." << std::endl << std::endl;
	std::wcout << L"--target_folder [path]" << std::endl;
	std::wcout << L"\tIf you want to download on your computer must be appended '--target_folder [path]'." << std::endl << std::endl;
	std::wcout << L"--logging_level [value]" << std::endl;
	std::wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << std::endl;
}