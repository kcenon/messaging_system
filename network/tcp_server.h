#pragma once

#include "container.h"

#include <memory>
#include <vector>
#include <string>

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
		void send(const container::value_container& message);
		void send(std::shared_ptr<container::value_container> message);

	private:
		std::vector<std::shared_ptr<tcp_session>> _sessions;
	};
}