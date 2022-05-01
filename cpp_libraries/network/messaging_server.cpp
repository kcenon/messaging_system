#include "messaging_server.h"

#include "logging.h"
#include "converting.h"
#include "messaging_session.h"
#include "thread_worker.h"
#include "job.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <algorithm>

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace converting;

	messaging_server::messaging_server(const wstring& source_id)
		: _io_context(nullptr), _acceptor(nullptr), _source_id(source_id), _connection_key(L"connection_key"), _encrypt_mode(false),
		_received_file(nullptr), _received_data(nullptr), _connection(nullptr), _received_message(nullptr), _compress_mode(false),
		_high_priority(8), _normal_priority(8), _low_priority(8), _session_limit_count(0), _possible_session_types({ session_types::binary_line })
	{

	}

	messaging_server::~messaging_server(void)
	{
		stop();
	}

	shared_ptr<messaging_server> messaging_server::get_ptr(void)
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

	void messaging_server::set_connection_key(const wstring& connection_key)
	{
		_connection_key = connection_key;
	}

	void messaging_server::set_ignore_target_ids(const vector<wstring>& ignore_target_ids)
	{
		_ignore_target_ids = ignore_target_ids;
	}

	void messaging_server::set_ignore_snipping_targets(const vector<wstring>& ignore_snipping_targets)
	{
		_ignore_snipping_targets = ignore_snipping_targets;
	}

	void messaging_server::set_possible_session_types(const vector<session_types>& possible_session_types)
	{
		_possible_session_types = possible_session_types;
	}

	void messaging_server::set_session_limit_count(const bool& session_limit_count)
	{
		_session_limit_count = session_limit_count;
	}

	void messaging_server::set_connection_notification(const function<void(const wstring&, const wstring&, const bool&)>& notification)
	{
		_connection = notification;
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_server::set_message_notification(const function<void(shared_ptr<json::value>)>& notification)
#else
	void messaging_server::set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification)
