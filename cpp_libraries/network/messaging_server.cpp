#include "messaging_server.h"

#include "logging.h"
#include "converting.h"
#include "messaging_session.h"
#include "thread_worker.h"
#include "job.h"

#include "fmt/format.h"

#include <algorithm>

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace converting;

	messaging_server::messaging_server(const std::wstring& source_id)
		: _io_context(nullptr), _acceptor(nullptr), _source_id(source_id), _connection_key(L"connection_key"), _encrypt_mode(false),
		_received_file(nullptr), _received_data(nullptr), _connection(nullptr), _received_message(nullptr), _compress_mode(false),
		_high_priority(8), _normal_priority(8), _low_priority(8), _session_limit_count(0), _possible_session_types({ session_types::binary_line })
	{

	}

	messaging_server::~messaging_server(void)
	{
		stop();
	}

	std::shared_ptr<messaging_server> messaging_server::get_ptr(void)
	{
		return shared_from_this();
	}

	void messaging_server::set_encrypt_mode(const bool& encrypt_mode)
	{
		_encrypt_mode = encrypt_mode;
	}

	void messaging_server::set_compress_mode(const bool& compress_mode)
	{
		_compress_mode = compress_mode;
	}

	void messaging_server::set_connection_key(const std::wstring& connection_key)
	{
		_connection_key = connection_key;
	}

	void messaging_server::set_ignore_snipping_targets(const std::vector<std::wstring>& ignore_snipping_targets)
	{
		_ignore_snipping_targets = ignore_snipping_targets;
	}

	void messaging_server::set_possible_session_types(const std::vector<session_types>& possible_session_types)
	{
		_possible_session_types = possible_session_types;
	}

	void messaging_server::set_session_limit_count(const bool& session_limit_count)
	{
		_session_limit_count = session_limit_count;
	}

	void messaging_server::set_connection_notification(const std::function<void(const std::wstring&, const std::wstring&, const bool&)>& notification)
	{
		_connection = notification;
	}

	void messaging_server::set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification)
	{
		_received_message = notification;
	}

	void messaging_server::set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification)
	{
		_received_file = notification;
	}

	void messaging_server::set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification)
	{
		_received_data = notification;
	}

	void messaging_server::start(const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_thread_pool = std::make_shared<threads::thread_pool>();
		_thread_pool->append(std::make_shared<thread_worker>(priorities::high), true);

		_high_priority = high_priority;
		_normal_priority = normal_priority;
		_low_priority = low_priority;

		_io_context = std::make_shared<asio::io_context>();
		_acceptor = std::make_shared<asio::ip::tcp::acceptor>(*_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

		wait_connection();

		_thread = std::thread([this](std::shared_ptr<asio::io_context> context)
			{
				while (context)
				{
					try
					{
						logger::handle().write(logging::logging_level::information, fmt::format(L"start messaging_server({})", _source_id));
						context->run();
						logger::handle().write(logging::logging_level::information, fmt::format(L"stop messaging_server({})", _source_id));
						break;
					}
					catch (const std::overflow_error&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_server({}) with overflow error", _source_id)); context->reset(); }
					catch (const std::runtime_error&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_server({}) with runtime error", _source_id)); context->reset(); }
					catch (const std::exception&) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_server({}) with exception", _source_id)); context->reset(); }
					catch (...) { if (context == nullptr) { break; } logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_server({}) with error", _source_id)); context->reset(); }
				}
			}, _io_context);
	}

	void messaging_server::wait_stop(const unsigned int& seconds)
	{
		_future_status = _promise_status.get_future();

		if (seconds == 0)
		{
			_future_status.wait();
			return;
		}

		_future_status.wait_for(std::chrono::seconds(seconds));
	}

	void messaging_server::stop(void)
	{
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}

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

	void messaging_server::echo(void)
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

	void messaging_server::send(const container::value_container& message)
	{
		send(std::make_shared<container::value_container>(message));
	}

	void messaging_server::send(std::shared_ptr<container::value_container> message)
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

	void messaging_server::send_files(const container::value_container& message)
	{
		send_files(std::make_shared<container::value_container>(message));
	}

	void messaging_server::send_files(std::shared_ptr<container::value_container> message)
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

			session->send_files(message);
		}
	}

	void messaging_server::send_binary(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
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

	void messaging_server::send_binary(const std::wstring source_id, const std::wstring& source_sub_id, const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
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

	void messaging_server::wait_connection(void)
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

				std::shared_ptr<messaging_session> session = std::make_shared<messaging_session>(_source_id, _connection_key, socket);
				if (session == nullptr)
				{
					wait_connection();

					return;
				}

				if (_session_limit_count > 0)
				{
					session->set_kill_code(_sessions.size() >= _session_limit_count);
				}

				session->set_connection_notification(std::bind(&messaging_server::connect_condition, this, std::placeholders::_1, std::placeholders::_2));
				session->set_message_notification(std::bind(&messaging_server::received_message, this, std::placeholders::_1));
				session->set_file_notification(_received_file);
				session->set_binary_notification(std::bind(&messaging_server::received_binary, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

				session->start(_encrypt_mode, _compress_mode, _ignore_snipping_targets, _possible_session_types, _high_priority, _normal_priority, _low_priority);

				_sessions.push_back(session);

				_thread_pool->push(std::make_shared<job>(priorities::high, std::bind(&messaging_server::check_confirm_condition, this)));

				wait_connection();
			});
	}

	bool messaging_server::check_confirm_condition(void)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		std::vector<std::shared_ptr<messaging_session>> sessions = _sessions;
		for (auto& session : sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			if (session->get_confirom_status() == session_conditions::expired)
			{
				connect_condition(session, false);
			}
		}

		return true;
	}

	void messaging_server::connect_condition(std::shared_ptr<messaging_session> target, const bool& condition)
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

	void messaging_server::received_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_received_message)
		{
			_received_message(message);
		}
	}

	void messaging_server::received_binary(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_received_data)
		{
			_received_data(source_id, source_sub_id, target_id, target_sub_id, data);
		}
	}
}
