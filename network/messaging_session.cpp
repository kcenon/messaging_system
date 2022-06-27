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

#include "messaging_session.h"

#ifdef __USE_TYPE_CONTAINER__
#include "values/bool_value.h"
#include "values/string_value.h"
#include "values/container_value.h"
#endif

#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "data_lengths.h"
#include "file_handler.h"

#include <future>

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace threads;

#ifdef __USE_TYPE_CONTAINER__
	using namespace container;
#endif

	using namespace converting;
	using namespace encrypting;
	using namespace compressing;
	using namespace file_handler;

	messaging_session::messaging_session(const wstring& source_id, const wstring& connection_key, asio::ip::tcp::socket& socket,
		const unsigned char& start_code_value, const unsigned char& end_code_value)
		: data_handling(start_code_value, end_code_value), _bridge_line(false),
		_source_id(source_id), _source_sub_id(L""), _target_id(L""), _target_sub_id(L""), 
		_connection_key(connection_key), _connection(nullptr), _kill_code(false),
		_socket(make_shared<asio::ip::tcp::socket>(move(socket))), _auto_echo_interval_seconds(1), _auto_echo(false)
	{
		_socket->set_option(asio::ip::tcp::no_delay(true));
		_socket->set_option(asio::socket_base::keep_alive(true));
		_socket->set_option(asio::socket_base::receive_buffer_size(buffer_size));

		_source_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->local_endpoint().address().to_string()), _socket->local_endpoint().port());
		_target_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port());

		_message_handlers.insert({ L"request_connection", bind(&messaging_session::connection_message, this, placeholders::_1) });
		_message_handlers.insert({ L"request_files", bind(&messaging_session::request_files, this, placeholders::_1) });
		_message_handlers.insert({ L"echo", bind(&messaging_session::echo_message, this, placeholders::_1) });
	}

	messaging_session::~messaging_session(void)
	{
		if (_socket != nullptr)
		{
			if (_socket->is_open())
			{
				_socket->close();
			}
			_socket.reset();
		}

		stop();
	}

	shared_ptr<messaging_session> messaging_session::get_ptr(void)
	{
		return shared_from_this();
	}

	void messaging_session::set_kill_code(const bool& kill_code)
	{
		_kill_code = kill_code;
	}

	void messaging_session::set_acceptable_target_ids(const vector<wstring>& acceptable_target_ids)
	{
		_acceptable_target_ids = acceptable_target_ids;
	}

	void messaging_session::set_ignore_target_ids(const vector<wstring>& ignore_target_ids)
	{
		_ignore_target_ids = ignore_target_ids;
	}

	void messaging_session::set_ignore_snipping_targets(const vector<wstring>& ignore_snipping_targets)
	{
		_ignore_snipping_targets = ignore_snipping_targets;
	}

	void messaging_session::set_connection_notification(const function<void(shared_ptr<messaging_session>, const bool&)>& notification)
	{
		_connection = notification;
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_session::set_message_notification(const function<void(shared_ptr<json::value>)>& notification)
#else
	void messaging_session::set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification)
#endif
	{
		_received_message = notification;
	}

	void messaging_session::set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification)
	{
		_received_file = notification;
	}

	void messaging_session::set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<uint8_t>&)>& notification)
	{
		_received_data = notification;
	}

	const connection_conditions messaging_session::get_confirom_status(void)
	{
		return _confirm;
	}

	const session_types messaging_session::get_session_type(void)
	{
		return _session_type;
	}

	const wstring messaging_session::target_id(void)
	{
		return _target_id;
	}

	const wstring messaging_session::target_sub_id(void)
	{
		return _target_sub_id;
	}

	void messaging_session::start(const bool& encrypt_mode, const bool& compress_mode, const unsigned short& compress_block_size, 
		const vector<session_types>& possible_session_types, const unsigned short& high_priority, const unsigned short& normal_priority, 
		const unsigned short& low_priority, const unsigned short& drop_connection_time)
	{
		stop();

		_encrypt_mode = encrypt_mode;
		_compress_mode = compress_mode;
		_compress_block_size = compress_block_size;
		_drop_connection_time = drop_connection_time;
		_possible_session_types = possible_session_types;
		_thread_pool = make_shared<threads::thread_pool>();

		_thread_pool->append(make_shared<thread_worker>(priorities::top), true);
		for (unsigned short high = 0; high < high_priority; ++high)
		{
			_thread_pool->append(make_shared<thread_worker>(priorities::high, vector<priorities> { priorities::normal, priorities::low }), true);
		}
		for (unsigned short normal = 0; normal < normal_priority; ++normal)
		{
			_thread_pool->append(make_shared<thread_worker>(priorities::normal, vector<priorities> { priorities::high, priorities::low }), true);
		}
		for (unsigned short low = 0; low < low_priority; ++low)
		{
			_thread_pool->append(make_shared<thread_worker>(priorities::low, vector<priorities> { priorities::high, priorities::normal }), true);
		}

		read_start_code(_socket);

		logger::handle().write(logging_level::information, fmt::format(L"started session: {}:{}", 
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port()));
	}

	void messaging_session::stop(void)
	{
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}
	}

	void messaging_session::echo(void)
	{
#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));
#else
		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"echo",
			vector<shared_ptr<container::value>> {});
