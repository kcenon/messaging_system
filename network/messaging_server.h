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

#include "constexpr_string.h"
#include "session_types.h"

#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "asio.hpp"

namespace network
{
	class messaging_session;
	class messaging_server
		: public std::enable_shared_from_this<messaging_server>
	{
	public:
		messaging_server(const std::string& source_id,
						 const unsigned char& start_code_value = 231,
						 const unsigned char& end_code_value = 67);
		~messaging_server(void);

	public:
		std::shared_ptr<messaging_server> get_ptr(void);

	public:
		void set_encrypt_mode(const bool& encrypt_mode);
		void set_compress_mode(const bool& compress_mode);
		void set_compress_block_size(const unsigned short& compress_block_size);
		void set_use_message_response(const bool& use_message_response);
		void set_drop_connection_time(
			const unsigned short& drop_connection_time);
		void set_connection_key(const std::string& connection_key);
		void set_acceptable_target_ids(
			const std::vector<std::string>& acceptable_target_ids);
		void set_ignore_target_ids(
			const std::vector<std::string>& ignore_target_ids);
		void set_ignore_snipping_targets(
			const std::vector<std::string>& ignore_snipping_targets);
		void set_possible_session_types(
			const std::vector<session_types>& possible_session_types);
		void set_session_limit_count(const size_t& session_limit_count);

	public:
		void set_connection_notification(
			const std::function<void(const std::string&,
									 const std::string&,
									 const bool&)>& notification);
		void set_message_notification(
			const std::function<void(
				std::shared_ptr<container::value_container>)>& notification);
		void set_file_notification(
			const std::function<void(const std::string&,
									 const std::string&,
									 const std::string&,
									 const std::string&)>& notification);
		void set_binary_notification(
			const std::function<void(const std::string&,
									 const std::string&,
									 const std::string&,
									 const std::string&,
									 const std::vector<uint8_t>&)>&
				notification);
		void set_specific_compress_sequence(
			const std::function<
				std::vector<uint8_t>(const std::vector<uint8_t>&, const bool&)>&
				specific_compress_sequence);
		void set_specific_encrypt_sequence(
			const std::function<
				std::vector<uint8_t>(const std::vector<uint8_t>&, const bool&)>&
				specific_encrypt_sequence);

	public:
		void start(const unsigned short& port,
				   const unsigned short& high_priority = 8,
				   const unsigned short& normal_priority = 8,
				   const unsigned short& low_priority = 8);
		void wait_stop(const unsigned int& seconds = 0);
		void stop(void);

	public:
		void disconnect(const std::string& target_id,
						const std::string& target_sub_id);

	public:
		void echo(void);
		bool send(const container::value_container& message,
				  std::optional<session_types> type
				  = session_types::message_line);
		bool send(std::shared_ptr<container::value_container> message,
				  std::optional<session_types> type
				  = session_types::message_line);
		void send_files(const container::value_container& message);
		void send_files(std::shared_ptr<container::value_container> message);
		void send_binary(const std::string& target_id,
						 const std::string& target_sub_id,
						 const std::vector<uint8_t>& data);
		void send_binary(const std::string& source_id,
						 const std::string& source_sub_id,
						 const std::string& target_id,
						 const std::string& target_sub_id,
						 const std::vector<uint8_t>& data);

	protected:
		void wait_connection(void);
		void connect_condition(std::shared_ptr<messaging_session> target,
							   const bool& condition);

	private:
		std::vector<std::shared_ptr<messaging_session>> current_sessions(void);
		void received_message(
			std::shared_ptr<container::value_container> message);
		void received_binary(const std::string& source_id,
							 const std::string& source_sub_id,
							 const std::string& target_id,
							 const std::string& target_sub_id,
							 const std::vector<uint8_t>& data);

	private:
		bool encrypt_mode_;
		bool compress_mode_;
		bool _use_message_response;
		unsigned short compress_block_size_;
		unsigned short _drop_connection_time;
		std::string source_id_;
		std::string connection_key_;
		unsigned short _high_priority;
		unsigned short _normal_priority;
		unsigned short _low_priority;
		size_t _session_limit_count;
		std::vector<std::string> _acceptable_target_ids;
		std::vector<std::string> _ignore_target_ids;
		std::vector<std::string> _ignore_snipping_targets;
		std::vector<session_types> _possible_session_types;

	private:
		unsigned char _start_code_value;
		unsigned char _end_code_value;

	private:
		std::shared_ptr<std::thread> thread_;
		std::shared_ptr<asio::io_context> io_context_;
		std::shared_ptr<asio::ip::tcp::acceptor> _acceptor;

	private:
		std::optional<std::promise<bool>> _promise_status;
		std::future<bool> _future_status;
		std::vector<std::shared_ptr<messaging_session>> _sessions;

	private:
		std::function<void(const std::string&, const std::string&, const bool&)>
			connection_;
		std::function<void(std::shared_ptr<container::value_container>)>
			received_message_;

		std::function<void(const std::string&,
						   const std::string&,
						   const std::string&,
						   const std::string&)>
			received_file_;
		std::function<void(const std::string&,
						   const std::string&,
						   const std::string&,
						   const std::string&,
						   const std::vector<uint8_t>&)>
			received_data_;

	private:
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
										   const bool&)>
			specific_compress_sequence_;
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
										   const bool&)>
			specific_encrypt_sequence_;
	};
} // namespace network