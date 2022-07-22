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

#include "messaging_client.h"

#include "value.h"
#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/short_value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"
#include "values/container_value.h"

#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "data_lengths.h"
#include "file_handler.h"
#include "binary_combiner.h"

#include <future>
#include <functional>

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

	messaging_client::messaging_client(const wstring& source_id, const unsigned char& start_code_value, const unsigned char& end_code_value)
		: data_handling(start_code_value, end_code_value), _auto_echo(false), _bridge_line(false),
		_io_context(nullptr), _auto_echo_interval_seconds(1), _connection(nullptr),
		_connection_key(L"connection_key"), _source_id(source_id), _source_sub_id(L""), 
		_target_id(L"unknown"), _target_sub_id(L"0.0.0.0:0"), _socket(nullptr),
		_session_type(session_types::message_line), _thread(nullptr)
	{
		_message_handlers.insert({ L"confirm_connection", bind(&messaging_client::confirm_message, this, placeholders::_1) });
		_message_handlers.insert({ L"request_files", bind(&messaging_client::request_files, this, placeholders::_1) });
		_message_handlers.insert({ L"echo", bind(&messaging_client::echo_message, this, placeholders::_1) });
	}

	messaging_client::~messaging_client(void)
	{
		stop();
	}

	shared_ptr<messaging_client> messaging_client::get_ptr(void)
	{
		return shared_from_this();
	}

	wstring messaging_client::source_id(void) const
	{
		return _source_id;
	}

	wstring messaging_client::source_sub_id(void) const
	{
		return _source_sub_id;
	}

	void messaging_client::set_auto_echo(const bool& auto_echo, const unsigned short& echo_interval)
	{
		if (_session_type == session_types::binary_line)
		{
			logger::handle().write(logging_level::error, L"cannot set auto echo mode on binary line");
			return;
		}

		_auto_echo = auto_echo;
		_auto_echo_interval_seconds = echo_interval;
	}

	void messaging_client::set_bridge_line(const bool& bridge_line)
	{
		_bridge_line = bridge_line;
	}

	void messaging_client::set_encrypt_mode(const bool& encrypt_mode)
	{
		_encrypt_mode = encrypt_mode;
	}

	void messaging_client::set_compress_mode(const bool& compress_mode)
	{
		_compress_mode = compress_mode;
	}

	void messaging_client::set_compress_block_size(const unsigned short& compress_block_size)
	{
		_compress_block_size = compress_block_size;
	}

	void messaging_client::set_session_types(const session_types& session_type)
	{
		_session_type = session_type;
	}

	void messaging_client::set_connection_key(const wstring& connection_key)
	{
		_connection_key = connection_key;
	}

	void messaging_client::set_snipping_targets(const vector<wstring>& snipping_targets)
	{
		_snipping_targets = snipping_targets;
	}

	void messaging_client::set_connection_notification(const function<void(const wstring&, const wstring&, const bool&)>& notification)
	{
		_connection = notification;
	}

	void messaging_client::set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification)
	{
		_received_message = notification;
	}

	void messaging_client::set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification)
	{
		_received_file = notification;
	}

	void messaging_client::set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<uint8_t>&)>& notification)
	{
		_received_data = notification;
	}
		
	void messaging_client::set_specific_compress_sequence(const function<vector<uint8_t>(const vector<uint8_t>&, const bool&)>& specific_compress_sequence)
	{
		_specific_compress_sequence = specific_compress_sequence;	
	}
		
	void messaging_client::set_specific_encryp_sequence(const function<vector<uint8_t>(const vector<uint8_t>&, const bool&)>& specific_encrypt_sequence)
	{
		_specific_encrypt_sequence = specific_encrypt_sequence;
	}

	connection_conditions messaging_client::get_confirm_status(void) const
	{
		return _confirm;
	}

	void messaging_client::start(const wstring& ip, const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		create_thread_pool(high_priority, normal_priority, low_priority);

		logger::handle().write(logging_level::sequence, L"attempts to create io_context");

		_io_context = make_shared<asio::io_context>();

		logger::handle().write(logging_level::sequence, L"attempts to create socket");

		if (!create_socket(ip, port))
		{
			return;
		}

		_source_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->local_endpoint().address().to_string()), _socket->local_endpoint().port());
		_target_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port());
			
		read_start_code(_socket);

		send_connection();

		_thread = make_shared<thread>(bind(&messaging_client::run, this));
	}

	void messaging_client::stop(void)
	{
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}

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
	}

	bool messaging_client::echo(void)
	{
		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"echo",
			vector<shared_ptr<container::value>> {});

		return send(container);
	}

	bool messaging_client::send(const container::value_container& message)
	{
		return send(make_shared<container::value_container>(message));
	}

	bool messaging_client::send(shared_ptr<container::value_container> message)
	{
		if (_socket == nullptr)
		{
			return false;
		}

		if (message == nullptr)
		{
			return false;
		}

		if (_session_type == session_types::binary_line)
		{
			return false;
		}

		if (get_confirm_status() != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error, L"cannot send data on not confirmed line");

			return false;
		}

		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		auto serialize = message->serialize();
		auto serialize_array = converter::to_array(serialize);

		logger::handle().write(logging_level::packet, fmt::format(L"send: {}", serialize));

		send_packet_job(serialize_array);

		return true;
	}

	bool messaging_client::send_files(const container::value_container& message)
	{
		return send_files(make_shared<container::value_container>(message));
	}

	bool messaging_client::send_files(shared_ptr<container::value_container> message)
	{
		if (_socket == nullptr)
		{
			return false;
		}

		if (message == nullptr)
		{
			return false;
		}

		if (_session_type != session_types::file_line)
		{
			return false;
		}

		if (get_confirm_status() != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error, L"cannot send data on not confirmed line");

			return false;
		}

		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
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

		return true;
	}

	bool messaging_client::send_binary(const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data)
	{
		if (_socket == nullptr)
		{
			return false;
		}

		if (_session_type != session_types::binary_line)
		{
			return false;
		}

		if (get_confirm_status() != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error, L"cannot send data on not confirmed line");

			return false;
		}

		vector<uint8_t> result;
		combiner::append(result, converter::to_array(_source_id));
		combiner::append(result, converter::to_array(_source_sub_id));
		combiner::append(result, converter::to_array(target_id));
		combiner::append(result, converter::to_array(target_sub_id));
		combiner::append(result, data);

		logger::handle().write(logging_level::packet, fmt::format(L"send binary: source[{}:{}] -> target[{}:{}], {} bytes", _source_id, _source_sub_id, target_id, target_sub_id, data.size()));

		send_binary_job(result);

		return true;
	}

	void messaging_client::send_connection(void)
	{
		shared_ptr<container::container_value> snipping_targets = make_shared<container::container_value>(L"snipping_targets");
		for (auto& snipping_target : _snipping_targets)
		{
			snipping_targets->add(make_shared<container::string_value>(L"snipping_target", snipping_target));
		}

		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"request_connection",
			vector<shared_ptr<container::value>> {
				make_shared<container::string_value>(L"connection_key", _connection_key),
				make_shared<container::bool_value>(L"auto_echo", _auto_echo),
				make_shared<container::ushort_value>(L"auto_echo_interval_seconds", _auto_echo_interval_seconds),
				make_shared<container::short_value>(L"session_type", (short)_session_type),
				make_shared<container::bool_value>(L"bridge_mode", _bridge_line),
				snipping_targets
		});

		auto serialize = container->serialize();
		auto serialize_array = converter::to_array(serialize);

