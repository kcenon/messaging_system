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

#include "values/bool_value.h"
#include "values/string_value.h"
#include "values/container_value.h"

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
#include "binary_combiner.h"

#include <future>

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace container;
	using namespace converting;
	using namespace encrypting;
	using namespace compressing;
	using namespace file_handler;
	using namespace binary_parser;

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
				asio::error_code ec;
				_socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
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

	void messaging_session::set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification)
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
		
	void messaging_session::set_specific_compress_sequence(const function<vector<uint8_t>(const vector<uint8_t>&, const bool&)>& specific_compress_sequence)
	{
		_specific_compress_sequence = specific_compress_sequence;	
	}
		
	void messaging_session::set_specific_encryp_sequence(const function<vector<uint8_t>(const vector<uint8_t>&, const bool&)>& specific_encrypt_sequence)
	{
		_specific_encrypt_sequence = specific_encrypt_sequence;
	}

	const connection_conditions messaging_session::get_confirm_status(void)
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
		_thread_pool = make_shared<threads::thread_pool>(L"messaging_session");

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
		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"echo",
			vector<shared_ptr<container::value>> {});

		send(container);
	}

	bool messaging_session::send(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

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

		auto serialize = message->serialize();
		auto serialize_array = converter::to_array(serialize);

		logger::handle().write(logging_level::packet, fmt::format(L"send: {}", serialize));

		send_packet_job(serialize_array);

		return true;
	}

	void messaging_session::send_files(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_session_type != session_types::file_line)
		{
			return;
		}

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

		if (!_bridge_line && !target_sub_id.empty() && target_sub_id != _target_sub_id)
		{
			return;
		}

		vector<uint8_t> result;
		combiner::append(result, converter::to_array(_source_id));
		combiner::append(result, converter::to_array(_source_sub_id));
		combiner::append(result, converter::to_array(target_id));
		combiner::append(result, converter::to_array(target_sub_id));
		combiner::append(result, data);

		logger::handle().write(logging_level::packet, fmt::format(L"send binary: source[{}:{}] -> target[{}:{}], {} bytes", _source_id, _source_sub_id, target_id, target_sub_id, data.size()));

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
		combiner::append(result, converter::to_array(source_id));
		combiner::append(result, converter::to_array(source_sub_id));
		combiner::append(result, converter::to_array(target_id));
		combiner::append(result, converter::to_array(target_sub_id));
		combiner::append(result, data);

		logger::handle().write(logging_level::packet, fmt::format(L"send binary: source[{}:{}] -> target[{}:{}], {} bytes", source_id, source_sub_id, target_id, target_sub_id, data.size()));

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

	void messaging_session::normal_message(shared_ptr<container::value_container> message)
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

	void messaging_session::connection_message(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			logger::handle().write(logging_level::error, L"cannot parse connection message with empty message");

			return;
		}

		_target_id = message->source_id();
		_session_type = (session_types)message->get_value(L"session_type")->to_short();
		_bridge_line = message->get_value(L"bridge_mode")->to_boolean();
		if (_session_type != session_types::binary_line)
		{
			_auto_echo = message->get_value(L"auto_echo")->to_boolean();
			_auto_echo_interval_seconds = message->get_value(L"auto_echo_interval_seconds")->to_ushort();
		}

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
		if (!same_key_check(message->get_value(L"connection_key")))
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

		send_packet_job(converter::to_array(container->serialize()));

		_confirm = connection_conditions::confirmed;
		
		if(_connection != nullptr)
		{
			auto result = async(launch::async, _connection, get_ptr(), true);
		}
	}

	void messaging_session::request_files(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			return;
		}

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
	}

	void messaging_session::echo_message(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			return;
		}

		vector<shared_ptr<value>> response = (*message)[L"response"];
		if (!response.empty())
		{
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));

			return;
		}

		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();

		container << make_shared<bool_value>(L"response", true);

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
		
		auto encrypt_key = cryptor::create_key();
		_key = encrypt_key.first;
		_iv = encrypt_key.second;

		logger::handle().write(logging_level::sequence, L"generated encrypt key");
	}

	bool messaging_session::same_key_check(shared_ptr<container::value> key)
	{
		if (key != nullptr && _connection_key == key->to_string())
		{
			return true;
		}

		logger::handle().write(logging_level::information, L"ignored this line = \"unknown connection key\"");

		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, 
			L"confirm_connection", vector<shared_ptr<container::value>> {
				make_shared<container::bool_value>(L"confirm", false),
				make_shared<container::string_value>(L"reason", L"ignored this line = \"unknown connection key\"")
		});

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

		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, 
			L"confirm_connection", vector<shared_ptr<container::value>> {
				make_shared<container::bool_value>(L"confirm", false),
				make_shared<container::string_value>(L"reason", L"ignored this line = \"cannot use same id with server\"")
		});

		send(container);

		return false;
	}
}
