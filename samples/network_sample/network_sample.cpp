#include <iostream>

#include "logging.h"
#include "tcp_server.h"
#include "tcp_client.h"

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"network_sample";
constexpr auto CONNECTION_KEY = L"network_sample";

using namespace logging;
using namespace network;

void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition);
void received_message(std::shared_ptr<container::value_container> container);
void received_file(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path);

int main(void)
{
	bool encrypt_mode = true;
	bool compress_mode = false;

	logger::handle().set_target_level(logging_level::information);
	logger::handle().start(PROGRAM_NAME);

	std::shared_ptr<tcp_server> server = std::make_shared<tcp_server>(L"server");
	server->set_encrypt_mode(encrypt_mode);
	server->set_compress_mode(compress_mode);
	server->set_connection_key(CONNECTION_KEY);
	server->set_connection_notification(&connection);
	server->set_message_notification(&received_message);
	server->set_file_notification(&received_file);
	server->start(5690, 1, 1, 1);

	std::shared_ptr<tcp_client> client = std::make_shared<tcp_client>(L"client");
	client->set_compress_mode(compress_mode);
	client->set_connection_key(CONNECTION_KEY);
	client->set_connection_notification(&connection);
	client->set_message_notification(&received_message);
	client->set_file_notification(&received_file);
	client->start(L"127.0.0.1", 5690, 1, 1, 1);

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

	logger::handle().write(logging::logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
}

void received_file(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& indication_id, const std::wstring& target_path)
{
	logger::handle().write(logging::logging_level::information,
		fmt::format(L"source_id: {}, source_sub_id: {}, indication_id: {}, file_path: {}", source_id, source_sub_id, indication_id, target_path));
}