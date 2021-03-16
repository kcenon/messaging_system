#include "tcp_server.h"

#include "tcp_session.h"

namespace network
{
	tcp_server::tcp_server() : _io_context(nullptr), _acceptor(nullptr)
	{

	}

	tcp_server::~tcp_server(void)
	{

	}

	std::shared_ptr<tcp_server> tcp_server::get_ptr(void)
	{
		return shared_from_this();
	}

	void tcp_server::start(const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_high_priority = high_priority;
		_normal_priority = normal_priority;
		_low_priority = low_priority;

		_io_context = std::make_shared<asio::io_context>();
		_acceptor = std::make_shared<asio::ip::tcp::acceptor>(*_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

		wait_connection();

		_thread = std::thread([](std::shared_ptr<asio::io_context> context)
			{
				while (context)
				{
					try
					{
						context->run();
						break;
					}
					catch (const std::overflow_error&) { if (context == nullptr) { break; } context->reset(); }
					catch (const std::runtime_error&) { if (context == nullptr) { break; } context->reset(); }
					catch (const std::exception&) { if (context == nullptr) { break; } context->reset(); }
					catch (...) { if (context == nullptr) { break; } context->reset(); }					
				}
			}, _io_context);
	}

	void tcp_server::stop(void)
	{
		if (_acceptor != nullptr && _acceptor->is_open())
		{
			_acceptor->close();
		}
		_acceptor.reset();

		if (_io_context != nullptr)
		{
			_io_context->stop();
		}
		_io_context.reset();

		if (_thread.joinable())
		{
			_thread.join();
		}
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
			if (session == nullptr)
			{
				continue;
			}

			session->send(message);
		}
	}

	void tcp_server::wait_connection(void)
	{
		_acceptor->async_accept(
			[this](std::error_code ec, asio::ip::tcp::socket socket)
			{
				if (ec)
				{
					return;
				}

				std::shared_ptr<tcp_session> session = std::make_shared<tcp_session>(socket);
				session->start(_high_priority, _normal_priority, _low_priority);

				_sessions.push_back(session);

				wait_connection();
			});
	}
}
