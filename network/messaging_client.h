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

#pragma once

#ifdef __USE_TYPE_CONTAINER__
#include "container.h"
#else
#include "cpprest/json.h"
#endif

#include "thread_pool.h"
#include "data_handling.h"
#include "session_types.h"
#include "constexpr_string.h"

#include <map>
#include <memory>
#include <string>
#include <functional>

#include <thread>
#include "asio.hpp"

namespace network
{
	using namespace std;

#ifndef __USE_TYPE_CONTAINER__
	using namespace web;
#endif

	class messaging_client : public enable_shared_from_this<messaging_client>, data_handling
	{
	public:
		messaging_client(const wstring& source_id, const unsigned char& start_code_value = 231, const unsigned char& end_code_value = 67);
		~messaging_client(void);

	public:
		shared_ptr<messaging_client> get_ptr(void);

	public:
		wstring source_id(void) const;
		wstring source_sub_id(void) const;

	public:
		void set_auto_echo(const bool& auto_echo, const unsigned short& echo_interval);
		void set_bridge_line(const bool& bridge_line);
		void set_compress_mode(const bool& compress_mode);
		void set_session_types(const session_types& session_type);
		void set_connection_key(const wstring& connection_key);
		void set_snipping_targets(const vector<wstring>& snipping_targets);

	public:
		void set_connection_notification(const function<void(const wstring&, const wstring&, const bool&)>& notification);

#ifndef __USE_TYPE_CONTAINER__
		void set_message_notification(const function<void(shared_ptr<json::value>)>& notification);
#else
		void set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification);
#endif

		void set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification);
		void set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)>& notification);

	public:
		bool is_confirmed(void) const;
		void start(const wstring& ip, const unsigned short& port, const unsigned short& high_priority = 8, 
			const unsigned short& normal_priority = 8, const unsigned short& low_priority = 8);
		void stop(void);

	public:
		void echo(void);

#ifndef __USE_TYPE_CONTAINER__
		void send(const json::value& message);
		void send(shared_ptr<json::value> message);
		void send_files(const json::value& message);
		void send_files(shared_ptr<json::value> message);
#else
		void send(const container::value_container& message);
		void send(shared_ptr<container::value_container> message);
		void send_files(const container::value_container& message);
		void send_files(shared_ptr<container::value_container> message);
#endif

		void send_binary(const wstring& target_id, const wstring& target_sub_id, const vector<unsigned char>& data);

	protected:
		void send_connection(void);
		void receive_on_tcp(const data_modes& data_mode, const vector<unsigned char>& data) override;
		void disconnected(void) override;

		// packet
	private:
		void compress_packet(const vector<unsigned char>& data);
		void encrypt_packet(const vector<unsigned char>& data);
		void send_packet(const vector<unsigned char>& data);

	private:
		void decompress_packet(const vector<unsigned char>& data);
		void decrypt_packet(const vector<unsigned char>& data);
		void receive_packet(const vector<unsigned char>& data);

		// file
	private:
		void load_file_packet(const vector<unsigned char>& data);
		void compress_file_packet(const vector<unsigned char>& data);
		void encrypt_file_packet(const vector<unsigned char>& data);
		void send_file_packet(const vector<unsigned char>& data);

	private:
		void decompress_file_packet(const vector<unsigned char>& data);
		void decrypt_file_packet(const vector<unsigned char>& data);
		void receive_file_packet(const vector<unsigned char>& data);
		void notify_file_packet(const vector<unsigned char>& data);

		// binary
	private:
		void compress_binary_packet(const vector<unsigned char>& data);
		void encrypt_binary_packet(const vector<unsigned char>& data);
		void send_binary_packet(const vector<unsigned char>& data);

	private:
		void decompress_binary_packet(const vector<unsigned char>& data);
		void decrypt_binary_packet(const vector<unsigned char>& data);
		void receive_binary_packet(const vector<unsigned char>& data);

	private:
#ifndef __USE_TYPE_CONTAINER__
		void normal_message(shared_ptr<json::value> message);
		void confirm_message(shared_ptr<json::value> message);
		void request_files(shared_ptr<json::value> message);
		void echo_message(shared_ptr<json::value> message);
#else
		void normal_message(shared_ptr<container::value_container> message);
		void confirm_message(shared_ptr<container::value_container> message);
		void request_files(shared_ptr<container::value_container> message);
		void echo_message(shared_ptr<container::value_container> message);
#endif

	private:
		void connection_notification(const bool& condition);

	private:
		bool create_socket(const wstring& ip, const unsigned short& port);
		void run(void);
		void create_thread_pool(const unsigned short& high_priority, const unsigned short& normal_priority, 
			const unsigned short& low_priority);

	private:
		bool _confirm;
		bool _auto_echo;
		bool _bridge_line;
		session_types _session_type;
		wstring _source_id;
		wstring _source_sub_id;
		wstring _target_id;
		wstring _target_sub_id;
		wstring _connection_key;
		unsigned short _auto_echo_interval_seconds;
		vector<wstring> _snipping_targets;

	private:
		function<void(const wstring&, const wstring&, const bool&)> _connection;

	private:
		shared_ptr<thread> _thread;
		shared_ptr<asio::ip::tcp::socket> _socket;
		shared_ptr<asio::io_context> _io_context;
	};
}