#endif

		send(container);
	}

#ifndef __USE_TYPE_CONTAINER__
	bool messaging_session::send(shared_ptr<json::value> message)
#else
	bool messaging_session::send(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return false;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		if (!_bridge_line && (*message)[HEADER][TARGET_ID].as_string() != _target_id && 
			!contained_snipping_target((*message)[HEADER][TARGET_ID].as_string()))
		{
			return false;
		}
		
		if (!_bridge_line && !contained_snipping_target((*message)[HEADER][TARGET_ID].as_string()) && 
			!(*message)[HEADER][TARGET_SUB_ID].is_null() && !(*message)[HEADER][TARGET_SUB_ID].as_string().empty() && 
			(*message)[HEADER][TARGET_SUB_ID].as_string() != _target_sub_id)
		{
			return false;
		}
#else
		if (!_bridge_line && (*message)[HEADER][TARGET_ID].as_string() != converter::to_string(_target_id) && 
			!contained_snipping_target(converter::to_wstring((*message)[HEADER][TARGET_ID].as_string())))
		{
			return false;
		}

		if (!_bridge_line && !contained_snipping_target(converter::to_wstring((*message)[HEADER][TARGET_ID].as_string())) &&
			!(*message)[HEADER][TARGET_SUB_ID].is_null() && !(*message)[HEADER][TARGET_SUB_ID].as_string().empty() && 
			(*message)[HEADER][TARGET_SUB_ID].as_string() != converter::to_string(_target_sub_id))
		{
			return false;
		}
#endif
#else
		if (!_bridge_line && message->target_id() != _target_id && 
			!contained_snipping_target(message->target_id()))
		{
			return false;
		}

		if (!_bridge_line && !contained_snipping_target(message->target_id()) && 
			!message->target_sub_id().empty() && message->target_sub_id() != _target_sub_id)
		{
			return false;
		}
#endif

		auto serialize = message->serialize();
		auto serialize_array = converter::to_array(serialize);

#ifdef __USE_TYPE_CONTAINER__
		logger::handle().write(logging_level::packet, fmt::format(L"send: {}", serialize));
#else
#ifdef _WIN32
		logger::handle().write(logging_level::packet, fmt::format(L"send: {}", serialize));
#else
		logger::handle().write(logging_level::packet, converter::to_wstring(fmt::format("send: {}", serialize)));
#endif
#endif

		send_packet_job(serialize_array);

		return true;
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_session::send_files(shared_ptr<json::value> message)
#else
	void messaging_session::send_files(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

		if (_session_type != session_types::file_line)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		if (_target_id != (*message)[HEADER][SOURCE_ID].as_string() && 
			_target_sub_id != (*message)[HEADER][SOURCE_SUB_ID].as_string())
		{
			return;
		}

		if ((*message)[HEADER][SOURCE_ID].is_null())
		{
			(*message)[HEADER][SOURCE_ID] = json::value::string(_source_id);
			(*message)[HEADER][SOURCE_SUB_ID] = json::value::string(_source_sub_id);
		}
#else
		if (converter::to_string(_target_id) != (*message)[HEADER][SOURCE_ID].as_string() && 
			converter::to_string(_target_sub_id) != (*message)[HEADER][SOURCE_SUB_ID].as_string())
		{
			return;
		}

		if ((*message)[HEADER][SOURCE_ID].is_null())
		{
			(*message)[HEADER][SOURCE_ID] = json::value::string(converter::to_string(_source_id));
			(*message)[HEADER][SOURCE_SUB_ID] = json::value::string(converter::to_string(_source_sub_id));
		}
#endif

		auto& files = (*message)[DATA][FILES].as_array();
		for (int index = 0; index < files.size(); ++index)
		{
			shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

			(*container)[HEADER][SOURCE_ID] = (*message)[HEADER][TARGET_ID];
			(*container)[HEADER][SOURCE_SUB_ID] = (*message)[HEADER][TARGET_SUB_ID];
			(*container)[HEADER][TARGET_ID] = (*message)[HEADER][GATEWAY_SOURCE_ID];
			(*container)[HEADER][TARGET_SUB_ID] = (*message)[HEADER][GATEWAY_SOURCE_SUB_ID];
			(*container)[HEADER][MESSAGE_TYPE] = json::value::string(REQUEST_FILE);

			(*container)[DATA][INDICATION_ID] = (*message)[DATA][INDICATION_ID];
			(*container)[DATA][SOURCE] = files[index][SOURCE];
			(*container)[DATA][TARGET] = files[index][TARGET];

			send_file_job(converter::to_array(container->serialize()));
		}
#else
		if (_target_id != message->source_id() && _target_sub_id != message->source_sub_id())
		{
			return;
		}

		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
		container->set_target(
			message->get_value(L"gateway_source_id")->to_string(), 
			message->get_value(L"gateway_source_sub_id")->to_string());
		container->set_message_type(L"request_files");

		vector<shared_ptr<container::value>> files = message->value_array(L"file");
		for (auto& file : files)
		{
			container << make_shared<container::string_value>(L"indication_id", message->get_value(L"indication_id")->to_string());
			container << make_shared<container::string_value>(L"source", (*file)[L"source"]->to_string());
			container << make_shared<container::string_value>(L"target", (*file)[L"target"]->to_string());

			send_file_job(container->serialize_array());
			container->clear_value();
		}
#endif
	}

	void messaging_session::send_binary(const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_session_type != session_types::binary_line)
		{
			return;
		}

		if (!_bridge_line && target_id != _target_id)
		{
			return;
		}

		if (!_bridge_line && !target_sub_id.empty() && target_id != _target_sub_id)
		{
			return;
		}

		vector<uint8_t> result;
		append_binary_on_packet(result, converter::to_array(_source_id));
		append_binary_on_packet(result, converter::to_array(_source_sub_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		append_binary_on_packet(result, data);

		send_binary_job(result);
	}

	void messaging_session::send_binary(const wstring& source_id, const wstring& source_sub_id, const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_session_type != session_types::binary_line)
		{
			return;
		}

		if (!_bridge_line && target_id != _target_id)
		{
			return;
		}

		if (!_bridge_line && !target_sub_id.empty() && target_id != _target_sub_id)
		{
			return;
		}

		vector<uint8_t> result;
		append_binary_on_packet(result, converter::to_array(source_id));
		append_binary_on_packet(result, converter::to_array(source_sub_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		append_binary_on_packet(result, data);

		send_binary_job(result);
	}

	void messaging_session::disconnected(void)
	{
		stop();

		if(_connection != nullptr)
		{
			auto result = async(launch::async, _connection, get_ptr(), false);
		}
	}

	bool messaging_session::contained_snipping_target(const wstring& snipping_target)
	{
		auto target = find(_snipping_targets.begin(), _snipping_targets.end(), snipping_target);
		if (target == _snipping_targets.end())
		{
			return false;
		}

		return true;
	}

	void messaging_session::send_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	void messaging_session::send_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::file_mode, data);
	}

	void messaging_session::send_binary_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::binary_mode, data);
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_session::normal_message(shared_ptr<json::value> message)
#else
	void messaging_session::normal_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			return;
		}

		if (_received_message)
		{
			auto result = async(launch::async, _received_message, message);
		}
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_session::connection_message(shared_ptr<json::value> message)
#else
	void messaging_session::connection_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			logger::handle().write(logging_level::error, L"cannot parse connection message with empty message");

			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		_target_id = (*message)[HEADER][SOURCE_ID].as_string();
		_session_type = (session_types)(*message)[DATA][L"session_type"].as_integer();
		_bridge_line = (*message)[DATA][L"bridge_mode"].as_bool();
		_auto_echo = (*message)[DATA][L"auto_echo"].as_bool();
		_auto_echo_interval_seconds = (unsigned short)(*message)[DATA][L"auto_echo_interval_seconds"].as_integer();
