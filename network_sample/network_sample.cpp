#include <iostream>

#include "logging.h"
#include "tcp_server.h"
#include "tcp_client.h"

#include "fmt/format.h"

using namespace logging;
using namespace network;

int main()
{
	bool encrypt_mode = true;
	bool compress_mode = true;

	logger::handle().set_target_level(logging_level::sequence);
	logger::handle().start();

	tcp_server server(L"server", L"any string can be used for it");
	server.set_encrypt_mode(encrypt_mode);
	server.set_compress_mode(compress_mode);

	tcp_client client(L"client", L"any string can be used for it");
	client.set_compress_mode(compress_mode);

	server.start(5690, 1, 1, 1);
	client.start(L"127.0.0.1", 5690, 1, 1, 1);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	client.stop();
	server.stop();

	logger::handle().stop();

	return 0;
}