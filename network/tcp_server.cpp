#include "tcp_server.h"

#include "logging.h"
#include "converting.h"
#include "tcp_session.h"

#include "fmt/format.h"

#include <algorithm>

namespace network
{
	using namespace logging;
	using namespace converting;

	tcp_server::tcp_server(const std::wstring& source_id, const std::wstring& connection_key) 
		: _io_context(nullptr), _acceptor(nullptr), _source_id(source_id), _connection_key(connection_key), 
		_received_file(nullptr), _connection(nullptr), _received_message(nullptr)
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

	void tcp_server::set_encrypt_mode(const bool& encrypt_mode)
	{
		_encrypt_mode = encrypt_mode;
	}

	void tcp_server::set_compress_mode(const bool& compress_mode)
	{
		_compress_mode = compress_mode;
	}

	void tcp_server::set_connection_notification(const std::function<void(const std::wstring&, const std::wstring&, const bool&)>& notification)
	{
		_connection = notification;
	}

	void tcp_server::set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification)
	{
		_received_message = notification;
	}

	void tcp_server::set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification)
	{
		_received_file = notification;
	}

	void tcp_server::start(const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_high_priority = high_priority;
		_normal_priority = normal_priority;
		_low_priority = low_priority;

#ifdef ASIO_STANDALONE
		_io_context = std::make_shared<asio::io_context>();
		_acceptor = std::make_shared<asio::ip::tcp::acceptor>(*_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
#else
		_io_context = std::make_shared<boost::asio::io_context>();
		_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(*_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
#endif

		wait_connection();

#ifdef ASIO_STANDALONE
		_thread = std::thread([](std::shared_ptr<asio::io_context> context)
#else
		_thread = std::thread([](std::shared_ptr<boost::asio::io_context> context)
#endif
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
		if (_acceptor != nullptr)
		{
			if (_acceptor->is_open())
			{
				_acceptor->close();
			}
			_acceptor.reset();
		}

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
			_io_context->stop();
			_io_context.reset();
		}

		if (_thread.joinable())
		{
			_thread.join();
		}
	}

	void tcp_server::echo(void)
	{
		for (auto& session : _sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			session->echo();
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
#ifdef ASIO_STANDALONE
			[this](std::error_code ec, asio::ip::tcp::socket socket)
#else
			[this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
#endif
			{
				if (ec)
				{
					return;
				}

				logger::handle().write(logging::logging_level::information, fmt::format(L"accepted new client: {}:{}", 
					converter::to_wstring(socket.remote_endpoint().address().to_string()), socket.remote_endpoint().port()));

				std::shared_ptr<tcp_session> session = std::make_shared<tcp_session>(_source_id, _connection_key, socket);
				session->set_connection_notification(std::bind(&tcp_server::connect_condition, this, std::placeholders::_1, std::placeholders::_2));
				session->set_message_notification(_received_message);
				session->set_file_notification(_received_file);
				session->start(_encrypt_mode, _compress_mode, _high_priority, _normal_priority, _low_priority);

				_sessions.push_back(session);

				wait_connection();
			});
	}

	void tcp_server::connect_condition(std::shared_ptr<tcp_session> target, const bool& condition)
	{
		if (target == nullptr)
		{
			return;
		}

		if (_connection)
		{
			_connection(target->target_id(), target->target_sub_id(), condition);
		}

		if (!condition)
		{
			auto iter = std::find(_sessions.begin(), _sessions.end(), target);
			if (iter != _sessions.end())
			{
				_sessions.erase(iter);
			}
		}
	}
}