#else
		_target_id = converter::to_wstring((*message)[HEADER][SOURCE_ID].as_string());
		_session_type = (session_types)(*message)[DATA]["session_type"].as_integer();
		_bridge_line = (*message)[DATA]["bridge_mode"].as_bool();
		_auto_echo = (*message)[DATA]["auto_echo"].as_bool();
		_auto_echo_interval_seconds = (unsigned short)(*message)[DATA]["auto_echo_interval_seconds"].as_integer();
#endif
#else
		_target_id = message->source_id();
		_session_type = (session_types)message->get_value(L"session_type")->to_short();
		_bridge_line = message->get_value(L"bridge_mode")->to_boolean();
		_auto_echo = message->get_value(L"auto_echo")->to_boolean();
		_auto_echo_interval_seconds = message->get_value(L"auto_echo_interval_seconds")->to_ushort();
#endif

		auto iter = find_if(_possible_session_types.begin(), _possible_session_types.end(), 
			[&](const session_types& type) 
			{
				return type == _session_type;
			});
		if (iter == _possible_session_types.end())
		{
			_confirm = connection_conditions::expired;
			logger::handle().write(logging_level::error, L"expired this line = \"cannot accept unknown session type\"");

			return;
		}

		if (_source_id == _target_id)
		{
			_confirm = connection_conditions::expired;
			logger::handle().write(logging_level::error, L"expired this line = \"cannot use same id with server\"");

			return;
		}

		if (!_acceptable_target_ids.empty())
		{
			auto target = find(_acceptable_target_ids.begin(), _acceptable_target_ids.end(), _target_id);
			if (target == _acceptable_target_ids.end())
			{
				_confirm = connection_conditions::expired;
				logger::handle().write(logging_level::error, L"expired this line = \"cannot connect with unknown id on server\"");

				return;
			}
		}

		if (!_ignore_target_ids.empty())
		{
			auto target = find(_ignore_target_ids.begin(), _ignore_target_ids.end(), _target_id);
			if (target != _ignore_target_ids.end())
			{
				_confirm = connection_conditions::expired;
				logger::handle().write(logging_level::error, L"expired this line = \"cannot connect with ignored id on server\"");

				return;
			}
		}

		if (_kill_code)
		{
			_confirm = connection_conditions::expired;
			logger::handle().write(logging_level::error, L"expired this line = \"set kill code\"");

			return;
		}

		// check connection key
