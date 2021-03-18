#include <iostream>

#include "logging.h"
#include "tcp_server.h"
#include "tcp_client.h"

#include "fmt/format.h"

using namespace logging;
using namespace network;

int main()
{
	logger::handle().set_target_level(logging_level::sequence);
	logger::handle().start();

	tcp_server server;
	tcp_client client;

	server.start(5690, 1, 1, 1);
	client.start(L"127.0.0.1", 5690, 1, 1, 1);

	std::this_thread::sleep_for(std::chrono::seconds(5));

	client.stop();
	server.stop();

	logger::handle().stop();

	return 0;
}