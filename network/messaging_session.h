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

#include "container.h"
#include "thread_pool.h"
#include "data_handling.h"
#include "session_types.h"
#include "constexpr_string.h"
#include "connection_conditions.h"

#include <map>
#include <memory>
#include <string>
#include <functional>

#include "asio.hpp"

using namespace std;

namespace network
{
	class messaging_session : public enable_shared_from_this<messaging_session>, data_handling
	{
	public:
		messaging_session(const wstring& source_id, const wstring& connection_key, asio::ip::tcp::socket& socket, 
			const unsigned char& start_code_value, const unsigned char& end_code_value);
		~messaging_session(void);

	public:
		shared_ptr<messaging_session> get_ptr(void);

	public:
		void set_kill_code(const bool& kill_code);
		void set_acceptable_target_ids(const vector<wstring>& acceptable_target_ids);
		void set_ignore_target_ids(const vector<wstring>& ignore_target_ids);
		void set_ignore_snipping_targets(const vector<wstring>& ignore_snipping_targets);
		void set_connection_notification(const function<void(shared_ptr<messaging_session>, const bool&)>& notification);
		void set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification);
		void set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification);
		void set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<uint8_t>&)>& notification);
		void set_specific_compress_sequence(const function<vector<uint8_t>(const vector<uint8_t>&, const bool&)>& specific_compress_sequence);
		void set_specific_encrypt_sequence(const function<vector<uint8_t>(const vector<uint8_t>&, const bool&)>& specific_encrypt_sequence);

	public:
		const connection_conditions get_confirm_status(void);
		const session_types get_session_type(void);
		const wstring target_id(void);
		const wstring target_sub_id(void);

	public:
		void start(const bool& encrypt_mode, const bool& compress_mode, const unsigned short& compress_block_size, 
			const vector<session_types>& possible_session_types, const unsigned short& high_priority = 8, 
			const unsigned short& normal_priority = 8, const unsigned short& low_priority = 8, const unsigned short& drop_connection_time = 1);
		void stop(void);

	public:
		void echo(void);
		bool send(shared_ptr<container::value_container> message);
		void send_files(shared_ptr<container::value_container> message);
		void send_binary(const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data);
		void send_binary(const wstring& source_id, const wstring& source_sub_id, const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data);

	protected:
		void disconnected(void) override;

	protected:
		bool contained_snipping_target(const wstring& snipping_target);
		bool check_confirm_condition(void);
		bool send_auto_echo(void);
		
	private:
		void send_packet(const vector<uint8_t>& data) override;
		void send_file_packet(const vector<uint8_t>& data) override;
		void send_binary_packet(const vector<uint8_t>& data) override;

	private:
		void normal_message(shared_ptr<container::value_container> message) override;
		void connection_message(shared_ptr<container::value_container> message);
		void request_files(shared_ptr<container::value_container> message);
		void echo_message(shared_ptr<container::value_container> message);

	private:
		void generate_key(void);
		bool same_key_check(shared_ptr<container::value> key);
		bool same_id_check(void);

	private:
		bool _kill_code;
		bool _auto_echo;
		bool _bridge_line;
		session_types _session_type;
		unsigned short _drop_connection_time;
		wstring _source_id;
		wstring _source_sub_id;
		wstring _target_id;
		wstring _target_sub_id;
		wstring _connection_key;
		vector<wstring> _snipping_targets;
		vector<wstring> _acceptable_target_ids;
		vector<wstring> _ignore_target_ids;
		vector<wstring> _ignore_snipping_targets;
		vector<session_types> _possible_session_types;
		unsigned short _auto_echo_interval_seconds;

	private:
		shared_ptr<asio::ip::tcp::socket> _socket;
		function<void(shared_ptr<messaging_session>, const bool&)> _connection;
	};
}