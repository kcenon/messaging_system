#include <iostream>

#include "logging.h"
#include "messaging_server.h"
#include "messaging_client.h"
#include "argument_parsing.h"

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"network_sample";
constexpr auto CONNECTION_KEY = L"network_sample";

using namespace logging;
using namespace network;
using namespace argument_parsing;

bool write_console = false;
bool encrypt_mode = false;
bool compress_mode = false;
logging_level log_level = logging_level::information;
unsigned short server_port = 5555;

bool parse_arguments(const std::map<std::wstring, std::wstring>& arguments);
void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument_parser::parse(argc, argv)))
	{
		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	std::shared_ptr<messaging_server> server = std::make_shared<messaging_server>(L"server");
	server->set_encrypt_mode(encrypt_mode);
	server->set_compress_mode(compress_mode);
	server->set_connection_key(CONNECTION_KEY);
	server->set_connection_notification(&connection);
	server->start(server_port, 1, 1, 1);

	std::shared_ptr<messaging_client> client = std::make_shared<messaging_client>(L"client");
	client->set_compress_mode(compress_mode);
	client->set_connection_key(CONNECTION_KEY);
	client->set_connection_notification(&connection);
	client->start(L"127.0.0.1", server_port, 1, 1, 1);

	std::this_thread::sleep_for(std::chrono::seconds(1));
	for (int i = 0; i < 100; ++i)
	{
		client->echo();
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));

	client->stop();
	server->stop();

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

	target = arguments.find(L"--server_port");
	if (target != arguments.end())
	{
		server_port = (unsigned short)_wtoi(target->second.c_str());
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

void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"a client on main server: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
}

void display_help(void)
{
	std::wcout << L"network sample options:" << std::endl << std::endl;
	std::wcout << L"--encrypt_mode [value] " << std::endl;
	std::wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << std::endl << std::endl;
	std::wcout << L"--compress_mode [value]" << std::endl;
	std::wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << std::endl << std::endl;
	std::wcout << L"--server_port [value]" << std::endl;
	std::wcout << L"\tIf you want to change a port number for the connection to the main server must be appended\n\t'--server_port [port number]'." << std::endl << std::endl;
	std::wcout << L"--write_console_mode [value] " << std::endl;
	std::wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << std::endl << std::endl;
	std::wcout << L"--logging_level [value]" << std::endl;
	std::wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << std::endl;
}