#ifdef _DEBUG
		logger::handle().write(logging_level::packet, fmt::format(L"send: {}", serialize));
#endif

		send_packet_job(serialize_array);
	}

	void messaging_client::disconnected(void)
	{
		connection_notification(false);
	}

	void messaging_client::send_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	void messaging_client::send_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::file_mode, data);
	}

	void messaging_client::send_binary_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::binary_mode, data);
	}

	void messaging_client::normal_message(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error, L"cannot send data on not confirmed line");

			return;
		}

		if (_received_message != nullptr)
		{
			auto result = async(launch::async, _received_message, message);
		}
	}

	void messaging_client::confirm_message(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}
		
		logger::handle().write(logging_level::sequence, L"received confirm_message");

		_target_id = message->source_id();
		_target_sub_id = message->source_sub_id();
		_source_sub_id = message->target_sub_id();

		if (!message->get_value(L"confirm")->to_boolean())
		{
			connection_notification(false);

			return;
		}

		_confirm = connection_conditions::confirmed;
		_encrypt_mode = message->get_value(L"encrypt_mode")->to_boolean();

		if (_encrypt_mode)
		{
			_key = message->get_value(L"key")->to_string();
			_iv = message->get_value(L"iv")->to_string();
		}

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

			logger::handle().write(logging_level::information, fmt::format(L"accepted snipping target: {}", snipping_target->to_string()));
		}

		connection_notification(true);
	}

	void messaging_client::request_files(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error, L"cannot send data on not confirmed line");

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

	void messaging_client::echo_message(shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_confirm != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error, L"cannot send data on not confirmed line");

			return;
		}

		vector<shared_ptr<value>> response = (*message)[L"response"];
		if (!response.empty())
		{
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));

			return;
		}

		message->swap_header();

		message << make_shared<bool_value>(L"response", true);

		_thread_pool->push(make_shared<job>(priorities::top, message->serialize_array(), bind(&messaging_client::send_packet, this, placeholders::_1)));
	}

	void messaging_client::connection_notification(const bool& condition)
	{
		logger::handle().write(logging_level::information, fmt::format(L"{} a client {} {}[{}]", 
			(condition?L"connected":L"disconnected"), (condition?L"to":L"from"), _target_id, _target_sub_id));

		if (!condition)
		{
			_confirm = connection_conditions::expired;
		}

		if(_connection != nullptr)
		{
			auto result = async(launch::async, _connection, _target_id, _target_sub_id, condition);
		}
	}

	bool messaging_client::create_socket(const wstring& ip, const unsigned short& port)
	{
		try
		{
			_socket = make_shared<asio::ip::tcp::socket>(*_io_context);
			_socket->open(asio::ip::tcp::v4());
			_socket->bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
			_socket->connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(converter::to_string(ip)), port));

			_socket->set_option(asio::ip::tcp::no_delay(true));
			_socket->set_option(asio::socket_base::keep_alive(true));
			_socket->set_option(asio::socket_base::receive_buffer_size(buffer_size));

			return true;
		}
		catch (const overflow_error&) {
			connection_notification(false);

			return false;
		}
		catch (const runtime_error&) {
			connection_notification(false);

			return false;
		}
		catch (const exception&) {
			connection_notification(false);

			return false;
		}
		catch (...) {
			connection_notification(false);

			return false;
		}
	}

	void messaging_client::run(void)
	{
		try
		{
			logger::handle().write(logging_level::information, 
				fmt::format(L"start messaging_client({})", _source_id));
			_io_context->run();
		}
		catch (const overflow_error&) { 
		}
		catch (const runtime_error&) { 
		}
		catch (const exception&) { 
		}
		catch (...) { 
		}

		logger::handle().write(logging_level::information, fmt::format(L"stop messaging_client({})", _source_id));
		connection_notification(false);
	}

	void messaging_client::create_thread_pool(const unsigned short& high_priority, const unsigned short& normal_priority, 
		const unsigned short& low_priority)
	{
		_thread_pool = make_shared<threads::thread_pool>(L"messaging_client");

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
	}
}
