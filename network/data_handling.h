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

#include "data_modes.h"
#include "data_lengths.h"
#include "thread_pool.h"
#include "connection_conditions.h"

#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <system_error>

#include "asio.hpp"

#ifdef __USE_TYPE_CONTAINER__
#include "container.h"
#else
#include "cpprest/json.h"
#endif

namespace network
{
	using namespace std;

#ifndef __USE_TYPE_CONTAINER__
	using namespace web;
#endif

	class data_handling
	{
	public:
		data_handling(const unsigned char& start_code_value, const unsigned char& end_code_value);
		~data_handling(void);

	protected:
		//void read_buffer(weak_ptr<asio::ip::tcp::socket> socket);
		
	protected:
		void read_start_code(weak_ptr<asio::ip::tcp::socket> socket);
		void read_packet_code(weak_ptr<asio::ip::tcp::socket> socket);
		void read_length_code(const data_modes& packet_mode, weak_ptr<asio::ip::tcp::socket> socket);
		void read_data(const data_modes& packet_mode, const size_t& remained_length, weak_ptr<asio::ip::tcp::socket> socket);
		void read_end_code(const data_modes& packet_mode, weak_ptr<asio::ip::tcp::socket> socket);

	protected:
		bool send_on_tcp(weak_ptr<asio::ip::tcp::socket> socket, const data_modes& data_mode, const vector<uint8_t>& data);
		void receive_on_tcp(const data_modes& data_mode, const vector<uint8_t>& data);

	protected:
		virtual void disconnected(void) = 0;

#ifndef __USE_TYPE_CONTAINER__
		virtual void normal_message(shared_ptr<json::value> message) = 0;
#else
		virtual void normal_message(shared_ptr<container::value_container> message) = 0;
#endif

	protected:
		void send_packet_job(const vector<uint8_t>& data);
		void send_file_job(const vector<uint8_t>& data);
		void send_binary_job(const vector<uint8_t>& data);

		// packet
	protected:
		void compress_packet(const vector<uint8_t>& data);
		void encrypt_packet(const vector<uint8_t>& data);
		virtual void send_packet(const vector<uint8_t>& data) = 0;

	protected:
		void decompress_packet(const vector<uint8_t>& data);
		void decrypt_packet(const vector<uint8_t>& data);
		void receive_packet(const vector<uint8_t>& data);

		// file
	protected:
		void load_file_packet(const vector<uint8_t>& data);
		void compress_file_packet(const vector<uint8_t>& data);
		void encrypt_file_packet(const vector<uint8_t>& data);
		virtual void send_file_packet(const vector<uint8_t>& data) = 0;

	protected:
		void decompress_file_packet(const vector<uint8_t>& data);
		void decrypt_file_packet(const vector<uint8_t>& data);
		void receive_file_packet(const vector<uint8_t>& data);
		void notify_file_packet(const vector<uint8_t>& data);

		// binary
	protected:
		void compress_binary_packet(const vector<uint8_t>& data);
		void encrypt_binary_packet(const vector<uint8_t>& data);
		virtual void send_binary_packet(const vector<uint8_t>& data) = 0;

	protected:
		void decompress_binary_packet(const vector<uint8_t>& data);
		void decrypt_binary_packet(const vector<uint8_t>& data);
		void receive_binary_packet(const vector<uint8_t>& data);

	protected:
#ifndef __USE_TYPE_CONTAINER__
		function<void(shared_ptr<json::value>)> _received_message;
#else
		function<void(shared_ptr<container::value_container>)> _received_message;
#endif

		function<void(const wstring&, const wstring&, const wstring&, const wstring&)> _received_file;
		function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<uint8_t>&)> _received_data;

	protected:
		shared_ptr<threads::thread_pool> _thread_pool;

#ifndef __USE_TYPE_CONTAINER__
		map<wstring, function<void(shared_ptr<json::value>)>> _message_handlers;
#else
		map<wstring, function<void(shared_ptr<container::value_container>)>> _message_handlers;
#endif

	protected:
		connection_conditions _confirm;
		bool _compress_mode;
		unsigned short _compress_block_size;

		bool _encrypt_mode;
		wstring _key;
		wstring _iv;

	private:
		char _start_code_tag[start_code];
		char _end_code_tag[end_code];
		char _receiving_buffer[buffer_size];
		vector<uint8_t> _received_data_vector;
	};
}