#ifndef __USE_TYPE_CONTAINER__
		if (!same_key_check((*message)[DATA][CONNECTION_KEY]))
#else
		if (!same_key_check(message->get_value(L"connection_key")))
#endif
		{
			_confirm = connection_conditions::expired;

			return;
		}

		logger::handle().write(logging_level::sequence, L"confirmed connection key");

		// compare both session id an client id
		if (!same_id_check())
		{
			_confirm = connection_conditions::expired;

			return;
		}

		logger::handle().write(logging_level::sequence, L"confirmed client id");

		generate_key();

		// check snipping target list
#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

#ifdef _WIN32
		(*container)[HEADER][SOURCE_ID] = json::value::string(_source_id);
		(*container)[HEADER][SOURCE_SUB_ID] = json::value::string(_source_sub_id);
		(*container)[HEADER][TARGET_ID] = json::value::string(_target_id);
		(*container)[HEADER][TARGET_SUB_ID] = json::value::string(_target_sub_id);
		(*container)[HEADER][MESSAGE_TYPE] = json::value::string(CONFIRM_CONNECTION);
#else
		(*container)[HEADER][SOURCE_ID] = json::value::string(converter::to_string(_source_id));
		(*container)[HEADER][SOURCE_SUB_ID] = json::value::string(converter::to_string(_source_sub_id));
		(*container)[HEADER][TARGET_ID] = json::value::string(converter::to_string(_target_id));
		(*container)[HEADER][TARGET_SUB_ID] = json::value::string(converter::to_string(_target_sub_id));
		(*container)[HEADER][MESSAGE_TYPE] = json::value::string(CONFIRM_CONNECTION);
