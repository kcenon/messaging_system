#include "tcp_server.h"

#include "tcp_session.h"

namespace network
{
	tcp_server::tcp_server(void)
	{

	}

	tcp_server::~tcp_server(void)
	{

	}

	std::shared_ptr<tcp_server> tcp_server::get_ptr(void)
	{
		return shared_from_this();
	}

	void tcp_server::send(const container::value_container& message)
	{
		send(std::make_shared<container::value_container>(message));
	}

	void tcp_server::send(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		for (auto& session : _sessions)
		{
			session->send(message);
		}
	}
}
