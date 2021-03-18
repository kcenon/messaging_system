#include "tcp_server.h"

#include "logging.h"
#include "converting.h"
#include "tcp_session.h"

#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace converting;

	tcp_server::tcp_server(void) : _io_context(nullptr), _acceptor(nullptr)
	{

	}

	tcp_server::~tcp_server(void)
	{
		stop();
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
						logger::handle().write(logging::logging_level::information, L"start tcp_server");
						context->run();
						logger::handle().write(logging::logging_level::information, L"stop tcp_server");
						break;
					}
					catch (const std::overflow_error&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, L"break tcp_client with overflow error"); context->reset(); }
					catch (const std::runtime_error&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, L"break tcp_client with runtime error"); context->reset(); }
					catch (const std::exception&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, L"break tcp_client with exception"); context->reset(); }
					catch (...) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, L"break tcp_client with error"); context->reset(); }
				}
			}, _io_context);
	}

	void tcp_server::stop(void)
	{
		if (_acceptor != nullptr && _acceptor->is_open())
		{
			logger::handle().write(logging::logging_level::sequence, L"attempts to close acceptor");
			_acceptor->close();
		}
		_acceptor.reset();

		for (auto& session : _sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			session->stop();
		}
		_sessions.clear();

		if (_io_context != nullptr)
		{
			logger::handle().write(logging::logging_level::sequence, L"attempts to stop io_context");
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

				logger::handle().write(logging::logging_level::information, fmt::format(L"accepted new client: {}:{}", 
					converter::to_wstring(socket.remote_endpoint().address().to_string()), socket.remote_endpoint().port()));

				std::shared_ptr<tcp_session> session = std::make_shared<tcp_session>(socket);
				session->start(_high_priority, _normal_priority, _low_priority);

				_sessions.push_back(session);

				wait_connection();
			});
	}
}