#endif
	{
		_received_message = notification;
	}

	void messaging_server::set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification)
	{
		_received_file = notification;
	}

	void messaging_server::set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)>& notification)
	{
		_received_data = notification;
	}

	void messaging_server::start(const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_thread_pool = make_shared<threads::thread_pool>();
		_thread_pool->append(make_shared<thread_worker>(priorities::high), true);

		_high_priority = high_priority;
		_normal_priority = normal_priority;
		_low_priority = low_priority;

		_io_context = make_shared<asio::io_context>();
		_acceptor = make_shared<asio::ip::tcp::acceptor>(*_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

		wait_connection();

		_thread = thread([&]()
			{
				while (_io_context)
				{
					try
					{
						logger::handle().write(logging_level::information, fmt::format(L"start messaging_server({})", _source_id));
						_io_context->run();
						logger::handle().write(logging_level::information, fmt::format(L"stop messaging_server({})", _source_id));
						break;
					}
					catch (const overflow_error&) { if (_io_context == nullptr) { break; } logger::handle().write(logging_level::exception, fmt::format(L"break messaging_server({}) with overflow error", _source_id)); _io_context->reset(); }
					catch (const runtime_error&) { if (_io_context == nullptr) { break; } logger::handle().write(logging_level::exception, fmt::format(L"break messaging_server({}) with runtime error", _source_id)); _io_context->reset(); }
					catch (const exception&) { if (_io_context == nullptr) { break; } logger::handle().write(logging_level::exception, fmt::format(L"break messaging_server({}) with exception", _source_id)); _io_context->reset(); }
					catch (...) { if (_io_context == nullptr) { break; } logger::handle().write(logging_level::exception, fmt::format(L"break messaging_server({}) with error", _source_id)); _io_context->reset(); }
				}
			});
	}

	void messaging_server::wait_stop(const unsigned int& seconds)
	{
		_future_status = _promise_status.get_future();

		if (seconds == 0)
		{
			_future_status.wait();
			return;
		}

		_future_status.wait_for(chrono::seconds(seconds));
	}

	void messaging_server::stop(void)
	{
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
		}

		if (_acceptor != nullptr)
		{
			if (_acceptor->is_open())
			{
				_acceptor->close();
			}
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

#ifndef __USE_TYPE_CONTAINER__
	void messaging_server::send(const json::value& message, optional<session_types> type)
#else
	void messaging_server::send(const container::value_container& message, optional<session_types> type)
#endif
	{
#ifndef __USE_TYPE_CONTAINER__
		send(make_shared<json::value>(message), type);
#else
		send(make_shared<container::value_container>(message), type);
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_server::send(shared_ptr<json::value> message, optional<session_types> type)
#else
	void messaging_server::send(shared_ptr<container::value_container> message, optional<session_types> type)
#endif
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

			if (type.has_value() && session->get_session_type() != type.value())
			{
				continue;
			}

			session->send(message);
		}
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_server::send_files(const json::value& message)
#else
	void messaging_server::send_files(const container::value_container& message)
#endif
	{
#ifndef __USE_TYPE_CONTAINER__
		send_files(make_shared<json::value>(message));
#else
		send_files(make_shared<container::value_container>(message));
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_server::send_files(shared_ptr<json::value> message)
#else
	void messaging_server::send_files(shared_ptr<container::value_container> message)
#endif
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

	void messaging_server::send_binary(const wstring target_id, const wstring& target_sub_id, const vector<unsigned char>& data)
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

	void messaging_server::send_binary(const wstring source_id, const wstring& source_sub_id, const wstring target_id, const wstring& target_sub_id, const vector<unsigned char>& data)
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
			[this](error_code ec, asio::ip::tcp::socket socket)
			{
				if (ec)
				{
					return;
				}

				logger::handle().write(logging_level::information, fmt::format(L"accepted new client: {}:{}", 
					converter::to_wstring(socket.remote_endpoint().address().to_string()), socket.remote_endpoint().port()));

				shared_ptr<messaging_session> session = make_shared<messaging_session>(_source_id, _connection_key, socket);
				if (session == nullptr)
				{
					wait_connection();

					return;
				}

				if (_session_limit_count > 0)
				{
					session->set_kill_code(_sessions.size() >= _session_limit_count);
				}

				session->set_ignore_target_ids(_ignore_target_ids);
				session->set_ignore_snipping_targets(_ignore_snipping_targets);
				session->set_connection_notification(bind(&messaging_server::connect_condition, this, placeholders::_1, placeholders::_2));
				session->set_message_notification(bind(&messaging_server::received_message, this, placeholders::_1));
				session->set_file_notification(_received_file);
				session->set_binary_notification(bind(&messaging_server::received_binary, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5));

				session->start(_encrypt_mode, _compress_mode, _possible_session_types, _high_priority, _normal_priority, _low_priority);

				_sessions.push_back(session);

				_thread_pool->push(make_shared<job>(priorities::high, bind(&messaging_server::check_confirm_condition, this)));

				wait_connection();
			});
	}

	bool messaging_server::check_confirm_condition(void)
	{
		this_thread::sleep_for(chrono::seconds(1));

		vector<shared_ptr<messaging_session>> sessions = _sessions;
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

	void messaging_server::connect_condition(shared_ptr<messaging_session> target, const bool& condition)
	{
		if (target == nullptr)
		{
			return;
		}

		if (!condition)
		{
			auto iter = find(_sessions.begin(), _sessions.end(), target);
			if (iter != _sessions.end())
			{
				_sessions.erase(iter);
			}
		}

		thread thread([this](const wstring& target_id, const wstring& target_sub_id, const bool& connection)
			{
				if (_connection)
				{
					_connection(target_id, target_sub_id, connection);
				}
			}, target->target_id(), target->target_sub_id(), condition);
		thread.detach();
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_server::received_message(shared_ptr<json::value> message)
#else
	void messaging_server::received_message(shared_ptr<container::value_container> message)
#endif
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

	void messaging_server::received_binary(const wstring& source_id, const wstring& source_sub_id, const wstring& target_id, const wstring& target_sub_id, const vector<unsigned char>& data)
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