#endif

		_snipping_targets.clear();

		int index2 = 0;
#ifdef _WIN32
		(*container)[DATA][SNIPPING_TARGETS] = json::value::array();
		auto& snipping_targets = (*message)[DATA][SNIPPING_TARGETS].as_array();
#else
		(*container)[DATA][SNIPPING_TARGETS] = json::value::array();
		auto& snipping_targets = (*message)[DATA][SNIPPING_TARGETS].as_array();
#endif
		for (int index = 0; index < snipping_targets.size(); ++index)
		{
#ifdef _WIN32
			auto target = find(_ignore_snipping_targets.begin(), _ignore_snipping_targets.end(), snipping_targets[index].as_string());
#else
			auto target = find(_ignore_snipping_targets.begin(), _ignore_snipping_targets.end(), converter::to_wstring(snipping_targets[index].as_string()));
#endif
			if (target != _ignore_snipping_targets.end())
			{
				continue;
			}

#ifdef _WIN32
			_snipping_targets.push_back(snipping_targets[index].as_string());
			(*container)[DATA][SNIPPING_TARGETS][index2++] = json::value::string(snipping_targets[index].as_string());
#else
			_snipping_targets.push_back(converter::to_wstring(snipping_targets[index].as_string()));
			(*container)[DATA][SNIPPING_TARGETS][index2++] = json::value::string(snipping_targets[index].as_string());
#endif
		}

#ifdef _WIN32
		(*container)[DATA][L"confirm"] = json::value::boolean(true);
		if (_encrypt_mode)
		{
			(*container)[DATA][L"key"] = json::value::string(_key);
			(*container)[DATA][L"iv"] = json::value::string(_iv);
		}
		(*container)[DATA][ENCRYPT_MODE] = json::value::boolean(_encrypt_mode);
#else
		(*container)[DATA]["confirm"] = json::value::boolean(true);
		if (_encrypt_mode)
		{
			(*container)[DATA]["key"] = json::value::string(converter::to_string(_key));
			(*container)[DATA]["iv"] = json::value::string(converter::to_string(_iv));
		}
		(*container)[DATA][ENCRYPT_MODE] = json::value::boolean(_encrypt_mode);
#endif
#else
		shared_ptr<value> acceptable_snipping_targets = make_shared<container::container_value>(L"snipping_targets");

		_snipping_targets.clear();
		vector<shared_ptr<value>> snipping_targets = message->get_value(L"snipping_targets")->children();
		for (auto& snipping_target : snipping_targets)
		{
			if (snipping_target == nullptr)
			{
				continue;
			}

			if (snipping_target->name() != L"snipping_target")
			{
				continue;
			}

			auto target = find(_ignore_snipping_targets.begin(), _ignore_snipping_targets.end(), snipping_target->to_string());
			if (target != _ignore_snipping_targets.end())
			{
				continue;
			}

			_snipping_targets.push_back(snipping_target->to_string());

			acceptable_snipping_targets->add(make_shared<container::string_value>(L"snipping_target", snipping_target->to_string()));
		}

		vector<shared_ptr<container::value>> temp;
		temp.push_back(make_shared<container::bool_value>(L"confirm", true));
		temp.push_back(make_shared<container::bool_value>(L"encrypt_mode", _encrypt_mode));
		temp.push_back(acceptable_snipping_targets);
		if (_encrypt_mode)
		{
			temp.push_back(make_shared<container::string_value>(L"key", _key));
			temp.push_back(make_shared<container::string_value>(L"iv", _iv));
		}

		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, 
			L"confirm_connection", temp);
