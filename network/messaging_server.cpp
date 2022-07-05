/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "messaging_server.h"

#include "logging.h"
#include "converting.h"
#include "messaging_session.h"
#include "thread_worker.h"
#include "job.h"

#ifdef __USE_TYPE_CONTAINER__
#include "values/bool_value.h"
#include "values/string_value.h"
#endif

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <future>
#include <algorithm>

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace converting;

	messaging_server::messaging_server(const wstring& source_id, const unsigned char& start_code_value, const unsigned char& end_code_value)
		: _io_context(nullptr), _acceptor(nullptr), _source_id(source_id), _connection_key(L"connection_key"), _encrypt_mode(false),
		_received_file(nullptr), _received_data(nullptr), _connection(nullptr), _received_message(nullptr), _compress_mode(false),
		_high_priority(8), _normal_priority(8), _low_priority(8), _session_limit_count(0), _possible_session_types({ session_types::message_line }),
		_start_code_value(start_code_value), _end_code_value(end_code_value), _compress_block_size(1024),
		_drop_connection_time(10)
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

	void messaging_server::set_compress_block_size(const unsigned short& compress_block_size)
	{
		_compress_block_size = compress_block_size;
	}

	void messaging_server::set_drop_connection_time(const unsigned short& drop_connection_time)
	{
		_drop_connection_time = drop_connection_time;
	}

	void messaging_server::set_connection_key(const wstring& connection_key)
	{
		_connection_key = connection_key;
	}

	void messaging_server::set_acceptable_target_ids(const vector<wstring>& acceptable_target_ids)
	{
		_acceptable_target_ids = acceptable_target_ids;
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

	void messaging_server::set_session_limit_count(const size_t& session_limit_count)
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

	void messaging_server::set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<uint8_t>&)>& notification)
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

		_thread = make_shared<thread>([&]()
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
					catch (const overflow_error&) 
					{ 
						if (_io_context != nullptr) 
						{ 
							_io_context->reset(); 
							logger::handle().write(logging_level::information, fmt::format(L"reset messaging_server({})", _source_id));
						} 						
					}
					catch (const runtime_error&) 
					{ 
						if (_io_context != nullptr) 
						{ 
							_io_context->reset(); 
							logger::handle().write(logging_level::information, fmt::format(L"reset messaging_server({})", _source_id));
						}  	
					}
					catch (const exception&) 
					{ 
						if (_io_context != nullptr) 
						{ 
							_io_context->reset(); 
							logger::handle().write(logging_level::information, fmt::format(L"reset messaging_server({})", _source_id));
						} 	
					}
					catch (...)
					{ 
						if (_io_context != nullptr) 
						{ 
							_io_context->reset(); 
							logger::handle().write(logging_level::information, fmt::format(L"reset messaging_server({})", _source_id));
						} 	
					}
				}
			});
	}

	void messaging_server::wait_stop(const unsigned int& seconds)
	{
		if (!_promise_status.has_value())
		{
			_promise_status = { promise<bool>() };
		}

		_future_status = _promise_status.value().get_future();

		if (seconds == 0)
		{
			_future_status.wait();
			_promise_status.reset();
			return;
		}

		_future_status.wait_for(chrono::seconds(seconds));
		_promise_status.reset();
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
				_acceptor.reset();
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
			if (_thread != nullptr)
			{
				if (_thread->joinable() && !_io_context->stopped())
				{
					_io_context->stop();
					_thread->join();
				}

				_thread.reset();
			}
			
			_io_context.reset();
		}

		if (_promise_status.has_value())
		{
			_promise_status.value().set_value(true);
		}
	}

	void messaging_server::disconnect(const wstring& target_id, const wstring& target_sub_id)
	{
		vector<shared_ptr<messaging_session>>::iterator target = _sessions.begin();
		while (target != _sessions.end())
		{
			if (*target == nullptr)
			{
				target++;
				continue;
			}

			if ((*target)->target_id() != target_id ||
				(*target)->target_sub_id() != target_sub_id)
			{
				target++;
				continue;
			}

			target = _sessions.erase(target);
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
	bool messaging_server::send(const json::value& message, optional<session_types> type)
#else
	bool messaging_server::send(const container::value_container& message, optional<session_types> type)
#endif
	{
#ifndef __USE_TYPE_CONTAINER__
		return send(make_shared<json::value>(message), type);
#else
		return send(make_shared<container::value_container>(message), type);
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	bool messaging_server::send(shared_ptr<json::value> message, optional<session_types> type)
#else
	bool messaging_server::send(shared_ptr<container::value_container> message, optional<session_types> type)
#endif
	{
		if (message == nullptr)
		{
			return false;
		}

		bool result = false;
		for (auto& session : _sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			if (session->get_confirm_status() != connection_conditions::confirmed)
			{
				continue;
			}

			if (type.has_value() && session->get_session_type() != type.value())
			{
				continue;
			}

			result |= session->send(message);
		}

		return result;
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

			if (session->get_confirm_status() != connection_conditions::confirmed)
			{
				continue;
			}

			session->send_files(message);
		}
	}

	void messaging_server::send_binary(const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data)
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

			if (session->get_confirm_status() != connection_conditions::confirmed)
			{
				continue;
			}

			session->send_binary(target_id, target_sub_id, data);
		}
	}

	void messaging_server::send_binary(const wstring& source_id, const wstring& source_sub_id, const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data)
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

			if (session->get_confirm_status() != connection_conditions::confirmed)
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

				shared_ptr<messaging_session> session = make_shared<messaging_session>(
					_source_id, _connection_key, socket, _start_code_value, _end_code_value);
				if (session == nullptr)
				{
					wait_connection();

					return;
				}

				if (_session_limit_count > 0)
				{
					session->set_kill_code(_sessions.size() >= _session_limit_count);
				}

				session->set_acceptable_target_ids(_acceptable_target_ids);
				session->set_ignore_target_ids(_ignore_target_ids);
				session->set_ignore_snipping_targets(_ignore_snipping_targets);
				session->set_connection_notification(bind(&messaging_server::connect_condition, this, placeholders::_1, placeholders::_2));
				session->set_message_notification(bind(&messaging_server::received_message, this, placeholders::_1));
				session->set_file_notification(_received_file);
				session->set_binary_notification(bind(&messaging_server::received_binary, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5));

				session->start(_encrypt_mode, _compress_mode, _compress_block_size, _possible_session_types, _high_priority, _normal_priority, _low_priority, _drop_connection_time);

				_sessions.push_back(session);

				_thread_pool->push(make_shared<job>(priorities::low, bind(&messaging_server::check_confirm_condition, this)));

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

			if (session->get_confirm_status() == connection_conditions::expired)
			{
				connect_condition(session, false);
			}
		}

		_thread_pool->push(make_shared<job>(priorities::low, bind(&messaging_server::check_confirm_condition, this)));

		return true;
	}

	void messaging_server::connect_condition(shared_ptr<messaging_session> target, const bool& condition)
	{
		if (target == nullptr)
		{
			return;
		}

		logger::handle().write(logging_level::sequence,
			fmt::format(L"{} a client with manager: {}[{}]", 
				condition ? L"connected" : L"disconnected",
				target->target_id(), target->target_sub_id())
    	);

		if (!condition)
		{
			auto iter = find(_sessions.begin(), _sessions.end(), target);
			if (iter != _sessions.end())
			{
				_sessions.erase(iter);
			}
		}

		if(_connection != nullptr && _thread_pool != nullptr)
		{
			auto result = async(launch::async, _connection, target->target_id(), target->target_sub_id(), condition);
		}
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

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		auto target_id = (*message)[HEADER][TARGET_ID].as_string();
#else
		auto target_id = converter::to_wstring((*message)[HEADER][TARGET_ID].as_string());
#endif
#else
		auto target_id = message->target_id();
#endif

		if (target_id != _source_id)
		{
			logger::handle().write(logging_level::sequence,
				fmt::format(L"attempt to transfer message to {}", 
					target_id));

			if (!send(message))
			{
				logger::handle().write(logging_level::sequence,
					fmt::format(L"there is no target id on server: {}", 
						target_id));

#ifndef __USE_TYPE_CONTAINER__
				shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

				(*container)[HEADER][SOURCE_ID] = (*message)[HEADER][TARGET_ID];
				(*container)[HEADER][SOURCE_SUB_ID] = (*message)[HEADER][TARGET_SUB_ID];
				(*container)[HEADER][TARGET_ID] = (*message)[HEADER][SOURCE_ID];
				(*container)[HEADER][TARGET_SUB_ID] = (*message)[HEADER][SOURCE_SUB_ID];
				(*container)[HEADER][MESSAGE_TYPE] = json::value::string(CANNOT_SEND_MESSAGE);

				(*container)[DATA][INDICATION_ID] = (*message)[DATA][INDICATION_ID];
				(*container)[DATA][MESSAGE_TYPE] = (*message)[HEADER][MESSAGE_TYPE];
    			(*container)[DATA][RESPONSE] = json::value::boolean(false);
#else
				shared_ptr<container::value_container> container = message->copy(false);
				container->swap_header();
#ifdef _WIN32
				container->set_message_type(CANNOT_SEND_MESSAGE);
#else
				container->set_message_type(converter::to_wstring(CANNOT_SEND_MESSAGE));
#endif
				container << make_shared<container::string_value>(L"indication_id", message->get_value(L"indication_id")->to_string());
				container << make_shared<container::string_value>(L"message_type", message->message_type());
				container << make_shared<container::bool_value>(L"response", false);
#endif

    			send(container);
			}

			return;
		}

		if (_received_message != nullptr)
		{
			auto result = async(launch::async, _received_message, message);
		}
	}

	void messaging_server::received_binary(const wstring& source_id, const wstring& source_sub_id, const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data)
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
