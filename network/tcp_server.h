#pragma once

#include "container.h"

#include <memory>
#include <vector>
#include <string>
#include <thread>

#include "asio.hpp"

namespace network
{
	class tcp_session;
	class tcp_server : public std::enable_shared_from_this<tcp_server>
	{
	public:
		tcp_server(void);
		~tcp_server(void);

	public:
		std::shared_ptr<tcp_server> get_ptr(void);

	public:
		void start(const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority);
		void stop(void);

	public:
		void send(const container::value_container& message);
		void send(std::shared_ptr<container::value_container> message);

	protected:
		void wait_connection(void);

	private:
		unsigned short _high_priority;
		unsigned short _normal_priority;
		unsigned short _low_priority;

	private:
		std::thread _thread;
		std::shared_ptr<asio::io_context> _io_context;
		std::shared_ptr<asio::ip::tcp::acceptor> _acceptor;
		std::vector<std::shared_ptr<tcp_session>> _sessions;
	};
}