#endif

		send_packet_job(converter::to_array(container->serialize()));

		_confirm = connection_conditions::confirmed;
		
		if(_connection != nullptr)
		{
			auto result = async(launch::async, _connection, get_ptr(), true);
		}
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_session::request_files(shared_ptr<json::value> message)
#else
	void messaging_session::request_files(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		auto& files = (*message)[DATA][FILES].as_array();
		for (int index = 0; index < files.size(); ++index)
		{
			shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

			(*container)[HEADER][SOURCE_ID] = (*message)[HEADER][TARGET_ID];
			(*container)[HEADER][SOURCE_SUB_ID] = (*message)[HEADER][TARGET_SUB_ID];
			(*container)[HEADER][TARGET_ID] = (*message)[HEADER][SOURCE_ID];
			(*container)[HEADER][TARGET_SUB_ID] = (*message)[HEADER][SOURCE_SUB_ID];
			(*container)[HEADER][MESSAGE_TYPE] = json::value::string(REQUEST_FILE);

			(*container)[DATA][INDICATION_ID] = (*message)[DATA][INDICATION_ID];
			(*container)[DATA][SOURCE] = files[index][SOURCE];
			(*container)[DATA][TARGET] = files[index][TARGET];

			send_file_job(converter::to_array(container->serialize()));
		}
#else
		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
		container->set_message_type(L"request_file");

		vector<shared_ptr<container::value>> files = message->value_array(L"file");
		for (auto& file : files)
		{
			container << make_shared<container::string_value>(L"indication_id", message->get_value(L"indication_id")->to_string());
			container << make_shared<container::string_value>(L"source", (*file)[L"source"]->to_string());
			container << make_shared<container::string_value>(L"target", (*file)[L"target"]->to_string());

			send_file_job(container->serialize_array());
			container->clear_value();
		}
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_session::echo_message(shared_ptr<json::value> message)
#else
	void messaging_session::echo_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			return;
		}

#ifdef __USE_TYPE_CONTAINER__
		vector<shared_ptr<value>> response = (*message)[L"response"];
		if (!response.empty())
		{
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));

			return;
		}

		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();

		container << make_shared<bool_value>(L"response", true);
#else
		if (!(*message)[DATA][RESPONSE].is_null())
		{
#ifdef _WIN32
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));
#else
			logger::handle().write(logging_level::information, converter::to_wstring(fmt::format("received echo: {}", message->serialize())));
#endif
			return;
		}

		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

		(*container)[HEADER][SOURCE_ID] = (*message)[HEADER][TARGET_ID];
		(*container)[HEADER][SOURCE_SUB_ID] = (*message)[HEADER][TARGET_SUB_ID];
		(*container)[HEADER][TARGET_ID] = (*message)[HEADER][SOURCE_ID];
		(*container)[HEADER][TARGET_SUB_ID] = (*message)[HEADER][SOURCE_SUB_ID];
		(*container)[HEADER][MESSAGE_TYPE] = (*message)[HEADER][MESSAGE_TYPE];

		(*container)[DATA] = (*message)[DATA];
		(*container)[DATA][RESPONSE] = json::value::boolean(true);
#endif

		send_packet_job(converter::to_array(container->serialize()));
	}

	void messaging_session::generate_key(void)
	{
		if (!_encrypt_mode)
		{
			_key = L"";
			_iv = L"";

			return;
		}

		logger::handle().write(logging_level::sequence, L"attempt to generate encrypt key");
		
		auto encrypt_key = encryptor::create_key();
		_key = encrypt_key.first;
		_iv = encrypt_key.second;

		logger::handle().write(logging_level::sequence, L"generated encrypt key");
	}

#ifndef __USE_TYPE_CONTAINER__
	bool messaging_session::same_key_check(const json::value& key)
#else
	bool messaging_session::same_key_check(shared_ptr<container::value> key)
#endif
	{
#ifdef __USE_TYPE_CONTAINER__
		if (key != nullptr && _connection_key == key->to_string())
#else
#ifdef _WIN32
		if (!key.is_null() && _connection_key == key.as_string())
#else
		if (!key.is_null() && converter::to_string(_connection_key) == key.as_string())
#endif
#endif
		{
			return true;
		}

		logger::handle().write(logging_level::information, L"ignored this line = \"unknown connection key\"");

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));
#else
		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, 
			L"confirm_connection", vector<shared_ptr<container::value>> {
				make_shared<container::bool_value>(L"confirm", false),
				make_shared<container::string_value>(L"reason", L"ignored this line = \"unknown connection key\"")
		});
#endif

		send(container);

		return false;
	}

	bool messaging_session::same_id_check(void)
	{
		if (_target_id != _source_id)
		{
			return true;
		}

		logger::handle().write(logging_level::information, L"ignored this line = \"cannot use same id with server\"");

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));
#else
		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, 
			L"confirm_connection", vector<shared_ptr<container::value>> {
				make_shared<container::bool_value>(L"confirm", false),
				make_shared<container::string_value>(L"reason", L"ignored this line = \"cannot use same id with server\"")
		});
#endif

		send(container);

		return false;
	}
}
