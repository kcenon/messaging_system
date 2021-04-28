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

	tcp_server::tcp_server(const std::wstring& source_id) 
		: _io_context(nullptr), _acceptor(nullptr), _source_id(source_id), _connection_key(L"connection_key"), 
		_received_file(nullptr), _received_data(nullptr), _connection(nullptr), _received_message(nullptr), _broadcast_mode(false)
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

	void tcp_server::set_broadcast_mode(const bool& broadcast_mode)
	{
		_broadcast_mode = broadcast_mode;
	}

	void tcp_server::set_connection_key(const std::wstring& connection_key)
	{
		_connection_key = connection_key;
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

	void tcp_server::set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification)
	{
		_received_data = notification;
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
		_thread = std::thread([this](std::shared_ptr<asio::io_context> context)
#else
		_thread = std::thread([this](std::shared_ptr<boost::asio::io_context> context)
#endif
			{
				while (context)
				{
					try
					{
						logger::handle().write(logging::logging_level::information, fmt::format(L"start tcp_server({})", _source_id));
						context->run();
						logger::handle().write(logging::logging_level::information, fmt::format(L"stop tcp_server({})", _source_id));
						break;
					}
					catch (const std::overflow_error&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break tcp_server({}) with overflow error", _source_id)); context->reset(); }
					catch (const std::runtime_error&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break tcp_server({}) with runtime error", _source_id)); context->reset(); }
					catch (const std::exception&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break tcp_server({}) with exception", _source_id)); context->reset(); }
					catch (...) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break tcp_server({}) with error", _source_id)); context->reset(); }
				}
			}, _io_context);
	}

	void tcp_server::wait_stop(const unsigned int& seconds)
	{
		_future_status = _promise_status.get_future();

		if (seconds == 0)
		{
			_future_status.wait();
			return;
		}

		_future_status.wait_for(std::chrono::seconds(seconds));
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
			_promise_status.set_value(true);

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

	void tcp_server::send_files(const container::value_container& message)
	{
		send_files(std::make_shared<container::value_container>(message));
	}

	void tcp_server::send_files(std::shared_ptr<container::value_container> message)
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

			if (session->get_session_type() != session_types::file_line)
			{
				continue;
			}

			if (session->target_id() != message->get_value(L"gateway_source_id")->to_string() &&
				session->target_sub_id() != message->get_value(L"gateway_source_sub_id")->to_string())
			{
				continue;
			}

			session->send_files(message);
		}
	}

	void tcp_server::send_binary(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		for (auto& session : _sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			session->send_binary(target_id, target_sub_id, data);
		}
	}

	void tcp_server::send_binary(const std::wstring source_id, const std::wstring& source_sub_id, const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
	}

		for (auto& session : _sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			session->send_binary(source_id, source_sub_id, target_id, target_sub_id, data);
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
				session->set_message_notification(std::bind(&tcp_server::received_message, this, std::placeholders::_1));
				session->set_file_notification(_received_file);
				session->set_binary_notification(std::bind(&tcp_server::received_binary, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
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

		std::thread thread([this](const std::wstring& target_id, const std::wstring& target_sub_id, const bool& connection)
			{
				if (_connection)
				{
					_connection(target_id, target_sub_id, connection);
				}
			}, target->target_id(), target->target_sub_id(), condition);
		thread.detach();

		if (!condition)
		{
			auto iter = std::find(_sessions.begin(), _sessions.end(), target);
			if (iter != _sessions.end())
			{
				_sessions.erase(iter);
			}
		}
	}

	void tcp_server::received_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_broadcast_mode && _source_id.compare(message->source_id()) != 0)
		{
			send(message);

			return;
		}

		if (_received_message)
		{
			_received_message(message);
		}
	}

	void tcp_server::received_binary(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_broadcast_mode && _source_id.compare(source_id) != 0)
		{
			send_binary(source_id, source_sub_id, target_id, target_sub_id, data);

			return;
		}

		if (_received_data)
		{
			_received_data(source_id, source_sub_id, target_id, target_sub_id, data);
		}
	}
}
